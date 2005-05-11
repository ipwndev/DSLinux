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
#define	ARM_MODE_IRQ	0x12
#define ARM_MODE_SVC	0x13

#define PAGE_4K		(0b01011 << 1)
#define PAGE_8K		(0b01100 << 1)
#define PAGE_16K	(0b01101 << 1)
#define PAGE_32K	(0b01110 << 1)
#define PAGE_64K	(0b00111 << 1)
#define PAGE_128K	(0b10000 << 1)
#define PAGE_256K	(0b10001 << 1)
#define PAGE_512K	(0b10010 << 1)
#define PAGE_1M		(0b10011 << 1)
#define PAGE_2M		(0b10100 << 1)
#define PAGE_4M		(0b10101 << 1)
#define PAGE_8M		(0b10110 << 1)
#define PAGE_16M	(0b10111 << 1)
#define PAGE_32M	(0b11000 << 1)
#define PAGE_64M	(0b11001 << 1)
#define PAGE_128M	(0b11010 << 1)
#define PAGE_256M	(0b11011 << 1)
#define PAGE_512M	(0b11100 << 1)
#define PAGE_1G		(0b11101 << 1)
#define PAGE_2G		(0b11110 << 1)
#define PAGE_4G		(0b11111 << 1)

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
        b switch_to_arm9
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0

        .string "DSBooter"
.align
        .long 0,0,0,0,0

switch_to_arm9:
    mov r0, #0
    ldr r1, =0x04000208
    str r0, [r1]
    ldr r1, =0x04000210
    str r0, [r1]

    /* Copy small ARM9 program to RAM */
    ldr r0, =arm9boot_start
    ldr r1, =0x02004000
    ldr r2, =arm9boot_end
    subs r2, r2, r0
    add r2, r2, r1

_copyarm9boot:
	cmp	r1, r2				/* All copied? */
	ldrcc	r3, [r0], #4
	strcc	r3, [r1], #4			/* Copy next word */
	bcc	_copyarm9boot			/* Keep going till done */


    /* copy small ARM7 program to RAM */
    ldr r0, =arm7_start
    ldr r1, =0x037f8000
    ldr r2, =arm7_end
    subs r2, r2, r0
    add r2, r2, r1

_copyarm7:
	cmp	r1, r2				/* All copied? */
	ldrcc	r3, [r0], #4
	strcc	r3, [r1], #4			/* Copy next word */
	bcc	_copyarm7			/* Keep going till done */

    /* jump to ARM7 program */
    ldr pc, =0x037f8000

/****************************************************************************/

/*
 *	This is where it all starts.
 */
