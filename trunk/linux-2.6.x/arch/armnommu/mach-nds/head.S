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
#include <asm/mach-types.h>
#include <asm/procinfo.h>
#include <asm/ptrace.h>
#include <asm/mach/arch.h>

/****************************************************************************/

#define PAGE_4K		(0b01011 << 1)
#define PAGE_8K		(0b01100 << 1)
#define PAGE_16K	(0b01101 << 1)
#define PAGE_32K	(0b01110 << 1)
#define PAGE_64K	(0b00111 << 1)
#define PAGE_128K	(0b10000 << 1)
#define PAGE_256K	(0b10001 << 1)
#define PAGE_512K	(0b10010 << 1)
#define PAGE_1M		(0b10011 << 1)
#define PAGE_2M		(0b10100 << 1)
#define PAGE_4M		(0b10101 << 1)
#define PAGE_8M		(0b10110 << 1)
#define PAGE_16M	(0b10111 << 1)
#define PAGE_32M	(0b11000 << 1)
#define PAGE_64M	(0b11001 << 1)
#define PAGE_128M	(0b11010 << 1)
#define PAGE_256M	(0b11011 << 1)
#define PAGE_512M	(0b11100 << 1)
#define PAGE_1G		(0b11101 << 1)
#define PAGE_2G		(0b11110 << 1)
#define PAGE_4G		(0b11111 << 1)

/*
 *	RAM base/size set in kernel config.
 */
#define DRAM_LIMIT	(DRAM_BASE + DRAM_SIZE)

/****************************************************************************/

/*
 * Kernel startup entry point.
 */
	__INIT
	.type	stext, #function
ENTRY(stext)

	/* GBA has header info at start of ROM */
	b switch_to_arm9
	.long 0,0,0,0,0,0,0,0
	.long 0,0,0,0,0,0,0,0
	.long 0,0,0,0,0,0,0,0
	.long 0,0,0,0,0,0,0,0
	.long 0,0,0,0,0,0,0

	.string "DSBooter"
.align
	.long 0,0,0,0,0

switch_to_arm9:
	mov r0, #0
	ldr r1, =0x04000208
	str r0, [r1]
	ldr r1, =0x04000210
	str r0, [r1]

	/* Copy small ARM9 program to RAM */
	ldr r0, =arm9boot_start
	ldr r1, =0x02004000
	ldr r2, =arm9boot_end
	subs r2, r2, r0
	add r2, r2, r1

_copyarm9boot:
	cmp	r1, r2				/* All copied? */
	ldrcc	r3, [r0], #4
	strcc	r3, [r1], #4			/* Copy next word */
	bcc	_copyarm9boot			/* Keep going till done */

	/* copy small ARM7 program to RAM */
	ldr r0, =arm7_start
	ldr r1, =0x037f8000
	ldr r2, =arm7_end
	subs r2, r2, r0
	add r2, r2, r1

_copyarm7:
	cmp	r1, r2				/* All copied? */
	ldrcc	r3, [r0], #4
	strcc	r3, [r1], #4			/* Copy next word */
	bcc	_copyarm7			/* Keep going till done */

	/* jump to ARM7 program */
	ldr pc, =0x037f8000

/****************************************************************************/

_entry:
	/* move this to the framebuffer driver */
	ldr r1, =0x04000000
	ldr r0, =0x00010100
	str r0, [r1]
	ldr r1, =0x04000240
	ldr r0, =0x81
	strb r0, [r1]

	mov	r12, r0
	mov	r0, #PSR_F_BIT | PSR_I_BIT | MODE_SVC	@ make sure svc mode
	msr	cpsr_c, r0			@ and all irqs disabled

	adr	r5, LC0
	ldmia	r5, {r5, r6, r8, r9, sp}			@ Setup stack

	/*  Copy data sections to their new home.  */


	/*  Clear BSS */
	mov	r4, #0
1:	cmp	r5, r8					
	strcc	r4, [r5],#4
	bcc	1b

	/*  Pretend we know what our processor code is (for arm_id)   */

	ldr	r2, L_AT91_SF_CIDR
	ldr	r2, [r2]		@ read processor id

	str	r2, [r6]
	ldr	r2, =MACH_TYPE_NDS
	str	r2, [r9]

#if 0
	ldr	r0, =vector_IRQ
	ldr	r1, =0x00803FFC
	str	r0, [r1]
#endif

	mov fp, #0
	b	start_kernel

LC0:	.long	__bss_start
	.long	processor_id
	.long	_end
	.long   __machine_arch_type
	.long	init_thread_union+8192

L_AT91_SF_CIDR:
	.long	0xfff00000

/****************************************************************************/

.ltorg

arm9boot_start:
	ldr r0, =0xe800
	ldr r1, =0x04000204
	strh r0, [r1]
	ldr r0, =_entry
	bx r0
.ltorg
arm9boot_end:

arm7_start:
	/* Run program on ARM9 */
	ldr r0, =0x02004000
	ldr r1, =0x027FFE24
	str r0, [r1]

	/* busy loop */
	ldr r0, =0xffff
arm7loop:
	swi 0x03
	b arm7loop
.ltorg
arm7_end:

