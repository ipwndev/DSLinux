/*
 * Copyright (C) 2004 IBM Corporation
 *
 * Authors:
 * Leendert van Doorn <leendert@watson.ibm.com>
 * Dave Safford <safford@watson.ibm.com>
 * Reiner Sailer <sailer@watson.ibm.com>
 * Kylene Hall <kjhall@us.ibm.com>
 *
 * Maintained by: <tpmdd_devel@lists.sourceforge.net>
 *
 * Device driver for TCG/TCPA TPM (trusted platform module).
 * Specifications at www.trustedcomputinggroup.org	 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 * 
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

enum tpm_timeout {
	TPM_TIMEOUT = 5,	/* msecs */
};

/* TPM addresses */
enum tpm_addr {
	TPM_SUPERIO_ADDR = 0x2E,
	TPM_ADDR = 0x4E,
};

extern ssize_t tpm_show_pubek(struct device *, struct device_attribute *attr,
				char *);
extern ssize_t tpm_show_pcrs(struct device *, struct device_attribute *attr,
				char *);
extern ssize_t tpm_show_caps(struct device *, struct device_attribute *attr,
				char *);
extern ssize_t tpm_store_cancel(struct device *, struct device_attribute *attr,
				const char *, size_t);

struct tpm_chip;

struct tpm_vendor_specific {
	u8 req_complete_mask;
	u8 req_complete_val;
	u8 req_canceled;
	u16 base;		/* TPM base address */

	int (*recv) (struct tpm_chip *, u8 *, size_t);
	int (*send) (struct tpm_chip *, u8 *, size_t);
	void (*cancel) (struct tpm_chip *);
	struct miscdevice miscdev;
	struct attribute_group *attr_group;
};

struct tpm_chip {
	struct pci_dev *pci_dev;	/* PCI device stuff */

	int dev_num;		/* /dev/tpm# */
	int num_opens;		/* only one allowed */
	int time_expired;

	/* Data passed to and from the tpm via the read/write calls */
	u8 *data_buffer;
	atomic_t data_pending;
	struct semaphore buffer_mutex;

	struct timer_list user_read_timer;	/* user needs to claim result */
	struct semaphore tpm_mutex;	/* tpm is processing */

	struct tpm_vendor_specific *vendor;

	struct list_head list;
};

static inline int tpm_read_index(int base, int index)
{
	outb(index, base);
	return inb(base+1) & 0xFF;
}

static inline void tpm_write_index(int base, int index, int value)
{
	outb(index, base);
	outb(value & 0xFF, base+1);
}

extern int tpm_register_hardware(struct pci_dev *,
				 struct tpm_vendor_specific *);
extern int tpm_open(struct inode *, struct file *);
extern int tpm_release(struct inode *, struct file *);
extern ssize_t tpm_write(struct file *, const char __user *, size_t,
			 loff_t *);
extern ssize_t tpm_read(struct file *, char __user *, size_t, loff_t *);
extern void __devexit tpm_remove(struct pci_dev *);
extern int tpm_pm_suspend(struct pci_dev *, pm_message_t);
extern int tpm_pm_resume(struct pci_dev *);
