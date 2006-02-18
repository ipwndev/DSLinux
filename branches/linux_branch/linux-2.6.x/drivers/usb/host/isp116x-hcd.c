/*
 * ISP116x HCD (Host Controller Driver) for USB.
 *
 * Derived from the SL811 HCD, rewritten for ISP116x.
 * Copyright (C) 2005 Olav Kongas <ok@artecdesign.ee>
 *
 * Portions:
 * Copyright (C) 2004 Psion Teklogix (for NetBook PRO)
 * Copyright (C) 2004 David Brownell
 *
 * Periodic scheduling is based on Roman's OHCI code
 * Copyright (C) 1999 Roman Weissgaerber
 *
 */

/*
 * The driver basically works. A number of people have used it with a range
 * of devices.
 *
 * The driver passes all usbtests 1-14.
 *
 * Suspending/resuming of root hub via sysfs works. Remote wakeup works too.
 * And suspending/resuming of platform device works too. Suspend/resume
 * via HCD operations vector is not implemented.
 *
 * Iso transfer support is not implemented. Adding this would include
 * implementing recovery from the failure to service the processed ITL
 * fifo ram in time, which will involve chip reset.
 *
 * TODO:
 + More testing of suspend/resume.
*/

/*
  ISP116x chips require certain delays between accesses to its
  registers. The following timing options exist.

  1. Configure your memory controller (the best)
  2. Implement platform-specific delay function possibly
  combined with configuring the memory controller; see
  include/linux/usb-isp116x.h for more info. Some broken
  memory controllers line LH7A400 SMC need this. Also,
  uncomment for that to work the following
  USE_PLATFORM_DELAY macro.
  3. Use ndelay (easiest, poorest). For that, uncomment
  the following USE_NDELAY macro.
*/
#define USE_PLATFORM_DELAY
//#define USE_NDELAY

//#define DEBUG
//#define VERBOSE
/* Transfer descriptors. See dump_ptd() for printout format  */
//#define PTD_TRACE
/* enqueuing/finishing log of urbs */
//#define URB_TRACE

#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/usb.h>
#include <linux/usb_isp116x.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/byteorder.h>

#ifndef DEBUG
#	define	STUB_DEBUG_FILE
#endif

#include "../core/hcd.h"
#include "isp116x.h"

#define DRIVER_VERSION	"05 Aug 2005"
#define DRIVER_DESC	"ISP116x USB Host Controller Driver"

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

static const char hcd_name[] = "isp116x-hcd";

/*-----------------------------------------------------------------*/

/*
  Write len bytes to fifo, pad till 32-bit boundary
 */
static void write_ptddata_to_fifo(struct isp116x *isp116x, void *buf, int len)
{
	u8 *dp = (u8 *) buf;
	u16 *dp2 = (u16 *) buf;
	u16 w;
	int quot = len % 4;

	if ((unsigned long)dp2 & 1) {
		/* not aligned */
		for (; len > 1; len -= 2) {
			w = *dp++;
			w |= *dp++ << 8;
			isp116x_raw_write_data16(isp116x, w);
		}
		if (len)
			isp116x_write_data16(isp116x, (u16) * dp);
	} else {
		/* aligned */
		for (; len > 1; len -= 2)
			isp116x_raw_write_data16(isp116x, *dp2++);
		if (len)
			isp116x_write_data16(isp116x, 0xff & *((u8 *) dp2));
	}
	if (quot == 1 || quot == 2)
		isp116x_raw_write_data16(isp116x, 0);
}

/*
  Read len bytes from fifo and then read till 32-bit boundary.
 */
static void read_ptddata_from_fifo(struct isp116x *isp116x, void *buf, int len)
{
	u8 *dp = (u8 *) buf;
	u16 *dp2 = (u16 *) buf;
	u16 w;
	int quot = len % 4;

	if ((unsigned long)dp2 & 1) {
		/* not aligned */
		for (; len > 1; len -= 2) {
			w = isp116x_raw_read_data16(isp116x);
			*dp++ = w & 0xff;
			*dp++ = (w >> 8) & 0xff;
		}
		if (len)
			*dp = 0xff & isp116x_read_data16(isp116x);
	} else {
		/* aligned */
		for (; len > 1; len -= 2)
			*dp2++ = isp116x_raw_read_data16(isp116x);
		if (len)
			*(u8 *) dp2 = 0xff & isp116x_read_data16(isp116x);
	}
	if (quot == 1 || quot == 2)
		isp116x_raw_read_data16(isp116x);
}

/*
  Write ptd's and data for scheduled transfers into
  the fifo ram. Fifo must be empty and ready.
*/
static void pack_fifo(struct isp116x *isp116x)
{
	struct isp116x_ep *ep;
	struct ptd *ptd;
	int buflen = isp116x->atl_last_dir == PTD_DIR_IN
	    ? isp116x->atl_bufshrt : isp116x->atl_buflen;
	int ptd_count = 0;

	isp116x_write_reg16(isp116x, HCuPINT, HCuPINT_AIIEOT);
	isp116x_write_reg16(isp116x, HCXFERCTR, buflen);
	isp116x_write_addr(isp116x, HCATLPORT | ISP116x_WRITE_OFFSET);
	for (ep = isp116x->atl_active; ep; ep = ep->active) {
		++ptd_count;
		ptd = &ep->ptd;
		dump_ptd(ptd);
		dump_ptd_out_data(ptd, ep->data);
		isp116x_write_data16(isp116x, ptd->count);
		isp116x_write_data16(isp116x, ptd->mps);
		isp116x_write_data16(isp116x, ptd->len);
		isp116x_write_data16(isp116x, ptd->faddr);
		buflen -= sizeof(struct ptd);
		/* Skip writing data for last IN PTD */
		if (ep->active || (isp116x->atl_last_dir != PTD_DIR_IN)) {
			write_ptddata_to_fifo(isp116x, ep->data, ep->length);
			buflen -= ALIGN(ep->length, 4);
		}
	}
	BUG_ON(buflen);
}

/*
  Read the processed ptd's and data from fifo ram back to
  URBs' buffers. Fifo must be full and done
*/
static void unpack_fifo(struct isp116x *isp116x)
{
	struct isp116x_ep *ep;
	struct ptd *ptd;
	int buflen = isp116x->atl_last_dir == PTD_DIR_IN
	    ? isp116x->atl_buflen : isp116x->atl_bufshrt;

	isp116x_write_reg16(isp116x, HCuPINT, HCuPINT_AIIEOT);
	isp116x_write_reg16(isp116x, HCXFERCTR, buflen);
	isp116x_write_addr(isp116x, HCATLPORT);
	for (ep = isp116x->atl_active; ep; ep = ep->active) {
		ptd = &ep->ptd;
		ptd->count = isp116x_read_data16(isp116x);
		ptd->mps = isp116x_read_data16(isp116x);
		ptd->len = isp116x_read_data16(isp116x);
		ptd->faddr = isp116x_read_data16(isp116x);
		buflen -= sizeof(struct ptd);
		/* Skip reading data for last Setup or Out PTD */
		if (ep->active || (isp116x->atl_last_dir == PTD_DIR_IN)) {
			read_ptddata_from_fifo(isp116x, ep->data, ep->length);
			buflen -= ALIGN(ep->length, 4);
		}
		dump_ptd(ptd);
		dump_ptd_in_data(ptd, ep->data);
	}
	BUG_ON(buflen);
}

/*---------------------------------------------------------------*/

