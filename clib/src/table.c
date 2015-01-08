/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/src/table.c $                                            */
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
 *   File: table.c
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr:
 *   Note:
 *   Date: 08/21/10
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
#include "table.h"
#include "vector_iter.h"

#define TABLE_PAGE_SIZE         4096
#define TABLE_PAGE_DIVISOR      32

/* ======================================================================= */
int table_init(table_t * self, const char *name, size_t col_nr)
{
	assert(self != NULL);
	assert(name != NULL);

	if (col_nr == 0) {
		UNEXPECTED("'%d' invalid column number", col_nr);
		return -1;
	}

	self->hdr.id[IDENT_MAGIC_0] = TABLE_MAGIC[IDENT_MAGIC_0];
	self->hdr.id[IDENT_MAGIC_1] = TABLE_MAGIC[IDENT_MAGIC_1];
	self->hdr.id[IDENT_MAGIC_2] = TABLE_MAGIC[IDENT_MAGIC_2];
	self->hdr.id[IDENT_MAGIC_3] = TABLE_MAGIC[IDENT_MAGIC_3];

	self->hdr.id[IDENT_MAJOR] = CLIB_MAJOR;
	self->hdr.id[IDENT_MINOR] = CLIB_MINOR;
	self->hdr.id[IDENT_PATCH] = CLIB_PATCH;

	if (__BYTE_ORDER == __LITTLE_ENDIAN)
		self->hdr.id[IDENT_FLAGS] |= TABLE_FLAG_LSB;
	if (__BYTE_ORDER == __BIG_ENDIAN)
		self->hdr.id[IDENT_FLAGS] |= TABLE_FLAG_MSB;

	/* FIX ME -- handle this more elegantly */
	//assert((col_nr * sizeof(value_t)) < TABLE_PAGE_SIZE);

	self->hdr.col_nr = col_nr;

	if (name == NULL || *name == '\0')
		memset(self->hdr.name, 0, sizeof(self->hdr.name));
	else
		strncpy(self->hdr.name, name, sizeof(self->hdr.name));

	char name_vector[strlen(self->hdr.name) + 5];

	size_t row_size = self->hdr.col_nr * sizeof(value_t);

	sprintf(name_vector, "%s.table", self->hdr.name);
	vector_init(&self->table, name_vector, row_size,
		    __round_pow2(row_size * TABLE_PAGE_DIVISOR));

	sprintf(name_vector, "%s.string", self->hdr.name);
	vector_init(&self->string, name_vector, 1, TABLE_PAGE_SIZE);

	sprintf(name_vector, "%s.blob", self->hdr.name);
	vector_init(&self->blob, name_vector, 1, TABLE_PAGE_SIZE);

	return 0;
}

int table_delete(table_t * self)
{
	if (unlikely(self == NULL))
		return 0;

	if (MAGIC_CHECK(self->hdr.id, TABLE_MAGIC)) {
		UNEXPECTED("'%2.2x%2.2x%2.2x%2.2x' invalid table magic "
			   "'%2.2x%2.2x%2.2x%2.2x",
			   self->hdr.id[IDENT_MAGIC_0],
			   self->hdr.id[IDENT_MAGIC_1],
			   self->hdr.id[IDENT_MAGIC_2],
			   self->hdr.id[IDENT_MAGIC_3],
			   TABLE_MAGIC[IDENT_MAGIC_0],
			   TABLE_MAGIC[IDENT_MAGIC_2],
			   TABLE_MAGIC[IDENT_MAGIC_3],
			   TABLE_MAGIC[IDENT_MAGIC_3]);
		return -1;
	}

	if (0 < vector_size(&self->table)) {
		vector_iter_t it;
		if (vector_iter_init(&it, &self->table, VI_FLAG_FWD) < 0)
			return -1;

		value_t *v;
		vector_for_each(&it, v)
		    for (size_t c = 0; c < self->hdr.col_nr; c++)
			value_clear(v + c);
	}

	if (vector_delete(&self->table) < 0)
		return -1;
	if (vector_delete(&self->string) < 0)
		return -1;
	if (vector_delete(&self->blob) < 0)
		return -1;

	return 0;
}

value_t *table_get(table_t * self, size_t row, size_t col)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));

	row++;			/* hide the column names */

	return (value_t *) vector_at(&self->table,
				     row * self->hdr.col_nr + col);
}

