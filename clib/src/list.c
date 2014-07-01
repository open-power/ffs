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

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include "assert.h"
#include "misc.h"
#include "list.h"

/* ======================================================================= */

void list_dump(list_t * self, FILE * out)
{
	assert(self != NULL);

	fprintf(out,
		"===================================================================\n");
	fprintf(out, "head: %8x node: %8x\n", (uint32_t) self,
		(uint32_t) & self->node);

	list_node_t *node = &self->node;
	do {
		fprintf(out, "    node: %8x - prev: %8x - next: %8x\n",
			(uint32_t) node, (uint32_t) node->prev,
			(uint32_t) node->next);
		node = node->next;
	} while (node != &self->node);

	fprintf(out,
		"===================================================================\n");
}
