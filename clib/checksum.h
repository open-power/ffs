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

/*! @file checksum.h
 *  @brief Simple XOR checksum
 *  @author Mike Kobler <mkobler@us.ibm.com>
 *  @date 2007-2012
 */

#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

#include <stdint.h>

/*!
 * @brief Copy bytes from the source reference to the destination reference
 *        while computing a 32-bit checksum
 * @param __dst [in] Destination reference
 * @param __src [in] Source reference (must be 4 byte aligned)
 * @param __n [in] Number of bytes to copy / compute
 * @return 32-bit Checksum value
 */
extern uint32_t memcpy_checksum(void *__restrict __dst,
				const void *__restrict __src, size_t __n)
/*! @cond */
__THROW __nonnull((2)) /*! @endcond */ ;

#endif				/* __CHECKSUM_H__ */
