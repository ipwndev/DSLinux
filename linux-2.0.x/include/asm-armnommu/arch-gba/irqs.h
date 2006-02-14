/****************************************************************************/

/*
 *	linux/include/asm-armnommu/arch-gba/irqs.h
 *
 *	(C) Copyright 2001, Greg Ungerer (gerg@snapgear.com)
 */

/****************************************************************************/
#ifndef __ASM_ARCH_IRQS_H
#define __ASM_ARCH_IRQS_H
/****************************************************************************/

/*
 *	GBA interrupts are controlled by 3 registers in the IO memory space.
 */
#define	GBA_IE		0x04000200		/* Interrupt mask */
#define	GBA_IF		0x04000202		/* Interrup service */
#define	GBA_IME		0x04000208		/* Enable/disable */

/*
 *	Interrupt numbers. These are as per hardware registers.
 *	To generate bit mask simply shift up by interrupt number.
 */
#define	IRQ_VBLANK	0			/* Vertical blank intr */
#define	IRQ_HBLANK	1			/* Horizontal blank intr */
#define	IRQ_YTRIG	2			/* Y Trigger intr */
#define	IRQ_TC0		3			/* Timer 0 intr */
#define	IRQ_TC1		4			/* Timer 1 intr */
#define	IRQ_TC2		5			/* Timer 2 intr */
#define	IRQ_TC3		6			/* Timer 3 intr */
#define	IRQ_COMMS	7			/* Comms intr */
#define	IRQ_DMA0	8			/* DMA 0 intr */
#define	IRQ_DMA1	9			/* DMA 1 intr */
#define	IRQ_DMA2	10			/* DMA 2 intr */
#define	IRQ_DMA3	11			/* DMA 3 intr */
#define	IRQ_KEYPAD	12			/* Keypad intr */
#define	IRQ_CART	13			/* CART intr */

#define	GBA_IRQ_MASK	0x3fff			/* All intr mask */


/* Machine specific interrupt sources.
 *
 * Adding an interrupt service routine for a source with this bit
 * set indicates a special machine specific interrupt source.
 * The machine specific files define these sources.  
 */
#define IRQ_MACHSPEC     (0x10000000L)
#define IRQ_IDX(irq)    ((irq) & ~IRQ_MACHSPEC)

/* various flags for request_irq() */
#define IRQ_FLG_LOCK    (0x0001)        /* handler is not replaceable   */
#define IRQ_FLG_REPLACE (0x0002)        /* replace existing handler     */
#define IRQ_FLG_FAST    (0x0004)
#define IRQ_FLG_SLOW    (0x0008)
#define IRQ_FLG_STD     (0x8000)        /* internally used              */

/****************************************************************************/
#endif /* __ASM_ARCH_IRQS_H */
