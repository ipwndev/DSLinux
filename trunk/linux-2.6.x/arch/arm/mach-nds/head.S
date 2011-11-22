/****************************************************************************/

/*
 *	linux/arch/arm/mach-nds/head.S
 *
 *	Copyright (C) 2005, Malcolm Parsons <pepsiman@blueyonder.co.uk>
 *
 *  uClinux kernel startup code for NDS
 *  which has no proper bootloader for linux startup
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
#include <asm/system.h>
#include <asm/mach/arch.h>
#include <asm/thread_info.h>
#include <asm/arch/gbarom-macro.S>

/* Copied here from another machine type.
 * All arm machines have the same values for these. (stsp) */
#define MACHINFO_TYPE		0
#define MACHINFO_PHYSRAM	4
#define MACHINFO_PHYSIO		8
#define MACHINFO_PGOFFIO	12
#define MACHINFO_NAME		16

/****************************************************************************/

#define PAGE_4K		(0b01011 << 1)
#define PAGE_8K		(0b01100 << 1)
#define PAGE_16K	(0b01101 << 1)
#define PAGE_32K	(0b01110 << 1)
#define PAGE_64K	(0b01111 << 1)
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

/*
 *	Start address of the DTCM Memory.
 */
#define DTCM_BASE	0x027C0000	/* NDS DTCM standard address */


/****************************************************************************/

/*
 * Kernel startup entry point.
 */
	__INIT
	.type	stext, #function
ENTRY(stext)

#ifdef CONFIG_XIP_KERNEL
	/* GBA has header info at start of ROM */
	b switch_to_arm9
	.long 0,0,0,0,0,0,0,0
	.long 0,0,0,0,0,0,0,0
	.long 0,0,0,0,0,0,0,0
	.long 0,0,0,0,0,0,0,0
	.long 0,0,0,0,0,0,0

	.ascii "DS Linux 2.6"
	.ascii "PASS"
	.long 0,0,0,0

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
	ldr r1, =0x03800000
	ldr r2, =arm7_end
	subs r2, r2, r0
	add r2, r2, r1

_copyarm7:
	cmp	r1, r2				/* All copied? */
	ldrcc	r3, [r0], #4
	strcc	r3, [r1], #4			/* Copy next word */
	bcc	_copyarm7			/* Keep going till done */

	/* jump to ARM7 program */
	ldr pc, =0x03800000

#endif


/****************************************************************************/

_entry:
#if 0
	/* Map VRAM so the screen goes black and we can do palette debugging */
	ldr r1, =0x04000000
	ldr r0, =0x00010100
	str r0, [r1]
	ldr r1, =0x04000240
	ldr r0, =0x81
	strb r0, [r1]
#endif
#if 0
	/* does not work for some reason */
	/* Palette debugging - draw a pixel with colour != [black|white] */
	ldr r1, =0x05000000
	ldr r0, =0x05
	strh r0, [r1]
#endif

	mov	r0, #PSR_F_BIT | PSR_I_BIT | MODE_SVC	@ make sure svc mode
	msr	cpsr_c, r0			@ and all irqs disabled

	mov	r0, #0x04000000
	mov	r1, #0				@ enable arm9 iwram
	strb	r1,[r0, #0x247]

	ldr	r1, =0x00002078			@ disable DTCM and protection unit
	mcr	p15, 0, r1, c1, c0

	@---------------------------------------------------------------------------------
	@ allocate GBA+Main Memory to ARM9
	@ (Setting wait_cr here allows init of card specific registers here)
	@---------------------------------------------------------------------------------
	ldr	r0, =0x04000204			@ wait_cr
	@ bit  7: GBA  Slot allocated to ARM9
	@ bit 11: DS   Slot allocated to ARM9
	@ bit 15: Main Memory priority to ARM9
	ldrh	r1,[r0]
	bic	r1, r1, #0x0080
	bic	r1, r1, #0x8800
	strh	r1,[r0]

	@---------------------------------------------------------------------------------
	@ Protection Unit Setup added by Sasq
	@---------------------------------------------------------------------------------
	@ Disable cache
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ Flush Instruction cache
	mcr	p15, 0, r0, c7, c6, 0		@ Flush Data cache

	@ Wait for write buffer to empty
	mcr	p15, 0, r0, c7, c10, 4

	ldr	r0, =(DTCM_BASE | 0x0A)         @ set DTCM base and size (16K)
	mcr	p15, 0, r0, c9, c1

	@-------------------------------------------------------------------------
	@ Enable RAM on RAM-based cards.
	@ This should be done with data cache disabled for GBA ROM space.
	@ This has to happen early, because the kernel tries to
        @ use this RAM.
	@-------------------------------------------------------------------------
#ifdef CONFIG_NDS_ROM8BIT
#if defined(CONFIG_MMC_SCSD) || defined(CONFIG_IDE_NDS_SUPERCARD) || defined(CONFIG_NDS_BLK_SCCF)
	sc_set_ram	@ swith to RAM mode, restore @SC_LOCK from R2
#endif
#if defined(CONFIG_MMC_M3SD) || defined(CONFIG_IDE_NDS_M3) || defined(CONFIG_NDS_BLK_M3CF)
	m3_set_ram	@ switch to RAM mode
#endif
#endif

	@-------------------------------------------------------------------------
	@ Setup memory regions
	@-------------------------------------------------------------------------

	@-------------------------------------------------------------------------
	@ Region 0 - ITCM (vectors)
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_32K | 0x00000000 | 1)
	mcr	p15, 0, r0, c6, c0, 0

	@-------------------------------------------------------------------------
	@ Region 1 - Main Memory
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_4M  | 0x02000000 | 1)
	mcr	p15, 0, r0, c6, c1, 0
	
	@-------------------------------------------------------------------------
	@ Region 2 - DTCM
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_16K | DTCM_BASE  | 1)
	mcr	p15, 0, r0, c6, c2, 0

	@-------------------------------------------------------------------------
	@ Region 3 - IWRAM
	@-------------------------------------------------------------------------
	ldr 	r0,=( PAGE_32K | 0x037F8000 | 1)
	mcr 	p15, 0, r0, c6, c3, 0

	@-------------------------------------------------------------------------
	@ Region 4 - I/O (0x04000000) and Kernel VRAM (0x05000000,0x07000000)
	@-------------------------------------------------------------------------
	ldr 	r0,=( PAGE_64M | 0x04000000 | 1)
	mcr 	p15, 0, r0, c6, c4, 0

	@-------------------------------------------------------------------------
	@ Region 5 - User-accessible VRAM (X-Server), overriding Region 4
	@-------------------------------------------------------------------------
	ldr 	r0,=( PAGE_16M | 0x06000000 | 1)
	mcr 	p15, 0, r0, c6, c5, 0

	@-------------------------------------------------------------------------
	@ Region 6 - GBA SLOT ROM for I/O access, including GBA RAM area
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_64M | 0x08000000 | 1)
	mcr	p15, 0, r0, c6, c6, 0

	@-------------------------------------------------------------------------
	@ Region 7 - GBA SLOT ROM for Program+Data, overriding Region 6
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_32M | 0x08000000 | 1)
	mcr	p15, 0, r0, c6, c7, 0

	@-------------------------------------------------------------------------
	@ Write buffer enable
	@-------------------------------------------------------------------------
	@ It is safe for all I/O and memory to enable the write buffer.
	@ (The write buffer is drained before the next read).
	@ TCMs don't use the write buffer.
	@	      gGViWDMI
	ldr	r0,=0b11111010
	mcr	p15, 0, r0, c3, c0, 0

	@-------------------------------------------------------------------------
	@ Data cache enable
	@-------------------------------------------------------------------------
	@ Disable data cache for GBA ROM, unless we have full 8bit write support.
	@	      gGViWDMI
	@ NOTE: this code must be synchronous with include/asm-arm/arch-nds/gbarom-macros.S
	@ and with gbaram.c and with dldi_s.S
