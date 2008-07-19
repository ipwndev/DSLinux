/*
 *  linux/drivers/mmc/scsd_s.S - Supercard SD driver
 *
 *  Copyright (C) 2006 Amadeus, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This driver must be in main memory (or if you are doing XIP) in the
 * lower 16 MBytes of GBA ROM space.
 */

/* common macros for all NDS GBA ROM device drivers */
#include <asm/arch/gbarom-macro.S>

    	.TEXT

/*****************************************************************************/
/* IO registers */
#define SC_SD_CMD	0x09800000
	/* bit 0: data bit to read  		*/
	/* bit 7: data bit to write 		*/

#define SC_SD_DATAWRITE 0x09000000
#define SC_SD_DATAREAD  0x09100000
#define SC_SDL_DOWRITE	0x09440000
	/* SC lite: write 0 before write command */

/*****************************************************************************/

	@ Test if the card is present.
	@ This test is tricky because if it's NOT this card,
        @ we are not allowed to destroy contents of GBA ROM space.
	@ R0: Return != 0 if present. 
	.ALIGN
	.GLOBAL scsd_detect_card
scsd_detect_card:	
	gba_prefix
	@ read old value @ SC_SD_CMD (in RAM)
	ldr	r3, =SC_SD_CMD
	ldrh	r1, [r3]
	@ store opposite of requested value in SC_SD_CMD (in RAM)
	mov	r0, #-2
	strh	r0, [r3]
	@ now switch to IO mode (save old value @SC_LOCK in R2)
	sc_set_io
	@ now read SC_SD_CMD (in IO)
	ldr	r3, =SC_SD_CMD
	ldrh	r0, [r3]
	tst	r0, #0x300		@ both bits must be 0
	movne	r0, #0			@ failure code
	bne	scsd_detect_exit
	tst	r0, #0x001		@ bit must be 1
	movne	r0, #1			@ success code
	moveq	r0, #0			@ failure code
scsd_detect_exit:
	@ switch back to RAM (restore old value @SC_LOCK from R2)
	sc_set_ram
	@ restore RAM contents
	ldr	r3, =SC_SD_CMD
	strh	r1, [r3]
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Write at minimum 8 clock cycles to the card.
	@ Wait max. 1ms for the CMD line to become HIGH.
	@ Send a command to the card. Skip 2 Z bits.
	@ if R2 > 0:
   	@ Wait max. 1ms for the CMD line to become LOW.
   	@ Read a response from the device and skip 2 Z bits. 
        @ R0: pointer to start of command & response. 32bit aligned.
	@ R1: length of the command (including CRC7).
	@ R2: length of response.
   	@ R0 Return 0 if OK, 1 if timeout waiting for CMD HIGH,
	@                    2 if timeout waiting for CMD LOW 
	.ALIGN
	.GLOBAL scsd_send_command_resp
scsd_send_command_resp:
	stmfd   sp!,{r4-r8}		@ use additional registers
	gba_prefix
	mov	r8, r2			@ store 3rd parameter in R8
	sc_set_io			@ switch to IO mode, save @SC_LOCK in R2
	ldr	r7,=SC_SD_CMD
	ldmia	r7,{r3-r6}		@ write 8 clocks
	mov	r4, #1024		@ max. loop count
scsd_send_command_busy:
	subs	r4, r4, #1		@ dec. loop count
	moveq	r0, #1			@ exit R0=1
	beq	scsd_send_command_exit
	ldrh	r3,[r7]   
	tst	r3,#0x1
	beq	scsd_send_command_busy
	ldrh	r3,[r7]			@ one additional clock needed here(?)
	@ supercard lite: write 0 before write command
	ldrb	r3,[r0]			@ R3 = command byte
	cmp	r3,#(0x40+24)		@ write single sector
	cmpne	r3,#(0x40+25)		@ write multiple sectors
	ldreq 	ip,=SC_SDL_DOWRITE
	streqh	ip, [ip]
	mov	ip, r0
