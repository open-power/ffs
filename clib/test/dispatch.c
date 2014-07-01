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
#include <unistd.h>

#include <clib/watch.h>
#include <clib/dispatch.h>

int callback(dispatch_event_t * ev, void * ptr) {
    char buf[256];
    int rc;

    rc = read(ev->fd, buf, sizeof buf);
    printf("rc[%d]\n", rc);

    printf("fd[%x]\n", ev->fd);
    printf("events[%x]\n", ev->events);

    return 0;
}

int main(const int argc, const char * argv[]) {
    watch_t w;
    watch_init(&w);

    watch_callback_t wc = {
        .data = NULL,
        .func = NULL,
    };

    watch_add(&w, "/dev/mqueue", IN_CREATE | IN_ATTRIB, &wc);

    dispatch_t d;
    dispatch_init(&d);

    dispatch_callback_t dc = {
        .data = NULL,
        .func = callback,
    };

    dispatch_add(&d, watch_fileno(&w), EPOLLIN, &dc);
    dispatch_wait(&d);

    return 0;
}
