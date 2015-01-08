/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/dispatch.h $                                             */
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
 *   File: dispatch.h
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr: File descriptor activity callback dispatcher
 *   Note:
 *   Date: 10/03/10
 */

#ifndef __DISPATCH_H__
#define __DISPATCH_H__

#include <stdint.h>
#include <stdbool.h>

#include <sys/epoll.h>

#include "attribute.h"
#include "builtin.h"
#include "assert.h"

#include "tree.h"
#include "array.h"

#define	DISPATCH_READ		EPOLLIN
#define DISPATCH_WRITE		EPOLLOUT
#define DISPATCH_READ_HUP	EPOLLRDHUP
#define DISPATCH_READ_PRIORITY	EPOLLPRI
#define DISPATCH_ERROR		EPOLLERR
#define DISPATCH_WRITE_HUP	EPOLLRDHUP
#define DISPATCH_EDGE_TRIGGER	EPOLLET
#define DISPATCH_ONE_SHOT	EPOLLONESHOT

/* ======================================================================= */

typedef struct dispatch dispatch_t;
typedef struct dispatch_callback dispatch_callback_t;
typedef struct dispatch_event dispatch_event_t;

typedef int (*dispatch_f) (dispatch_event_t *, void *);

struct dispatch {
	int fd;
	array_t events;
};

struct dispatch_callback {
	int fd;
	void *data;
	dispatch_f func;
};

struct dispatch_event {
	int fd;
	uint32_t events;
};

/* ======================================================================= */

extern void dispatch_init(dispatch_t *) __nonnull((1));

extern void dispatch_delete(dispatch_t *) __nonnull((1));

extern int dispatch_fileno(dispatch_t *) __nonnull((1));

extern int dispatch_add(dispatch_t *, int, uint32_t, dispatch_callback_t *)
__nonnull((1, 4));
extern void dispatch_remove(dispatch_t *, int) __nonnull((1));

#define dispatch_wait(...) STRCAT(dispatch_wait, NARGS(__VA_ARGS__))(__VA_ARGS__)
extern void dispatch_wait1(dispatch_t *) __nonnull((1));
extern void dispatch_wait2(dispatch_t *, int) __nonnull((1));

/* ======================================================================= */

#endif				/* __DISPATCH_H__ */
