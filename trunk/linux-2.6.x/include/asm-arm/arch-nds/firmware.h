/*
 * include/asm-armnommu/arch-nds/firmware.h
 *
 * Copyright 2006 Stefan Sperling <stsp@stsp.in-berlin.de>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define NDS_FIRMWARE_BLOCK_SIZE	512
struct nds_firmware_block {
	/* data is first, because we want ARM9 cache line alignment */
	u_char data[NDS_FIRMWARE_BLOCK_SIZE];
	loff_t from;
	size_t len;
	u_char *destination;
};

