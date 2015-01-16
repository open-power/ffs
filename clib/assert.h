/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/assert.h $                                               */
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

/*! @file assert.h
 *  @brief Assertion helpers
 *  @author Shaun Wetzstein <shaun@us.ibm.com>
 *  @date 2010-2011
 */

#ifdef assert
#undef assert
#endif

/*!
 * @def assert(e)
 * @hideinitializer
 * @brief Check for program assertion failures
 * @param e [in] assertion expression
 * @throws ASSERTION iff expression @em e evaluates @em false
 */

#ifdef NDEBUG
#define assert(e) ((void)0)
#else
#include "exception.h"
#define assert(e) ((void)((e) ? (void)0 : throw_bytes(ASSERTION, __STRING((e)), strlen(__STRING((e))))))
#endif
