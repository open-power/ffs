/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/src/array.c $                                            */
/*                                                                        */
/* OpenPOWER FFS Project                                                  */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

/*
 *   File: array.c
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr: Sparse Array
 *   Note:
 *   Date: 08/29/10
 */

#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "assert.h"
#include "misc.h"
#include "hash.h"
#include "array.h"
#include "slab.h"
#include "mqueue.h"
#include "tree.h"
#include "tree_iter.h"
#include "bb_trace.h"
#include "memory_leak_detection.h"

/*! @cond */
#define ARRAY_ELEM_SIZE_MIN	1
#define ARRAY_ELEM_NUM_MIN	64

#define ARRAY_MAGIC		0x48444152
#define ARRAY_NODE_MAGIC	0x4E444152
/*! @endcond */

/* ======================================================================= */

/*! @cond */
typedef struct array_node array_node_t;

struct array_node {
	uint32_t magic;
	uint32_t address;

	tree_node_t node;
};

const char *__array_msg[] = {
	"array: unexpected NULL pointer",
	"array: unexpected cache and/or alloc size",
	"array: out-of-memory",
	"array: uninitialized array element",
	"array: unexpected magic number",
	"array: unexpected NULL message channel",
};

#define ARRAY_NULL               (__array_msg[0])
#define ARRAY_SIZE               (__array_msg[1])
#define ARRAY_OOM                (__array_msg[2])
#define ARRAY_UNINIT_ELEM        (__array_msg[3])
#define ARRAY_MAGIC_CHECK        (__array_msg[4])
#define ARRAY_MQ_NULL            (__array_msg[5])
/*! @endcond */

/* ======================================================================= */

#define __index_to_page(i,s)                                    \
({                                                              \
    typeof(i) _p = ((i) / (s));                                 \
    (i) >= 0 ? _p :_p - 1;                                      \
})

#define __index_to_page_hashed(i,s)                             \
({                                                              \
    typeof(i) _h = int32_hash1(__index_to_page((i),(s)));       \
    _h;                                                         \
})

#define __page_to_low(p,s)                                      \
({                                                              \
    typeof(p) _l = (p) * (s);                                   \
    (p) >= 0 ? _l : _l - 1;                                     \
})

#define __page_to_high(p,s)                                     \
({                                                              \
    typeof(p) _h = (p) * (s) + (s) - (typeof(p))1;              \
    _h;                                                         \
})

/* ======================================================================= */

static array_node_t *__array_grow(array_t * self, size_t idx)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);

	array_node_t *node = NULL;
	switch (POSIX_MEMALIGN
		((void **)&node, self->page_size, self->page_size)) {
	case EINVAL:
		throw_unexpected(ARRAY_SIZE);
	case ENOMEM:
		throw_unexpected(ARRAY_OOM);
	}
	assert(node != NULL);

	node->magic = ARRAY_NODE_MAGIC;
	node->address = (uint32_t) node;

	const void *key =
	    (const void *)__index_to_page_hashed(idx, self->elem_num);
	tree_node_init(&node->node, key);

	splay_insert(&self->tree, &node->node);
	self->pages++;

	size_t page = __index_to_page(idx, self->elem_num);

	self->low = min(self->low, __page_to_low(page, self->elem_num));
	self->high = max(self->high, __page_to_high(page, self->elem_num));

	return node;
}

static int __array_compare(const int i1, const int i2)
{
	return i1 - i2;
}

void array_init(array_t * self, size_t elem_size, size_t elem_num)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);

	self->magic = ARRAY_MAGIC;
	self->high = 0;
	self->low = UINT32_MAX;
	self->size = 0;
	self->pages = 0;

	self->elem_size = max(elem_size, ARRAY_ELEM_SIZE_MIN);
	self->elem_num = max(elem_num, ARRAY_ELEM_NUM_MIN);

	self->page_size = __round_pow2(elem_size * elem_num);

	self->map_size = align(elem_num / CHAR_BIT, sizeof(uint32_t));

	self->elem_num =
	    (self->page_size - sizeof(array_node_t) -
	     self->map_size) / self->elem_size;

	tree_init(&self->tree, (compare_f) __array_compare);
}

