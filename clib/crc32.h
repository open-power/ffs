/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/crc32.h $                                                */
/*                                                                        */
/* OpenPOWER FFS Project                                                  */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

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
