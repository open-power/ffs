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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>

#include <clib/assert.h>
#include <clib/version.h>
#include <clib/err.h>

extern char * program_invocation_short_name;

#define FOO_MAJOR	0x01
#define FOO_MINOR	0x00
#define FOO_PATCH	0x00
#define FOO_VER		VER(FOO_MAJOR, FOO_MINOR, FOO_PATCH)

#define FOO_UNEXPECTED(f, ...)		({		\
	UNEXPECTED(f, ##__VA_ARGS__);			\
	VERSION(FOO_VER, "%s", program_invocation_short_name);	\
					})


int main (int argc, char * argv[]) {
	ERRNO(EINVAL);
	FOO_UNEXPECTED("cannot frob the ka-knob");

	goto error;

	if (false) {
		err_t * err = NULL;
error:
		while ((err = err_get()) != NULL) {
			switch (err_type(err)) {
			case ERR_VERSION:
				fprintf(stderr, "%s: %s : %s(%d) : v%d.%02d.%04d %.*s\n",
					basename((char *)argv[0]),
					err_type_name(err), basename(err_file(err)), err_line(err),
					VER_TO_MAJOR(err_code(err)), VER_TO_MINOR(err_code(err)),
					VER_TO_PATCH(err_code(err)),
					err_size(err), (char *)err_data(err));
				break;
			default:
				fprintf(stderr, "%s: %s : %s(%d) : (code=%d) %.*s\n",
					basename((char *)argv[0]),
					err_type_name(err), basename(err_file(err)), err_line(err),
					err_code(err), err_size(err), (char *)err_data(err));
			}
		}
	}

	return 0;
}

