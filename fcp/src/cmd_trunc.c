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
 *    File: cmd_trunc.c
 *  Author: Shaun Wetzstein <shaun@us.ibm.com>
 *   Descr: trunc implementation
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

#include "misc.h"
#include "main.h"

static int __trunc(args_t * args, off_t offset)
{
	assert(args != NULL);

	char * type = args->dst_type;
	char * target = args->dst_target;
	char * name = args->dst_name;

	RAII(FILE*, file, __fopen(type, target, "r+", debug), fclose);
	if (file == NULL)
		return -1;
	if (check_file(target, file, offset) < 0)
		return -1;
	RAII(ffs_t*, ffs, __ffs_fopen(file, offset), __ffs_fclose);
	if (ffs == NULL)
		return -1;

	if (ffs->count <= 0)
		return 0;

	size_t block_size;
	if (__ffs_info(ffs, FFS_INFO_BLOCK_SIZE, &block_size) < 0)
		return -1;

	if (args->buffer != NULL) {
		size_t buffer;
		if (parse_size(args->buffer, &buffer) < 0)
			return -1;
		if (__ffs_buffer(ffs, buffer) < 0)
			return -1;
	}

	ffs_entry_t entry;
	if (__ffs_entry_find(ffs, name, &entry) == false) {
		UNEXPECTED("partition entry '%s' not found\n", name);
		return -1;
	}

	char full_name[page_size];
	if (__ffs_entry_name(ffs, &entry, full_name,
			     sizeof full_name) < 0)
		return -1;

	if (entry.type == FFS_TYPE_LOGICAL) {
		if (args->verbose == f_VERBOSE)
			fprintf(stderr, "%8llx: %s: logical (skip)\n",
				offset, full_name);
		return 0;
	}

	if (args->protected != f_PROTECTED &&
	    entry.flags && FFS_FLAGS_PROTECTED) {
		if (args->verbose == f_VERBOSE)
			fprintf(stderr, "%8llx: %s: protected (skip)\n",
				offset, full_name);
		return 0;
	}

	size_t size = 0;
	if (args->opt_nr == 1) {
		size = entry.size * block_size;
	} else if (args->opt_nr == 2) {
		if (parse_number(args->opt[1], &size) < 0)
			return -1;
	}

	if (__ffs_entry_truncate(ffs, full_name, size) < 0) {
		ERRNO(errno);
		return -1;
	}

	if (args->verbose == f_VERBOSE)
		fprintf(stderr, "%8llx: %s: truncate '%x' (done)\n",
			offset, full_name, size);

	return 0;
}

int command_trunc(args_t * args)
{
	assert(args != NULL);

	int rc = 0;

	char * end = (char *)args->offset;
	while (rc == 0 && end != NULL && *end != '\0') {
		errno = 0;
		off_t offset = strtoull(end, &end, 0);
		if (end == NULL || errno != 0) {
			UNEXPECTED("invalid --offset specified '%s'",
				   args->offset);
			return -1;
		}

		if (*end != ',' && *end != ':' && *end != '\0') {
			UNEXPECTED("invalid --offset separator "
				   "character '%c'", *end);
			return -1;
		}

		rc = __trunc(args, offset);
		if (rc < 0)
			break;

		if (*end == '\0')
			break;
		end++;
	}

	return rc;
}
