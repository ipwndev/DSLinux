/**
 *
 * $Id$
 *
 * Copyright (c) Jochen Schaeuble <psionic@psionic.de>
 * 07/2003      rewritten by Joern Engel <joern@wh.fh-wedel.de>
 * 09/2005      modified for SRAM by Malcolm Parsons <malcolm.parsons@gmail.com>
 *
 */

#include <asm/io.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mtd/mtd.h>

#define ERROR(fmt, args...) printk(KERN_ERR "sram: " fmt , ## args)

static struct mtd_info *mtdinfo;

static void * sram_memcpy(void * dest,const void *src,size_t count)
{
        char *tmp = (char *) dest, *s = (char *) src;

        while (count--)
                *tmp++ = *s++;

        return dest;
}

static void * sram_memset(void * s,int c,size_t count)
{
        char *xs = (char *) s;

        while (count--)
                *xs++ = c;

        return s;
}

int sram_erase(struct mtd_info *mtd, struct erase_info *instr)
{
        u_char *start = (u_char *)mtd->priv;

        if (instr->addr + instr->len > mtd->size)
                return -EINVAL;
        
        sram_memset(start + instr->addr, 0xff, instr->len);

        /* This'll catch a few races. Free the thing before returning :) 
         * I don't feel at all ashamed. This kind of thing is possible anyway
         * with flash, but unlikely.
         */

        instr->state = MTD_ERASE_DONE;

        mtd_erase_callback(instr);

        return 0;
}

int sram_read(struct mtd_info *mtd, loff_t from, size_t len,
                size_t *retlen, u_char *buf)
{
        u_char *start = (u_char *)mtd->priv;

        if (from + len > mtd->size)
                return -EINVAL;
        
        sram_memcpy(buf, start + from, len);

        *retlen = len;
        return 0;
}

int sram_write(struct mtd_info *mtd, loff_t to, size_t len,
                size_t *retlen, const u_char *buf)
{
        u_char *start = (u_char *)mtd->priv;

        if (to + len > mtd->size)
                return -EINVAL;
        
        sram_memcpy(start + to, buf, len);

        *retlen = len;
        return 0;
}

static void unregister_devices(void)
{
        del_mtd_device(mtdinfo);
        iounmap(mtdinfo->priv);
        kfree(mtdinfo);
}

static int register_device(char *name, unsigned long start, unsigned long len)
{
        int ret = -ENOMEM;

        mtdinfo = kmalloc(sizeof(struct mtd_info), GFP_KERNEL);
        if (!mtdinfo)
                goto out1;
        
        memset(mtdinfo, 0, sizeof(struct mtd_info));

        ret = -EIO;
        mtdinfo->priv = ioremap(start, len);
        if (!mtdinfo->priv) {
                ERROR("ioremap failed\n");
                goto out2;
        }


        mtdinfo->name = name;
        mtdinfo->size = len;
        mtdinfo->flags = MTD_CAP_RAM | MTD_ERASEABLE ;
        mtdinfo->erase = sram_erase;
        mtdinfo->point = NULL ; /* XIP from 8bit SRAM? no way */
        mtdinfo->unpoint = NULL ;
        mtdinfo->read = sram_read;
        mtdinfo->write = sram_write;
        mtdinfo->owner = THIS_MODULE;
        mtdinfo->type = MTD_RAM;
        mtdinfo->erasesize = 1024;

        ret = -EAGAIN;
        if (add_mtd_device(mtdinfo)) {
                ERROR("Failed to register new device\n");
                goto out3;
        }

        return 0;       

out3:
        iounmap(mtdinfo->priv);
out2:
        kfree(mtdinfo);
out1:
        return ret;
}

int __init init_sram(void)
{
        *(volatile u32*)0x04000204 &= ~0x8080;
        register_device("sram", 0x0a000000, 64 * 1024);
        return 0;
}

static void __exit cleanup_sram(void)
{
        unregister_devices();
}

module_init(init_sram);
module_exit(cleanup_sram);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Malcolm Parsons <malcolm.parsons@gmail.com>");
MODULE_DESCRIPTION("MTD driver for SRAM");
