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
#include <linux/workqueue.h>

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

struct dldi_params {
	void *oldstack;
	void *oldlink;
	long para1;
	long para2;
	long para3;
	void *function;
	int result;
};

extern struct dldi_params _param_dldi;

extern int _call_dldi(void);

extern char _buf_dldi[];

/*
 * Minor number.
 */
#define dldi_MINORS	5

#define SECTOR_SIZE	512

#define REQ_SIZE	64

#define BUF_SIZE	2048

/* This is the major number of /dev/dldi */
static int dldi_major = 240;

/* We don't know how big the medium is. So we take some big number, */
static unsigned int nsectors   = (8*1024*1024*2);	/* 8 GBytes */

/*
 * The internal representation of our device.
 */
struct dldi_dev {
        int size;                       /* Device size in bytes */
        spinlock_t lock;                /* For mutual exclusion */
	struct work_struct dldi_work;	/* For deferred start */
	int running;			
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
	unsigned long sectors;
	while (nsect > 0) {
		sectors = nsect;
		if (sectors > (BUF_SIZE/SECTOR_SIZE))
			sectors = (BUF_SIZE/SECTOR_SIZE);
		
		_param_dldi.para1 = sector;
		_param_dldi.para2 = sectors;
		_param_dldi.para3 = (long)_buf_dldi;

		if (write) {
			// printk(KERN_ERR "Write Sector %ld, %ld Blocks\n", sector, nsect);
			memcpy(_buf_dldi, buffer, sectors * SECTOR_SIZE);
			_param_dldi.function = _io_dldi.fn_writeSectors;
		} else {
			// printk(KERN_ERR "Read Sector %ld, %ld Blocks\n", sector, nsect);
			_param_dldi.function = _io_dldi.fn_readSectors;
		}
		ret = _call_dldi();
		if (ret == 0) {
 			printk(KERN_ERR "dldi_transfer failure\n");
		} else {
			// printk(KERN_ERR "all OK\n");
		}
		if (!write)
			memcpy(buffer, _buf_dldi, sectors * SECTOR_SIZE);

		sector += sectors;
		nsect  -= sectors;
		buffer += (sectors * SECTOR_SIZE);
	}
}

/*
 * Delayed request function. This function is a complex one because
 * I want to merge requests here: I have found that for FAT file
 * systems, I will get 512 bytes per request, and this is faaar to slow.
 */
static void dldi_do_request(void *arg)
{
	struct dldi_dev *dev = (struct dldi_dev *)arg;
	request_queue_t *q = dev->queue;
	struct request *req;
	struct request *next;
	struct request *requests[REQ_SIZE];
	int index;

	long dir;
	sector_t sector;
	int nr_sec;
	void *buffer;

	spin_lock(&dev->lock);
	dev->running = 0;

	// printk (KERN_NOTICE "do_request\n");

	next = NULL;

	/* search the first fs request */
again:	while (1) {
		if (next) {
			req = next;
			next = NULL;
		} else {
			req = elv_next_request(q);
			if (!req)
				/* no request found */
				break;
			blkdev_dequeue_request(req);
		}
		if (blk_fs_request(req))
			/* fs request found */
			break;
		/* skip non-fs requests */
		end_request(req, 0);
	}
	if (req) {
		/* setup r/w parameters for first request */
		dir 	= rq_data_dir(req);
		sector  = req->sector;
		nr_sec  = req->current_nr_sectors;
		buffer  = req->buffer;
		/* store first request for later signaling */
		memset(requests, 0, sizeof(requests));
		index = 0;
		requests[0] = req;
		/* walk through the requests and try to merge */
		index++;
		while (index < REQ_SIZE) {
			req = elv_next_request(q);
			if (!req)
				/* end of requests */
				break;
			blkdev_dequeue_request(req);
			requests[index++] = req;
			if (!blk_fs_request(req))
				/* skip non-fs requests */
				continue;
			/* test for possible merge */
			if ((dir == rq_data_dir(req))
			  &&((sector + nr_sec) == req->sector)
			  &&((buffer + nr_sec*SECTOR_SIZE) == req->buffer)) {
				/* do the merge */
				nr_sec += req->current_nr_sectors;
			} else {
				/* no merge possible */
				next = req;
				requests[--index] = NULL;
				break;
			}	
		}	
		/* do the transfer */
		dldi_transfer(dev, sector, nr_sec, buffer, dir);
 		/* send completion to all requests */
		for (index = 0; index < REQ_SIZE; index++) {
			req = requests[index];
			if (!req)
				break;
			end_request(req, blk_fs_request(req));
		}
		/* try the next request */
		goto again;
	}
	spin_unlock(&dev->lock);
}


/*
 * Request function. Defer action to have the chance to group requests together.
 */
static void dldi_request(request_queue_t *q)
{
	struct dldi_dev *dev = Devices;
	
	if (!dev->running) {
		dev->running = 1;
		schedule_work(&dev->dldi_work);
	}
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
	dev->size = nsectors*SECTOR_SIZE;
	spin_lock_init(&dev->lock);
	INIT_WORK(&dev->dldi_work, dldi_do_request, dev);
	dev->running = 0;

	/*
	 * The I/O queue, depending on whether we are using our own
	 * make_request function or not.
	 */
	dev->queue = blk_init_queue(dldi_request, &dev->lock);
	if (dev->queue == NULL)
		return;

	blk_queue_hardsect_size(dev->queue, SECTOR_SIZE);
	blk_queue_max_phys_segments(dev->queue, 1);
	blk_queue_max_sectors(dev->queue, 128);
	blk_queue_max_hw_segments(dev->queue, 1);
	blk_queue_max_segment_size(dev->queue, SECTOR_SIZE * 128);
	blk_queue_dma_alignment(dev->queue, 3);
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
	snprintf (dev->gd->disk_name, 32, "dldi");
	set_capacity(dev->gd, nsectors);
	add_disk(dev->gd);
	return;
}



static int __init dldi_init(void)
{
	int ret;

	/* Start the interface */
	_param_dldi.function = _io_dldi.fn_startup;
	ret = _call_dldi();
	if (ret == 0) {
		printk(KERN_ERR "dldi: fn_startup failure\n");
		return -EIO;
	}		

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

	return 0;
}

static void dldi_exit(void)
{
	int ret;
	struct dldi_dev *dev = Devices;

	cancel_delayed_work(&dev->dldi_work);

	/* Shutdown the interface */
	_param_dldi.function = _io_dldi.fn_shutdown;
	ret = _call_dldi();
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
