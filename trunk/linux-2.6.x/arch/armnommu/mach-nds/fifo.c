/*
 * NDS buttons driver
 *
 * Copyright (c) 2005 Malcolm Parsons
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h> 
#include <linux/interrupt.h> 

#include <asm/irq.h>
#include <asm/io.h>

#define REG_IPCFIFOSEND (*(volatile u32*) 0x04000188)
#define REG_IPCFIFORECV (*(volatile u32*) 0x04100000)
#define REG_IPCFIFOCNT  (*(volatile u16*) 0x04000184)

static irqreturn_t ndsfifo_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 data;
	printk("fifo data %d, %x:",irq, REG_IPCFIFOCNT);
	while ( ! ( REG_IPCFIFOCNT & (1<<8) ) )
	{
		data = REG_IPCFIFORECV;
		printk(" %d", data);
	}
	printk("\n");

	return IRQ_HANDLED ;
}

static int __init ndsfifo_init(void)
{
	u32 data;

        if (request_irq(IRQ_RECV, ndsfifo_interrupt, 0, "fifo", NULL)) {
                printk(KERN_ERR "fifo.c: Can't allocate irq %d\n", IRQ_RECV);
                return -EBUSY;
        }

	REG_IPCFIFOCNT = (1<<15) | (1<<10) | (1<<3) | (1<<14) ;

	return 0;
}

static void __exit ndsfifo_exit(void)
{
        free_irq(IRQ_RECV, ndsfifo_interrupt);
}

module_init(ndsfifo_init);
module_exit(ndsfifo_exit);

