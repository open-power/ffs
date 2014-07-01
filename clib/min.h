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
 *   File: min.h
 * Author:
 *  Descr: typesafe min function
 *   Note:
 *   Date: 10/22/10
 */

#ifndef __MIN_H__
#define __MIN_H__

#define min(x,y)			\
({					\
    typeof(x) _x = (x);			\
    typeof(y) _y = (y);			\
    (void)((void*)&_x == (void*)&_y);	\
    _x < _y ? _x : _y;			\
})

#endif				/* __MIN_H__ */
