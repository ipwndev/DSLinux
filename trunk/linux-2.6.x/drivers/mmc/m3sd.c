/*
 *  linux/drivers/mmc/m3sd.c - M3 SD driver
 *
 *  Copyright (C) 2006 Malcolm Parsons, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/host.h>
#include <linux/mmc/protocol.h>

#include <asm/dma.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/scatterlist.h>
#include <asm/sizes.h>

#include "m3sd.h"

#ifdef CONFIG_MMC_DEBUG
#define DBG(x...)	printk(x)
#else
#define DBG(x...)	do { } while (0)
#endif

#define DRIVER_NAME	"m3sd"

#define NR_SG	1

struct m3sd_host {
	struct mmc_host		*mmc;
	spinlock_t		lock;
	int			dma;
	unsigned int		cmdat;
	unsigned int		power_mode;

};

static void m3sd_unlock( void )
{
    volatile u16 tmp ;
    tmp = *(volatile u16 *)0x08000000 ;
    tmp = *(volatile u16 *)0x08E00002 ;
    tmp = *(volatile u16 *)0x0800000E ;
    tmp = *(volatile u16 *)0x08801FFC ;
    tmp = *(volatile u16 *)0x0800104A ;
    tmp = *(volatile u16 *)0x08800612 ;
    tmp = *(volatile u16 *)0x08000000 ;
    tmp = *(volatile u16 *)0x08801B66 ;
    tmp = *(volatile u16 *)0x08800006 ;
    tmp = *(volatile u16 *)0x08000000 ;
}

static void m3sd_stop_clock(struct m3sd_host *host)
{
    printk("%s\n",__FUNCTION__);
}

static void m3sd_send_cmd(struct mmc_command *cmd)
{
    int i;

    printk("%s\n",__FUNCTION__);

    writew(0x08, M3SD_CTRL);
    printk("opcode = %d\n",cmd->opcode);
    printk("arg = %d\n",cmd->arg);
    printk("flags = %d\n",cmd->flags);

    cmd->opcode = 12 ;
	writew(cmd->opcode | 0x40, M3SD_CMD);
	writew(cmd->arg >> 16, M3SD_ARGH);
	writew(cmd->arg & 0xffff, M3SD_ARGL);

    printk("0x29\n");
	writew(0x29, M3SD_STRPCL);

    /* wait for command to complete */
    while ( (readw(M3SD_STAT) & 0x01) == 0 ) ;

    printk("0x09\n");
    writew(0x09, M3SD_STRPCL);

    if ( cmd->flags )
    {
        writew(0x08, M3SD_STRPCL);
        writew(0x04, M3SD_CTRL);

#if 0
        for (i = 0 ; i < 4 ; i++)
        {
            u32 w1 = readw( M3SD_STRPCL ) ;
            u32 w2 = readw( M3SD_STRPCL ) ;
            cmd->resp[i] = (w1 << 16 ) | w2 ;
        }
#endif
    }
}

static void m3sd_xfer( struct mmc_data * data )
{
    u16 crc ;
    u16 ignore ;
    int i;
    printk("%s\n",__FUNCTION__);


    printk("0x49\n");
    writew(0x49, M3SD_STRPCL);
    printk("reading3 %x\n", readw(M3SD_STRPCL)) ;

    while ( (readw(M3SD_STAT) & 0x40) != 0 ) ;

    printk("reading4 %x\n", readw(M3SD_STRPCL)) ;
    writew(0x09, M3SD_STRPCL);

    printk("reading5 %x\n", readw(M3SD_STRPCL)) ;
    writew(0x08, M3SD_STRPCL);

    printk("reading %x\n", readw(M3SD_STRPCL)) ;
    writew(0x04, M3SD_CTRL);

    if ( data->flags & MMC_DATA_READ )
    {
        ignore = readw(M3SD_STRPCL) ;

        for ( i = 0 ; i < 256 ; i++ )
        {
            data = readw(M3SD_STRPCL) ;
        }

        crc = readw(M3SD_STRPCL) ;
        crc = readw(M3SD_STRPCL) ;
        crc = readw(M3SD_STRPCL) ;
        crc = readw(M3SD_STRPCL) ;
    }
    else
    {
        /* TODO */
    }

    data->bytes_xfered = data->blocks << data->blksz_bits;
}

