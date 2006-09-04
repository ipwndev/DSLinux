/*
 *  linux/drivers/ide/arm/sccf_s.S - Supercard CF driver
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

#define REG_SCCF_LBA1	0x09060000	@ 1st byte of sector address
#define SC_SD_CMD	0x09800000

/*****************************************************************************/

	@ Test if the card is present.
	@ This test is tricky because if it's NOT this card,
        @ we are not allowed to destroy contents of GBA ROM space.
	@ R0: Return != 0 if present. 
	.ALIGN
	.GLOBAL sccf_detect_card
sccf_detect_card:	
	stmfd   sp!,{r4-r5}		@ use additional registers
	gba_prefix
	@ there is a special problem: if we have a supercard lite,
	@ accessing REG_SCCF_LBA1 is bad for the scsd driver.
	@ So we have to check for a supercard SD first.

	@ read old value @ SC_SD_CMD (in RAM)
	ldr	r3, =SC_SD_CMD
	ldrh	r4, [r3]
	@ store opposite of requested value in SC_SD_CMD (in RAM)
	mov	r0, #-2
	strh	r0, [r3]
	@ read old value @ REG_SCCF_LBA1 (in RAM)
	ldr	r3, =REG_SCCF_LBA1
	ldrh	r5, [r3]
	@ now switch to IO mode (save old value @SC_LOCK in R2)
	sc_set_io
	@ now read SC_SD_CMD (in IO)
	ldr	r3, =SC_SD_CMD
	ldrh	r0, [r3]
	tst	r0, #0x300		@ both bits must be 0
	bne	sccf_detect_testcf	@ if not: test for CF
	tst	r0, #0x001		@ bit must be 1
	beq	sccf_detect_testcf	@ if not: test for CF
	@ it's a supercard SD. So signal failure.
	mov	r0, #0
	b	sccf_detect_exit
	@ it's no supercard SD. So we can test for CF.
sccf_detect_testcf:
	@ test if the lower 8 bit of LBA1 are read- and writable	
	ldr	r3, =REG_SCCF_LBA1
	ldrb	ip, [r3]
	eor	ip, ip, #0xFF		@ invert lower 8 bit of LBA1
	strh	ip, [r3]		@ store complement in LBA1
	ldrb	r0, [r3]
	teq	ip, r0			@ are they the same?
	movne	r0, #0			@ failure code
	bne	sccf_detect_exit	@ no: no CF
	@ make sure the register is 8 bit, not 16
	ldr	ip, =0xAA55
	strh	ip, [r3]
	ldrh	r0, [r3]
	teq	ip, r0			@ are they the same?
	moveq	r0, #0			@ yes: can't be a CF card
	movne	r0, #1			@ positive detection
sccf_detect_exit:
	@ switch back to RAM (restore old value @SC_LOCK from R2)
	sc_set_ram
	@ restore RAM contents @ REG_SCCF_LBA1 from R5
	ldr	r3, =REG_SCCF_LBA1
	strh	r5, [r3]
	@ restore RAM contents @ SC_SD_CMD from R4
	ldr	r3, =SC_SD_CMD
	strh	r4, [r3]
	gba_suffix
	ldmfd   sp!,{r4-r5}		@ restore used registers
	mov	pc, lr

/*****************************************************************************/

	@ Read a byte from card register.
	@ R0: address of register.
	@ R0: return value @ register.	
	.ALIGN
	.GLOBAL	sccf_ide_inb
sccf_ide_inb:
	gba_prefix
	@ now switch to IO mode (save old value @SC_LOCK in R2)
	sc_set_io
	ldrb	r0, [r0]
	@ switch back to RAM (restore old value @SC_LOCK from R2)
	sc_set_ram
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Read a word from card register.
	@ R0: address of register.
	@ R0: return value @ register.	
	.ALIGN
	.GLOBAL	sccf_ide_inw
sccf_ide_inw:
	gba_prefix
	@ now switch to IO mode (save old value @SC_LOCK in R2)
	sc_set_io
	ldrh	r0, [r0]
	@ switch back to RAM (restore old value @SC_LOCK from R2)
	sc_set_ram
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Read a long from card register.
	@ R0: address of register.
	@ R0: return value @ register.	
	.ALIGN
	.GLOBAL	sccf_ide_inl
sccf_ide_inl:
	gba_prefix
	@ now switch to IO mode (save old value @SC_LOCK in R2)
	sc_set_io
	ldr	r0, [r0]
	@ switch back to RAM (restore old value @SC_LOCK from R2)
	sc_set_ram
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Write a byte/word to card register.
	@ R0: value
	@ R1: address of register.
	.ALIGN
	.GLOBAL sccf_ide_outb
	.GLOBAL sccf_ide_outw
sccf_ide_outb:
sccf_ide_outw:
	gba_prefix
	@ now switch to IO mode (save old value @SC_LOCK in R2)
	sc_set_io
	strh	r0, [r1]	@ write a word. NDS can't write bytes.
	@ switch back to RAM (restore old value @SC_LOCK from R2)
	sc_set_ram
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Write a long to card register.
	@ R0: value
	@ R1: address of register.
	.ALIGN
	.GLOBAL	sccf_ide_outl
sccf_ide_outl:
	gba_prefix
	@ now switch to IO mode (save old value @SC_LOCK in R2)
	sc_set_io
	str	r0, [r1]
	@ switch back to RAM (restore old value @SC_LOCK from R2)
	sc_set_ram
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Read a block of word data from card register.
	@ R0: address of register.
	@ R1: destination address.
	@ R2: count
	.ALIGN
	.GLOBAL	sccf_ide_insw
sccf_ide_insw:
	stmfd   sp!,{r4}		@ use additional registers
	mov	r4, r2			@ save word count
	gba_prefix
	@ now switch to IO mode (save old value @SC_LOCK in R2)
	sc_set_io
sccf_ide_insw_loop:
	ldrh	r3, [r0]
	strh	r3, [r1], #2
	subs	r4, r4, #1
	bne	sccf_ide_insw_loop
	@ switch back to RAM (restore old value @SC_LOCK from R2)
	sc_set_ram
	gba_suffix
	ldmfd   sp!,{r4}		@ restore used registers
	mov	pc, lr

/*****************************************************************************/

	@ Write a block of word data to card register.
	@ R0: address of register.
	@ R1: source address.
	@ R2: count
	.ALIGN
	.GLOBAL	sccf_ide_outsw
sccf_ide_outsw:
	stmfd   sp!,{r4}		@ use additional registers
	mov	r4, r2			@ save word count
	gba_prefix
	@ now switch to IO mode (save old value @SC_LOCK in R2)
	sc_set_io
sccf_ide_outsw_loop:
	ldrh	r3, [r1], #2
	strh	r3, [r0]
	subs	r4, r4, #1
	bne	sccf_ide_outsw_loop
	@ switch back to RAM (restore old value @SC_LOCK from R2)
	sc_set_ram
	gba_suffix
	ldmfd   sp!,{r4}		@ restore used registers
	mov	pc, lr

/*****************************************************************************/

	.END

/*****************************************************************************/
