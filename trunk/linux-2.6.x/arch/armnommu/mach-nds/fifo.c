/*
 * NDS FIFO driver
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

#include <asm/semaphore.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <asm/arch/fifo.h>

#define REG_IPCFIFOSEND (*(volatile u32*) 0x04000188)
#define REG_IPCFIFORECV (*(volatile u32*) 0x04100000)
#define REG_IPCFIFOCNT  (*(volatile u16*) 0x04000184)

struct fifo_cb *cbhead = NULL ;
static DECLARE_MUTEX(cbhead_mtx);

static irqreturn_t ndsfifo_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 data;
	struct fifo_cb * cb;

	while ( ! ( REG_IPCFIFOCNT & (1<<8) ) )
	{
		data = REG_IPCFIFORECV;
		for ( cb = cbhead ; cb ; cb = cb->next )
		{
			if ( cb->type == ( data & 0xf0000000 ) )
			{
				switch ( cb->type )
				{
					case FIFO_BUTTONS:
						cb->handler.button_handler( data & 0xff ) ;
						break ;
					case FIFO_TOUCH:
						cb->handler.touch_handler( (data & (1<<16)) >> 16, (data & (0xff<<8)) >> 8, data & 0xff ) ;
						break;
                    case FIFO_TIME:
                        cb->handler.time_handler( data & 0xffffff ) ;
                        break;
					default:
						break;
				}
			}
		}
	}

	return IRQ_HANDLED ;
}

int register_fifocb( struct fifo_cb *fifo_cb )
{
	down(&cbhead_mtx);
	fifo_cb->next = cbhead ;
	cbhead = fifo_cb ;
	up(&cbhead_mtx);
	return 0 ;
}

int unregister_fifocb(struct fifo_cb *fifo_cb)
{
	struct fifo_cb *cb;

	down(&cbhead_mtx);
	for (cb = cbhead; cb; cb = cb->next) {
		if (cb->next == NULL) { /* fifo_cb not found in list */
			up(&cbhead_mtx);
			return -EINVAL;
		} else if (cb->next == fifo_cb) {
			cb->next = cb->next->next;
			break;
		}
	}
	up(&cbhead_mtx);
	return 0;
}

static int __init ndsfifo_init(void)
{
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

