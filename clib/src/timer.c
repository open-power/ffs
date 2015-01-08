/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/src/timer.c $                                            */
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
