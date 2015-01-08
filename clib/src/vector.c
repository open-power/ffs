/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/src/vector.c $                                           */
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
 *   File: vector.c
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr: dynamic vector
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

#include "libclib.h"

#include "vector.h"
#include "tree.h"
#include "tree_iter.h"
#include "mq.h"

/* ======================================================================= */

/*! @cond */
#define VECTOR_NODE_MAGIC	"VCND"

#define VECTOR_NODE_MAGIC_CHECK(m) ({                           \
    bool rc = (((m)[0] != VECTOR_NODE_MAGIC[0]) ||              \
               ((m)[1] != VECTOR_NODE_MAGIC[1]) ||              \
               ((m)[2] != VECTOR_NODE_MAGIC[2]) ||              \
               ((m)[3] != VECTOR_NODE_MAGIC[3]));               \
    rc;                                                         \
                                    })

typedef struct vector_node vector_node_t;

struct vector_node {
	uint8_t magic[4];

	uint32_t address;
	tree_node_t node;

	uint8_t data[];
};

#define VECTOR_PAGE_MAX		UINT16_MAX
#define VECTOR_PAGE_DIVISOR	32

#define __index_to_page(i,s)                                    \
({                                                              \
    typeof(i) _p = ((i) / (s));                                 \
    _p;                                      			\
})

#define __index_to_page_hashed(i,s)                             \
({                                                              \
    typeof(i) _h = int32_hash1(__index_to_page((i),(s)));       \
    _h;                                                         \
})
/*! @endcond */

/* ======================================================================= */

static vector_node_t *__vector_find_page(vector_t * self, size_t idx)
{
	const void *hash;
	hash = (const void *)__index_to_page_hashed(idx, self->hdr.elem_num);

	tree_node_t *node = tree_find(&self->tree, hash);
	if (unlikely(node == NULL)) {
		UNEXPECTED("'%d' index out of range", idx);
		return NULL;
	}

	return container_of(node, vector_node_t, node);
}

static int __vector_shrink(vector_t * self)
{
	assert(self != NULL);

	vector_node_t *node = __vector_find_page(self,
						 vector_capacity(self) - 1);
	assert(node != NULL);

	int rc = splay_remove(&self->tree, &node->node);

	free(node);
	self->hdr.page_count--;

	return rc;
}

static vector_node_t *__vector_grow(vector_t * self)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));

	vector_node_t *node = NULL;
	int rc = posix_memalign((void **)&node, sizeof(void *),
				self->hdr.page_size);
	if (rc != 0) {
		ERRNO(errno);
		return NULL;
	}

	memset(node, 0, self->hdr.page_size);

	node->magic[0] = VECTOR_NODE_MAGIC[0];
	node->magic[1] = VECTOR_NODE_MAGIC[1];
	node->magic[2] = VECTOR_NODE_MAGIC[2];
	node->magic[3] = VECTOR_NODE_MAGIC[3];

	node->address = (uint32_t) node;

	size_t hash = __index_to_page_hashed(vector_capacity(self),
					     self->hdr.elem_num);

	tree_node_init(&node->node, (const void *)hash);
	if (splay_insert(&self->tree, &node->node) < 0) {
		free(node);
		return NULL;
	}
	self->hdr.page_count++;

	return node;
}

static int __vector_compare(const int i1, const int i2)
{
	return i1 - i2;
}

/* ======================================================================= */

int vector_init3(vector_t * self, const char *name, size_t elem_size)
{
	size_t page_size = max(sysconf(_SC_PAGESIZE),
			       __round_pow2(elem_size * VECTOR_PAGE_DIVISOR));
	return vector_init4(self, name, elem_size, page_size);
}

