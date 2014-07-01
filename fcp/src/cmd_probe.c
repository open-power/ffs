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
 *    File: cmd_probe.c
 *  Author: Shaun Wetzstein <shaun@us.ibm.com>
 *   Descr: probe implementation
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

#include <spinor/spinor.h>
#include <spinor/aardvark.h>

#include <dbgx/rwflash.h>

#include "misc.h"
#include "main.h"

static int aa_probe(int verbose)
{
	int size = 16;

	uint16_t ports[size];
	uint32_t unique_ids[size];

	int rc = aa_find_devices_ext(size, ports, size, unique_ids);

	fprintf(stdout, "%d aardvark(s) found:\n", rc);

	for (int i = 0; i < min(rc, 16); i++) {
		if (ports[i] & AA_PORT_NOT_FREE) {
			fprintf(stdout, " %d: [%04d-%06d] ===> BUSY",
				ports[i] & ~AA_PORT_NOT_FREE,
				unique_ids[i] / 1000000,
				unique_ids[i] % 1000000);

			continue;
		}

		spinor_t * nor = spinor_open(ports[i]);
		if (nor == NULL) {
			debug("unable to open aaflash on port '%d'\n", ports[i]);
			continue;
		}

		if (spinor_configure(nor) < 0) {
			debug("unable to configure Aardvark on port '%d'\n",
			      ports[i]);
			continue;
		}

		fprintf(stdout, "  %d: [%04d-%06d] => ",
			ports[i] & ~AA_PORT_NOT_FREE,
			unique_ids[i] / 1000000, unique_ids[i] % 1000000);

		spinor_id_t id;
		if (spinor_read_id(nor, id) < 0) {
			debug("unable to read device id\n");
			continue;
		}
			
		fprintf(stdout, "%s %s %s",
			spinor_id_mfg(id), spinor_id_type(id),
			spinor_id_capacity(id));

		fprintf(stdout, " [%2.2x %2.2x %2.2x]\n",
			id[0], id[1], id[2]);

		if (spinor_close(nor) < 0) {
			debug("unable to read device id\n");
			continue;
		}

		nor = NULL;
	}

	fprintf(stdout, "\n");

	return 0;
}

static int rw_probe(const char * host, int verbose)
{
	assert(host != NULL);

	rwflash_cookie_t * cookie = rwflash_cookie_open(host, verbose);
	if (cookie == NULL) {
		debug("unable to open rwflash on host '%s'\n", host);
		return -1;
	}

	if (rwflash_cookie_init(cookie) < 0) {
		debug("unable to init rwflash on host '%s'\n", host);
		return -1;
	}

	if (rwflash_cookie_probe(cookie) < 0) {
		debug("unable to probe rwflash on host '%s'\n", host);
		return -1;
	}

	uint8_t * id = spinor_id(rwflash_cookie_spinor(cookie));
	if (id == NULL) {
		debug("invalid id rwflash on host '%s'\n", host);
		return -1;
	}

	fprintf(stdout, " %s ===> %s %s %s",
		host,
		spinor_id_mfg(id), spinor_id_type(id),
		spinor_id_capacity(id));

	fprintf(stdout, " [%2.2x %2.2x %2.2x]\n",
		id[0], id[1], id[2]);

	return 0;
}

int command_probe(args_t * args)
{
	assert(args != NULL);

	int rc = 0;

	char * type = args->dst_type;
	char * target = args->dst_target;

	int verbose = args->verbose == f_VERBOSE;

	if (strcasecmp(type, TYPE_AA) == 0) {
		rc = aa_probe(verbose);
	} else if (strcasecmp(type, TYPE_RW) == 0) {
		rc = rw_probe(target, verbose);
	} else {
		UNEXPECTED("NOT IMPLEMENTED YET!");
		rc = -1;
	}
	
	return rc;
}