reset:
_entry:
    ldr r1, =0x04000000
    ldr r0, =0x00010100
    str r0, [r1]
    ldr r1, =0x04000240
    ldr r0, =0x81
    strb r0, [r1]

	/* Disable interrupts */
	mov	r0, #(ARM_MODE_SVC | PSR_I_BIT | PSR_F_BIT )
	msr	cpsr, r0

	mov	r0, #0x04000000

	mov	r1,#0				@ enable arm9 iwram
	strb	r1,[r0, #0x247]

	ldr	r1, =0x00002078			@ disable DTCM and protection unit
	mcr	p15, 0, r1, c1, c0

@---------------------------------------------------------------------------------
@ Protection Unit Setup added by Sasq
@---------------------------------------------------------------------------------
	@ Disable cache
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ Instruction cache
	mcr	p15, 0, r0, c7, c6, 0		@ Data cache

	@ Wait for write buffer to empty 
	mcr	p15, 0, r0, c7, c10, 4

	ldr	r0, =0x0080000A
	mcr	p15, 0, r0, c9, c1		@ TCM base = 0x00800*4096, size = 16 KB
@---------------------------------------------------------------------------------
@ Setup memory regions similar to Release Version
@	this code currently breaks dualis
@---------------------------------------------------------------------------------

	@-------------------------------------------------------------------------
	@ Region 0 - IO registers
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_64M | 0x04000000 | 1)	
	mcr	p15, 0, r0, c6, c0, 0

	@-------------------------------------------------------------------------
	@ Region 1 - Main Memory
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_4M | 0x02000000 | 1)	
	mcr	p15, 0, r0, c6, c1, 0

	@-------------------------------------------------------------------------
	@ Region 2 - iwram
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_32K | 0x037F8000 | 1)	
	mcr	p15, 0, r0, c6, c2, 0

	@-------------------------------------------------------------------------
	@ Region 3 - DS Accessory
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_128M | 0x08000000 | 1)	
	mcr	p15, 0, r0, c6, c3, 0

	@-------------------------------------------------------------------------
	@ Region 4 - DTCM
	@-------------------------------------------------------------------------
	@ldr	r0,=( PAGE_16K | 0x027C0000 | 1)	
	ldr	r0,=( PAGE_16K | 0x00800000 | 1)	
	mcr	p15, 0, r0, c6, c4, 0

	@-------------------------------------------------------------------------
	@ Region 5 - ITCM
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_32K | 0x01000000 | 1)	
	mcr	p15, 0, r0, c6, c5, 0

	@-------------------------------------------------------------------------
	@ Region 6 - System ROM
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_32K | 0xFFFF0000 | 1)	
	mcr	p15, 0, r0, c6, c6, 0

	@-------------------------------------------------------------------------
	@ Region 7 - IPC
	@-------------------------------------------------------------------------
	ldr	r0,=( PAGE_4K  | 0x027FF000 | 1)	
	mcr	p15, 0, r0, c6, c7, 0

	@-------------------------------------------------------------------------
	@ Write buffer enable
	@-------------------------------------------------------------------------
	ldr	r0,=0b00000110
	mcr	p15, 0, r0, c3, c0, 0

	@-------------------------------------------------------------------------
	@ DCache & ICache enable
	@-------------------------------------------------------------------------
	ldr	r0,=0b01000110
	ldr	r0,=0x42
	mcr	p15, 0, r0, c2, c0, 0
	mcr	p15, 0, r0, c2, c0, 1

	@-------------------------------------------------------------------------
	@ IAccess
	@-------------------------------------------------------------------------
	ldr	r0,=0x06300333
	mcr	p15, 0, r0, c5, c0, 3

	@-------------------------------------------------------------------------
	@ DAccess
	@-------------------------------------------------------------------------
	ldr	r0,=0x36633333
	mcr     p15, 0, r0, c5, c0, 2

	@-------------------------------------------------------------------------
	@ Enable ICache, DCache, ITCM & DTCM
	@-------------------------------------------------------------------------
	mrc	p15, 0, r0, c1, c0, 0
	ldr	r1,=0x55004
	orr	r0,r0,r1
	mcr	p15, 0, r0, c1, c0, 0

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
	ldr	r1,=0x41007400			/* Hard code ARM CPU id */
	str	r1, [r0]

	ldr	sp, =init_kernel_stack+4096	/* Set initial stack */
	mov	fp, #0				/* Set initial frame */
#if defined(CONFIG_ARCH_GBA)
	/* TC!IB! GBA BIOS uses the value in this address to jump when
	   IRQ is detected. We set our handler address there. */
	ldr	r0, =vector_IRQ
	ldr	r1, =0x00803FFC
	str	r0, [r1]
#endif
	b	start_kernel			/* Kernel C code entry */

/****************************************************************************/

.ltorg

arm9boot_start:
    ldr r0, =0xe800
    ldr r1, =0x04000204
    strh r0, [r1]
    ldr r0, =_entry
    bx r0
.ltorg
arm9boot_end:

arm7_start:
    /* Run program on ARM9 */
    ldr r0, =0x02004000
    ldr r1, =0x027FFE24
    str r0, [r1]

    /* busy loop */
    ldr r0, =0xffff
arm7loop:
    swi 0x03
    b arm7loop
.ltorg
arm7_end:

