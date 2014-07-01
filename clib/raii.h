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
 *   File: raii.h
 * Author:
 *  Descr: RAII macro
 *   Note:
 *   Date: 04/03/13
 */

#ifndef __RAII_H__
#define __RAII_H__

#include "attribute.h"

#define CLEANUP(type,name,func) 			\
    void __cleanup_ ## name (type * __v) { 		\
		if (__v != NULL)			\
			func(__v); 			\
    }							\
    type name __cleanup(__cleanup_##name)

#define RAII(type,name,ctor,dtor) 			\
    void __cleanup_##name (type * __v) { 		\
		if (__v != NULL && *__v != NULL) 	\
			dtor(*__v); 			\
    }							\
    type name __cleanup(__cleanup_##name) = (ctor)

#endif				/* __RAII_H__ */
