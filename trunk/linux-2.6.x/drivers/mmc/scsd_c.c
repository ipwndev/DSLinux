/*
 *  linux/drivers/mmc/scsd_c.c - Nintendo DS SD driver
 *
 *  Copyright (C) 2006 Amadeus, All Rights Reserved.
 *  Based on the old non-mmc driver by by Jean-Pierre Thomasset.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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
#include <linux/timer.h>

#include <asm/dma.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/scatterlist.h>
#include <asm/sizes.h>

#undef  READ_CRC	/* do CRC for data reads */
#undef  HALT_ON_ERROR	/* stop after error, so we can see what's happened */

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

#define BLOCKSIZE 	512

#define DRIVER_NAME	"scsd"
#define DRIVER_VERSION	"1.1.2"

/*****************************************************************************/

struct scsd_host {
	struct mmc_host		*mmc;
	u8			inactive;
};

/*****************************************************************************/
/* Structure for device-specific functions in assembler.
*/
struct sd_functions {
	/* Test if the card is present.
	   Return != 0 if present. */
	int  	(*detect_card) (void);

	/* Say if the data transfer is 1 or 4 bit.
	   Return != 0 if 4 bit. */
	int     (*transfer_nibbles) (void);

	/* Write at minimum 8 clock cycles to the card.
	   Wait max. 1ms for the CMD line to become HIGH.
	   Send a command to the card. Skip 2 Z bits.
           data = pointer to start of command. 32bit aligned.
           len  = length of the command (including CRC7).
	   Return != 0 if OK, 0 if timeout. */
	int 	(*send_command)	(u8 *data, int len);

	/* Wait max. 1ms for the CMD line to become LOW.
 	   Read a response from the device and skip 2 Z bits. 
           data = pointer to start of response buffer. 32bit aligned.
           len  = number of bytes to read(including CRC).
	   Return != 0 if OK, 0 if timeout. */
	int	(*read_response) (u8* data, int len);

	/* Wait for the DATA line to become HIGH.
	   Return != 0 if OK, 0 if timeout.
	   Maximum length of testing is 1ms. */
	int 	(*wait_ready) (void);

	/* Write a start bit, send a Data block incl. CRC.
	   Write the end bit. Skip 2 Z bits.
           Wait max. 1ms for the start of the CRC response.
	   Check the CRC response.
	   data = pointer to start of data. 32bit aligned.
	   len  = number of bytes to send.
	   Return != 0 if OK, 0 if CRC missing or error. */
	int     (*send_data) (u8 *data, int len);

	/* Wait max 1ms for the DATA line to become LOW.
	   Receive a Data block and CRC, skip the end bit.
	   data = pointer to start of data. 32bit aligned.
	   len  = number of bytes to receive (incl. CRC) 
	   Return != 0 if OK, 0 if timeout. */
	int     (*read_data) (u8 *data, int len);

	/* Read the write protect switch.
	   Return != 0 if readonly, 0 if r/w.
	   When in doubt, return 0. */
	int	(*read_only) (void);

	/* Write at minimum 8 clock cycles to the card. */
	void	(*send_clocks) (void);
};

/*****************************************************************************/
/* Active link to device-specific functions.
*/
static struct sd_functions *sd_dev;

/*****************************************************************************/
/* Timer delayed execution of last clocks */
static volatile int sd_active;
static struct timer_list clock_timer;

/*****************************************************************************/
/* Buffer in main memory(!), for transfer of commands, responses and data
   between the C and assembler part of this driver. This buffer is 32bit aligned.
   This buffer holds data and CRC (two times for reading).
*/
static u8 sd_databuf[BLOCKSIZE+16] __attribute__ ((aligned (4)));

/*****************************************************************************/
/* Device-specific functions.
*/
#ifdef CONFIG_MMC_SCSD
/* Supercard SD(mini,lite) */
extern struct sd_functions supercard_functions;
#endif

/*****************************************************************************/

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