/*
  Set up PTD's.
*/
static void preproc_atl_queue(struct isp116x *isp116x)
{
	struct isp116x_ep *ep;
	struct urb *urb;
	struct ptd *ptd;
	u16 len;

	for (ep = isp116x->atl_active; ep; ep = ep->active) {
		u16 toggle = 0, dir = PTD_DIR_SETUP;

		BUG_ON(list_empty(&ep->hep->urb_list));
		urb = container_of(ep->hep->urb_list.next,
				   struct urb, urb_list);
		ptd = &ep->ptd;
		len = ep->length;
		spin_lock(&urb->lock);
		ep->data = (unsigned char *)urb->transfer_buffer
		    + urb->actual_length;

		switch (ep->nextpid) {
		case USB_PID_IN:
			toggle = usb_gettoggle(urb->dev, ep->epnum, 0);
			dir = PTD_DIR_IN;
			break;
		case USB_PID_OUT:
			toggle = usb_gettoggle(urb->dev, ep->epnum, 1);
			dir = PTD_DIR_OUT;
			break;
		case USB_PID_SETUP:
			len = sizeof(struct usb_ctrlrequest);
			ep->data = urb->setup_packet;
			break;
		case USB_PID_ACK:
			toggle = 1;
			len = 0;
			dir = (urb->transfer_buffer_length
			       && usb_pipein(urb->pipe))
			    ? PTD_DIR_OUT : PTD_DIR_IN;
			break;
		default:
			ERR("%s %d: ep->nextpid %d\n", __func__, __LINE__,
			    ep->nextpid);
			BUG();
		}

		ptd->count = PTD_CC_MSK | PTD_ACTIVE_MSK | PTD_TOGGLE(toggle);
		ptd->mps = PTD_MPS(ep->maxpacket)
		    | PTD_SPD(urb->dev->speed == USB_SPEED_LOW)
		    | PTD_EP(ep->epnum);
		ptd->len = PTD_LEN(len) | PTD_DIR(dir);
		ptd->faddr = PTD_FA(usb_pipedevice(urb->pipe));
		spin_unlock(&urb->lock);
		if (!ep->active) {
			ptd->mps |= PTD_LAST_MSK;
			isp116x->atl_last_dir = dir;
		}
		isp116x->atl_bufshrt = sizeof(struct ptd) + isp116x->atl_buflen;
		isp116x->atl_buflen = isp116x->atl_bufshrt + ALIGN(len, 4);
	}
}

/*
  Analyze transfer results, handle partial transfers and errors
*/
static void postproc_atl_queue(struct isp116x *isp116x)
{
	struct isp116x_ep *ep;
	struct urb *urb;
	struct usb_device *udev;
	struct ptd *ptd;
	int short_not_ok;
	u8 cc;

	for (ep = isp116x->atl_active; ep; ep = ep->active) {
		BUG_ON(list_empty(&ep->hep->urb_list));
		urb =
		    container_of(ep->hep->urb_list.next, struct urb, urb_list);
		udev = urb->dev;
		ptd = &ep->ptd;
		cc = PTD_GET_CC(ptd);

		spin_lock(&urb->lock);
		short_not_ok = 1;

		/* Data underrun is special. For allowed underrun
		   we clear the error and continue as normal. For
		   forbidden underrun we finish the DATA stage
		   immediately while for control transfer,
		   we do a STATUS stage. */
		if (cc == TD_DATAUNDERRUN) {
			if (!(urb->transfer_flags & URB_SHORT_NOT_OK)) {
				DBG("Allowed data underrun\n");
				cc = TD_CC_NOERROR;
				short_not_ok = 0;
			} else {
				ep->error_count = 1;
				if (usb_pipecontrol(urb->pipe))
					ep->nextpid = USB_PID_ACK;
				else
					usb_settoggle(udev, ep->epnum,
						      ep->nextpid ==
						      USB_PID_OUT,
						      PTD_GET_TOGGLE(ptd));
				urb->actual_length += PTD_GET_COUNT(ptd);
				urb->status = cc_to_error[TD_DATAUNDERRUN];
				spin_unlock(&urb->lock);
				continue;
			}
		}
		/* Keep underrun error through the STATUS stage */
		if (urb->status == cc_to_error[TD_DATAUNDERRUN])
			cc = TD_DATAUNDERRUN;

		if (cc != TD_CC_NOERROR && cc != TD_NOTACCESSED
		    && (++ep->error_count >= 3 || cc == TD_CC_STALL
			|| cc == TD_DATAOVERRUN)) {
			if (urb->status == -EINPROGRESS)
				urb->status = cc_to_error[cc];
			if (ep->nextpid == USB_PID_ACK)
				ep->nextpid = 0;
			spin_unlock(&urb->lock);
			continue;
		}
		/* According to usb spec, zero-length Int transfer signals
		   finishing of the urb. Hey, does this apply only
		   for IN endpoints? */
		if (usb_pipeint(urb->pipe) && !PTD_GET_LEN(ptd)) {
			if (urb->status == -EINPROGRESS)
				urb->status = 0;
			spin_unlock(&urb->lock);
			continue;
		}

		/* Relax after previously failed, but later succeeded
		   or correctly NAK'ed retransmission attempt */
		if (ep->error_count
		    && (cc == TD_CC_NOERROR || cc == TD_NOTACCESSED))
			ep->error_count = 0;

		/* Take into account idiosyncracies of the isp116x chip
		   regarding toggle bit for failed transfers */
		if (ep->nextpid == USB_PID_OUT)
			usb_settoggle(udev, ep->epnum, 1, PTD_GET_TOGGLE(ptd)
				      ^ (ep->error_count > 0));
		else if (ep->nextpid == USB_PID_IN)
			usb_settoggle(udev, ep->epnum, 0, PTD_GET_TOGGLE(ptd)
				      ^ (ep->error_count > 0));

		switch (ep->nextpid) {
		case USB_PID_IN:
		case USB_PID_OUT:
			urb->actual_length += PTD_GET_COUNT(ptd);
			if (PTD_GET_ACTIVE(ptd)
			    || (cc != TD_CC_NOERROR && cc < 0x0E))
				break;
			if (urb->transfer_buffer_length != urb->actual_length) {
				if (short_not_ok)
					break;
			} else {
				if (urb->transfer_flags & URB_ZERO_PACKET
				    && ep->nextpid == USB_PID_OUT
				    && !(PTD_GET_COUNT(ptd) % ep->maxpacket)) {
					DBG("Zero packet requested\n");
					break;
				}
			}
			/* All data for this URB is transferred, let's finish */
			if (usb_pipecontrol(urb->pipe))
				ep->nextpid = USB_PID_ACK;
			else if (urb->status == -EINPROGRESS)
				urb->status = 0;
			break;
		case USB_PID_SETUP:
			if (PTD_GET_ACTIVE(ptd)
			    || (cc != TD_CC_NOERROR && cc < 0x0E))
				break;
			if (urb->transfer_buffer_length == urb->actual_length)
				ep->nextpid = USB_PID_ACK;
			else if (usb_pipeout(urb->pipe)) {
				usb_settoggle(udev, 0, 1, 1);
				ep->nextpid = USB_PID_OUT;
			} else {
				usb_settoggle(udev, 0, 0, 1);
				ep->nextpid = USB_PID_IN;
			}
			break;
		case USB_PID_ACK:
			if (PTD_GET_ACTIVE(ptd)
			    || (cc != TD_CC_NOERROR && cc < 0x0E))
				break;
			if (urb->status == -EINPROGRESS)
				urb->status = 0;
			ep->nextpid = 0;
			break;
		default:
			BUG_ON(1);
		}
		spin_unlock(&urb->lock);
	}
}

/*
  Take done or failed requests out of schedule. Give back
  processed urbs.
*/
static void finish_request(struct isp116x *isp116x, struct isp116x_ep *ep,
			   struct urb *urb, struct pt_regs *regs)
