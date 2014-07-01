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
 *    File: copy.c
 *  Author: Shaun Wetzstein <shaun@us.ibm.com>
 *   Descr: --copy implementation
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

int command_copy(args_t * args)
{
	assert(args != NULL);

	char name[strlen(args->name) + 2];
	strcpy(name, args->name);
	if (strrchr(name, '$') == NULL)
		strcat(name, "$");

	char full_name[page_size];
	regex_t rx;

	ffs_t *__in, *__out;
  	off_t __poffset;

	list_t list;
	list_init(&list);

	list_iter_t it;
	ffs_entry_node_t * node;

	/* ========================= */

	int copy_entry(ffs_entry_t * src)
	{
                if (__ffs_entry_name(__in, src, full_name, sizeof full_name)< 0)
			return -1;

		if (regexec(&rx, full_name, 0, NULL, 0) == REG_NOMATCH)
			return 0;

		if (src->flags & FFS_FLAGS_PROTECTED &&
		    args->force != f_FORCE) {
			if (args->verbose == f_VERBOSE)
      				printf("%llx: %s: protected, skipping copy\n",
			               __poffset, full_name);
			return 0;
		}

		__ffs_entry_truncate(__out, full_name, src->actual);
		if (args->verbose == f_VERBOSE)
      			printf("%llx: %s: truncate size '%x'\n",
				__poffset, full_name, src->actual);

		uint32_t src_val, dst_val;
		for (uint32_t i=0; i<FFS_USER_WORDS; i++) {
			__ffs_entry_user_get(__in, full_name, i, &src_val);
			__ffs_entry_user_get(__out, full_name, i, &dst_val);
			if (src_val != dst_val)
				if (__ffs_entry_user_put(__out, full_name,
						     i, src_val) < 0)
					return -1;
		}

		if (args->verbose == f_VERBOSE)
			printf("%llx: %s: user[] copy from '%s'\n", __poffset,
				full_name, args->path);

		list_iter_init(&it, &list, LI_FLAG_FWD);
		list_for_each(&it, node, node) {
			ffs_entry_t * e = &node->entry;
			if (e->base == src->base && e->size == src->size) {
				if (args->verbose == f_VERBOSE)
					printf("%llx: %s: skipping copy from "
					       "'%s'\n", __poffset, full_name,
					       args->path);
				return 0;
			}
		}

		node = (ffs_entry_node_t *)malloc(sizeof(*node));
		assert(node != NULL);

		memcpy(&node->entry, src, sizeof(node->entry));
		list_add_tail(&list, &node->node);

		ffs_entry_t dst;
		if (__ffs_entry_find(__out, full_name, &dst) == false) {
			UNEXPECTED("'%s' entry not found '%s'",
				   args->target, args->name);
			return -1;
		}

		if (verify_operation(full_name, __in, src, __out, &dst) < 0)
			return -1;

		size_t block_size = __in->hdr->block_size;

		if (args->block != NULL) {
			if (parse_size(args->block, &block_size) < 0)
				return -1;
			if (block_size & (__in->hdr->block_size-1)) {
				UNEXPECTED("'%x' block size must be multiple "
					   "of target block size '%x'",
					   block_size, __in->hdr->block_size);
				return -1;
			}
		}

		if (__ffs_buffer(__in, block_size) < 0)
			return -1;
		if (__ffs_buffer(__out, block_size) < 0)
			return -1;

		RAII(void*, block, malloc(block_size), free);
		if (block == NULL) {
			ERRNO(errno);
			return -1;
		}

		size_t data_size = src->actual;
		off_t data_offset = 0;

		while (0 < data_size) {
			ssize_t rc;

			rc = __ffs_entry_read(__in, full_name, block,
					      data_offset,
					      min(block_size, data_size));
			if (rc < 0)
				return -1;

			rc = __ffs_entry_write(__out, full_name, block,
					       data_offset, rc);
			if (rc < 0)
				return -1;

			data_offset += rc;
			data_size -= rc;
		}

		__ffs_fsync(__out);

		if (args->verbose == f_VERBOSE)
			printf("%llx: %s: copy from '%s'\n", __poffset,
				full_name, args->path);

		return 0;
	}

	int copy(args_t * args, off_t poffset)
	{
		__poffset = poffset;

		const char * path = args->path;
		const char * target = args->target;
		int debug = args->debug;

		RAII(FILE*, inf, fopen_generic(path, "r", debug), fclose);
		if (inf == NULL)
			return -1;

		RAII(ffs_t*, in, __ffs_fopen(inf, poffset), __ffs_fclose);
		if (in == NULL)
			return -1;

		if (args->force == f_FORCE) {
			size_t block_size = in->hdr->block_size;
			char block[block_size];

			size_t rc;
			rc = __ffs_entry_read(in, FFS_PARTITION_NAME, block,
					      0, block_size);
			if (rc != block_size) {
				UNEXPECTED("'%s' unable to read '%s' from "
					   "offset '%llx'", args->path,
					   FFS_PARTITION_NAME, poffset);
				return -1;
			}

			RAII(FILE*, outf,
			     fopen_generic(target, "r+", debug), fclose);
			if (outf == NULL)
				return -1;

			if (fseeko(outf, poffset, SEEK_SET) != 0) {
				ERRNO(errno);
				return -1;
			}

			rc = fwrite(block, 1, block_size, outf);
			if (rc <= 0) {
				if (ferror(outf)) {
					ERRNO(errno);
					return -1;
				}
			}

			fflush(outf);
			if (fileno(outf) != -1)
				fsync(fileno(outf));

			if (args->verbose == f_VERBOSE)
				printf("%llx: %s: force partition table\n",
				       poffset, FFS_PARTITION_NAME);
		}

		RAII(FILE*, outf, fopen_generic(target, "r+", debug), fclose);
		if (outf == NULL)
			return -1;
		RAII(ffs_t*, out, __ffs_fopen(outf, poffset), __ffs_fclose);
		if (in == NULL)
			return -1;

		__in = in;
		__out = out;

		int rc = __ffs_iterate_entries(in, copy_entry);
		if (rc == 1)
			rc = -1;

		return rc;
	}

	/* ========================= */

	if (regcomp(&rx, name, REG_ICASE | REG_NOSUB) != 0) {
		ERRNO(errno);
		return -1;
	}

	int rc = command(args, copy);

	regfree(&rx);

	while (!list_empty(&list))
		free(container_of(list_remove_head(&list),
				  ffs_entry_node_t, node));

	return rc;
}
