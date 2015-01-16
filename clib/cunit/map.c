/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/cunit/map.c $                                            */
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
#include <string.h>

#include <clib/libclib.h>

#include <clib/slab.h>
#include <clib/map.h>
#include <clib/map_iter.h>

#include <CUnit/Basic.h>

#define COUNT 	20000
#define SEED    22

slab_t slab;

typedef struct {
    map_node_t node;
    int i;
    float f;
} data_t;

static int init_map(void) {
    slab_init(&slab, "my_slab", sizeof(data_t), 4096);
    return 0;
}

static int clean_map(void) {
    slab_delete(&slab);
    return 0;
}

static void __insert(map_t * t, int i) {
    data_t * d = (data_t *)slab_alloc(&slab);

    i %= INT32_MAX;

    d->i = i;
    d->f = (float)i;
    map_node_init(&d->node, (const void *)(intptr_t)(d->i));

    if (map_insert(t, &d->node) < 0) {
        err_t * err = err_get();
        fprintf(stderr, "%s(%d): %.*s\n",
                err_file(err), err_line(err), err_size(err),
		(const char *)err_data(err));
    }
}

static data_t * __remove(map_t * t, int i) {

    i %= INT32_MAX;

    map_node_t * n = map_find(t, (const void *)i);
    if (n == NULL) map_dump(t, stdout);
    CU_ASSERT_PTR_NOT_NULL_FATAL(n);

    map_remove(t, n);

    data_t * d = container_of(n, data_t, node);
    CU_ASSERT_PTR_NOT_NULL_FATAL(n);

    if (0 <= i)
        CU_ASSERT((int)d->node.key == i);

    return d;
}

static int compare(const void * v1, const void * v2) {
    const int i1 = (const int)v1, i2 = (const int)v2;
    return i1 - i2;
}

static void map_1(void) {
    map_t t;
    map_init(&t, compare);

    CU_ASSERT(map_min(&t) == NULL);
    CU_ASSERT(map_max(&t) == NULL);
    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);
}

static void map_2(void) {
    map_t t;
    map_init(&t, compare);

    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);

    for (int i=1; i<=COUNT; i++)
        __insert(&t, i);

    CU_ASSERT(map_empty(&t) == false);
    CU_ASSERT(map_size(&t) == COUNT);

    for (int i=1; i<=COUNT; i++) {
        CU_ASSERT(i == (int)map_min(&t)->key);
        CU_ASSERT(COUNT == (int)map_max(&t)->key);
        __remove(&t, (int)map_min(&t)->key);
        CU_ASSERT(map_size(&t) + i == COUNT);
    }

    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);

    for (int i=1; i<=COUNT; i++)
        __insert(&t, i);

    CU_ASSERT(map_empty(&t) == false);
    CU_ASSERT(map_size(&t) == COUNT);

    for (int i=1; i<=COUNT; i++) {
        CU_ASSERT(1 == (int)map_min(&t)->key);
        CU_ASSERT(COUNT - i + 1 == (int)map_max(&t)->key);
        __remove(&t, (int)map_max(&t)->key);
        CU_ASSERT(map_size(&t) + i == COUNT);
    }

    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);
}

static void map_3(void) {
    map_t t;
    map_init(&t, compare);

    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);

    for (int i=1; i<=COUNT; i++)
        __insert(&t, COUNT - i + 1);

    CU_ASSERT(map_empty(&t) == false);
    CU_ASSERT(map_size(&t) == COUNT);

    for (int i=1; i<=COUNT; i++) {
        CU_ASSERT(1 == (int)map_min(&t)->key);
        CU_ASSERT(COUNT - i + 1 == (int)map_max(&t)->key);
        __remove(&t, (int)map_max(&t)->key);
        CU_ASSERT(map_size(&t) + i == COUNT);
    }

    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);

    for (int i=1; i<=COUNT; i++)
        __insert(&t, COUNT - i + 1);

    CU_ASSERT(map_empty(&t) == false);
    CU_ASSERT(map_size(&t) == COUNT);

    for (int i=1; i<=COUNT; i++) {
        CU_ASSERT(i == (int)map_min(&t)->key);
        CU_ASSERT(COUNT == (int)map_max(&t)->key);
        __remove(&t, (int)map_min(&t)->key);
        CU_ASSERT(map_size(&t) + i == COUNT);
    }

    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);
}