__releases(isp116x->lock) __acquires(isp116x->lock)
{
	unsigned i;

	urb->hcpriv = NULL;
	ep->error_count = 0;

	if (usb_pipecontrol(urb->pipe))
		ep->nextpid = USB_PID_SETUP;

	urb_dbg(urb, "Finish");

	spin_unlock(&isp116x->lock);
	usb_hcd_giveback_urb(isp116x_to_hcd(isp116x), urb, regs);
	spin_lock(&isp116x->lock);

	/* take idle endpoints out of the schedule */
	if (!list_empty(&ep->hep->urb_list))
		return;

	/* async deschedule */
	if (!list_empty(&ep->schedule)) {
		list_del_init(&ep->schedule);
		return;
	}

	/* periodic deschedule */
	DBG("deschedule qh%d/%p branch %d\n", ep->period, ep, ep->branch);
	for (i = ep->branch; i < PERIODIC_SIZE; i += ep->period) {
		struct isp116x_ep *temp;
		struct isp116x_ep **prev = &isp116x->periodic[i];

		while (*prev && ((temp = *prev) != ep))
			prev = &temp->next;
		if (*prev)
			*prev = ep->next;
		isp116x->load[i] -= ep->load;
	}
	ep->branch = PERIODIC_SIZE;
	isp116x_to_hcd(isp116x)->self.bandwidth_allocated -=
	    ep->load / ep->period;

	/* switch irq type? */
	if (!--isp116x->periodic_count) {
		isp116x->irqenb &= ~HCuPINT_SOF;
		isp116x->irqenb |= HCuPINT_ATL;
	}
}

/*
  Scan transfer lists, schedule transfers, send data off
  to chip.
 */
static void start_atl_transfers(struct isp116x *isp116x)
{
	struct isp116x_ep *last_ep = NULL, *ep;
	struct urb *urb;
	u16 load = 0;
	int len, index, speed, byte_time;

	if (atomic_read(&isp116x->atl_finishing))
		return;

	if (!HC_IS_RUNNING(isp116x_to_hcd(isp116x)->state))
		return;

	/* FIFO not empty? */
	if (isp116x_read_reg16(isp116x, HCBUFSTAT) & HCBUFSTAT_ATL_FULL)
		return;

	isp116x->atl_active = NULL;
	isp116x->atl_buflen = isp116x->atl_bufshrt = 0;

	/* Schedule int transfers */
	if (isp116x->periodic_count) {
		isp116x->fmindex = index =
		    (isp116x->fmindex + 1) & (PERIODIC_SIZE - 1);
		if ((load = isp116x->load[index])) {
			/* Bring all int transfers for this frame
			   into the active queue */
			isp116x->atl_active = last_ep =
			    isp116x->periodic[index];
			while (last_ep->next)
				last_ep = (last_ep->active = last_ep->next);
			last_ep->active = NULL;
		}
	}

	/* Schedule control/bulk transfers */
	list_for_each_entry(ep, &isp116x->async, schedule) {
		urb = container_of(ep->hep->urb_list.next,
				   struct urb, urb_list);
		speed = urb->dev->speed;
		byte_time = speed == USB_SPEED_LOW
		    ? BYTE_TIME_LOWSPEED : BYTE_TIME_FULLSPEED;

		if (ep->nextpid == USB_PID_SETUP) {
			len = sizeof(struct usb_ctrlrequest);
		} else if (ep->nextpid == USB_PID_ACK) {
			len = 0;
		} else {
			/* Find current free length ... */
			len = (MAX_LOAD_LIMIT - load) / byte_time;

			/* ... then limit it to configured max size ... */
			len = min(len, speed == USB_SPEED_LOW ?
				  MAX_TRANSFER_SIZE_LOWSPEED :
				  MAX_TRANSFER_SIZE_FULLSPEED);

			/* ... and finally cut to the multiple of MaxPacketSize,
			   or to the real length if there's enough room. */
			if (len <
			    (urb->transfer_buffer_length -
			     urb->actual_length)) {
				len -= len % ep->maxpacket;
				if (!len)
					continue;
			} else
				len = urb->transfer_buffer_length -
				    urb->actual_length;
			BUG_ON(len < 0);
		}

		load += len * byte_time;
		if (load > MAX_LOAD_LIMIT)
			break;

		ep->active = NULL;
		ep->length = len;
		if (last_ep)
			last_ep->active = ep;
		else
			isp116x->atl_active = ep;
		last_ep = ep;
	}

	/* Avoid starving of endpoints */
	if ((&isp116x->async)->next != (&isp116x->async)->prev)
		list_move(&isp116x->async, (&isp116x->async)->next);

	if (isp116x->atl_active) {
		preproc_atl_queue(isp116x);
		pack_fifo(isp116x);
	}
}

/*
  Finish the processed transfers
*/
static void finish_atl_transfers(struct isp116x *isp116x, struct pt_regs *regs)
{
	struct isp116x_ep *ep;
	struct urb *urb;

	if (!isp116x->atl_active)
		return;
	/* Fifo not ready? */
	if (!(isp116x_read_reg16(isp116x, HCBUFSTAT) & HCBUFSTAT_ATL_DONE))
		return;

	atomic_inc(&isp116x->atl_finishing);
	unpack_fifo(isp116x);
	postproc_atl_queue(isp116x);
	for (ep = isp116x->atl_active; ep; ep = ep->active) {
		urb =
		    container_of(ep->hep->urb_list.next, struct urb, urb_list);
		/* USB_PID_ACK check here avoids finishing of
		   control transfers, for which TD_DATAUNDERRUN
		   occured, while URB_SHORT_NOT_OK was set */
		if (urb && urb->status != -EINPROGRESS
		    && ep->nextpid != USB_PID_ACK)
			finish_request(isp116x, ep, urb, regs);
	}
	atomic_dec(&isp116x->atl_finishing);
}

static irqreturn_t isp116x_irq(struct usb_hcd *hcd, struct pt_regs *regs)
{
	struct isp116x *isp116x = hcd_to_isp116x(hcd);
	u16 irqstat;
	irqreturn_t ret = IRQ_NONE;

	spin_lock(&isp116x->lock);
	isp116x_write_reg16(isp116x, HCuPINTENB, 0);
	irqstat = isp116x_read_reg16(isp116x, HCuPINT);
	isp116x_write_reg16(isp116x, HCuPINT, irqstat);

	if (irqstat & (HCuPINT_ATL | HCuPINT_SOF)) {
		ret = IRQ_HANDLED;
		finish_atl_transfers(isp116x, regs);
	}

	if (irqstat & HCuPINT_OPR) {
		u32 intstat = isp116x_read_reg32(isp116x, HCINTSTAT);
		isp116x_write_reg32(isp116x, HCINTSTAT, intstat);
		if (intstat & HCINT_UE) {
			ERR("Unrecoverable error\n");
			/* What should we do here? Reset?  */
		}
		if (intstat & HCINT_RHSC)
			/* When root hub or any of its ports is going
			   to come out of suspend, it may take more
			   than 10ms for status bits to stabilize. */
			mod_timer(&hcd->rh_timer, jiffies
				  + msecs_to_jiffies(20) + 1);
		if (intstat & HCINT_RD) {
			DBG("---- remote wakeup\n");
			schedule_work(&isp116x->rh_resume);
			ret = IRQ_HANDLED;
		}
		irqstat &= ~HCuPINT_OPR;
		ret = IRQ_HANDLED;
	}

	if (irqstat & (HCuPINT_ATL | HCuPINT_SOF)) {
		start_atl_transfers(isp116x);
	}

	isp116x_write_reg16(isp116x, HCuPINTENB, isp116x->irqenb);
	spin_unlock(&isp116x->lock);
	return ret;
}

/*-----------------------------------------------------------------*/

/* usb 1.1 says max 90% of a frame is available for periodic transfers.
 * this driver doesn't promise that much since it's got to handle an
 * IRQ per packet; irq handling latencies also use up that time.
 */

/* out of 1000 us */
#define	MAX_PERIODIC_LOAD	600
static int balance(struct isp116x *isp116x, u16 period, u16 load)
{
	int i, branch = -ENOSPC;

	/* search for the least loaded schedule branch of that period
	   which has enough bandwidth left unreserved. */
	for (i = 0; i < period; i++) {
		if (branch < 0 || isp116x->load[branch] > isp116x->load[i]) {
			int j;

			for (j = i; j < PERIODIC_SIZE; j += period) {
				if ((isp116x->load[j] + load)
				    > MAX_PERIODIC_LOAD)
					break;
			}
			if (j < PERIODIC_SIZE)
				continue;
			branch = i;
		}
	}
	return branch;
}

/* NB! ALL the code above this point runs with isp116x->lock
   held, irqs off
*/

/*-----------------------------------------------------------------*/

static int isp116x_urb_enqueue(struct usb_hcd *hcd,
			       struct usb_host_endpoint *hep, struct urb *urb,
			       unsigned mem_flags)
{
	struct isp116x *isp116x = hcd_to_isp116x(hcd);
	struct usb_device *udev = urb->dev;
	unsigned int pipe = urb->pipe;
	int is_out = !usb_pipein(pipe);
	int type = usb_pipetype(pipe);
	int epnum = usb_pipeendpoint(pipe);
	struct isp116x_ep *ep = NULL;
	unsigned long flags;
	int i;
	int ret = 0;

	urb_dbg(urb, "Enqueue");

	if (type == PIPE_ISOCHRONOUS) {
		ERR("Isochronous transfers not supported\n");
		urb_dbg(urb, "Refused to enqueue");
		return -ENXIO;
	}
	/* avoid all allocations within spinlocks: request or endpoint */
	if (!hep->hcpriv) {
		ep = kzalloc(sizeof *ep, mem_flags);
		if (!ep)
			return -ENOMEM;
	}

	spin_lock_irqsave(&isp116x->lock, flags);
	if (!HC_IS_RUNNING(hcd->state)) {
		ret = -ENODEV;
		goto fail;
	}

	if (hep->hcpriv)
		ep = hep->hcpriv;
	else {
		INIT_LIST_HEAD(&ep->schedule);
		ep->udev = usb_get_dev(udev);
		ep->epnum = epnum;
		ep->maxpacket = usb_maxpacket(udev, urb->pipe, is_out);
		usb_settoggle(udev, epnum, is_out, 0);

		if (type == PIPE_CONTROL) {
			ep->nextpid = USB_PID_SETUP;
		} else if (is_out) {
			ep->nextpid = USB_PID_OUT;
		} else {
			ep->nextpid = USB_PID_IN;
		}

		if (urb->interval) {
			/*
			   With INT URBs submitted, the driver works with SOF
			   interrupt enabled and ATL interrupt disabled. After
			   the PTDs are written to fifo ram, the chip starts
			   fifo processing and usb transfers after the next
			   SOF and continues until the transfers are finished
			   (succeeded or failed) or the frame ends. Therefore,
			   the transfers occur only in every second frame,
			   while fifo reading/writing and data processing
			   occur in every other second frame. */
			if (urb->interval < 2)
				urb->interval = 2;
			if (urb->interval > 2 * PERIODIC_SIZE)
				urb->interval = 2 * PERIODIC_SIZE;
			ep->period = urb->interval >> 1;
			ep->branch = PERIODIC_SIZE;
			ep->load = usb_calc_bus_time(udev->speed,
						     !is_out,
						     (type == PIPE_ISOCHRONOUS),
						     usb_maxpacket(udev, pipe,
								   is_out)) /
			    1000;
		}
		hep->hcpriv = ep;
		ep->hep = hep;
	}

	/* maybe put endpoint into schedule */
	switch (type) {
	case PIPE_CONTROL:
	case PIPE_BULK:
		if (list_empty(&ep->schedule))
			list_add_tail(&ep->schedule, &isp116x->async);
		break;
	case PIPE_INTERRUPT:
		urb->interval = ep->period;
		ep->length = min((int)ep->maxpacket,
				 urb->transfer_buffer_length);

		/* urb submitted for already existing endpoint */
		if (ep->branch < PERIODIC_SIZE)
			break;

		ret = ep->branch = balance(isp116x, ep->period, ep->load);
		if (ret < 0)
			goto fail;
		ret = 0;

		urb->start_frame = (isp116x->fmindex & (PERIODIC_SIZE - 1))
		    + ep->branch;

		/* sort each schedule branch by period (slow before fast)
		   to share the faster parts of the tree without needing
		   dummy/placeholder nodes */
		DBG("schedule qh%d/%p branch %d\n", ep->period, ep, ep->branch);
		for (i = ep->branch; i < PERIODIC_SIZE; i += ep->period) {
			struct isp116x_ep **prev = &isp116x->periodic[i];
			struct isp116x_ep *here = *prev;

			while (here && ep != here) {
				if (ep->period > here->period)
					break;
				prev = &here->next;
				here = *prev;
			}
			if (ep != here) {
				ep->next = here;
				*prev = ep;
			}
			isp116x->load[i] += ep->load;
		}
		hcd->self.bandwidth_allocated += ep->load / ep->period;

		/* switch over to SOFint */
		if (!isp116x->periodic_count++) {
			isp116x->irqenb &= ~HCuPINT_ATL;
			isp116x->irqenb |= HCuPINT_SOF;
			isp116x_write_reg16(isp116x, HCuPINTENB,
					    isp116x->irqenb);
		}
	}

	/* in case of unlink-during-submit */
	spin_lock(&urb->lock);
	if (urb->status != -EINPROGRESS) {
		spin_unlock(&urb->lock);
		finish_request(isp116x, ep, urb, NULL);
		ret = 0;
		goto fail;
	}
	urb->hcpriv = hep;
	spin_unlock(&urb->lock);
	start_atl_transfers(isp116x);

      fail:
	spin_unlock_irqrestore(&isp116x->lock, flags);
	return ret;
}

/*
   Dequeue URBs.
*/
static int isp116x_urb_dequeue(struct usb_hcd *hcd, struct urb *urb)
{
	struct isp116x *isp116x = hcd_to_isp116x(hcd);
	struct usb_host_endpoint *hep;
	struct isp116x_ep *ep, *ep_act;
	unsigned long flags;

	spin_lock_irqsave(&isp116x->lock, flags);
	hep = urb->hcpriv;
	/* URB already unlinked (or never linked)? */
	if (!hep) {
		spin_unlock_irqrestore(&isp116x->lock, flags);
		return 0;
	}
	ep = hep->hcpriv;
	WARN_ON(hep != ep->hep);

	/* In front of queue? */
	if (ep->hep->urb_list.next == &urb->urb_list)
		/* active? */
		for (ep_act = isp116x->atl_active; ep_act;
		     ep_act = ep_act->active)
			if (ep_act == ep) {
				VDBG("dequeue, urb %p active; wait for irq\n",
				     urb);
				urb = NULL;
				break;
			}

	if (urb)
		finish_request(isp116x, ep, urb, NULL);

	spin_unlock_irqrestore(&isp116x->lock, flags);
	return 0;
}

static void isp116x_endpoint_disable(struct usb_hcd *hcd,
				     struct usb_host_endpoint *hep)
{
	int i;
	struct isp116x_ep *ep = hep->hcpriv;;

	if (!ep)
		return;

	/* assume we'd just wait for the irq */
	for (i = 0; i < 100 && !list_empty(&hep->urb_list); i++)
		msleep(3);
	if (!list_empty(&hep->urb_list))
		WARN("ep %p not empty?\n", ep);

	usb_put_dev(ep->udev);
	kfree(ep);
	hep->hcpriv = NULL;
}

static int isp116x_get_frame(struct usb_hcd *hcd)
{
	struct isp116x *isp116x = hcd_to_isp116x(hcd);
	u32 fmnum;
	unsigned long flags;

	spin_lock_irqsave(&isp116x->lock, flags);
	fmnum = isp116x_read_reg32(isp116x, HCFMNUM);
	spin_unlock_irqrestore(&isp116x->lock, flags);
	return (int)fmnum;
}

/*----------------------------------------------------------------*/

