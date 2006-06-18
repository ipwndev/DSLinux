/****************************************************************************/

/*
 *	linux/arch/armnommu/mach-nds/head.S
 *
 *	Copyright (C) 2005, Malcolm Parsons <pepsiman@blueyonder.co.uk>
 *
 *  uClinux kernel startup code for NDS
 *  which has no proper bootloader for linux startup
 *  because of XIP.
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
 */
#include <linux/config.h>
#include <linux/linkage.h>
#include <linux/init.h>

#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <asm/mach-types.h>
#include <asm/procinfo.h>
#include <asm/ptrace.h>
#include <asm/mach/arch.h>
#include <asm/thread_info.h>

/****************************************************************************/

	.type	disable_writes_to_itcm, #function
ENTRY(disable_writes_to_itcm)
    mov r10, r0

	ldr	r1, =0x00002078			@ disable DTCM and protection unit
	mcr	p15, 0, r1, c1, c0, 0

	@-------------------------------------------------------------------------
	@ Protection Unit Setup added by Sasq
	@-------------------------------------------------------------------------

	@ NOTE: you can not invalidate the data cache here, 
	@ because they are dirty in writeback mode.

	@-------------------------------------------------------------------------
	@ DAccess
	@-------------------------------------------------------------------------
	@ modify only ITCM region 0
	mrc	p15, 0, r0, c5, c0, 2
	ldr	r1,=0xFFFFFFF0
	and	r0,r0,r1
	ldr	r1,=0x00000005
	orr	r0,r0,r1
	mcr	p15, 0, r0, c5, c0, 2

	@-------------------------------------------------------------------------
	@ Enable ICache, DCache, ITCM & DTCM. Set round-robin-replacement
	@-------------------------------------------------------------------------
	mrc	p15, 0, r0, c1, c0, 0
	ldr	r1,=0x55005
	orr	r0,r0,r1
	ldr	r1,=0xfdfff
	and	r0,r0,r1
	mcr	p15, 0, r0, c1, c0, 0

	bx lr

/****************************************************************************/
