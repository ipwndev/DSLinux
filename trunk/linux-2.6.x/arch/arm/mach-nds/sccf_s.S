/*
 *  linux/arch/arm/mach-nds/sccf_s.S - Supercard CF driver
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

#define REG_SCCF_STS	0x098C0000	@ Status of the CF Card / Device control
#define REG_SCCF_CMD    0x090E0000      @ Commands sent to control chip and status return
#define REG_SCCF_ERR    0x09020000      @ Errors / Features
#define REG_SCCF_SEC    0x09040000      @ Number of sector to transfer
#define REG_SCCF_LBA1   0x09060000      @ 1st byte of sector address
#define REG_SCCF_LBA2   0x09080000      @ 2nd byte of sector address
#define REG_SCCF_LBA3   0x090A0000      @ 3rd byte of sector address
#define REG_SCCF_LBA4   0x090C0000      @ last nibble of sector address | 0xE0
#define REG_SCCF_DATA   0x09000000      @ Pointer to buffer of CF data transered from card
#define SC_SD_CMD	0x09800000

#define CF_STS_INSERTED 0x50
#define CF_STS_READY    0x58
#define CF_STS_BUSY     0x80

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
	.GLOBAL	sccf_transfer
sccf_transfer:
	gba_prefix
	@ now switch to IO mode (save old value @SC_LOCK in R2)
	sc_set_io
	@ Wait until card is ready for commands
	mov	r3, #0x1000
	ldr	ip, =REG_SCCF_STS
sccf_transfer_ready:
	subs	r3, r3, #1
	moveq	r0, #2			@ exit R0=2
	beq	sccf_transfer_exit
	ldrb	r1, [ip]
	and	r1, #CF_STS_INSERTED
	teq	r1, #CF_STS_INSERTED
	bne	sccf_transfer_ready
	@ set one sector to read
	ldr	ip, =REG_SCCF_SEC
	mov	r3, #1
	strh	r3, [ip]
	@ set sector address
	ldrh	r3, [r0], #2
	ldr	ip, =REG_SCCF_LBA1
	strh	r3, [ip]
	ldrh	r3, [r0], #2
	ldr	ip, =REG_SCCF_LBA2
	strh	r3, [ip]
	ldrh	r3, [r0], #2
	ldr	ip, =REG_SCCF_LBA3
	strh	r3, [ip]
	ldrh	r3, [r0], #2
	ldr	ip, =REG_SCCF_LBA4
	strh	r3, [ip]
	@ send command
	ldrh	r3, [r0]
	ldr	ip, =REG_SCCF_CMD
	strh	r3, [ip]
	@ Wait until card is ready for transfer
	mov	r3, #0x100000		@ long timeout, can't stop here
	ldr	ip, =REG_SCCF_STS
sccf_transfer_wait:
	subs	r3, r3, #1
	moveq	r0, #3			@ exit R0=3
	beq	sccf_transfer_exit
	ldrb	r1, [ip]
	teq	r1, #CF_STS_READY
	bne	sccf_transfer_wait
	@ transfer the data
	ldr	ip, =REG_SCCF_DATA
	mov	r3, #0x100
	@ which command?
	ldrh	r1, [r0], #2
	teq	r1, #0x30		@ test for WRITE command
	beq	sccf_write_loop
	@ read the data
sccf_read_loop:
	ldrh	r1, [ip]
	strh	r1, [r0], #2
	subs	r3, r3, #1
	bne	sccf_read_loop
sccf_transfer_ok:
	@ Wait until CF card is finished
	mov	r3, #0x100000		@ long timeout, can't stop here
	ldr	ip, =REG_SCCF_CMD
sccf_transfer_busy:
	subs	r3, r3, #1
	moveq	r0, #1			@ exit R0=1
	beq	sccf_transfer_exit
	ldrb	r1, [ip]
	tst	r1, #CF_STS_BUSY
	bne	sccf_transfer_busy
	@ all OK
	mov	r0, #0			@ exit R0=0
sccf_transfer_exit:
	@ switch back to RAM (restore old value @SC_LOCK from R2)
	sc_set_ram
	gba_suffix
	mov	pc, lr
	@ write the data
sccf_write_loop:
	ldrh	r1, [r0], #2
	strh	r1, [ip]
	subs	r3, r3, #1
	bne	sccf_write_loop
	b	sccf_transfer_ok

/*****************************************************************************/

	.END

/*****************************************************************************/
