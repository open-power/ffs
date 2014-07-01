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

/*! @file crc32.h
 *  @brief 32-bit CRC
 *  @details Gary Brown's CRC32 algorithm for polynomial:
 *           X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
 *  @author Gary Brown
 *  @date 2010-2011
 */

#ifndef __CRC32_H__
#define __CRC32_H__

#include <stddef.h>
#include <stdint.h>

/*!
 * @brief Compute a 32-bit CRC value for an octet
 * @param c [in] Input data (octet)
 * @param crc [in] Input 32-bit CRC value
 * @return 32-bit CRC value
 */
extern uint32_t clib_crc32(unsigned char c, uint32_t crc);

/*!
 * @brief Copy bytes from the source reference to the destination reference
 *        while computing a 32-bit CRC
 * @param __dst [in] Destination reference
 * @param __src [in] Source reference
 * @param __n [in] Number of bytes to copy / compute
 * @return 32-bit CRC value
 */
extern uint32_t memcpy32(void *__restrict __dst, const void *__restrict __src,
			 size_t __n)
/*! @cond */
__THROW __nonnull((2)) /*! @endcond */ ;

#endif				/* __CRC32_H__ */
