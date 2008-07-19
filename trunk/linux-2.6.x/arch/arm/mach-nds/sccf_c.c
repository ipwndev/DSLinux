/*
 * Supercard Compact Flash Adapter for Nintendo DS driver.
 *
 * Copyright (c) 2006 Amadeus
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/hdreg.h>	/* HDIO_GETGEO */
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>	/* invalidate_bdev */
#include <linux/bio.h>

MODULE_LICENSE("GPL");

#define SCCF_MAJOR	3	/* use normal IDE major */

/*
 * Minor number and partition management.
 */
#define SCCF_MINORS	16

/*
 * We do only 512 byte sectors.
 */
#define KERNEL_SECTOR_SIZE	512

/*******************************************************************************/
/* Assembler functions from sccf_s.S */

extern int sccf_detect_card(void);
extern int sccf_transfer(u16 *data);

/*******************************************************************************/

/*
 * The internal representation of our device.
 */
static struct {
	unsigned long num_sectors;	/* number of sectors of the card */
        struct request_queue *queue;    /* The device request queue */
        struct gendisk *gd;             /* The gendisk structure */
} sccf_device;

/* data buffer in main memory */
static u16 cf_data_buf[(KERNEL_SECTOR_SIZE/2)+5];

/* Transfer a block of data with retry */
static int sccf_transfer_retry(void)
{
	int retry;
	for (retry = 1000; retry; retry--) {
		switch (sccf_transfer(cf_data_buf)) {
		case 0: /* all OK */
			return 0;
		case 1: /* timeout waiting for finish */
			printk(KERN_ERR "sccf: Timeout after data\n");
			return 1;
		case 2:	/* retry waiting for ready */
			break;
		case 3: /* timeout after command */
			printk(KERN_ERR "sccf: Timeout after command\n");
			return 1;
		}
	}
	printk(KERN_ERR "sccf: Timeout error\n");
	return 1;
}

/* Read the CF descriptor data */
static unsigned long sccf_identify(void)
{
	cf_data_buf[0] = 0;
	cf_data_buf[1] = 0;
	cf_data_buf[2] = 0;
	cf_data_buf[3] = 0xE0;
	cf_data_buf[4] = 0xEC; /* Identify Device */
	if (sccf_transfer_retry()) {
		// timeout
		printk( KERN_ERR "Timeout in Identify Device\n");
		return 0;
	}	
	return (cf_data_buf[5+7] << 16) | cf_data_buf[5+8];
}

/*
 * Handle an I/O request.
 */
static void sccf_handle(unsigned long sector,
		        unsigned long nsect, 
                        char *buffer, int write)
{
	if (write) {
		for ( ; nsect; nsect--) {
			cf_data_buf[0] = sector & 0xFF;
			cf_data_buf[1] = (sector >> 8) & 0xFF;
			cf_data_buf[2] = (sector >> 16) & 0xFF;
			cf_data_buf[3] = ((sector >> 24) & 0x0F) | 0xE0;
			cf_data_buf[4] = 0x30; /* WRITE */
			memcpy(&cf_data_buf[5], buffer, KERNEL_SECTOR_SIZE);
			if (sccf_transfer_retry()) {
				// timeout
				printk( KERN_ERR "Timeout in write transfer\n");
				return;
			}	
			buffer += KERNEL_SECTOR_SIZE;
			sector++;
 		}
	} else {
		for ( ; nsect; nsect--) {
			cf_data_buf[0] = sector & 0xFF;
			cf_data_buf[1] = (sector >> 8) & 0xFF;
			cf_data_buf[2] = (sector >> 16) & 0xFF;
			cf_data_buf[3] = ((sector >> 24) & 0x0F) | 0xE0;
			cf_data_buf[4] = 0x20; /* READ */
			if (sccf_transfer_retry()) {
				// timeout
				printk( KERN_ERR "Timeout in read transfer\n");
				return;
			}	
			memcpy(buffer, &cf_data_buf[5], KERNEL_SECTOR_SIZE);
			buffer += KERNEL_SECTOR_SIZE;
			sector++;
 		}
	}
}

/*
 * Transfer a single BIO.
 */
