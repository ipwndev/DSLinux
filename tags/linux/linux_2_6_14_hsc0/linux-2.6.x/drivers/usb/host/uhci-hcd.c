/*
 * Universal Host Controller Interface driver for USB.
 *
 * Maintainer: Alan Stern <stern@rowland.harvard.edu>
 *
 * (C) Copyright 1999 Linus Torvalds
 * (C) Copyright 1999-2002 Johannes Erdfelt, johannes@erdfelt.com
 * (C) Copyright 1999 Randy Dunlap
 * (C) Copyright 1999 Georg Acher, acher@in.tum.de
 * (C) Copyright 1999 Deti Fliegl, deti@fliegl.de
 * (C) Copyright 1999 Thomas Sailer, sailer@ife.ee.ethz.ch
 * (C) Copyright 1999 Roman Weissgaerber, weissg@vienna.at
 * (C) Copyright 2000 Yggdrasil Computing, Inc. (port of new PCI interface
 *               support from usb-ohci.c by Adam Richter, adam@yggdrasil.com).
 * (C) Copyright 1999 Gregory P. Smith (from usb-ohci.c)
 * (C) Copyright 2004-2005 Alan Stern, stern@rowland.harvard.edu
 *
 * Intel documents this fairly well, and as far as I know there
 * are no royalties or anything like that, but even so there are
 * people who decided that they want to do the same thing in a
 * completely different way.
 *
 */

#include <linux/config.h>
#ifdef CONFIG_USB_DEBUG
#define DEBUG
#else
#undef DEBUG
#endif
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/debugfs.h>
#include <linux/pm.h>
#include <linux/dmapool.h>
#include <linux/dma-mapping.h>
#include <linux/usb.h>
#include <linux/bitops.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>

#include "../core/hcd.h"
#include "uhci-hcd.h"

/*
 * Version Information
 */
#define DRIVER_VERSION "v2.3"
#define DRIVER_AUTHOR "Linus 'Frodo Rabbit' Torvalds, Johannes Erdfelt, \
Randy Dunlap, Georg Acher, Deti Fliegl, Thomas Sailer, Roman Weissgaerber, \
Alan Stern"
#define DRIVER_DESC "USB Universal Host Controller Interface driver"

/*
 * debug = 0, no debugging messages
 * debug = 1, dump failed URB's except for stalls
 * debug = 2, dump all failed URB's (including stalls)
 *            show all queues in /debug/uhci/[pci_addr]
 * debug = 3, show all TD's in URB's when dumping
 */
