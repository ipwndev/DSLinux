/*
 *  linux/drivers/block/supercard.c - Supercard SD driver (for Nintendo DS)
 *
 *  Copyright (C) 2005 Jean-Pierre Thomasset, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * 2005-11-15 : Initial version
 *
 * TODO :
 *  - read and set blocksize
 *  - find bug, correct bug, find bug, correct bug,...
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
//#include <linux/crc-citt.h>


/* some usefull stuff when debugging */

#define DRIVER_NAME "supercard-sd"
#define DEVICE_NAME "supercard"

#define SUPERCARD_RCA 0x7694
//#define SC_BLOCKSIZE 512

#define SC_SD_CMDWRITE  0x09800000
#define SC_SD_CMDREAD   0x09900000
#define SC_SD_DATAWRITE 0x09000000
#define SC_SD_DATAREAD  0x09100000
#define SC_SD_LOCK      0x09FFFFFE

#define SC_SD_MAXWAIT   10000000

#define SC_CRC16_POLY 0x1021

#define SC_SD_REG(type, reg) (*(volatile type *) (reg))

struct supercard_dev {
	/* CSD value : filled in response to cmd SEND_CSD */
	u32 csd[4];
		
	/* capacity in bytes computed from csd */
	/* capacity = (c_size+1) * \
	              ( 1 << (c_size_mult+2) ) * \
	              (1 << read_bl_len) ;
	*/
	u32 capacity; 
	u32 block_len;
	
	int major_num;
	spinlock_t lock;
	struct request_queue *queue;
	/* gendisk handle */
	struct gendisk *gd;	
};

static struct supercard_dev *current_device;

struct supercard_cmd {
	u32 command; /* command code */ 
	
/* supported commands */
#define SCCMD_SELECT_CARD          7
#define SCCMD_SEND_CSD             9
#define SCCMD_STOP_TRANSMISSION    12
#define SCCMD_READ_MULTIPLE_BLOCK  18
#define SCCMD_WRITE_MULTIPLE_BLOCK 25

/* unsupported commands */
#define SCCMD_SET_BLOCKLEN         16
#define SCCMD_READ_SINGLE_BLOCK    17
#define SCCMD_WRITE_SINGLE_BLOCK   24
	
	u32 argument; /* command argument */ 
	u32 response[4]; /* command response */
	u16 flags; /* flags */
#define SCFLG_RESP_NONE  (0)
#define SCFLG_RESP_SHORT (1)		/* 48 bits response (32bits stored)*/ 
#define SCFLG_RESP_LONG  (1 << 1)	/* 136 bits response (128bits stored)*/
#define SCFLG_RESP_MASK  (3)
#define SCFLG_DATA_READ  (1 << 2)
#define SCFLG_DATA_WRITE (1 << 3)
#define SCFLG_DATA_MASK  (3 << 2)
	
	u16 error;
#define SCERR_NONE    0
#define SCERR_TIMEOUT 1
#define SCERR_CRC     2
#define SCERR_IO      3
#define SCERR_NOTIMPL 4

	u16 *data; /* Data buffer */
	u32 data_len; /* requested data transfer length in bytes */
	u32 xfer_len; /* really transfered data length */

	struct supercard_dev *device;
};

void supercard_unlock( void )
{
	SC_SD_REG(u16, SC_SD_LOCK) = 0xa55a ;
	SC_SD_REG(u16, SC_SD_LOCK) = 0xa55a ;
	SC_SD_REG(u16, SC_SD_LOCK) = 0x3 ;
	SC_SD_REG(u16, SC_SD_LOCK) = 0x3 ;
}

void supercard_lock( void )
{
	SC_SD_REG(u16, SC_SD_LOCK)= 0xa55a ;
	SC_SD_REG(u16, SC_SD_LOCK) = 0xa55a ;
	SC_SD_REG(u16, SC_SD_LOCK) = 0x1 ;
	SC_SD_REG(u16, SC_SD_LOCK) = 0x1 ;
}

u8 supercard_crc7 (u8 *data, u8 length)
{
	/*
	u8 i, ibit, c, crc;
	crc = 0x00;
	for (i = 0; i < length; i++, data++)
	{
		c = data[i];
		for (ibit = 0; ibit < 8; ibit++)
		{
			crc = crc << 1;
			if ((c ^ crc) & 0x80) crc = crc ^ 0x09;
			c = c << 1;
		}
	}	
	*/
	int i;
	u32 r4 = 0x80808080;
	u8 crc = 0;
	u8 c;
	
	i = length * 8;
	do {
	    if (r4 & 0x80) c = *data++;
	    crc = crc << 1;
	
	    if (crc & 0x80) crc ^= 9;
	    if (c & (r4>>24)) crc ^= 9;
	    r4 = (r4 >> 1) | (r4 << 31);
  	} while (--i > 0);
	
	crc = (crc << 1) | 1;
	
	return crc;
}


