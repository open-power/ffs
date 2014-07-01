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
 * @file compare.h
 * @brief Generic compare function
 * @author Shaun Wetzstein <shaun@us.ibm.com>
 * @date 2010-2011
 */

#ifndef __COMPARE_H__
#define __COMPARE_H__

/*!
 * @brief Pointer to a generic compare function
 */
typedef int (*compare_f) (const void *, const void *);

/*!
 * @brief Default compare function
 */
static inline int default_compare(const void *v1, const void *v2)
{
	const int i1 = (const int)v1, i2 = (const int)v2;
	return i1 - i2;
}

#endif				/* __COMPARE_H__ */
