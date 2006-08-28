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
 * 
 * NOTE: this driver destroys the last 2 bytes of RAM in the GBA ROM space.
 *       You don't want to store valuable information here.
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
detect_card:	
	prefix
	@ read old value @ SC_SD_CMD (in RAM)
	ldr	r3, =SC_SD_CMD
	ldrh	r2, [r3]
	@ store opposite of requested value in SC_SD_CMD (in RAM)
	mov	r0, #-1
	strh	r0, [r3]
	@ read old value @ SC_LOCK (in RAM)
	ldr	r3, =SC_LOCK
	ldrh	r1, [r3]
	@ now switch to IO mode (destroy RAM @ SC_SD_LOCK)
	sc_setmode SC_MODE_IO
	@ now read SC_SD_CMD (in IO)
	ldr	r3, =SC_SD_CMD
	ldrh	r0, [r3]
	tst	r0, #0x300
	moveq	r0, #1
	movne	r0, #0
	@ switch back to RAM
	sc_setmode SC_MODE_RAM
	@ restore RAM contents
	ldr	r3, =SC_SD_CMD
	strh	r2, [r3]
	ldr	r3, =SC_LOCK
	strh	r1, [r3] 
	suffix
	mov	pc, lr

/*****************************************************************************/

	@ Say if the data transfer is 1 or 4 bit.
	@ R0: Return != 0 if 4 bit.
	.ALIGN
transfer_nibbles:
	mov	r0, #1			@ we do only 4 bit data transfers.
	mov	pc, lr

/*****************************************************************************/

	@ Write at minimum 8 clock cycles to the card.
	@ Wait max. 1ms for the CMD line to become HIGH.
	@ Send a command to the card. Skip 2 Z bits.
        @ R0: pointer to start of command. 32bit aligned.
	@ R1: length of the command (including CRC7).
	@ R0: Return != 0 if OK, 0 if timeout.
	.ALIGN
send_command:
	stmfd   sp!,{r4-r6}		@ use additional registers
	prefix
	sc_setmode SC_MODE_IO
	ldr	r2,=SC_SD_CMD
	ldmia	r2,{r3-r6}		@ write 8 clocks
	mov	r4, #1024		@ max. loop count
send_command_busy:
	subs	r4, r4, #1		@ dec. loop count
	moveq	r0, #0			@ exit R0=0
	beq	send_command_exit
	ldrh	r3,[r2]   
	tst	r3,#0x1
	beq	send_command_busy
	ldrh	r3,[r2]			@ one additional clock needed here(?)
	@ supercard lite: write 0 before write command
	ldrb	r3,[r0]			@ R3 = command byte
	cmp	r3,#(0x40+24)		@ write single sector
	cmpne	r3,#(0x40+25)		@ write multiple sectors
	ldreq 	ip,=SC_SDL_DOWRITE
	streqh	ip, [ip]
send_command_loop:
        ldrb	r3,[r0],#1		@ R3 = byte to send
	add	r3,r3,r3,lsl #17	@ some bit stuffing magic for supercard
	mov	r4,r3,lsl #2
	mov	r5,r4,lsl #2
	mov	r6,r5,lsl #2
        stmia   r2,{r3-r6}  		@ send 8 bit, one bit per 16 bit write
        subs    r1, r1, #1                  
        bne 	send_command_loop
	ldr	r3,[r2]			@ skip 2 Z bits
	mov	r0, #1			@ OK
send_command_exit:
	sc_setmode SC_MODE_RAM
	suffix
	ldmfd   sp!,{r4-r6}		@ restore used registers
	mov	pc, lr

/*****************************************************************************/

	@ Wait max. 1ms for the CMD line to become LOW.
	@ Read a response from the device and skip 2 Z bits. 
        @ R0: pointer to start of response buffer. 32bit aligned.
        @ R1: number of bytes to read(including CRC).
	@ R0: Return != 0 if OK, 0 if timeout.
	.ALIGN
read_response:
	prefix
	sc_setmode SC_MODE_IO
	ldr	r2, =SC_SD_CMD
	mov	ip, #1024
read_response_wait:
	subs	ip, ip, #1		@ dec. loop count
	moveq	r0, #0			@ exit R0=0
	beq	read_response_exit
	ldrh	r3,[r2]   
	tst	r3,#0x1
	bne	read_response_wait
	mov	ip, #0			@ the first bit is 0
	ldrh	r3,[r2]   
	tst	r3,#0x1
	b	read_response_7bit	@ and begin in the middle
read_response_loop:
	mov	ip, #0			@ read 8 bit from CMD line
	ldr	r3,[r2]
	tst	r3, #0x00000001
	orrne	ip, ip, #0x80
	tst	r3, #0x00010000
read_response_7bit:
	orrne	ip, ip, #0x40
	ldr	r3,[r2]
	tst	r3, #0x00000001
	orrne	ip, ip, #0x20
	tst	r3, #0x00010000
	orrne	ip, ip, #0x10
	ldr	r3,[r2]
	tst	r3, #0x00000001
	orrne	ip, ip, #0x08
	tst	r3, #0x00010000
	orrne	ip, ip, #0x04
	ldr	r3,[r2]
	tst	r3, #0x00000001
	orrne	ip, ip, #0x02
	tst	r3, #0x00010000
	orrne	ip, ip, #0x01
	strb	ip,[r0],#1		@ store byte in buffer (main memory)
        subs    r1, r1, #1                  
        bne 	read_response_loop
	ldr	r3,[r2]			@ skip 2 Z bits
	mov	r0, #1			@ R0=OK
