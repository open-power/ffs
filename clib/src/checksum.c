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

#include <stdio.h>
#include <stdint.h>

#include "assert.h"
#include "checksum.h"

uint32_t memcpy_checksum(void *__restrict __dst, const void *__restrict __src,
			 size_t __n)
{
	uint8_t sum[4] = { 0, };

	/* assert(((uintptr_t)__src & 3) == 0); */

	size_t i;

	if (__dst == NULL)
		for (i = 0; i < __n; i++)
			sum[i & 3] ^= ((uint8_t *) __src)[i];
	else
		for (i = 0; i < __n; i++)
			sum[i & 3] ^= ((uint8_t *) __src)[i],
			    ((uint8_t *) __dst)[i] = ((uint8_t *) __src)[i];

	return (sum[0] << 24) | (sum[1] << 16) | (sum[2] << 8) | sum[3];
}
