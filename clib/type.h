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

/*! @file type.h
 *  @brief Type helpers
 *  @author Shaun Wetzstein <shaun@us.ibm.com>
 *  @date 2010-2011
 */

#ifndef __TYPE_H__
#define __TYPE_H__

#include "align.h"

/*!
 * @def cast_type(e,T)
 * @hideinitializer
 * @brief Casts @em p to type @em T
 * @param e [in] Expression
 * @param T [in] Type name
 */
#define cast_type(e,T)						\
    ((T)(e))

/*!
 * @def check_type(e,T)
 * @hideinitializer
 * @brief Check the type of expression @em e is equal to type @em T
 * @param e [in] Expression
 * @param T [in] Type name
 */
#define check_type(e,T)						\
    ((typeof(e) *)0 != (T *)0)

/*!
 * @def check_types_match(e,T)
 * @hideinitializer
 * @brief Check the types of expressions @em e1 and @em e2 are equal
 * @param e1 [in] Expression
 * @param e2 [in] Expression
 */
#define check_types_match(e1,e2)				\
    ((typeof(e1) *) 0 != (typeof(e2) *)0)

/*!
 * @def container_of(member_ptr, containing_type, member)
 * @hideinitializer
 * @brief Returns a pointer to the containing structure of a member
 * @param p [in] Pointer to a strucure member
 * @param T [in] Type of the containing structure
 * @param m [in] Name of the member @em p references
 */
#define container_of(p,T,m)					\
    ((T *)((char *)(p) - offsetof(T,m)) - check_types_match(*(p),((T*)0)->m))

/*!
 * @def container_of_var(member_ptr, var, member)
 * @hideinitializer
 * @brief Returns a pointer to the containing structure of a member
 * @param p [in] Pointer to a strucure member
 * @param v [in] Pointer to a variable with the same of type as the containing structure
 * @param m [in] Name of the member @em p references
 */
#define container_of_var(p, v, m)				\
    container_of(p, typeof(*v), m)

#endif				/* __TYPE_H__ */
