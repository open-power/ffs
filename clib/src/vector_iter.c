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
 *   File: vector_iter.c
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

#include "libclib.h"
#include "vector_iter.h"

/* ======================================================================= */

static inline const void *__vector_iter_bwd(vector_iter_t * self)
{
	size_t low = 0;
	const void *ret = NULL;

	if (low < self->idx)
		self->idx--;

	if (low < self->idx)
		ret = vector_at(self->vector, self->idx);

	return ret;
}

static inline const void *__vector_iter_fwd(vector_iter_t * self)
{
	size_t high = vector_size(self->vector);
	const void *ret = NULL;

	if (self->idx < high)
		self->idx++;

	if (self->idx < high)
		ret = vector_at(self->vector, self->idx);

	return ret;
}

int vector_iter_init(vector_iter_t * self, vector_t * vector, uint32_t flags)
{
	assert(self != NULL);
	assert(vector != NULL);

	self->flags = flags;
	self->vector = vector;

	if (self->flags & VI_FLAG_BWD) {
		self->idx = vector_size(self->vector);
		__vector_iter_bwd(self);
	} else {
		self->idx = 0;
	}

	return 0;
}

int vector_iter_clear(vector_iter_t * self)
{
	assert(self != NULL);

	if (self->flags & VI_FLAG_BWD)
		self->idx = vector_size(self->vector);
	else
		self->idx = 0;

	self->vector = NULL;
	return 0;
}

const void *vector_iter_elem(vector_iter_t * self)
{
	assert(self != NULL);

	if (vector_capacity(self->vector) <= self->idx) {
		UNEXPECTED("'%d' index out-of-range", self->idx);
		return NULL;
	}

	if (vector_size(self->vector) <= self->idx)
		return NULL;

	return vector_at(self->vector, self->idx);
}

const void *vector_iter_inc1(vector_iter_t * self)
{
	return vector_iter_inc2(self, 1);
}

const void *vector_iter_inc2(vector_iter_t * self, size_t count)
{
	assert(self != NULL);

	const void *ret = NULL;

	for (size_t i = 0; i < count; i++) {
		if (self->flags & VI_FLAG_BWD)
			ret = __vector_iter_bwd(self);
		else
			ret = __vector_iter_fwd(self);
	}

	return ret;
}

const void *vector_iter_dec1(vector_iter_t * self)
{
	return vector_iter_dec2(self, 1);
}

const void *vector_iter_dec2(vector_iter_t * self, size_t count)
{
	assert(self != NULL);

	const void *ret = NULL;

	for (size_t i = 0; i < count; i++) {
		if (self->flags & VI_FLAG_BWD)
			ret = __vector_iter_fwd(self);
		else
			ret = __vector_iter_bwd(self);
	}

	return ret;
}

size_t vector_iter_pos1(vector_iter_t * self)
{
	assert(self != NULL);
	return self->idx;
}

int vector_iter_pos2(vector_iter_t * self, size_t pos)
{
	assert(self != NULL);

	if (vector_size(self->vector) <= pos) {
		UNEXPECTED("'%d' index out-of-range", pos);
		return -1;
	}

	self->idx = pos;
	return 0;
}

/* ======================================================================= */