int vector_init4(vector_t * self, const char *name, size_t elem_size,
		 size_t page_size)
{
	assert(self != NULL);

	if (unlikely(MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC)) == false)
		vector_delete(self);

	if (elem_size < VECTOR_ELEM_MIN || VECTOR_ELEM_MAX < elem_size) {
		UNEXPECTED("'%d' elem_size out of range [%d..%d]",
			   elem_size, VECTOR_ELEM_MIN, VECTOR_ELEM_MAX);
		return -1;
	}

	page_size = __round_pow2(page_size);
	if (page_size / elem_size < VECTOR_PAGE_DIVISOR) {
		UNEXPECTED("'%d' page_size out of range [%d..%d]",
			   page_size, elem_size * VECTOR_PAGE_DIVISOR,
			   VECTOR_PAGE_MAX);
		return -1;
	}

	memset(self, 0, sizeof *self);

	self->hdr.id[IDENT_MAGIC_0] = VECTOR_MAGIC[IDENT_MAGIC_0];
	self->hdr.id[IDENT_MAGIC_1] = VECTOR_MAGIC[IDENT_MAGIC_1];
	self->hdr.id[IDENT_MAGIC_2] = VECTOR_MAGIC[IDENT_MAGIC_2];
	self->hdr.id[IDENT_MAGIC_3] = VECTOR_MAGIC[IDENT_MAGIC_3];

	self->hdr.id[IDENT_MAJOR] = CLIB_MAJOR;
	self->hdr.id[IDENT_MINOR] = CLIB_MINOR;
	self->hdr.id[IDENT_PATCH] = CLIB_PATCH;

	if (__BYTE_ORDER == __LITTLE_ENDIAN)
		self->hdr.id[IDENT_FLAGS] |= VECTOR_FLAG_LSB;
	if (__BYTE_ORDER == __BIG_ENDIAN)
		self->hdr.id[IDENT_FLAGS] |= VECTOR_FLAG_MSB;

	self->hdr.page_size = page_size;
	self->hdr.elem_size = elem_size;
	self->hdr.elem_num = (self->hdr.page_size - sizeof(vector_node_t)) /
	    self->hdr.elem_size;

	if (name != NULL && *name != '\0')
		strncpy(self->hdr.name, name, sizeof(self->hdr.name));

	tree_init(&self->tree, (compare_f) __vector_compare);

	return 0;
}

int vector_delete(vector_t * self)
{
	if (unlikely(self == NULL))
		return 0;

	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));

	tree_iter_t it;
	tree_iter_init(&it, &self->tree, TI_FLAG_FWD);

	vector_node_t *node;
	tree_for_each(&it, node, node) {
		if (VECTOR_NODE_MAGIC_CHECK(node->magic)) {
			UNEXPECTED("'%s' invalid or corrupt vector_node"
				   "object => '%.4s'", self->hdr.name,
				   node->magic);
			return -1;
		}

		if (splay_remove(&self->tree, &node->node) < 0)
			return -1;

		memset(node, 0, sizeof(*node));
		free(node);
	}

	self->hdr.page_count = self->hdr.size = 0;

	return 0;
}

const void *vector_at(vector_t * self, size_t idx)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));
	assert(idx < self->hdr.size);

	vector_node_t *node = __vector_find_page(self, idx);
	return node->data + (self->hdr.elem_size * (idx % self->hdr.elem_num));
}

int vector_get3(vector_t * self, size_t idx, void *ptr)
{
	return vector_get4(self, idx, ptr, 1);
}

int vector_get4(vector_t * self, size_t idx, void *ptr, size_t count)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));

	while (0 < count) {
		memcpy(ptr, vector_at(self, idx), self->hdr.elem_size);

		idx++;
		count--;

		ptr += self->hdr.elem_size;
	}

	return 0;
}

static inline int __vector_put(vector_t * self, size_t idx, const void *ptr)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));

	vector_node_t *node = __vector_find_page(self, idx);
	assert(node != NULL);

	if (VECTOR_NODE_MAGIC_CHECK(node->magic)) {
		UNEXPECTED("'%s' invalid or corrupt vector_node object => "
			   "'%.4s'", self->hdr.name, node->magic);
		return -1;
	}

	memcpy(node->data + (self->hdr.elem_size * (idx % self->hdr.elem_num)),
	       ptr, self->hdr.elem_size);

	return 0;
}

int vector_put3(vector_t * self, size_t idx, const void *ptr)
{
	return vector_put4(self, idx, ptr, 1);
}

int vector_put4(vector_t * self, size_t idx, const void *ptr, size_t count)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));
	assert(idx < self->hdr.size);

	while (0 < count) {
		if (__vector_put(self, idx, ptr) < 0)
			return -1;

		idx++;
		count--;

		ptr += self->hdr.elem_size;
	}

	return 0;
}

size_t vector_size1(vector_t * self)
{
	assert(self != NULL);
	return self->hdr.size;
}

int vector_size2(vector_t * self, size_t size)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));

	size_t pages = __index_to_page(size, self->hdr.elem_num) + 1;

	if (vector_pages(self) < pages) {
		while (vector_pages(self) < pages)
			(void)__vector_grow(self);
	} else if (pages < vector_pages(self)) {
		if (size <= 0)
			vector_delete(self);
		else
			while (pages < vector_pages(self))
				if (__vector_shrink(self) < 0)
					return -1;
	}

	return self->hdr.size = size;
}