#ifdef DEBUG
static int debug = 1;
#else
static int debug = 0;
#endif
module_param(debug, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug level");
static char *errbuf;
#define ERRBUF_LEN    (32 * 1024)

static kmem_cache_t *uhci_up_cachep;	/* urb_priv */

static void suspend_rh(struct uhci_hcd *uhci, enum uhci_rh_state new_state);
static void wakeup_rh(struct uhci_hcd *uhci);
static void uhci_get_current_frame_number(struct uhci_hcd *uhci);

/* If a transfer is still active after this much time, turn off FSBR */
#define IDLE_TIMEOUT	msecs_to_jiffies(50)
#define FSBR_DELAY	msecs_to_jiffies(50)

/* When we timeout an idle transfer for FSBR, we'll switch it over to */
/* depth first traversal. We'll do it in groups of this number of TD's */
/* to make sure it doesn't hog all of the bandwidth */
#define DEPTH_INTERVAL 5

#include "uhci-debug.c"
#include "uhci-q.c"
#include "uhci-hub.c"

/*
 * Make sure the controller is completely inactive, unable to
 * generate interrupts or do DMA.
 */
static void reset_hc(struct uhci_hcd *uhci)
{
	int port;

	/* Turn off PIRQ enable and SMI enable.  (This also turns off the
	 * BIOS's USB Legacy Support.)  Turn off all the R/WC bits too.
	 */
	pci_write_config_word(to_pci_dev(uhci_dev(uhci)), USBLEGSUP,
			USBLEGSUP_RWC);

	/* Reset the HC - this will force us to get a
	 * new notification of any already connected
	 * ports due to the virtual disconnect that it
	 * implies.
	 */
	outw(USBCMD_HCRESET, uhci->io_addr + USBCMD);
	mb();
	udelay(5);
	if (inw(uhci->io_addr + USBCMD) & USBCMD_HCRESET)
		dev_warn(uhci_dev(uhci), "HCRESET not completed yet!\n");

	/* Just to be safe, disable interrupt requests and
	 * make sure the controller is stopped.
	 */
	outw(0, uhci->io_addr + USBINTR);
	outw(0, uhci->io_addr + USBCMD);

	/* HCRESET doesn't affect the Suspend, Reset, and Resume Detect
	 * bits in the port status and control registers.
	 * We have to clear them by hand.
	 */
	for (port = 0; port < uhci->rh_numports; ++port)
		outw(0, uhci->io_addr + USBPORTSC1 + (port * 2));

	uhci->port_c_suspend = uhci->suspended_ports =
			uhci->resuming_ports = 0;
	uhci->rh_state = UHCI_RH_RESET;
	uhci->is_stopped = UHCI_IS_STOPPED;
	uhci_to_hcd(uhci)->state = HC_STATE_HALT;
	uhci_to_hcd(uhci)->poll_rh = 0;
}

/*
 * Last rites for a defunct/nonfunctional controller
 * or one we don't want to use any more.
 */
static void hc_died(struct uhci_hcd *uhci)
{
	reset_hc(uhci);
	uhci->hc_inaccessible = 1;
}

/*
 * Initialize a controller that was newly discovered or has just been
 * resumed.  In either case we can't be sure of its previous state.
 */
static void check_and_reset_hc(struct uhci_hcd *uhci)
{
	u16 legsup;
	unsigned int cmd, intr;

	/*
	 * When restarting a suspended controller, we expect all the
	 * settings to be the same as we left them:
	 *
	 *	PIRQ and SMI disabled, no R/W bits set in USBLEGSUP;
	 *	Controller is stopped and configured with EGSM set;
	 *	No interrupts enabled except possibly Resume Detect.
	 *
	 * If any of these conditions are violated we do a complete reset.
	 */
	pci_read_config_word(to_pci_dev(uhci_dev(uhci)), USBLEGSUP, &legsup);
	if (legsup & ~(USBLEGSUP_RO | USBLEGSUP_RWC)) {
		dev_dbg(uhci_dev(uhci), "%s: legsup = 0x%04x\n",
				__FUNCTION__, legsup);
		goto reset_needed;
	}

	cmd = inw(uhci->io_addr + USBCMD);
	if ((cmd & USBCMD_RS) || !(cmd & USBCMD_CF) || !(cmd & USBCMD_EGSM)) {
		dev_dbg(uhci_dev(uhci), "%s: cmd = 0x%04x\n",
				__FUNCTION__, cmd);
		goto reset_needed;
	}

	intr = inw(uhci->io_addr + USBINTR);
	if (intr & (~USBINTR_RESUME)) {
		dev_dbg(uhci_dev(uhci), "%s: intr = 0x%04x\n",
				__FUNCTION__, intr);
		goto reset_needed;
	}
	return;

reset_needed:
	dev_dbg(uhci_dev(uhci), "Performing full reset\n");
	reset_hc(uhci);
}

/*
 * Store the basic register settings needed by the controller.
 */
static void configure_hc(struct uhci_hcd *uhci)
{
	/* Set the frame length to the default: 1 ms exactly */
	outb(USBSOF_DEFAULT, uhci->io_addr + USBSOF);

	/* Store the frame list base address */
	outl(uhci->fl->dma_handle, uhci->io_addr + USBFLBASEADD);

	/* Set the current frame number */
	outw(uhci->frame_number, uhci->io_addr + USBFRNUM);

	/* Mark controller as running before we enable interrupts */
	uhci_to_hcd(uhci)->state = HC_STATE_RUNNING;
	mb();

	/* Enable PIRQ */
	pci_write_config_word(to_pci_dev(uhci_dev(uhci)), USBLEGSUP,
			USBLEGSUP_DEFAULT);
}


static int resume_detect_interrupts_are_broken(struct uhci_hcd *uhci)
{
	int port;

	switch (to_pci_dev(uhci_dev(uhci))->vendor) {
	    default:
		break;

	    case PCI_VENDOR_ID_GENESYS:
		/* Genesys Logic's GL880S controllers don't generate
		 * resume-detect interrupts.
		 */
		return 1;

	    case PCI_VENDOR_ID_INTEL:
		/* Some of Intel's USB controllers have a bug that causes
		 * resume-detect interrupts if any port has an over-current
		 * condition.  To make matters worse, some motherboards
		 * hardwire unused USB ports' over-current inputs active!
		 * To prevent problems, we will not enable resume-detect
		 * interrupts if any ports are OC.
		 */
		for (port = 0; port < uhci->rh_numports; ++port) {
			if (inw(uhci->io_addr + USBPORTSC1 + port * 2) &
					USBPORTSC_OC)
				return 1;
		}
		break;
	}
	return 0;
}

static void suspend_rh(struct uhci_hcd *uhci, enum uhci_rh_state new_state)
__releases(uhci->lock)
__acquires(uhci->lock)
{
	int auto_stop;
	int int_enable;

	auto_stop = (new_state == UHCI_RH_AUTO_STOPPED);
	dev_dbg(uhci_dev(uhci), "%s%s\n", __FUNCTION__,
			(auto_stop ? " (auto-stop)" : ""));

	/* If we get a suspend request when we're already auto-stopped
	 * then there's nothing to do.
	 */
	if (uhci->rh_state == UHCI_RH_AUTO_STOPPED) {
		uhci->rh_state = new_state;
		return;
	}

	/* Enable resume-detect interrupts if they work.
	 * Then enter Global Suspend mode, still configured.
	 */
	uhci->working_RD = 1;
	int_enable = USBINTR_RESUME;
	if (resume_detect_interrupts_are_broken(uhci)) {
		uhci->working_RD = int_enable = 0;
	}
	outw(int_enable, uhci->io_addr + USBINTR);
	outw(USBCMD_EGSM | USBCMD_CF, uhci->io_addr + USBCMD);
	mb();
	udelay(5);

	/* If we're auto-stopping then no devices have been attached
	 * for a while, so there shouldn't be any active URBs and the
	 * controller should stop after a few microseconds.  Otherwise
	 * we will give the controller one frame to stop.
	 */
	if (!auto_stop && !(inw(uhci->io_addr + USBSTS) & USBSTS_HCH)) {
		uhci->rh_state = UHCI_RH_SUSPENDING;
		spin_unlock_irq(&uhci->lock);
		msleep(1);
		spin_lock_irq(&uhci->lock);
		if (uhci->hc_inaccessible)	/* Died */
			return;
	}
	if (!(inw(uhci->io_addr + USBSTS) & USBSTS_HCH))
		dev_warn(uhci_dev(uhci), "Controller not stopped yet!\n");

	uhci_get_current_frame_number(uhci);
	smp_wmb();

	uhci->rh_state = new_state;
	uhci->is_stopped = UHCI_IS_STOPPED;
	uhci_to_hcd(uhci)->poll_rh = !int_enable;

	uhci_scan_schedule(uhci, NULL);
}

static void start_rh(struct uhci_hcd *uhci)
{
	uhci->is_stopped = 0;
	smp_wmb();

	/* Mark it configured and running with a 64-byte max packet.
	 * All interrupts are enabled, even though RESUME won't do anything.
	 */
	outw(USBCMD_RS | USBCMD_CF | USBCMD_MAXP, uhci->io_addr + USBCMD);
	outw(USBINTR_TIMEOUT | USBINTR_RESUME | USBINTR_IOC | USBINTR_SP,
			uhci->io_addr + USBINTR);
	mb();
	uhci->rh_state = UHCI_RH_RUNNING;
	uhci_to_hcd(uhci)->poll_rh = 1;
}

static void wakeup_rh(struct uhci_hcd *uhci)
__releases(uhci->lock)
__acquires(uhci->lock)
{
	dev_dbg(uhci_dev(uhci), "%s%s\n", __FUNCTION__,
			uhci->rh_state == UHCI_RH_AUTO_STOPPED ?
				" (auto-start)" : "");

	/* If we are auto-stopped then no devices are attached so there's
	 * no need for wakeup signals.  Otherwise we send Global Resume
	 * for 20 ms.
	 */
	if (uhci->rh_state == UHCI_RH_SUSPENDED) {
		uhci->rh_state = UHCI_RH_RESUMING;
		outw(USBCMD_FGR | USBCMD_EGSM | USBCMD_CF,
				uhci->io_addr + USBCMD);
		spin_unlock_irq(&uhci->lock);
		msleep(20);
		spin_lock_irq(&uhci->lock);
		if (uhci->hc_inaccessible)	/* Died */
			return;

		/* End Global Resume and wait for EOP to be sent */
		outw(USBCMD_CF, uhci->io_addr + USBCMD);
		mb();
		udelay(4);
		if (inw(uhci->io_addr + USBCMD) & USBCMD_FGR)
			dev_warn(uhci_dev(uhci), "FGR not stopped yet!\n");
	}

	start_rh(uhci);

	/* Restart root hub polling */
	mod_timer(&uhci_to_hcd(uhci)->rh_timer, jiffies);
}

static irqreturn_t uhci_irq(struct usb_hcd *hcd, struct pt_regs *regs)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);
	unsigned short status;
	unsigned long flags;

	/*
	 * Read the interrupt status, and write it back to clear the
	 * interrupt cause.  Contrary to the UHCI specification, the
	 * "HC Halted" status bit is persistent: it is RO, not R/WC.
	 */
	status = inw(uhci->io_addr + USBSTS);
	if (!(status & ~USBSTS_HCH))	/* shared interrupt, not mine */
		return IRQ_NONE;
	outw(status, uhci->io_addr + USBSTS);		/* Clear it */

	if (status & ~(USBSTS_USBINT | USBSTS_ERROR | USBSTS_RD)) {
		if (status & USBSTS_HSE)
			dev_err(uhci_dev(uhci), "host system error, "
					"PCI problems?\n");
		if (status & USBSTS_HCPE)
			dev_err(uhci_dev(uhci), "host controller process "
					"error, something bad happened!\n");
		if (status & USBSTS_HCH) {
			spin_lock_irqsave(&uhci->lock, flags);
			if (uhci->rh_state >= UHCI_RH_RUNNING) {
				dev_err(uhci_dev(uhci),
					"host controller halted, "
					"very bad!\n");
				hc_died(uhci);

				/* Force a callback in case there are
				 * pending unlinks */
				mod_timer(&hcd->rh_timer, jiffies);
			}
			spin_unlock_irqrestore(&uhci->lock, flags);
		}
	}

	if (status & USBSTS_RD)
		usb_hcd_poll_rh_status(hcd);
	else {
		spin_lock_irqsave(&uhci->lock, flags);
		uhci_scan_schedule(uhci, regs);
		spin_unlock_irqrestore(&uhci->lock, flags);
	}

	return IRQ_HANDLED;
}

