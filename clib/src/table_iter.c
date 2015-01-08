/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/src/table_iter.c $                                       */
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
 *   File: table_iter.c
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr:
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

#include "table_iter.h"

/* ======================================================================= */

int table_iter_init(table_iter_t * self, table_t * table, uint32_t flags)
{
	assert(self != NULL);
	assert(table != NULL);

	self->flags = flags;
	self->table = table;

	if (self->flags & TI_FLAG_BWD) {
		if (vector_iter_init(&self->it, &table->table, VI_FLAG_BWD) < 0)
			return -1;
	} else {
		if (vector_iter_init(&self->it, &table->table, VI_FLAG_FWD) < 0)
			return -1;
	}

	return vector_iter_inc(&self->it) != NULL;
}

int table_iter_clear(table_iter_t * self)
{
	assert(self != NULL);

	if (vector_iter_clear(&self->it) < 0)
		return -1;

	self->table = NULL;

	return 0;
}

const value_t *table_iter_elem(table_iter_t * self)
{
	assert(self != NULL);
	return (value_t *) vector_iter_elem(&self->it);
}

const value_t *table_iter_inc1(table_iter_t * self)
{
	return table_iter_inc2(self, 1);
}

const value_t *table_iter_inc2(table_iter_t * self, size_t count)
{
	assert(self != NULL);

	const value_t *v = NULL;

	for (size_t i = 0; i < count; i++) {
		if (self->flags & TI_FLAG_BWD) {
			v = (value_t *) vector_iter_dec(&self->it);	// columns
		} else {
			v = (value_t *) vector_iter_inc(&self->it);	// columns
		}
	}

	return v;
}

const value_t *table_iter_dec1(table_iter_t * self)
{
	return table_iter_dec2(self, 1);
}

const value_t *table_iter_dec2(table_iter_t * self, size_t count)
{
	assert(self != NULL);

	const value_t *v = NULL;

	for (size_t i = 0; i < count; i++) {
		if (self->flags & TI_FLAG_BWD) {
			v = (value_t *) vector_iter_inc(&self->it);	// columns
		} else {
			v = (value_t *) vector_iter_dec(&self->it);	// columns
		}
	}

	return v;
}

size_t table_iter_pos1(table_iter_t * self)
{
	assert(self != NULL);
	return vector_iter_pos(&self->it) + 1;
}

int table_iter_pos2(table_iter_t * self, size_t pos)
{
	assert(self != NULL);
	assert(pos < table_rows(self->table));
	return vector_iter_pos(&self->it, pos + 1);
}

/* ======================================================================= */
