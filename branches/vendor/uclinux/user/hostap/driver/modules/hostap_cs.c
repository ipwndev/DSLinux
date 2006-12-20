#define PRISM2_PCCARD

#include <linux/config.h>
#ifdef  __IN_PCMCIA_PACKAGE__
#include <pcmcia/k_compat.h>
#endif /* __IN_PCMCIA_PACKAGE__ */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/if.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,44))
#include <linux/tqueue.h>
#else
#include <linux/workqueue.h>
#endif
#include "hostap_wext.h"

#include <pcmcia/version.h>
#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>

#include <asm/io.h>

#include "hostap_wlan.h"


#ifdef __IN_PCMCIA_PACKAGE__
#include <pcmcia/config.h>
#endif /* __IN_PCMCIA_PACKAGE__ */


static char *version = PRISM2_VERSION " (Jouni Malinen <jkmaline@cc.hut.fi>)";
static dev_info_t dev_info = "hostap_cs";
static dev_link_t *dev_list = NULL;

MODULE_AUTHOR("Jouni Malinen");
MODULE_DESCRIPTION("Support for Intersil Prism2-based 802.11 wireless LAN "
		   "cards (PC Card).");
MODULE_SUPPORTED_DEVICE("Intersil Prism2-based WLAN cards (PC Card)");
MODULE_LICENSE("GPL");


static unsigned int irq_mask = 0xdeb8;
MODULE_PARM(irq_mask, "i");

static int irq_list[4] = { -1 };
MODULE_PARM(irq_list, "1-4i");

static int ignore_cis_vcc = 0;
MODULE_PARM(ignore_cis_vcc, "i");
MODULE_PARM_DESC(ignore_cis_vcc, "Ignore broken CIS VCC entry");


#ifdef PRISM2_IO_DEBUG

static inline void hfa384x_outb_debug(struct net_device *dev, int a, u8 v)
{
	struct hostap_interface *iface = dev->priv;
	local_info_t *local = iface->local;
	unsigned long flags;

	spin_lock_irqsave(&local->lock, flags);
	prism2_io_debug_add(dev, PRISM2_IO_DEBUG_CMD_OUTB, a, v);
	outb(v, dev->base_addr + a);
	spin_unlock_irqrestore(&local->lock, flags);
}

static inline u8 hfa384x_inb_debug(struct net_device *dev, int a)
{
	struct hostap_interface *iface = dev->priv;
	local_info_t *local = iface->local;
	unsigned long flags;
	u8 v;

	spin_lock_irqsave(&local->lock, flags);
	v = inb(dev->base_addr + a);
	prism2_io_debug_add(dev, PRISM2_IO_DEBUG_CMD_INB, a, v);
	spin_unlock_irqrestore(&local->lock, flags);
	return v;
}

static inline void hfa384x_outw_debug(struct net_device *dev, int a, u16 v)
{
	struct hostap_interface *iface = dev->priv;
	local_info_t *local = iface->local;
	unsigned long flags;

	spin_lock_irqsave(&local->lock, flags);
	prism2_io_debug_add(dev, PRISM2_IO_DEBUG_CMD_OUTW, a, v);
	outw(v, dev->base_addr + a);
	spin_unlock_irqrestore(&local->lock, flags);
}

static inline u16 hfa384x_inw_debug(struct net_device *dev, int a)
{
	struct hostap_interface *iface = dev->priv;
	local_info_t *local = iface->local;
	unsigned long flags;
	u16 v;

	spin_lock_irqsave(&local->lock, flags);
	v = inw(dev->base_addr + a);
	prism2_io_debug_add(dev, PRISM2_IO_DEBUG_CMD_INW, a, v);
	spin_unlock_irqrestore(&local->lock, flags);
	return v;
}

static inline void hfa384x_outsw_debug(struct net_device *dev, int a,
				       u8 *buf, int wc)
{
	struct hostap_interface *iface = dev->priv;
	local_info_t *local = iface->local;
	unsigned long flags;

	spin_lock_irqsave(&local->lock, flags);
	prism2_io_debug_add(dev, PRISM2_IO_DEBUG_CMD_OUTSW, a, wc);
	outsw(dev->base_addr + a, buf, wc);
	spin_unlock_irqrestore(&local->lock, flags);
}

static inline void hfa384x_insw_debug(struct net_device *dev, int a,
				      u8 *buf, int wc)
{
	struct hostap_interface *iface = dev->priv;
	local_info_t *local = iface->local;
	unsigned long flags;

	spin_lock_irqsave(&local->lock, flags);
	prism2_io_debug_add(dev, PRISM2_IO_DEBUG_CMD_INSW, a, wc);
	insw(dev->base_addr + a, buf, wc);
	spin_unlock_irqrestore(&local->lock, flags);
}

#define HFA384X_OUTB(v,a) hfa384x_outb_debug(dev, (a), (v))
#define HFA384X_INB(a) hfa384x_inb_debug(dev, (a))
#define HFA384X_OUTW(v,a) hfa384x_outw_debug(dev, (a), (v))
#define HFA384X_INW(a) hfa384x_inw_debug(dev, (a))
#define HFA384X_OUTSW(a, buf, wc) hfa384x_outsw_debug(dev, (a), (buf), (wc))
#define HFA384X_INSW(a, buf, wc) hfa384x_insw_debug(dev, (a), (buf), (wc))

#else /* PRISM2_IO_DEBUG */

#define HFA384X_OUTB(v,a) outb((v), dev->base_addr + (a))
#define HFA384X_INB(a) inb(dev->base_addr + (a))
#define HFA384X_OUTW(v,a) outw((v), dev->base_addr + (a))
#define HFA384X_INW(a) inw(dev->base_addr + (a))
#define HFA384X_INSW(a, buf, wc) insw(dev->base_addr + (a), buf, wc)
#define HFA384X_OUTSW(a, buf, wc) outsw(dev->base_addr + (a), buf, wc)

#endif /* PRISM2_IO_DEBUG */


static int hfa384x_from_bap(struct net_device *dev, u16 bap, void *buf,
			    int len)
{
	u16 d_off;
	u16 *pos;

	d_off = (bap == 1) ? HFA384X_DATA1_OFF : HFA384X_DATA0_OFF;
	pos = (u16 *) buf;

	if (len / 2)
		HFA384X_INSW(d_off, buf, len / 2);
	pos += len / 2;

	if (len & 1)
		*((char *) pos) = HFA384X_INB(d_off);

	return 0;
}


static int hfa384x_to_bap(struct net_device *dev, u16 bap, void *buf, int len)
{
	u16 d_off;
	u16 *pos;

	d_off = (bap == 1) ? HFA384X_DATA1_OFF : HFA384X_DATA0_OFF;
	pos = (u16 *) buf;

	if (len / 2)
		HFA384X_OUTSW(d_off, buf, len / 2);
	pos += len / 2;

	if (len & 1)
		HFA384X_OUTB(*((char *) pos), d_off);

	return 0;
}


/* FIX: This might change at some point.. */
#include "hostap_hw.c"



static void prism2_detach(dev_link_t *link);
static void prism2_release(u_long arg);
static int prism2_event(event_t event, int priority,
			event_callback_args_t *args);


static int prism2_pccard_card_present(local_info_t *local)
{
	if (local->link != NULL &&
	    ((local->link->state & (DEV_PRESENT | DEV_CONFIG)) ==
	     (DEV_PRESENT | DEV_CONFIG)))
		return 1;
	return 0;
}

static void prism2_pccard_cor_sreset(local_info_t *local)
{
	int res;
	conf_reg_t reg;

	if (!prism2_pccard_card_present(local))
	       return;

	reg.Function = 0;
	reg.Action = CS_READ;
	reg.Offset = CISREG_COR;
	reg.Value = 0;
	res = pcmcia_access_configuration_register(local->link->handle, &reg);
	if (res != CS_SUCCESS) {
		printk(KERN_DEBUG "prism2_pccard_cor_sreset failed 1 (%d)\n",
		       res);
		return;
	}
	printk(KERN_DEBUG "prism2_pccard_cor_sreset: original COR %02x\n",
	       reg.Value);

	reg.Action = CS_WRITE;
	reg.Value |= COR_SOFT_RESET;
	res = pcmcia_access_configuration_register(local->link->handle, &reg);
	if (res != CS_SUCCESS) {
		printk(KERN_DEBUG "prism2_pccard_cor_sreset failed 2 (%d)\n",
		       res);
		return;
	}

	mdelay(2);

	reg.Value &= ~COR_SOFT_RESET;
	res = pcmcia_access_configuration_register(local->link->handle, &reg);
	if (res != CS_SUCCESS) {
		printk(KERN_DEBUG "prism2_pccard_cor_sreset failed 3 (%d)\n",
		       res);
		return;
	}

	mdelay(2);
}


static void prism2_pccard_genesis_reset(local_info_t *local, int hcr)
{
	int res;
	conf_reg_t reg;
	int old_cor;

	if (!prism2_pccard_card_present(local))
	       return;

	reg.Function = 0;
	reg.Action = CS_READ;
	reg.Offset = CISREG_COR;
	reg.Value = 0;
	res = pcmcia_access_configuration_register(local->link->handle, &reg);
	if (res != CS_SUCCESS) {
		printk(KERN_DEBUG "prism2_pccard_genesis_sreset failed 1 "
		       "(%d)\n", res);
		return;
	}
	printk(KERN_DEBUG "prism2_pccard_genesis_sreset: original COR %02x\n",
	       reg.Value);
	old_cor = reg.Value;

	reg.Action = CS_WRITE;
	reg.Value |= COR_SOFT_RESET;
	res = pcmcia_access_configuration_register(local->link->handle, &reg);
	if (res != CS_SUCCESS) {
		printk(KERN_DEBUG "prism2_pccard_genesis_sreset failed 2 "
		       "(%d)\n", res);
		return;
	}

	mdelay(10);

	/* Setup Genesis mode */
	reg.Action = CS_WRITE;
	reg.Value = hcr;
	reg.Offset = CISREG_CCSR;
	res = pcmcia_access_configuration_register(local->link->handle, &reg);
	if (res != CS_SUCCESS) {
		printk(KERN_DEBUG "prism2_pccard_genesis_sreset failed 3 "
		       "(%d)\n", res);
		return;
	}
	mdelay(10);

	reg.Action = CS_WRITE;
	reg.Offset = CISREG_COR;
	reg.Value = old_cor & ~COR_SOFT_RESET;
	res = pcmcia_access_configuration_register(local->link->handle, &reg);
	if (res != CS_SUCCESS) {
		printk(KERN_DEBUG "prism2_pccard_genesis_sreset failed 4 "
		       "(%d)\n", res);
		return;
	}

	mdelay(10);
}


static int prism2_pccard_dev_open(local_info_t *local)
{
	local->link->open++;
	return 0;
}


static int prism2_pccard_dev_close(local_info_t *local)
{
	if (local == NULL || local->link == NULL)
		return 1;

	if (!local->link->open) {
		printk(KERN_WARNING "%s: prism2_pccard_dev_close(): "
		       "link not open?!\n", local->dev->name);
		return 1;
	}

	local->link->open--;

	return 0;
}


static struct prism2_helper_functions prism2_pccard_funcs =
{
	.card_present	= prism2_pccard_card_present,
	.cor_sreset	= prism2_pccard_cor_sreset,
	.dev_open	= prism2_pccard_dev_open,
	.dev_close	= prism2_pccard_dev_close,
	.genesis_reset	= prism2_pccard_genesis_reset,
	.hw_type	= HOSTAP_HW_PCCARD,
};


#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,5,68)
static void cs_error(client_handle_t handle, int func, int ret)
{
	error_info_t err = { func, ret };
	pcmcia_report_error(handle, &err);
}
#endif


/* allocate local data and register with CardServices
 * initialize dev_link structure, but do not configure the card yet */
static dev_link_t *prism2_attach(void)
{
	dev_link_t *link;
	client_reg_t client_reg;
	int ret;

	link = kmalloc(sizeof(dev_link_t), GFP_KERNEL);
	if (link == NULL)
		return NULL;

	memset(link, 0, sizeof(dev_link_t));

	PDEBUG(DEBUG_HW, "%s: setting Vcc=33 (constant)\n", dev_info);
	link->conf.Vcc = 33;
	link->conf.IntType = INT_MEMORY_AND_IO;

	/* register with CardServices */
	link->next = dev_list;
	dev_list = link;
	client_reg.dev_info = &dev_info;
	client_reg.Attributes = INFO_IO_CLIENT;
	client_reg.EventMask = CS_EVENT_CARD_INSERTION |
		CS_EVENT_CARD_REMOVAL |
		CS_EVENT_RESET_PHYSICAL | CS_EVENT_CARD_RESET |
		CS_EVENT_PM_SUSPEND | CS_EVENT_PM_RESUME;
	client_reg.event_handler = &prism2_event;
	client_reg.Version = 0x0210;
	client_reg.event_callback_args.client_data = link;
	ret = pcmcia_register_client(&link->handle, &client_reg);
	if (ret != CS_SUCCESS) {
		cs_error(link->handle, RegisterClient, ret);
		prism2_detach(link);
		return NULL;
	}
	return link;
}


static void prism2_detach(dev_link_t *link)
{
	dev_link_t **linkp;

	PDEBUG(DEBUG_FLOW, "prism2_detach\n");

	for (linkp = &dev_list; *linkp; linkp = &(*linkp)->next)
		if (*linkp == link)
			break;
	if (*linkp == NULL) {
		printk(KERN_WARNING "%s: Attempt to detach non-existing "
		       "PCMCIA client\n", dev_info);
		return;
	}

	if (link->state & DEV_CONFIG) {
		prism2_release((u_long)link);
	}

	if (link->handle) {
		int res = pcmcia_deregister_client(link->handle);
		if (res) {
			printk("CardService(DeregisterClient) => %d\n", res);
			cs_error(link->handle, DeregisterClient, res);
		}
	}

	*linkp = link->next;
	/* release net devices */
	if (link->priv) {
		prism2_free_local_data((struct net_device *) link->priv);

	}
	kfree(link);
}


#define CS_CHECK(fn, ret) \
do { last_fn = (fn); if ((last_ret = (ret)) != 0) goto cs_failed; } while (0)

