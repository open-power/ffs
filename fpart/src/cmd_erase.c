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
 *    File: erase.c
 *  Author: Shaun Wetzstein <shaun@us.ibm.com>
 *   Descr: --erase implementation
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

int command_erase(args_t * args)
{
	assert(args != NULL);

	char name[strlen(args->name) + 2];
	strcpy(name, args->name);
	if (strrchr(name, '$') == NULL)
		strcat(name, "$");

	size_t pad = 0xff;
	if (args->pad != NULL)
		if (parse_size(args->pad, &pad) < 0)
			return -1;

	char full_name[page_size];
	regex_t rx;

	ffs_t * __ffs;
	off_t __poffset;

	list_t list;
	list_init(&list);

	list_iter_t it;
	ffs_entry_node_t * node;

	/* ========================= */

	int erase_entry(ffs_entry_t * entry)
	{
                if (__ffs_entry_name(__ffs, entry, full_name,
				     sizeof full_name) < 0)
			return -1;

		if (regexec(&rx, full_name, 0, NULL, 0) == REG_NOMATCH)
			return 0;

		if (entry->flags & FFS_FLAGS_PROTECTED &&
		    args->force != f_FORCE) {
			if (args->verbose == f_VERBOSE)
				printf("%llx: %s: protected, skipping erase\n",
			       	       __poffset, full_name);
			return 0;
		}

		if (__ffs_entry_truncate(__ffs, full_name, 0) < 0)
			return -1;

		if (args->verbose == f_VERBOSE)
			printf("%llx: %s: truncate size '%x'\n", __poffset,
		       		full_name, 0);

		for (uint32_t i=0; i<FFS_USER_WORDS; i++)
			if (__ffs_entry_user_put(__ffs, full_name, i, 0) < 0)
				return -1;

		if (args->verbose == f_VERBOSE)
			printf("%llx: %s: user[] zero\n", __poffset, full_name);

		list_iter_init(&it, &list, LI_FLAG_FWD);
		list_for_each(&it, node, node) {
			if (node->entry.base == entry->base) {
				if (args->verbose == f_VERBOSE)
					printf("%llx: %s: skipping fill with "
					       "'%x'\n", __poffset, full_name,
					       (uint8_t)pad);
				return 0;
			}
		}

		node = (ffs_entry_node_t *)malloc(sizeof(*node));
		assert(node != NULL);

		memcpy(&node->entry, entry, sizeof(node->entry));
		list_add_tail(&list, &node->node);

		uint32_t block_size = __ffs->hdr->block_size;

		if (args->block != NULL) {
			if (parse_size(args->block, &block_size) < 0)
				return -1;
			if (block_size & (__ffs->hdr->block_size-1)) {
				UNEXPECTED("'%x' block size must be multiple "
					   "of target block size '%x'",
					   block_size, __ffs->hdr->block_size);
				return -1;
			}
		}

		if (__ffs_buffer(__ffs, block_size) < 0)
			return -1;

		RAII(void*, block, malloc(block_size), free);
		if (block == NULL) {
			ERRNO(errno);
			return -1;
		}
		memset(block, pad, block_size);

		for (uint32_t i = 0; i < entry->size; i++)
			if(__ffs_entry_write(__ffs, full_name, block,					     		     i * block_size, block_size) < 0)
				return -1;

		__ffs_fsync(__ffs);

		if (__ffs_entry_truncate(__ffs, full_name, 0) < 0)
			return -1;

		if (args->verbose == f_VERBOSE)
			printf("%llx: %s: filled with '%x'\n", __poffset,
	       	       		full_name, (uint8_t)pad);

		return 0;
	}

	int erase(args_t * args, off_t poffset)
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

		int rc = __ffs_iterate_entries(ffs, erase_entry);
		if (rc == 1)
			rc = -1;

		return rc;
	}

	/* ========================= */

	if (regcomp(&rx, name, REG_ICASE | REG_NOSUB) != 0) {
		ERRNO(errno);
		return -1;
	}

	int rc = command(args, erase);

	regfree(&rx);

	while (!list_empty(&list))
		free(container_of(list_remove_head(&list),
				  ffs_entry_node_t, node));

	return rc;
}
