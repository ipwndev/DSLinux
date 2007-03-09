/*
 * This is a block driver for the famous DLDI dynamic link drivers by chism.
 * (c) 2007 Amadeus.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/hdreg.h>	/* HDIO_GETGEO */
#include <linux/kdev_t.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/bio.h>

MODULE_LICENSE("GPL");

/* Interface to dldi */
typedef int (* FN_MEDIUM_STARTUP)(void) ;
typedef int (* FN_MEDIUM_ISINSERTED)(void) ;
typedef int (* FN_MEDIUM_READSECTORS)(u32 sector, u32 numSectors, void* buffer) ;
typedef int (* FN_MEDIUM_WRITESECTORS)(u32 sector, u32 numSectors, const void* buffer) ;
typedef int (* FN_MEDIUM_CLEARSTATUS)(void) ;
typedef int (* FN_MEDIUM_SHUTDOWN)(void) ;

struct IO_INTERFACE_STRUCT {
	unsigned long		ioType ;
	unsigned long		features ;
	FN_MEDIUM_STARTUP	fn_startup ;
	FN_MEDIUM_ISINSERTED	fn_isInserted ;
	FN_MEDIUM_READSECTORS	fn_readSectors ;
	FN_MEDIUM_WRITESECTORS	fn_writeSectors ;
	FN_MEDIUM_CLEARSTATUS	fn_clearStatus ;
	FN_MEDIUM_SHUTDOWN	fn_shutdown ;
} ;

extern struct IO_INTERFACE_STRUCT _io_dldi;

/* This is the major number of /dev/hda */
static int dldi_major = 3;

/* We don't know how big the medium is. So we take some big number, */
static int nsectors   = (2*1024*1024*2);	/* 2 GBytes */

/*
 * Minor number.
 */
#define dldi_MINORS	5

/*
 * The internal representation of our device.
 */
struct dldi_dev {
        int size;                       /* Device size in bytes */
        struct request_queue *queue;    /* The device request queue */
        struct gendisk *gd;             /* The gendisk structure */
};

static struct dldi_dev *Devices = NULL;

/*
 * Handle an I/O request.
 */
static void dldi_transfer(struct dldi_dev *dev, unsigned long sector,
		unsigned long nsect, char *buffer, int write)
{
	int ret = 0;

	if (write) {
		// printk(KERN_ERR "Write Sector %ld, %ld Blocks\n", sector, nsect);
		ret = _io_dldi.fn_writeSectors(sector, nsect, buffer);
	} else {
		// printk(KERN_ERR "Read Sector %ld, %ld Blocks\n", sector, nsect);
		ret = _io_dldi.fn_readSectors(sector, nsect, buffer);
	}
	if (ret == 0) {
 		printk(KERN_ERR "dldi_transfer failure\n");
	} else {
		// printk(KERN_ERR "all OK\n");
	}
}

/*
 * Transfer a single BIO.
 */
static int dldi_xfer_bio(struct dldi_dev *dev, struct bio *bio)
{
	int i;
	struct bio_vec *bvec;
	sector_t sector = bio->bi_sector;

	/* Do each segment independently. */
	bio_for_each_segment(bvec, bio, i) {
		char *buffer = __bio_kmap_atomic(bio, i, KM_USER0);
		dldi_transfer(dev, sector, bio_cur_sectors(bio),
				buffer, bio_data_dir(bio) == WRITE);
		sector += bio_cur_sectors(bio);
		__bio_kunmap_atomic(bio, KM_USER0);
	}
	return 0; /* Always "succeed" */
}


/*
 * The direct make request version.
 */
static int dldi_make_request(request_queue_t *q, struct bio *bio)
{
	struct dldi_dev *dev = q->queuedata;
	int status;

	status = dldi_xfer_bio(dev, bio);
	bio_endio(bio, bio->bi_size, status);
	return 0;
}


/*
 * Open and close.
 */

