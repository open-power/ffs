/*
 * Copyright (c) International Business Machines Corp., 2014
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 *   File: array_iter.c
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr: dynamic array
 *   Note:
 *   Date: 10/22/10
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
#include "nargs.h"
#include "array_iter.h"

/* ======================================================================= */

const char *__array_iter_msg[] = {
	"array_iter: unexpected NULL pointer",
	"array_iter: index out-of-range",
};

#define ARRAY_ITER_NULL         (__array_iter_msg[0])
#define ARRAY_ITER_RANGE        (__array_iter_msg[1])

/* ======================================================================= */

static inline const void *__array_iter_bwd(array_iter_t * self)
{
	size_t low = array_low(self->array);
	const void *ret = NULL;

	if (low < self->idx)
		self->idx--;

	while (low < self->idx) {
		if (array_status(self->array, self->idx))
			break;
		self->idx--;
	}

	if (low < self->idx)
		ret = array_at(self->array, self->idx);

	return ret;
}

static inline const void *__array_iter_fwd(array_iter_t * self)
{
	size_t high = array_high(self->array);
	const void *ret = NULL;

	if (self->idx < high)
		self->idx++;

	while (self->idx < high) {
		if (array_status(self->array, self->idx))
			break;
		self->idx++;
	}

	if (self->idx < high)
		ret = array_at(self->array, self->idx);

	return ret;
}

void array_iter_init(array_iter_t * self, array_t * array, uint32_t flags)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_ITER_NULL);
	if (unlikely(array == NULL))
		throw_unexpected(ARRAY_ITER_NULL);

	self->flags = flags;
	self->array = array;

	if (self->flags & AI_FLAG_BWD) {
		self->idx = array_high(self->array);
		__array_iter_bwd(self);
	} else {
		self->idx = array_low(self->array);
		__array_iter_fwd(self);
	}
}

void array_iter_clear(array_iter_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_ITER_NULL);

	if (self->flags & AI_FLAG_BWD)
		self->idx = array_high(self->array);
	else
		self->idx = array_low(self->array);

	self->array = NULL;
}

const void *array_iter_elem(array_iter_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_ITER_NULL);
	if (self->idx < array_low(self->array))
		throw_unexpected(ARRAY_ITER_RANGE);
	if (array_high(self->array) <= self->idx)
		throw_unexpected(ARRAY_ITER_RANGE);

	return array_status(self->array, self->idx) ?
	    array_at(self->array, self->idx) : NULL;
}

const void *array_iter_inc1(array_iter_t * self)
{
	return array_iter_inc2(self, 1);
}

const void *array_iter_inc2(array_iter_t * self, size_t inc)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_ITER_NULL);

	const void *ret = NULL;

	if (self->flags & AI_FLAG_BWD)
		ret = __array_iter_bwd(self);
	else
		ret = __array_iter_fwd(self);

	return ret;
}

const void *array_iter_dec1(array_iter_t * self)
{
	return array_iter_dec2(self, 1);
}

const void *array_iter_dec2(array_iter_t * self, size_t dec)
{
	if (unlikely(self == NULL))
		throw_unexpected(ARRAY_ITER_NULL);

	const void *ret = NULL;

	if (self->flags & AI_FLAG_BWD)
		ret = __array_iter_fwd(self);
	else
		ret = __array_iter_bwd(self);

	return ret;
}

/* ======================================================================= */