void supercard_crc16_update(u32 *crc16, u8 c)
{
	int i;
		
	for(i=3; i>=0; i--) {
		crc16[i] = crc16[i] << 1;	
		if( crc16[i] & 0x10000 ) crc16[i] = crc16[i] ^ SC_CRC16_POLY;
		if( c & 0x80 ) crc16[i] = crc16[i] ^ SC_CRC16_POLY;
		crc16[i] = crc16[i] << 1;	
		if( crc16[i] & 0x10000 ) crc16[i] = crc16[i] ^ SC_CRC16_POLY;
		if( c & 0x08 ) crc16[i] = crc16[i] ^ SC_CRC16_POLY;
		c = c << 1;
	}
}

void supercard_crc16_finish(u32 *crc16, u8 *buffer)
{
	int i;
	int finalcrc;
	
	i = 16;
	finalcrc = 0;
	do {
		finalcrc = finalcrc << 4;
	    if (crc16[3] & 0x8000) finalcrc = finalcrc | 8;
	    if (crc16[2] & 0x8000) finalcrc = finalcrc | 4;
	    if (crc16[1] & 0x8000) finalcrc = finalcrc | 2;
	    if (crc16[0] & 0x8000) finalcrc = finalcrc | 1;
	
	    crc16[3] = crc16[3] << 1;
	    crc16[2] = crc16[2] << 1;
	    crc16[1] = crc16[1] << 1;
	    crc16[0] = crc16[0] << 1;	    
	
	    i--;
	    if (!(i & 1)) *buffer++ = finalcrc;
	} while(i);
}

void supercard_command_write8(u8 data)
{
	u32 temp;	
	/* Do not ask why...  */
	temp = (data << 17) + data;
	SC_SD_REG(u32, SC_SD_CMDWRITE) = temp;
	SC_SD_REG(u32, SC_SD_CMDWRITE+4) = temp<<2;
	SC_SD_REG(u32, SC_SD_CMDWRITE+8) = temp<<4;
	SC_SD_REG(u32, SC_SD_CMDWRITE+12) = temp<<6;
}

u8 supercard_command_read8( void )
{
	u32 temp, ret;	
	temp = SC_SD_REG(u32, SC_SD_CMDREAD);
	temp = SC_SD_REG(u32, SC_SD_CMDREAD);
	temp = SC_SD_REG(u32, SC_SD_CMDREAD);
	ret  = SC_SD_REG(u32, SC_SD_CMDREAD);
	
	return ret & 0xFF;
}

/* read a block from data line */
int supercard_data_read_block(u16 *buffer, struct supercard_dev *dev)
{
	int i;
	u32 temp;
	
	/* Wait for read ready */
	i=0;
	while ((SC_SD_REG(u16, SC_SD_DATAREAD) & 0x100) && (i++ < SC_SD_MAXWAIT));
	if(i>=SC_SD_MAXWAIT) {
		printk(KERN_WARNING DEVICE_NAME ": %s@%s:%d Time out\n", __func__, __FILE__, __LINE__);
		return SCERR_TIMEOUT;
	}
	
	i = 0;
	while(i<(dev->block_len / 2)) {
		temp = SC_SD_REG(u32, SC_SD_DATAREAD);
		temp = SC_SD_REG(u32, SC_SD_DATAREAD);
		buffer[i++] = temp >> 16;
	}
	
	i=0;
	/* strange, the supercard seems to send a 64bits crc16 ! */
	while(i++<4){
		temp = SC_SD_REG(u32, SC_SD_DATAREAD);
		temp = SC_SD_REG(u32, SC_SD_DATAREAD);
	}
	/* throw end bit */
	temp = SC_SD_REG(u16, SC_SD_DATAREAD);
	return SCERR_NONE;
}

