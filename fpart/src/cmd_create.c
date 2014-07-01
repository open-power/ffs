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
 *    File: create.c
 *  Author: Shaun Wetzstein <shaun@us.ibm.com>
 *   Descr: --create implementation
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

int command_create(args_t * args)
{
	assert(args != NULL);

	size_t block = 0;
	size_t size = 0;
	size_t pad = 0xff;

	if (parse_size(args->block, &block) < 0)
		return -1;
	if (parse_size(args->size, &size) < 0)
		return -1;
	if (args->pad != NULL)
		if (parse_size(args->pad, &pad) < 0)
			return -1;

	struct stat st;
	if (stat(args->target, &st) < 0) {
		if (errno == ENOENT) {
			create_regular_file(args->target, size, (uint8_t)pad);
		} else {
			ERRNO(errno);
			return -1;
		}
	} else {
		if (st.st_size != size) {
			create_regular_file(args->target, size, (uint8_t)pad);
		} else {
			if (args->force != f_FORCE && st.st_size != size) {
				UNEXPECTED("--size '%d' differs from actual "
					   "size '%lld', use --force to "
					   "override", size, st.st_size);
				return -1;
			}
		}
	}

	/* ========================= */

	int create(args_t * args, off_t poffset)
	{
		if (args->verbose == f_VERBOSE)
			printf("%llx: create partition table\n", poffset);

		const char * target = args->target;
		int debug = args->debug;

		RAII(FILE*, file, fopen_generic(target, "r+", debug), fclose);
		if (file == NULL)
			return -1;
		RAII(ffs_t*, ffs, __ffs_fcreate(file, poffset, block,
		     size / block), __ffs_fclose);
		if (ffs == NULL)
			return -1;

		return 0;
	}

	/* ========================= */

	return command(args, create);
}
