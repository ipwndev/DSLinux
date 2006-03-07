/*
 *
 * Inter-processor synchronization driver for Nintendo DS.
 *
 * Copyright (c) 2005 Stefan Sperling <stsp@stsp.in-berlin.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/module.h>
#include <linux/init.h> 
#include <linux/interrupt.h> 
#include <linux/version.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/semaphore.h>
#include <asm/arch/ipcsync.h>

#define SHMEMIPC_VERSION_CODE KERNEL_VERSION(0, 2, 0)
/* 
 * Version history:
 * 0.2.0: use linux lists
 * 0.1.0: initial version
 */


static LIST_HEAD(cblist);
static DECLARE_MUTEX(cblist_mtx);

static irqreturn_t ipcsync_isr(int irq, void *dev_id, struct pt_regs *regs)
{
	u8 type;
	struct list_head *p;
	struct ipcsync_cb *cb;

	type = ipcsync_get_remote_status();

	//printk("ipcsync: interrupt type %i\n", (int)type);
	
	list_for_each(p, &cblist) {
		cb = list_entry(p, struct ipcsync_cb, list);
		if (cb->type != type)
			continue;
		switch (type) {
			case SHMEMIPC_REQUEST_FLUSH:
				cb->handler.shmemipc_serve_flush_request();
				break;
			case SHMEMIPC_FLUSH_COMPLETE:
				cb->handler.shmemipc_flush_complete();
				break;
			default:
				return IRQ_NONE;
		}
	}
	return IRQ_HANDLED;
}

int register_ipcsync_cb(struct ipcsync_cb *cb)
{
	down(&cblist_mtx);
	list_add(&cb->list, &cblist);
	up(&cblist_mtx);
	return 0;
}

int unregister_ipcsync_cb(struct ipcsync_cb *cb)
{
	down(&cblist_mtx);
	list_del(&cb->list);
	up(&cblist_mtx);
	return 0;
}

static int __init ipcsync_init(void)
{
	printk("ipcsync: Driver version %d.%d.%d loaded\n",
	    (SHMEMIPC_VERSION_CODE >> 16) & 0xff,
	    (SHMEMIPC_VERSION_CODE >>  8) & 0xff,
	    (SHMEMIPC_VERSION_CODE      ) & 0xff);

        if (request_irq(IRQ_ARM7, ipcsync_isr, 0,
		"inter-processor synchronization driver", NULL)) {
                printk(KERN_ERR "ipcsync.c: Can't allocate irq %d\n", IRQ_ARM7);
                return -EBUSY;
        }

	/* Allow ipcsync interrupt from ARM7. */
	ipcsync_allow_local_interrupt();

	return 0;
}

static void __exit ipcsync_exit(void)
{
        free_irq(IRQ_ARM7, ipcsync_isr);
}

module_init(ipcsync_init);
module_exit(ipcsync_exit);

MODULE_AUTHOR("Stefan Sperling <stsp@stsp.in-berlin.de>");
MODULE_DESCRIPTION("Nintendo DS inter-processor synchronization driver");
MODULE_LICENSE("GPL");