/*
 * Store the current frame number in uhci->frame_number if the controller
 * is runnning
 */
static void uhci_get_current_frame_number(struct uhci_hcd *uhci)
{
	if (!uhci->is_stopped)
		uhci->frame_number = inw(uhci->io_addr + USBFRNUM);
}

/*
 * De-allocate all resources
 */
static void release_uhci(struct uhci_hcd *uhci)
{
	int i;

	for (i = 0; i < UHCI_NUM_SKELQH; i++)
		if (uhci->skelqh[i]) {
			uhci_free_qh(uhci, uhci->skelqh[i]);
			uhci->skelqh[i] = NULL;
		}

	if (uhci->term_td) {
		uhci_free_td(uhci, uhci->term_td);
		uhci->term_td = NULL;
	}

	if (uhci->qh_pool) {
		dma_pool_destroy(uhci->qh_pool);
		uhci->qh_pool = NULL;
	}

	if (uhci->td_pool) {
		dma_pool_destroy(uhci->td_pool);
		uhci->td_pool = NULL;
	}

	if (uhci->fl) {
		dma_free_coherent(uhci_dev(uhci), sizeof(*uhci->fl),
				uhci->fl, uhci->fl->dma_handle);
		uhci->fl = NULL;
	}

	if (uhci->dentry) {
		debugfs_remove(uhci->dentry);
		uhci->dentry = NULL;
	}
}

