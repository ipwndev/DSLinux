/*
 *  linux/arch/arm/mach-nds/m3cf_s.S - M3 CF driver
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

#define REG_M3CF_STS	0x080C0000	@ Status of the CF Card / Device control
#define REG_M3CF_CMD    0x088E0000      @ Commands sent to control chip and status return
#define REG_M3CF_ERR    0x08820000      @ Errors / Features
#define REG_M3CF_SEC    0x08840000      @ Number of sector to transfer
#define REG_M3CF_LBA1   0x08860000      @ 1st byte of sector address
#define REG_M3CF_LBA2   0x08880000      @ 2nd byte of sector address
#define REG_M3CF_LBA3   0x088A0000      @ 3rd byte of sector address
#define REG_M3CF_LBA4   0x088C0000      @ last nibble of sector address | 0xE0
#define REG_M3CF_DATA   0x08800000      @ Pointer to buffer of CF data transered from card

#define CF_STS_INSERTED 0x50
#define CF_STS_READY    0x58
#define CF_STS_BUSY     0x80

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

	@ Wait until CF card is ready for a new command.
	@ Set sector count to 1.
	@ Write sector address to card.
	@ Write a command to card.
	@ Wait until card has executed the command.
	@ Transfer a 256 word block of word data to/from card register.
	@ Wait until card has finished the command.
	@ R0: buffer address with sector number and command.
	@ Return: R0 = 1 if timeout waiting for finish 
	@	       2 if timeout waiting for ready
	@	       3 if timeout after command
	@	       0 if all OK
	.ALIGN
	.GLOBAL	m3cf_transfer
m3cf_transfer:
	gba_prefix
	@ now switch to IO mode
	m3_set_io
	@ Wait until card is ready for commands
	mov	r3, #0x1000
	ldr	ip, =REG_M3CF_STS
m3cf_transfer_ready:
	subs	r3, r3, #1
	moveq	r0, #2			@ exit R0=2
	beq	m3cf_transfer_exit
	ldrb	r1, [ip]
	and	r1, #CF_STS_INSERTED
	teq	r1, #CF_STS_INSERTED
	bne	m3cf_transfer_ready
	@ set one sector to read
	ldr	ip, =REG_M3CF_SEC
	mov	r3, #1
	strh	r3, [ip]
	@ set sector address
	ldrh	r3, [r0], #2
	ldr	ip, =REG_M3CF_LBA1
	strh	r3, [ip]
	ldrh	r3, [r0], #2
	ldr	ip, =REG_M3CF_LBA2
	strh	r3, [ip]
	ldrh	r3, [r0], #2
	ldr	ip, =REG_M3CF_LBA3
	strh	r3, [ip]
	ldrh	r3, [r0], #2
	ldr	ip, =REG_M3CF_LBA4
	strh	r3, [ip]
	@ send command
	ldrh	r3, [r0]
	ldr	ip, =REG_M3CF_CMD
	strh	r3, [ip]
	@ Wait until card is ready for transfer
	mov	r3, #0x100000		@ long timeout, can't stop here
	ldr	ip, =REG_M3CF_STS
m3cf_transfer_wait:
	subs	r3, r3, #1
	moveq	r0, #3			@ exit R0=3
	beq	m3cf_transfer_exit
	ldrb	r1, [ip]
	teq	r1, #CF_STS_READY
	bne	m3cf_transfer_wait
	@ transfer the data
	ldr	ip, =REG_M3CF_DATA
	mov	r3, #0x100
	@ which command?
	ldrh	r1, [r0], #2
	teq	r1, #0x30		@ test for WRITE command
	beq	m3cf_write_loop
	@ read the data
m3cf_read_loop:
	ldrh	r1, [ip]
	strh	r1, [r0], #2
	subs	r3, r3, #1
	bne	m3cf_read_loop
m3cf_transfer_ok:
	@ Wait until CF card is finished
	mov	r3, #0x100000		@ long timeout, can't stop here
	ldr	ip, =REG_M3CF_CMD
m3cf_transfer_busy:
	subs	r3, r3, #1
	moveq	r0, #1			@ exit R0=1
	beq	m3cf_transfer_exit
	ldrb	r1, [ip]
	tst	r1, #CF_STS_BUSY
	bne	m3cf_transfer_busy
	@ all OK
	mov	r0, #0			@ exit R0=0
m3cf_transfer_exit:
	@ switch back to RAM
	m3_set_ram
	gba_suffix
	mov	pc, lr
	@ write the data
m3cf_write_loop:
	ldrh	r1, [r0], #2
	strh	r1, [ip]
	subs	r3, r3, #1
	bne	m3cf_write_loop
	b	m3cf_transfer_ok

/*****************************************************************************/

	.END

/*****************************************************************************/