#define CFG_CHECK2(fn, retf) \
do { int ret = (retf); \
if (ret != 0) { \
	PDEBUG(DEBUG_EXTRA, "CardServices(" #fn ") returned %d\n", ret); \
	cs_error(link->handle, fn, ret); \
	goto next_entry; \
} \
} while (0)


/* run after a CARD_INSERTION event is received to configure the PCMCIA
 * socket and make the device available to the system */
static int prism2_config(dev_link_t *link)
{
	struct net_device *dev;
	struct hostap_interface *iface;
	local_info_t *local;
	int ret;
	tuple_t tuple;
	cisparse_t parse;
	int last_fn, last_ret;
	u_char buf[64];
	config_info_t conf;
	cistpl_cftable_entry_t dflt = { 0 };

	PDEBUG(DEBUG_FLOW, "prism2_config()\n");

	tuple.DesiredTuple = CISTPL_CONFIG;
	tuple.Attributes = 0;
	tuple.TupleData = buf;
	tuple.TupleDataMax = sizeof(buf);
	tuple.TupleOffset = 0;
	CS_CHECK(GetFirstTuple, pcmcia_get_first_tuple(link->handle, &tuple));
	CS_CHECK(GetTupleData, pcmcia_get_tuple_data(link->handle, &tuple));
	CS_CHECK(ParseTuple, pcmcia_parse_tuple(link->handle, &tuple, &parse));
	link->conf.ConfigBase = parse.config.base;
	link->conf.Present = parse.config.rmask[0];

	CS_CHECK(GetConfigurationInfo,
		 pcmcia_get_configuration_info(link->handle, &conf));
	PDEBUG(DEBUG_HW, "%s: %s Vcc=%d (from config)\n", dev_info,
	       ignore_cis_vcc ? "ignoring" : "setting", conf.Vcc);
	link->conf.Vcc = conf.Vcc;

	/* Look for an appropriate configuration table entry in the CIS */
	tuple.DesiredTuple = CISTPL_CFTABLE_ENTRY;
	CS_CHECK(GetFirstTuple, pcmcia_get_first_tuple(link->handle, &tuple));
	for (;;) {
		cistpl_cftable_entry_t *cfg = &(parse.cftable_entry);
		CFG_CHECK2(GetTupleData,
			   pcmcia_get_tuple_data(link->handle, &tuple));
		CFG_CHECK2(ParseTuple,
			   pcmcia_parse_tuple(link->handle, &tuple, &parse));

		if (cfg->flags & CISTPL_CFTABLE_DEFAULT)
			dflt = *cfg;
		if (cfg->index == 0)
			goto next_entry;
		link->conf.ConfigIndex = cfg->index;
		PDEBUG(DEBUG_EXTRA, "Checking CFTABLE_ENTRY 0x%02X "
		       "(default 0x%02X)\n", cfg->index, dflt.index);
	
		/* Does this card need audio output? */
		if (cfg->flags & CISTPL_CFTABLE_AUDIO) {
			link->conf.Attributes |= CONF_ENABLE_SPKR;
			link->conf.Status = CCSR_AUDIO_ENA;
		}
	
		/* Use power settings for Vcc and Vpp if present */
		/*  Note that the CIS values need to be rescaled */
		if (cfg->vcc.present & (1 << CISTPL_POWER_VNOM)) {
			if (conf.Vcc != cfg->vcc.param[CISTPL_POWER_VNOM] /
			    10000 && !ignore_cis_vcc) {
				PDEBUG(DEBUG_EXTRA, "  Vcc mismatch - skipping"
				       " this entry\n");
				goto next_entry;
			}
		} else if (dflt.vcc.present & (1 << CISTPL_POWER_VNOM)) {
			if (conf.Vcc != dflt.vcc.param[CISTPL_POWER_VNOM] /
			    10000 && !ignore_cis_vcc) {
				PDEBUG(DEBUG_EXTRA, "  Vcc (default) mismatch "
				       "- skipping this entry\n");
				goto next_entry;
			}
		}

		if (cfg->vpp1.present & (1 << CISTPL_POWER_VNOM))
			link->conf.Vpp1 = link->conf.Vpp2 =
				cfg->vpp1.param[CISTPL_POWER_VNOM] / 10000;
		else if (dflt.vpp1.present & (1 << CISTPL_POWER_VNOM))
			link->conf.Vpp1 = link->conf.Vpp2 =
				dflt.vpp1.param[CISTPL_POWER_VNOM] / 10000;

		/* Do we need to allocate an interrupt? */
		if (cfg->irq.IRQInfo1 || dflt.irq.IRQInfo1)
			link->conf.Attributes |= CONF_ENABLE_IRQ;
		else if (!(link->conf.Attributes & CONF_ENABLE_IRQ)) {
			/* At least Compaq WL200 does not have IRQInfo1 set,
			 * but it does not work without interrupts.. */
			printk("Config has no IRQ info, but trying to enable "
			       "IRQ anyway..\n");
			link->conf.Attributes |= CONF_ENABLE_IRQ;
		}

		/* IO window settings */
		PDEBUG(DEBUG_EXTRA, "IO window settings: cfg->io.nwin=%d "
		       "dflt.io.nwin=%d\n",
		       cfg->io.nwin, dflt.io.nwin);
		link->io.NumPorts1 = link->io.NumPorts2 = 0;
		if ((cfg->io.nwin > 0) || (dflt.io.nwin > 0)) {
			cistpl_io_t *io = (cfg->io.nwin) ? &cfg->io : &dflt.io;
			link->io.Attributes1 = IO_DATA_PATH_WIDTH_AUTO;
			PDEBUG(DEBUG_EXTRA, "io->flags = 0x%04X, "
			       "io.base=0x%04x, len=%d\n", io->flags,
			       io->win[0].base, io->win[0].len);
			if (!(io->flags & CISTPL_IO_8BIT))
				link->io.Attributes1 = IO_DATA_PATH_WIDTH_16;
			if (!(io->flags & CISTPL_IO_16BIT))
				link->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
			link->io.IOAddrLines = io->flags &
				CISTPL_IO_LINES_MASK;
			link->io.BasePort1 = io->win[0].base;
			link->io.NumPorts1 = io->win[0].len;
			if (io->nwin > 1) {
				link->io.Attributes2 = link->io.Attributes1;
				link->io.BasePort2 = io->win[1].base;
				link->io.NumPorts2 = io->win[1].len;
			}
		}

		/* This reserves IO space but doesn't actually enable it */
		CFG_CHECK2(RequestIO,
			   pcmcia_request_io(link->handle, &link->io));

		/* This configuration table entry is OK */
		break;

	next_entry:
		CS_CHECK(GetNextTuple,
			 pcmcia_get_next_tuple(link->handle, &tuple));
	}

	/* Need to allocate net_device before requesting IRQ handler */
	dev = prism2_init_local_data(&prism2_pccard_funcs, 0);
	if (dev == NULL)
		goto failed;
	link->priv = dev;

	/*
	 * Allocate an interrupt line.  Note that this does not assign a
	 * handler to the interrupt, unless the 'Handler' member of the
	 * irq structure is initialized.
	 */
	if (link->conf.Attributes & CONF_ENABLE_IRQ) {
		int i;
		link->irq.Attributes = IRQ_TYPE_EXCLUSIVE | IRQ_HANDLE_PRESENT;
		link->irq.IRQInfo1 = IRQ_INFO2_VALID | IRQ_LEVEL_ID;
		if (irq_list[0] == -1)
			link->irq.IRQInfo2 = irq_mask;
		else
			for (i = 0; i < 4; i++)
				link->irq.IRQInfo2 |= 1 << irq_list[i];
		link->irq.Handler = prism2_interrupt;
		link->irq.Instance = dev;
		CS_CHECK(RequestIRQ,
			 pcmcia_request_irq(link->handle, &link->irq));
	}

	/*
	 * This actually configures the PCMCIA socket -- setting up
	 * the I/O windows and the interrupt mapping, and putting the
	 * card and host interface into "Memory and IO" mode.
	 */
	CS_CHECK(RequestConfiguration,
		 pcmcia_request_configuration(link->handle, &link->conf));

	dev->irq = link->irq.AssignedIRQ;
	dev->base_addr = link->io.BasePort1;

	/* Finally, report what we've done */
	printk(KERN_INFO "%s: index 0x%02x: Vcc %d.%d",
	       dev_info, link->conf.ConfigIndex,
	       link->conf.Vcc / 10, link->conf.Vcc % 10);
	if (link->conf.Vpp1)
		printk(", Vpp %d.%d", link->conf.Vpp1 / 10,
		       link->conf.Vpp1 % 10);
	if (link->conf.Attributes & CONF_ENABLE_IRQ)
		printk(", irq %d", link->irq.AssignedIRQ);
	if (link->io.NumPorts1)
		printk(", io 0x%04x-0x%04x", link->io.BasePort1,
		       link->io.BasePort1+link->io.NumPorts1-1);
	if (link->io.NumPorts2)
		printk(" & 0x%04x-0x%04x", link->io.BasePort2,
		       link->io.BasePort2+link->io.NumPorts2-1);
	printk("\n");

	link->state |= DEV_CONFIG;
	link->state &= ~DEV_CONFIG_PENDING;

	iface = dev->priv;
	local = iface->local;
	local->link = link;
	strcpy(local->node.dev_name, dev->name);
	link->dev = &local->node;

	local->shutdown = 0;

	ret = prism2_hw_config(dev, 1);
	if (!ret) {
		ret = hostap_hw_ready(dev);
		if (ret == 0 && local->ddev)
			strcpy(local->node.dev_name, local->ddev->name);
	}
	return ret;

 cs_failed:
	cs_error(link->handle, last_fn, last_ret);

 failed:
	prism2_release((u_long)link);
	return 1;
}


static void prism2_release(u_long arg)
{
	dev_link_t *link = (dev_link_t *)arg;

	PDEBUG(DEBUG_FLOW, "prism2_release\n");

	if (link->priv) {
		struct net_device *dev = link->priv;
		struct hostap_interface *iface = dev->priv;
		if (link->state & DEV_CONFIG)
			prism2_hw_shutdown(dev, 0);
		iface->local->shutdown = 1;
	}

	if (link->win)
		pcmcia_release_window(link->win);
	pcmcia_release_configuration(link->handle);
	if (link->io.NumPorts1)
		pcmcia_release_io(link->handle, &link->io);
	if (link->irq.AssignedIRQ)
		pcmcia_release_irq(link->handle, &link->irq);

	link->state &= ~DEV_CONFIG;

	PDEBUG(DEBUG_FLOW, "release - done\n");
}


static int prism2_event(event_t event, int priority,
			event_callback_args_t *args)
{
	dev_link_t *link = args->client_data;
	struct net_device *dev = (struct net_device *) link->priv;

	switch (event) {
	case CS_EVENT_CARD_INSERTION:
		PDEBUG(DEBUG_EXTRA, "%s: CS_EVENT_CARD_INSERTION\n", dev_info);
		link->state |= DEV_PRESENT | DEV_CONFIG_PENDING;
		if (prism2_config(link)) {
			PDEBUG(DEBUG_EXTRA, "prism2_config() failed\n");
		}
		break;

	case CS_EVENT_CARD_REMOVAL:
		PDEBUG(DEBUG_EXTRA, "%s: CS_EVENT_CARD_REMOVAL\n", dev_info);
		link->state &= ~DEV_PRESENT;
		if (link->state & DEV_CONFIG) {
			netif_stop_queue(dev);
			netif_device_detach(dev);
			prism2_release((u_long) link);
		}
		break;

	case CS_EVENT_PM_SUSPEND:
		PDEBUG(DEBUG_EXTRA, "%s: CS_EVENT_PM_SUSPEND\n", dev_info);
		link->state |= DEV_SUSPEND;
		/* fall through */

	case CS_EVENT_RESET_PHYSICAL:
		PDEBUG(DEBUG_EXTRA, "%s: CS_EVENT_RESET_PHYSICAL\n", dev_info);
		if (link->state & DEV_CONFIG) {
			if (link->open) {
				netif_stop_queue(dev);
				netif_device_detach(dev);
			}
			pcmcia_release_configuration(link->handle);
		}
		break;

	case CS_EVENT_PM_RESUME:
		PDEBUG(DEBUG_EXTRA, "%s: CS_EVENT_PM_RESUME\n", dev_info);
		link->state &= ~DEV_SUSPEND;
		/* fall through */

	case CS_EVENT_CARD_RESET:
		PDEBUG(DEBUG_EXTRA, "%s: CS_EVENT_CARD_RESET\n", dev_info);
		if (link->state & DEV_CONFIG) {
			pcmcia_request_configuration(link->handle,
						     &link->conf);
			prism2_hw_shutdown(dev, 1);
			prism2_hw_config(dev, link->open ? 0 : 1);
			if (link->open) {
				netif_device_attach(dev);
				netif_start_queue(dev);
			}
		}
		break;

	default:
		PDEBUG(DEBUG_EXTRA, "%s: prism2_event() - unknown event %d\n",
		       dev_info, event);
		break;
	}
	return 0;
}


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,67)
static struct pcmcia_driver hostap_driver = {
	.drv		= {
		.name	= "hostap_cs",
	},
	.attach		= prism2_attach,
	.detach		= prism2_detach,
	.owner		= THIS_MODULE,
};