/*
  Adapted from ohci-hub.c. Currently we don't support autosuspend.
*/
static int isp116x_hub_status_data(struct usb_hcd *hcd, char *buf)
{
	struct isp116x *isp116x = hcd_to_isp116x(hcd);
	int ports, i, changed = 0;
	unsigned long flags;

	if (!HC_IS_RUNNING(hcd->state))
		return -ESHUTDOWN;

	/* Report no status change now, if we are scheduled to be
	   called later */
	if (timer_pending(&hcd->rh_timer))
		return 0;

	ports = isp116x->rhdesca & RH_A_NDP;
	spin_lock_irqsave(&isp116x->lock, flags);
	isp116x->rhstatus = isp116x_read_reg32(isp116x, HCRHSTATUS);
	if (isp116x->rhstatus & (RH_HS_LPSC | RH_HS_OCIC))
		buf[0] = changed = 1;
	else
		buf[0] = 0;

	for (i = 0; i < ports; i++) {
		u32 status = isp116x->rhport[i] =
		    isp116x_read_reg32(isp116x, i ? HCRHPORT2 : HCRHPORT1);

		if (status & (RH_PS_CSC | RH_PS_PESC | RH_PS_PSSC
			      | RH_PS_OCIC | RH_PS_PRSC)) {
			changed = 1;
			buf[0] |= 1 << (i + 1);
			continue;
		}
	}
	spin_unlock_irqrestore(&isp116x->lock, flags);
	return changed;
}

static void isp116x_hub_descriptor(struct isp116x *isp116x,
				   struct usb_hub_descriptor *desc)
{
	u32 reg = isp116x->rhdesca;

	desc->bDescriptorType = 0x29;
	desc->bDescLength = 9;
	desc->bHubContrCurrent = 0;
	desc->bNbrPorts = (u8) (reg & 0x3);
	/* Power switching, device type, overcurrent. */
	desc->wHubCharacteristics =
	    (__force __u16) cpu_to_le16((u16) ((reg >> 8) & 0x1f));
	desc->bPwrOn2PwrGood = (u8) ((reg >> 24) & 0xff);
	/* two bitmaps:  ports removable, and legacy PortPwrCtrlMask */
	desc->bitmap[0] = desc->bNbrPorts == 1 ? 1 << 1 : 3 << 1;
	desc->bitmap[1] = ~0;
}

/* Perform reset of a given port.
   It would be great to just start the reset and let the
   USB core to clear the reset in due time. However,
   root hub ports should be reset for at least 50 ms, while
   our chip stays in reset for about 10 ms. I.e., we must
   repeatedly reset it ourself here.
*/
static inline void root_port_reset(struct isp116x *isp116x, unsigned port)
{
	u32 tmp;
	unsigned long flags, t;

	/* Root hub reset should be 50 ms, but some devices
	   want it even longer. */
	t = jiffies + msecs_to_jiffies(100);

	while (time_before(jiffies, t)) {
		spin_lock_irqsave(&isp116x->lock, flags);
		/* spin until any current reset finishes */
		for (;;) {
			tmp = isp116x_read_reg32(isp116x, port ?
						 HCRHPORT2 : HCRHPORT1);
			if (!(tmp & RH_PS_PRS))
				break;
			udelay(500);
		}
		/* Don't reset a disconnected port */
		if (!(tmp & RH_PS_CCS)) {
			spin_unlock_irqrestore(&isp116x->lock, flags);
			break;
		}
		/* Reset lasts 10ms (claims datasheet) */
		isp116x_write_reg32(isp116x, port ? HCRHPORT2 :
				    HCRHPORT1, (RH_PS_PRS));
		spin_unlock_irqrestore(&isp116x->lock, flags);
		msleep(10);
	}
}

/* Adapted from ohci-hub.c */
static int isp116x_hub_control(struct usb_hcd *hcd,
			       u16 typeReq,
			       u16 wValue, u16 wIndex, char *buf, u16 wLength)
{
	struct isp116x *isp116x = hcd_to_isp116x(hcd);
	int ret = 0;
	unsigned long flags;
	int ports = isp116x->rhdesca & RH_A_NDP;
	u32 tmp = 0;

	switch (typeReq) {
	case ClearHubFeature:
		DBG("ClearHubFeature: ");
		switch (wValue) {
		case C_HUB_OVER_CURRENT:
			DBG("C_HUB_OVER_CURRENT\n");
			spin_lock_irqsave(&isp116x->lock, flags);
			isp116x_write_reg32(isp116x, HCRHSTATUS, RH_HS_OCIC);
			spin_unlock_irqrestore(&isp116x->lock, flags);
		case C_HUB_LOCAL_POWER:
			DBG("C_HUB_LOCAL_POWER\n");
			break;
		default:
			goto error;
		}
		break;
	case SetHubFeature:
		DBG("SetHubFeature: ");
		switch (wValue) {
		case C_HUB_OVER_CURRENT:
		case C_HUB_LOCAL_POWER:
			DBG("C_HUB_OVER_CURRENT or C_HUB_LOCAL_POWER\n");
			break;
		default:
			goto error;
		}
		break;
	case GetHubDescriptor:
		DBG("GetHubDescriptor\n");
		isp116x_hub_descriptor(isp116x,
				       (struct usb_hub_descriptor *)buf);
		break;
	case GetHubStatus:
		DBG("GetHubStatus\n");
		*(__le32 *) buf = 0;
		break;
	case GetPortStatus:
		DBG("GetPortStatus\n");
		if (!wIndex || wIndex > ports)
			goto error;
		tmp = isp116x->rhport[--wIndex];
		*(__le32 *) buf = cpu_to_le32(tmp);
		DBG("GetPortStatus: port[%d]  %08x\n", wIndex + 1, tmp);
		break;
	case ClearPortFeature:
		DBG("ClearPortFeature: ");
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;

		switch (wValue) {
		case USB_PORT_FEAT_ENABLE:
			DBG("USB_PORT_FEAT_ENABLE\n");
			tmp = RH_PS_CCS;
			break;
		case USB_PORT_FEAT_C_ENABLE:
			DBG("USB_PORT_FEAT_C_ENABLE\n");
			tmp = RH_PS_PESC;
			break;
		case USB_PORT_FEAT_SUSPEND:
			DBG("USB_PORT_FEAT_SUSPEND\n");
			tmp = RH_PS_POCI;
			break;
		case USB_PORT_FEAT_C_SUSPEND:
			DBG("USB_PORT_FEAT_C_SUSPEND\n");
			tmp = RH_PS_PSSC;
			break;
		case USB_PORT_FEAT_POWER:
			DBG("USB_PORT_FEAT_POWER\n");
			tmp = RH_PS_LSDA;
			break;
		case USB_PORT_FEAT_C_CONNECTION:
			DBG("USB_PORT_FEAT_C_CONNECTION\n");
			tmp = RH_PS_CSC;
			break;
		case USB_PORT_FEAT_C_OVER_CURRENT:
			DBG("USB_PORT_FEAT_C_OVER_CURRENT\n");
			tmp = RH_PS_OCIC;
			break;
		case USB_PORT_FEAT_C_RESET:
			DBG("USB_PORT_FEAT_C_RESET\n");
			tmp = RH_PS_PRSC;
			break;
		default:
			goto error;
		}
		spin_lock_irqsave(&isp116x->lock, flags);
		isp116x_write_reg32(isp116x, wIndex
				    ? HCRHPORT2 : HCRHPORT1, tmp);
		isp116x->rhport[wIndex] =
		    isp116x_read_reg32(isp116x, wIndex ? HCRHPORT2 : HCRHPORT1);
		spin_unlock_irqrestore(&isp116x->lock, flags);
		break;
	case SetPortFeature:
		DBG("SetPortFeature: ");
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		switch (wValue) {
		case USB_PORT_FEAT_SUSPEND:
			DBG("USB_PORT_FEAT_SUSPEND\n");
			spin_lock_irqsave(&isp116x->lock, flags);
			isp116x_write_reg32(isp116x, wIndex
					    ? HCRHPORT2 : HCRHPORT1, RH_PS_PSS);
			break;
		case USB_PORT_FEAT_POWER:
			DBG("USB_PORT_FEAT_POWER\n");
			spin_lock_irqsave(&isp116x->lock, flags);
			isp116x_write_reg32(isp116x, wIndex
					    ? HCRHPORT2 : HCRHPORT1, RH_PS_PPS);
			break;
		case USB_PORT_FEAT_RESET:
			DBG("USB_PORT_FEAT_RESET\n");
			root_port_reset(isp116x, wIndex);
			spin_lock_irqsave(&isp116x->lock, flags);
			break;
		default:
			goto error;
		}
		isp116x->rhport[wIndex] =
		    isp116x_read_reg32(isp116x, wIndex ? HCRHPORT2 : HCRHPORT1);
		spin_unlock_irqrestore(&isp116x->lock, flags);
		break;

	default:
	      error:
		/* "protocol stall" on error */
		DBG("PROTOCOL STALL\n");
		ret = -EPIPE;
	}
	return ret;
}