static int dldi_open(struct inode *inode, struct file *filp)
{
	struct dldi_dev *dev = inode->i_bdev->bd_disk->private_data;

	filp->private_data = dev;
	return 0;
}

static int dldi_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * The ioctl() implementation
 */

int dldi_ioctl (struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	long size;
	struct hd_geometry geo;
	struct dldi_dev *dev = filp->private_data;

	switch(cmd) {
	    case HDIO_GETGEO:
        	/*
		 * Get geometry: since we are a virtual device, we have to make
		 * up something plausible.  So we claim 16 sectors, four heads,
		 * and calculate the corresponding number of cylinders.  We set the
		 * start of data at sector four.
		 */
		size = dev->size;
		geo.cylinders = (size & ~0x3f) >> 6;
		geo.heads = 4;
		geo.sectors = 16;
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
static struct block_device_operations dldi_ops = {
	.owner           = THIS_MODULE,
	.open 	         = dldi_open,
	.release 	 = dldi_release,
	.ioctl	         = dldi_ioctl
};


/*
 * Set up our internal device.
 */
static void setup_device(struct dldi_dev *dev, int which)
{
	/*
	 * Get some memory.
	 */
	memset (dev, 0, sizeof (struct dldi_dev));
	dev->size = nsectors*512;
	
	/*
	 * The I/O queue, depending on whether we are using our own
	 * make_request function or not.
	 */
	dev->queue = blk_alloc_queue(GFP_KERNEL);
	if (dev->queue == NULL)
		return;
	blk_queue_make_request(dev->queue, dldi_make_request);

	blk_queue_hardsect_size(dev->queue, 512);
	dev->queue->queuedata = dev;
	/*
	 * And the gendisk structure.
	 */
	dev->gd = alloc_disk(dldi_MINORS);
	if (! dev->gd) {
		printk (KERN_NOTICE "alloc_disk failure\n");
		return;
	}
	dev->gd->major = dldi_major;
	dev->gd->first_minor = which*dldi_MINORS;
	dev->gd->fops = &dldi_ops;
	dev->gd->queue = dev->queue;
	dev->gd->private_data = dev;
	snprintf (dev->gd->disk_name, 32, "hd%c", which + 'a');
	set_capacity(dev->gd, nsectors);
	add_disk(dev->gd);
	return;
}



static int __init dldi_init(void)
{
	int ret;

	/*
	 * Get registered.
	 */
	if(register_blkdev(dldi_major, "dldi")) {
		printk(KERN_WARNING "dldi: unable to get major number\n");
		return -EBUSY;
	}
	/*
	 * Allocate the device array, and initialize each one.
	 */
	Devices = kmalloc(sizeof (struct dldi_dev), GFP_KERNEL);
	if (Devices == NULL) {
		unregister_blkdev(dldi_major, "dldi");
		return -ENOMEM;
	}
	setup_device(Devices, 0);
    
	/* Start the interface */
	ret = _io_dldi.fn_startup();
	if (ret == 0) {
		printk(KERN_ERR "dldi: fn_startup failure\n");
		if (Devices->gd) {
			del_gendisk(Devices->gd);
			put_disk(Devices->gd);
		}
		if (Devices->queue) {
			blk_put_queue(Devices->queue);
		}
		unregister_blkdev(dldi_major, "dldi");
		return -EIO;
	}

	return 0;
}

static void dldi_exit(void)
{
	int ret;
	struct dldi_dev *dev = Devices;

	/* Shutdown the interface */
	ret = _io_dldi.fn_shutdown();
	if (ret == 0) {
		printk(KERN_ERR "dldi: fn_shutdown failure\n");
	}

	if (dev->gd) {
		del_gendisk(dev->gd);
		put_disk(dev->gd);
	}
	if (dev->queue) {
		blk_put_queue(dev->queue);
	}

	unregister_blkdev(dldi_major, "dldi");
	kfree(Devices);
}
	
module_init(dldi_init);
module_exit(dldi_exit);
