/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/test/splay.c $                                           */
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

#include <clib/assert.h>
#include <clib/slab.h>
#include <clib/type.h>
#include <clib/tree.h>

struct data {
    tree_node_t node;
    int i;
    float f;
};
typedef struct data data_t;

static inline int hash(int key) {
    key = ~key + (key << 15);
    key =  key ^ (key >> 12);
    key =  key + (key << 2);
    key =  key ^ (key >> 4);
    key =  key * 2057;
    key =  key ^ (key >> 16);

    return key;
}

int main (void) {
    slab_t s = INIT_SLAB;
    slab_init(&s, "my_slab", sizeof(data_t), 4096);

    int compare(const int i1, const int i2) {
        return i1 - i2;
    }

    tree_t l = INIT_TREE;
    tree_init(&l, (compare_f)compare);

    int i;
    for (i=0; i<10000; i++) {
        data_t * d = (data_t *)slab_alloc(&s);

        d->i = hash(i);
        d->f = (float)i;

        tree_node_init(&d->node, (const void *)d->i);
        splay_insert(&l, &d->node);
    }

    tree_dump(&l, stdout);

    i = hash(6);
    tree_node_t * n = tree_find(&l, (const void *)i);
    printf("n[%p]\n", n);

    data_t * d = container_of(n, data_t, node);
    printf("d->i[%d]\n", d->i);

    splay_remove(&l, n);
    n = tree_find(&l, (const void *)i);
    printf("n[%p]\n", n);

    i = 2;
    splay_find(&l, (const void *)i);
    i = 8;
    splay_find(&l, (const void *)i);

#if 0
    slab_dump(&s, stdout);
    tree_delete(&l, d, node);
#endif

    tree_dump(&l, stdout);
    slab_delete(&s);

    return 0;
}