#ifdef	CONFIG_PM

static int isp116x_hub_suspend(struct usb_hcd *hcd)
{
	struct isp116x *isp116x = hcd_to_isp116x(hcd);
	unsigned long flags;
	u32 val;
	int ret = 0;

	spin_lock_irqsave(&isp116x->lock, flags);

	val = isp116x_read_reg32(isp116x, HCCONTROL);
	switch (val & HCCONTROL_HCFS) {
	case HCCONTROL_USB_OPER:
		hcd->state = HC_STATE_QUIESCING;
		val &= (~HCCONTROL_HCFS & ~HCCONTROL_RWE);
		val |= HCCONTROL_USB_SUSPEND;
		if (hcd->remote_wakeup)
			val |= HCCONTROL_RWE;
		/* Wait for usb transfers to finish */
		mdelay(2);
		isp116x_write_reg32(isp116x, HCCONTROL, val);
		hcd->state = HC_STATE_SUSPENDED;
		/* Wait for devices to suspend */
		mdelay(5);
	case HCCONTROL_USB_SUSPEND:
		break;
	case HCCONTROL_USB_RESUME:
		isp116x_write_reg32(isp116x, HCCONTROL,
				    (val & ~HCCONTROL_HCFS) |
				    HCCONTROL_USB_RESET);
	case HCCONTROL_USB_RESET:
		ret = -EBUSY;
		break;
	default:
		ret = -EINVAL;
	}

	spin_unlock_irqrestore(&isp116x->lock, flags);
	return ret;
}

static int isp116x_hub_resume(struct usb_hcd *hcd)
{
	struct isp116x *isp116x = hcd_to_isp116x(hcd);
	u32 val;
	int ret = -EINPROGRESS;

	msleep(5);
	spin_lock_irq(&isp116x->lock);

	val = isp116x_read_reg32(isp116x, HCCONTROL);
	switch (val & HCCONTROL_HCFS) {
	case HCCONTROL_USB_SUSPEND:
		val &= ~HCCONTROL_HCFS;
		val |= HCCONTROL_USB_RESUME;
		isp116x_write_reg32(isp116x, HCCONTROL, val);
	case HCCONTROL_USB_RESUME:
		break;
	case HCCONTROL_USB_OPER:
		/* Without setting power_state here the
		   SUSPENDED state won't be removed from
		   sysfs/usbN/power.state as a response to remote
		   wakeup. Maybe in the future. */
		hcd->self.root_hub->dev.power.power_state = PMSG_ON;
		ret = 0;
		break;
	default:
		ret = -EBUSY;
	}

	if (ret != -EINPROGRESS) {
		spin_unlock_irq(&isp116x->lock);
		return ret;
	}

	val = isp116x->rhdesca & RH_A_NDP;
	while (val--) {
		u32 stat =
		    isp116x_read_reg32(isp116x, val ? HCRHPORT2 : HCRHPORT1);
		/* force global, not selective, resume */
		if (!(stat & RH_PS_PSS))
			continue;
		DBG("%s: Resuming port %d\n", __func__, val);
		isp116x_write_reg32(isp116x, RH_PS_POCI, val
				    ? HCRHPORT2 : HCRHPORT1);
	}
	spin_unlock_irq(&isp116x->lock);

	hcd->state = HC_STATE_RESUMING;
	mdelay(20);

	/* Go operational */
	spin_lock_irq(&isp116x->lock);
	val = isp116x_read_reg32(isp116x, HCCONTROL);
	isp116x_write_reg32(isp116x, HCCONTROL,
			    (val & ~HCCONTROL_HCFS) | HCCONTROL_USB_OPER);
	spin_unlock_irq(&isp116x->lock);
	/* see analogous comment above */
	hcd->self.root_hub->dev.power.power_state = PMSG_ON;
	hcd->state = HC_STATE_RUNNING;

	return 0;
}

static void isp116x_rh_resume(void *_hcd)
{
	struct usb_hcd *hcd = _hcd;

	usb_resume_device(hcd->self.root_hub);
}

#else

#define	isp116x_hub_suspend	NULL
#define	isp116x_hub_resume	NULL

static void isp116x_rh_resume(void *_hcd)
{
}

#endif

/*-----------------------------------------------------------------*/

#ifdef STUB_DEBUG_FILE

static inline void create_debug_file(struct isp116x *isp116x)
{
}

static inline void remove_debug_file(struct isp116x *isp116x)
{
}

#else

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static void dump_irq(struct seq_file *s, char *label, u16 mask)
{
	seq_printf(s, "%s %04x%s%s%s%s%s%s\n", label, mask,
		   mask & HCuPINT_CLKRDY ? " clkrdy" : "",
		   mask & HCuPINT_SUSP ? " susp" : "",
		   mask & HCuPINT_OPR ? " opr" : "",
		   mask & HCuPINT_AIIEOT ? " eot" : "",
		   mask & HCuPINT_ATL ? " atl" : "",
		   mask & HCuPINT_SOF ? " sof" : "");
}

static void dump_int(struct seq_file *s, char *label, u32 mask)
{
	seq_printf(s, "%s %08x%s%s%s%s%s%s%s\n", label, mask,
		   mask & HCINT_MIE ? " MIE" : "",
		   mask & HCINT_RHSC ? " rhsc" : "",
		   mask & HCINT_FNO ? " fno" : "",
		   mask & HCINT_UE ? " ue" : "",
		   mask & HCINT_RD ? " rd" : "",
		   mask & HCINT_SF ? " sof" : "", mask & HCINT_SO ? " so" : "");
}