static int uhci_reset(struct usb_hcd *hcd)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);
	unsigned io_size = (unsigned) hcd->rsrc_len;
	int port;

	uhci->io_addr = (unsigned long) hcd->rsrc_start;

	/* The UHCI spec says devices must have 2 ports, and goes on to say
	 * they may have more but gives no way to determine how many there
	 * are.  However according to the UHCI spec, Bit 7 of the port
	 * status and control register is always set to 1.  So we try to
	 * use this to our advantage.  Another common failure mode when
	 * a nonexistent register is addressed is to return all ones, so
	 * we test for that also.
	 */
	for (port = 0; port < (io_size - USBPORTSC1) / 2; port++) {
		unsigned int portstatus;

		portstatus = inw(uhci->io_addr + USBPORTSC1 + (port * 2));
		if (!(portstatus & 0x0080) || portstatus == 0xffff)
			break;
	}
	if (debug)
		dev_info(uhci_dev(uhci), "detected %d ports\n", port);

	/* Anything greater than 7 is weird so we'll ignore it. */
	if (port > UHCI_RH_MAXCHILD) {
		dev_info(uhci_dev(uhci), "port count misdetected? "
				"forcing to 2 ports\n");
		port = 2;
	}
	uhci->rh_numports = port;

	/* Kick BIOS off this hardware and reset if the controller
	 * isn't already safely quiescent.
	 */
	check_and_reset_hc(uhci);
	return 0;
}

