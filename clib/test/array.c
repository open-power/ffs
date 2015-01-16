/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/test/array.c $                                           */
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