static int proc_isp116x_show(struct seq_file *s, void *unused)
{
	struct isp116x *isp116x = s->private;
	struct isp116x_ep *ep;
	struct urb *urb;
	unsigned i;
	char *str;

	seq_printf(s, "%s\n%s version %s\n",
		   isp116x_to_hcd(isp116x)->product_desc, hcd_name,
		   DRIVER_VERSION);

	if (HC_IS_SUSPENDED(isp116x_to_hcd(isp116x)->state)) {
		seq_printf(s, "HCD is suspended\n");
		return 0;
	}
	if (!HC_IS_RUNNING(isp116x_to_hcd(isp116x)->state)) {
		seq_printf(s, "HCD not running\n");
		return 0;
	}

	spin_lock_irq(&isp116x->lock);

	dump_irq(s, "hc_irq_enable", isp116x_read_reg16(isp116x, HCuPINTENB));
	dump_irq(s, "hc_irq_status", isp116x_read_reg16(isp116x, HCuPINT));
	dump_int(s, "hc_int_enable", isp116x_read_reg32(isp116x, HCINTENB));
	dump_int(s, "hc_int_status", isp116x_read_reg32(isp116x, HCINTSTAT));

	list_for_each_entry(ep, &isp116x->async, schedule) {

		switch (ep->nextpid) {
		case USB_PID_IN:
			str = "in";
			break;
		case USB_PID_OUT:
			str = "out";
			break;
		case USB_PID_SETUP:
			str = "setup";
			break;
		case USB_PID_ACK:
			str = "status";
			break;
		default:
			str = "?";
			break;
		};
		seq_printf(s, "%p, ep%d%s, maxpacket %d:\n", ep,
			   ep->epnum, str, ep->maxpacket);
		list_for_each_entry(urb, &ep->hep->urb_list, urb_list) {
			seq_printf(s, "  urb%p, %d/%d\n", urb,
				   urb->actual_length,
				   urb->transfer_buffer_length);
		}
	}
	if (!list_empty(&isp116x->async))
		seq_printf(s, "\n");

	seq_printf(s, "periodic size= %d\n", PERIODIC_SIZE);

	for (i = 0; i < PERIODIC_SIZE; i++) {
		ep = isp116x->periodic[i];
		if (!ep)
			continue;
		seq_printf(s, "%2d [%3d]:\n", i, isp116x->load[i]);

		/* DUMB: prints shared entries multiple times */
		do {
			seq_printf(s, "   %d/%p (%sdev%d ep%d%s max %d)\n",
				   ep->period, ep,
				   (ep->udev->speed ==
				    USB_SPEED_FULL) ? "" : "ls ",
				   ep->udev->devnum, ep->epnum,
				   (ep->epnum ==
				    0) ? "" : ((ep->nextpid ==
						USB_PID_IN) ? "in" : "out"),
				   ep->maxpacket);
			ep = ep->next;
		} while (ep);
	}
	spin_unlock_irq(&isp116x->lock);
	seq_printf(s, "\n");

	return 0;
}

static int proc_isp116x_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_isp116x_show, PDE(inode)->data);
}

static struct file_operations proc_ops = {
	.open = proc_isp116x_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

/* expect just one isp116x per system */
static const char proc_filename[] = "driver/isp116x";

static void create_debug_file(struct isp116x *isp116x)
{
	struct proc_dir_entry *pde;

	pde = create_proc_entry(proc_filename, 0, NULL);
	if (pde == NULL)
		return;

	pde->proc_fops = &proc_ops;
	pde->data = isp116x;
	isp116x->pde = pde;
}

static void remove_debug_file(struct isp116x *isp116x)
{
	if (isp116x->pde)
		remove_proc_entry(proc_filename, NULL);
}

#endif

/*-----------------------------------------------------------------*/

/*
  Software reset - can be called from any contect.
*/
static int isp116x_sw_reset(struct isp116x *isp116x)
{
	int retries = 15;
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&isp116x->lock, flags);
	isp116x_write_reg16(isp116x, HCSWRES, HCSWRES_MAGIC);
	isp116x_write_reg32(isp116x, HCCMDSTAT, HCCMDSTAT_HCR);
	while (--retries) {
		/* It usually resets within 1 ms */
		mdelay(1);
		if (!(isp116x_read_reg32(isp116x, HCCMDSTAT) & HCCMDSTAT_HCR))
			break;
	}
	if (!retries) {
		ERR("Software reset timeout\n");
		ret = -ETIME;
	}
	spin_unlock_irqrestore(&isp116x->lock, flags);
	return ret;
}

static int isp116x_reset(struct usb_hcd *hcd)
{
	struct isp116x *isp116x = hcd_to_isp116x(hcd);
	unsigned long t;
	u16 clkrdy = 0;
	int ret = 0, timeout = 15 /* ms */ ;

	ret = isp116x_sw_reset(isp116x);
	if (ret)
		return ret;

	t = jiffies + msecs_to_jiffies(timeout);
	while (time_before_eq(jiffies, t)) {
		msleep(4);
		spin_lock_irq(&isp116x->lock);
		clkrdy = isp116x_read_reg16(isp116x, HCuPINT) & HCuPINT_CLKRDY;
		spin_unlock_irq(&isp116x->lock);
		if (clkrdy)
			break;
	}
	if (!clkrdy) {
		ERR("Clock not ready after 20ms\n");
		/* After sw_reset the clock won't report to be ready, if
		   H_WAKEUP pin is high. */
		ERR("Please make sure that the H_WAKEUP pin is pulled low!\n");
		ret = -ENODEV;
	}
	return ret;
}

static void isp116x_stop(struct usb_hcd *hcd)
{
	struct isp116x *isp116x = hcd_to_isp116x(hcd);
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&isp116x->lock, flags);
	isp116x_write_reg16(isp116x, HCuPINTENB, 0);

	/* Switch off ports' power, some devices don't come up
	   after next 'insmod' without this */
	val = isp116x_read_reg32(isp116x, HCRHDESCA);
	val &= ~(RH_A_NPS | RH_A_PSM);
	isp116x_write_reg32(isp116x, HCRHDESCA, val);
	isp116x_write_reg32(isp116x, HCRHSTATUS, RH_HS_LPS);
	spin_unlock_irqrestore(&isp116x->lock, flags);

	isp116x_sw_reset(isp116x);
}

/*
  Configure the chip. The chip must be successfully reset by now.
*/
static int isp116x_start(struct usb_hcd *hcd)
{
	struct isp116x *isp116x = hcd_to_isp116x(hcd);
	struct isp116x_platform_data *board = isp116x->board;
	u32 val;
	unsigned long flags;

	spin_lock_irqsave(&isp116x->lock, flags);

	/* clear interrupt status and disable all interrupt sources */
	isp116x_write_reg16(isp116x, HCuPINT, 0xff);
	isp116x_write_reg16(isp116x, HCuPINTENB, 0);

	val = isp116x_read_reg16(isp116x, HCCHIPID);
	if ((val & HCCHIPID_MASK) != HCCHIPID_MAGIC) {
		ERR("Invalid chip ID %04x\n", val);
		spin_unlock_irqrestore(&isp116x->lock, flags);
		return -ENODEV;
	}

	/* To be removed in future */
	hcd->uses_new_polling = 1;

	isp116x_write_reg16(isp116x, HCITLBUFLEN, ISP116x_ITL_BUFSIZE);
	isp116x_write_reg16(isp116x, HCATLBUFLEN, ISP116x_ATL_BUFSIZE);

	/* ----- HW conf */
	val = HCHWCFG_INT_ENABLE | HCHWCFG_DBWIDTH(1);
	if (board->sel15Kres)
		val |= HCHWCFG_15KRSEL;
	/* Remote wakeup won't work without working clock */
	if (board->remote_wakeup_enable)
		val |= HCHWCFG_CLKNOTSTOP;
	if (board->oc_enable)
		val |= HCHWCFG_ANALOG_OC;
	if (board->int_act_high)
		val |= HCHWCFG_INT_POL;
	if (board->int_edge_triggered)
		val |= HCHWCFG_INT_TRIGGER;
	isp116x_write_reg16(isp116x, HCHWCFG, val);

	/* ----- Root hub conf */
	val = (25 << 24) & RH_A_POTPGT;
	/* AN10003_1.pdf recommends RH_A_NPS (no power switching) to
	   be always set. Yet, instead, we request individual port
	   power switching. */
	val |= RH_A_PSM;
	/* Report overcurrent per port */
	val |= RH_A_OCPM;
	isp116x_write_reg32(isp116x, HCRHDESCA, val);
	isp116x->rhdesca = isp116x_read_reg32(isp116x, HCRHDESCA);

	val = RH_B_PPCM;
	isp116x_write_reg32(isp116x, HCRHDESCB, val);
	isp116x->rhdescb = isp116x_read_reg32(isp116x, HCRHDESCB);

	val = 0;
	if (board->remote_wakeup_enable) {
		hcd->can_wakeup = 1;
		val |= RH_HS_DRWE;
	}
	isp116x_write_reg32(isp116x, HCRHSTATUS, val);
	isp116x->rhstatus = isp116x_read_reg32(isp116x, HCRHSTATUS);

	isp116x_write_reg32(isp116x, HCFMINTVL, 0x27782edf);

	hcd->state = HC_STATE_RUNNING;

	/* Set up interrupts */
	isp116x->intenb = HCINT_MIE | HCINT_RHSC | HCINT_UE;
	if (board->remote_wakeup_enable)
		isp116x->intenb |= HCINT_RD;
	isp116x->irqenb = HCuPINT_ATL | HCuPINT_OPR;	/* | HCuPINT_SUSP; */
	isp116x_write_reg32(isp116x, HCINTENB, isp116x->intenb);
	isp116x_write_reg16(isp116x, HCuPINTENB, isp116x->irqenb);

	/* Go operational */
	val = HCCONTROL_USB_OPER;
	if (board->remote_wakeup_enable)
		val |= HCCONTROL_RWE;
	isp116x_write_reg32(isp116x, HCCONTROL, val);

	/* Disable ports to avoid race in device enumeration */
	isp116x_write_reg32(isp116x, HCRHPORT1, RH_PS_CCS);
	isp116x_write_reg32(isp116x, HCRHPORT2, RH_PS_CCS);

	isp116x_show_regs(isp116x);
	spin_unlock_irqrestore(&isp116x->lock, flags);
	return 0;
}