value_t *table_row2(table_t * self, size_t row_nr)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));

	row_nr++;		/* hide the column names */

	return (value_t *) vector_at(&self->table, row_nr * self->hdr.col_nr);
}

int table_row3(table_t * self, size_t row_nr, value_t * row)
{
	assert(self != NULL);
	assert(row != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));

	row_nr++;		/* hide the column names */

	if (vector_size(&self->table) <= row_nr + 1) {
		if (vector_size(&self->table, row_nr + 1) < 0)
			return -1;
	} else {
		value_t *old;
		old = (value_t *) vector_at(&self->table, row_nr);
		assert(old != NULL);

		for (size_t col_nr = 0; col_nr < self->hdr.col_nr; col_nr++)
			value_clear(old + col_nr);
	}

	if (vector_put(&self->table, row_nr, row) < 0)
		return -1;

	for (size_t col_nr = 0; col_nr < self->hdr.col_nr; col_nr++)
		if (value_type(row + col_nr) == VT_STR ||
		    value_type(row + col_nr) == VT_BLOB)
			value_type(row + col_nr, VT_UNKNOWN);

	return 0;
}

value_t *table_column(table_t * self, value_t * row, size_t col)
{
	assert(self != NULL);
	assert(row != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));
	assert(col < self->hdr.col_nr);

	return (void *)row + (vector_elem_size(&self->table) * col);
}

int table_put(table_t * self, size_t row, size_t col, value_t * val)
{
	assert(self != NULL);
	assert(val != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));

	row++;			/* hide the column names */

	size_t size = (row + 1) * self->hdr.col_nr;

	if (vector_size(&self->table) <= size)
		if (vector_size(&self->table, size) < 0)
			return -1;

	/* free existing pointer data */
	value_t *old = (value_t *) vector_at(&self->table,
					     row * self->hdr.col_nr + col);
	assert(old != NULL);
	value_clear(old);

	vector_put(&self->table, row * self->hdr.col_nr + col, (void *)val, 1);

	if ((value_type(val) == VT_STR) || (value_type(val) == VT_BLOB))
		value_type(val, VT_UNKNOWN);

	return 0;
}

const char *table_name2(table_t * self, size_t col_nr)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));
	assert(col_nr < self->hdr.col_nr);

	const char *rc = NULL;

	value_t *row = (value_t *) vector_at(&self->table, 0);
	if (row != NULL) {
		value_t *col = row + col_nr;
		switch (value_type(col)) {
		case VT_STR_OFF:
			rc = vector_at(&self->string, col->u64);
			break;
		case VT_STR_INLINE:
			rc = (const char *)col->data;
			break;
		case VT_STR_CONST:
		case VT_STR:
			rc = value_string(col);
			break;
		default:
			rc = NULL;
		}
	}

	return rc;
}

int _table_name3(table_t * self, size_t col_nr, const char *name)
{
	return _table_name4(self, col_nr, name, strlen(name));
}

int _table_name4(table_t * self, size_t col_nr, const char *name, size_t len)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));
	assert(col_nr < self->hdr.col_nr);

	if (vector_size(&self->table) <= 0)
		if (vector_size(&self->table, 1) < 0)
			return -1;

	value_t *row = (value_t *) vector_at(&self->table, 0);
	if (row != NULL)
		value_string(row + col_nr, name, len);

	return 0;
}

size_t table_rows(table_t * self)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));
	return vector_size(&self->table) - 1;
}

size_t table_columns(table_t * self)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));
	return self->hdr.col_nr;
}

int table_serialize(table_t * self)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));

	vector_iter_t it;
	vector_iter_init(&it, &self->table, VI_FLAG_FWD);

	value_t *row;
	vector_for_each(&it, row) {
		for (size_t c = 0; c < self->hdr.col_nr; c++) {
			value_t *col = row + c;

			value_type_t type = value_type(col);
			size_t len = value_size(col);

			vector_t *vec = NULL;
			if (type == VT_STR)
				vec = &self->string, type = VT_STR_OFF, len++;
			else if (type == VT_BLOB)
				vec = &self->blob, type = VT_BLOB_OFF;
			else
				continue;

			uint64_t off = vector_size(vec);

			if (vector_size(vec, off + len) < 0)
				return -1;
			if (vector_put(vec, off, col->ptr, len) < 0)
				return -1;

			free(col->ptr), col->ptr = NULL;

			col->u64 = off;
			value_type(col, type);
		}
	}

	return 0;
}

