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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clib/libclib.h>

#include <clib/crc32.h>
#include <clib/vector.h>

#include <CUnit/Basic.h>

#define COUNT	10000
#define SEED    41

static int init_vector(void) {
    return 0;
}

static int clean_vector(void) {
    return 0;
}

static void vector_1(void) {
    vector_t v;

    CU_ASSERT(vector_init(&v, "my_vector", 0) == -1)
    err_t * err = err_get();
    fprintf(stderr, "%s(%d): %.*s\n",
            err_file(err), err_line(err), err_size(err),
            (const char *)err_data(err));

    for (size_t i=VECTOR_ELEM_MIN; i<=VECTOR_ELEM_MAX; i++) {
        CU_ASSERT(vector_init(&v, "my_vector", i) == 0);

        CU_ASSERT(vector_size(&v) == 0);
        CU_ASSERT(vector_pages(&v) == 0);
        CU_ASSERT(vector_capacity(&v) == 0);
        CU_ASSERT(vector_elem_size(&v) == i);
        CU_ASSERT(vector_elem_size(&v) * v.hdr.elem_num <= v.hdr.page_size);

        CU_ASSERT(vector_delete(&v) == 0);
    }
}

uint32_t crc2;

static void vector_2(void) {
    vector_t v;

    CU_ASSERT(vector_init(&v, "my_vector.bin", 1) == 0);
    CU_ASSERT(vector_size(&v) == 0);
    CU_ASSERT(vector_pages(&v) == 0);
    CU_ASSERT(vector_capacity(&v) == 0);
    CU_ASSERT(vector_elem_size(&v) == 1);

    CU_ASSERT(vector_size(&v, COUNT) == COUNT);
    CU_ASSERT(COUNT <= vector_capacity(&v));
    CU_ASSERT(COUNT <= vector_size(&v));
    CU_ASSERT(3 <= vector_pages(&v));

    for (int i=0; i<COUNT; i++) {
        unsigned char c = (unsigned char)i;
        crc2 = clib_crc32(c, crc2);
        CU_ASSERT(vector_put(&v, i, &c) == 0);
    }
    CU_ASSERT_FATAL(crc2 != 0);

    FILE * f = fopen(v.hdr.name, "w");
    CU_ASSERT_FATAL(f != NULL);
    CU_ASSERT(COUNT < vector_save(&v, f));
    fclose(f), f = NULL;

    CU_ASSERT(vector_delete(&v) == 0);
}

uint32_t crc3;

static void vector_3(void) {
    vector_t v;
    vector_init(&v, __func__, 1);

    CU_ASSERT(vector_size(&v) == 0);
    CU_ASSERT(vector_pages(&v) == 0);
    CU_ASSERT(vector_capacity(&v) == 0);
    CU_ASSERT(vector_elem_size(&v) == 1);

    FILE * f = fopen("my_vector.bin", "r");
    CU_ASSERT_FATAL(f != NULL);
    CU_ASSERT(COUNT < vector_load(&v, f));
    fclose(f), f = NULL;

    crc3 = 0;

    for (int i=0; i<COUNT; i++) {
        unsigned char c;
        vector_get(&v, i, &c);
        crc3 = clib_crc32(c, crc3);
    }
    CU_ASSERT_FATAL(crc3 != 0);

    CU_ASSERT_FATAL(crc2 == crc3);

    vector_delete(&v);
}

static void vector_4(void) {
    vector_t v;

    if (vector_init(&v, NULL, 0) < 0) {
        err_t * err = err_get();
        fprintf(stderr, "%s(%d): UNEXPECTED: %.*s\n",
                err_file(err), err_line(err), err_size(err),
		(const char *)err_data(err));
    }

    if (vector_init(&v, "my_vector", 0) < 0) {
	err_t * err = err_get();
        fprintf(stderr, "%s(%d): UNEXPECTED: %.*s\n",
                err_file(err), err_line(err), err_size(err),
		(const char *)err_data(err));
    }

    for (size_t i=VECTOR_ELEM_MIN; i<=VECTOR_ELEM_MAX; i+=3) {
        vector_init(&v, "my_vector", i);

        CU_ASSERT(vector_size(&v) == 0);
        CU_ASSERT(vector_pages(&v) == 0);
        CU_ASSERT(vector_capacity(&v) == 0);
        CU_ASSERT(vector_elem_size(&v) == i);
        CU_ASSERT(vector_elem_size(&v) * v.hdr.elem_num <= v.hdr.page_size);

        vector_size(&v, COUNT);

        CU_ASSERT(vector_size(&v) == COUNT);
        CU_ASSERT(vector_size(&v) <= vector_capacity(&v));
        CU_ASSERT(0 < vector_pages(&v));

        vector_delete(&v);
    }
}

void vector_test(void) {
    CU_pSuite suite = CU_add_suite("vector", init_vector, clean_vector);
    if (NULL == suite)
	return;

    if (CU_add_test(suite, "vector_1: vector_init()", vector_1) == NULL) return;
    if (CU_add_test(suite, "vector_2: vector_save()", vector_2) == NULL) return;
    if (CU_add_test(suite, "vector_3: vector_load()", vector_3) == NULL) return;
    if (CU_add_test(suite, "vector_4: vector_size()", vector_4) == NULL) return;
}