/* Calculate the data crc.
   data = pointer to start of data. 32bit aligned.
   len  = number of bytes of CRC calculation.
   crc  = pointer to start of CRC. 
*/
/* non-static */ void scsd_calc_crc(u8 *data, int len, u8* crc)
{
	__asm__ __volatile__(					
"	mov	r9,%2		\n"
"                               \n"
"	mov	r3,#0           \n"
"	mov	r4,#0           \n"          
"	mov	r5,#0           \n"          
"	mov	r6,#0           \n"          
"                               \n"
"	ldr	r7,=0x80808080  \n"
"	ldr	r8,=0x1021      \n"          
"	mov	%1,%1,lsl #3    \n"          
"1:                             \n"
"	tst	r7,#0x80        \n"          
"	ldrneb	%2,[%0],#1      \n"       
"                               \n"
"	mov	r3,r3,lsl #1    \n"
"	tst	r3,#0x10000     \n"          
"	eorne	r3,r3,r8        \n"        
"	tst	%2,r7,lsr #24   \n"          
"	eorne	r3,r3,r8        \n"        
"	                        \n"      
"	mov	r4,r4,lsl #1    \n"          
"	tst	r4,#0x10000     \n"          
"	eorne	r4,r4,r8        \n"        
"	tst	%2,r7,lsr #25   \n"          
"	eorne	r4,r4,r8        \n"        
"	                        \n"      
"	mov	r5,r5,lsl #1    \n"          
"	tst	r5,#0x10000     \n"          
"	eorne	r5,r5,r8        \n"        
"	tst	%2,r7,lsr #26   \n"          
"	eorne	r5,r5,r8        \n"        
"	                        \n"      
"	mov	r6,r6,lsl #1    \n"          
"	tst	r6,#0x10000     \n"          
"	eorne	r6,r6,r8        \n"        
"	tst	%2,r7,lsr #27   \n"          
"	eorne	r6,r6,r8        \n"        
"                               \n"
"	mov	r7,r7,ror #4    \n"
"	subs	%1,%1,#4        \n"         
"       bne     1b              \n"
"                               \n"
"	mov	%2,r9           \n"
"	mov	r8,#16          \n"          
"2:                             \n"
"	mov	r7,r7,lsl #4    \n"          
"	tst	r3,#0x8000      \n"          
"	orrne	r7,r7,#8        \n"        
"	tst	r4,#0x8000      \n"          
"	orrne	r7,r7,#4        \n"        
"	tst	r5,#0x8000      \n"          
"	orrne	r7,r7,#2        \n"        
"	tst	r6,#0x8000      \n"          
"	orrne	r7,r7,#1        \n"        
"                               \n"
"	mov	r3,r3,lsl #1    \n"
"	mov	r4,r4,lsl #1    \n"          
"	mov	r5,r5,lsl #1    \n"          
"	mov	r6,r6,lsl #1    \n"          
"                               \n"
"	sub	r8,r8,#1        \n"
"	tst	r8,#1           \n"          
"	streqb	r7,[%2],#1	\n"
"	cmp	r8,#0           \n"          
"	bne	2b              \n"          
"                               \n"
	: /* no output registers */					
	: "r" (data), "r" (len), "r" (crc)
	: "r3", "r4", "r5", "r6", "r7", "r8", "r9", "cc");
}


/* Wait until ready on the data lines */
/* return 0 if ready, 1 if busy */
inline static int scsd_waitready(void)
{
	int i;

	for (i = 1000; i; i--) {
		if (sd_dev->wait_ready())
			return 0;
	}
	return 1;
}

