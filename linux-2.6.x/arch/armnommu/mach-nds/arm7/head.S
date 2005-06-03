.code 32
.align
	.type stext, #function
.globl stext;
stext:
	mov	r0, #0
	ldr	r1, =0x04000208
	str	r0, [r1]
	ldr	r1, =0x04000210
	str	r0, [r1]

	b	main

