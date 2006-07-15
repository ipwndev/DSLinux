/*
 *  linux/drivers/mmc/scsd_c.c - Supercard SD driver
 *
 *  Copyright (C) 2006 Amadeus, All Rights Reserved.
 *  Based on the old non-mmc driver by by Jean-Pierre Thomasset.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This driver must be in Main Memory or in the first 16 MBytes of GBA Slot ROM.
 * The 2nd 16 MBytes of Slot ROM are switched to IO mode.
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
#include <linux/mmc/card.h>
#include <linux/mmc/protocol.h>
#include <linux/kmod.h>

#include <asm/dma.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/scatterlist.h>
#include <asm/sizes.h>

#undef   READ_CRC	/* do CRC for data reads */
#undef   HALT_ON_ERROR	/* stop after error, so we can see what's happened */

#ifdef CONFIG_MMC_DEBUG
#define READ_CRC
#define HALT_ON_ERROR
#define DBG(x...)	printk(x)
#else
#define DBG(x...)	do { } while (0)
#endif

#ifdef HALT_ON_ERROR
#define halt() do { } while (1)
#else
#define halt() do { } while (0)
#endif

#define DRIVER_NAME	"scsd"
#define DRIVER_VERSION	"1.0.1"

/*****************************************************************************/
/* IO registers */
#define SC_SD_CMD	0x09800000
	/* bit 0: data bit to read  		*/
	/* bit 7: data bit to write 		*/

#define SC_SD_DATAWRITE 0x09000000
#define SC_SD_DATAREAD  0x09100000
#define SC_SD_LOCK      0x09FFFFFE
	/* bit 0: 1				*/
	/* bit 1: enable IO interface (SD,CF)	*/
	/* bit 2: enable R/W SDRAM access 	*/

/*****************************************************************************/

struct scsd_host {
	struct mmc_host		*mmc;
	u8			inactive;
};

/*****************************************************************************/
/* Assembler functions in scsd_s.S */
extern void sd_crc16_s(u16* buff, u16 num, u16* crc16buff);
extern void sd_data_write_s(u16 *buff, u32 length);
extern void sd_data_read_s(u16 *buff, u32 length);

/*****************************************************************************/

/* Unlock the SD interface. */
inline static void scsd_unlock( void )
{
	writew(0xa55a, SC_SD_LOCK);
	writew(0xa55a, SC_SD_LOCK);
	/* we switch GBA ROM to r/w here */
	writew(0x0007, SC_SD_LOCK);
	writew(0x0007, SC_SD_LOCK);
}

/* Improved CRC7 function provided by cory1492 */
inline static u8 scsd_crc7(u8* data, int cnt)
{
    int i;
    u8 crc, temp;

    crc = 0;
    for ( ; cnt; cnt--)
    {
        temp = *data++;
        for (i = 0; i < 8; i++)
        {
            crc <<= 1;
            if ((temp & 0x80) ^ (crc & 0x80)) crc ^= 0x09;
            temp <<= 1;
        }
    }
    crc = (crc << 1) | 1; /* stop bit */
    return(crc);
} 

/* write a single byte of a SD command */
inline static void scsd_write_command_byte(u32 data)
{
	/* With every 16 bit write to SC_SD_CMD, write a single bit. */
	/* We are writing 32 bit == 2 bits of the command per cycle. */
	/* The bit to write is in D7 of SC_SD_CMD. */
	u32 ioadr = SC_SD_CMD;
	data |= data << 17;
	writel(data, ioadr);
	data <<= 2;
	writel(data, ioadr);
	data <<= 2;
	writel(data, ioadr);
	data <<= 2;
	writel(data, ioadr);
}

/* read a single byte of a SD response */
static u32 scsd_read_response_byte(void)
{
	/* With every 16 bit read to SC_SD_CMD, read a single bit. */
        /* So we read 32 bits and get 2 data bits. */
	u32 ioadr = SC_SD_CMD;
	u32 temp;	
	u32 res = 0;
	temp = readl(ioadr);
	if (temp & 0x00000001)
		res |= 0x80;
	if (temp & 0x00010000)
		res |= 0x40;
	temp = readl(ioadr);
	if (temp & 0x00000001)
		res |= 0x20;
	if (temp & 0x00010000)
		res |= 0x10;
	temp = readl(ioadr);
	if (temp & 0x00000001)
		res |= 0x08;
	if (temp & 0x00010000)
		res |= 0x04;
	temp = readl(ioadr);
	if (temp & 0x00000001)
		res |= 0x02;
	if (temp & 0x00010000)
		res |= 0x01;
	return res;
}

/* Wait until ready on the data lines */
/* return 0 if ready, 1 if busy */
static int scsd_waitready(void)
{
	u32 ioadr = SC_SD_DATAREAD;
	u32 tries = 2000000; /* app. 1000 ms */
	u32 count = 0;

	/* skip max. 2 Z-bits */
	readl(ioadr);

	do {
		if (readw(ioadr) & 0x0100) {
			/* data line is high */
			count++;
			if (count > 8) {
				/* minimum 8 bit not busy */
				return 0;
			}
		} else {				
			/* data line is low */
			count = 0;
			tries--;
			if (!tries) {
				/* busy for a very long time */
				return 1;
			}
		}		
	} while (1);
}

/* send a SD command */
/* @return 0 for success, != 0 otherwise */
static int scsd_send_cmd(struct mmc_command *cmd)
{
	u8 databuff[6];
	u8 *p = &databuff[0];
	u32 ioadr = SC_SD_CMD;
	int i;

	/* Start without error */
	cmd->error = MMC_ERR_NONE;

	/* write command and arg into command buffer, append crc */
	*p++ = cmd->opcode | 0x40;
	*p++ = cmd->arg >> 24; 
	*p++ = cmd->arg >> 16; 
	*p++ = cmd->arg >> 8; 
	*p++ = cmd->arg; 
	*p   = scsd_crc7(databuff, 5);
	
	/* wait for CMD line high */
	i = 500000; /* app. 250 ms */
	while (!(readw(ioadr) & 0x0001) && i) i--;
	if (!i) {
		printk( KERN_ERR "CMD:timeout after command\n");
		cmd->error = MMC_ERR_TIMEOUT;
		halt();
		return -1;
	}

	/* one extra wait clk needed here ... */
	readw(ioadr);

	// output command
	p = &databuff[0];
	for (i = 6; i; i--) {
		scsd_write_command_byte(*p++);
	}

	/* what type of response do we get? */
	switch (cmd->flags & MMC_RSP_MASK) {

	case MMC_RSP_NONE:		/* no response */
		break;

	case MMC_RSP_SHORT:		/* 48 bit = 6 bytes response */
	case MMC_RSP_LONG:		/* 136 bit response */

		// skip two Z bits
		readl(ioadr);

		/* Skip response, if it is a read data command.
                   Response and data may come the same time. */
		switch (cmd->opcode) {
		case MMC_READ_SINGLE_BLOCK:
		case MMC_READ_MULTIPLE_BLOCK:
		case SD_APP_SEND_SCR:
			cmd->resp[0] = 0x00000B20; /* fake response */
			return 0;
		}  

		/* Wait for start bit of response */
		i = 100; /* Ncr == 64 */
		while ((readw(ioadr) & 0x0001) && i) i--;
		if (!i) {
			/* Timeout error */
			printk(KERN_ERR "Timeout after Command\n");
			cmd->error = MMC_ERR_TIMEOUT;
			return -1;
		}

		/* Skip rest of first byte (the command) */
		i=7; while (i--) readw(ioadr);

		/* get response */
		cmd->resp[0]  = scsd_read_response_byte() << 24;
		cmd->resp[0] |= scsd_read_response_byte() << 16;
		cmd->resp[0] |= scsd_read_response_byte() << 8;
		cmd->resp[0] |= scsd_read_response_byte();
		if ((cmd->flags & MMC_RSP_MASK) == MMC_RSP_LONG) {
			cmd->resp[1]  = scsd_read_response_byte() << 24;
			cmd->resp[1] |= scsd_read_response_byte() << 16;
			cmd->resp[1] |= scsd_read_response_byte() << 8;
			cmd->resp[1] |= scsd_read_response_byte();
			cmd->resp[2]  = scsd_read_response_byte() << 24;
			cmd->resp[2] |= scsd_read_response_byte() << 16;
			cmd->resp[2] |= scsd_read_response_byte() << 8;
			cmd->resp[2] |= scsd_read_response_byte();
			cmd->resp[3]  = scsd_read_response_byte() << 24;
			cmd->resp[3] |= scsd_read_response_byte() << 16;
			cmd->resp[3] |= scsd_read_response_byte() << 8;
			cmd->resp[3] |= scsd_read_response_byte();
		}
		/* Skip CRC and 2 Z bits */
		i=5; while (i--) readl(ioadr);

		DBG("Got native response %X %X %X %X\n", cmd->resp[0], cmd->resp[1], cmd->resp[2], cmd->resp[3]);
		break;
	default:
		printk(KERN_ERR "Invalid flags code from mmc layer: %d\n", cmd->flags);
		cmd->error = MMC_ERR_INVALID;
		halt();
		return -1;
	}

	/* Busy command? (== STOP) */
	if (cmd->flags & MMC_RSP_BUSY) {

		/* Wait until ready on the data lines */
		if (scsd_waitready()) {
			/* Timeout error */
			printk(KERN_ERR "Busy: timeout waiting for not busy\n");
			cmd->error = MMC_ERR_TIMEOUT;
			halt();
			return -1;
		}
	}

	/* success */
	cmd->error = MMC_ERR_NONE;
	return 0;
}

/*
 * Transfer data blocks from/to sd card.
 */
static void scsd_xfer( struct mmc_data *data )
{
	int sgindex;
	int size = data->blocks << data->blksz_bits;
	struct scatterlist *sg = data->sg;
	int i;
	u32 ioadr;
	u32 tmp;
	u16 crc16[4];

     	DBG("%s Bytes=%d\n",__FUNCTION__, size);

	/* Iterate through the s/g list */
	data->bytes_xfered = 0;
	data->error = MMC_ERR_NONE;
	for (sgindex = 0; sgindex < data->sg_len; sgindex++) {
		u16 *p;
		int sglength;

		/* get the start address */
		p = (page_address(sg[sgindex].page)+sg[sgindex].offset);

		/* compute the length */
		sglength = sg[sgindex].length;
		if (size < sglength)
			sglength = size;

		/* transfer the physical sectors */
		while (sglength > 0) {
			int length = sglength;
			/* break down to sector length */
			if (length > 512)
				length = 512;
			
			/* do the transfer */
			if (data->flags & MMC_DATA_READ) {
				u16 crcread[4];

				ioadr = SC_SD_DATAREAD;

				/* test for data */
				i = 200000; /* app. 100 ms */
				while ((readw(ioadr) & 0x0100) && i) i--;
				if (!i) {
					/* Timeout error */
					printk(KERN_ERR "Read: timeout waiting for read data\n");
					data->error = MMC_ERR_TIMEOUT;
					halt();
					return;
				}
		
				/* read the data */
				sd_data_read_s(p, length);

				/* read the CRC */
				sd_data_read_s(crcread, 8);

				/* read the end bit */
				readw(ioadr);
#ifdef READ_CRC
				/* check the CRC */
				sd_crc16_s(p, length, crc16);
#endif
				/* advance data pointer */
				p += (length/2);
#ifdef READ_CRC
				for (i = 0; i < 4; i++) {
					if (crc16[i] != crcread[i]) {
						printk (KERN_ERR "Read: CRC read different\n");
						data->error = MMC_ERR_BADCRC;
						halt();
						return;
					}
				} 
#endif
			} else if (data->flags & MMC_DATA_WRITE) {

				/* Wait until ready on the data lines */
				if (scsd_waitready()) {
					/* Timeout error */
					printk(KERN_ERR "before Write: timeout waiting for ready\n");
					data->error = MMC_ERR_TIMEOUT;
					halt();
					return;
				}
				ioadr = SC_SD_DATAWRITE;

				/* calculate the crc */
				sd_crc16_s(p, length, crc16);

				/* write the start bit */
				writew(0, ioadr);

				/* write the data */
				sd_data_write_s(p, length);

				/* write the crc */
				sd_data_write_s(crc16, 8);

				/* write the end bit */
				writew(0xFF, ioadr);

				ioadr = SC_SD_DATAREAD;

				/* skip two Z bits */
				readl(ioadr);

				i = 500000; /* app. 250 ms */
				while ((readw(ioadr) & 0x0100) && i) i--;
				if (!i) {
					/* Timeout error */
					printk(KERN_ERR "Timeout waiting for CRC response\n");
					data->error = MMC_ERR_TIMEOUT;
					halt();
					return;
				}

				/* read CRC response (3 bit) + end bit */
				readl(ioadr);
				tmp = readl(ioadr);
				tmp >>= 16;
				/* CRC response is in bit 12,8,4 here */
				i = 0;
				if (tmp & 0x1000) i |= 0x04;
				if (tmp & 0x0100) i |= 0x02;
 				if (tmp & 0x0010) i |= 0x01;
				DBG("CRC Response = %d\n", i);
				/* found that SD cards give response codes 2 AND 3, without error */
				if ((i != 2) && (i != 3)) {
					printk(KERN_ERR "Write: CRC bad code= %d tmp= %X\n", i, tmp);
					data->error = MMC_ERR_BADCRC;
					halt();
					return;				
				}					
			
				/* Wait until ready on the data lines */
				if (scsd_waitready()) {
					/* Timeout error */
					printk(KERN_ERR "Write: timeout waiting for ready\n");
					data->error = MMC_ERR_TIMEOUT;
					halt();
					return;
				}

				/* advance data pointer */
				p += (length/2);
			}
	
			/* ready with one physical sector */
			data->bytes_xfered += length;
			size -= length;
			sglength -= length;
		}

		/* stop if all done */
		if (size <= 0) break;
	}

	DBG("xfer done\n");
}