/*-----------------------------------------------------------------*/

static struct hc_driver isp116x_hc_driver = {
	.description = hcd_name,
	.product_desc = "ISP116x Host Controller",
	.hcd_priv_size = sizeof(struct isp116x),

	.irq = isp116x_irq,
	.flags = HCD_USB11,

	.reset = isp116x_reset,
	.start = isp116x_start,
	.stop = isp116x_stop,

	.urb_enqueue = isp116x_urb_enqueue,
	.urb_dequeue = isp116x_urb_dequeue,
	.endpoint_disable = isp116x_endpoint_disable,

	.get_frame_number = isp116x_get_frame,

	.hub_status_data = isp116x_hub_status_data,
	.hub_control = isp116x_hub_control,
	.hub_suspend = isp116x_hub_suspend,
	.hub_resume = isp116x_hub_resume,
};

/*----------------------------------------------------------------*/

static int __init_or_module isp116x_remove(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	struct isp116x *isp116x;
	struct platform_device *pdev;
	struct resource *res;

	if (!hcd)
		return 0;
	isp116x = hcd_to_isp116x(hcd);
	pdev = container_of(dev, struct platform_device, dev);
	remove_debug_file(isp116x);
	usb_remove_hcd(hcd);

	iounmap(isp116x->data_reg);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	release_mem_region(res->start, 2);
	iounmap(isp116x->addr_reg);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, 2);

	usb_put_hcd(hcd);
	return 0;
}

#define resource_len(r) (((r)->end - (r)->start) + 1)

static int __init isp116x_probe(struct device *dev)
{
	struct usb_hcd *hcd;
	struct isp116x *isp116x;
	struct platform_device *pdev;
	struct resource *addr, *data;
	void __iomem *addr_reg;
	void __iomem *data_reg;
	int irq;
	int ret = 0;

	pdev = container_of(dev, struct platform_device, dev);
	if (pdev->num_resources < 3) {
		ret = -ENODEV;
		goto err1;
	}

	data = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	addr = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	irq = platform_get_irq(pdev, 0);
	if (!addr || !data || irq < 0) {
		ret = -ENODEV;
		goto err1;
	}

	if (dev->dma_mask) {
		DBG("DMA not supported\n");
		ret = -EINVAL;
		goto err1;
	}

	if (!request_mem_region(addr->start, 2, hcd_name)) {
		ret = -EBUSY;
		goto err1;
	}
	addr_reg = ioremap(addr->start, resource_len(addr));
	if (addr_reg == NULL) {
		ret = -ENOMEM;
		goto err2;
	}
	if (!request_mem_region(data->start, 2, hcd_name)) {
		ret = -EBUSY;
		goto err3;
	}
	data_reg = ioremap(data->start, resource_len(data));
	if (data_reg == NULL) {
		ret = -ENOMEM;
		goto err4;
	}

	/* allocate and initialize hcd */
	hcd = usb_create_hcd(&isp116x_hc_driver, dev, dev->bus_id);
	if (!hcd) {
		ret = -ENOMEM;
		goto err5;
	}
	/* this rsrc_start is bogus */
	hcd->rsrc_start = addr->start;
	isp116x = hcd_to_isp116x(hcd);
	isp116x->data_reg = data_reg;
	isp116x->addr_reg = addr_reg;
	spin_lock_init(&isp116x->lock);
	INIT_LIST_HEAD(&isp116x->async);
	INIT_WORK(&isp116x->rh_resume, isp116x_rh_resume, hcd);
	isp116x->board = dev->platform_data;

	if (!isp116x->board) {
		ERR("Platform data structure not initialized\n");
		ret = -ENODEV;
		goto err6;
	}
	if (isp116x_check_platform_delay(isp116x)) {
		ERR("USE_PLATFORM_DELAY defined, but delay function not "
		    "implemented.\n");
		ERR("See comments in drivers/usb/host/isp116x-hcd.c\n");
		ret = -ENODEV;
		goto err6;
	}

	ret = usb_add_hcd(hcd, irq, SA_INTERRUPT);
	if (ret != 0)
		goto err6;

	create_debug_file(isp116x);
	return 0;

      err6:
	usb_put_hcd(hcd);
      err5:
	iounmap(data_reg);
      err4:
	release_mem_region(data->start, 2);
      err3:
	iounmap(addr_reg);
      err2:
	release_mem_region(addr->start, 2);
      err1:
	ERR("init error, %d\n", ret);
	return ret;
}

#ifdef	CONFIG_PM
/*
  Suspend of platform device
*/
static int isp116x_suspend(struct device *dev, pm_message_t state, u32 phase)
{
	int ret = 0;
	struct usb_hcd *hcd = dev_get_drvdata(dev);

	VDBG("%s: state %x, phase %x\n", __func__, state, phase);

	if (phase != SUSPEND_DISABLE && phase != SUSPEND_POWER_DOWN)
		return 0;

	ret = usb_suspend_device(hcd->self.root_hub, state);
	if (!ret) {
		dev->power.power_state = state;
		INFO("%s suspended\n", hcd_name);
	} else
		ERR("%s suspend failed\n", hcd_name);

	return ret;
}

/*
  Resume platform device
*/
static int isp116x_resume(struct device *dev, u32 phase)
{
	int ret = 0;
	struct usb_hcd *hcd = dev_get_drvdata(dev);

	VDBG("%s:  state %x, phase %x\n", __func__, dev->power.power_state,
	     phase);
	if (phase != RESUME_POWER_ON)
		return 0;

	ret = usb_resume_device(hcd->self.root_hub);
	if (!ret) {
		dev->power.power_state = PMSG_ON;
		VDBG("%s resumed\n", (char *)hcd_name);
	}
	return ret;
}

#else

#define	isp116x_suspend    NULL
#define	isp116x_resume     NULL

#endif

static struct device_driver isp116x_driver = {
	.name = (char *)hcd_name,
	.bus = &platform_bus_type,
	.probe = isp116x_probe,
	.remove = isp116x_remove,
	.suspend = isp116x_suspend,
	.resume = isp116x_resume,
};

/*-----------------------------------------------------------------*/

static int __init isp116x_init(void)
{
	if (usb_disabled())
		return -ENODEV;

	INFO("driver %s, %s\n", hcd_name, DRIVER_VERSION);
	return driver_register(&isp116x_driver);
}

module_init(isp116x_init);

static void __exit isp116x_cleanup(void)
{
	driver_unregister(&isp116x_driver);
}

module_exit(isp116x_cleanup);
