/**
 *
 * $Id$
 *
 * Copyright (c) Jochen Schaeuble <psionic@psionic.de>
 * 07/2003      rewritten by Joern Engel <joern@wh.fh-wedel.de>
 * 09/2005      modified for Firmware by Malcolm Parsons <malcolm.parsons@gmail.com>
 * 11/2005	shmemipc stuff added by Stefan Sperling <stsp@stsp.in-berlin.de>
 *
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

#include <asm/arch/shmemipc.h>

#define ERROR(fmt, args...) printk(KERN_ERR "firmware: " fmt , ## args)

static struct mtd_info *mtdinfo;
static int firmware_data_read;
static wait_queue_head_t wait_queue;

int firmware_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{

	if (from + len > mtd->size)
		return -EINVAL;

	if (len > SHMEMIPC_FIRMWARE_DATA_SIZE)
		len = SHMEMIPC_FIRMWARE_DATA_SIZE;

	firmware_data_read = 0;

	shmemipc_lock();
	SHMEMIPC_BLOCK_ARM9->firmware.from = from;
	SHMEMIPC_BLOCK_ARM9->firmware.len = len;
	SHMEMIPC_BLOCK_ARM9->firmware.destination = buf;
	shmemipc_unlock();
	/* 'flush' is not semantically correct here.
	 * We are actually requesting the arm7 to read firmware data
	 * and send us a "flush complete" */
	shmemipc_flush(SHMEMIPC_USER_FIRMWARE);

	if(wait_event_interruptible(wait_queue, firmware_data_read)) {
		return -ERESTART;
	}

	*retlen = len;
	return 0;
}

static void firmware_shmemipc_isr(u8 type)
{
	switch(type) {
		case SHMEMIPC_REQUEST_FLUSH:
			break;
		case SHMEMIPC_FLUSH_COMPLETE:
			memcpy(SHMEMIPC_BLOCK_ARM9->firmware.destination,
			    &SHMEMIPC_BLOCK_ARM9->firmware.data,
			    SHMEMIPC_BLOCK_ARM9->firmware.len);
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

	return 0;

out2:
	kfree(mtdinfo);
out1:
	return ret;
}

static struct shmemipc_cb callback = {
	.user = SHMEMIPC_USER_FIRMWARE,
	.handler.firmware_callback = firmware_shmemipc_isr
};

int __init init_firmware(void)
{
	firmware_data_read = 0;
	register_device("firmware", 256 * 1024);
	init_waitqueue_head(&wait_queue);
	register_shmemipc_cb(&callback);
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
