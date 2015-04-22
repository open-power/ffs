/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: clib/src/crc32_main.c $                                       */
/*                                                                        */
/* OpenPOWER FFS Project                                                  */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

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
		printf("mmap %d failed: %s\n", (uint32_t)filesize, strerror(errno));
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