static void map_4(void) {
    map_t t;
    map_init(&t, compare);

    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);

    srandom(SEED);
    for (int i=1; i<=COUNT; i++)
        __insert(&t, (int)random());

    CU_ASSERT(map_empty(&t) == false);
    CU_ASSERT(map_size(&t) == COUNT);

    for (int i=1; i<=COUNT; i++) {
         __remove(&t, (int)map_min(&t)->key);
        CU_ASSERT(map_size(&t) + i == COUNT);
    }

    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);

    srandom(SEED);
    for (int i=1; i<=COUNT; i++)
        __insert(&t, (int)random());

    CU_ASSERT(map_empty(&t) == false);
    CU_ASSERT(map_size(&t) == COUNT);

    for (int i=1; i<=COUNT; i++) {
         __remove(&t, (int)map_max(&t)->key);
        CU_ASSERT(map_size(&t) + i == COUNT);
    }

    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);

    srandom(SEED);
    for (int i=1; i<=COUNT; i++)
        __insert(&t, (int)random());

    CU_ASSERT(map_empty(&t) == false);
    CU_ASSERT(map_size(&t) == COUNT);

    srandom(SEED);
    for (int i=1; i<=COUNT; i++) {
         __remove(&t, (int)random());
        CU_ASSERT(map_size(&t) + i == COUNT);
    }

    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);
}

static void map_5(void) {
    map_t t;
    map_init(&t, compare);

    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);

    srandom(SEED);
    for (int i=1; i<=COUNT; i++)
        __insert(&t, (int)random());

    CU_ASSERT(map_empty(&t) == false);
    CU_ASSERT(map_size(&t) == COUNT);

    data_t * d;
    int key = 0;

    map_iter_t it;
    map_iter_init(&it, &t, MI_FLAG_FWD);
    map_for_each(&it, d, node) {
        CU_ASSERT(key < (int)d->node.key);
        key = (int)d->node.key;
    }

    key = INT32_MAX;
    map_iter_init(&it, &t, MI_FLAG_BWD);
    map_for_each(&it, d, node) {
        CU_ASSERT((int)d->node.key < key);
        key = (int)d->node.key;
    }
}

static void map_6(void) {
    map_t t;
    map_init(&t, compare);

    CU_ASSERT(map_empty(&t) == true);
    CU_ASSERT(map_size(&t) == 0);

    srandom(SEED);
    for (int i=1; i<=COUNT; i++)
        __insert(&t, (int)random());

    CU_ASSERT(map_empty(&t) == false);
    CU_ASSERT(map_size(&t) == COUNT);

    srandom(SEED);
    for (int i=1; i<=COUNT; i++) {
        map_node_t * node = map_find(&t, (const void *)random());
        CU_ASSERT_PTR_NOT_NULL_FATAL(node);
    }
}

void map_test(void) {
    CU_pSuite suite = CU_add_suite("map", init_map, clean_map);
    if (NULL == suite)
	return;

    if (CU_add_test(suite, "test of --> map_1", map_1) == NULL) return;
    if (CU_add_test(suite, "test of --> map_2", map_2) == NULL) return;
    if (CU_add_test(suite, "test of --> map_3", map_3) == NULL) return;
    if (CU_add_test(suite, "test of --> map_4", map_4) == NULL) return;
    if (CU_add_test(suite, "test of --> map_5", map_5) == NULL) return;
    if (CU_add_test(suite, "test of --> map_6", map_6) == NULL) return;
}