scsd_send_command_loop:
        ldrb	r3,[ip],#1		@ R3 = byte to send
	add	r3,r3,r3,lsl #17	@ some bit stuffing magic for supercard
	mov	r4,r3,lsl #2
	mov	r5,r4,lsl #2
	mov	r6,r5,lsl #2
        stmia   r7,{r3-r6}  		@ send 8 bit, one bit per 16 bit write
        subs    r1, r1, #1                  
        bne 	scsd_send_command_loop
	ldr	r3,[r7]			@ skip 2 Z bits
	@ test if we have to read a response
	teq	r8, #0			@ reslen = 0?
	beq	scsd_send_command_ok	@ yes: no response please
	@ wait for CMD line LOW
	mov	ip, #1024
scsd_read_response_wait:
	subs	ip, ip, #1		@ dec. loop count
	moveq	r0, #2			@ exit R0=2
	beq	scsd_send_command_exit
	ldrh	r3,[r7]   
	tst	r3,#0x1
	bne	scsd_read_response_wait
	mov	ip, #0			@ the first bit is 0
	ldrh	r3,[r7]   
	tst	r3,#0x1
	b	scsd_read_response_7bit	@ and begin in the middle
scsd_read_response_loop:
	mov	ip, #0			@ read 8 bit from CMD line
	ldr	r3,[r7]
	tst	r3, #0x00000001
	orrne	ip, ip, #0x80
	tst	r3, #0x00010000
scsd_read_response_7bit:
	orrne	ip, ip, #0x40
	ldr	r3,[r7]
	tst	r3, #0x00000001
	orrne	ip, ip, #0x20
	tst	r3, #0x00010000
	orrne	ip, ip, #0x10
	ldr	r3,[r7]
	tst	r3, #0x00000001
	orrne	ip, ip, #0x08
	tst	r3, #0x00010000
	orrne	ip, ip, #0x04
	ldr	r3,[r7]
	tst	r3, #0x00000001
	orrne	ip, ip, #0x02
	tst	r3, #0x00010000
	orrne	ip, ip, #0x01
	strb	ip,[r0],#1		@ store byte in buffer (main memory)
        subs    r8, r8, #1                  
        bne 	scsd_read_response_loop
	ldr	r3,[r7]			@ skip 2 Z bits
scsd_send_command_ok:
	mov	r0, #0			@ OK
scsd_send_command_exit:
	sc_set_ram			@ swith to RAM mode, restore @SC_LOCK from R2
	gba_suffix
	ldmfd   sp!,{r4-r8}		@ restore used registers
	mov	pc, lr

/*****************************************************************************/

	@ Wait for the DATA line to become HIGH.
	@ R0: Return != 0 if OK, 0 if timeout.
	@ Maximum length of testing is 1ms.
	.ALIGN
	.GLOBAL scsd_wait_ready
scsd_wait_ready:
	gba_prefix
	sc_set_io			@ switch to IO mode, save @SC_LOCK in R2
	ldr	r1,=SC_SD_DATAREAD	@ IO address
	mov	ip, #1024		@ number of tries
scsd_wait_ready_again:
	mov	r3, #8			@ high times counter
scsd_wait_ready_loop:
	subs	ip, ip, #1		@ dec. loop count
	moveq	r0, #0			@ exit R0=0
	beq	scsd_wait_ready_exit
	ldrh	r0,[r1]			@ read DATA line
	tst	r0, #0x100
	beq	scsd_wait_ready_again	@ low: once more
	subs	r3, r3, #1		@ high: dec high counter
	bne	scsd_wait_ready_loop	@ test again
	mov	r0, #1			@ 8 times HIGH - OK!
scsd_wait_ready_exit:
	sc_set_ram			@ swith to RAM mode, restore @SC_LOCK from R2
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Write a start bit, send a Data block incl. CRC.
	@ Write the end bit. Skip 2 Z bits.
        @ Wait max. 1ms for the start of the CRC response.
	@ Check the CRC response.
	@ R0: pointer to start of data. 32bit aligned.
	@ R1: number of bytes to send.
	@ R0: Return != 0 if OK, 0 if CRC missing or error. */
	.ALIGN
	.GLOBAL scsd_send_data
scsd_send_data:
	stmfd   sp!,{r4}		@ use additional registers
	gba_prefix
	sc_set_io			@ switch to IO mode, save @SC_LOCK in R2
	@ write the start bit (0)
	ldr	ip,=SC_SD_DATAWRITE
	strh	ip, [ip]
scsd_send_data_loop:
	ldrh	r3,[r0],#2		@ read 16 bit
	add	r3,r3,r3,lsl #20
	mov	r4,r3,lsr #8	
	stmia   ip,{r3-r4}
	subs    r1,r1,#2
	bne     scsd_send_data_loop
	@ write the end bit
	ldr	r3,=-1
	strh	r3,[ip]
	@ switch to read mode
	ldr	ip,=SC_SD_DATAREAD
	@ skip 2 Z bits
	ldr	r3,[ip]
	mov	r4, #1024
scsd_send_data_wait:
	subs	r4, r4, #1		@ dec. loop count
	moveq	r0, #0			@ exit R0=0
	beq	scsd_send_data_exit
	ldrh	r3,[ip]
	tst	r3, #0x100
	bne	scsd_send_data_wait
	@ read CRC response and end bit
	mov	r0, #0			@ failure code
	ldr	r3,[ip]
	ldr	r3,[ip]			@ 1=bit28, 2=bit24, 3=bit20, end=bit16
	tst	r3, #0x10000000
	bne	scsd_send_data_exit
	tst	r3, #0x01000000
	movne	r0, #1			@ R0=OK if CRC code == 2 or 3
scsd_send_data_exit:
	sc_set_ram			@ swith to RAM mode, restore @SC_LOCK from R2
	gba_suffix
	ldmfd   sp!,{r4}		@ restore used registers
	mov	pc, lr

/*****************************************************************************/

	@ Wait max 1ms for the DATA line to become LOW.
	@ Receive a Data block and CRC, skip the end bit.
	@ R0: pointer to start of data. 32bit aligned.
	@ R1: number of bytes to receive (incl. CRC)
	@ R0: Return != 0 if OK, 0 if timeout.
	.ALIGN
	.GLOBAL scsd_read_data
scsd_read_data:
	stmfd   sp!,{r4}		@ use additional registers
	gba_prefix
	sc_set_io			@ switch to IO mode, save @SC_LOCK in R2
	ldr	ip,=SC_SD_DATAREAD
	mov	r3, #1024
scsd_read_data_wait:
	subs	r3, r3, #1		@ dec. loop count
	moveq	r0, #0			@ exit R0=0
	beq	scsd_read_data_exit
	ldrh	r4,[ip]
	tst	r4,#0x100
	bne	scsd_read_data_wait
scsd_read_data_loop:
	ldmia	ip,{r3-r4} 
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	ip,{r3-r4} 
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	ip,{r3-r4} 
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	ip,{r3-r4} 
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

        subs    r1, r1, #8                
        bne     scsd_read_data_loop 
	
	ldrh	r3,[ip]			@ read the end bit
	mov	r0, #1			@ R0=OK
scsd_read_data_exit:
	sc_set_ram			@ swith to RAM mode, restore @SC_LOCK from R2
	gba_suffix
	ldmfd   sp!,{r4}		@ restore used registers
	mov	pc, lr

/*****************************************************************************/

	@ Write at minimum 8 clock cycles to the card.
	.ALIGN
	.GLOBAL scsd_send_clocks
scsd_send_clocks:
	gba_prefix
	sc_set_io			@ switch to IO mode, save @SC_LOCK in R2
	ldr	r0,=SC_SD_CMD
	ldr	r1,[r0]
	ldr	r1,[r0]
	ldr	r1,[r0]
	ldr	r1,[r0]
	sc_set_ram		@ switch to RAM mode, restore @SC_LOCK from R2
	gba_suffix
	mov	pc, lr

/*****************************************************************************/

	@ Calculate the data crc.
   	@ R0: pointer to start of data. 32bit aligned.
   	@ R1: number of bytes of CRC calculation.
   	@ R2: pointer to start of CRC. 
	.ALIGN
	.GLOBAL scsd_calc_crc
scsd_calc_crc:
	stmfd   sp!,{r4-r9}
	mov	r9,r2

	mov	r3,#0  
	mov	r4,#0  
	mov	r5,#0  
	mov	r6,#0  

	ldr	r7,=0x80808080
	ldr	r8,=0x1021
	mov	r1,r1,lsl #3
scsd_calc_crc_loop:

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
        bne     scsd_calc_crc_loop 

	mov	r2,r9
	mov	r8,#16
scsd_calc_crc_write_data:
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
	bne	scsd_calc_crc_write_data

	ldmfd	sp!,{r4-r9}
	mov	pc, lr

	.END

/*****************************************************************************/
