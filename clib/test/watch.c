/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/test/watch.c $                                           */
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
#include <unistd.h>

#include <clib/watch.h>

int callback(watch_event_t * we) {
    printf("hello\n");
    return 0;
}

int main(const int argc, const char * argv[]) {
    watch_t w;
    watch_init(&w);

    watch_callback_t cb = {
        .data = NULL,
        .func = callback,
    };

    watch_add(&w, "/dev/mqueue", IN_CREATE | IN_ATTRIB, &cb);
    watch_wait(&w);

    return 0;
}
