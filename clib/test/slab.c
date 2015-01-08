/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/test/slab.c $                                            */
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
