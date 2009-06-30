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
#include <linux/list.h>

#include <asm/semaphore.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <asm/arch/fifo.h>

static LIST_HEAD(cblist);
static DECLARE_MUTEX(cblist_mtx);

static irqreturn_t ndsfifo_interrupt(int irq, void *dev_id,
				     struct pt_regs *regs)
{
	u32 fifo_recv;
	u32 data;
	struct list_head *p;
	struct fifo_cb *cb;

	while (!(NDS_REG_IPCFIFOCNT & FIFO_EMPTY)) {
		fifo_recv = NDS_REG_IPCFIFORECV;
		list_for_each(p, &cblist) {
			cb = list_entry(p, struct fifo_cb, list);
			if (cb->type == FIFO_GET_TYPE(fifo_recv)) {
				data = FIFO_GET_TYPE_DATA(fifo_recv);
				switch (cb->type) {
				case FIFO_FIRMWARE:
					cb->handler.firmware_handler(
					    FIFO_FIRMWARE_GET_CMD(data));
					break;
				case FIFO_BUTTONS:
					cb->handler.button_handler(data & 0xff);
					break;
				case FIFO_TOUCH:
					cb->handler.touch_handler(
					    (data & (1 << 16)) >> 16,
					    (data & (0xff << 8))
						>> 8, data & 0xff);
					break;
				case FIFO_TIME:
					cb->handler.time_handler(
					    data & 0xffffff);
					break;
				case FIFO_WIFI:
					cb->handler.wifi_handler(
					    FIFO_WIFI_GET_CMD(data),
					    FIFO_WIFI_GET_DATA(data));
					break;
				case FIFO_MIC:
					cb -> handler.mic_handler();
					break;
				case FIFO_POWER:
					cb->handler.power_handler(
					    FIFO_POWER_GET_CMD(data),
					    FIFO_POWER_GET_DATA(data));
					break;
				default:
					break;
				}
			}
		}
	}

	return IRQ_HANDLED;
}

int register_fifocb(struct fifo_cb *cb)
{
	down(&cblist_mtx);
	list_add(&cb->list, &cblist);
	up(&cblist_mtx);
	return 0;
}

int unregister_fifocb(struct fifo_cb *cb)
{
	down(&cblist_mtx);
	list_del(&cb->list);
	up(&cblist_mtx);
	return 0;
}

static int __init ndsfifo_init(void)
{
	printk("NDS FIFO driver\n");

	if (request_irq(IRQ_RECV, ndsfifo_interrupt, 0, "fifo", NULL)) {
		printk(KERN_ERR "fifo.c: Can't allocate irq %d\n", IRQ_RECV);
		return -EBUSY;
	}

	NDS_REG_IPCFIFOCNT = FIFO_ENABLE | FIFO_IRQ_ENABLE | FIFO_CLEAR | FIFO_ERROR;
	return 0;
}

static void __exit ndsfifo_exit(void)
{
	free_irq(IRQ_RECV, ndsfifo_interrupt);
}

module_init(ndsfifo_init);
module_exit(ndsfifo_exit);
