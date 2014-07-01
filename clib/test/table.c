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
