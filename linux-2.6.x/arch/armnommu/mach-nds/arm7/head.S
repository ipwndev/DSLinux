
#include "linux/config.h"

.code 32
.align
	.type stext, #function
.globl stext;
stext:

#ifdef CONFIG_NDS_DSGBA
	/* Run program on ARM9 */
	ldr	r0, =0x02004000
	ldr	r1, =0x027FFE24
	str	r0, [r1]
#endif

	/* Disable interrupts */
	mov	r0, #0
	ldr	r1, =0x04000208
	str	r0, [r1]
	ldr	r1, =0x04000210
	str	r0, [r1]

	/* Jump to main */
	b	main

.global swiWaitForVBlank
swiWaitForVBlank:
	swi	0x050000
	bx	lr

