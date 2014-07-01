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
 *    File: list.c
 *  Author: Shaun Wetzstein <shaun@us.ibm.com>
 *   Descr: --list implementation
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

int command_list(args_t * args)
{
	assert(args != NULL);

	char name[strlen(args->name) + 2];
	strcpy(name, args->name);
	if (strrchr(name, '$') == NULL)
		strcat(name, "$");

	ffs_t *__ffs;

	char full_name[page_size];
	regex_t rx;

	/* ========================= */

	int __list_entry(ffs_entry_t * entry)
	{

		size_t offset = entry->base * __ffs->hdr->block_size;
		size_t size = entry->size * __ffs->hdr->block_size;

                if (__ffs_entry_name(__ffs, entry, full_name,
				     sizeof full_name) < 0)
			return -1;

		if (regexec(&rx, full_name, 0, NULL, 0) == REG_NOMATCH)
			return 0;

		char type;
		if (entry->type == FFS_TYPE_LOGICAL) {
			type ='l';
		} else if (entry->type == FFS_TYPE_DATA) {
			type ='d';
		} else if (entry->type == FFS_TYPE_PARTITION) {
			type ='p';
		}
		fprintf(stdout, "%3d [%08x-%08x] [%8x:%8x] ",
			entry->id, offset, offset+size-1,
			size, entry->actual);

		fprintf(stdout, "[%c%c%c%c%c%c] %s\n",
			type, '-', '-', '-',
			entry->flags & FFS_FLAGS_U_BOOT_ENV ? 'b' : '-',
			entry->flags & FFS_FLAGS_PROTECTED ? 'p' : '-',
			full_name);

		if (args->verbose == f_VERBOSE) {
			for (int i=0; i<FFS_USER_WORDS; i++) {
				fprintf(stdout, "[%2d] %8x ", i,
					entry->user.data[i]);
				if ((i+1) % 4 == 0)
					fprintf(stdout, "\n");
			}
		}

		return 0;
	}

	int list(args_t * args, off_t poffset)
	{
		const char * target = args->target;
		int debug = args->debug;
		int rc = 0;

		RAII(FILE*, file, fopen_generic(target, "r", debug), fclose);
		if (file == NULL)
			return -1;

		if (__ffs_fcheck(file, poffset) < 0)
			return -1;

		RAII(ffs_t*, ffs, __ffs_fopen(file, poffset), __ffs_fclose);
		if (ffs == NULL)
			return -1;

		if (0 < ffs->count) {
			printf("========================[ PARTITION TABLE"
				" 0x%llx ]=======================\n",
				ffs->offset);
			printf("vers:%04x size:%04x * blk:%06x blk(s):"
				"%06x * entsz:%06x ent(s):%06x\n",
				ffs->hdr->version,
				ffs->hdr->size,
				ffs->hdr->block_size,
				ffs->hdr->block_count,
				ffs->hdr->entry_size,
				ffs->hdr->entry_count);
			printf("----------------------------------"
			       "----------------------------------"
			       "-------\n");

			__ffs = ffs;

			rc = __ffs_iterate_entries(ffs, __list_entry);
			if (rc == 1)
				rc = -1;

			printf("\n");
		}

		return rc;
	}

	/* ========================= */

	if (regcomp(&rx, name, REG_ICASE | REG_NOSUB) != 0) {
		ERRNO(errno);
		return -1;
	}

	int rc = command(args, list);

	regfree(&rx);

	return rc;
}
