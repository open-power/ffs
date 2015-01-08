/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/src/mq.c $                                               */
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