#if defined(CONFIG_NDS_ROM8BIT) && !defined(CONFIG_NDS_DLDI) 
	ldr	r0,=0b10000010
#else
	ldr	r0,=0b00000010
#endif
	mcr	p15, 0, r0, c2, c0, 0

	@-------------------------------------------------------------------------
	@ Instruction cache enable
	@-------------------------------------------------------------------------
	@ Instruction cache is on for
	@ - Main Memory
	@ - GBA Slot ROM (for XIP and program executing)
	@	      gGViWDMI
	ldr	r0,=0b10000010
	mcr	p15, 0, r0, c2, c0, 1

	@-------------------------------------------------------------------------
	@ IAccess
	@-------------------------------------------------------------------------
	@ - ITCM: kernel ro
	@ - Main: all ro
	@ - DTCM: none
	@ - IWRAM: none
	@ - IO+VRAM: none
	@ - VRAM: none
	@ - GBA IO: none
	@ - GBA ROM: all ro
	@	      gGViWDMI
	ldr	r0,=0x60000065
	mcr	p15, 0, r0, c5, c0, 3

	@-------------------------------------------------------------------------
	@ DAccess
	@-------------------------------------------------------------------------
	@ - ITCM: kernel r+w (write is later switched off)
	@ - Main: all r+w
	@ - DTCM: all r+w
	@ - IWRAM: kernel r+w
	@ - IO+VRAM: kernel r+w
	@ - VRAM: all r+w
	@ - GBA IO: kernel r+w
	@ - GBA ROM: all r+w
	@	      gGViWDMI
	ldr	r0,=0x31311331
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

#ifdef CONFIG_XIP_KERNEL
	/*  Copy data sections to their new home.  */
	ldr	r1, =_etext			/* Addr of data in FLASH */
	ldr	r2, =__data_start		/* Addr of real RAM data */
	ldr	r3, =_edata			/* Addr of end of data */
_copydata:
	cmp	r2, r3				/* All copied? */
	ldrcc	r4, [r1], #4
	strcc   r4, [r2], #4			/* Copy next word */
	bcc	_copydata			/* Keep going till done */
#endif

	adr	r5, __LC0_init_data
	ldmia	r5, {r5, r6, r8, r9, r10, sp}			@ Setup stack
	bic	r4, r0, #CR_A
	stmia	r10, {r0, r4}

	/*  Clear BSS */
	mov	r4, #0
1:	cmp	r5, r8
	strcc	r4, [r5],#4
	bcc	1b

	/*  Pretend we know what our processor code is (for arm_id)   */

	ldr	r2, =0x41009460
	str	r2, [r6]
	ldr	r2, =MACH_TYPE_NDS
	str	r2, [r9]

	mov fp, #0
	b	start_kernel


	.type	__LC0_init_data, %object
__LC0_init_data:
	.long	__bss_start			@ r5
	.long	processor_id			@ r6
	.long	_end				@ r8
	.long   __machine_arch_type		@ r9
	.long	cr_alignment			@ r10
	.long	init_thread_union+THREAD_START_SP @ sp

/****************************************************************************/

#ifdef CONFIG_XIP_KERNEL

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
.incbin "arch/arm/mach-nds/arm7.bin"
arm7_end:

#endif

#include "../kernel/head-common.S"
