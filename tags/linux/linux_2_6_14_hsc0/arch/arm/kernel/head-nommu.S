/*
 *  linux/arch/arm/kernel/head-nommu.S
 *
 *  Copyright (C) 1994-2002 Russell King
 *  Copyright (C) 2003,2004 Hyok S. Choi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Common kernel startup code (non-paged MM)
 *    for 32-bit CPUs which has a process ID register.
 *
 *  note:
 *  For the architectures which has NO process ID register (MMU/MPU),
 *   or which needs some architecture specific setup codes,
 *   Please create a "head.S" in each machine directories.
 */
#include <linux/config.h>
#include <linux/linkage.h>
#include <linux/init.h>

#include <asm/assembler.h>
#include <asm/mach-types.h>
#include <asm/procinfo.h>
#include <asm/ptrace.h>
#include <asm/asm-offsets.h>
#include <asm/system.h>

#define PROCINFO_INITFUNC       8

#define MACHINFO_TYPE		0
#define MACHINFO_PHYSRAM	4
#define MACHINFO_PHYSIO		8
#define MACHINFO_PGOFFIO	12
#define MACHINFO_NAME		16

/*
 * Kernel startup entry point.
 * ---------------------------
 *
 * This is normally called from the decompressor code.  The requirements
 * are: MMU = off, D-cache = off, I-cache = dont care, r0 = 0,
 * r1 = machine nr.
 *
 * See linux/arch/arm/tools/mach-types for the complete list of machine
 * numbers for r1.
 *
 */
	__INIT
	.type	stext, %function
ENTRY(stext)
	msr	cpsr_c, #PSR_F_BIT | PSR_I_BIT | MODE_SVC @ ensure svc mode
						@ and irqs disabled
	bl	__lookup_processor_type		@ r5=procinfo r9=cpuid
	movs	r10, r5				@ invalid processor (r5=0)?
	beq	__error_p			@ yes, error 'p'
	bl	__lookup_machine_type		@ r5=machinfo
	movs	r8, r5				@ invalid machine (r5=0)?
	beq	__error_a			@ yes, error 'a'

	adr	lr, __after_proc_init		@ return (PIC) address
	add	pc, r10, #PROCINFO_INITFUNC

	.type	__LC0_init_data, %object
__LC0_init_data:
	.long	__data_loc			@ r4
	.long	__data_start			@ r5
	.long	__bss_start			@ r6
	.long	_end				@ r7
	.long	processor_id			@ r4
	.long	__machine_arch_type		@ r5
	.long	cr_alignment			@ r6
	.long	init_thread_union+8192		@ sp

/*
 * Set the Control Register and Read the process ID.
 */
	.type	__after_proc_init, %function
__after_proc_init:
	mrc	p15, 0, r0, c1, c0, 0		@ read control reg
#ifdef CONFIG_ALIGNMENT_TRAP
	orr	r0, r0, #CR_A
#else
	bic	r0, r0, #CR_A
#endif
#ifdef CONFIG_CPU_DCACHE_DISABLE
	bic	r0, r0, #CR_C
#endif
#ifdef CONFIG_CPU_BPREDICT_DISABLE
	bic	r0, r0, #CR_Z
#endif
#ifdef CONFIG_CPU_ICACHE_DISABLE
	bic	r0, r0, #CR_I
#endif
	mcr	p15, 0, r0, c1, c0, 0		@ write control reg

/*
 *  r0  = processor control register
 *  r1  = machine ID
 *  r9  = processor ID
 */
	adr	r3, __LC0_init_data

	ldmia	r3!, {r4, r5, r6, r7}
	cmp	r4, r5				@ Copy data segment if needed
1:	cmpne	r5, r6
	ldrne	fp, [r4], #4
	strne	fp, [r5], #4
	bne	1b

	mov	fp, #0				@ Clear BSS (and zero fp)
1:	cmp	r6, r7
	strcc	fp, [r6],#4
	bcc	1b

	ldmia	r3, {r4, r5, r6, sp}
	str	r9, [r4]			@ Save processor ID
	str	r1, [r5]			@ Save machine type
	bic	r4, r0, #CR_A			@ Clear 'A' bit
	stmia	r6, {r0, r4}			@ Save control register values
	b	start_kernel

#include "head-common.S"
