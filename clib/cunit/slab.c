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

#include <clib/exception.h>
#include <clib/assert.h>
#include <clib/type.h>

#include <clib/slab.h>

#include <CUnit/Basic.h>

#define COUNT	10000
#define SEED    41

static int init_slab(void) {
    return 0;
}

static int clean_slab(void) {
    return 0;
}

static void slab_1(void) {
    slab_t s[SLAB_ALLOC_MAX+1] = {INIT_SLAB,};

    for (int i=SLAB_ALLOC_MIN; i<=SLAB_ALLOC_MAX; i+=3) {
        slab_init(&s[i], "my_slab", i);
//slab_dump(&s[i], stdout);

        for (int j=0; j<COUNT; j++) {
            void * ptr = slab_alloc(&s[i]);
            memset(ptr, j, i);
        }

        slab_delete(&s[i]);
    }
}

void slab_test(void) {
    CU_pSuite suite = CU_add_suite("slab", init_slab, clean_slab);
    if (NULL == suite)
	return;

    if (CU_add_test(suite, "test of --> slab_1", slab_1) == NULL) return;
}
