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
#include <clib/type.h>
#include <clib/slab.h>

#include <clib/tree.h>
#include <clib/tree_iter.h>

struct data {
    tree_node_t node;
    int i;
    float f;
};
typedef struct data data_t;

int main (void) {
    slab_t s = INIT_SLAB;
    slab_init(&s, "my_slab", sizeof(data_t), 4096);

    tree_t t = INIT_TREE;
    tree_init(&t, default_compare);

    int i;
    for (i=0; i<25; i++) {
        data_t * d = (data_t *)slab_alloc(&s);

        printf("insert i[%d] --> %p\n", i, d);
        d->i = i;
        d->f = (float)i;        /* key */

        tree_node_init(&d->node, (const void *)(d->i));
        tree_insert(&t, &d->node);
    }

#if 1
    i = 6;
    tree_node_t * n = tree_find(&t, (const void *)(i));
    tree_remove(&t, n);
    n = tree_find(&t, (const void *)i);

    i = 2;
    tree_find(&t, (const void *)i);
    i = 8;
    tree_find(&t, (const void *)i);
#endif

    tree_dump(&t, stdout);

    data_t * d;

    tree_iter_t it;
    tree_iter_init(&it, &t, TI_FLAG_FWD);

    tree_for_each(&it, d, node) {
        printf("depth first (FWD) i[%d] f[%f]\n", d->i, d->f);
    }

//    tree_dump(&t, stdout);
    slab_delete(&s);

    return 0;
}