static void m3sd_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct m3sd_host *host = mmc_priv(mmc);

    printk("%s\n",__FUNCTION__);

    /* Send the command to the card */
	m3sd_send_cmd(mrq->cmd);

    unsigned stat = readw(M3SD_STAT);
    DBG("M3SD: stat %08x\n", stat);

    if ( mrq->data )
    {
        m3sd_xfer( mrq->data );
        if (mrq->stop) {
            m3sd_send_cmd(mrq->stop);
        }
    }

    mmc_request_done(mmc, mrq);
}

static int m3sd_get_ro(struct mmc_host *mmc)
{
    printk("%s\n",__FUNCTION__);
	/* assume readonly for now */
	return 1;
}

static void m3sd_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	struct m3sd_host *host = mmc_priv(mmc);
    printk("%s\n",__FUNCTION__);
#if 0
    printk("bus_mode = %x\n", ios->bus_mode);
    printk("chip_select = %x\n", ios->chip_select);
    printk("bus_width = %x\n", ios->bus_width);
#endif

	DBG("m3sd_set_ios: clock %u power %u vdd %u.%02u\n",
	    ios->clock, ios->power_mode, ios->vdd / 100,
	    ios->vdd % 100);

	if (ios->clock) {
        /* TODO */
	} else {
		m3sd_stop_clock(host);
	}

	if (host->power_mode != ios->power_mode) {
		host->power_mode = ios->power_mode;

#if 0
		if (ios->power_mode == MMC_POWER_ON)
			host->cmdat |= CMDAT_INIT;
#endif
	}
}

static struct mmc_host_ops m3sd_ops = {
	.request	= m3sd_request,
	.get_ro		= m3sd_get_ro,
	.set_ios	= m3sd_set_ios,
};

static int m3sd_probe(struct device *dev)
{
	struct mmc_host *mmc;
	struct m3sd_host *host = NULL;
	int ret;
    printk("%s\n",__FUNCTION__);

    m3sd_unlock();

	mmc = mmc_alloc_host(sizeof(struct m3sd_host), dev);
	if (!mmc) {
		ret = -ENOMEM;
		goto out;
	}

	mmc->ops = &m3sd_ops;
#if 0
	mmc->f_min = CLOCKRATE_MIN;
	mmc->f_max = CLOCKRATE_MAX;
#endif

	mmc->max_phys_segs = 1;
	mmc->max_seg_size = PAGE_SIZE;

	host = mmc_priv(mmc);
	host->mmc = mmc;
	host->dma = -1;

	spin_lock_init(&host->lock);

	/*
	 * Ensure that the host controller is shut down, and setup
	 * with our defaults.
	 */
	//m3sd_stop_clock(host);
#if 0
	writew(0, M3SD_SPI);
	writew(64, M3SD_RESTO);
#endif

	dev_set_drvdata(dev, mmc);

	mmc_add_host(mmc);

	return 0;

 out:
	if (mmc)
		mmc_free_host(mmc);
	return ret;
}

static int m3sd_remove(struct device *dev)
{
#if 0
	struct mmc_host *mmc = dev_get_drvdata(dev);

	dev_set_drvdata(dev, NULL);

	if (mmc) {
		struct m3sd_host *host = mmc_priv(mmc);

		if (host->pdata && host->pdata->exit)
			host->pdata->exit(dev, mmc);

		mmc_remove_host(mmc);

		m3sd_stop_clock(host);

		mmc_free_host(mmc);
	}
#endif
	return 0;
}

#define m3sd_suspend	NULL
#define m3sd_resume	NULL

static void m3sd_platform_release(struct device * device){
}

static struct platform_device m3sd_device = {
    .name = "m3sd",
    .dev = {
        .release = m3sd_platform_release,
        }
};

static struct device_driver m3sd_driver = {
	.name		= DRIVER_NAME,
	.bus		= &platform_bus_type,
	.probe		= m3sd_probe,
	.remove		= m3sd_remove,
	.suspend	= m3sd_suspend,
	.resume		= m3sd_resume,
};

static int __init m3sd_init(void)
{
    int ret;
	ret = driver_register(&m3sd_driver);
    if (!ret) {
        ret = platform_device_register( &m3sd_device );
    }
    return ret ;
}

static void __exit m3sd_exit(void)
{
	driver_unregister(&m3sd_driver);
}

module_init(m3sd_init);
module_exit(m3sd_exit);

MODULE_DESCRIPTION("M3 SD Driver");
MODULE_LICENSE("GPL");
