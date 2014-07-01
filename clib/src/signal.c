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
 *   File: signal.c
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
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "misc.h"
#include "signal.h"

/* ======================================================================= */

const char *__signal_msg[] = {
	"signal: unexpected NULL self pointer",
	"signal: unexpected NULL callback structure",
};

#define SIGNAL_NULL             (__signal_msg[0])

/* ======================================================================= */

void signal_init(signal_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(SIGNAL_NULL);

	if (self->fd != -1)
		close(self->fd), self->fd = -1;

	sigemptyset(&self->mask);
	self->flags = SFD_CLOEXEC;

	self->fd = signalfd(self->fd, &self->mask, self->flags);
	if (unlikely(self->fd == -1))
		throw_errno(errno);
}

void signal_delete(signal_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(SIGNAL_NULL);

	close(self->fd), self->fd = -1;

	if (sigprocmask(SIG_UNBLOCK, &self->mask, NULL) == -1)
		throw_errno(errno);

	sigemptyset(&self->mask);
	self->flags = 0;
}

int signal_fileno(signal_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(SIGNAL_NULL);
	return self->fd;
}

int signal_setmask(signal_t * self, const sigset_t mask)
{
	if (unlikely(self == NULL))
		throw_unexpected(SIGNAL_NULL);

	self->mask = mask;

	if (sigprocmask(SIG_BLOCK, &self->mask, NULL) == -1)
		throw_errno(errno);

	self->fd = signalfd(self->fd, &self->mask, self->flags);
	if (unlikely(self->fd == -1))
		throw_errno(errno);

	return self->fd;
}

#if 0
void signal_wait1(signal_t * self)
{
	signal_wait2(self, -1);
}

void signal_wait2(signal_t * self, int timeout)
{
	if (unlikely(self == NULL))
		throw_unexpected(SIGNAL_NULL);

	struct epoll_event events[signal_EVENT_SIZE];

	int rc = epoll_wait(self->fd, events,
			    signal_EVENT_SIZE, timeout);

	for (int n = 0; n < rc; n++) {
		signal_callback *cb = (signal_callback *) events[n].data.ptr;
		if (cb != NULL)
			if (cb->func != NULL)
				cb->func((signal_event[1]) { {
					 events[n].data.u64 >> 32,
					 events[n].events}
					 }
					 , cb->data);
	}
}
#endif

/* ======================================================================= */