/* write a block */
int supercard_data_write_block(u16 *buffer, struct supercard_dev *dev)
{
	int i;
	u32 temp;
	u32 crc[4] = {0,0,0,0};
	u16 crcbuffer[4];
	
	/* Wait for write ready */
	i=0;
	while (!(SC_SD_REG(u16, SC_SD_DATAWRITE) & 0x100) && (i++ < SC_SD_MAXWAIT));
	if(i>=SC_SD_MAXWAIT) {
		printk(KERN_WARNING DEVICE_NAME ": %s@%s:%d Time out\n", __func__, __FILE__, __LINE__);
		return SCERR_TIMEOUT;
	}
	temp = SC_SD_REG(u16, SC_SD_DATAWRITE);
	
	i = 0;
	SC_SD_REG(u16, SC_SD_DATAWRITE) = 0; // Start data
	while(i<(dev->block_len / 2)) {
		temp = buffer[i] + (buffer[i] << 20);
		SC_SD_REG(u32, SC_SD_DATAWRITE) = temp;
		SC_SD_REG(u32, SC_SD_DATAWRITE+4) = temp << 8;
		supercard_crc16_update(crc, *( (u8*) (buffer+2*i) ) );
		supercard_crc16_update(crc, *( (u8*) (buffer+2*i+1) ) );
		i++;
	}
	supercard_crc16_finish(crc, (u8*)crcbuffer);	
	
	/* Sending CRC ! */
	for(i=0; i<4; i++){
		temp = crcbuffer[i] + (crcbuffer[i] << 20);
		SC_SD_REG(u32, SC_SD_DATAWRITE) = temp;
		SC_SD_REG(u32, SC_SD_DATAWRITE+4) = temp << 8;
	}
	
	i=0;
		
	/* end bit */
	SC_SD_REG(u16, SC_SD_DATAWRITE) = 0xFF;

	// Wait for ready
	i=0;
	while ((SC_SD_REG(u16, SC_SD_DATAWRITE) & 0x100) && (i++ < SC_SD_MAXWAIT));
	if(i>=SC_SD_MAXWAIT) {
		printk(KERN_WARNING DEVICE_NAME ": %s@%s:%d Time out\n", __func__, __FILE__, __LINE__);
		return SCERR_TIMEOUT;
	}

	/* throw read */
	temp = SC_SD_REG(u32, SC_SD_DATAWRITE);
	temp = SC_SD_REG(u32, SC_SD_DATAWRITE);
	
	/* Clock ?*/
	i=0x10;
	while (i--) temp = SC_SD_REG(u16, SC_SD_CMDWRITE);
	
	return SCERR_NONE;
}


void supercard_command(struct supercard_cmd *cmd)
{
	u8 buffer[6];
	u32 temp;
	int i;
	u8 *resp;
	
	cmd->error = SCERR_NONE;
	
	buffer[0] = (u8) (cmd->command | 0x40); /* Start bit */
	buffer[1] = (u8) (cmd->argument >> 24);
	buffer[2] = (u8) (cmd->argument >> 16);
	buffer[3] = (u8) (cmd->argument >> 8);
	buffer[4] = (u8) (cmd->argument);
	buffer[5] = supercard_crc7(buffer, 5);
		
	// Wait for write ready bit set
	i=0;
	while (!(SC_SD_REG(u16, SC_SD_CMDWRITE) & 1) && (i++ < SC_SD_MAXWAIT)) ;
	if(i>=SC_SD_MAXWAIT) {
		cmd->error = SCERR_TIMEOUT;
		printk(KERN_WARNING DEVICE_NAME ": %s@%s:%d Time out\n", __func__, __FILE__, __LINE__);
		return;
	}
	
	temp = SC_SD_REG(u16, SC_SD_CMDWRITE);
	
	for(i=0; i<6; i++) {
		supercard_command_write8(buffer[i]);
	}
	
	/* check for response */
	switch (cmd->flags & SCFLG_RESP_MASK) {
	case SCFLG_RESP_NONE:
	default:
		break;
	case SCFLG_RESP_SHORT:
		/* 6 bytes response */
		resp = (u8*)(cmd->response);
		/* Wait for start bit unset */
		i=0;
		while ( (SC_SD_REG(u16, SC_SD_CMDWRITE) & 1)  && (i++ < SC_SD_MAXWAIT)) ;
		if(i>=SC_SD_MAXWAIT) {
			cmd->error = SCERR_TIMEOUT;
			printk(KERN_WARNING DEVICE_NAME ": %s@%s:%d Time out\n", __func__, __FILE__, __LINE__);
			return;
		}
		
		// throw away first byte
		temp = supercard_command_read8();
		temp = temp & 0x3f;
		if( temp != cmd->command ){
			printk(KERN_WARNING DEVICE_NAME ": Got Response to cmd %d (expecting %d)\n", temp, cmd->command);
			cmd->error = SCERR_IO;
			return;
		}
		for(i=3; i>=0; i--) {
			resp[i] = supercard_command_read8();
		}
		//printk(KERN_INFO DEVICE_NAME ": Status (cmd=%d) %x\n", cmd->command, cmd->response[0]);
		/* throw away crc */
		temp = supercard_command_read8();
		break;
	case SCFLG_RESP_LONG:
		/* 17 bytes response */
		resp = (u8*)(cmd->response);
		/* Wait for start bit unset */
		i=0;
		while ( (SC_SD_REG(u16, SC_SD_CMDWRITE) & 1)  && (i++ < SC_SD_MAXWAIT)) ;
		if(i>=SC_SD_MAXWAIT) {
			cmd->error = SCERR_TIMEOUT;
			printk(KERN_WARNING DEVICE_NAME ": %s@%s:%d Time out\n", __func__, __FILE__, __LINE__);
			return;
		}
		// throw away first byte
		temp = supercard_command_read8();
		for(i=0; i<4; i++) {
			resp[4*i+3] = supercard_command_read8();
			resp[4*i+2] = supercard_command_read8();
			resp[4*i+1] = supercard_command_read8();
			resp[4*i] = supercard_command_read8();
		}
		break;
	}
	/* Clock ?*/
	i=0x10;
	while (i--) temp = SC_SD_REG(u16, SC_SD_CMDWRITE);
	
	/* data transfer if any */
	switch (cmd->flags & SCFLG_DATA_MASK) {
	case SCFLG_DATA_READ:
		for(i=0; i<cmd->data_len; i+=cmd->device->block_len){
			temp = supercard_data_read_block(cmd->data + i, cmd->device);
			if(!temp) {
				cmd->error = temp;
				return;
			}
		}
		break;
	case SCFLG_DATA_WRITE:
		for(i=0; i<cmd->data_len; i+=cmd->device->block_len){
			temp = supercard_data_write_block(cmd->data + i, cmd->device);
			if(!temp) {
				cmd->error = temp;
				return;
			}
		}
		break;
	default:
		break;
	}
	
}

