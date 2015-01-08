/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/timer.h $                                                */
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
 *   File: timer.h
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr: Timer
 *   Note:
 *   Date: 10/03/10
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include <sys/timerfd.h>

#include "attribute.h"
#include "builtin.h"
#include "assert.h"
#include "vector.h"

/* ==================================================================== */

typedef struct timer_struct timer_t;
typedef struct timer_callback_struct timer_callback_t;
typedef struct timer_event_struct timer_event_t;

typedef int (*timer_f) (timer_event *);

struct timer_struct {
	int fd;
	vector_t callbacks;
};

struct timer_callback_struct {
	void *data;
	timer_f func;
};

struct timer_event_struct {
};

/* ======================================================================= */

extern void timer_init(timer *, int) __nonnull((1));
extern void timer_delete(timer *) __nonnull((1));

extern int timer_fileno(timer *) __nonnull((1));

extern uint32_t timer_add(timer *, const char *, uint32_t,
			  timer_callback *) __nonnull((1, 2));
extern void timer_remove(timer *, uint32_t) __nonnull((1));

extern void timer_wait(timer *) __nonnull((1));

/* ======================================================================= */

#endif				/* __timer_H__ */
