/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/builtin.h $                                              */
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

/*! @file builtin.h
 *  @brief Builtin Function Macros
 *  @author Shaun Wetzstein <shaun@us.ibm.com>
 *  @date 2010-2011
 */

#ifndef __BUILTIN_H__
#define __BUILTIN_H__

/*!
 * @def parity(x)
 * @hideinitializer
 * @brief Return parity of the input
 * @param x [in] Object
 */
#define	parity(x)		__builtin_parity((x))
#define	ffs(x)			__builtin_ffs((x))

/*!
 * @def popcount(x)
 * @hideinitializer
 * @brief Return number of 0b'1' bits of an int
 * @param x [in] Object
 */
#define	popcount(x)		__builtin_popcount((x))

/*!
 * @def ctz(x)
 * @hideinitializer
 * @brief Count trailing zeros of an int
 * @param x [in] @em int value
 */
#define	ctz(x)			__builtin_ctz((x))

/*!
 * @def ctzl(x)
 * @hideinitializer
 * @brief Count trailing zeros of a long
 * @param x [in] @em long value
 */
#define	ctzl(x)			__builtin_ctzl((x))

/*!
 * @def ctzll(x)
 * @hideinitializer
 * @brief Count trailing zeros of a long long
 * @param x [in] @em long @em long value
 */
#define	ctzll(x)		__builtin_ctzll((x))

/*!
 * @def clz(x)
 * @hideinitializer
 * @brief Count leading zeros of an int
 * @param x [in] @em int value
 */
#define	clz(x)			__builtin_clzl((x))

/*!
 * @def clzl(x)
 * @hideinitializer
 * @brief Count leading zeros of a long
 * @param x [in] @em long value
 */
#define	clzl(x)			__builtin_clzl((x))

/*!
 * @def clzll(x)
 * @hideinitializer
 * @brief Count leading zeros of a long long
 * @param x [in] @em long long value
 */
#define	clzll(x)		__builtin_clzll((x))

/*!
 * @def caller(x)
 * @hideinitializer
 * @brief Return callers return address
 * @param x [in] Function name
 */
#define caller(x)		__builtin_return_address((x))

#define choose_expr(x,y,z)	__builtin_choose_expr((x),(y),(z))

/*!
 * @def const_expr(x)
 * @hideinitializer
 * @brief Return callers return address
 * @param x [in] Function name
 */
#define const_expr(x)		__builtin_constant_p((x))

/*!
 * @def compatible_type(x)
 * @hideinitializer
 * @brief Return @em true if typeof(x) and typeof(y) are compatible
 * @param x [in] Type name
 * @param y [in] Type name
 */
#define compatible_type(x,y)	__builtin_types_compatible_p((x),(y))

#endif				/* __BUILTIN_H__ */
