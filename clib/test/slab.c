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

#include <clib/slab.h>

#define SIZE 	6500
#define ALLOC	16
#define PAGE	4096

int main(void) {
    slab_t s = INIT_SLAB;
    slab_init(&s, "my_slab", ALLOC, PAGE);

    void * ptr[SIZE] = {NULL,};

    int i;
    for (i=0; i<SIZE; i++) {
        ptr[i] = slab_alloc(&s);
        memset(ptr[i], i % 256, ALLOC);
    }

    slab_dump(&s, stdout);

    for (i=0; i<SIZE; i++) {
        slab_free(&s, ptr[i]);
    }

    slab_dump(&s, stdout);
    slab_delete(&s);

    return 0;
}
