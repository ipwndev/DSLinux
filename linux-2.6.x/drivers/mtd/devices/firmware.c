/**
 *
 * $Id$
 *
 * Copyright (c) Jochen Schaeuble <psionic@psionic.de>
 * 07/2003      rewritten by Joern Engel <joern@wh.fh-wedel.de>
 * 09/2005      modified for Firmware by Malcolm Parsons <malcolm.parsons@gmail.com>
 * 11/2005	shmemipc stuff added by Stefan Sperling <stsp@stsp.in-berlin.de>
 * 04/2006	shmemipc stuff removed and converted to use the fifo
 * 		by Stefan Sperling <stsp@stsp.in-berlin.de>
 * 05/2006      ARM9 cache handling added; remove as much as possible
 *              from interrupt callback by Amadeus <amadeus@iksw-muees.de>
 */

#include <asm/io.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <asm/semaphore.h>
#include <linux/wait.h>
#include <linux/mtd/mtd.h>

#include <asm/arch/firmware.h>
#include <asm/arch/fifo.h>
#include <asm/cacheflush.h>

#define ERROR(fmt, args...) printk(KERN_ERR "firmware: " fmt , ## args)

static struct mtd_info *mtdinfo;
static int firmware_data_read;
static wait_queue_head_t wait_queue;
/* This buffer is aligned to the ARM9 cache lines, because of cache line invalidation */
static struct nds_firmware_block firmware_block __attribute__ ((aligned (32)));

int firmware_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{

	if (from + len > mtd->size)
		return -EINVAL;

	if (len > NDS_FIRMWARE_BLOCK_SIZE)
		len = NDS_FIRMWARE_BLOCK_SIZE;

	firmware_data_read = 0;
	firmware_block.from = from;
	firmware_block.len = len;
	firmware_block.destination = buf;
	/* writeback cached data before ARM7 gets hand on. Invalidate cache so
           we can read the new values after ARM7 has written them. */
	dmac_flush_range((unsigned long)&firmware_block,
                        ((unsigned long)&firmware_block)+sizeof(firmware_block)); 

	nds_fifo_send(FIFO_FIRMWARE_CMD(FIFO_FIRMWARE_CMD_READ, 0));

	if(wait_event_interruptible(wait_queue, firmware_data_read)) {
		return -ERESTART;
	}

	/* copy data to caller. Here better than in interrupt callback. */
	memcpy(firmware_block.destination,
  	      &firmware_block.data,
	       firmware_block.len);

	*retlen = len;
	return 0;
}

static void firmware_fifo_cb(u8 cmd)
{
	switch(cmd) {
		case FIFO_FIRMWARE_CMD_READ:
			firmware_data_read = 1;
			wake_up_interruptible(&wait_queue);
			break;
		default:
			break;
	}
}

static void unregister_devices(void)
{
	del_mtd_device(mtdinfo);
	kfree(mtdinfo);
}

static int register_device(char *name, unsigned long len)
{
	int ret = -ENOMEM;

	mtdinfo = kmalloc(sizeof(struct mtd_info), GFP_KERNEL);
	if (!mtdinfo)
		goto out1;

	memset(mtdinfo, 0, sizeof(struct mtd_info));

	mtdinfo->name = name;
	mtdinfo->size = len;
	mtdinfo->flags = MTD_CAP_ROM ;
	mtdinfo->read = firmware_read;
	mtdinfo->owner = THIS_MODULE;
	mtdinfo->type = MTD_RAM;
	mtdinfo->erasesize = 1024;

	ret = -EAGAIN;
	if (add_mtd_device(mtdinfo)) {
		ERROR("Failed to register new device\n");
		goto out2;
	}

	nds_fifo_send(FIFO_FIRMWARE_CMD(FIFO_FIRMWARE_CMD_BUFFER_ADDRESS,
	    (u32)&firmware_block));

	return 0;

out2:
	kfree(mtdinfo);
out1:
	return ret;
}

static struct fifo_cb callback = {
	.type = FIFO_FIRMWARE,
	.handler.firmware_handler = firmware_fifo_cb
};

int __init init_firmware(void)
{
	firmware_data_read = 0;
	register_device("firmware", 256 * 1024);
	init_waitqueue_head(&wait_queue);
	register_fifocb(&callback);
	return 0;
}

static void __exit cleanup_firmware(void)
{
	unregister_devices();
}

module_init(init_firmware);
module_exit(cleanup_firmware);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Malcolm Parsons <malcolm.parsons@gmail.com> / Stefan Sperling <stsp@stsp.in-berlin.de");
MODULE_DESCRIPTION("MTD driver for Firmware");