/* Make sure the controller is quiescent and that we're not using it
 * any more.  This is mainly for the benefit of programs which, like kexec,
 * expect the hardware to be idle: not doing DMA or generating IRQs.
 *
 * This routine may be called in a damaged or failing kernel.  Hence we
 * do not acquire the spinlock before shutting down the controller.
 */
static void uhci_shutdown(struct pci_dev *pdev)
{
	struct usb_hcd *hcd = (struct usb_hcd *) pci_get_drvdata(pdev);

	hc_died(hcd_to_uhci(hcd));
}

/*
 * Allocate a frame list, and then setup the skeleton
 *
 * The hardware doesn't really know any difference
 * in the queues, but the order does matter for the
 * protocols higher up. The order is:
 *
 *  - any isochronous events handled before any
 *    of the queues. We don't do that here, because
 *    we'll create the actual TD entries on demand.
 *  - The first queue is the interrupt queue.
 *  - The second queue is the control queue, split into low- and full-speed
 *  - The third queue is bulk queue.
 *  - The fourth queue is the bandwidth reclamation queue, which loops back
 *    to the full-speed control queue.
 */
static int uhci_start(struct usb_hcd *hcd)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);
	int retval = -EBUSY;
	int i;
	dma_addr_t dma_handle;
	struct dentry *dentry;

	hcd->uses_new_polling = 1;
	if (pci_find_capability(to_pci_dev(uhci_dev(uhci)), PCI_CAP_ID_PM))
		hcd->can_wakeup = 1;		/* Assume it supports PME# */

	dentry = debugfs_create_file(hcd->self.bus_name,
			S_IFREG|S_IRUGO|S_IWUSR, uhci_debugfs_root, uhci,
			&uhci_debug_operations);
	if (!dentry) {
		dev_err(uhci_dev(uhci),
				"couldn't create uhci debugfs entry\n");
		retval = -ENOMEM;
		goto err_create_debug_entry;
	}
	uhci->dentry = dentry;

	uhci->fsbr = 0;
	uhci->fsbrtimeout = 0;

	spin_lock_init(&uhci->lock);
	INIT_LIST_HEAD(&uhci->qh_remove_list);

	INIT_LIST_HEAD(&uhci->td_remove_list);

	INIT_LIST_HEAD(&uhci->urb_remove_list);

	INIT_LIST_HEAD(&uhci->urb_list);

	INIT_LIST_HEAD(&uhci->complete_list);

	init_waitqueue_head(&uhci->waitqh);

	uhci->fl = dma_alloc_coherent(uhci_dev(uhci), sizeof(*uhci->fl),
			&dma_handle, 0);
	if (!uhci->fl) {
		dev_err(uhci_dev(uhci), "unable to allocate "
				"consistent memory for frame list\n");
		goto err_alloc_fl;
	}

	memset((void *)uhci->fl, 0, sizeof(*uhci->fl));

	uhci->fl->dma_handle = dma_handle;

	uhci->td_pool = dma_pool_create("uhci_td", uhci_dev(uhci),
			sizeof(struct uhci_td), 16, 0);
	if (!uhci->td_pool) {
		dev_err(uhci_dev(uhci), "unable to create td dma_pool\n");
		goto err_create_td_pool;
	}

	uhci->qh_pool = dma_pool_create("uhci_qh", uhci_dev(uhci),
			sizeof(struct uhci_qh), 16, 0);
	if (!uhci->qh_pool) {
		dev_err(uhci_dev(uhci), "unable to create qh dma_pool\n");
		goto err_create_qh_pool;
	}

	uhci->term_td = uhci_alloc_td(uhci);
	if (!uhci->term_td) {
		dev_err(uhci_dev(uhci), "unable to allocate terminating TD\n");
		goto err_alloc_term_td;
	}

	for (i = 0; i < UHCI_NUM_SKELQH; i++) {
		uhci->skelqh[i] = uhci_alloc_qh(uhci);
		if (!uhci->skelqh[i]) {
			dev_err(uhci_dev(uhci), "unable to allocate QH\n");
			goto err_alloc_skelqh;
		}
	}

	/*
	 * 8 Interrupt queues; link all higher int queues to int1,
	 * then link int1 to control and control to bulk
	 */
	uhci->skel_int128_qh->link =
			uhci->skel_int64_qh->link =
			uhci->skel_int32_qh->link =
			uhci->skel_int16_qh->link =
			uhci->skel_int8_qh->link =
			uhci->skel_int4_qh->link =
			uhci->skel_int2_qh->link =
			cpu_to_le32(uhci->skel_int1_qh->dma_handle) | UHCI_PTR_QH;
	uhci->skel_int1_qh->link = cpu_to_le32(uhci->skel_ls_control_qh->dma_handle) | UHCI_PTR_QH;

	uhci->skel_ls_control_qh->link = cpu_to_le32(uhci->skel_fs_control_qh->dma_handle) | UHCI_PTR_QH;
	uhci->skel_fs_control_qh->link = cpu_to_le32(uhci->skel_bulk_qh->dma_handle) | UHCI_PTR_QH;
	uhci->skel_bulk_qh->link = cpu_to_le32(uhci->skel_term_qh->dma_handle) | UHCI_PTR_QH;

	/* This dummy TD is to work around a bug in Intel PIIX controllers */
	uhci_fill_td(uhci->term_td, 0, (UHCI_NULL_DATA_SIZE << 21) |
		(0x7f << TD_TOKEN_DEVADDR_SHIFT) | USB_PID_IN, 0);
	uhci->term_td->link = cpu_to_le32(uhci->term_td->dma_handle);

	uhci->skel_term_qh->link = UHCI_PTR_TERM;
	uhci->skel_term_qh->element = cpu_to_le32(uhci->term_td->dma_handle);

	/*
	 * Fill the frame list: make all entries point to the proper
	 * interrupt queue.
	 *
	 * The interrupt queues will be interleaved as evenly as possible.
	 * There's not much to be done about period-1 interrupts; they have
	 * to occur in every frame.  But we can schedule period-2 interrupts
	 * in odd-numbered frames, period-4 interrupts in frames congruent
	 * to 2 (mod 4), and so on.  This way each frame only has two
	 * interrupt QHs, which will help spread out bandwidth utilization.
	 */
	for (i = 0; i < UHCI_NUMFRAMES; i++) {
		int irq;

		/*
		 * ffs (Find First bit Set) does exactly what we need:
		 * 1,3,5,...  => ffs = 0 => use skel_int2_qh = skelqh[6],
		 * 2,6,10,... => ffs = 1 => use skel_int4_qh = skelqh[5], etc.
		 * ffs > 6 => not on any high-period queue, so use
		 *	skel_int1_qh = skelqh[7].
		 * Add UHCI_NUMFRAMES to insure at least one bit is set.
		 */
		irq = 6 - (int) __ffs(i + UHCI_NUMFRAMES);
		if (irq < 0)
			irq = 7;

		/* Only place we don't use the frame list routines */
		uhci->fl->frame[i] = UHCI_PTR_QH |
				cpu_to_le32(uhci->skelqh[irq]->dma_handle);
	}

	/*
	 * Some architectures require a full mb() to enforce completion of
	 * the memory writes above before the I/O transfers in configure_hc().
	 */
	mb();

	configure_hc(uhci);
	start_rh(uhci);
	return 0;