/* send a SD command */
/* @return 0 for success, != 0 otherwise */
static int scsd_send_cmd(struct mmc_command *cmd)
{
	u8 *p = sd_databuf;
	int i;

	/* stop the clock timer, we will send a command */
	del_timer(&clock_timer);

	/* Start without error */
	cmd->error = MMC_ERR_NONE;

	/* write command and arg into command buffer, append crc */
	*p++ = cmd->opcode | 0x40;
	*p++ = cmd->arg >> 24; 
	*p++ = cmd->arg >> 16; 
	*p++ = cmd->arg >> 8; 
	*p++ = cmd->arg; 
	*p   = scsd_crc7(sd_databuf, 5);
	
	/* waiting for CMD line high, write command to card */
	for (i = 250; i; i--) {
		if (sd_dev->send_command(sd_databuf, 6))
			break;
	}
	if (!i) {
		printk( KERN_ERR "CMD:Timeout before command\n");
		cmd->error = MMC_ERR_TIMEOUT;
		halt();
		return -1;
	}

	/* what type of response do we get? */
	switch (cmd->flags & MMC_RSP_MASK) {

	case MMC_RSP_NONE:		/* no response */
		break;

	case MMC_RSP_SHORT:		/* 48 bit = 6 bytes response */
	case MMC_RSP_LONG:		/* 136 bit response */

		/* Skip response, if it is a read data command.
                   Response and data may come the same time. */
		switch (cmd->opcode) {
		case MMC_READ_SINGLE_BLOCK:
		case MMC_READ_MULTIPLE_BLOCK:
		case SD_APP_SEND_SCR:
			cmd->resp[0] = 0x00000B20; /* fake response */
			return 0;
		}  

		/* Read the response */
		if ((cmd->flags & MMC_RSP_MASK) == MMC_RSP_LONG)
			i = 18;
		else
			i = 6;
		if (!sd_dev->read_response(sd_databuf, i)) {
			/* Timeout error */
			printk(KERN_ERR "CMD:Timeout after command\n");
			cmd->error = MMC_ERR_TIMEOUT;
			return -1;
		}
		p = sd_databuf;
		p++; /* skip command byte */
		cmd->resp[0]  = *p++ << 24;
		cmd->resp[0] |= *p++ << 16;
		cmd->resp[0] |= *p++ << 8;
		cmd->resp[0] |= *p++;
		if ((cmd->flags & MMC_RSP_MASK) == MMC_RSP_LONG) {
			cmd->resp[1]  = *p++ << 24;
			cmd->resp[1] |= *p++ << 16;
			cmd->resp[1] |= *p++ << 8;
			cmd->resp[1] |= *p++;
			cmd->resp[2]  = *p++ << 24;
			cmd->resp[2] |= *p++ << 16;
			cmd->resp[2] |= *p++ << 8;
			cmd->resp[2] |= *p++;
			cmd->resp[3]  = *p++ << 24;
			cmd->resp[3] |= *p++ << 16;
			cmd->resp[3] |= *p++ << 8;
			cmd->resp[3] |= *p++;
		}
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

     	DBG("%s Bytes=%d\n",__FUNCTION__, size);

	/* Iterate through the s/g list */
	data->bytes_xfered = 0;
	data->error = MMC_ERR_NONE;
	for (sgindex = 0; sgindex < data->sg_len; sgindex++) {
		u8 *p;
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

				/* wait max. 100ms for start bit, read the data+crc+end bit */
				for( i=100; i; i--) {
					if (sd_dev->read_data(sd_databuf, length+8))
						break;
				}
				if (!i) {
					/* Timeout error */
					printk(KERN_ERR "Read: timeout waiting for read data\n");
					data->error = MMC_ERR_TIMEOUT;
					halt();
					return;
				}
#ifdef READ_CRC
				/* check the CRC */
				scsd_calc_crc(sd_databuf, length, &sd_databuf[BLOCKSIZE+8]);
				for (i = 0; i < 8; i++) {
					if (sd_databuf[length+i] != sd_databuf[BLOCKSIZE+8+i]) {
						printk (KERN_ERR "Read: CRC read different\n");
						data->error = MMC_ERR_BADCRC;
						halt();
						return;
					}
				} 
#endif
				/* copy the data */
				memcpy(p, sd_databuf, length);

			} else if (data->flags & MMC_DATA_WRITE) {

				/* copy the data */
				memcpy(sd_databuf, p, length);

				/* calculate the crc */
				scsd_calc_crc(sd_databuf, length, &sd_databuf[length]);

				/* Wait until ready on the data lines */
				if (scsd_waitready()) {
					/* Timeout error */
					printk(KERN_ERR "before Write: timeout waiting for ready\n");
					data->error = MMC_ERR_TIMEOUT;
					halt();
					return;
				}

				/* Write data, check CRC response */
				if (!sd_dev->send_data(sd_databuf, length+8)) {
					/* Timeout error */
					printk(KERN_ERR "BAD CRC response\n");
					data->error = MMC_ERR_TIMEOUT;
					halt();
					return;
				}
			}
	
			/* ready with one physical sector */
			p += length;
			data->bytes_xfered += length;
			size -= length;
			sglength -= length;
		}

		/* stop if all done */
		if (size <= 0) break;
	}

	if (data->flags & MMC_DATA_WRITE) {
		/* Wait until ready on the data lines */
		if (scsd_waitready()) {
			/* Timeout error */
			printk(KERN_ERR "Write: timeout waiting for ready\n");
			data->error = MMC_ERR_TIMEOUT;
			halt();
			return;
		}
	}
	DBG("xfer done\n");
}

static void scsd_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct scsd_host *host = mmc_priv(mmc);
	int ret;
	int retry = 1;

	/* access to SD in progress */
	sd_active = 1;

    	DBG("%s Opcode %d, arg %d\n",__FUNCTION__, mrq->cmd->opcode, mrq->cmd->arg);

	/* Fooling mmc core to only do 4bit data transfers.
	   if the hardware/the driver can only do 4bit transfers. */
	if (sd_dev->transfer_nibbles() && mmc->card_selected) {
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
			sd_active = 0;
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

	/* access done */
	sd_active = 0;

	/* start timer for the last clock cycles */
	mod_timer(&clock_timer, jiffies + HZ/10);
	
   	mmc_request_done(mmc, mrq);
}

/* send 8 clock bits after last command */
static void scsd_timeout(unsigned long arg)
{
	if (sd_active)
		return;	/* nothing to do, next timeout will come */
	
	/* send 8 clocks after last command */
	sd_dev->send_clocks();
}

/* read the write protect switch */
static int scsd_get_ro(struct mmc_host *mmc)
{
	DBG("%s\n",__FUNCTION__);
	return sd_dev->read_only();
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

	/* Pointer to device-specific functions */
#ifdef CONFIG_MMC_SCSD
	sd_dev = &supercard_functions;
	if (sd_dev->detect_card()) {
		printk(KERN_INFO "Supercard SD detected\n");
		goto success;
	}
#endif	

	/* device not found */
	printk(KERN_INFO "No SD device found\n");
	return -ENODEV;

success:
        /* allocate host data structures */
	mmc = mmc_alloc_host(sizeof(struct scsd_host), dev);
	if (!mmc) {
		return -ENOMEM;
	}

	/* init timer structure */
	init_timer(&clock_timer);
	clock_timer.function = scsd_timeout;

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

MODULE_DESCRIPTION("Nintendo DS SD Driver");
MODULE_LICENSE("GPL");