read_response_exit:
	sc_setmode SC_MODE_RAM
	suffix
	mov	pc, lr

/*****************************************************************************/

	@ Wait for the DATA line to become HIGH.
	@ R0: Return != 0 if OK, 0 if timeout.
	@ Maximum length of testing is 1ms.
	.ALIGN
wait_ready:
	prefix
	sc_setmode SC_MODE_IO
	ldr	r1,=SC_SD_DATAREAD	@ IO address
	mov	r2, #1024		@ number of tries
wait_ready_again:
	mov	r3, #8			@ high counter
wait_ready_loop:
	subs	r2, r2, #1		@ dec. loop count
	moveq	r0, #0			@ exit R0=0
	beq	wait_ready_exit
	ldrh	r0,[r1]			@ read DATA line
	tst	r0, #0x100
	beq	wait_ready_again	@ low: once more
	subs	r3, r3, #1		@ high: dec high counter
	bne	wait_ready_loop		@ test again
	@ 8 times HIGH - OK!
	mov	r0, #1			@ R0=OK
wait_ready_exit:
	sc_setmode SC_MODE_RAM
	suffix
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
send_data:
	prefix
	sc_setmode SC_MODE_IO
	@ write the start bit (0)
	ldr	ip,=SC_SD_DATAWRITE
	strh	ip, [ip]
send_data_loop:
	ldrh	r2,[r0],#2		@ read 16 bit
	add	r2,r2,r2,lsl #20
	mov	r3,r2,lsr #8	
	stmia   ip,{r2-r3}
	subs    r1,r1,#2
	bne     send_data_loop
	@ write the end bit
	ldr	r2,=-1
	strh	r2,[ip]
	@ switch to read mode
	ldr	ip,=SC_SD_DATAREAD
	@ skip 2 Z bits
	ldr	r2,[ip]
	mov	r3, #1024
send_data_wait:
	subs	r3, r3, #1		@ dec. loop count
	moveq	r0, #0			@ exit R0=0
	beq	send_data_exit
	ldrh	r2,[ip]
	tst	r2, #0x100
	bne	send_data_wait
	@ read CRC response and end bit
	mov	r0, #0			@ failure code
	ldr	r2,[ip]
	ldr	r2,[ip]			@ 1=bit28, 2=bit24, 3=bit20, end=bit16
	tst	r2, #0x10000000
	bne	send_data_exit
	tst	r2, #0x01000000
	movne	r0, #1			@ R0=OK if CRC code == 2 or 3
send_data_exit:
	sc_setmode SC_MODE_RAM
	suffix
	mov	pc, lr

/*****************************************************************************/

	@ Wait max 1ms for the DATA line to become LOW.
	@ Receive a Data block and CRC, skip the end bit.
	@ R0: pointer to start of data. 32bit aligned.
	@ R1: number of bytes to receive (incl. CRC)
	@ R0: Return != 0 if OK, 0 if timeout.
	.ALIGN
read_data:
	prefix
	sc_setmode SC_MODE_IO
	ldr	ip,=SC_SD_DATAREAD
	mov	r2, #1024
read_data_wait:
	subs	r2, r2, #1		@ dec. loop count
	moveq	r0, #0			@ exit R0=0
	beq	read_data_exit
	ldrh	r3,[ip]
	tst	r3,#0x100
	bne	read_data_wait
read_data_loop:
	ldmia	ip,{r2-r3} 
	mov	r2,r3,lsr #16
	strh	r2 ,[r0],#2

	ldmia	ip,{r2-r3} 
	mov	r2,r3,lsr #16
	strh	r2 ,[r0],#2

	ldmia	ip,{r2-r3} 
	mov	r2,r3,lsr #16
	strh	r2 ,[r0],#2

	ldmia	ip,{r2-r3} 
	mov	r2,r3,lsr #16
	strh	r2 ,[r0],#2

        subs    r1, r1, #8                
        bne     read_data_loop 
	
	ldrh	r2,[ip]		@ read the end bit
	mov	r0, #1		@ R0=OK
read_data_exit:
	sc_setmode SC_MODE_RAM
	suffix
	mov	pc, lr

/*****************************************************************************/

	@ Read the write protect switch.
	@ R0: Return != 0 if readonly, 0 if r/w.
	@ When in doubt, return 0.
	.ALIGN
read_only:
	mov	r0, #0		@ we cannot read the r/w switch
	mov	pc, lr

/*****************************************************************************/

	@ Write at minimum 8 clock cycles to the card.
	.ALIGN
send_clocks:
	prefix
	sc_setmode SC_MODE_IO
	ldr	r0,=SC_SD_CMD
	ldr	r1,[r0]
	ldr	r1,[r0]
	ldr	r1,[r0]
	ldr	r1,[r0]
	sc_setmode SC_MODE_RAM
	suffix
	mov	pc, lr

/*****************************************************************************/

	@ Structure for device-specific functions in Assembler.
	.GLOBAL	supercard_functions
	.ALIGN
supercard_functions:
	.word	detect_card
	.word	transfer_nibbles
	.word	send_command
	.word	read_response
	.word	wait_ready
	.word	send_data
	.word	read_data
	.word	read_only
	.word	send_clocks

	.END

/*****************************************************************************/
