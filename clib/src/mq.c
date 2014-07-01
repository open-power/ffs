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
 *   File: mqueue.c
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr: POSIX message queue wrapper
 *   Note:
 *   Date: 10/07/10
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
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "libclib.h"
#include "mq.h"

#define MQUEUE_ROOT        "/dev/mqueue"

/* ======================================================================= */

int mqueue_init(mqueue_t * self, const char *service)
{
	assert(self != NULL);

	self->service = strdup(service);
	self->in = self->out = (mqd_t) - 1;

	return 0;
}

int mqueue_create(mqueue_t * self, pid_t tid)
{
	assert(self != NULL);

	char path[pathconf(MQUEUE_ROOT, _PC_PATH_MAX)];

	sprintf(path, "%s/%d->%s", MQUEUE_ROOT, tid, self->service);

	self->out = open(path, O_WRONLY | O_CREAT, S_IWUSR);
	if (self->out == (mqd_t) - 1) {
		ERRNO(errno);
		return -1;
	}

	sprintf(path, "%s/%d<-%s", MQUEUE_ROOT, tid, self->service);

	self->in = open(path, O_RDONLY | O_CREAT, S_IRUSR);
	if (self->in == (mqd_t) - 1) {
		ERRNO(errno);
		return -1;
	}

	return 0;
}

int mqueue_open(mqueue_t * self, char *path)
{
	assert(self != NULL);

	if (path != NULL) {
		char *endp = NULL;
		(void)strtol(path + 1, &endp, 10);

		if (strncmp(endp, "->", 2) == 0) {
			self->in = mq_open((char *)path, O_RDONLY,
					   S_IRWXU, NULL);
			if (self->in == (mqd_t) - 1) {
				ERRNO(errno);
				return -1;
			}
		} else if (strncmp(endp, "<-", 2) == 0) {
			self->out = mq_open((char *)path, O_WRONLY,
					    S_IRWXU, NULL);
			if (self->out == (mqd_t) - 1) {
				ERRNO(errno);
				return -1;
			}
		} else {
			UNEXPECTED("'%s' invalid service", path);
			return -1;
		}
	}

	return 0;
}

int mqueue_close(mqueue_t * self, char *path)
{
	assert(self != NULL);

	if (path != NULL) {
		char *endp = NULL;
		(void)strtol(path + 1, &endp, 10);

		if (strncmp(endp, "->", 2) == 0) {
			if (self->in != (mqd_t) - 1)
				mq_close(self->in), self->in = (mqd_t) - 1;
		} else if (strncmp(endp, "<-", 2) == 0) {
			if (self->out != (mqd_t) - 1)
				mq_close(self->out), self->out = (mqd_t) - 1;
		} else {
			UNEXPECTED("'%s' invalid service", path);
			return -1;
		}
	}

	return 0;
}

mqueue_attr_t mqueue_getattr(mqueue_t * self)
{
	assert(self != NULL);

	mqueue_attr_t attr;
	mq_getattr(self->in, &attr);

	return attr;
}

int mqueue_delete(mqueue_t * self)
{
	assert(self != NULL);

	char path[pathconf(MQUEUE_ROOT, _PC_PATH_MAX)];
	if (self->in != (mqd_t) - 1) {
		sprintf(path, "%s/%d->%s",
			MQUEUE_ROOT, gettid(), self->service);
		unlink(path);
		if (mq_close(self->in) == (mqd_t) - 1) {
			ERRNO(errno);
			return -1;
		}
		self->in = (mqd_t) - 1;
	}

	if (self->out != (mqd_t) - 1) {
		sprintf(path, "%s/%d<-%s",
			MQUEUE_ROOT, gettid(), self->service);
		unlink(path);
		if (mq_close(self->in) == (mqd_t) - 1) {
			ERRNO(errno);
			return -1;
		}
		self->out = (mqd_t) - 1;
	}

	if (self->service) {
		free((void *)self->service);
		self->service = NULL;
	}

	return 0;
}

int mqueue_send(mqueue_t * self, void *ptr, size_t len)
{
	assert(self != NULL);

	int rc = mq_send(self->out, (char *)ptr, len, 0);
	if (rc == -1) {
		ERRNO(errno);
		return -1;
	}

	return rc;
}

int mqueue_receive(mqueue_t * self, void *ptr, size_t len)
{
	assert(self != NULL);

	int rc = mq_receive(self->in, (char *)ptr, len, 0);
	if (rc == -1) {
		ERRNO(errno);
		return -1;
	}

	return rc;
}

/* ======================================================================= */