static void scsd_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct scsd_host *host = mmc_priv(mmc);

	u32 ioadr = SC_SD_CMD;
	int ret;
	int retry = 1;

    	DBG("%s Opcode %d, arg %d\n",__FUNCTION__, mrq->cmd->opcode, mrq->cmd->arg);

	/* Fooling mmc core to only do 4bit data transfers.
	   We don't have a function for doing 1bit data transfers. */
	if (mmc->card_selected) {
		mmc->card_selected->scr.bus_widths |= SD_SCR_BUS_WIDTH_4;
	}

	switch (mrq->cmd->opcode){
	/* do a retry for data I/O */
	case MMC_READ_SINGLE_BLOCK:
	case MMC_READ_MULTIPLE_BLOCK:
	case MMC_WRITE_BLOCK:
	case MMC_WRITE_MULTIPLE_BLOCK:
		retry = 3;
		break;
	/* check if we have an inactivation command */
	case SD_APP_OP_COND:
	case MMC_GO_INACTIVE_STATE:
		host->inactive = 1;
		break;
	/* MMC_ALL_SEND_CID: fake a timeout for the second card */
	/* (maybe better if we know to switch CS on/off) */
	case MMC_ALL_SEND_CID:
		if (!host->inactive) {
			DBG("Fake Timeout for second SEND_CID\n");
			mrq->cmd->error = MMC_ERR_TIMEOUT;
		    	mmc_request_done(mmc, mrq);
			return;
		}
		break;
	}

	while (retry--) {
		/* Send the command to the card */
		mrq->cmd->error = MMC_ERR_NONE;
		ret = scsd_send_cmd(mrq->cmd);
		if (ret) 
			continue;

		/* check if we have an activation */
		if (mrq->cmd->opcode == MMC_SET_RELATIVE_ADDR)
			host->inactive = 0;

		if ( mrq->data ) {
			mrq->data->error = MMC_ERR_NONE;
			scsd_xfer( mrq->data );
			if (mrq->stop) {
				mrq->stop->error = MMC_ERR_NONE;
				ret = scsd_send_cmd(mrq->stop);
				if (ret)
					continue;
			}
			if (mrq->data->error != MMC_ERR_NONE)
				continue;
		}
		break;
	}

	/* minimum 8 bits after last command */
	ret=4; while (ret--) readl(ioadr);

    	mmc_request_done(mmc, mrq);
}

