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
 *   File: dispatch.c
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
#include "dispatch.h"

#define DISPATCH_PAGE_SIZE      64
#define DISPATCH_EVENT_SIZE     10

/* ======================================================================= */

const char *__dispatch_msg[] = {
	"dispatch: unexpected NULL self pointer",
	"dispatch: unexpected NULL callback structure",
};

#define DISPATCH_NULL                   (__dispatch_msg[0])
#define DISPATCH_CALLBACK_NULL          (__dispatch_msg[1])

/* ======================================================================= */

void dispatch_init(dispatch_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(DISPATCH_NULL);

	if (self->fd != -1)
		close(self->fd), self->fd = -1;

	self->fd = epoll_create1(EPOLL_CLOEXEC);
	if (unlikely(self->fd == -1))
		throw_errno(errno);

	array_init(&self->events, sizeof(struct epoll_event),
		   DISPATCH_PAGE_SIZE);
}

void dispatch_delete(dispatch_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(DISPATCH_NULL);
	close(self->fd), self->fd = -1;
	array_delete(&self->events);
}

int dispatch_fileno(dispatch_t * self)
{
	if (unlikely(self == NULL))
		throw_unexpected(DISPATCH_NULL);
	return self->fd;
}

int dispatch_add(dispatch_t * self, int fd, uint32_t events,
		 dispatch_callback_t * cb)
{
	if (unlikely(self == NULL))
		throw_unexpected(DISPATCH_NULL);

	array_status(&self->events, fd, true);

	struct epoll_event *ep;
	ep = (struct epoll_event *)array_at(&self->events, fd);
	ep->events = events;
	ep->data.u64 = (uint64_t) fd << 32;
	ep->data.ptr = (void *)cb;

	if (unlikely(epoll_ctl(self->fd, EPOLL_CTL_ADD, fd, ep)))
		throw_errno(errno);

	return fd;
}

void dispatch_remove(dispatch_t * self, int fd)
{
	if (unlikely(self == NULL))
		throw_unexpected(DISPATCH_NULL);

	if (unlikely(epoll_ctl(self->fd, EPOLL_CTL_DEL, fd, NULL)))
		throw_errno(errno);

	array_status(&self->events, fd, false);
}

void dispatch_wait1(dispatch_t * self)
{
	dispatch_wait2(self, -1);
}

void dispatch_wait2(dispatch_t * self, int timeout)
{
	if (unlikely(self == NULL))
		throw_unexpected(DISPATCH_NULL);

	if (-1 < self->fd) {
		struct epoll_event events[DISPATCH_EVENT_SIZE];

		int rc = epoll_wait(self->fd, events,
				    DISPATCH_EVENT_SIZE, timeout);

		if (0 < rc) {
			dump_memory(stdout, 0, events, sizeof(events));
		}
		for (int n = 0; n < rc; n++) {
			dispatch_callback_t *cb =
			    (dispatch_callback_t *) events[n].data.ptr;
			if (cb != NULL) {
				printf
				    ("cb[%p] fd[%d] data[%p] ep->data.u64[%lld]\n",
				     cb, cb->fd, cb->data, events[n].data.u64);
				if (cb->func != NULL) {
					// dispatch_event ev = {events[n].data.u64 >> 32, events[n].events};
					dispatch_event_t ev =
					    { cb->fd, events[n].events };
					cb->func(&ev, cb->data);
				}
			}
		}
	}
}

/* ======================================================================= */