/*
 * Ioctl.
 */
int supercard_ioctl (struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	return -ENOTTY; /* unknown command */
}


/* Driver registration functions & data */
static struct block_device_operations supercard_ops = {
    .owner = THIS_MODULE,
    .ioctl = supercard_ioctl
};

/* */
int supercard_fillstruct(struct supercard_dev *dev)
{
	struct supercard_cmd *cmd;
	unsigned int read_bl_len, c_size, c_size_mult;
	
	cmd = vmalloc(sizeof(struct supercard_cmd));
	if(!cmd) 
		return -ENOMEM;
	// First deselect all card
	cmd->command = SCCMD_SELECT_CARD;
	cmd->argument = 0; /* deselect all cards */
	cmd->flags = SCFLG_RESP_NONE ;
	cmd->device = dev;
	supercard_command(cmd);
	if(cmd->error != SCERR_NONE){
		printk(KERN_WARNING DEVICE_NAME ": Error when deselecting card\n");
		vfree(cmd);
		return 0;	
	}
	
	// Get CSD
	cmd->command = SCCMD_SEND_CSD;
	cmd->argument = SUPERCARD_RCA << 16;
	cmd->flags = SCFLG_RESP_LONG ;
	supercard_command(cmd);
	if(cmd->error != SCERR_NONE) {
		printk(KERN_WARNING DEVICE_NAME ": Error when getting CSD\n");
		vfree(cmd);
		return 0;	
	}
	
	dev->csd[0] = cmd->response[0];
	dev->csd[1] = cmd->response[1];
	dev->csd[2] = cmd->response[2];
	dev->csd[3] = cmd->response[3];
	
	read_bl_len = (cmd->response[1] >> 16) & 0xF;
	c_size = ((cmd->response[2] >> 30) & 3) | ((cmd->response[1] & 0x3FF) << 2);
	c_size_mult = (cmd->response[2] >> 15) & 0x7;
	
	/* extract capacity from CSD */
	dev->capacity = (c_size+1) << (c_size_mult+2+read_bl_len);
	dev->block_len = 1 << read_bl_len;
	
	// reselect card
	cmd->command = SCCMD_SELECT_CARD;
	cmd->argument = SUPERCARD_RCA << 16; 
	cmd->flags = SCFLG_RESP_SHORT;
	supercard_command(cmd);
	if(cmd->error != SCERR_NONE){
		printk(KERN_WARNING DEVICE_NAME ": Error when selecting card\n");
		vfree(cmd);
		return 0;	
	}
	
	printk(KERN_INFO DEVICE_NAME ": Found device with %d MB\n", dev->capacity >> 20);
	
	vfree(cmd);
	return 1;
}