/* read the write protect switch */
static int scsd_get_ro(struct mmc_host *mmc)
{
	DBG("%s\n",__FUNCTION__);
	/* can't distinguish between ro and rw */
	return 0;
}

/* set operating conditions for the host */
static void scsd_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
#if 0
	struct dsmemsd_host *host = mmc_priv(mmc);
    	DBG("%s\n",__FUNCTION__);

    	DBG("bus_mode    = %x\n", ios->bus_mode);
	DBG("chip_select = %x\n", ios->chip_select);
    	DBG("bus_width   = %x\n", ios->bus_width);

	DBG("scsd_set_ios: clock %u power %u vdd %u.%01u\n",
	    ios->clock, ios->power_mode, (ios->vdd-4+17) / 10,
	    (ios->vdd-4+17) % 10);
#endif

	/* ignore power mode changes - we are always on */
	/* ignore chip select changes for now - don't know how to change */
	/* ignore clock - only one speed available */
	/* ignore vdd - only 3.3 volts available */
	/* ignore bus width - only 4 bit available */
	/* ignore bus mode - only push-pull available */
}

static struct mmc_host_ops scsd_ops = {
 	.request	= scsd_request,
	.get_ro		= scsd_get_ro,
	.set_ios	= scsd_set_ios,
};

/* setup the SD host controller */
static int scsd_probe(struct device *dev)
{
	struct mmc_host *mmc;
	struct scsd_host *host = NULL;
    	DBG("%s\n",__FUNCTION__);

        /* allocate host data structures */
	mmc = mmc_alloc_host(sizeof(struct scsd_host), dev);
	if (!mmc) {
		return -ENOMEM;
	}

	/* unlock the SD interface */
	scsd_unlock();

	/* populate host data structures */
	mmc->ops = &scsd_ops;
	mmc->f_min = 25000000;
	mmc->f_max = 25000000;
	mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->caps = MMC_CAP_4_BIT_DATA;
	mmc->mode = MMC_MODE_SD;

	/* As we are doing transfers in software, we have no real limits.
	   So use some big numbers here. */
	mmc->max_seg_size = 0x10000;
	mmc->max_hw_segs  = 0x100;	
	mmc->max_phys_segs= 0x100;	
	mmc->max_sectors  = 0x100;	

	host = mmc_priv(mmc);
	host->mmc = mmc;

	dev_set_drvdata(dev, mmc);
	mmc_add_host(mmc);

	return 0;
}

static int scsd_remove(struct device *dev)
{
	struct mmc_host *mmc = dev_get_drvdata(dev);
	dev_set_drvdata(dev, NULL);

	if (mmc) {
		mmc_remove_host(mmc);
		mmc_free_host(mmc);
	}
	return 0;
}

/* no power managment here */
#define scsd_suspend	NULL
#define scsd_resume	NULL

static void scsd_platform_release(struct device * device){
	/* never */
}

static struct platform_device scsd_device = {
    .name = DRIVER_NAME,
    .dev = {
        .release = scsd_platform_release,
        }
};

static struct device_driver scsd_driver = {
	.name		= DRIVER_NAME,
	.bus		= &platform_bus_type,
	.probe		= scsd_probe,
	.remove		= scsd_remove,
	.suspend	= scsd_suspend,
	.resume		= scsd_resume,
};

static int __init scsd_init(void)
{
    int ret;
	ret = driver_register(&scsd_driver);
    if (!ret) {
        ret = platform_device_register( &scsd_device );
    }
    return ret ;
}

static void __exit scsd_exit(void)
{
	driver_unregister(&scsd_driver);
}

module_init(scsd_init);
module_exit(scsd_exit);

MODULE_DESCRIPTION("Supercard SD Driver");
MODULE_LICENSE("GPL");
