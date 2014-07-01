/*
 *   Copyright (c) International Business Machines Corp., 2012
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 *    File: main.h
 *  Author: Shaun Wetzstein <shaun@us.ibm.com>
 *   Descr: cmdline tool for ECC
 *    Date: 08/30/2012
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>

typedef enum {
    c_ERROR = 0,
    c_INJECT = 'I',
    c_REMOVE = 'R',
    c_HEXDUMP = 'H',
} cmd_t;

typedef enum {
    o_ERROR = 0,
    o_OUTPUT = 'o',
} option_t;

typedef enum {
    f_ERROR = 0,
    f_FORCE = 'f',
    f_P8 = 'p',
    f_VERBOSE = 'v',
    f_HELP = 'h',
} flag_t;

typedef struct {
    const char * short_name;

    /* target */
    const char * path;

    /* command */
    cmd_t cmd;

    /* options */
    const char * file;

    /* flags */
    flag_t force, p8, verbose;

    const char ** opt;
    int opt_sz, opt_nr;
} args_t;

extern args_t args;

#define ECC_MAJOR	0x02
#define ECC_MINOR	0x00
#define ECC_PATCH	0x00

#define ECC_EXT		".ecc"

#endif /* __MAIN_H__ */
