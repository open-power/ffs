/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/src/vector_iter.c $                                      */
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
		UNEXPECTED("'%ld' index out-of-range", pos);
		return -1;
	}

	self->idx = pos;
	return 0;
}

/* ======================================================================= */
