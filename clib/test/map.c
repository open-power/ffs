/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/test/map.c $                                             */
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
#include <clib/map.h>

struct data {
    map_node_t node;
    int i;
    float f;
};
typedef struct data data_t;

int main (void) {
    slab_t s = INIT_SLAB;
    slab_init(&s, "my_slab", sizeof(data_t), 0);

    int compare(const int i1, const int i2) {
        return i1 - i2;
    }

    map_t m;
    map_init(&m, (compare_f)compare);

    int i;
    for (i=0; i<100; i++) {
        data_t * d = (data_t *)slab_alloc(&s);

        printf("i[%d]\n", i);

        d->i = i;
        d->f = (float)i;

        map_node_init(&d->node, (const void *)(d->i));
        map_insert(&m, &d->node);
    }

#if 1
    i = 6;
    map_node_t * n = map_find(&m, (const void *)(i));
    map_remove(&m, n);
    n = map_find(&m, (const void *)i);

    i = 2;
    map_find(&m, (const void *)i);
    i = 8;
    map_find(&m, (const void *)i);
#endif

    map_dump(&m, stdout);
    slab_delete(&s);

    return 0;
}

