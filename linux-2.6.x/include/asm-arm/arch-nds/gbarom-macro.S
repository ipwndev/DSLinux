/*
 *  include/asm-arm/arch-nds/gbarom-macro.S - GBA ROM switching macros
 *
 *  Copyright (C) 2006 Amadeus, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

/*****************************************************************************/
/* Global interrupt enable register.
   Bit 0: 1 = enabled, 0 = disabled
*/
#define NDS_IME     		0x04000208

/* Size of the ARM9 data cache.
*/
#define CACHE_DSIZE		4096

/* Size of a cache line.
*/
#define CACHE_DLINESIZE		32

/*****************************************************************************/
/*
 * Prefix macro. This macro does all the magic required to switch the 
 * GBA ROM space from RAM mode to IO mode (exclusive the switching itself,
 * which is device dependent).
 * This is a macro and not a function, because the stack(!) may be in GBA
 * ROM space and therefore the stack must not be modified until calling the
 * suffix macro again.
 *
 * Register R0-R2 are not altered and used as Parameter registers.
 * R3 and IP are used and altered. If you need more registers, you
 * must reserve these registers BEFORE calling this macro.
 */
	.macro	prefix
	@ Disable interrupts
	ldr	r3, =NDS_IME		@ interrupt mask register
	mov	ip, #0			@ 0 = disable
	strh	ip, [r3]
	mcr	p15, 0, ip, c7, c10, 4	@ drain write buffer for IME to take effect.
	@ Flush the data cache. Interrupts must be disabled because
  	@ interrupts may load the data cache again.
	mov	r3, #(CACHE_DSIZE/4)	@ div # of segments
1:	sub	r3, r3, #CACHE_DLINESIZE @ next line
	mcr	p15, 0, r3, c7, c14, 2	@ clean and flush a D-cache line
	add	r3, r3, #0x40000000	@ next segment
	mcr	p15, 0, r3, c7, c14, 2	@ clean and flush a D-cache line
	add	r3, r3, #0x40000000	@ next segment
	mcr	p15, 0, r3, c7, c14, 2	@ clean and flush a D-cache line
	add	r3, r3, #0x40000000	@ next segment
	mcr	p15, 0, r3, c7, c14, 2	@ clean and flush a D-cache line
	adds	r3, r3, #0x40000000	@ next segment
	bne	1b			@ next cache line
	mcr	p15, 0, r3, c7, c10, 4	@ drain write buffer
	@ disable the data cache for GBA ROM space
	@ note: this code must be synchronous with /arch/arm/mach-nds/head.S!
	mov	r3, #0b00000010
	mcr	p15, 0, r3, c2, c0, 0
	.endm

/*****************************************************************************/
/*
 * Suffix macro. This macro does all the magic required to switch the
 * GBA ROM space from IO mode to RAM mode (exclusive the switching itself,
 * which is device dependent).
 * This is a macro and not a function, because the stack(!) may be in GBA
 * ROM space and therefore the stack must not be modified until the end of 
 * this macro.
 *
 * Register R0 is not altered and used as a return register.
 */
	.macro	suffix
	@ be safe that all switching commands before are executed
	mov	ip, #0
	mcr	p15, 0, ip, c7, c10, 4	@ drain write buffer
	@ enable the data cache for GBA ROM space
	@ note: this code must be synchronous with /arch/arm/mach-nds/head.S!
	mov	r3, #0b10000010
	mcr	p15, 0, r3, c2, c0, 0
	@ enable interrupts
	ldr	r3, =NDS_IME		@ interrupt mask register
	mov	ip, #1			@ 1 = enable
	strh	ip, [r3]
	.endm

/*****************************************************************************/