static int __init init_prism2_pccard(void)
{
	printk(KERN_INFO "%s: %s\n", dev_info, version);
	return pcmcia_register_driver(&hostap_driver);
}

static void __exit exit_prism2_pccard(void)
{
	pcmcia_unregister_driver(&hostap_driver);
	printk(KERN_INFO "%s: Driver unloaded\n", dev_info);
}

#else

static int __init init_prism2_pccard(void)
{
	servinfo_t serv;

	printk(KERN_INFO "%s: %s\n", dev_info, version);
	pcmcia_get_card_services_info(&serv);
	if (serv.Revision != CS_RELEASE_CODE) {
		printk(KERN_NOTICE
		       "%s: CardServices release does not match!\n", dev_info);
		return -1;
	}
	register_pccard_driver(&dev_info, &prism2_attach, &prism2_detach);

	return 0;
}


static void __exit exit_prism2_pccard(void)
{
	unregister_pccard_driver(&dev_info);
	while (dev_list) {
		PDEBUG(DEBUG_FLOW, "exit_prism2 - detaching device\n");
		if (dev_list->state & DEV_CONFIG)
			prism2_release((u_long)dev_list);
		prism2_detach(dev_list);
	}

	printk(KERN_INFO "%s: Driver unloaded\n", dev_info);
}
#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2,5,67) */


module_init(init_prism2_pccard);
module_exit(exit_prism2_pccard);