size_t vector_pages(vector_t * self)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));
	return self->hdr.page_count;
}

size_t vector_capacity(vector_t * self)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));
	return self->hdr.page_count * self->hdr.elem_num;
}

size_t vector_elem_size(vector_t * self)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));
	return self->hdr.elem_size;
}

ssize_t vector_save(vector_t * self, FILE * out)
{
	assert(self != NULL);
	assert(out != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));

	int header_swap(vector_header_t * hdr) {
		assert(hdr != NULL);

		if (hdr->id[IDENT_FLAGS] & VECTOR_FLAG_MSB) {
			hdr->page_size = htobe32(hdr->page_size);
			hdr->elem_size = htobe16(hdr->elem_size);
			hdr->elem_num = htobe16(hdr->elem_num);
			hdr->size = htobe32(hdr->size);
			hdr->page_count = htobe32(hdr->page_count);
		} else if (hdr->id[IDENT_FLAGS] & VECTOR_FLAG_LSB) {
			hdr->page_size = htole32(hdr->page_size);
			hdr->elem_size = htole16(hdr->elem_size);
			hdr->elem_num = htole16(hdr->elem_num);
			hdr->size = htole32(hdr->size);
			hdr->page_count = htole32(hdr->page_count);
		} else {
			UNEXPECTED("'%s' invalid or corrupt flash object => "
				   "'%x'", hdr->name, hdr->id[IDENT_FLAGS]);
			return -1;
		}

		return 0;
	}

	ssize_t save(vector_t * self, FILE * out) {
		tree_iter_t it;
		tree_iter_init(&it, &self->tree, TI_FLAG_FWD);

		ssize_t len = 0;

		vector_node_t *node;
		tree_for_each(&it, node, node) {
			if (VECTOR_NODE_MAGIC_CHECK(node->magic)) {
				UNEXPECTED("'%s' invalid or corrupt vector_node"
					   "object => '%.4s'", self->hdr.name,
					   node->magic);
				return -1;
			}

			size_t rc;

			vector_node_t copy = *node;

			copy.address = 0;
			copy.node.left = copy.node.right = NULL;
			copy.node.parent = NULL;

			rc = fwrite((char *)&copy, 1, sizeof(copy), out);
			if (rc != sizeof(copy)) {
				if (ferror(out)) {
					ERRNO(errno);
					return -1;
				}
			}
			len += rc;

			rc = fwrite((char *)node->data, 1,
				    self->hdr.page_size - sizeof(*node), out);
			if (rc != self->hdr.page_size - sizeof(*node)) {
				if (ferror(out)) {
					ERRNO(errno);
					return -1;
				}
			}
			len += rc;
		}

		return len;
	}

	ssize_t total = 0;

	vector_header_t hdr = self->hdr;
	if (header_swap(&hdr) < 0)
		return -1;

	clearerr(out);
	total = fwrite(&hdr, 1, sizeof(hdr), out);
	if (total != sizeof(hdr)) {
		if (ferror(out)) {
			ERRNO(errno);
			return -1;
		}
	}

	total += save(self, out);

	return total;
}

