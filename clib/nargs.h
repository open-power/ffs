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

/*!
 * @file nargs.h
 * @brief Macro utilities
 * @author Shaun Wetzstein <shaun@us.ibm.com>
 * @date 2010-2011
 */

#ifndef __NARGS_H__
#define __NARGS_H__

/*!
 * @def STRCAT(x,y)
 * @hideinitializer
 * @brief C string concatination of @em x and @em y
 * @param x [in] C-style string
 * @param y [in] C-style string
 */
/*! @cond */
#define STRCAT(x,y)	__C__(x, y)
#define __C__(x,y)	x ## y
/*! @endcond */

/*!
 * @def NARGS(...)
 * @hideinitializer
 * @brief Return the number of pre-process macro parameters
 */
/*! @cond */
#define __NARGS__(junk, _10, _9, _8, _7, _6, _5, _4, _3, _2, _1, _, ...) _
#define NARGS(...) __NARGS__(junk, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
/*! @endcond */

#endif				/* __NARGS_H__ */