static void supercard_request(request_queue_t *q)
{
    struct request *req;
	struct supercard_cmd cmd;
	struct supercard_dev *dev;

	dev = (struct supercard_dev *) q->queuedata;
	
	while ((req = elv_next_request(q)) != NULL) {
		if (! blk_fs_request(req)) {
		    printk (KERN_NOTICE "Skip non-CMD request\n");
		    end_request(req, 0);
		    continue;
		}
	
		if(rq_data_dir(req)){
			/* writing */
			end_request(req, 0); // Do not support writing
		    continue;
			/*cmd.command = SCCMD_WRITE_MULTIPLE_BLOCK;
			cmd.flags = SCFLG_RESP_SHORT | SCFLG_DATA_WRITE;
			 printk (KERN_INFO DEVICE_NAME ": WRITING %d sector(s) @ %x\n", req->current_nr_sectors, req->sector);
			 */
		} else {
			/* reading */
			cmd.command = SCCMD_READ_MULTIPLE_BLOCK;
			cmd.flags = SCFLG_RESP_SHORT | SCFLG_DATA_READ;
			//printk (KERN_INFO DEVICE_NAME ": READING %d sector(s) @ %x\n", req->current_nr_sectors, req->sector);
		}
		cmd.argument = req->sector << 9;
		cmd.data = (u16*) (req->buffer);
		cmd.data_len = req->current_nr_sectors << 9;
		cmd.device = dev;
		supercard_command(&cmd);
		if(cmd.error != SCERR_NONE) {
			end_request(req, 0);	
		} else {
			/* operation done, stop now */
			cmd.command = SCCMD_STOP_TRANSMISSION;
			cmd.argument = 0;
			cmd.flags = SCFLG_RESP_SHORT;
			cmd.data = NULL;
			cmd.data_len = 0;
			supercard_command(&cmd);
			if(cmd.error != SCERR_NONE) {
				if(rq_data_dir(req)){
					int i;
					i=0;
					while (!(SC_SD_REG(u16, SC_SD_DATAWRITE) & 0x100) && (i++ < SC_SD_MAXWAIT));
					if(i>=SC_SD_MAXWAIT) {
						printk(KERN_WARNING DEVICE_NAME ": %s@%s:%d Time out\n", __func__, __FILE__, __LINE__);
						return SCERR_TIMEOUT;
					}
				}
				end_request(req, 0);
			} else {
				end_request(req, 1);
			}
		}
    }
}

/* Module initialization */
static int __init supercard_init(void)
{
	int ret = 0;
	struct supercard_dev *dev;
	struct request_queue *q;
	
	printk(KERN_WARNING DEVICE_NAME ": Initializing...\n");
	
	supercard_unlock();
	dev = vmalloc(sizeof(struct supercard_dev));
	if(!dev) 
		return -ENOMEM;
	
	/* Get a request queue. */
    q = blk_init_queue(supercard_request, &dev->lock);
    if (! q )
	    goto out;
	q->queuedata = dev;
    blk_queue_hardsect_size(q, 512);
	dev->queue = q;
	
	/* Get card size */
	if( !supercard_fillstruct(dev)) {
		printk(KERN_WARNING DEVICE_NAME ": unable to get CSD from card\n");
		ret = -EIO;
		goto out;
	}	
		
	dev->major_num = 0;
	dev->major_num = register_blkdev(dev->major_num, DEVICE_NAME);
	if (dev->major_num<= 0) {
		printk(KERN_WARNING DEVICE_NAME ": unable to get major number\n");
		ret = dev->major_num;
		goto out;
    }
    
    
    dev->gd = alloc_disk(4); /* only 4 minor number, should be sufficient */
    if (! dev->gd)
		goto out_unregister;

	dev->gd->major = dev->major_num;
    dev->gd->first_minor = 0;
    dev->gd->fops = &supercard_ops;
    dev->gd->private_data = dev;
    strcpy (dev->gd->disk_name, DEVICE_NAME "0");
    set_capacity(dev->gd, dev->capacity/512);
    dev->gd->queue = q;
    add_disk(dev->gd);
	current_device = dev;
	return 0;

out_unregister:
	unregister_blkdev(dev->major_num, DEVICE_NAME);  
out:
	vfree(dev);
	supercard_lock();
	return ret;
}

static void __exit supercard_exit(void)
{
	del_gendisk(current_device->gd);
    put_disk(current_device->gd);
    unregister_blkdev(current_device->major_num, DEVICE_NAME);
    blk_cleanup_queue(current_device->queue);
    vfree(current_device);
	
	supercard_lock();
	return ;
}

module_init(supercard_init);
module_exit(supercard_exit);

MODULE_DESCRIPTION("Supercard SD driver for Nintendo DS");
MODULE_LICENSE("GPL");
