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
 *   File: hash.h
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr: Various int32 hash functions
 *   Note:
 *   Date: 10/03/10
 */

#ifndef __HASH_H__
#define __HASH_H__

#include <stdint.h>

/* ======================================================================= */

static inline int32_t int32_hash1(int32_t);

typedef uint32_t(*hash_t) (char *, uint32_t);

/* ======================================================================= */

static inline int32_t int32_hash1(int32_t key)
{
	key = ~key + (key << 15);
	key = key ^ (key >> 12);
	key = key + (key << 2);
	key = key ^ (key >> 4);
	key = key * 2057;
	key = key ^ (key >> 16);

	return key;
}

/* ======================================================================= */

#endif				/* __HASH_H__ */
