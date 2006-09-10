/*
 *  linux/drivers/ide/arm/m3cf_s.S - M3 CF driver
 *
 *  Copyright (C) 2006 Amadeus, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This driver must be in main memory.
 */

/* common macros for all NDS GBA ROM device drivers */
#include <asm/arch/gbarom-macro.S>

    	.TEXT

/*****************************************************************************/

#define REG_M3CF_LBA1	0x08860000	@ 1st byte of sector address

/*****************************************************************************/

	@ Test if the card is present.
	@ This test is tricky because if it's NOT this card,
        @ we are not allowed to destroy contents of GBA ROM space.
	@ R0: Return != 0 if present. 
	.ALIGN
	.GLOBAL m3cf_detect_card
m3cf_detect_card:	
	gba_prefix
	@ read old value @ REG_M3CF_LBA1 (in RAM)
	ldr	r3, =REG_M3CF_LBA1
	ldrh	r1, [r3]
	@ now switch to IO mode
	m3_set_io
	@ test if the lower 8 bit of LBA1 are read- and writable	
	ldr	r3, =REG_M3CF_LBA1
	ldrb	ip, [r3]
	eor	ip, ip, #0xFF		@ invert lower 8 bit of LBA1
	strh	ip, [r3]		@ store complement in LBA1
	ldrb	r0, [r3]
	teq	ip, r0			@ are they the same?
	movne	r0, #0			@ failure code
	bne	m3cf_detect_exit	@ no: no CF
	@ make sure the register is 8 bit, not 16
	ldr	ip, =0xAA55
	strh	ip, [r3]
	ldrh	r0, [r3]
	teq	ip, r0			@ are they the same?
	moveq	r0, #0			@ yes: can't be a CF card
	movne	r0, #1			@ positive detection
m3cf_detect_exit:
	@ switch back to RAM
	m3_set_ram
	@ restore RAM contents @ REG_M3CF_LBA1 from R1
	ldr	r3, =REG_M3CF_LBA1
	strh	r1, [r3]
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Read a byte from card register.
	@ R0: address of register.
	@ R0: return value @ register.	
	.ALIGN
	.GLOBAL	m3cf_ide_inb
m3cf_ide_inb:
	gba_prefix
	@ now switch to IO mode
	m3_set_io
	ldrb	r0, [r0]
	@ switch back to RAM
	m3_set_ram
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Read a word from card register.
	@ R0: address of register.
	@ R0: return value @ register.	
	.ALIGN
	.GLOBAL	m3cf_ide_inw
m3cf_ide_inw:
	gba_prefix
	@ now switch to IO mode
	m3_set_io
	ldrh	r0, [r0]
	@ switch back to RAM
	m3_set_ram
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Read a long from card register.
	@ R0: address of register.
	@ R0: return value @ register.	
	.ALIGN
	.GLOBAL	m3cf_ide_inl
m3cf_ide_inl:
	gba_prefix
	@ now switch to IO mode
	m3_set_io
	ldr	r0, [r0]
	@ switch back to RAM
	m3_set_ram
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Write a byte/word to card register.
	@ R0: value
	@ R1: address of register.
	.ALIGN
	.GLOBAL m3cf_ide_outb
	.GLOBAL m3cf_ide_outw
m3cf_ide_outb:
m3cf_ide_outw:
	gba_prefix
	@ now switch to IO mode
	m3_set_io
	strh	r0, [r1]	@ write a word. NDS can't write bytes.
	@ switch back to RAM
	m3_set_ram
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Write a long to card register.
	@ R0: value
	@ R1: address of register.
	.ALIGN
	.GLOBAL	m3cf_ide_outl
m3cf_ide_outl:
	gba_prefix
	@ now switch to IO mode
	m3_set_io
	str	r0, [r1]
	@ switch back to RAM
	m3_set_ram
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Read a block of word data from card register.
	@ R0: address of register.
	@ R1: destination address.
	@ R2: count
	.ALIGN
	.GLOBAL	m3cf_ide_insw
m3cf_ide_insw:
	gba_prefix
	@ now switch to IO mode
	m3_set_io
m3cf_ide_insw_loop:
	ldrh	r3, [r0]
	strh	r3, [r1], #2
	subs	r2, r2, #1
	bne	m3cf_ide_insw_loop
	@ switch back to RAM
	m3_set_ram
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Write a block of word data to card register.
	@ R0: address of register.
	@ R1: source address.
	@ R2: count
	.ALIGN
	.GLOBAL	m3cf_ide_outsw
m3cf_ide_outsw:
	gba_prefix
	@ now switch to IO mode
	m3_set_io
m3cf_ide_outsw_loop:
	ldrh	r3, [r1], #2
	strh	r3, [r0]
	subs	r2, r2, #1
	bne	m3cf_ide_outsw_loop
	@ switch back to RAM
	m3_set_ram
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	.END

/*****************************************************************************/
