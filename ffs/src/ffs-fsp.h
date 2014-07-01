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
 * Copyright (c) International Business Machines Corp., 2012
 *
 * FSP Flash Structure, Boot specific adaptations
 */

#ifndef __FFS_BOOT_H__
#define __FFS_BOOT_H__

#include "ffs.h"

/*
 * Values to use in USER_DATA_VOL
 *
 * @note: Side 0/1 must be defined as even/odd values.  Code in the IPL depends
 * on this to be able to use the appropriate volume based on the boot bank.
 */
enum vol {
	FFS_VOL_IPL0       = 0,
	FFS_VOL_IPL1       = 1,
	FFS_VOL_SPL0       = 2,
	FFS_VOL_SPL1       = 3,
	FFS_VOL_BOOTENV0   = 4,
	FFS_VOL_BOOTENV1   = 5,
	FFS_VOL_KERNEL0    = 6,
	FFS_VOL_KERNEL1    = 7,
	FFS_VOL_INITRAMFS0 = 8,
	FFS_VOL_INITRAMFS1 = 9,
	FFS_VOL_DTB0       = 10,
	FFS_VOL_DTB1       = 11,
	FFS_VOL_SERIES0    = 12,
	FFS_VOL_SERIES1    = 13,
	FFS_VOL_CARD0      = 14,
	FFS_VOL_CARD1      = 15,
	FFS_VOL_DUMP0      = 16,
	FFS_VOL_DUMP1      = 17,
	FFS_VOL_DUMP2      = 18,
	FFS_VOL_DUMP3      = 19,
};

#endif /* __FFS_BOOT_H__ */