/*
 * error exits:
 */
err_alloc_skelqh:
	for (i = 0; i < UHCI_NUM_SKELQH; i++)
		if (uhci->skelqh[i]) {
			uhci_free_qh(uhci, uhci->skelqh[i]);
			uhci->skelqh[i] = NULL;
		}

	uhci_free_td(uhci, uhci->term_td);
	uhci->term_td = NULL;

err_alloc_term_td:
	dma_pool_destroy(uhci->qh_pool);
	uhci->qh_pool = NULL;

err_create_qh_pool:
	dma_pool_destroy(uhci->td_pool);
	uhci->td_pool = NULL;

err_create_td_pool:
	dma_free_coherent(uhci_dev(uhci), sizeof(*uhci->fl),
			uhci->fl, uhci->fl->dma_handle);
	uhci->fl = NULL;

err_alloc_fl:
	debugfs_remove(uhci->dentry);
	uhci->dentry = NULL;

err_create_debug_entry:
	return retval;
}

static void uhci_stop(struct usb_hcd *hcd)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);

	spin_lock_irq(&uhci->lock);
	if (!uhci->hc_inaccessible)
		reset_hc(uhci);
	uhci_scan_schedule(uhci, NULL);
	spin_unlock_irq(&uhci->lock);

	release_uhci(uhci);
}

