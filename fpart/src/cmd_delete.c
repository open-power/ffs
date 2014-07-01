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
 *    File: delete.c
 *  Author: Shaun Wetzstein <shaun@us.ibm.com>
 *   Descr: --delete implementation
 *    Date: 01/30/2013
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <regex.h>

#include <clib/attribute.h>
#include <clib/assert.h>
#include <clib/list.h>
#include <clib/list_iter.h>
#include <clib/misc.h>
#include <clib/min.h>
#include <clib/err.h>
#include <clib/raii.h>

#include "main.h"

int command_delete(args_t * args)
{
	assert(args != NULL);

	/* ========================= */

	int delete(args_t * args, off_t poffset)
	{
		const char * target = args->target;
		int debug = args->debug;

		RAII(FILE*, file, fopen_generic(target, "r+", debug), fclose);
		RAII(ffs_t*, ffs, __ffs_fopen(file, poffset), __ffs_fclose);

		if (__ffs_entry_delete(ffs, args->name) < 0)
			return -1;

		if (args->verbose == f_VERBOSE)
			printf("%llx: %s: delete\n", poffset, args->name);

		return 0;
	}

	/* ========================= */

	return command(args, delete);
}
