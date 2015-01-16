/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/cunit/slab.c $                                           */
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
