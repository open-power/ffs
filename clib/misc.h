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
 *   File: misc.h
 * Author: Shaun Wetzstein <shaun@us.ibm.com?
 *  Descr: Misc. library helper functions
 *   Note:
 *   Date: 10/22/10
 */

#ifndef __MISC_H__
#define __MISC_H__

#include <stdint.h>
#include <stdio.h>

#include <sys/syscall.h>

#include "attribute.h"

#include "align.h"
#include "min.h"
#include "max.h"

#define INT32_BIT	(CHAR_BIT*sizeof(int32_t))

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

#ifndef gettid
#define gettid()	({			\
    pid_t __tid = (pid_t)syscall(SYS_gettid);	\
    __tid;					\
			 })
#endif

#define is_pow2(x)				\
({						\
    typeof(x) _x = (x);				\
    (((_x) != 0) && !((_x) & ((_x) - 1)));	\
})

#define __align(x,y)				\
({						\
    assert(is_pow2((y)));			\
    typeof((y)) _y = (y) - 1;			\
    typeof((x)) _x = ((x)+_y) & ~_y;		\
    _x;						\
})

extern void prefetch(void *addr, size_t len, ...) __nonnull((1));
extern void print_binary(FILE *, void *, size_t);
extern void dump_memory(FILE *, uint32_t, const void *__restrict,
			size_t) __nonnull((1, 3));
extern unsigned long align(unsigned long size, unsigned long offset);
extern int __round_pow2(int size);

#endif				/* __MISC_H__ */
