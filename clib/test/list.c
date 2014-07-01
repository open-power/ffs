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
#include <clib/list.h>
#include <clib/list_iter.h>

struct data {
    list_node_t node;
    int i;
    float f;
};
typedef struct data data_t;

int main (void) {
//    slab_t s = INIT_SLAB;
//    slab_init(&s, "my_slab", sizeof(data_t), 0);

    list_t l = INIT_LIST;
    list_init(&l);

    int i;
    for (i=0; i<10; i++) {
//      data_t * d = (data_t *)slab_alloc(&s);
	data_t * d = (data_t *)malloc(sizeof(*d));

        d->i = i;
        d->f = (float)i;

        list_add_tail(&l, &d->node);
    }

    list_iter_t it;
    list_iter_init(&it, &l, LI_FLAG_FWD);

    data_t * d;
    list_for_each(&it, d, node) {
        printf("i: %d f: %f\n", d->i, d->f);
    }

    while (list_empty(&l) == false) {
	data_t * d = container_of(list_remove_tail(&l), data_t, node);
        printf("i: %d f: %f\n", d->i, d->f);
    }

//    slab_dump(&s, stdout);
//    slab_delete(&s);

    return 0;
}

