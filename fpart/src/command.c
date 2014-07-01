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
 *    File: command.c
 *  Author: Shaun Wetzstein <shaun@us.ibm.com>
 *   Descr: generic --<command> wrapper
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

//#include <spinor/aaflash.h>	// --target aa:
//#include <dbgx/rwflash.h>	// --target rw:

#include "main.h"

int parse_offset(const char *str, off_t *offset)
{
	assert(offset != NULL);

	if (str == NULL) {
		*offset = 0;
		return 0;
	}

	char *end = NULL;

	errno = 0;
	*offset = strtoull(str, &end, 0);
	if (errno != 0) {
		ERRNO(errno);
		return -1;
	}

	if (*end != '\0') {
		if (!strcmp(end, "KiB")    ||
		    !strcasecmp(end, "KB") ||
		    !strcasecmp(end, "K"))
			*offset <<= 10;
		else if (!strcmp(end, "MiB")    ||
			 !strcasecmp(end, "MB") ||
			 !strcasecmp(end, "M"))
			*offset <<= 20;
		else if (!strcmp(end, "GiB")    ||
		    	 !strcasecmp(end, "GB") ||
			 !strcasecmp(end, "G"))
			*offset <<= 30;
		else {
			UNEXPECTED("invalid offset specified '%s'", end);
			return -1;
		}
	}

	return 0;
}

int parse_size(const char *str, size_t *size)
{
	assert(size != NULL);

	if (str == NULL) {
		*size = 0;
		return 0;
	}

	char *end = NULL;

	errno = 0;
	*size = strtoul(str, &end, 0);
	if (errno != 0) {
		ERRNO(errno);
		return -1;
	}

	if (*end != '\0') {
		if (!strcmp(end, "KiB") || !strcasecmp(end, "K") ||
		    !strcasecmp(end, "KB"))
			*size <<= 10;
		else if (!strcmp(end, "MiB") || !strcasecmp(end, "M") ||
			 !strcasecmp(end, "MB"))
			*size <<= 20;
		else if (!strcmp(end, "GiB") || !strcasecmp(end, "G") ||
			 !strcasecmp(end, "GB"))
			*size <<= 30;
		else {
			UNEXPECTED("invalid size specified '%s'", end);
			return -1;
		}
	}

	return 0;
}

int parse_number(const char *str, size_t *num)
{
	assert(num != NULL);

	if (str == NULL) {
		*num = 0;
		return 0;
	}

	char *end = NULL;

	errno = 0;
	*num = strtoul(str, &end, 0);
	if (errno != 0) {
		ERRNO(errno);
		return -1;
	}

	if (*end != '\0') {
		UNEXPECTED("invalid number specified '%s'", end);
		return -1;
	}

	return 0;
}

bool check_extension(const char *path, const char *ext)
{
	int len = strlen(path), ext_len = strlen(ext);
	return (ext_len < len)
	    && (strncasecmp(path + len - ext_len, ext, ext_len) == 0);
}

int create_regular_file(const char *path, size_t size, char pad)
{
	assert(path != NULL);

	RAII(FILE*, file, fopen(path, "w"), fclose);
	if (file == NULL) {
		ERRNO(errno);
		return -1;
	}

	if (ftruncate(fileno(file), size) < 0) {
		ERRNO(errno);
		return -1;
	}

	uint32_t page_size = sysconf(_SC_PAGESIZE);
	char buf[page_size];
	memset(buf, pad, page_size);

	while (0 < size) {
		ssize_t rc = fwrite(buf, 1, min(sizeof(buf), size), file);
		if (rc <= 0 && ferror(file)) {
			ERRNO(errno);
			return -1;
		}

		size -= rc;
	}

	return 0;
}

int command(args_t * args, int (*cmd)(args_t *, off_t))
{
	assert(args != NULL);
	assert(cmd != NULL);
	int rc = 0;

	char * end = (char *)args->poffset;
	while (rc == 0 && end != NULL && *end != '\0') {
		errno = 0;
		off_t poffset = strtoull(end, &end, 0);
		if (end == NULL || errno != 0) {
			UNEXPECTED("invalid --partition-offset specified '%s'",
				   args->poffset);
			return -1;
		}

		if (*end != ',' && *end != ':' && *end != '\0') {
			UNEXPECTED("invalid --partition-offset separator "
				   "character '%c'", *end);
			return -1;
		}

		if (cmd != NULL)
			rc = cmd(args, poffset);

		if (*end == '\0')
			break;
		end++;
	}

	return rc;
}

FILE *fopen_generic(const char *path, const char *mode, int debug)
{
	assert(path != NULL);
	assert(mode != NULL);

	FILE *file = NULL;
	size_t port = 0;

	if (strncasecmp(path, "aa:", 3) == 0) {
		if (parse_number(path + 3, &port) == 0)
                        assert(0);
			//file = fopen_aaflash(port, mode, debug);
	} else if (strncasecmp(path, "rw:", 3) == 0) {
                assert(0);
		//file = fopen_rwflash(path + 3, mode, debug);
	} else {
		file = fopen(path, mode);
	}

	if (file == NULL)
		ERRNO(errno);

	return file;
}

int verify_operation(const char * name, ffs_t * in, ffs_entry_t * in_e,
				        ffs_t * out, ffs_entry_t * out_e)
{
	if (in->hdr->block_size != out->hdr->block_size) {
		UNEXPECTED("block sizes differ '%x' != '%x'",
			   in->hdr->block_size, out->hdr->block_size);
		return -1;
	}

	if (in->hdr->block_count != out->hdr->block_count) {
		UNEXPECTED("block counts differ '%x' != '%x'",
			   in->hdr->block_size, out->hdr->block_size);
		return -1;
	}

	if (in->hdr->entry_size != out->hdr->entry_size) {
		UNEXPECTED("entry sizes differ '%x' != '%x'",
			   in->hdr->entry_size, out->hdr->entry_size);
		return -1;
	}

	if (in->hdr->entry_count != out->hdr->entry_count) {
		UNEXPECTED("entry counts differ '%x' != '%x'",
			   in->hdr->entry_size, out->hdr->entry_size);
		return -1;
	}

	if (in_e->base != out_e->base) {
		UNEXPECTED("partition '%s' offsets differ '%x' != '%x'",
			   name, in_e->base, out_e->base);
		return -1;
	}

	if (in_e->size != out_e->size) {
		UNEXPECTED("partition '%s' sizes differ '%x' != '%x'",
			   name, in_e->size, out_e->size);
		return -1;
	}

	return 0;
}