ssize_t table_save(table_t * self, FILE * out)
{
	assert(self != NULL);
	assert(out != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));

	/* ============= */

	int header_swap(table_header_t * hdr) {
		assert(hdr != NULL);

		if (hdr->id[IDENT_FLAGS] & TABLE_FLAG_MSB) {
			hdr->col_nr = htobe32(hdr->col_nr);
		} else if (hdr->id[IDENT_FLAGS] & TABLE_FLAG_LSB) {
			hdr->col_nr = htole32(hdr->col_nr);
		} else {
			UNEXPECTED("'%s' invalid or corrupt table object => "
				   "'%x'", hdr->name, hdr->id[IDENT_FLAGS]);
			return -1;
		}

		return 0;
	}

	/* ============= */

	ssize_t len = 0;

	table_header_t hdr = self->hdr;
	if (header_swap(&hdr) < 0)
		return -1;

	clearerr(out);
	len = fwrite(&self->hdr, 1, sizeof(self->hdr), out);
	if (len != sizeof(self->hdr)) {
		if (ferror(out)) {
			ERRNO(errno);
			return -1;
		}
	}

	ssize_t rc = vector_save(&self->table, out);
	if (rc < 0)
		return -1;
	len += rc;

	rc = vector_save(&self->string, out);
	if (rc < 0)
		return -1;
	len += rc;

	rc = vector_save(&self->blob, out);
	if (rc < 0)
		return -1;
	len += rc;

	return len;
}

int table_deserialize(table_t * self)
{
	assert(self != NULL);
	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));

	vector_iter_t it;
	vector_iter_init(&it, &self->table, VI_FLAG_FWD);

	value_t *row;
	vector_for_each(&it, row) {
		for (size_t c = 0; c < self->hdr.col_nr; c++) {
			value_t *col = row + c;

			value_type_t type = value_type(col);
			size_t len = value_size(col);

			switch (type) {
				uint64_t off;
			case VT_STR_OFF:
				len++;
				value_type(col, VT_STR);
			case VT_BLOB_OFF:
				off = col->u64;
				col->ptr = malloc(len);

				if (col->ptr == NULL) {
					ERRNO(errno);
					return -1;
				}

				if (vector_get(&self->string, off, col->ptr,
					       len) < 0)
					return -1;

				if (value_type(col) == VT_BLOB_OFF)
					value_type(col, VT_BLOB);
				break;
			case VT_STR:
			case VT_BLOB:
			case VT_STR_CONST:
				UNEXPECTED("'%s' invalid or corrupt type %d",
					   self->hdr.name, type);
				return -1;
			default:
				;
			}
		}
	}

	if (vector_delete(&self->string) < 0)
		return -1;
	if (vector_delete(&self->blob) < 0)
		return -1;

	return 0;
}

