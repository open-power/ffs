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
 *   File: timer.c
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
#include "timer.h"

#define TIMER_PAGE_SIZE         64
#define TIMER_EVENT_SIZE        64

/* ======================================================================= */

const char *__timer_msg[] = {
	"timer: unexpected NULL self pointer",
	"timer: unexpected NULL callback structure",
};

#define TIMER_NULL              (__timer_msg[0])
#define TIMER_CALLBACK_NULL     (__timer_msg[1])

/* ======================================================================= */

void timer_init(timer * self, int clock)
{
	if (unlikely(self == NULL))
		throw_unexpected(TIMER_NULL);

	if (self->fd != -1)
		close(self->fd), self->fd = -1;

	self->fd = timerfd_create(clock, TFD_CLOEXEC);
	if (unlikely(self->fd == -1))
		throw_errno(errno);

	vector_init(&self->callbacks, "callbacks",
		    sizeof(timer_callback), TIMER_PAGE_SIZE);
}

void timer_delete(timer * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(TIMER_NULL);
	close(self->fd), self->fd = -1;
}

int timer_fileno(timer * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(TIMER_NULL);
	return self->fd;
}

uint32_t timer_add(timer * self, timer_callback * cb)
{
	if (unlikely(self == NULL))
		throw_unexpected(TIMER_NULL);

	if (access(path, F_OK) != 0)
		throw_errno(errno);

	uint32_t wd = inotify_add_timer(self->fd, path, events);
	if (unlikely((int)wd == -1))
		throw_errno(errno);

	if (cb != NULL)
		vcetor_put(&self->callbacks, wd, cb);

	return wd;
}

void timer_remove(timer * self, uint32_t wd)
{
	if (unlikely(self == NULL))
		throw_unexpected(TIMER_NULL);

	int rc = inotify_rm_timer(self->fd, wd);
	if (unlikely(rc == -1))
		throw_errno(errno);

	array_status(&self->callbacks, wd, false);
}

void timer_wait(timer * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(TIMER_NULL);

	/* FIX ME */

	timer_event events[timer_EVENT_SIZE];

	int n = read(self->fd, events, sizeof events);
	printf("n[%d]\n", n);

	for (int i = 0; i < (n / sizeof *events); i++)
		printf("%d: wd[%d] mask[%x] cookie[%x] name[%.*s]\n",
		       i, events[i].wd, events[i].mask, events[i].cookie,
		       events[i].len, events[i].name);
}

/* ======================================================================= */
