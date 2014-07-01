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
 *    File: fcp_misc.h
 *  Author: Shaun Wetzstein <shaun@us.ibm.com>
 *   Descr: Misc. helpers
 *    Date: 05/12/2012
 */

#ifndef __MISC__H__
#define __MISC__H__

#include <sys/types.h>

#include <stdio.h>
#include <regex.h>

#include <clib/list.h>

#include <ffs/libffs.h>

typedef struct entry_list entry_list_t;
struct entry_list {
	list_t list;
	ffs_t * ffs;
};

typedef struct entry_node entry_node_t;
struct entry_node {
	list_node_t node;
	ffs_entry_t entry;
};

extern entry_list_t * entry_list_create(ffs_t *);
extern entry_list_t * entry_list_create_by_regex(ffs_t *, const char *);
extern int entry_list_add(entry_list_t *, ffs_entry_t *);
extern int entry_list_add_child(entry_list_t *, ffs_entry_t *);
extern int entry_list_remove(entry_list_t *, entry_node_t *);
extern int entry_list_delete(entry_list_t *);
extern int entry_list_exists(entry_list_t *, ffs_entry_t *);
extern ffs_entry_t * entry_list_find(entry_list_t *, const char *);
extern int entry_list_dump(entry_list_t *, FILE *);

extern int parse_offset(const char *, off_t *);
extern int parse_size(const char *, size_t *);
extern int parse_number(const char *, size_t *);
extern int parse_path(const char *, char **, char **, char **);

extern int dump_errors(const char *, FILE *);
extern int check_file(const char *, FILE *, off_t);
extern int is_file(const char *, const char *, const char *);
extern int valid_type(const char *);

extern FILE *__fopen(const char *, const char *, const char *, int);

#endif /* __MISC__H__ */
