/*
 *  linux/arch/arm/kernel/head-common.S
 *
 *  Copyright (C) 1994-2002 Russell King
 *  Copyright (c) 2003 ARM Limited
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Kernel common startup code for all 32-bit CPUs.
 *  It is included in head.S and head-nommu.S
 */

/*
 * Exception handling.  Something went wrong and we can't proceed.  We
 * ought to tell the user, but since we don't have any guarantee that
 * we're even running on the right architecture, we do virtually nothing.
 *
 * If CONFIG_DEBUG_LL is set we try to print out something about the error
 * and hope for the best (useful if bootloader fails to pass a proper
 * machine ID for example).
 */

	.type	__error_p, %function
__error_p:
#ifdef CONFIG_DEBUG_LL
	adr	r0, str_p1
	bl	printascii
	b	__error
str_p1:	.asciz	"\nError: unrecognized/unsupported processor variant.\n"
	.align
#endif

	.type	__error_a, %function
__error_a:
#ifdef CONFIG_DEBUG_LL
	mov	r4, r1				@ preserve machine ID
	adr	r0, str_a1
	bl	printascii
	mov	r0, r4
	bl	printhex8
	adr	r0, str_a2
	bl	printascii
	adr	r3, 3f
	ldmia	r3, {r4, r5, r6}		@ get machine desc list
	sub	r4, r3, r4			@ get offset between virt&phys
	add	r5, r5, r4			@ convert virt addresses to
	add	r6, r6, r4			@ physical address space
1:	ldr	r0, [r5, #MACHINFO_TYPE]	@ get machine type
	bl	printhex8
	mov	r0, #'\t'
	bl	printch
	ldr     r0, [r5, #MACHINFO_NAME]	@ get machine name
	add	r0, r0, r4
	bl	printascii
	mov	r0, #'\n'
	bl	printch
	add	r5, r5, #SIZEOF_MACHINE_DESC	@ next machine_desc
	cmp	r5, r6
	blo	1b
	adr	r0, str_a3
	bl	printascii
	b	__error
str_a1:	.asciz	"\nError: unrecognized/unsupported machine ID (r1 = 0x"
str_a2:	.asciz	").\n\nAvailable machine support:\n\nID (hex)\tNAME\n"
str_a3:	.asciz	"\nPlease check your kernel config and/or bootloader.\n"
	.align
#endif

	.type	__error, %function
__error:
#ifdef CONFIG_ARCH_RPC
/*
 * Turn the screen red on a error - RiscPC only.
 */
	mov	r0, #0x02000000
	mov	r3, #0x11
	orr	r3, r3, r3, lsl #8
	orr	r3, r3, r3, lsl #16
	str	r3, [r0], #4
	str	r3, [r0], #4
	str	r3, [r0], #4
	str	r3, [r0], #4
#endif
1:	mov	r0, r0
	b	1b


/*
 * Read processor ID register (CP#15, CR0), and look up in the linker-built
 * supported processor list.  Note that we can't use the absolute addresses
 * for the __proc_info lists since we aren't running with the MMU on
 * (and therefore, we are not in the correct address space).  We have to
 * calculate the offset.
 *
 * Returns:
 *	r3, r4, r6 corrupted
 *	r5 = proc_info pointer in physical address space
 *	r9 = cpuid
 */
	.type	__lookup_processor_type, %function
__lookup_processor_type:
	adr	r3, 3f
	ldmda	r3, {r5, r6, r9}
	sub	r3, r3, r9			@ get offset between virt&phys
	add	r5, r5, r3			@ convert virt addresses to
	add	r6, r6, r3			@ physical address space
	mrc	p15, 0, r9, c0, c0		@ get processor id
1:	ldmia	r5, {r3, r4}			@ value, mask
	and	r4, r4, r9			@ mask wanted bits
	teq	r3, r4
	beq	2f
	add	r5, r5, #PROC_INFO_SZ		@ sizeof(proc_info_list)
	cmp	r5, r6
	blo	1b
	mov	r5, #0				@ unknown processor
2:	mov	pc, lr

/*
 * This provides a C-API version of the above function.
 */
ENTRY(lookup_processor_type)
	stmfd	sp!, {r4 - r6, r9, lr}
	bl	__lookup_processor_type
	mov	r0, r5
	ldmfd	sp!, {r4 - r6, r9, pc}

/*
 * Look in include/asm-arm/procinfo.h and arch/arm/kernel/arch.[ch] for
 * more information about the __proc_info and __arch_info structures.
 */
	.long	__proc_info_begin
	.long	__proc_info_end
3:	.long	.
	.long	__arch_info_begin
	.long	__arch_info_end

/*
 * Lookup machine architecture in the linker-build list of architectures.
 * Note that we can't use the absolute addresses for the __arch_info
 * lists since we aren't running with the MMU on (and therefore, we are
 * not in the correct address space).  We have to calculate the offset.
 *
 *  r1 = machine architecture number
 * Returns:
 *  r3, r4, r6 corrupted
 *  r5 = mach_info pointer in physical address space
 */
	.type	__lookup_machine_type, %function
__lookup_machine_type:
	adr	r3, 3b
	ldmia	r3, {r4, r5, r6}
	sub	r3, r3, r4			@ get offset between virt&phys
	add	r5, r5, r3			@ convert virt addresses to
	add	r6, r6, r3			@ physical address space
1:	ldr	r3, [r5, #MACHINFO_TYPE]	@ get machine type
	teq	r3, r1				@ matches loader number?
	beq	2f				@ found
	add	r5, r5, #SIZEOF_MACHINE_DESC	@ next machine_desc
	cmp	r5, r6
	blo	1b
	mov	r5, #0				@ unknown machine
2:	mov	pc, lr

/*
 * This provides a C-API version of the above function.
 */
ENTRY(lookup_machine_type)
	stmfd	sp!, {r4 - r6, lr}
	mov	r1, r0
	bl	__lookup_machine_type
	mov	r0, r5
	ldmfd	sp!, {r4 - r6, pc}
