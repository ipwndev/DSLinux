/*
 *  linux/drivers/mmc/m3sd_s.S - M3 SD driver
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
/* IO registers */

#define REG_M3SD_DIR	0x08800000	@ direction control register
#define REG_M3SD_DAT	0x09000000	@ SD data line, 8 bits at a time
#define REG_M3SD_CMD	0x09200000	@ SD command byte
#define REG_M3SD_ARGH	0x09400000	@ SD command argument, high halfword
#define REG_M3SD_ARGL	0x09600000	@ SD command argument, low halfword
#define REG_M3SD_STS	0x09800000	@ command and status register

/* LBA1 register of M3 CF */
#define REG_M3CF_LBA1	0x08860000	@ 1st byte of sector address

/*****************************************************************************/

	@ Test if the card is present.
	@ This test is tricky because if it's NOT this card,
        @ we are not allowed to destroy contents of GBA ROM space.
	@ R0: Return != 0 if present. 
	.ALIGN
	.GLOBAL m3sd_detect_card
m3sd_detect_card:	
	gba_prefix
	@ read and save old value @ REG_M3CF_LBA1
	ldr	r3, =REG_M3CF_LBA1
	ldrh	r2, [r3]
	ldr	r1, =0x789A		@ random value
	strh	r1, [r3]
	@ switch M3 to IO mode
	m3_set_io
	@ test if the value has remained the same
	ldr	r3, =REG_M3CF_LBA1
	ldrh	ip, [r3]
	teq	r1, ip
	moveq	r0, #0			@ YES: no M3
	beq	m3sd_detect_exit
	@ it's a M3. Test for CF. If not CF, it must be a SD.
	@ test if the lower 8 bit of LBA1 are read- and writable	
	ldrb	ip, [r3]
	eor	ip, ip, #0xFF		@ invert lower 8 bit of LBA1
	strh	ip, [r3]		@ store complement in LBA1
	ldrb	r0, [r3]
	teq	ip, r0			@ are they the same?
	movne	r0, #1			@ NO: no CF, must be SD
	bne	m3sd_detect_exit
	@ make sure the register is 8 bit, not 16
	ldr	ip, =0xAA55
	strh	ip, [r3]
	ldrh	r0, [r3]
	teq	ip, r0			@ are they the same?
	moveq	r0, #1			@ YES: no CF, must be SD
	movne	r0, #0			@ NO: it's a CF
m3sd_detect_exit:
	m3_set_ram			@ switch back to RAM
	@ restore RAM contents
	ldr	r3, =REG_M3CF_LBA1
	strh	r2, [r3]
	gba_suffix
	mov	pc, lr

/*****************************************************************************/
	@ macro to read a byte of response (from CMD line)

	@ register input:
	@ R3 = REG_M3SD_STS
	@ R4 = REG_M3SD_DIR

	@ register used:
	@ R5, CC

	@ register output:
	@ IP = byte to read

	@ error codes:
 	@ R0 = 3: timeout @ readbyte, first half
 	@ R0 = 4: timeout @ readbyte, 2nd half
 
	.macro	readbyte exitlabel
	mov	ip, #0x02
	strh	ip, [r4]		@ REG_M3SD_DIR
	strh	ip, [r3]		@ REG_M3SD_STS
 	mov	r5, #16384		@ timeout counter
1:	subs	r5, r5, #1
	moveq	r0, #3			@ exit R0=3
	beq	\exitlabel
	ldrh	ip, [r3]		@ REG_M3SD_STS
	tst	ip, #0x08
	beq	1b
	mov	r5, #16384		@ timeout counter
2:	subs	r5, r5, #1
	moveq	r0, #4			@ exit R0=4
	beq	\exitlabel
	ldrh	ip, [r3]		@ REG_M3SD_STS
	tst	ip, #0x08
	bne	2b
	ldr	r5, =REG_M3SD_DAT
	ldrb	ip, [r5]		@ get data byte
	.endm

/*****************************************************************************/

	@ Write at minimum 8 clock cycles to the card.
	@ Wait max. 1ms for the CMD line to become HIGH.
	@ Send a command to the card.
	@ if R2 > 0:
   	@ Wait max. 1ms for the CMD line to become LOW.
   	@ Read a response from the device. 
        @ R0: pointer to start of command & response. 32bit aligned.
	@ R1: length of the command (including CRC7).
	@ R2: length of response.
   	@ R0 Return 0 if OK, 1 if timeout waiting for CMD HIGH,
	@                    2 if timeout waiting for CMD LOW 
 	@ 		     3 if timeout @ readbyte, first half
 	@                    4 if timeout @ readbyte, 2nd half
	@                    5 if timeout for M3 command processing
	.ALIGN
	.GLOBAL m3sd_send_command_resp
