/*
 * M3 Compact Flash Adapter for Nintendo DS driver.
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

#define M3CF_MAJOR	3	/* use normal IDE major */

/*
 * Minor number and partition management.
 */
#define M3CF_MINORS	16

/*
 * We do only 512 byte sectors.
 */
#define KERNEL_SECTOR_SIZE	512

/*******************************************************************************/
/* Assembler functions from m3cf_s.S */

extern int m3cf_detect_card(void);
extern int m3cf_transfer(u16 *data);

/*******************************************************************************/

/*
 * The internal representation of our device.
 */
static struct {
	unsigned long num_sectors;	/* number of sectors of the card */
        struct request_queue *queue;    /* The device request queue */
        struct gendisk *gd;             /* The gendisk structure */
} m3cf_device;

/* data buffer in main memory */
static u16 cf_data_buf[(KERNEL_SECTOR_SIZE/2)+5];

/* Transfer a block of data with retry */
static int m3cf_transfer_retry(void)
{
	int retry;
	for (retry = 1000; retry; retry--) {
		switch (m3cf_transfer(cf_data_buf)) {
		case 0: /* all OK */
			return 0;
		case 1: /* timeout waiting for finish */
			printk(KERN_ERR "m3cf: Timeout after data\n");
			return 1;
		case 2:	/* retry waiting for ready */
			break;
		case 3: /* timeout after command */
			printk(KERN_ERR "m3cf: Timeout after command\n");
			return 1;
		}
	}
	printk(KERN_ERR "m3cf: Timeout error\n");
	return 1;
}

/* Read the CF descriptor data */
static unsigned long m3cf_identify(void)
{
	cf_data_buf[0] = 0;
	cf_data_buf[1] = 0;
	cf_data_buf[2] = 0;
	cf_data_buf[3] = 0xE0;
	cf_data_buf[4] = 0xEC; /* Identify Device */
	if (m3cf_transfer_retry()) {
		// timeout
		printk( KERN_ERR "Timeout in Identify Device\n");
		return 0;
	}	
	return (cf_data_buf[5+7] << 16) | cf_data_buf[5+8];
}

/*
 * Handle an I/O request.
 */
static void m3cf_handle(unsigned long sector,
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
			if (m3cf_transfer_retry()) {
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
			if (m3cf_transfer_retry()) {
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
static int m3cf_xfer_bio(struct bio *bio)
{
	int i;
	struct bio_vec *bvec;
	sector_t sector = bio->bi_sector;

	/* Do each segment independently. */
	bio_for_each_segment(bvec, bio, i) {
		char *buffer = __bio_kmap_atomic(bio, i, KM_USER0);
		m3cf_handle(sector, bio_cur_sectors(bio),
			    buffer, bio_data_dir(bio) == WRITE);
		sector += bio_cur_sectors(bio);
		__bio_kunmap_atomic(bio, KM_USER0);
	}
	return 0; /* Always "succeed" */
}

/*
 * The direct make request version.
 */
static int m3cf_make_request(request_queue_t *q, struct bio *bio)
{
	int status;

	status = m3cf_xfer_bio(bio);
	bio_endio(bio, bio->bi_size, status);
	return 0;
}

/*
 * Open and close.
 */

static int m3cf_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int m3cf_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * The ioctl() implementation
 */

int m3cf_ioctl (struct inode *inode, struct file *filp,
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
		geo.cylinders = m3cf_device.num_sectors >> 8;
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
static struct block_device_operations m3cf_ops = {
	.owner           = THIS_MODULE,
	.open 	         = m3cf_open,
	.release 	 = m3cf_release,
	.ioctl	         = m3cf_ioctl
};

/* Initialize the driver */
static int __init m3cf_init(void)
{
	int i;

	/* Is a M3CF detected? */
	if (!m3cf_detect_card()) {
		/* no: what else can we do? */
		return -ENODEV;
	}
	printk(KERN_INFO "M3CF detected\n");

	/* Get registered. */
	i = register_blkdev(M3CF_MAJOR, "m3cf");
	if (i) {
		printk(KERN_ERR "m3cf: unable to get major number\n");
		return -EBUSY;
	}

	/* Init device structure */
	memset(&m3cf_device, 0, sizeof (m3cf_device));
	
	/* Init the block queue */
	m3cf_device.queue = blk_alloc_queue(GFP_KERNEL);
	if (m3cf_device.queue == NULL) {
		printk(KERN_ERR "m3cf: unable to allocate block queue\n");
		unregister_blkdev(M3CF_MAJOR, "m3cf");
		return -ENOMEM;
	}
	blk_queue_make_request(m3cf_device.queue, m3cf_make_request);
	m3cf_device.queue->queuedata = &m3cf_device;

	/* Init the gendisk structure. */
	m3cf_device.gd = alloc_disk(M3CF_MINORS);
	if (!m3cf_device.gd) {
		printk (KERN_ERR "m3cf: alloc_disk failure\n");
		blk_put_queue(m3cf_device.queue);
		unregister_blkdev(M3CF_MAJOR, "m3cf");
		return -ENOMEM;
	}
	m3cf_device.gd->major = M3CF_MAJOR;
	m3cf_device.gd->first_minor = 0;
	m3cf_device.gd->fops  = &m3cf_ops;
	m3cf_device.gd->queue = m3cf_device.queue;
	snprintf (m3cf_device.gd->disk_name, 32, "hda");

	/* Calculate the size of the CF card */
	m3cf_device.num_sectors = m3cf_identify();
	set_capacity(m3cf_device.gd, m3cf_device.num_sectors);

	add_disk(m3cf_device.gd);
	return 0;
}

/* unregister the driver */
static void __exit m3cf_exit(void)
{
	if (m3cf_device.gd) {
		del_gendisk(m3cf_device.gd);
		put_disk(m3cf_device.gd);
	}
	if (m3cf_device.queue) {
		blk_put_queue(m3cf_device.queue);
	}
	unregister_blkdev(M3CF_MAJOR, "m3cf");
}
	
module_init(m3cf_init);
module_exit(m3cf_exit);
