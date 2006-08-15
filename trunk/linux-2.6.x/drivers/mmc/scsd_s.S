    .TEXT

@------ IO ports
.equ sd_comadd,0x9800000
.equ sd_dataadd,0x9000000  
.equ sd_dataradd,0x9100000

@------void sd_crc16_s(u16* buff,u16 num,u16* crc16buff)
    .ALIGN
    .GLOBAL	sd_crc16_s 
    .CODE 32
sd_crc16_s:
	stmfd   r13!,{r4-r9}
	mov	r9,r2

	mov	r3,#0  
	mov	r4,#0  
	mov	r5,#0  
	mov	r6,#0  

	ldr	r7,=0x80808080
	ldr	r8,=0x1021
	mov	r1,r1,lsl #3
sd_crc16_loop:

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
        bne     sd_crc16_loop 

	mov	r2,r9
	mov	r8,#16
sd_crc16_write_data:
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
	swpeqb	r9,r7,[r2]	@ don't use strb here
	addeq	r2,r2,#1
	cmp	r8,#0
	bne	sd_crc16_write_data

	ldmfd	r13!,{r4-r9}
	bx      r14
@------end sd_crc16_s-----------------------------------

@-----------------void sd_data_write_s(u16 *buff, u32 length)
    .ALIGN
    .GLOBAL sd_data_write_s	
    .CODE 32
sd_data_write_s:
	stmfd   r13!,{r2-r4}
	mov	r2,#sd_dataadd
sd_data_write_loop:
	ldrh	r3,[r0],#2
	add	r3,r3,r3,lsl #20
	mov	r4,r3,lsr #8	
	stmia   r2,{r3-r4}
	subs    r1,r1,#2
	bne     sd_data_write_loop
	ldmfd	r13!,{r2-r4}
	bx      r14
@-----------------end sd_data_write_s-------------------

@----------void sd_data_read_s(u16 *buff, u32 length)
    .ALIGN
    .GLOBAL sd_data_read_s	
    .CODE 32
sd_data_read_s:
	stmfd   r13!,{r2-r4}
	mov	r2,#sd_dataradd
sd_data_read_loop:
	ldmia	r2,{r3-r4} 
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	r2,{r3-r4} 
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	r2,{r3-r4} 
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

	ldmia	r2,{r3-r4} 
	mov	r3,r4,lsr #16
	strh	r3 ,[r0],#2

        subs    r1, r1, #8                
        bne     sd_data_read_loop 

	ldmfd	r13!,{r2-r4}  
	bx      r14
@----------end sd_data_read_s-------------



    .END











