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
 *    File: user.c
 *  Author: Shaun Wetzstein <shaun@us.ibm.com>
 *   Descr: --user implementation
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

int command_user(args_t * args)
{
	assert(args != NULL);

	char name[strlen(args->name) + 2];
	strcpy(name, args->name);
	if (strrchr(name, '$') == NULL)
		strcat(name, "$");

	char full_name[page_size];
	regex_t rx;

	off_t __poffset;
	ffs_t * __ffs;

	uint32_t user = 0;
	if (args->user != NULL)
		if (parse_size(args->user, &user) < 0)
			return -1;

	if (FFS_USER_WORDS <= user) {
		UNEXPECTED("invalid user word '%d', valid range [0..%d]",
			   user, FFS_USER_WORDS);
		return -1;
	}

	int user_entry(ffs_entry_t * entry)
	{
		assert(entry != NULL);

		if (__ffs_entry_name(__ffs, entry, full_name,
			 	     sizeof full_name) < 0)
			return -1;

		if (regexec(&rx, full_name, 0, NULL, 0) == REG_NOMATCH)
			return 0;

		if (entry->flags & FFS_FLAGS_PROTECTED &&
		    args->force != f_FORCE) {
			printf("%llx: %s: protected, skipping user[%d]\n",
			       __poffset, full_name, user);
			return 0;
		}

		uint32_t value = 0;
		if (args->value != NULL) {
			if (args->value != NULL)
				if (parse_size(args->value, &value) < 0)
					return -1;

			if (__ffs_entry_user_put(__ffs, full_name,
						 user, value) < 0)
				return -1;

			if (args->verbose == f_VERBOSE)
				printf("%llx: %s: user[%d] = '%x'\n", __poffset,
				       args->name, user, value);
		} else {
			if (__ffs_entry_user_get(__ffs, args->name,
						 user, &value) < 0)
				return -1;

			printf("%llx: %s: user[%d] = '%x'\n", __poffset,
			       args->name, user, value);
		}

		return 0;
	}

	int __user(args_t * args, off_t poffset)
	{
		__poffset = poffset;

		const char * target = args->target;
		int debug = args->debug;

		RAII(FILE*, file, fopen_generic(target, "r+", debug), fclose);
		if (file == NULL)
			return -1;
		RAII(ffs_t*, ffs, __ffs_fopen(file, poffset), __ffs_fclose);
		if (ffs == NULL)
			return -1;

		__ffs = ffs;

		int rc = __ffs_iterate_entries(ffs, user_entry);
		if (rc == 1)
			return -1;

		return rc;
	}

	/* ========================= */

	if (regcomp(&rx, name, REG_ICASE | REG_NOSUB) != 0) {
		ERRNO(errno);
		return-1;
	}

	int rc = command(args, __user);

	regfree(&rx);

	return rc;
}