ssize_t vector_load(vector_t * self, FILE * in)
{
	assert(self != NULL);
	assert(in != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));

	int header_swap(vector_header_t * hdr) {
		assert(hdr != NULL);

		if (hdr->id[IDENT_FLAGS] & VECTOR_FLAG_MSB) {
			hdr->page_size = be32toh(hdr->page_size);
			hdr->elem_size = be16toh(hdr->elem_size);
			hdr->elem_num = be16toh(hdr->elem_num);
			hdr->size = be32toh(hdr->size);
			hdr->page_count = be32toh(hdr->page_count);
		} else if (hdr->id[IDENT_FLAGS] & VECTOR_FLAG_LSB) {
			hdr->page_size = le32toh(hdr->page_size);
			hdr->elem_size = le16toh(hdr->elem_size);
			hdr->elem_num = le16toh(hdr->elem_num);
			hdr->size = le32toh(hdr->size);
			hdr->page_count = le32toh(hdr->page_count);
		} else {
			UNEXPECTED("'%s' invalid or corrupt flash object => "
				   "'%x'", hdr->name, hdr->id[IDENT_FLAGS]);
			return -1;
		}

		return 0;
	}

	vector_delete(self);

	clearerr(in);
	ssize_t len = fread(&self->hdr, 1, sizeof(self->hdr), in);
	if (len != sizeof(self->hdr)) {
		if (feof(in)) {
			UNEXPECTED("'%s' end-of-file encountered",
				   self->hdr.name);
			return -1;
		}
		if (ferror(in)) {
			ERRNO(errno);
			return -1;
		}
	}

	if (header_swap(&self->hdr) < 0)
		return -1;

	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));

	tree_init(&self->tree, (compare_f) __vector_compare);
	vector_node_t *node = NULL;

	for (size_t i = 0; i < vector_pages(self); i++) {
		size_t rc = posix_memalign((void **)&node, sizeof(void *),
					   self->hdr.page_size);
		if (rc != 0) {
			ERRNO(errno);
			return -1;
		}
		memset(node, 0, self->hdr.page_size);

		rc = fread((void *)node, 1, self->hdr.page_size, in);
		if (rc != self->hdr.page_size) {
			if (feof(in)) {
				UNEXPECTED("'%s' end-of-file encountered",
					   self->hdr.name);
				return -1;
			}
			if (ferror(in)) {
				ERRNO(errno);
				return -1;
			}
		}

		len += rc;

		if (VECTOR_NODE_MAGIC_CHECK(node->magic)) {
			UNEXPECTED("'%s' invalid or corrupt vector_node "
				   "object => '%.4s'", self->hdr.name,
				   node->magic);
			return -1;
		}

		node->address = (uint32_t) node;
		tree_node_init(&node->node, node->node.key);
		splay_insert(&self->tree, &node->node);

		node = NULL;
	}

	return len;
}

int vector_send(vector_t * self, mqueue_t * mq)
{
	assert(self != NULL);
	assert(mq != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));

	mqueue_send(mq, (char *)self, sizeof(*self));

	tree_iter_t it;
	tree_iter_init(&it, &self->tree, TI_FLAG_FWD);

	vector_node_t *node;
	tree_for_each(&it, node, node) {
		assert(!VECTOR_NODE_MAGIC_CHECK(node->magic));
		mqueue_send(mq, (char *)node, self->hdr.page_size);
	}

	return 0;
}

int vector_receive(vector_t * self, mqueue_t * mq)
{
	assert(self != NULL);
	assert(mq != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));

	vector_delete(self);

	mqueue_attr_t attr = mqueue_getattr(mq);

	vector_node_t *node = NULL;
	size_t rc = posix_memalign((void **)&node, attr.mq_msgsize,
				   attr.mq_msgsize);
	if (rc != 0) {
		ERRNO(errno);
		return -1;
	}

	ssize_t len = mqueue_receive(mq, (void *)node, attr.mq_msgsize);
	assert(0 < len);

	assert(!MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC));

	memcpy(self, (void *)node, sizeof(*self));
	tree_init(&self->tree, (compare_f) __vector_compare);

	for (size_t i = 0; i < vector_pages(self); i++) {
		rc = posix_memalign((void **)&node, attr.mq_msgsize,
				    attr.mq_msgsize);
		if (rc != 0) {
			ERRNO(errno);
			return -1;
		}

		len = mqueue_receive(mq, (void *)node, attr.mq_msgsize);
		assert(0 < len);

		assert(!VECTOR_NODE_MAGIC_CHECK(node->magic));

		node->address = (uint32_t) node;
		tree_node_init(&node->node, node->node.key);
		splay_insert(&self->tree, &node->node);

		node = NULL;
	}

	return 0;
}

void vector_dump(vector_t * self, FILE * out)
{
	if (out == NULL)
		out = stdout;

	if (self != NULL) {
		assert(!unlikely(MAGIC_CHECK(self->hdr.id, VECTOR_MAGIC)));

		fprintf(out, "%s: page_size: %d elem_size: %d elem_num: %d -- "
			"size: %d capacity: %d -- page_count: %d\n",
			self->hdr.name, self->hdr.page_size,
			self->hdr.elem_size, self->hdr.elem_num,
			vector_size(self), vector_capacity(self),
			self->hdr.page_count);

		tree_iter_t it;
		tree_iter_init(&it, &self->tree, TI_FLAG_FWD);

		vector_node_t *node;
		tree_for_each(&it, node, node) {
			fprintf(out, "magic[%.4s] node: %p data: %p -- "
				"address: %x\n", node->magic, &node->node,
				node->data, node->address);

			dump_memory(out, (unsigned long)node, node,
				    self->hdr.page_size);
		}
	}
}

/* ======================================================================= */
