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

#include <clib/array.h>
#include <clib/array_iter.h>

#define SIZE 2550

int main(const int argc, const char * argv[]) {
    array_t a;
    array_init(&a, 4, 1024);

printf("size[%d]\n", array_size(&a));
printf("pages[%d]\n", array_pages(&a));
printf("capacity[%d]\n", array_capacity(&a));
printf("low[%u] high[%u]\n", array_low(&a), array_high(&a));
exit(1);

    array_dump(&a, stdout);

    array_put(&a, 52, (uint32_t[]){52});
    array_put(&a, 53, (uint32_t[]){53});
    array_put(&a, 167, (uint32_t[]){167});
    array_put(&a, 223, (uint32_t[]){223});
    array_put(&a, 78, (uint32_t[]){78});
    array_put(&a, 205, (uint32_t[]){205});
    array_put(&a, 183, (uint32_t[]){183});
    array_put(&a, 150, (uint32_t[]){150});
    array_put(&a, 90, (uint32_t[]){90});
    array_put(&a, 66, (uint32_t[]){66});
    array_put(&a, 91, (uint32_t[]){91});
    array_put(&a, 45, (uint32_t[]){45});
    array_put(&a, 211, (uint32_t[]){211});

    uint32_t arr[] = {985,986,987,988,990,991,992,993,994};
    array_put(&a, 985, arr, 9);

    array_iter_t it;
    array_iter_init(&it, &a, AI_FLAG_FWD);

    uint32_t * j;
    array_for_each(&it, j) {
        printf("arr[%d]\n", *j);
    }

    array_dump(&a, stdout);
    array_delete(&a);

    return 0;
}
