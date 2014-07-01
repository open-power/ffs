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

/*
 *   File: watch.c
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr:
 *   Note:
 *   Date: 10/03/10
 */

#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "misc.h"
#include "nargs.h"
#include "watch.h"

#define WATCH_PAGE_SIZE         64
#define WATCH_EVENT_SIZE        64

/* ======================================================================= */

const char *__watch_msg[] = {
	"watch: unexpected NULL self pointer",
	"watch: unexpected NULL callback structure",
};

#define WATCH_NULL              (__watch_msg[0])
#define WATCH_CALLBACK_NULL     (__watch_msg[1])

/* ======================================================================= */

void watch_init(watch_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(WATCH_NULL);

	if (self->fd != -1)
		close(self->fd), self->fd = -1;

	self->fd = inotify_init1(IN_CLOEXEC);
	if (unlikely(self->fd == -1))
		throw_errno(errno);

	array_init(&self->callbacks, sizeof(watch_callback_t), WATCH_PAGE_SIZE);
}

void watch_delete(watch_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(WATCH_NULL);
	close(self->fd), self->fd = -1;
	array_delete(&self->callbacks);
}

int watch_fileno(watch_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(WATCH_NULL);
	return self->fd;
}

uint32_t watch_add(watch_t * self, const char *path, uint32_t events,
		   watch_callback_t * cb)
{
	if (unlikely(self == NULL))
		throw_unexpected(WATCH_NULL);

	if (access(path, F_OK) != 0)
		throw_errno(errno);

	uint32_t wd = inotify_add_watch(self->fd, path, events);
	if (unlikely((int)wd == -1))
		throw_errno(errno);

	if (cb != NULL)
		array_put(&self->callbacks, wd, cb, 1);

	return wd;
}

void watch_remove(watch_t * self, uint32_t wd)
{
	if (unlikely(self == NULL))
		throw_unexpected(WATCH_NULL);

	int rc = inotify_rm_watch(self->fd, wd);
	if (unlikely(rc == -1))
		throw_errno(errno);

	array_status(&self->callbacks, wd, false);
}

void watch_wait(watch_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(WATCH_NULL);

	/* FIX ME */

	watch_event_t events[WATCH_EVENT_SIZE];

	ssize_t n = read(self->fd, events, sizeof events);
	printf("n[%d]\n", n);

	for (ssize_t i = 0; i < (ssize_t) (n / sizeof *events); i++)
		printf("%d: wd[%d] mask[%x] cookie[%x] name[%.*s]\n",
		       i, events[i].wd, events[i].mask, events[i].cookie,
		       events[i].len, events[i].name);
}

/* ======================================================================= */
