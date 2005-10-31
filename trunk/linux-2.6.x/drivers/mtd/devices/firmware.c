/**
 *
 * $Id$
 *
 * Copyright (c) Jochen Schaeuble <psionic@psionic.de>
 * 07/2003      rewritten by Joern Engel <joern@wh.fh-wedel.de>
 * 09/2005      modified for Firmware by Malcolm Parsons <malcolm.parsons@gmail.com>
 *
 */

#include <asm/io.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mtd/mtd.h>

#define ERROR(fmt, args...) printk(KERN_ERR "firmware: " fmt , ## args)

static struct mtd_info *mtdinfo;

int firmware_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{
    u32 * address ;

	if (from + len > mtd->size)
		return -EINVAL;

    /* TODO: replace this with shmipc stuff: */
    address = requestFirmwareFromArm7( from, len );

	memcpy(buf, address, len);

	*retlen = len;
	return 0;
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
		goto out3;
	}

	return 0;

out2:
	kfree(mtdinfo);
out1:
	return ret;
}

int __init init_firmware(void)
{
	register_device("firmware", 256 * 1024);
	return 0;
}

static void __exit cleanup_firmware(void)
{
	unregister_devices();
}

module_init(init_firmware);
module_exit(cleanup_firmware);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Malcolm Parsons <malcolm.parsons@gmail.com>");
MODULE_DESCRIPTION("MTD driver for Firmware");
