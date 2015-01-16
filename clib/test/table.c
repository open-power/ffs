/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/test/table.c $                                           */
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

#include <clib/table.h>
#include <clib/table_iter.h>

#define COLS 5

int main(void) {
    table_t t = INIT_TABLE;
    table_init(&t, "table", COLS);

    const char * str = "column4 is a really long assed string";

    table_name(&t, 0, "column0");
    table_name(&t, 1, "column1");
    table_name(&t, 2, "column2");
    table_name(&t, 3, "column3");
    table_name(&t, 4, str);

    printf("%s\n", table_name(&t, 4));
    printf("cols: %d\n", table_columns(&t));
    printf("rows: %d\n", table_rows(&t));

    table_dump(&t, stdout);

    table_iter_t it;
    table_iter_init(&it, &t, TI_FLAG_FWD);

    size_t r, c;

    value_t * val;
    table_for_each(&it, val) {
        for (c=0; c<COLS; c++)
            value_dump(val+c, stdout);
    }
    printf("xxx =================\n");

    value_t v[COLS];

    for (r=0; r<COLS; r++) {
        for (c=0; c<COLS; c++) {
            printf("t[%d][%d] --> %d\n", r, c, r * COLS + c);
            value_i32(v+c, r * COLS + c);
     	    value_dump(v+c, stdout);
        }
        table_row(&t, r, v);
    }
    table_dump(&t, stdout);
    printf("yyy =================\n");

    table_for_each(&it, val) {
        for (c=0; c<COLS; c++)
            value_dump(val+c, stdout);
    }
    printf("zzz =================\n");

    FILE * f = fopen("table.bin", "w+");
    table_serialize(&t);
    table_save(&t, f);
    table_delete(&t);
    fclose(f);

    printf("111 =================\n");

    f = fopen("table.bin", "r+");
    table_load(&t, f);
    table_deserialize(&t);
    table_dump(&t, stdout);
    fclose(f);

    table_iter_init(&it, &t, TI_FLAG_FWD);
    table_for_each(&it, val) {
        for (c=0; c<COLS; c++)
            value_dump(val+c, stdout);
    }
    printf("aaa =================\n");

    table_dump(&t, stdout);
    table_delete(&t);

    return 0;
}
