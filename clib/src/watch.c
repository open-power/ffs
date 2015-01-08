/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/src/watch.c $                                            */
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
