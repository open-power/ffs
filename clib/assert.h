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

/*! @file assert.h
 *  @brief Assertion helpers
 *  @author Shaun Wetzstein <shaun@us.ibm.com>
 *  @date 2010-2011
 */

#ifdef assert
#undef assert
#endif

/*!
 * @def assert(e)
 * @hideinitializer
 * @brief Check for program assertion failures
 * @param e [in] assertion expression
 * @throws ASSERTION iff expression @em e evaluates @em false
 */

#ifdef NDEBUG
#define assert(e) ((void)0)
#else
#include "exception.h"
#define assert(e) ((void)((e) ? (void)0 : throw_bytes(ASSERTION, __STRING((e)), strlen(__STRING((e))))))
#endif
