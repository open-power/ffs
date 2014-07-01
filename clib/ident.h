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
 * @file ident.h
 * @brief Identification object
 * @author Shaun Wetzstein <shaun@us.ibm.com>
 * @date 2008-2011
 */

#ifndef __IDENT_H__
#define __IDENT_H__

#include <stdint.h>

#define IDENT_MAGIC_0		0
#define IDENT_MAGIC_1		1
#define IDENT_MAGIC_2		2
#define IDENT_MAGIC_3		3
#define IDENT_MAJOR		4
#define IDENT_MINOR		5
#define IDENT_PATCH		6
#define IDENT_FLAGS		7
#define IDENT_SIZE		8

#define INIT_IDENT		{0,}

#define MAGIC_CHECK(i, m) ({					\
    bool rc = (((i)[IDENT_MAGIC_0] != (m)[IDENT_MAGIC_0]) ||	\
               ((i)[IDENT_MAGIC_1] != (m)[IDENT_MAGIC_1]) ||	\
               ((i)[IDENT_MAGIC_2] != (m)[IDENT_MAGIC_2]) ||	\
               ((i)[IDENT_MAGIC_3] != (m)[IDENT_MAGIC_3]));	\
    rc;								\
			    })

typedef uint8_t ident_t[IDENT_SIZE];

#endif				/* __IDENT_H__ */