static int sccf_xfer_bio(struct bio *bio)
{
	int i;
	struct bio_vec *bvec;
	sector_t sector = bio->bi_sector;

	/* Do each segment independently. */
	bio_for_each_segment(bvec, bio, i) {
		char *buffer = __bio_kmap_atomic(bio, i, KM_USER0);
		sccf_handle(sector, bio_cur_sectors(bio),
			    buffer, bio_data_dir(bio) == WRITE);
		sector += bio_cur_sectors(bio);
		__bio_kunmap_atomic(bio, KM_USER0);
	}
	return 0; /* Always "succeed" */
}

/*
 * The direct make request version.
 */
static int sccf_make_request(request_queue_t *q, struct bio *bio)
{
	int status;

	status = sccf_xfer_bio(bio);
	bio_endio(bio, bio->bi_size, status);
	return 0;
}

/*
 * Open and close.
 */

static int sccf_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int sccf_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * The ioctl() implementation
 */

int sccf_ioctl (struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	struct hd_geometry geo;

	switch(cmd) {
	    case HDIO_GETGEO:
        	/*
		 * Get geometry: since we are a LBA, we have to make
		 * up something plausible.  So we claim 32 sectors, 8 heads,
		 * and calculate the corresponding number of cylinders.
		 */
		geo.cylinders = sccf_device.num_sectors >> 8;
		geo.heads = 8;
		geo.sectors = 32;
		geo.start = 4;
		if (copy_to_user((void __user *) arg, &geo, sizeof(geo)))
			return -EFAULT;
		return 0;
	}

	return -ENOTTY; /* unknown command */
}

/*
 * The device operations structure.
 */
static struct block_device_operations sccf_ops = {
	.owner           = THIS_MODULE,
	.open 	         = sccf_open,
	.release 	 = sccf_release,
	.ioctl	         = sccf_ioctl
};

/* Initialize the driver */
static int __init sccf_init(void)
{
	int i;

	/* Is a sccf detected? */
	if (!sccf_detect_card()) {
		/* no: what else can we do? */
		return -ENODEV;
	}
	printk(KERN_INFO "sccf detected\n");

	/* Get registered. */
	i = register_blkdev(SCCF_MAJOR, "sccf");
	if (i) {
		printk(KERN_ERR "sccf: unable to get major number\n");
		return -EBUSY;
	}

	/* Init device structure */
	memset(&sccf_device, 0, sizeof (sccf_device));
	
	/* Init the block queue */
	sccf_device.queue = blk_alloc_queue(GFP_KERNEL);
	if (sccf_device.queue == NULL) {
		printk(KERN_ERR "sccf: unable to allocate block queue\n");
		unregister_blkdev(SCCF_MAJOR, "sccf");
		return -ENOMEM;
	}
	blk_queue_make_request(sccf_device.queue, sccf_make_request);
	sccf_device.queue->queuedata = &sccf_device;

	/* Init the gendisk structure. */
	sccf_device.gd = alloc_disk(SCCF_MINORS);
	if (!sccf_device.gd) {
		printk (KERN_ERR "sccf: alloc_disk failure\n");
		blk_put_queue(sccf_device.queue);
		unregister_blkdev(SCCF_MAJOR, "sccf");
		return -ENOMEM;
	}
	sccf_device.gd->major = SCCF_MAJOR;
	sccf_device.gd->first_minor = 0;
	sccf_device.gd->fops  = &sccf_ops;
	sccf_device.gd->queue = sccf_device.queue;
	snprintf (sccf_device.gd->disk_name, 32, "hda");

	/* Calculate the size of the CF card */
	sccf_device.num_sectors = sccf_identify();
	set_capacity(sccf_device.gd, sccf_device.num_sectors);

	add_disk(sccf_device.gd);
	return 0;
}

/* unregister the driver */
static void __exit sccf_exit(void)
{
	if (sccf_device.gd) {
		del_gendisk(sccf_device.gd);
		put_disk(sccf_device.gd);
	}
	if (sccf_device.queue) {
		blk_put_queue(sccf_device.queue);
	}
	unregister_blkdev(SCCF_MAJOR, "sccf");
}
	
module_init(sccf_init);
module_exit(sccf_exit);
