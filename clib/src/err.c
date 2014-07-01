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
 *   File: err.c
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr: Error container
 *   Date: 04/04/12
 */

#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <execinfo.h>

#include "attribute.h"
#include "min.h"

#include "err.h"

static pthread_key_t __err_key = 0;

static const char *__err_type_name[] = {
	[ERR_NONE] = "none",
	[ERR_ERRNO] = "errno",
	[ERR_UNEXPECTED] = "unexpected",
	[ERR_VERSION] = "version",
};

/* =======================================================================*/

void err_delete(err_t * self)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	memset(self, 0, sizeof(*self));
	free(self);

	return;
}

err_t *err_get(void)
{
	list_t *list = (list_t *) pthread_getspecific(__err_key);

	err_t *self = NULL;

	if (list != NULL) {
		self = container_of(list_remove_tail(list), err_t, node);

		if (list_empty(list)) {
			free(list), list = NULL;
			assert(pthread_setspecific(__err_key, list) == 0);
		}
	}

	return self;
}

void err_put(err_t * self)
{
	assert(self != NULL);

	list_t *list = pthread_getspecific(__err_key);
	if (list == NULL) {
		list = (list_t *) malloc(sizeof(*list));
		assert(list != NULL);

		list_init(list);
		assert(pthread_setspecific(__err_key, list) == 0);
	}

	list_add_head(list, &self->node);

	return;
}

int err_type1(err_t * self)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	return self->type;
}

void err_type2(err_t * self, int type)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	self->type = type;
}

int err_code1(err_t * self)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	return self->code;
}

void err_code2(err_t * self, int code)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	self->code = code;

	return;
}

const char *err_file1(err_t * self)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	return self->file;
}

void err_file2(err_t * self, const char *file)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	self->file = file;

	return;
}

int err_line1(err_t * self)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	return self->line;
}

void err_line2(err_t * self, int line)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	self->line = line;

	return;
}

const void *err_data1(err_t * self)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	return self->data;
}

void err_data2(err_t * self, int size, const void *data)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	self->size = min(size, ERR_DATA_SIZE);
	memcpy(self->data, data, self->size);

	return;
}

int err_size(err_t * self)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	return self->size;
}

const char *err_type_name(err_t * self)
{
	assert(self != NULL);
	assert(self->magic == ERR_MAGIC);

	return __err_type_name[self->type];
}

/* =======================================================================*/

__constructor static void __err__ctor__(void)
{
	assert(pthread_key_create(&__err_key, NULL) == 0);
}