ssize_t table_load(table_t * self, FILE * in)
{
	assert(self != NULL);

	/* ============= */

	int header_swap(table_header_t * hdr) {
		assert(hdr != NULL);
		if (hdr->id[IDENT_FLAGS] & TABLE_FLAG_MSB) {
			hdr->col_nr = be32toh(hdr->col_nr);
		} else if (hdr->id[IDENT_FLAGS] & TABLE_FLAG_LSB) {
			hdr->col_nr = le32toh(hdr->col_nr);
		} else {
			UNEXPECTED("'%s' invalid or corrupt table object => "
				   "'%x'", hdr->name, hdr->id[IDENT_FLAGS]);
			return -1;
		}

		return 0;
	}

	int table_swap(table_t * tbl) {
		vector_iter_t it;
		if (vector_iter_init(&it, &tbl->table, VI_FLAG_FWD) < 0)
			return -1;

		value_t *row;
		vector_for_each(&it, row) {
			for (size_t c = 0; c < self->hdr.col_nr; c++) {
				value_t *col = row + c;

				value_type_t type = value_type(row);

#define iBE(s) value_i##s(col, be##s##toh(value_i##s(col)))
#define uBE(s) value_u##s(col, be##s##toh(value_u##s(col)))
#define iLE(s) value_i##s(col, le##s##toh(value_i##s(col)))
#define uLE(s) value_u##s(col, le##s##toh(value_u##s(col)))

				if (tbl->hdr.id[IDENT_FLAGS] & TABLE_FLAG_MSB) {
					switch (type) {
					case VT_I16:
						iBE(16);
						break;
					case VT_U16:
						uBE(16);
						break;
					case VT_I32:
						iBE(32);
						break;
					case VT_U32:
						uBE(32);
						break;
					case VT_I64:
						iBE(64);
						break;
					case VT_U64:
						uBE(64);
						break;
					case VT_STR_OFF:
					case VT_BLOB_OFF:
						col->u64 = be64toh(col->u64);
						break;
					default:
						;
					}
				} else if (tbl->hdr.id[IDENT_FLAGS] &
					   TABLE_FLAG_LSB) {
					switch (type) {
					case VT_I16:
						iLE(16);
						break;
					case VT_U16:
						uLE(16);
						break;
					case VT_I32:
						iLE(32);
						break;
					case VT_U32:
						uLE(32);
						break;
					case VT_I64:
						iLE(64);
						break;
					case VT_U64:
						uLE(64);
						break;
					case VT_STR_OFF:
					case VT_BLOB_OFF:
						col->u64 = le64toh(col->u64);
						break;
					default:
						;
					}
				} else {
					UNEXPECTED("'%s' invalid or corrupt "
						   "table object => '%x'",
						   tbl->hdr.name,
						   tbl->hdr.id[IDENT_FLAGS]);
					return -1;
				}
			}
		}

		return 0;
	}

	/* ============= */

	// zero'd table will cause a magic check
	(void)table_delete(self);

	clearerr(in);
	size_t len = fread(&self->hdr, 1, sizeof(self->hdr), in);
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

	assert(!MAGIC_CHECK(self->hdr.id, TABLE_MAGIC));

	if (header_swap(&self->hdr) < 0)
		return -1;

	ssize_t rc = vector_load(&self->table, in);
	if (rc < 0)
		return -1;
	len += rc;

	if (table_swap(self) < 0)
		return -1;

	rc = vector_load(&self->string, in);
	if (rc < 0)
		return -1;
	len += rc;

	rc = vector_load(&self->blob, in);
	if (rc < 0)
		return -1;
	len += rc;

	return len;
}

void table_print(table_t * self, FILE * out)
{
	if (self != NULL) {
		if (unlikely(MAGIC_CHECK(self->hdr.id, TABLE_MAGIC))) {
			UNEXPECTED("'%s' invalid or corrupt table object",
				   self->hdr.name);
			return;
		}

		vector_iter_t it;
		vector_iter_init(&it, &self->table, VI_FLAG_FWD);

		value_t *row;
		vector_for_each(&it, row)
		    for (size_t c = 0; c < self->hdr.col_nr; c++)
			value_dump(row + c, out);

		vector_dump(&self->string, out);
	}
}

void table_dump(table_t * self, FILE * out)
{
	if (self != NULL) {
		if (unlikely(MAGIC_CHECK(self->hdr.id, TABLE_MAGIC))) {
			UNEXPECTED("'%s' invalid or corrupt table object",
				   self->hdr.name);
			return;
		}

		fprintf(out,
			"table: [ size: %d cols: %d rows: %d  name: '%s']\n",
			sizeof(value_t), table_columns(self), table_rows(self),
			self->hdr.name);

		dump_memory(out, (unsigned long)&self->hdr, &self->hdr,
			    sizeof(self->hdr));

		vector_dump(&self->table, out);
		vector_dump(&self->string, out);
		vector_dump(&self->blob, out);
	}
}

#if 0
void table_sort(table_t * self, compare_f cmp)
{
	/* The exchange function swaps two rows within the table
	 * exchange(table_t * self, size_t i, size_t j)
	 */

	/* The partition method receives a list or sublist, and places the first element
	 * in its correct position within the list. It also ensures that all elements to
	 * the left of this are smaller, and all to the right are larger.
	 *
	 * partition(a[], p, r)
	 *         i = p
	 *         j = r + 1
	 *         pivot = a[p]
	 *         do {
	 *             do i = i + 1 while (a[i]<pivot)
	 *             do j = j - 1 while (a[j]>pivot)
	 *             if (i < j) exchange(a[i], a[j])
	 *         } while (i<j)
	 *         exchange(a[p], a[j])
	 *         return j
	 */

	/*
	 * quicksort(a[], p, r)
	 * if r > p then
	 *     j = partition(a[], p, r)
	 *     quicksort(a[], p, j-1)
	 *     quicksort(a[], j+1, r)
	 *
	 */
}
#endif

/* ======================================================================= */
