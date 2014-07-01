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

