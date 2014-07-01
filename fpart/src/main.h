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
 *    File: main.h
 *  Author: Shaun Wetzstein <shaun@us.ibm.com>
 *   Descr: cmdline tool for libffs.so
 *    Date: 05/12/2012
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>

#include <ffs/libffs.h>

#define FPART_MAJOR	0x01
#define FPART_MINOR	0x00
#define FPART_PATCH	0x00

typedef enum {
	c_ERROR = 0,
	c_CREATE = 'C',
	c_ADD = 'A',
	c_DELETE = 'D',
	c_ERASE = 'E',
	c_LIST = 'L',
	c_TRUNC = 'T',
	c_USER = 'U',
} cmd_t;

typedef enum {
	o_ERROR = 0,
	o_POFFSET = 'p',
	o_TARGET = 't',
	o_NAME = 'n',
	o_OFFSET = 'o',
	o_SIZE = 's',
	o_BLOCK = 'b',
	o_VALUE = 'u',
	o_FLAGS = 'g',
	o_PAD = 'a',
} option_t;

typedef enum {
	f_ERROR = 0,
	f_FORCE = 'f',
	f_PROTECTED = 'r',
	f_LOGICAL = 'l',
	f_VERBOSE = 'v',
	f_DEBUG = 'd',
	f_HELP = 'h',
} flag_t;

typedef struct {
	const char *short_name;

	/* target */
	const char *path;

	/* command */
	cmd_t cmd;

	/* options */
	const char *name, *target;
	const char *offset, *poffset;
	const char *size, *block;
	const char *user, *value;
	const char *flags, *pad;

	/* flags */
	flag_t force, logical;
	flag_t verbose, debug;
	flag_t protected;

	const char **opt;
	int opt_sz, opt_nr;
} args_t;

typedef struct ffs_entry_node ffs_entry_node_t;
struct ffs_entry_node {
	list_node_t node;
	ffs_entry_t entry;
};

extern args_t args;
extern size_t page_size;

extern int parse_offset(const char *, off_t *);
extern int parse_size(const char *, size_t *);
extern int parse_number(const char *, size_t *);

extern bool check_extension(const char *, const char *);
extern int create_regular_file(const char *, size_t, char);
extern FILE *fopen_generic(const char *, const char *, int);

extern int command(args_t *, int (*)(args_t *, off_t));
extern int verify_operation(const char *, ffs_t *, ffs_entry_t *,
				          ffs_t *, ffs_entry_t *);

extern int command_create(args_t *);
extern int command_add(args_t *);
extern int command_delete(args_t *);
extern int command_list(args_t *);
extern int command_trunc(args_t *);
extern int command_erase(args_t *);
extern int command_user(args_t *);

#endif /* __MAIN_H__ */
