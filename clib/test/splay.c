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

