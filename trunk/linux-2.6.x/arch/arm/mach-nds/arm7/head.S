
#include "linux/config.h"

.code 32
.align
	.type stext, #function
.globl stext;
stext:

#ifdef CONFIG_XIP_KERNEL
	/* Run program on ARM9 */
	ldr	r0, =0x02004000
	ldr	r1, =0x027FFE24
	str	r0, [r1]
#endif

	mov	r0, #0x12			@ Switch to IRQ Mode
	msr	cpsr, r0
	ldr	sp, =__sp_irq			@ Set IRQ stack

	mov	r0, #0x1F			@ Switch to System Mode
	msr	cpsr, r0
	ldr	sp, =__sp_usr			@ Set user stack

	mov	r0, #0
	ldr	r1, =__bss_start
	ldr	r2, =_end
1:	cmp 	r1, r2
	strcc	r0, [r1],#4
	bcc	1b

	/* Jump to main */
	b	main

.global swiDelay
swiDelay:
	swi	0x030000
	bx	lr

.global swiIntrWait
swiIntrWait:
	swi	0x040000
	bx	lr

.global swiWaitForVBlank
swiWaitForVBlank:
	swi	0x050000
	bx	lr
