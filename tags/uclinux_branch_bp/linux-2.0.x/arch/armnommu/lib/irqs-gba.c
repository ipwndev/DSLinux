
#include <linux/sched.h>
#include <linux/interrupt.h>

#include <asm/io.h>
#include <asm/system.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/arch/irq.h>


void do_IRQ(int irq, struct pt_regs * regs);


inline void irq_ack(int priority)
{
	/* FIXME: need GBA support */
}
  
asmlinkage int probe_IRQ_interrupt(int irq, struct pt_regs * regs)
{
	/* FIXME: need GBA support */
  	mask_irq(irq);
  	irq_ack(0);
	return 0;
}

asmlinkage int bad_IRQ_interrupt(int irqn, struct pt_regs * regs)
{
	/* FIXME: need GBA support */
	printk("bad interrupt %d recieved!\n", irqn);
	irq_ack(0);
	return 0;
}

asmlinkage int IRQ_interrupt(int irq, struct pt_regs * regs)
{
	register unsigned long flags;
	register unsigned long saved_count;

	/* FIXME: need GBA support */

	saved_count = intr_count;
	intr_count = saved_count + 1;

	save_flags(flags);
	sti();
	do_IRQ(irq, regs);
	restore_flags(flags);
	intr_count = saved_count;
	irq_ack(0);
	return 0;
}

asmlinkage int timer_IRQ_interrupt(int irq, struct pt_regs * regs)
{
	register unsigned long flags;
	register unsigned long saved_count;

#if 0
	printk("timer_IRQ_interrupt(irq=%d)\n", irq);
#endif

	saved_count = intr_count;
	intr_count = saved_count + 1;

	save_flags(flags);
	do_timer(regs);
	restore_flags(flags);
	intr_count = saved_count;
	irq_ack(0);

#if 0
	printk("timer_IRQ_interrupt(irq=%d): DONE\n", irq);
#endif
	return 0;
}

asmlinkage int fast_IRQ_interrupt(int irq, struct pt_regs * regs)
{
	register unsigned long saved_count;

	/* FIXME: need GBA support */

	saved_count = intr_count;
	intr_count = saved_count + 1;

	do_IRQ(irq, regs);
	cli();
	intr_count = saved_count;
	return 1;
}

