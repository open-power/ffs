/*
 * Copyright (c) International Business Machines Corp., 2014
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "crc32.h"

void usage(void)
{
	printf("Usage: crc32 infile\n");
}

int main(int argc, const char *argv[])
{
	if (argc != 2) {
		usage();
		return 1;
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("open %s: %s\n", argv[1], strerror(errno));
		return 1;
	}

	struct stat stat;
	int rc = fstat(fd, &stat);
	if (rc < 0) {
		printf("stat %s: %s\n", argv[1], strerror(errno));
		return 1;
	}

	off_t filesize = stat.st_size;

	void *in_buf = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
	if (in_buf == MAP_FAILED) {
		/* FIXME could still try to use read().... */
		printf("mmap %d failed: %s\n", filesize, strerror(errno));
		return 1;
	}

	rc = close(fd);
	if (rc < 0) {
		printf("close %s: %s\n", argv[1], strerror(errno));
		return 1;
	}

	unsigned long crc32 = memcpy32(NULL, in_buf, filesize);

	printf("%#010lx", crc32), fflush(stdout);

	return 0;
}
