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
#define DBG(x...)	printk(KERN_DEBUG x)
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
#if 0
	if (readw(M3SD_STAT) & STAT_CLK_EN) {
		unsigned long timeout = 10000;
		unsigned int v;

		writew(STOP_CLOCK, M3SD_STRPCL);

		do {
			v = readw(M3SD_STAT);
			if (!(v & STAT_CLK_EN))
				break;
			udelay(1);
		} while (timeout--);

		if (v & STAT_CLK_EN)
			dev_err(mmc_dev(host->mmc), "unable to stop clock\n");
	}
#endif
}

static void m3sd_setup_data(struct m3sd_host *host, struct mmc_data *data)
{
    printk("%s\n",__FUNCTION__);
#if 0
	unsigned int nob = data->blocks;
	unsigned int timeout;
	u32 dcmd;
	int i;

	host->data = data;

	if (data->flags & MMC_DATA_STREAM)
		nob = 0xffff;

	writew(nob, M3SD_NOB);
	writew(1 << data->blksz_bits, M3SD_BLKLEN);

	timeout = ns_to_clocks(data->timeout_ns) + data->timeout_clks;
	writew((timeout + 255) / 256, M3SD_RDTO);

	if (data->flags & MMC_DATA_READ) {
		host->dma_dir = DMA_FROM_DEVICE;
		dcmd = DCMD_INCTRGADDR | DCMD_FLOWTRG;
		DRCMRTXMMC = 0;
		DRCMRRXMMC = host->dma | DRCMR_MAPVLD;
	} else {
		host->dma_dir = DMA_TO_DEVICE;
		dcmd = DCMD_INCSRCADDR | DCMD_FLOWSRC;
		DRCMRRXMMC = 0;
		DRCMRTXMMC = host->dma | DRCMR_MAPVLD;
	}

	for (i = 0; i < host->dma_len; i++) {
		if (data->flags & MMC_DATA_READ) {
			host->sg_cpu[i].dsadr = host->res->start + M3SD_RXFIFO;
			host->sg_cpu[i].dtadr = sg_dma_address(&data->sg[i]);
		} else {
			host->sg_cpu[i].dsadr = sg_dma_address(&data->sg[i]);
			host->sg_cpu[i].dtadr = host->res->start + M3SD_TXFIFO;
		}
		host->sg_cpu[i].dcmd = dcmd | sg_dma_len(&data->sg[i]);
		host->sg_cpu[i].ddadr = host->sg_dma + (i + 1) *
					sizeof(struct pxa_dma_desc);
	}

#endif
}

static void m3sd_start_cmd(struct m3sd_host *host, struct mmc_command *cmd)
{
    unsigned int stat ;
    unsigned int laststat = 0 ;
    int i;
    printk("%s\n",__FUNCTION__);

#if 0
	if (cmd->flags & MMC_RSP_BUSY)
		cmdat |= CMDAT_BUSY;
#endif

#if 0
	switch (cmd->flags & (MMC_RSP_MASK | MMC_RSP_CRC)) {
	case MMC_RSP_SHORT | MMC_RSP_CRC:
		cmdat |= CMDAT_RESP_SHORT;
		break;
	case MMC_RSP_SHORT:
		cmdat |= CMDAT_RESP_R3;
		break;
	case MMC_RSP_LONG | MMC_RSP_CRC:
		cmdat |= CMDAT_RESP_R2;
		break;
	default:
		break;
	}
#endif

    writew(0x08, M3SD_CTRL);
    printk("opcode = %d\n",cmd->opcode);
    printk("arg = %d\n",cmd->arg);
    printk("flags = %d\n",cmd->flags);

	writew(cmd->opcode | 0x40, M3SD_CMD);
	writew(cmd->arg >> 16, M3SD_ARGH);
	writew(cmd->arg & 0xffff, M3SD_ARGL);

    printk("0x29\n");
	writew(0x29, M3SD_STRPCL);

    for (i = 0 ; i < 10 ; i++ )
    {
        stat = readw(M3SD_STAT);
        if ( laststat != stat )
        {
            printk("stat=%x\n",stat);
            laststat = stat ;
        }
    }

    printk("0x09\n");
    writew(0x09, M3SD_STRPCL);

    for (i = 0 ; i < 10 ; i++ )
    {
        stat = readw(M3SD_STAT);
        if ( laststat != stat )
        {
            printk("stat=%x\n",stat);
            laststat = stat ;
        }
    }
    printk("0x49\n");
    writew(0x49, M3SD_STRPCL);

    for (i = 0 ; i < 10 ; i++ )
    {
        stat = readw(M3SD_STAT);
        if ( laststat != stat )
        {
            printk("stat=%x\n",stat);
            laststat = stat ;
        }
    }
}

static int m3sd_cmd_done(struct m3sd_host *host, unsigned int stat)
{
    printk("%s\n",__FUNCTION__);
#if 0
	struct mmc_command *cmd = host->cmd;
	int i;
	u32 v;

	if (!cmd)
		return 0;

	host->cmd = NULL;

	v = readl(M3SD_RES) & 0xffff;
	for (i = 0; i < 4; i++) {
		u32 w1 = readl(M3SD_RES) & 0xffff;
		u32 w2 = readl(M3SD_RES) & 0xffff;
		cmd->resp[i] = v << 24 | w1 << 8 | w2 >> 8;
		v = w2;
	}

	if (stat & STAT_TIME_OUT_RESPONSE) {
		cmd->error = MMC_ERR_TIMEOUT;
	} else if (stat & STAT_RES_CRC_ERR && cmd->flags & M3SD_RSP_CRC) {
		cmd->error = MMC_ERR_BADCRC;
	}
#endif

	return 1;
}

#if 0
static int m3sd_data_done(struct m3sd_host *host, unsigned int stat)
{
    printk("%s\n",__FUNCTION__);
	struct mmc_data *data = host->data;

	if (!data)
		return 0;

	if (stat & STAT_READ_TIME_OUT)
		data->error = MMC_ERR_TIMEOUT;
	else if (stat & (STAT_CRC_READ_ERROR|STAT_CRC_WRITE_ERROR))
		data->error = MMC_ERR_BADCRC;

	if (data->error == MMC_ERR_NONE)
		data->bytes_xfered = data->blocks << data->blksz_bits;
	else
		data->bytes_xfered = 0;


	host->data = NULL;
	if (host->mrq->stop && data->error == MMC_ERR_NONE) {
		m3sd_stop_clock(host);
		m3sd_start_cmd(host, host->mrq->stop, 0);
	} else {
		m3sd_finish_request(host, host->mrq);
	}

	return 1;
}
#endif

static void m3sd_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct m3sd_host *host = mmc_priv(mmc);
    printk("%s\n",__FUNCTION__);
#if 0
	unsigned int cmdat;

	cmdat = host->cmdat;
	host->cmdat &= ~CMDAT_INIT;

	if (mrq->data) {
		m3sd_setup_data(host, mrq->data);

		cmdat &= ~CMDAT_BUSY;
		cmdat |= CMDAT_DATAEN | CMDAT_DMAEN;
		if (mrq->data->flags & MMC_DATA_WRITE)
			cmdat |= CMDAT_WRITE;

		if (mrq->data->flags & MMC_DATA_STREAM)
			cmdat |= CMDAT_STREAM;
	}

#endif
	m3sd_start_cmd(host, mrq->cmd);
    unsigned stat = readw(M3SD_STAT);

    DBG("M3SD: stat %08x\n", stat);
#if 0
    handled |= m3sd_cmd_done(host, stat);
    handled |= m3sd_data_done(host, stat);
#endif
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
