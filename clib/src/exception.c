/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/src/exception.c $                                        */
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
 *   File: exc.c
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr: {set,long}jmp implementation of exceptions for C code.
 *   Note: using these macros will create an exception context
 *         in each thread.
 *   Date: 7/06/09
 */

#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <execinfo.h>

#include "exception.h"
#include "misc.h"

/* =======================================================================*/

const char *exception_name[] = {
	"assertion",
	"unexpected",
	"errno",
};

typedef struct exception_context {
	pthread_once_t once;
	pthread_key_t key;
} exception_context;

/* =======================================================================*/

static exception_context __exc_ctx = { PTHREAD_ONCE_INIT, 0 };

static inline void __exc_key_create(void)
{
	if (pthread_key_create(&__exc_ctx.key, NULL))
		__exc_throw(ASSERTION, NULL, 0, __FILE__, __LINE__);
}

/* =======================================================================*/

void __exc_init(void)
{
	if (pthread_once(&__exc_ctx.once, __exc_key_create))
		__exc_throw(ASSERTION, NULL, 0, __FILE__, __LINE__);
}

exception_frame_t *__exc_get_frame(void)
{
	return (exception_frame_t *) (pthread_getspecific(__exc_ctx.key));
}

void __exc_set_frame(exception_frame_t * f)
{
	if (pthread_setspecific(__exc_ctx.key, f))
		__exc_throw(ASSERTION, NULL, 0, __FILE__, __LINE__);
}

void __exc_backtrace(const char *fmt, ...)
{
	if (fmt != NULL) {
		va_list va;
		va_start(va, fmt);
		vfprintf(stderr, fmt, va);
		va_end(va);
	}

	fprintf(stderr, "========== backtrace ==========\n");
	void *bt[1024];
	int nr = backtrace(bt, sizeof bt);
	backtrace_symbols_fd(bt, nr, fileno(stderr));
	fprintf(stderr, "========== backtrace ==========\n");
}

int __exc_throw(int ex, void *data, int size, const char *file, int line)
{
	extern char *program_invocation_short_name;

	exception_frame_t *frame = __exc_get_frame();

	if (frame == NULL) {
		__exc_backtrace
		    ("*** UNHANDLED EXCEPTION *** -- %s: %s(%d) ex=%d\n\n",
		     program_invocation_short_name, file, line, ex);
		abort();
	}

	if (frame->magic != EXC_MAGIC) {
		__exc_backtrace
		    ("*** CORRUPTED EXCEPTION FRAME *** -- %s: %s(%d) ex=%d\n\n",
		     program_invocation_short_name, file, line, ex);
		abort();
	}

	frame->exc.file = file;
	frame->exc.line = line;

	frame->exc.data = data;
	frame->exc.size = size;

	longjmp(frame->jmp, ex);

	return -1;
}

int __exc_rethrow(int ex, void *data, int size, const char *file, int line)
{
	exception_frame_t *frame = __exc_get_frame();

	if (frame != NULL)
		__exc_set_frame(frame->prev);

	return __exc_throw(ex, data, size, file, line);
}

const char *__exc_name(int exc)
{
	return (exc < 0 || EXC_LAST <= exc) ? NULL : exception_name[exc];
}

__constructor void __exc_ctor(void)
{
	__exc_init();
}