m3sd_send_command_resp:
	stmfd   sp!,{r4-r8}		@ use additional registers
	gba_prefix
	m3_set_io			@ switch to IO mode
	@ setup pointers to the most used card registers
	ldr	r3, =REG_M3SD_STS
	ldr	r4, =REG_M3SD_DIR
	@ Wait max. 1ms for the CMD line to become HIGH.
	@ this implies at minimum 8 clock cycles.
	mov	r6, #1024		@ retry counter
m3sd_send_command_cmdhigh:
	subs	r6, r6, #1
	moveq	r0, #1			@ exit R0=1
	beq	m3sd_send_command_exit
	readbyte m3sd_send_command_exit
	teq	ip, #0xFF		@ all bits high?
	bne	m3sd_send_command_cmdhigh
	@ Send a command to the card. 
	mov	ip, #0x08
	strh	ip, [r3]
	ldr	r5, =REG_M3SD_CMD
	ldrh	ip, [r0, #0]
	strh	ip, [r5]
	ldr	r5, =REG_M3SD_ARGH
	ldrh	ip, [r0, #2]
	strh	ip, [r5]
	ldr	r5, =REG_M3SD_ARGL
	ldrh	ip, [r0, #4]
	strh	ip, [r5]
	@ CRC is done by the M3.
	@ wait until M3 has processed the command.
	mov	ip, #0x29
	strh	ip, [r4]		@ REG_M3SD_DIR
	mov	r5, #16384		@ timeout counter
m3sd_send_command_proc:
	subs	r5, r5, #1
	moveq	r0, #0x09
	streqh	r0, [r4]		@ REG_M3SD_DIR
	moveq	r0, #5			@ exit R0=5
	beq	m3sd_send_command_exit
	ldrh	ip, [r3]		@ REG_M3SD_STS
	tst	ip, #0x01
	beq	m3sd_send_command_proc
	mov	ip, #0x09
	strh	ip, [r4]		@ REG_M3SD_DIR
	@ test if we have to read a response
	teq	r2, #0			@ reslen = 0?
	moveq	r0, #0			@ exit R0=0
	beq	m3sd_send_command_exit
	@ wait until start of response
	mov	r6, #256		@ max. byte count
m3sd_send_command_wait:
	subs	r6, r6, #1
	moveq	r0, #2			@ exit R0=2
	beq	m3sd_send_command_exit
	readbyte m3sd_send_command_exit
	teq	ip, #0xFF
	beq	m3sd_send_command_wait
	@ now we have the first bits of the response in IP
	@ align to first bit (==0)
	mov	r6, #8			@ shift count
	mov	r5, #0x80		@ test pattern
m3sd_send_command_align:
	tst	ip, r5			@ is this bit 0?
	movne	r5, r5, lsr #1
	subne	r6, r6, #1
	bne	m3sd_send_command_align
	mov	r7, ip			@ save byte in R7
	@ read bytes and store into response buffer
m3sd_read_response_loop:
	readbyte m3sd_send_command_exit
	orr	r7, ip, r7, lsl #8	@ shift data byte in R7
	mov	ip, r7, lsr r6		@ deshifting
	strb	ip, [r0], #1		@ store byte
	subs	r2, r2, #1
	bne	m3sd_read_response_loop
	mov	r0, #0			@ success code
	@ exit
m3sd_send_command_exit:
	m3_set_ram			@ swith to RAM mode
	gba_suffix
	ldmfd   sp!,{r4-r8}		@ restore used registers
	mov	pc, lr

/*****************************************************************************/

	@ Wait for the DATA line to become HIGH.
	@ R0: Return != 0 if OK, 0 if timeout.
	@ Maximum length of testing is 1ms.
	.ALIGN
	.GLOBAL m3sd_wait_ready
m3sd_wait_ready:
	gba_prefix
	m3_set_io			@ switch to IO mode
	ldr	r2, =REG_M3SD_DIR
	ldr	r1, =REG_M3SD_DAT
	mov	ip, #1024		@ number of tries
m3sd_wait_ready_again:
	mov	r3, #8			@ high times counter
m3sd_wait_ready_loop:
	subs	ip, ip, #1		@ dec. loop count
	moveq	r0, #0			@ exit R0=0
	beq	m3sd_wait_ready_exit
	mov	r0, #0x00
	strh	r0, [r2]		@ REG_M3SD_DIR
	mov	r0, r0			@ nop
	mov	r0, r0			@ nop
	mov	r0, #0x08
	strh	r0, [r2]		@ REG_M3SD_DIR
	ldrh	r0, [r1]		@ REG_M3SD_DAT
	tst	r0, #0x100
	beq	m3sd_wait_ready_again	@ low: once more
	subs	r3, r3, #1		@ high: dec high counter
	bne	m3sd_wait_ready_loop	@ test again
	mov	r0, #1			@ 8 times HIGH - OK!
m3sd_wait_ready_exit:
	m3_set_ram			@ swith to RAM mode
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Send a Data block incl. CRC.
	@ R0: pointer to start of data. 32bit aligned.
	@ R1: number of bytes to send.
	@ R0: Return != 0 if OK, 0 if CRC missing or error. */
	.ALIGN
	.GLOBAL m3sd_send_data
m3sd_send_data:
	stmfd   sp!,{r4-r5}		@ use additional registers
	gba_prefix
	m3_set_io			@ switch to IO mode
	@ setup pointers to the most used card registers
	ldr	r3, =REG_M3SD_STS
	ldr	r4, =REG_M3SD_DIR
	ldr	r5, =REG_M3SD_DAT
	mov	ip, #0x04
	strh	ip, [r4]		@ REG_M3SD_DIR
	mov	ip, #0x00
	strh	ip, [r3]		@ REG_M3SD_STS
	@ start bit
	mov	ip, #0x00
	strh	ip, [r5]		@ REG_M3SD_DAT
	mov	ip, #0x04
	strh	ip, [r4]		@ REG_M3SD_DIR
	mov	ip, ip			@ nop
	mov	ip, ip			@ nop
	mov	ip, #0x0C
	strh	ip, [r4]		@ REG_M3SD_DIR
m3sd_send_data_loop:
	ldrb	ip, [r0], #1		@ read next byte
	mov	r2, ip, lsr #4		@ MSN
	strh	r2, [r5]		@ REG_M3SD_DAT
	mov	r2, #0x04
	strh	r2, [r4]		@ REG_M3SD_DIR
	mov	r2, r2			@ nop
	mov	r2, r2			@ nop
	mov	r2, #0x0C
	strh	r2, [r4]		@ REG_M3SD_DIR
	strh	ip, [r5]		@ REG_M3SD_DAT
	mov	r2, #0x04
	strh	r2, [r4]		@ REG_M3SD_DIR
	mov	r2, r2			@ nop
	mov	r2, r2			@ nop
	mov	r2, #0x0C
	strh	r2, [r4]		@ REG_M3SD_DIR
	subs	r1, r1, #1
	bne     m3sd_send_data_loop
	@ wait a little while
	mov	ip, #32
m3sd_send_wait:
	mov	r1, r1
	subs	ip, ip, #1
	bne	m3sd_send_wait
	@ send some FF
	mov	r1, #32
m3sd_send_FF:
	mov	ip, #0xFF
	strh	ip, [r5]		@ REG_M3SD_DAT
	mov	r2, #0x04
	strh	r2, [r4]		@ REG_M3SD_DIR
	mov	r2, r2			@ nop
	mov	r2, r2			@ nop
	mov	r2, #0x0C
	strh	r2, [r4]		@ REG_M3SD_DIR
	subs	r1, r1, #1
	bne     m3sd_send_FF
	mov	r0, #1			@ R0=OK
m3sd_send_data_exit:
	m3_set_ram			@ swith to RAM mode
	gba_suffix
	ldmfd   sp!,{r4-r5}		@ restore used registers
	mov	pc, lr

/*****************************************************************************/

	@ Wait max 1ms for the DATA line to become LOW.
	@ Receive a Data block and CRC.
	@ R0: pointer to start of data. 32bit aligned.
	@ R1: number of bytes to receive (incl. CRC)
	@ R0: Return != 0 if OK, 0 if timeout.
	.ALIGN
	.GLOBAL m3sd_read_data
m3sd_read_data:
	stmfd   sp!,{r4-r8}		@ use additional registers
	gba_prefix
	m3_set_io			@ switch to IO mode
	@ setup pointers to the most used card registers
	ldr	r3, =REG_M3SD_STS
	ldr	r4, =REG_M3SD_DIR
	mov	r2, #0x49
	strh	r2, [r4]		@ REG_M3SD_DIR
	mov	r2, #1024
m3sd_read_data_wait:
	subs	r2, r2, #1		@ dec. loop count
	moveq	r2, #0x09
	streqh	r2, [r4]		@ REG_M3SD_DIR
	moveq	r0, #0			@ exit R0=0
	beq	m3sd_read_data_exit
	ldrh	ip, [r3]		@ REG_M3SD_STS
	tst	ip, #0x40
	beq	m3sd_read_data_wait
	mov	r2, #0x09
	strh	r2, [r4]		@ REG_M3SD_DIR
	mov	r2, #0x08
	strh	r2, [r4]		@ REG_M3SD_DIR
	mov	r2, #0x04
	strh	r2, [r3]		@ REG_M3SD_STS
	ldrh	r2, [r4]		@ REG_M3SD_DIR
m3sd_read_data_loop:
	ldrh	r2, [r4]		@ REG_M3SD_DIR
	strh	r2, [r0], #2
	subs	r1, r1, #2
	bne	m3sd_read_data_loop
	mov	ip, #0x08
	strh	ip, [r3]		@ REG_M3SD_STS
	mov	r0, #1			@ R0=OK
m3sd_read_data_exit:
	m3_set_ram			@ swith to RAM mode
	gba_suffix
	ldmfd   sp!,{r4-r8}		@ restore used registers
	mov	pc, lr

/*****************************************************************************/

	@ Write at minimum 8 clock cycles to the card.
	.ALIGN
	.GLOBAL m3sd_send_clocks
m3sd_send_clocks:
	stmfd   sp!,{r4-r5}		@ use additional registers
	gba_prefix
	m3_set_io			@ switch to IO mode
	@ setup pointers to the most used card registers
	ldr	r3, =REG_M3SD_STS
	ldr	r4, =REG_M3SD_DIR
	readbyte m3sd_send_clocks_exit
m3sd_send_clocks_exit:
	m3_set_ram			@ swith to RAM mode
	gba_suffix
	ldmfd   sp!,{r4-r5}		@ restore used registers
	mov	pc, lr

/*****************************************************************************/

	@ Calculate the data crc.
   	@ R0: pointer to start of data. 32bit aligned.
   	@ R1: number of bytes of CRC calculation.
   	@ R2: pointer to start of CRC. 
	.ALIGN
	.GLOBAL m3sd_calc_crc
m3sd_calc_crc:
	stmfd   sp!,{r4-r9}
	mov	r9,r2

	mov	r3,#0  
	mov	r4,#0  
	mov	r5,#0  
	mov	r6,#0  

	ldr	r7,=0x80808080
	ldr	r8,=0x1021
	mov	r1,r1,lsl #3
m3sd_calc_crc_loop:

	tst	r7,#0x80
	ldrneb	r2,[r0],#1

	mov	r3,r3,lsl #1
	tst	r3,#0x10000
	eorne	r3,r3,r8
	tst	r2,r7,lsr #24
	eorne	r3,r3,r8
	
	mov	r4,r4,lsl #1
	tst	r4,#0x10000
	eorne	r4,r4,r8
	tst	r2,r7,lsr #25
	eorne	r4,r4,r8
	
	mov	r5,r5,lsl #1
	tst	r5,#0x10000
	eorne	r5,r5,r8
	tst	r2,r7,lsr #26
	eorne	r5,r5,r8
	
	mov	r6,r6,lsl #1
	tst	r6,#0x10000
	eorne	r6,r6,r8
	tst	r2,r7,lsr #27
	eorne	r6,r6,r8

	mov	r7,r7,ror #4
	subs	r1,r1,#4
        bne     m3sd_calc_crc_loop 

	mov	r2,r9
	mov	r8,#16
m3sd_calc_crc_write_data:
	mov	r7,r7,lsl #4
	tst	r3,#0x8000
	orrne	r7,r7,#8
	tst	r4,#0x8000
	orrne	r7,r7,#4
	tst	r5,#0x8000
	orrne	r7,r7,#2
	tst	r6,#0x8000
	orrne	r7,r7,#1

	mov	r3,r3,lsl #1
	mov	r4,r4,lsl #1
	mov	r5,r5,lsl #1
	mov	r6,r6,lsl #1

	sub	r8,r8,#1
	tst	r8,#1
	streqb	r7,[r2], #1
	cmp	r8,#0
	bne	m3sd_calc_crc_write_data

	ldmfd	sp!,{r4-r9}
	mov	pc, lr

/*****************************************************************************/

	.END

/*****************************************************************************/
