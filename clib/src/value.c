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
 *   File: value.c
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr:
 *   Note:
 *   Date: 08/23/10
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
#include "value.h"

/* ======================================================================= */

void value_dump(const value_t * self, FILE * out)
{
	assert(self != NULL);

	static const char *type_name[] = {
		"<unknown>",
		"int8_t", "int16_t", "int32_t", "int64_t",
		"uint8_t", "uint16_t", "uint32_t", "uint64_t",
		"float", "double",
		"string", "string-inline", "string-offset", "string-const",
		"blob", "blob-inline", "blob-offset", "blob-file",
	};

	switch (self->type) {
	case VT_I8:
		fprintf(out, "value: [ data: %d size: %d type: %s ] %p\n",
			self->i8, self->size, type_name[self->type], self);
		break;
	case VT_I16:
		fprintf(out, "value: [ data: %d size: %d type: %s ] %p\n",
			self->i16, self->size, type_name[self->type], self);
		break;
	case VT_I32:
		fprintf(out, "value: [ data: %d size: %d type: %s ] %p\n",
			self->i32, self->size, type_name[self->type], self);
		break;
	case VT_I64:
		fprintf(out, "value: [ data: %lld size: %d type: %s ] %p\n",
			self->i64, self->size, type_name[self->type], self);
		break;
	case VT_U8:
		fprintf(out, "value: [ data: %ud size: %d type: %s ] %p\n",
			self->u8, self->size, type_name[self->type], self);
		break;
	case VT_U16:
		fprintf(out, "value: [ data: %d size: %d type: %s ] %p\n",
			self->u16, self->size, type_name[self->type], self);
		break;
	case VT_U32:
		fprintf(out, "value: [ data: %d size: %d type: %s ] %p\n",
			self->u32, self->size, type_name[self->type], self);
		break;
	case VT_U64:
		fprintf(out, "value: [ data: %lld size: %d type: %s ] %p\n",
			self->u64, self->size, type_name[self->type], self);
		break;
	case VT_REAL32:
		fprintf(out, "value: [ data: %f size: %d type: %s ] %p\n",
			self->r32, self->size, type_name[self->type], self);
		break;
	case VT_REAL64:
		fprintf(out, "value: [ data: %g size: %d type: %s ] %p\n",
			self->r64, self->size, type_name[self->type], self);
		break;
	case VT_STR:
		fprintf(out, "value: [ data: '%s' size: %d type: %s ] %p\n",
			(char *)self->ptr, self->size, type_name[self->type],
			self);
		break;
	case VT_STR_INLINE:
		fprintf(out, "value: [ data: '%s' size: %d type: %s ] %p\n",
			(char *)self->data, self->size, type_name[self->type],
			self);
		break;
	case VT_STR_OFF:
		fprintf(out, "value: [ data: %llu size: %d type: %s ] %p\n",
			self->u64, self->size, type_name[self->type], self);
		break;
	case VT_STR_CONST:
		fprintf(out, "value: [ data: '%s' size: %d type: %s ] %p\n",
			(const char *)self->ptr, self->size,
			type_name[self->type], self);
		break;
	case VT_BLOB:
		fprintf(out, "value: [ size: %d type: %s ] %p\n",
			self->size, type_name[self->type], self);
		dump_memory(out, (unsigned long)self->ptr, self->ptr,
			    self->size);
		break;
	case VT_BLOB_INLINE:
		fprintf(out, "value: [ size: %d type: %s ] %p\n",
			self->size, type_name[self->type], self);
		dump_memory(out, (unsigned long)self->data, self->data,
			    self->size);
		break;
	case VT_BLOB_OFF:
		fprintf(out, "value: [ data: %llu size: %d type: %s ] %p\n",
			self->u64, self->size, type_name[self->type], self);
		break;
	case VT_BLOB_FILE:
		fprintf(out, "value: [ data: '%s' size: %d type: %s ] %p\n",
			(char *)self->data, self->size, type_name[self->type],
			self);
		break;
	default:
		// throw_unexpected(VALUE_TYPE);
		fprintf(out, "value: [ size: %d type: %s ] %p\n",
			self->size, type_name[self->type], self);
	}
}

/* ======================================================================= */
