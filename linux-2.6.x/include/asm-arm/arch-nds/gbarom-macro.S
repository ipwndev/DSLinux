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
 * GBA prefix macro. This macro does all the magic required to switch the 
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
	.macro	gba_prefix
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
 * GBA suffix macro. This macro does all the magic required to switch the
 * GBA ROM space from IO mode to RAM mode (exclusive the switching itself,
 * which is device dependent).
 * This is a macro and not a function, because the stack(!) may be in GBA
 * ROM space and therefore the stack must not be modified until the end of 
 * this macro.
 *
 * Register R0 is not altered and used as a return register.
 */
	.macro	gba_suffix
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
/*
 * Supercard SD/CF ROM/RAM/IO switching.
 */
#define SC_LOCK      0x09FFFFFE
	/* bit 0: must be 1, unknown            */
	/* bit 1: enable IO interface (SD,CF)	*/
	/* bit 2: enable R/W SDRAM access 	*/
	/* bit 3-7: unknown, 0                  */
#define SC_MODE_ROM	0x01	/* 32 MByte ROM 		*/
#define SC_MODE_IO	0x03	/* 16 MByte ROM + 16 MByte IO 	*/
#define SC_MODE_RAM	0x05	/* 32 MByte RAM 		*/
#define SC_MODE_IORAM	0x07	/* 16 MByte RAM + 16 MByte IO 	*/
/* NOTE: the Supercard Lite doesn't like WRITE access to lower 
   ===== 16 MByte RAM, while I/O is in progress. */
#define SC_PATTERN	0xA55A	/* Key pattern for SC_LOCK      */

/* sc_set_io: set mode to I/O, save old value @SC_LOCK in R2.
   Use register R2, R3, IP. */
	.macro	sc_set_io
	ldr	r3, =SC_LOCK
	ldr	ip, =SC_PATTERN
	ldrh	r2, [r3]
	strh	ip, [r3]
	strh	ip, [r3]
	mov	ip, #SC_MODE_IO
	strh	ip, [r3]
	strh	ip, [r3]
	.endm

/* sc_set_ram: set mode to RAM, restore old value @SC_LOCK from R2.
   Use register R2, R3, IP. */
	.macro	sc_set_ram
	ldr	r3, =SC_LOCK
	ldr	ip, =SC_PATTERN
	strh	ip, [r3]
	strh	ip, [r3]
	mov	ip, #SC_MODE_RAM
	strh	ip, [r3]
	strh	ip, [r3]
	strh	r2, [r3]
	.endm

/*****************************************************************************/
/*
 * M3 Adapter SD/CF ROM/RAM/IO switching.
 */

/* 
 * m3_set_io: M3SD/CF is set to I/O ("MEDIA") mode.
 * Use only register R3 and IP.
 */
	.macro	m3_set_io
	ldr	r3, =0x08e00002
	ldrh	ip, [r3]
	ldr	r3, =0x0800000e
	ldrh	ip, [r3]
	ldr	r3, =0x08801ffc
	ldrh	ip, [r3]
	ldr	r3, =0x0800104a
	ldrh	ip, [r3]
	ldr	r3, =0x08800612
	ldrh	ip, [r3]
	ldr	r3, =0x08000000
	ldrh	ip, [r3]
	ldr	r3, =0x08801b66
	ldrh	ip, [r3]
	ldr	r3, =0x08800006
	ldrh	ip, [r3]
	ldr	r3, =0x0800080e
	ldrh	ip, [r3]
	ldr	r3, =0x08000000
	ldrh	ip, [r3]
	ldr	r3, =0x09000000
	ldrh	ip, [r3]
	.endm
/* 
 * m3_set_rom: M3SD/CF is set to ROM mode.
 * Use only register R3 and IP.
 */
	.macro	m3_set_rom
	ldr	r3, =0x08e00002
	ldrh	ip, [r3]
	ldr	r3, =0x0800000e
	ldrh	ip, [r3]
	ldr	r3, =0x08801ffc
	ldrh	ip, [r3]
	ldr	r3, =0x0800104a
	ldrh	ip, [r3]
	ldr	r3, =0x08800612
	ldrh	ip, [r3]
	ldr	r3, =0x08000000
	ldrh	ip, [r3]
	ldr	r3, =0x08801b66
	ldrh	ip, [r3]
	ldr	r3, =0x08800008
	ldrh	ip, [r3]
	ldr	r3, =0x0800080e
	ldrh	ip, [r3]
	ldr	r3, =0x08000000
	ldrh	ip, [r3]
	ldr	r3, =0x080001e4
	ldrh	ip, [r3]
	ldrh	ip, [r3]
	ldr	r3, =0x08000188
	ldrh	ip, [r3]
	ldrh	ip, [r3]
	.endm
/* 
 * m3_set_rw: if the previous mode of M3SD/CF was "ROM",
 * allow write access. Use only register R2, R3 and IP.
 */
	.macro	m3_set_rw
	ldr	r3, =0x09FFEFFE
	ldr	ip, =0xAA55
	ldrh	r2, [r3]	@ read old value @ 0x09FFEFFE
	strh	ip, [r3]
	strh	ip, [r3]
	strh	r2, [r3]	@ restore old value
	.endm

/*****************************************************************************/
