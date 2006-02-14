/****************************************************************************/

/*
 *	linux/include/asm-armnommu/arch-gba/irq.h
 *
 *	(C) Copyright 2001, Greg Ungerer (gerg@snapgear.com)
 */

/****************************************************************************/
#ifndef __ASM_ARCH_IRQ_H
#define __ASM_ARCH_IRQ_H
/****************************************************************************/

#define BUILD_IRQ(s,n,m)

struct pt_regs;

extern int IRQ_interrupt(int irq, struct pt_regs *regs);
extern int timer_IRQ_interrupt(int irq, struct pt_regs *regs);
extern int fast_IRQ_interrupt(int irq, struct pt_regs *regs);
extern int bad_IRQ_interrupt(int irq, struct pt_regs *regs);
extern int probe_IRQ_interrupt(int irq, struct pt_regs *regs);

#define IRQ_interrupt0  IRQ_interrupt
#define IRQ_interrupt1  IRQ_interrupt
#define IRQ_interrupt2  IRQ_interrupt
#define IRQ_interrupt3  IRQ_interrupt
#define IRQ_interrupt4  IRQ_interrupt
#define IRQ_interrupt5  IRQ_interrupt
#define IRQ_interrupt6  IRQ_interrupt
#define IRQ_interrupt7  IRQ_interrupt
#define IRQ_interrupt8  IRQ_interrupt
#define IRQ_interrupt9  IRQ_interrupt
#define IRQ_interrupt10 IRQ_interrupt
#define IRQ_interrupt11 IRQ_interrupt
#define IRQ_interrupt12 IRQ_interrupt
#define IRQ_interrupt13 IRQ_interrupt
#define IRQ_interrupt14 IRQ_interrupt
#define IRQ_interrupt15 IRQ_interrupt
#define IRQ_interrupt16 IRQ_interrupt
#define IRQ_interrupt17 IRQ_interrupt
#define IRQ_interrupt18 IRQ_interrupt
#define IRQ_interrupt19 IRQ_interrupt
#define IRQ_interrupt20 IRQ_interrupt
#define IRQ_interrupt21 IRQ_interrupt
#define IRQ_interrupt22 IRQ_interrupt
#define IRQ_interrupt23 IRQ_interrupt
#define IRQ_interrupt24 IRQ_interrupt
#define IRQ_interrupt25 IRQ_interrupt
#define IRQ_interrupt26 IRQ_interrupt
#define IRQ_interrupt27 IRQ_interrupt
#define IRQ_interrupt28 IRQ_interrupt
#define IRQ_interrupt29 IRQ_interrupt
#define IRQ_interrupt30 IRQ_interrupt
#define IRQ_interrupt31 IRQ_interrupt

/* GBA timer is configured on TIMER1 (cascaded from 0). */
#undef IRQ_interrupt4
#define	IRQ_interrupt4	timer_IRQ_interrupt

#define IRQ_INTERRUPT(n)   (void (*)(void))IRQ_interrupt##n
#define FAST_INTERRUPT(n)  (void (*)(void))fast_IRQ_interrupt
#define BAD_INTERRUPT(n)   (void (*)(void))bad_IRQ_interrupt
#define PROBE_INTERRUPT(n) (void (*)(void))probe_IRQ_interrupt

/****************************************************************************/

static __inline__ void mask_irq(unsigned int irq)
{
	*((unsigned short *) GBA_IE) &= ~(0x1 << irq);
} 

static __inline__ void unmask_irq(unsigned int irq)
{
	*((unsigned short *) GBA_IE) |= (0x1 << irq);
}

static __inline__ unsigned long get_enabled_irqs(void)
{
	return((unsigned long) *((unsigned short *) GBA_IE));
}

static __inline__ void irq_init_irq(void)
{
	extern void vector_IRQ(void);
#if 0
	*((unsigned long *) GBA_VECIRQ) = vector_IRQ;
#endif
#if 0
	printk("%s(%d): setting time_IRQ...\n", __FILE__, __LINE__);
	*((unsigned long *) GBA_VECIRQ) = (unsigned long) timer_IRQ_interrupt;
#endif
	*((unsigned short *) GBA_IE) = 0;
	*((unsigned short *) GBA_IME) = 0x0001;
}

/****************************************************************************/
#endif	/* __ASM_ARCH_IRQ_H */