#ifdef CONFIG_PM
static int uhci_rh_suspend(struct usb_hcd *hcd)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);

	spin_lock_irq(&uhci->lock);
	if (!uhci->hc_inaccessible)		/* Not dead */
		suspend_rh(uhci, UHCI_RH_SUSPENDED);
	spin_unlock_irq(&uhci->lock);
	return 0;
}

static int uhci_rh_resume(struct usb_hcd *hcd)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);
	int rc = 0;

	spin_lock_irq(&uhci->lock);
	if (uhci->hc_inaccessible) {
		if (uhci->rh_state == UHCI_RH_SUSPENDED) {
			dev_warn(uhci_dev(uhci), "HC isn't running!\n");
			rc = -ENODEV;
		}
		/* Otherwise the HC is dead */
	} else
		wakeup_rh(uhci);
	spin_unlock_irq(&uhci->lock);
	return rc;
}

static int uhci_suspend(struct usb_hcd *hcd, pm_message_t message)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);
	int rc = 0;

	dev_dbg(uhci_dev(uhci), "%s\n", __FUNCTION__);

	spin_lock_irq(&uhci->lock);
	if (uhci->hc_inaccessible)	/* Dead or already suspended */
		goto done;

#ifndef CONFIG_USB_SUSPEND
	/* Otherwise this would never happen */
	suspend_rh(uhci, UHCI_RH_SUSPENDED);
#endif

	if (uhci->rh_state > UHCI_RH_SUSPENDED) {
		dev_warn(uhci_dev(uhci), "Root hub isn't suspended!\n");
		hcd->state = HC_STATE_RUNNING;
		rc = -EBUSY;
		goto done;
	};

	/* All PCI host controllers are required to disable IRQ generation
	 * at the source, so we must turn off PIRQ.
	 */
	pci_write_config_word(to_pci_dev(uhci_dev(uhci)), USBLEGSUP, 0);
	uhci->hc_inaccessible = 1;
	hcd->poll_rh = 0;

	/* FIXME: Enable non-PME# remote wakeup? */

done:
	spin_unlock_irq(&uhci->lock);
	return rc;
}

static int uhci_resume(struct usb_hcd *hcd)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);

	dev_dbg(uhci_dev(uhci), "%s\n", __FUNCTION__);

	if (uhci->rh_state == UHCI_RH_RESET)	/* Dead */
		return 0;
	spin_lock_irq(&uhci->lock);

	/* FIXME: Disable non-PME# remote wakeup? */

	uhci->hc_inaccessible = 0;

	/* The BIOS may have changed the controller settings during a
	 * system wakeup.  Check it and reconfigure to avoid problems.
	 */
	check_and_reset_hc(uhci);
	configure_hc(uhci);

#ifndef CONFIG_USB_SUSPEND
	/* Otherwise this would never happen */
	wakeup_rh(uhci);
#endif
	if (uhci->rh_state == UHCI_RH_RESET)
		suspend_rh(uhci, UHCI_RH_SUSPENDED);

	spin_unlock_irq(&uhci->lock);

	if (!uhci->working_RD) {
		/* Suspended root hub needs to be polled */
		hcd->poll_rh = 1;
		usb_hcd_poll_rh_status(hcd);
	}
	return 0;
}
#endif

