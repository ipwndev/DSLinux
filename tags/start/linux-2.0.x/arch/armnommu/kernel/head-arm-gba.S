/****************************************************************************/

/*
 *	linux/arch/armnommu/kernel/head-arm-gba.S
 *
 *	(C) Copyright 2001, Greg Ungerer (gerg@lineo.com)
 */

/****************************************************************************/

#include <linux/config.h>
#include <asm/assembler.h>
#include <asm/hardware.h>

/****************************************************************************/

#define	ARM_MODE_USR	0x10
#defin	ARM_MODE_IRQ	0x12
#define ARM_MODE_SVC	0x13

/*
 *	RAM base/size set in kernel config.
 */
#define DRAM_LIMIT	(DRAM_BASE + DRAM_SIZE)

/****************************************************************************/

.extern _stext, _etext
.extern _sdata, _edata
.extern _sbss, _ebss

.global start
.global _start
.global _entry
.global reset

/****************************************************************************/

.text
.align

start:
_start:
	/* GBA has header info at start of ROM */
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0

        .long 0,0,0,0,0,0,0,0

/****************************************************************************/

/*
 *	This is where it all starts.
 */
reset:
_entry:
	/* Disable interrupts */
	mov	r0, #(ARM_MODE_SVC | I_BIT | F_BIT )
	msr	cpsr, r0

	/*
	 *	Copy data segment into RAM, and clear BSS.
	 */
	ldr	r0, =_etext			/* Addr of data in FLASH */
	ldr	r1, =_sdata			/* Addr of real RAM data */
	ldr	r2, =_edata			/* Addr of end of data */
_copydata:
	cmp	r1, r2				/* All copied? */
	ldrcc	r3, [r0], #4
	strcc	r3, [r1], #4			/* Copy next word */
	bcc	_copydata			/* Keep going till done */

	ldr	r2, =_ebss			/* Addr of end of bss */
	mov	r3, #0
_zerobss:
	cmp	r1, r2				/* All zeroed? */
	strcc	r3, [r1], #4			/* Zero next word */
	bcc	_zerobss			/* Keep going till done */
	
	ldr	r0, =arm_id
	ldr	r1,=0x41007300			/* Hard code ARM CPU id */
	str	r1, [r0]

	ldr	sp, =init_kernel_stack+4096	/* Set initial stack */
	mov	fp, #0				/* Set initial frame */
	b	start_kernel			/* Kernel C code entry */

/****************************************************************************/