static void __array_delete(array_t * self)
{
	if (self != NULL) {
		while (self->tree.root != NULL) {
			array_node_t *node;
			node =
			    container_of(tree_root(&self->tree), array_node_t,
					 node);
			splay_remove(&self->tree, &node->node);
			FREE(node);
		}
		tree_init(&self->tree, (compare_f) __array_compare);
	}
}

void array_delete(array_t * self)
{
	__array_delete(self);
}

static void *__array_find_page(array_t * self, size_t idx)
{
	const void *hash =
	    (const void *)__index_to_page_hashed(idx, self->elem_num);
	tree_node_t *node = tree_find(&self->tree, hash);

	if (node == NULL)
		return (void *)__array_grow(self, idx);
	else
		return (void *)container_of(node, array_node_t, node);

	return NULL;
}

static uint8_t *__array_find_map(array_t * self, size_t idx)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);

	const void *hash =
	    (const void *)__index_to_page_hashed(idx, self->elem_num);
	uint8_t *map = (uint8_t *) tree_find(&self->tree, hash);
	if (map != NULL)
		map += sizeof(tree_node_t);

	return map;
}

const void *array_at(array_t * self, size_t idx)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);

	uint32_t offset = idx % self->elem_num;

	uint8_t *map = __array_find_page(self, idx);
	map += sizeof(array_node_t);

	uint8_t byte = offset / CHAR_BIT;
	uint8_t mask = 0x80 >> (offset % CHAR_BIT);

	if ((map[byte] & mask) == 0)
		throw_unexpected(ARRAY_UNINIT_ELEM);

	return map + self->map_size + (self->elem_size * offset);
}

void array_get3(array_t * self, size_t elem_off, void *ptr)
{
	return array_get4(self, elem_off, ptr, 1);
}

void array_get4(array_t * self, size_t elem_off, void *ptr, size_t elem_num)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);

	while (0 < elem_num) {
		memcpy(ptr, array_at(self, elem_off), self->elem_size);

		elem_off++;
		elem_num--;

		ptr += self->elem_size;
	}
}

void __array_put(array_t * self, size_t elem_off, const void *ptr)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);

	uint32_t offset = elem_off % self->elem_num;

	uint8_t *map = __array_find_page(self, elem_off);
	map += sizeof(array_node_t);

	uint8_t byte = offset / CHAR_BIT;
	uint8_t mask = 0x80 >> (offset % CHAR_BIT);

	if ((map[byte] & mask) == 0)
		self->size++;

	map[byte] |= mask;

	memcpy(map + self->map_size + (self->elem_size * offset),
	       ptr, self->elem_size);
}

void array_put3(array_t * self, size_t elem_off, const void *ptr)
{
	return array_put4(self, elem_off, ptr, 1);
}

void array_put4(array_t * self, size_t elem_off, const void *ptr,
		size_t elem_num)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);

	while (0 < elem_num) {
		__array_put(self, elem_off, ptr);

		elem_off++;
		elem_num--;

		ptr += self->elem_size;
	}
}

bool array_status2(array_t * self, size_t idx)
{
	uint8_t *map = __array_find_map(self, idx);

	uint32_t offset = idx % self->elem_num;
	uint8_t byte = offset / CHAR_BIT;
	uint8_t mask = 0x80 >> (offset % CHAR_BIT);

	return ! !(map[byte] & mask);
}

bool array_status3(array_t * self, size_t idx, bool status)
{
	uint8_t *map = __array_find_map(self, idx);

	if (map == NULL) {
		map = (uint8_t *) __array_grow(self, idx);
		map += sizeof(array_node_t);
	}

	uint32_t offset = idx % self->elem_num;
	uint8_t byte = offset / CHAR_BIT;
	uint8_t mask = 0x80 >> (offset % CHAR_BIT);

	bool ret = ! !(map[byte] & mask);

	map[byte] &= ~mask;
	if (status)
		map[byte] |= mask;

	return ret;
}

