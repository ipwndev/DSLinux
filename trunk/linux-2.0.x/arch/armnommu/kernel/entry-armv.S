/*
 * linux/arch/armnommu/kernel/entry-armv.S
 *
 * Copyright (C) 1996 Russell King.
 * ARM700 fix by Matthew Godbolt (linux-user@willothewisp.demon.co.uk)
 *
 * Low-level vector interface routines
 *
 * Note:  there is a StrongARM bug in the STMIA rn, {regs}^ instruction
 * that causes it to save wrong values...  Be aware!
 *	
 * NET+ARM specific modifications subject to:
 *
 * Copyright (C) 2001 NETsilicon, Inc.
 * Copyright (C) 2001 Red Hat, Inc.
 *
 * This software is copyrighted by Red Hat. LICENSEE agrees that
 * it will not delete this copyright notice, trademarks or protective
 * notices from any copy made by LICENSEE.
 *
 * This software is provided "AS-IS" and any express or implied 
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall Red Hat
 * be liable for any indirect, consequential, or incidental damages,
 * loss of profits or revenue, loss of use or data, or interruption
 * of business, whether the alleged damages are labeled in contract,
 * tort, or indemnity.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author(s) : Joe deBlaquiere
 */
#include <linux/autoconf.h>
#include <asm/assembler.h>
#include <asm/errno.h>
#include <asm/hardware.h>

#ifdef	CONFIG_ARCH_NETARM
#include <asm/arch/netarm_registers.h>
#endif

#define current current_set

