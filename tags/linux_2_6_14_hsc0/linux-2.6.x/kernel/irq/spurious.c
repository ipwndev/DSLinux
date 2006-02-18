/*
 * linux/kernel/irq/spurious.c
 *
 * Copyright (C) 1992, 1998-2004 Linus Torvalds, Ingo Molnar
 *
 * This file contains spurious interrupt handling.
 */

#include <linux/irq.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/interrupt.h>

static int irqfixup;

/*
 * Recovery handler for misrouted interrupts.
 */

static int misrouted_irq(int irq, struct pt_regs *regs)
{
	int i;
	irq_desc_t *desc;
	int ok = 0;
	int work = 0;	/* Did we do work for a real IRQ */

	for(i = 1; i < NR_IRQS; i++) {
		struct irqaction *action;

		if (i == irq)	/* Already tried */
			continue;
		desc = &irq_desc[i];
		spin_lock(&desc->lock);
		action = desc->action;
		/* Already running on another processor */
		if (desc->status & IRQ_INPROGRESS) {
			/*
			 * Already running: If it is shared get the other
			 * CPU to go looking for our mystery interrupt too
			 */
			if (desc->action && (desc->action->flags & SA_SHIRQ))
				desc->status |= IRQ_PENDING;
			spin_unlock(&desc->lock);
			continue;
		}
		/* Honour the normal IRQ locking */
		desc->status |= IRQ_INPROGRESS;
		spin_unlock(&desc->lock);
		while (action) {
			/* Only shared IRQ handlers are safe to call */
			if (action->flags & SA_SHIRQ) {
				if (action->handler(i, action->dev_id, regs) ==
						IRQ_HANDLED)
					ok = 1;
			}
			action = action->next;
		}
		local_irq_disable();
		/* Now clean up the flags */
		spin_lock(&desc->lock);
		action = desc->action;

		/*
		 * While we were looking for a fixup someone queued a real
		 * IRQ clashing with our walk
		 */

		while ((desc->status & IRQ_PENDING) && action) {
			/*
			 * Perform real IRQ processing for the IRQ we deferred
			 */
			work = 1;
			spin_unlock(&desc->lock);
			handle_IRQ_event(i, regs, action);
			spin_lock(&desc->lock);
			desc->status &= ~IRQ_PENDING;
		}
		desc->status &= ~IRQ_INPROGRESS;
		/*
		 * If we did actual work for the real IRQ line we must let the
		 * IRQ controller clean up too
		 */
		if(work)
			desc->handler->end(i);
		spin_unlock(&desc->lock);
	}
	/* So the caller can adjust the irq error counts */
	return ok;
}

/*
 * If 99,900 of the previous 100,000 interrupts have not been handled
 * then assume that the IRQ is stuck in some manner. Drop a diagnostic
 * and try to turn the IRQ off.
 *
 * (The other 100-of-100,000 interrupts may have been a correctly
 *  functioning device sharing an IRQ with the failing one)
 *
 * Called under desc->lock
 */

static void
__report_bad_irq(unsigned int irq, irq_desc_t *desc, irqreturn_t action_ret)
{
	struct irqaction *action;

	if (action_ret != IRQ_HANDLED && action_ret != IRQ_NONE) {
		printk(KERN_ERR "irq event %d: bogus return value %x\n",
				irq, action_ret);
	} else {
		printk(KERN_ERR "irq %d: nobody cared (try booting with "
				"the \"irqpoll\" option)\n", irq);
	}
	dump_stack();
	printk(KERN_ERR "handlers:\n");
	action = desc->action;
	while (action) {
		printk(KERN_ERR "[<%p>]", action->handler);
		print_symbol(" (%s)",
			(unsigned long)action->handler);
		printk("\n");
		action = action->next;
	}
}

static void report_bad_irq(unsigned int irq, irq_desc_t *desc, irqreturn_t action_ret)
{
	static int count = 100;

	if (count > 0) {
		count--;
		__report_bad_irq(irq, desc, action_ret);
	}
}

void note_interrupt(unsigned int irq, irq_desc_t *desc, irqreturn_t action_ret,
			struct pt_regs *regs)
{
	if (action_ret != IRQ_HANDLED) {
		desc->irqs_unhandled++;
		if (action_ret != IRQ_NONE)
			report_bad_irq(irq, desc, action_ret);
	}

	if (unlikely(irqfixup)) {
		/* Don't punish working computers */
		if ((irqfixup == 2 && irq == 0) || action_ret == IRQ_NONE) {
			int ok = misrouted_irq(irq, regs);
			if (action_ret == IRQ_NONE)
				desc->irqs_unhandled -= ok;
		}
	}

	desc->irq_count++;
	if (desc->irq_count < 100000)
		return;

	desc->irq_count = 0;
	if (desc->irqs_unhandled > 99900) {
		/*
		 * The interrupt is stuck
		 */
		__report_bad_irq(irq, desc, action_ret);
		/*
		 * Now kill the IRQ
		 */
		printk(KERN_EMERG "Disabling IRQ #%d\n", irq);
		desc->status |= IRQ_DISABLED;
		desc->handler->disable(irq);
	}
	desc->irqs_unhandled = 0;
}

int noirqdebug;

int __init noirqdebug_setup(char *str)
{
	noirqdebug = 1;
	printk(KERN_INFO "IRQ lockup detection disabled\n");
	return 1;
}

__setup("noirqdebug", noirqdebug_setup);

static int __init irqfixup_setup(char *str)
{
	irqfixup = 1;
	printk(KERN_WARNING "Misrouted IRQ fixup support enabled.\n");
	printk(KERN_WARNING "This may impact system performance.\n");
	return 1;
}

__setup("irqfixup", irqfixup_setup);

static int __init irqpoll_setup(char *str)
{
	irqfixup = 2;
	printk(KERN_WARNING "Misrouted IRQ fixup and polling support "
				"enabled\n");
	printk(KERN_WARNING "This may significantly impact system "
				"performance\n");
	return 1;
}

__setup("irqpoll", irqpoll_setup);
