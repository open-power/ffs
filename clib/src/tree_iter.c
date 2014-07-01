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

/*
 *   File: tree_iter.c
 * Author: Shaun Wetzstein <shaun@us.ibm.com>
 *  Descr:
 *   Note:
 *   Date: 10/22/10
 */

#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "libclib.h"
#include "vector.h"
#include "tree_iter.h"

/* ======================================================================= */

int tree_iter_init(tree_iter_t * self, tree_t * tree, uint32_t flags)
{
	assert(self != NULL);
	assert(tree != NULL);

	self->node = NULL;
	self->safe = NULL;

	self->tree = tree;
	self->flags = flags;

	return tree_iter_clear(self);
}

int tree_iter_clear(tree_iter_t * self)
{
	assert(self != NULL);

	if (self->flags & TI_FLAG_FWD) {
		self->node = tree_min(self->tree);
		if (self->node != NULL)
			self->safe = tree_node_next(self->node);
	} else if (self->flags & TI_FLAG_BWD) {
		self->node = tree_max(self->tree);
		if (self->node != NULL)
			self->safe = tree_node_prev(self->node);
	} else {
		UNEXPECTED("invalid tree_iter flags");
		return -1;
	}

	return 0;
}

tree_node_t *tree_iter_elem(tree_iter_t * self)
{
	assert(self != NULL);
	return self->node;
}

tree_node_t *tree_iter_inc1(tree_iter_t * self)
{
	return tree_iter_inc2(self, 1);
}

tree_node_t *tree_iter_inc2(tree_iter_t * self, size_t count)
{
	assert(self != NULL);

	for (size_t i = 0; i < count && self->node != NULL; i++) {
		if (self->flags & TI_FLAG_FWD) {
			self->node = self->safe;
			if (self->node != NULL)
				self->safe = tree_node_next(self->node);
		} else if (self->flags & TI_FLAG_BWD) {
			self->node = self->safe;
			if (self->node != NULL)
				self->safe = tree_node_prev(self->node);
		} else {
			UNEXPECTED("invalid tree_iter flags");
			return NULL;
		}
	}

	return self->node;
}

tree_node_t *tree_iter_dec1(tree_iter_t * self)
{
	return tree_iter_dec2(self, 1);
}

tree_node_t *tree_iter_dec2(tree_iter_t * self, size_t count)
{
	assert(self != NULL);

	for (size_t i = 0; i < count && self->node != NULL; i++) {
		if (self->flags & TI_FLAG_FWD) {
			self->node = self->safe;
			if (self->node != NULL)
				self->safe = tree_node_prev(self->node);
		} else if (self->flags & TI_FLAG_BWD) {
			self->node = self->safe;
			if (self->node != NULL)
				self->safe = tree_node_next(self->node);
		} else {
			UNEXPECTED("invalid tree_iter flags");
			return NULL;
		}
	}

	return self->node;
}

/* ======================================================================= */