#ifdef IOC_BASE
/* IOC / IOMD based hardware */
		.equ	ioc_base_high, IOC_BASE & 0xff000000
		.equ	ioc_base_low, IOC_BASE & 0x00ff0000
		.macro	disable_fiq
		mov	r12, #ioc_base_high
		.if	ioc_base_low
		orr	r12, r12, #ioc_base_low
		.endif
		strb	r12, [r12, #0x38]	@ Disable FIQ register
		.endm

		.macro	get_irqnr_and_base, irqnr, base
		mov	r4, #ioc_base_high		@ point at IOC
		.if	ioc_base_low
		orr	r4, r4, #ioc_base_low
		.endif
		ldrb	\irqnr, [r4, #0x24]		@ get high priority first
		adr	\base, irq_prio_h
		teq	\irqnr, #0
#ifdef IOMD_BASE
		ldreqb	\irqnr, [r4, #0x1f4]		@ get dma
		adreq	\base, irq_prio_d
		teqeq	\irqnr, #0
#endif
		ldreqb	\irqnr, [r4, #0x14]		@ get low priority
		adreq	\base, irq_prio_l
		.endm

/*
 * Interrupt table (incorporates priority)
 */
		.macro	irq_prio_table
irq_prio_l:	.byte	 0, 0, 1, 0, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3
		.byte	 4, 0, 1, 0, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3
		.byte	 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
		.byte	 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
		.byte	 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3, 3, 3, 3
		.byte	 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3, 3, 3, 3, 3
		.byte	 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
		.byte	 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
		.byte	 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
		.byte	 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
		.byte	 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
		.byte	 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
		.byte	 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
		.byte	 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
		.byte	 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
		.byte	 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
#ifdef IOMD_BASE
irq_prio_d:	.byte	 0,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	20,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	21,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	21,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	22,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	22,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	21,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	21,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	23,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	23,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	21,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	21,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	22,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	22,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	21,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
		.byte	21,16,17,16,18,16,17,16,19,16,17,16,18,16,17,16
#endif
irq_prio_h:	.byte	 0, 8, 9, 8,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	12, 8, 9, 8,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	13,13,13,13,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	13,13,13,13,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	14,14,14,14,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	14,14,14,14,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	13,13,13,13,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	13,13,13,13,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	15,15,15,15,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	15,15,15,15,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	13,13,13,13,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	13,13,13,13,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	15,15,15,15,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	15,15,15,15,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	13,13,13,13,10,10,10,10,11,11,11,11,10,10,10,10
		.byte	13,13,13,13,10,10,10,10,11,11,11,11,10,10,10,10
		.endm

#elif defined(CONFIG_ARCH_EBSA110)

		.macro	disable_fiq
		.endm

		.macro	get_irqnr_and_base, irqnr, base
		mov	r4, #0xf3000000
		ldrb	\irqnr, [r4]			@ get interrupts
		adr	\base, irq_prio_ebsa110
		.endm

		.macro	irq_prio_table
irq_prio_ebsa110:
		.byte	 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 1, 1, 2, 2, 2, 2
		.byte	 4, 4, 1, 1, 2, 2, 2, 2, 3, 3, 1, 1, 2, 2, 2, 2
		.byte	 5, 5, 1, 1, 2, 2, 2, 2, 3, 3, 1, 1, 2, 2, 2, 2
		.byte	 5, 5, 1, 1, 2, 2, 2, 2, 3, 3, 1, 1, 2, 2, 2, 2

		.byte	 6, 6, 6, 6, 2, 2, 2, 2, 3, 3, 6, 6, 2, 2, 2, 2
		.byte	 6, 6, 6, 6, 2, 2, 2, 2, 3, 3, 6, 6, 2, 2, 2, 2
		.byte	 6, 6, 6, 6, 2, 2, 2, 2, 3, 3, 6, 6, 2, 2, 2, 2
		.byte	 6, 6, 6, 6, 2, 2, 2, 2, 3, 3, 6, 6, 2, 2, 2, 2

		.byte	 7, 0, 1, 1, 2, 2, 2, 2, 3, 3, 1, 1, 2, 2, 2, 2
		.byte	 4, 4, 1, 1, 2, 2, 2, 2, 3, 3, 1, 1, 2, 2, 2, 2
		.byte	 5, 5, 1, 1, 2, 2, 2, 2, 3, 3, 1, 1, 2, 2, 2, 2
		.byte	 5, 5, 1, 1, 2, 2, 2, 2, 3, 3, 1, 1, 2, 2, 2, 2

		.byte	 6, 6, 6, 6, 2, 2, 2, 2, 3, 3, 6, 6, 2, 2, 2, 2
		.byte	 6, 6, 6, 6, 2, 2, 2, 2, 3, 3, 6, 6, 2, 2, 2, 2
		.byte	 6, 6, 6, 6, 2, 2, 2, 2, 3, 3, 6, 6, 2, 2, 2, 2
		.byte	 6, 6, 6, 6, 2, 2, 2, 2, 3, 3, 6, 6, 2, 2, 2, 2
		.endm

#elif defined(CONFIG_ARCH_TRIO)

		.macro	disable_fiq
		.endm

		.macro	get_irqnr_and_base, irqnr, base
		ldr r4, =AIC_IVR
		ldr	\irqnr, [r4]			@ get interrupts
		bic	\irqnr, \irqnr, #0xffffffe0
		adr	\base, irq_prio_trio
		.endm
		
		.macro	irq_prio_table
irq_prio_trio:
		.byte   0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
		.endm

#elif defined(CONFIG_ARCH_ATMEL)

		.macro	disable_fiq
		.endm

		.macro	get_irqnr_and_base, irqnr, base
		ldr	r4, =AIC_IVR
		ldr	\irqnr, [r4]			@ get interrupts
		bic	\irqnr, \irqnr, #0xffffffe0
		ldr     r0, =AIC_IMR
		ldr	r1, [r0]
		mov 	r4, \irqnr
		mov	r0, #1
		ands 	r0, r1, r0, lsl r4
		bne	1f	
		mov 	\irqnr, #0
1:
		adr	\base, irq_prio_atmel
		.endm
		
		.macro	irq_prio_table
irq_prio_atmel:
		.byte   0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
		.endm

#elif defined(CONFIG_ARCH_NETARM)

		.macro	disable_fiq
		.endm

		.macro	get_irqnr_and_base, irqnr, base
		ldr 	\irqnr, =(NETARM_GEN_MODULE_BASE+NETARM_GEN_INTR_STATUS_EN)


		ldr	\base, [\irqnr, #0]	@ stash ISTATUS
		mov	\irqnr, #0
1:		
		cmp	\base, #0		@ no flags set ??? 
		beq	2f
		tst	\base, #1		@ lsb set ??? 
		addeq	\irqnr, \irqnr, #1	@ if not, incr IRQ#
		moveq	\base, \base, LSR #1	@ r = r >> 1 
		beq	1b
2:
		cmp	\irqnr, #32		@ IRQ# too big ???
		blge	_netarm_led_FAIL2
		adr	\base, irq_prio_netarm
		.endm
		
		.macro	irq_prio_table
irq_prio_netarm:
		.byte   0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
		.byte   16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
		.endm

#elif defined(CONFIG_ARCH_GBA)

		.macro	disable_fiq
		.endm

		.macro	get_irqnr_and_base, irqnr, base
		ldr	r4, =0x04000200			/* Get IE/IF reg */
		ldr	r0, [r4]
		and	r0, r0, r0, lsr #16		/* IE & IF */

		/* This is grossly inefficient, I dis-own it :-) */
		mov	\irqnr, #0			/* default IRQ=0 */
		tst	r0, #0x0001			/* VBLANK intr */
		movne	\irqnr, #1
		tst	r0, #0x0002			/* HBLANK intr */
		movne	\irqnr, #2
		tst	r0, #0x0004			/* YTRIG intr */
		movne	\irqnr, #3
		tst	r0, #0x0008			/* TIMER0 intr */
		movne	\irqnr, #4
		tst	r0, #0x0010			/* TIMER1 intr */
		movne	\irqnr, #5
		tst	r0, #0x0020			/* TIMER2 intr */
		movne	\irqnr, #6
		tst	r0, #0x0040			/* TIMER3 intr */
		movne	\irqnr, #7
		tst	r0, #0x0080			/* COMMS intr */
		movne	\irqnr, #8
		tst	r0, #0x0100			/* DMA0 intr */
		movne	\irqnr, #9
		tst	r0, #0x0200			/* DMA1 intr */
		movne	\irqnr, #10
		tst	r0, #0x0400			/* DMA2 intr */
		movne	\irqnr, #11
		tst	r0, #0x0800			/* DMA3 intr */
		movne	\irqnr, #12
		tst	r0, #0x1000			/* KEYPAD intr */
		movne	\irqnr, #13
		tst	r0, #0x2000			/* CART intr */
		movne	\irqnr, #14

		adr	\base, irq_prio_gba
		.endm
		
		.macro	irq_prio_table
irq_prio_gba:
		.byte   0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18
		.endm

#else
#error Unknown architecture
#endif

		.text

@ Offsets into task structure
@ ---------------------------
@
#define STATE		0
#define COUNTER		4
#define PRIORITY	8
#define SIGNAL		12
#define BLOCKED		16
#define FLAGS		20
#define ERRNO		24

#define PF_TRACESYS	0x20

@ Bad Abort numbers
@ -----------------
@
#define BAD_PREFETCH	0
#define BAD_DATA	1
#define BAD_ADDREXCPTN	2
#define BAD_IRQ		3
#define BAD_UNDEFINSTR	4

@ Number of syscalls accepted
@
#define NSYS_CALL	142

@ OS version number used in SWIs
@  RISC OS is 0
@  RISC iX is 8
@
#define OS_NUMBER	9

@
@ Stack format (ensured by USER_* and SVC_*)
@
#define S_FRAME_SIZE	72
#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52
#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#include "../lib/constants.h"

		.global	_ret_from_sys_call,ret_from_sys_call

/*
 * Low-level interface code
 *
 * Trap initialisation
 *
 * Note - FIQ code has changed.  The default is a couple of words in 0x1c, 0x20
 * that call _unexp_fiq.  Nowever, we now copy the FIQ routine to 0x1c (removes
 * some excess cycles).
 *
 * What we need to put into 0-0x1c are ldrs to branch to 0xC0000000
 * (the kernel).
 * 0x1c onwards is reserved for FIQ, so I think that I will allocate 0xe0 
 * onwards for the actuall address to jump to.
 *
 */

/*
 * these go into 0x00
 */
Lbranches:	swi	SYS_ERROR0
		ldr	pc, Lbranches + 0xe4
		ldr	pc, Lbranches + 0xe8
		ldr	pc, Lbranches + 0xec
		ldr	pc, Lbranches + 0xf0
		ldr	pc, Lbranches + 0xf4
		ldr	pc, Lbranches + 0xf8
		ldr	pc, . + 4
		.word	_unexp_fiq
/*
 * this is put into 0xe0 and above
 */
jump_addresses:	.word	0
		.word	vector_undefinstr
		.word	vector_swi
		.word	vector_prefetch
		.word	vector_data
		.word	vector_addrexcptn
		.word	vector_IRQ
/*
 * initialise the trap system
 */
		.global	_trap_init,trap_init

trap_init:
_trap_init:	stmfd	sp !,{r4 - r9, lr}		@ Save link register
		mrs	r0, cpsr
		bic	r0, r0, #31
		orr	r0, r0, #0xd3
		msr	cpsr, r0
		mov	r0, #0xe0
#ifdef CONFIG_ARCH_GBA
		add	r0, r0, #0x03000000		@ SRAM for GBA
#endif
		adr	r1, jump_addresses
		ldmia	r1, {r1 - r7}
		stmia	r0, {r1 - r7}
		mov	r0, #0				@ Lowest location
#ifdef CONFIG_ARCH_GBA
		add	r0, r0, #0x03000000		@ SRAM for GBA
#endif
		adr	r1, Lbranches
		ldmia	r1, {r1 - r9}
		stmia	r0, {r1 - r9}			@ Save all into page 0 ram
		ldmfd	sp!, {r4 - r9, pc}
/*
 * Undefined FIQs
 *	
 * Enter in FIQ mode, spsr = ANY CPSR, lr = ANY PC
 * MUST PRESERVE SVC SPSR, but need to switch to SVC mode to show our msg.
 * Basically to switch modes, we *HAVE* to clobber one register...  
 * brain damage alert!
 * I don't think that we can execute any code in here in any other mode than
 * FIQ...  Ok
 * you can switch to another mode, but you can't get out of that mode without
 * clobbering one register.
 *
 */
_unexp_fiq:	disable_fiq
		subs	pc, lr, #4
/*
 * Interrupt entry dispatcher - dispatches it to the correct handler for the
 * processor mode
 *
 * Enter in IRQ mode, spsr = SVC/USR CPSR, lr = SVC/USR PC
 *
 */
		.global vector_IRQ,vector_undefinstr,vector_data,vector_prefetch,vector_swi


LCirq:		.word	__temp_irq
vector_IRQ:	@
		@ save mode specific registers
		@
		ldr	r13, LCirq
		sub	lr, lr, #4
		str	lr, [r13]			@ save lr_IRQ
		mrs	lr, spsr
		str	lr, [r13, #4]			@ save spsr_IRQ
		@
		@ now branch to the relevent MODE handling routine
		@
		mrs	sp, cpsr			@ switch to SVC mode
		bic	sp, sp, #31
		orr	sp, sp, #0x13
		msr	spsr, sp
		and	lr, lr, #15
		cmp	lr, #4
		addlts	pc, pc, lr, lsl #2		@ Changes mode and branches
		b	__irq_invalid			@  4 - 15
		b	__irq_usr			@  0  (USR_26 / USR_32)
		b	__irq_invalid			@  1  (FIQ_26 / FIQ_32)
		b	__irq_invalid			@  2  (IRQ_26 / IRQ_32)
		b	__irq_svc			@  3  (SVC_26 / SVC_32)
/*
 * Undef instr entry dispatcher - dispatches it to the correct handler for
 * the processor mode
 *
 * Enter in UND mode, spsr = SVC/USR CPSR, lr = SVC/USR PC
 *
 */
LCund:		.word	__temp_und
vector_undefinstr:
		@
		@ save mode specific registers
		@
		ldr	r13, [pc, #LCund - . - 8]
		str	lr, [r13]
		mrs	lr, spsr
		str	lr, [r13, #4]
		@
		@ now branch to the relevent MODE handling routine
		@
		mrs	sp, cpsr
		bic	sp, sp, #31
		orr	sp, sp, #0x13
		msr	spsr, sp
		and	lr, lr, #15
		cmp	lr, #4
		addlts	pc, pc, lr, lsl #2		@ Changes mode and branches
		b	__und_invalid			@  4 - 15
		b	__und_usr			@  0 (USR_26 / USR_32)
		b	__und_invalid			@  1 (FIQ_26 / FIQ_32)
		b	__und_invalid			@  2 (IRQ_26 / IRQ_32)
		b	__und_svc			@  3 (SVC_26 / SVC_32)
/*
 * Prefetch abort dispatcher - dispatches it to the correct handler for the
 * processor mode
 *
 * Enter in ABT mode, spsr = USR CPSR, lr = USR PC
 *
 */
LCabt:		.word	__temp_abt
vector_prefetch:
		@
		@ save mode specific registers
		@
		sub	lr, lr, #4
		ldr	r13, LCabt
		str	lr, [r13]
		mrs	lr, spsr
		str	lr, [r13, #4]
		@
		@ now branch to the relevent MODE handling routine
		@
		mrs	sp, cpsr
		bic	sp, sp, #31
		orr	sp, sp, #0x13
		msr	spsr, sp
		and	lr, lr, #15
		cmp	lr, #4
		addlts	pc, pc, lr, lsl #2		@ Changes mode and branches
		b	__pabt_invalid			@  4 - 15
		b	__pabt_usr			@  0  (USR_26 / USR_32)
		b	__pabt_invalid			@  1  (FIQ_26 / FIQ_32)
		b	__pabt_invalid			@  2  (IRQ_26 / IRQ_32)
		b	__pabt_invalid			@  3  (SVC_26 / SVC_32)
/*
 * Data abort dispatcher - dispatches it to the correct handler for the
 * processor mode
 *
 * Enter in ABT mode, spsr = USR CPSR, lr = USR PC
 *
 */
vector_data:	@
		@ save mode specific registers
		@
		sub	lr, lr, #8
		ldr	r13, LCabt
		str	lr, [r13]
		mrs	lr, spsr
		str	lr, [r13, #4]
		@
		@ now branch to the relevent MODE handling routine
		@
		mrs	sp, cpsr
		bic	sp, sp, #31
		orr	sp, sp, #0x13
		msr	spsr, sp
		and	lr, lr, #15
		cmp	lr, #4
		addlts	pc, pc, lr, lsl #2		@ Changes mode & branches
		b	__dabt_invalid			@  4 - 15
		b	__dabt_usr			@  0  (USR_26 / USR_32)
		b	__dabt_invalid			@  1  (FIQ_26 / FIQ_32)
		b	__dabt_invalid			@  2  (IRQ_26 / IRQ_32)
		b	__dabt_svc			@  3  (SVC_26 / SVC_32)
/*
 * SWI handler
 *
 * We now handle sys-call tracing, and the errno in the task structure.
 * Still have a problem with >4 arguments for functions.  Theres only
 * a couple of functions in the code that have 5 arguments, so Im not
 * too worried.
 */

#include "calls.S"

LC1:		.word	current
/*
 * Enter in SVC mode, spsr_all = OLD_USER_cpsr_all, R14 = OLD_USER_PC
 */
vector_swi:	sub	sp, sp, #S_FRAME_SIZE
		stmia	sp, {r0 - r12}			@ Calling r0 - r12
		add	r8, sp, #S_PC
		stmdb	r8, {sp, lr}^			@ Calling sp, lr
		mov	r7, r0
		mrs	r6, spsr
		mov	r5, lr
		stmia	r8, {r5, r6, r7}		@ Save calling PC, CPSR, OLD_R0
		mov	fp, #0
		ldr	r6, [lr, #-4]!			@ get swi instruction
		and	r5, r6, #0x0f000000
		teq	r5, #0x0f000000
		bne	Larm700bug
		mrs	r5, cpsr			@ enable irqs
		bic	r5, r5, #I_BIT
		msr	cpsr, r5
		bic	r6, r6, #0xff000000		@ mask off swi op code
		eor	r6, r6, #OS_NUMBER << 20	@ check OS number
Lretry:		cmp	r6, #NR_SYSCALLS		@ check upper syscall limit
		bcs	Lswi_bad_call

		ldr	r5, [pc, #LC1 - . - 8]
		ldr	r5, [r5]
		mov	ip, #0				@ zero errno
		str	ip, [r5, #ERRNO]

		ldr	ip, [r5, #FLAGS]		@ check for syscall tracing
		tst	ip, #PF_TRACESYS
		bne	Ltrace_this_syscall

		adr	ip, _sys_call_table
		mov	r9, sp				@ hack for routines needing > 4 values
		str	r4, [sp, #-4]!			@ new-style: (r0 = arg1, r4 = arg5)
		mov	lr, pc
		ldr	pc, [ip, r6, lsl #2]		@ call sys routine
		add	sp, sp, #4

		ldr	ip, [r5, #ERRNO]		@ check errno
		rsbs	ip, ip, #0
		movne	r0, ip
		str	r0, [sp, #S_R0]			@ returned r0
		b	_ret_from_sys_call

Ltrace_this_syscall:
		ldr	r7, [sp, #S_IP]
		mov	r0, #0
		str	r0, [sp, #S_IP]
		bl	syscall_trace			@ trace entry [IP must = 0]
		str	r7, [sp, #S_IP]
		ldmia	sp, {r0 - r3}			@ have to reload r0 - r3

		adr	ip, _sys_call_table
		mov	r9, sp				@ hack for routines needing > 4 values
		str	r4, [sp, #-4]!			@ new-style: (r0 = arg1, r4 = arg5)
		mov	lr, pc
		ldr	pc, [ip, r6, lsl #2]		@ call sys routine
		add	sp, sp, #4

		ldr	ip, [r5, #ERRNO]
		rsbs	ip, ip, #0
		movne	r0, ip
		str	r0, [sp, #S_R0]			@ returned r0

		mov	r0, #1
		str	r0, [sp, #S_IP]
		bl	syscall_trace			@ trace exit [IP must != 0]
		str	r7, [sp, #S_IP]
		b	_ret_from_sys_call

Lswi_bad_call:	tst	r6, #0x00f00000
		bne	Lbad
		cmp	r6, #(KSWI_SYS_BASE - KSWI_BASE) @ check for arm private syscalls
		bcs	Larm_sys_call
		bl	sys_ni_syscall
		str	r0, [sp, #0]			@ returned r0
		b	_ret_from_sys_call

Lbad:		eor	r0, r6, #OS_NUMBER << 20	@ Put OS number back
		mov	r1, sp
		bl	deferred
		ldmfd	sp, {r0 - r3}
		b	_ret_from_sys_call

Larm_sys_call:	bic	r0, r6, #0x000f0000
		mov	r1, sp
		bl	arm_syscall
		b	_ret_from_sys_call

@ r0 = syscall number
@ r1 = syscall r0
@ r5 = syscall r4
@ ip = syscall table
sys_syscall:	mov	r6, r0
		eor	r6, r6, #OS_NUMBER << 20
		cmp	r6, #NR_SYSCALLS		@ check range
		movgt	r0, #-ENOSYS
		movgt	pc, lr
		add	sp, sp, #4			@ take of the save of our r4
		ldmib	sp, {r0 - r4}			@ get our args
		str	r4, [sp, #-4]!			@ Put our arg on the stack
		ldr	pc, [ip, r6, lsl #2]

Larm700bug:	str	lr, [r8]
		ldr	r0, [sp, #S_PSR]		@ Get calling cpsr
		msr	spsr, r0
		ldmia	sp, {r0 - lr}^			@ Get calling r0 - lr
		mov	r0, r0
		add	sp, sp, #S_PC
		ldr	lr, [sp], #S_FRAME_SIZE - S_PC	@ Get PC and jump over PC, PSR, OLD_R0
		movs	pc, lr

		.globl	_sys_call_table,sys_call_table
sys_call_table:
_sys_call_table:
#include "calls.S"

/*
 * Undefined instruction handler
 */
LC2:		.word	last_task_used_math
		.word	current
		.word	fp_enter

__und_usr:	sub	sp, sp, #S_FRAME_SIZE		@ Allocate frame size in one go
		stmia	sp, {r0 - r12}			@ Save r0 - r12
		add	r8, sp, #S_PC
		stmdb	r8, {sp, lr}^			@ Save user r0 - r12
		ldr	r4, [pc, #LCund - . - 8]
		ldmia	r4, {r5 - r7}
		stmia	r8, {r5 - r7}			@ Save USR pc, cpsr, old_r0

		adr	r1, LC2
		ldmia	r1, {r1, r2, r4}
		ldr	r1, [r1]
		ldr	r2, [r2]
		teq	r1, r2
		blne	math_state_restore
		adr	r9, _fpreturn
		adr	lr, _fpundefinstr
		ldr	pc, [r4]			@ Call FP module USR entry point

		.global	_fpreturn,fpreturn
		.global	_fpundefinstr,fpundefinstr
fpundefinstr:
_fpundefinstr:	mov	r0, lr				@ Called by FP module on undefined instr
		mov	r1, sp
		mrs	r4, cpsr			@ Enable interrupts
		bic	r4, r4, #I_BIT
		msr	cpsr, r4
		bl	do_undefinstr
fpreturn:
_fpreturn:	b	_ret_from_sys_call		@ Normal FP exit

__und_svc:	sub	sp, sp, #S_FRAME_SIZE
		stmia	sp, {r0 - r12}			@ save r0 - r12
		mov	r6, lr
		mov	fp, #0
		ldr	r7, [pc, #LCund - . - 8]
		ldmia	r7, {r7 - r9}
		add	r5, sp, #S_FRAME_SIZE
		add	r4, sp, #S_SP
		stmia	r4, {r5 - r9}			@ save sp_SVC, lr_SVC, pc, cpsr, old_ro

		adr	r1, LC2
		ldmia	r1, {r1, r2, r4}
		ldr	r1, [r1]
		ldr	r2, [r2]
		teq	r1, r2
		blne	math_state_restore
		adr	r9, _fpreturnsvc
		adr	lr, _fpundefinstrsvc
		ldr	pc, [r4]			@ Call FP module SVC entry point

		.globl	_fpreturnsvc,fpreturnsvc
		.globl	_fpundefinstrsvc,fpundefinstrsvc

fpundefinstrsvc:
_fpundefinstrsvc:
		mov	r0, r5				@ unsigned long pc
		mov	r1, sp				@ struct pt_regs *regs
		bl	do_undefinstr
fpreturnsvc:
_fpreturnsvc:	ldr	lr, [sp, #S_PSR]		@ Get SVC cpsr
		msr	spsr, lr
		ldmia	sp, {r0 - pc}^			@ Restore SVC registers

/* We get here if an undefined instruction happens and the floating
 * point emulator is not present.  If the offending instruction was
 * a WFS, we just perform a normal return as if we had emulated the
 * operation.  This is a hack to allow some basic userland binaries
 * to run so that the emulator module proper can be loaded. --philb
 */
fpe_not_present:
		adr	r10, wfs_mask_data
		ldmia	r10, {r4, r5, r6, r7, r8}
		ldr	r10, [sp, #S_PC]		@ Load PC
		sub	r10, r10, #4
		ldrt	r10, [r10]			@ get instruction
		and	r5, r10, r5
		teq	r5, r4				@ Is it WFS?
		moveq	pc, r9
		and	r5, r10, r8
		teq	r5, r6				@ Is it LDF/STF on sp or fp?
		teqne	r5, r7
		movne	pc, lr
		tst	r10, #0x00200000		@ Does it have WB
		moveq	pc, r9
		and	r4, r10, #255			@ get offset
		and	r6, r10, #0x000f0000
		tst	r10, #0x00800000		@ +/-
		rsbeq	r4, r4, #0
		ldr	r5, [sp, r6, lsr #14]		@ Load reg
		add	r5, r5, r4, lsl #2
		str	r5, [sp, r6, lsr #14]		@ Save reg
		mov	pc, r9

wfs_mask_data:	.word	0x0e200110			@ WFS
		.word	0x0fff0fff
		.word	0x0d0d0100			@ LDF [sp]/STF [sp]
		.word	0x0d0b0100			@ LDF [fp]/STF [fp]
		.word	0x0f0f0f00


LC3:		.word	_fp_save
   	        .word	_fp_restore


/*
 * Function to call when switching tasks to save FP state
 */
		.global	_fpe_save,fpe_save
fpe_save:
_fpe_save:	ldr	r1, [pc, #LC3 - . - 8]
		ldr	pc, [r1]

/*
 * Function to call when switching tasks to restore FP state
 */
		.global	_fpe_restore,fpe_restore
fpe_restore:
_fpe_restore:	ldr	r1, [pc, #LC3 - . - 4]
		ldr	pc, [r1]

__und_invalid:	sub	sp, sp, #S_FRAME_SIZE
		stmia	sp, {r0 - lr}
		mov	r7, r0
		ldr	r4, [pc, #LCund - . - 8]
		ldmia	r4, {r5, r6}			@ Get UND/IRQ/FIQ/ABT pc, cpsr
		add	r4, sp, #S_PC
		stmia	r4, {r5, r6, r7}		@ Save UND/IRQ/FIQ/ABT pc, cpsr, old_r0
		mov	r0, sp				@ struct pt_regs *regs
		mov	r1, #BAD_UNDEFINSTR		@ int reason
		and	r2, r6, #31			@ int mode
		b	bad_mode			@ Does not ever return...
/*
 * Prefetch abort handler
 */
pabtmsg:	.ascii	"Pabt: %08lX\n\0"
		.align
__pabt_usr:	sub	sp, sp, #S_FRAME_SIZE		@ Allocate frame size in one go
		stmia	sp, {r0 - r12}			@ Save r0 - r12
		add	r8, sp, #S_PC
		stmdb	r8, {sp, lr}^			@ Save sp_usr lr_usr
		ldr	r4, [pc, #LCabt - . - 8]
		ldmia	r4, {r5 - r7}			@ Get USR pc, cpsr
		stmia	r8, {r5 - r7}			@ Save USR pc, cpsr, old_r0

		mrs	r7, cpsr			@ Enable interrupts if they were
		bic	r7, r7, #I_BIT			@ previously
		msr	cpsr, r7
		mov	r0, r5				@ address (pc)
		mov	r1, sp				@ regs
		bl	do_PrefetchAbort		@ call abort handler
		teq	r0, #0				@ Does this still apply???
		bne	_ret_from_sys_call		@ Return from sys call
#ifdef DEBUG_UNDEF
		adr	r0, t
		bl	_printk
#endif
		mov	r0, r5
		mov	r1, sp
		and	r2, r6, #31
		bl	do_undefinstr
		ldr	lr, [sp, #S_PSR]		@ Get USR cpsr
		msr	spsr, lr
		ldmia	sp, {r0 - pc}^			@ Restore USR registers

__pabt_invalid:	sub	sp, sp, #S_FRAME_SIZE		@ Allocate frame size in one go
		stmia	sp, {r0 - lr}			@ Save XXX r0 - lr
		mov	r7, r0				@ OLD R0
		ldr	r4, [pc, #LCabt - . - 8]
		ldmia	r4, {r5 - r7}			@ Get XXX pc, cpsr
		add	r4, sp, #S_PC
		stmia	r4, {r5 - r7}			@ Save XXX pc, cpsr, old_r0
		mov	r0, sp				@ Prefetch aborts are definitely *not*
		mov	r1, #BAD_PREFETCH		@ allowed in non-user modes.  We cant
		and	r2, r6, #31			@ recover from this problem.
		b	bad_mode

#ifdef DEBUG_UNDEF
t:	.ascii "*** undef ***\r\n\0"
	.align
#endif
/*
 * Address exception handler
 *
 * These aren't too critical.
 * (they're not supposed to happen, and won't happen in 32-bit mode).
 */

vector_addrexcptn:
		b	vector_addrexcptn

/*
 * Interrupt (IRQ) handler (r13 points to irq temp save area)
 * MOD: if in user mode, then *no* kernel routine is running, so dont have to
 *      save svc lr
 */
LC4:		.word	irqjump
__irq_usr:	sub	sp, sp, #S_FRAME_SIZE
		stmia	sp, {r0 - r12}			@ save r0 - r12
		add	r8, sp, #S_PC
		stmdb	r8, {sp, lr}^
		ldr	r4, [pc, #LCirq - . - 8]
		ldmia	r4, {r5 - r7}			@ get saved PC, SPSR
		stmia	r8, {r5 - r7}			@ save pc, psr, old_r0
urepeat:	get_irqnr_and_base r6, r5
#if defined(CONFIG_ARCH_TRIO)
		teq	r6, #0x1f
		bne	has_irq_usr
		ldr	r4,=AIC_EOICR
		eor	r6,r6,r6
		str	r6,[r4]
#elif defined(CONFIG_ARCH_ATMEL)
		teq	r6, #0x0
		bne	has_irq_usr
		ldr	r4,=AIC_EOICR
		eor	r6,r6,r6
		str	r6,[r4]
#else
		teq	r6, #0
		bne	has_irq_usr
#endif
		b	_ret_from_sys_call
has_irq_usr:
		ldrb	r0, [r5, r6]			@ Get IRQ number
		ldr	r2, [pc, #LC4 - . - 8]
		mov	r1, sp
		mov	lr, pc
		@
		@ routine gets called with r0 = interrupt number, r1 = struct pt_regs *
		@
		ldr	pc, [r2, r0, lsl #2]
		mov	r2, #0
		teq	r0, #0				@ Check to see if it is a fast IRQ
		beq	_ret_from_sys_call
		ldr	r0, [sp, #S_PSR]		@ Get saved SPSR
		msr	spsr, r0			@ restore SPSR
		ldmia	sp, {r0 - lr}^			@ Get calling r0 - lr
		mov	r0, r0
		add	sp, sp, #S_PC
		ldr	lr, [sp], #S_FRAME_SIZE - S_PC
		movs	pc, lr

		irq_prio_table

LC5:		.word	intr_count			@ -8
		.word	bh_mask				@ -4
		.word	bh_active			@ -0

__irq_svc:	sub	sp, sp, #S_FRAME_SIZE
		stmia	sp, {r0 - r12}			@ save r0 - r12
		mov	r6, lr
		mov	fp, #0
		ldr	r7, [pc, #LCirq - . - 8]
		ldmia	r7, {r7 - r9}
		add	r5, sp, #S_FRAME_SIZE
		add	r4, sp, #S_SP
		stmia	r4, {r5, r6, r7, r8, r9}	@ save sp_SVC, lr_SVC, pc, cpsr, old_ro
srepeat:	get_irqnr_and_base r6, r5
#if defined(CONFIG_ARCH_TRIO)
		teq	r6, #0x1f
#elif defined(CONFIG_ARCH_ATMEL)
		teq	r6, #0x0
#else
		teq	r6, #0
#endif
		beq	no_irq2
		ldrb	r0, [r5, r6]			@ Get IRQ number
		ldr	r2, [pc, #LC4 - . - 8]
		mov	r1, sp
		mov	lr, pc
		@
		@ routine gets called with r0 = interrupt number, r1 = struct pt_regs *
		@
		ldr	pc, [r2, r0, lsl #2]
		teq	r0, #0				@ Check to see if it was a fast IRQ
#ifndef CONFIG_ARCH_GBA
		bne	srepeat
#endif
		ldr	r4, [pc, #LC5 - . - 8]
		ldr	r5, [r4]
		teq	r5, #0
#ifndef CONFIG_ARCH_GBA
		bne	srepeat
#endif
		ldr	r6, [pc, #LC5 - . - 4]
		ldr	r7, [pc, #LC5 - . - 0]
recheck_bh2:	ldr	r0, [r6]
		ldr	r1, [r7]
		tst	r0, r1
#ifndef CONFIG_ARCH_GBA
		beq	srepeat
#endif
		add	r0, r5, #1
		str	r0, [r4]
		mrs	r8, cpsr			@ Enable interrupts
		bic	lr, r8, #I_BIT
		msr	cpsr, lr
		bl	do_bottom_half
		msr	cpsr, r8			@ Restore interrupt state
		str	r5, [r4]
#ifndef CONFIG_ARCH_GBA
		b	recheck_bh2
#endif
no_irq2:
#if defined(CONFIG_ARCH_TRIO)
		ldr	r4, =AIC_EOICR
		eor	r6,r6,r6
		str	r6,[r4]
#elif defined(CONFIG_ARCH_ATMEL)
		ldr	r4, =AIC_EOICR
		eor	r6,r6,r6
		str	r6,[r4]
#endif
		ldr	r0, [sp, #S_PSR]
		msr	spsr, r0
		ldmia	sp, {r0 - pc}^			@ load r0 - pc, cpsr

__irq_invalid:	sub	sp, sp, #S_FRAME_SIZE	@ Allocate space on stack for frame
		stmfd	sp, {r0 - lr}		@ Save r0 - lr
		mov	r7, #-1
		ldr	r4, [pc, #LCirq - . - 8]
		ldmia	r4, {r5, r6}		@ get saved pc, psr
		add	r4, sp, #S_PC
		stmia	r4, {r5, r6, r7}
		mov	fp, #0
		mov	r0, sp
		mov	r1, #BAD_IRQ
		b	bad_mode
/*
 * Data abort handler code
 */
LCprocfns:	.word	processor

__dabt_usr:	sub	sp, sp, #S_FRAME_SIZE		@ Allocate frame size in one go
		stmia	sp, {r0 - r12}			@ save r0 - r12
		add	r3, sp, #S_PC
		stmdb	r3, {sp, lr}^
		ldr	r0, [pc, #LCabt - . - 8]
		ldmia	r0, {r0 - r2}			@ Get USR pc, cpsr
		stmia	r3, {r0 - r2}			@ Save USR pc, cpsr, old_r0
		mov	fp, #0
		mrs	r2, cpsr			@ Enable interrupts if they were
		bic	r2, r2, #I_BIT			@ previously
		msr	cpsr, r2
		ldr	r2, LCprocfns
		mov	lr, pc
		ldr	pc, [r2, #8]			@ call processor specific code
		mov	r3, sp
#ifdef NO_MM
		cmp	r0, #0				@ check return status
		bne	1f
		ldr	r0, [sp, #S_PSR]
		msr	spsr, r0
		ldmia	sp, {r0 - pc}^			@ load r0 - pc, cpsr
1:
#endif
		bl	do_DataAbort
		b	_ret_from_sys_call

__dabt_svc:	sub	sp, sp, #S_FRAME_SIZE
		stmia	sp, {r0 - r12}			@ save r0 - r12
		ldr	r2, [pc, #LCabt - . - 8]
		add	r0, sp, #S_FRAME_SIZE
		add	r5, sp, #S_SP
		mov	r1, lr
		ldmia	r2, {r2 - r4}			@ get pc, cpsr
		stmia	r5, {r0 - r4}			@ save sp_SVC, lr_SVC, pc, cpsr, old_ro
		tst	r3, #I_BIT
		mrseq	r0, cpsr			@ Enable interrupts if they were
		biceq	r0, r0, #I_BIT			@ previously
		msreq	cpsr, r0
		mov	r0, r2
		ldr	r2, LCprocfns
		mov	lr, pc
		ldr	pc, [r2, #8]			@ call processor specific code
		mov	r3, sp
#ifdef NO_MM
		cmp	r0, #0				@ check return status
		bne	1f
		ldr	r0, [sp, #S_PSR]
		msr	spsr, r0
		ldmia	sp, {r0 - pc}^			@ load r0 - pc, cpsr
1:
#endif
		bl	do_DataAbort
		ldr	r0, [sp, #S_PSR]
		msr	spsr, r0
		ldmia	sp, {r0 - pc}^			@ load r0 - pc, cpsr

__dabt_invalid:	sub	sp, sp, #S_FRAME_SIZE
		stmia	sp, {r0 - lr}			@ Save SVC r0 - lr [lr *should* be intact]
		mov	r7, r0
		ldr	r4, [pc, #LCabt - . - 8]
		ldmia	r4, {r5, r6}			@ Get SVC pc, cpsr
		add	r4, sp, #S_PC
		stmia	r4, {r5, r6, r7}		@ Save SVC pc, cpsr, old_r0
		mov	r0, sp
		mov	r1, #BAD_DATA
		and	r2, r6, #31
		b	bad_mode

/*
 * All exits to user mode from the kernel go through this code.
 */

LC6:
		.word	intr_count		@ -8
		.word	bh_mask		@ -4
		.word	bh_active		@ -0
		.word	need_resched		@ +4
		.word	current		@ +8
		.word	init_task		@ +12

Lreschedule:	bl	schedule

ret_from_sys_call:
_ret_from_sys_call:
		adr	r9, LC6
		ldmia	r9!, {r4, r6, r7}
		ldr	r5, [r4]
		teq	r5, #0
		bne	Lret_no_check
Lrecheck_bh:	ldr	r0, [r6]
		ldr	r1, [r7]
		tst	r0, r1
		bne	Lhandle_bottom_half
		ldr	r0, [sp, #S_PSR]
		tst	r0, #3
		bne	Lret_no_check
		ldmia	r9, {r5, r6, r7}
		ldr	r0, [r5]
		teq	r0, #0
		bne	Lreschedule
		ldr	r0, [r6]
		teq	r0, r7
		beq	Lret_no_check

		ldr	r1, [r0, #SIGNAL]
		ldr	r0, [r0, #BLOCKED]
		bics	r1, r1, r0
		movne	r1, sp
		blne	do_signal
Lret_no_check:	mrs	r0, cpsr			@ disable IRQs
		orr	r0, r0, #I_BIT
		msr	cpsr, r0
		ldr	r0, [sp, #S_PSR]		@ Get calling cpsr
		msr	spsr, r0
		ldmia	sp, {r0 - lr}^			@ Get calling r0 - lr
		mov	r0, r0
		add	sp, sp, #S_PC
		ldr	lr, [sp], #S_FRAME_SIZE - S_PC	@ Get PC and jump over PC, PSR, OLD_R0
		movs	pc, lr

Lhandle_bottom_half:
		add	r0, r5, #1
		str	r0, [r4]
		mrs	r8, cpsr
		bic	lr, r8, #I_BIT
		msr	cpsr, lr
		bl	do_bottom_half
		msr	cpsr, r8
		str	r5, [r4]
		b	Lrecheck_bh

_fpnull:	mov	pc, lr

		.data

		.global	_fp_enter,fp_enter
		.global	_fp_save,fp_save
		.global	_fp_restore,fp_restore

fp_enter:
_fp_enter:	.word	fpe_not_present
		.word	fpe_not_present
fp_save:
_fp_save:	.word	_fpnull
fp_restore:
_fp_restore:	.word	_fpnull

__temp_irq:	.word	0				@ saved lr_irq
		.word	0				@ saved spsr_irq
		.word	-1				@ old_r0
__temp_und:	.word	0				@ Saved lr_und
		.word	0				@ Saved spsr_und
		.word	-1				@ old_r0
__temp_abt:	.word	0				@ Saved lr_abt
		.word	0				@ Saved spsr_abt
		.word	-1				@ old_r0
