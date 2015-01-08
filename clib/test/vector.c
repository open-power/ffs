/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/test/vector.c $                                          */
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

#include <clib/vector.h>
#include <clib/vector_iter.h>

int main(void) {
    vector_t a = INIT_VECTOR;
    vector_init(&a, "foo", 4, 1024);

    vector_size(&a, 1000);

    vector_put(&a, 0, (uint32_t[]){0xffffffff});
    vector_put(&a, 1, (uint32_t[]){0xffffffff});
    vector_put(&a, 52, (uint32_t[]){52});
    vector_put(&a, 53, (uint32_t[]){53});
    vector_put(&a, 167, (uint32_t[]){167});
    vector_put(&a, 223, (uint32_t[]){223});
    vector_put(&a, 78, (uint32_t[]){78});
    vector_put(&a, 205, (uint32_t[]){205});
    vector_put(&a, 183, (uint32_t[]){183});
    vector_put(&a, 150, (uint32_t[]){150});
    vector_put(&a, 90, (uint32_t[]){90});
    vector_put(&a, 66, (uint32_t[]){66});
    vector_put(&a, 91, (uint32_t[]){91});
    vector_put(&a, 45, (uint32_t[]){45});
    vector_put(&a, 211, (uint32_t[]){211});

    uint32_t arr[] = {985,986,987,988,990,991,992,993,994};
    vector_put(&a, 985, arr, 9);

    vector_dump(&a, stdout);

    vector_size(&a, 200);

    vector_dump(&a, stdout);

    FILE *f = fopen("vector.bin", "w+");
    vector_save(&a, f);
    fclose(f);

    vector_delete(&a);

    f = fopen("vector.bin", "r");
    vector_load(&a, f);
    fclose(f);

#if 1
    vector_iter_t it;
    vector_iter_init(&it, &a, VI_FLAG_FWD);

    uint32_t * j;
    vector_for_each(&it, j) {
        printf("arr[%d] = %d\n", it.idx, *j);
    }
#endif

    vector_dump(&a, stdout);

    vector_delete(&a);

    return 0;
}
