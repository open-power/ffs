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
 * @file align.h
 * @brief Alignment helpers
 * @author Shaun Wetzstein <shaun@us.ibm.com>
 * @date 2010-2011
 */

#ifndef __ALIGN_H__
#define __ALIGN_H__

/*!
 * @def alignof(t)
 * @hideinitializer
 * @brief Returns the alignment of an object or minimum alignment required by a type
 * @param t [in] Object or type name
 */
#define alignof(t)			__alignof__(t)

#ifndef offsetof

/*!
 * @def offsetof(t,m)
 * @hideinitializer
 * @brief Returns the offset of a member within a structure
 * @param t [in] Structure type name
 * @param m [in] Member name within a structure
 */
#define offsetof(t,m)			__builtin_offsetof(t, m)
#endif

#endif				/* __ALIGN_H__ */
