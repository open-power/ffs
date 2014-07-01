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

#include <clib/exception.h>
#include <clib/memory_leak_detection.h>

#define EXCEPTION_FOO   1
#define EXCEPTION_BAR   2
#define EXCEPTION_TAZ   3

int bar(int i) {
    int * data = MALLOC(sizeof *data);
    *data = i;

    throw_bytes(EXCEPTION_BAR, data, sizeof *data);

    return 0;
}

int foo(int i) {

    exception_t ex;
    try {
	printf("%s: %d\n", __func__, __LINE__);
        bar(i);
    } catch (EXCEPTION_BAR, ex) {
        printf("foo: CAUGHT %s(%d) EXCEPTION_BAR data[%d]\n",
               ex.file, ex.line, *(int *)ex.data);
        throw_bytes(EXCEPTION_FOO, ex.data, ex.size);
    } end_try

    throw_bytes(EXCEPTION_FOO, "this is a test", 14);

	printf("%s: %d\n", __func__, __LINE__);

    return 0;
}

int main(void) {
    exception_t ex;

    try {
	printf("%s: %d\n", __func__, __LINE__);
        foo(1);
        printf("try block: AFTER foo <-- should not get here\n");
    } catch (EXCEPTION_FOO, ex) {
        printf("main: CAUGHT %s(%d) EXCEPTION_FOO data[%d]\n",
               ex.file, ex.line,        *(int *)ex.data);
    } else (ex) {
        printf("main: CAUGHT %s(%d) data[%d]\n",
               ex.file, ex.line, *(int *)ex.data);
    } end_try

    return 0;
}
