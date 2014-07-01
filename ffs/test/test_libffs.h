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
 *    File: test_libffs.c
 *  Author: Shekar Babu S <shekbabu@in.ibm.com>
 *   Descr: unit test tool for api's in libffs.so
 *    Date: 06/26/2012
 */

#include "libffs.h"

#define FFS_ERROR -1
#define PART_OFFSET 0x3F0000

typedef struct ffs_operations{
	const char  * nor_image;  //!< Path to nor image special/regular file
	const char  * part_entry; //!< Logical partition/entry name
	const char  * i_file;	  //!< Input file
	const char  * o_file;	  //!< Output file
	FILE	    * log;	  //!< Log file
	size_t        device_size;//!< Size of the nor flash
	off_t         part_off;	  //!< Offset of partition table
	size_t        blk_sz;	  //!< Block size
	size_t        entry_sz;	  //!< Partition entry size
	off_t         entry_off;  //!< Offset of partition entry
	uint32_t      user;	  //!< Index to user word in any entry
	uint32_t      value;      //!< User word at index in entry
	ffs_type_t    type;	  //!< Partition type
} ffs_ops_t;