/* Wait until all the URBs for a particular device/endpoint are gone */
static void uhci_hcd_endpoint_disable(struct usb_hcd *hcd,
		struct usb_host_endpoint *ep)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);

	wait_event_interruptible(uhci->waitqh, list_empty(&ep->urb_list));
}

static int uhci_hcd_get_frame_number(struct usb_hcd *hcd)
{
	struct uhci_hcd *uhci = hcd_to_uhci(hcd);
	unsigned long flags;
	int is_stopped;
	int frame_number;

	/* Minimize latency by avoiding the spinlock */
	local_irq_save(flags);
	is_stopped = uhci->is_stopped;
	smp_rmb();
	frame_number = (is_stopped ? uhci->frame_number :
			inw(uhci->io_addr + USBFRNUM));
	local_irq_restore(flags);
	return frame_number;
}

static const char hcd_name[] = "uhci_hcd";

static const struct hc_driver uhci_driver = {
	.description =		hcd_name,
	.product_desc =		"UHCI Host Controller",
	.hcd_priv_size =	sizeof(struct uhci_hcd),

	/* Generic hardware linkage */
	.irq =			uhci_irq,
	.flags =		HCD_USB11,

	/* Basic lifecycle operations */
	.reset =		uhci_reset,
	.start =		uhci_start,
#ifdef CONFIG_PM
	.suspend =		uhci_suspend,
	.resume =		uhci_resume,
	.hub_suspend =		uhci_rh_suspend,
	.hub_resume =		uhci_rh_resume,
#endif
	.stop =			uhci_stop,

	.urb_enqueue =		uhci_urb_enqueue,
	.urb_dequeue =		uhci_urb_dequeue,

	.endpoint_disable =	uhci_hcd_endpoint_disable,
	.get_frame_number =	uhci_hcd_get_frame_number,

	.hub_status_data =	uhci_hub_status_data,
	.hub_control =		uhci_hub_control,
};

static const struct pci_device_id uhci_pci_ids[] = { {
	/* handle any USB UHCI controller */
	PCI_DEVICE_CLASS(((PCI_CLASS_SERIAL_USB << 8) | 0x00), ~0),
	.driver_data =	(unsigned long) &uhci_driver,
	}, { /* end: all zeroes */ }
};

MODULE_DEVICE_TABLE(pci, uhci_pci_ids);

static struct pci_driver uhci_pci_driver = {
	.name =		(char *)hcd_name,
	.id_table =	uhci_pci_ids,

	.probe =	usb_hcd_pci_probe,
	.remove =	usb_hcd_pci_remove,
	.shutdown =	uhci_shutdown,

#ifdef	CONFIG_PM
	.suspend =	usb_hcd_pci_suspend,
	.resume =	usb_hcd_pci_resume,
#endif	/* PM */
};
 
static int __init uhci_hcd_init(void)
{
	int retval = -ENOMEM;

	printk(KERN_INFO DRIVER_DESC " " DRIVER_VERSION "\n");

	if (usb_disabled())
		return -ENODEV;

	if (debug) {
		errbuf = kmalloc(ERRBUF_LEN, GFP_KERNEL);
		if (!errbuf)
			goto errbuf_failed;
	}

	uhci_debugfs_root = debugfs_create_dir("uhci", NULL);
	if (!uhci_debugfs_root)
		goto debug_failed;

	uhci_up_cachep = kmem_cache_create("uhci_urb_priv",
		sizeof(struct urb_priv), 0, 0, NULL, NULL);
	if (!uhci_up_cachep)
		goto up_failed;

	retval = pci_register_driver(&uhci_pci_driver);
	if (retval)
		goto init_failed;

	return 0;

init_failed:
	if (kmem_cache_destroy(uhci_up_cachep))
		warn("not all urb_priv's were freed!");

up_failed:
	debugfs_remove(uhci_debugfs_root);

debug_failed:
	kfree(errbuf);

errbuf_failed:

	return retval;
}

static void __exit uhci_hcd_cleanup(void) 
{
	pci_unregister_driver(&uhci_pci_driver);
	
	if (kmem_cache_destroy(uhci_up_cachep))
		warn("not all urb_priv's were freed!");

	debugfs_remove(uhci_debugfs_root);
	kfree(errbuf);
}

module_init(uhci_hcd_init);
module_exit(uhci_hcd_cleanup);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