void array_resize(array_t * self, size_t size)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);
	while (0 < size) {
		(void)__array_grow(self, self->high + 1);
		size /= self->elem_num;
	}
}

size_t array_size(array_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);
	return self->size;
}

size_t array_pages(array_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);
	return self->pages;
}

size_t array_capacity(array_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);
	return self->high - self->low + 1;
}

size_t array_low(array_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);
	return self->low;
}

size_t array_high(array_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);
	return self->high;
}

void array_send(array_t * self, mqueue_t * mq)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);
	if (unlikely(mq == NULL))
		throw_unexpected(ARRAY_MQ_NULL);

	mqueue_send(mq, (char *)self, sizeof(*self));

	tree_iter_t it;
	tree_iter_init(&it, &self->tree, TI_FLAG_FWD);

	array_node_t *node;
	tree_for_each(&it, node, node) {
		if (node->magic != ARRAY_NODE_MAGIC)
			throw_unexpected(ARRAY_MAGIC_CHECK);

		mqueue_send(mq, (char *)node, self->page_size);
	}
}

void array_receive(array_t * self, mqueue_t * mq)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_NULL);
	if (unlikely(mq == NULL))
		throw_unexpected(ARRAY_MQ_NULL);

	__array_delete(self);

	mqueue_attr_t attr = mqueue_getattr(mq);

	array_node_t *node = NULL;
	switch (POSIX_MEMALIGN
		((void **)&node, attr.mq_msgsize, attr.mq_msgsize)) {
	case EINVAL:
		throw_unexpected(ARRAY_SIZE);
	case ENOMEM:
		throw_unexpected(ARRAY_OOM);
	}
	assert(node != NULL);

	ssize_t len = mqueue_receive(mq, (void *)node, attr.mq_msgsize);
	assert(0 < len);

	memcpy(self, (void *)node, sizeof(*self));
	tree_init(&self->tree, (compare_f) __array_compare);

	for (int i = 0; i < array_pages(self); i++) {
		if (node == NULL) {
			switch (POSIX_MEMALIGN
				((void **)&node, attr.mq_msgsize,
				 attr.mq_msgsize)) {
			case EINVAL:
				throw_unexpected(ARRAY_SIZE);
			case ENOMEM:
				throw_unexpected(ARRAY_OOM);
			}
		}

		len = mqueue_receive(mq, (void *)node, attr.mq_msgsize);
		assert(0 < len);

		node->address = (uint32_t) node;
		tree_node_init(&node->node, node->node.key);
		splay_insert(&self->tree, &node->node);

		node = NULL;
	}
}

void array_dump(array_t * self, FILE * out)
{
	if (self != NULL) {
		fprintf(out,
			"array: [ page_size: %5u    pages: %5u map_size: %5u   capacity: %10u ]\n",
			self->page_size, self->pages, self->map_size,
			array_capacity(self));
		fprintf(out,
			"       [ elem_size: %5u elem_num: %5u     size: %10u range: (%u....%u) ]\n",
			self->elem_size, self->elem_num, array_size(self),
			array_low(self), array_high(self));

		dump_memory(out, (unsigned long)self, self, sizeof(*self));

		tree_iter_t it;
		for (tree_iter_init(&it, &self->tree, TI_FLAG_FWD);
		     tree_iter_elem(&it); tree_iter_inc(&it)) {
			array_node_t *node;
			node =
			    container_of(tree_iter_elem(&it), array_node_t,
					 node);

			fprintf(out, "magic[%x] address[%x]\n",
				node->magic, node->address);
			fprintf(out,
				"n[%p] left[%p] right[%p] parent[%p] key[%p] \n",
				&node->node, node->node.left, node->node.right,
				node->node.parent, node->node.key);

			dump_memory(out, (unsigned long)node, node,
				    self->page_size);
		}
	}
}

/* ======================================================================= */
