/* orinoco_cs.c (formerly known as dldwd_cs.c)
 *
 * A driver for "Hermes" chipset based PCMCIA wireless adaptors, such
 * as the Lucent WavelanIEEE/Orinoco cards and their OEM (Cabletron/
 * EnteraSys RoamAbout 802.11, ELSA Airlancer, Melco Buffalo and others).
 * It should also be usable on various Prism II based cards such as the
 * Linksys, D-Link and Farallon Skyline. It should also work on Symbol
 * cards such as the 3Com AirConnect and Ericsson WLAN.
 * 
 * Copyright notice & release notes in file orinoco.c
 */

#define DRIVER_NAME "orinoco_cs"
#define PFX DRIVER_NAME ": "

#include <linux/config.h>
#ifdef  __IN_PCMCIA_PACKAGE__
#include <pcmcia/k_compat.h>
#endif /* __IN_PCMCIA_PACKAGE__ */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/wireless.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/system.h>

#include "orinoco.h"

/********************************************************************/
/* Module stuff							    */
/********************************************************************/

MODULE_AUTHOR("David Gibson <hermes@gibson.dropbear.id.au>");
MODULE_DESCRIPTION("Driver for PCMCIA Lucent Orinoco, Prism II based and similar wireless cards");
MODULE_LICENSE("Dual MPL/GPL");

/* Module parameters */

/* Some D-Link cards have buggy CIS. They do work at 5v properly, but
 * don't have any CIS entry for it. This workaround it... */
static int ignore_cis_vcc; /* = 0 */
module_param(ignore_cis_vcc, int, 0);
MODULE_PARM_DESC(ignore_cis_vcc, "Allow voltage mismatch between card and socket");

/********************************************************************/
/* Magic constants						    */
/********************************************************************/

/*
 * The dev_info variable is the "key" that is used to match up this
 * device driver with appropriate cards, through the card
 * configuration database.
 */
static dev_info_t dev_info = DRIVER_NAME;

/********************************************************************/
/* Data structures						    */
/********************************************************************/

/* PCMCIA specific device information (goes in the card field of
 * struct orinoco_private */
struct orinoco_pccard {
	dev_link_t link;
	dev_node_t node;

	/* Used to handle hard reset */
	/* yuck, we need this hack to work around the insanity of the
         * PCMCIA layer */
	unsigned long hard_reset_in_progress; 
};

/*
 * A linked list of "instances" of the device.  Each actual PCMCIA
 * card corresponds to one device instance, and is described by one
 * dev_link_t structure (defined in ds.h).
 */
static dev_link_t *dev_list; /* = NULL */

/********************************************************************/
/* Function prototypes						    */
/********************************************************************/

/* device methods */
static int orinoco_cs_hard_reset(struct orinoco_private *priv);

/* PCMCIA gumpf */
static void orinoco_cs_config(dev_link_t * link);
static void orinoco_cs_release(dev_link_t * link);
static int orinoco_cs_event(event_t event, int priority,
			    event_callback_args_t * args);

static dev_link_t *orinoco_cs_attach(void);
static void orinoco_cs_detach(dev_link_t *);

/********************************************************************/
/* Device methods     						    */
/********************************************************************/

static int
orinoco_cs_hard_reset(struct orinoco_private *priv)
{
	struct orinoco_pccard *card = priv->card;
	dev_link_t *link = &card->link;
	int err;

	/* We need atomic ops here, because we're not holding the lock */
	set_bit(0, &card->hard_reset_in_progress);

	err = pcmcia_reset_card(link->handle, NULL);
	if (err)
		return err;

	msleep(100);
	clear_bit(0, &card->hard_reset_in_progress);

	return 0;
}

/********************************************************************/
/* PCMCIA stuff     						    */
/********************************************************************/

/*
 * This creates an "instance" of the driver, allocating local data
 * structures for one device.  The device is registered with Card
 * Services.
 * 
 * The dev_link structure is initialized, but we don't actually
 * configure the card at this point -- we wait until we receive a card
 * insertion event.  */
static dev_link_t *
orinoco_cs_attach(void)
{
	struct net_device *dev;
	struct orinoco_private *priv;
	struct orinoco_pccard *card;
	dev_link_t *link;
	client_reg_t client_reg;
	int ret;

	dev = alloc_orinocodev(sizeof(*card), orinoco_cs_hard_reset);
	if (! dev)
		return NULL;
	priv = netdev_priv(dev);
	card = priv->card;

	/* Link both structures together */
	link = &card->link;
	link->priv = dev;

	/* Interrupt setup */
	link->irq.Attributes = IRQ_TYPE_EXCLUSIVE | IRQ_HANDLE_PRESENT;
	link->irq.IRQInfo1 = IRQ_LEVEL_ID;
	link->irq.Handler = orinoco_interrupt;
	link->irq.Instance = dev; 

	/* General socket configuration defaults can go here.  In this
	 * client, we assume very little, and rely on the CIS for
	 * almost everything.  In most clients, many details (i.e.,
	 * number, sizes, and attributes of IO windows) are fixed by
	 * the nature of the device, and can be hard-wired here. */
	link->conf.Attributes = 0;
	link->conf.IntType = INT_MEMORY_AND_IO;

	/* Register with Card Services */
	/* FIXME: need a lock? */
	link->next = dev_list;
	dev_list = link;

	client_reg.dev_info = &dev_info;
	client_reg.Version = 0x0210; /* FIXME: what does this mean? */
	client_reg.event_callback_args.client_data = link;

	ret = pcmcia_register_client(&link->handle, &client_reg);
	if (ret != CS_SUCCESS) {
		cs_error(link->handle, RegisterClient, ret);
		orinoco_cs_detach(link);
		return NULL;
	}

	return link;
}				/* orinoco_cs_attach */

/*
 * This deletes a driver "instance".  The device is de-registered with
 * Card Services.  If it has been released, all local data structures
 * are freed.  Otherwise, the structures will be freed when the device
 * is released.
 */
static void orinoco_cs_detach(dev_link_t *link)
{
	dev_link_t **linkp;
	struct net_device *dev = link->priv;

	/* Locate device structure */
	for (linkp = &dev_list; *linkp; linkp = &(*linkp)->next)
		if (*linkp == link)
			break;

	BUG_ON(*linkp == NULL);

	if (link->state & DEV_CONFIG)
		orinoco_cs_release(link);

	/* Break the link with Card Services */
	if (link->handle)
		pcmcia_deregister_client(link->handle);

	/* Unlink device structure, and free it */
	*linkp = link->next;
	DEBUG(0, PFX "detach: link=%p link->dev=%p\n", link, link->dev);
	if (link->dev) {
		DEBUG(0, PFX "About to unregister net device %p\n",
		      dev);
		unregister_netdev(dev);
	}
	free_orinocodev(dev);
}				/* orinoco_cs_detach */

/*
 * orinoco_cs_config() is scheduled to run after a CARD_INSERTION
 * event is received, to configure the PCMCIA socket, and to make the
 * device available to the system.
 */

#define CS_CHECK(fn, ret) do { \
		last_fn = (fn); if ((last_ret = (ret)) != 0) goto cs_failed; \
	} while (0)

static void
orinoco_cs_config(dev_link_t *link)
{
	struct net_device *dev = link->priv;
	client_handle_t handle = link->handle;
	struct orinoco_private *priv = netdev_priv(dev);
	struct orinoco_pccard *card = priv->card;
	hermes_t *hw = &priv->hw;
	int last_fn, last_ret;
	u_char buf[64];
	config_info_t conf;
	cisinfo_t info;
	tuple_t tuple;
	cisparse_t parse;
	void __iomem *mem;

	CS_CHECK(ValidateCIS, pcmcia_validate_cis(handle, &info));

	/*
	 * This reads the card's CONFIG tuple to find its
	 * configuration registers.
	 */
	tuple.DesiredTuple = CISTPL_CONFIG;
	tuple.Attributes = 0;
	tuple.TupleData = buf;
	tuple.TupleDataMax = sizeof(buf);
	tuple.TupleOffset = 0;
	CS_CHECK(GetFirstTuple, pcmcia_get_first_tuple(handle, &tuple));
	CS_CHECK(GetTupleData, pcmcia_get_tuple_data(handle, &tuple));
	CS_CHECK(ParseTuple, pcmcia_parse_tuple(handle, &tuple, &parse));
	link->conf.ConfigBase = parse.config.base;
	link->conf.Present = parse.config.rmask[0];

	/* Configure card */
	link->state |= DEV_CONFIG;

	/* Look up the current Vcc */
	CS_CHECK(GetConfigurationInfo,
		 pcmcia_get_configuration_info(handle, &conf));
	link->conf.Vcc = conf.Vcc;

	/*
	 * In this loop, we scan the CIS for configuration table
	 * entries, each of which describes a valid card
	 * configuration, including voltage, IO window, memory window,
	 * and interrupt settings.
	 *
	 * We make no assumptions about the card to be configured: we
	 * use just the information available in the CIS.  In an ideal
	 * world, this would work for any PCMCIA card, but it requires
	 * a complete and accurate CIS.  In practice, a driver usually
	 * "knows" most of these things without consulting the CIS,
	 * and most client drivers will only use the CIS to fill in
	 * implementation-defined details.
	 */
	tuple.DesiredTuple = CISTPL_CFTABLE_ENTRY;
	CS_CHECK(GetFirstTuple, pcmcia_get_first_tuple(handle, &tuple));
	while (1) {
		cistpl_cftable_entry_t *cfg = &(parse.cftable_entry);
		cistpl_cftable_entry_t dflt = { .index = 0 };

		if ( (pcmcia_get_tuple_data(handle, &tuple) != 0)
		    || (pcmcia_parse_tuple(handle, &tuple, &parse) != 0))
			goto next_entry;

		if (cfg->flags & CISTPL_CFTABLE_DEFAULT)
			dflt = *cfg;
		if (cfg->index == 0)
			goto next_entry;
		link->conf.ConfigIndex = cfg->index;

		/* Does this card need audio output? */
		if (cfg->flags & CISTPL_CFTABLE_AUDIO) {
			link->conf.Attributes |= CONF_ENABLE_SPKR;
			link->conf.Status = CCSR_AUDIO_ENA;
		}

		/* Use power settings for Vcc and Vpp if present */
		/* Note that the CIS values need to be rescaled */
		if (cfg->vcc.present & (1 << CISTPL_POWER_VNOM)) {
			if (conf.Vcc != cfg->vcc.param[CISTPL_POWER_VNOM] / 10000) {
				DEBUG(2, "orinoco_cs_config: Vcc mismatch (conf.Vcc = %d, CIS = %d)\n",  conf.Vcc, cfg->vcc.param[CISTPL_POWER_VNOM] / 10000);
				if (!ignore_cis_vcc)
					goto next_entry;
			}
		} else if (dflt.vcc.present & (1 << CISTPL_POWER_VNOM)) {
			if (conf.Vcc != dflt.vcc.param[CISTPL_POWER_VNOM] / 10000) {
				DEBUG(2, "orinoco_cs_config: Vcc mismatch (conf.Vcc = %d, CIS = %d)\n",  conf.Vcc, dflt.vcc.param[CISTPL_POWER_VNOM] / 10000);
				if(!ignore_cis_vcc)
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
		link->conf.Attributes |= CONF_ENABLE_IRQ;

		/* IO window settings */
		link->io.NumPorts1 = link->io.NumPorts2 = 0;
		if ((cfg->io.nwin > 0) || (dflt.io.nwin > 0)) {
			cistpl_io_t *io =
			    (cfg->io.nwin) ? &cfg->io : &dflt.io;
			link->io.Attributes1 = IO_DATA_PATH_WIDTH_AUTO;
			if (!(io->flags & CISTPL_IO_8BIT))
				link->io.Attributes1 =
				    IO_DATA_PATH_WIDTH_16;
			if (!(io->flags & CISTPL_IO_16BIT))
				link->io.Attributes1 =
				    IO_DATA_PATH_WIDTH_8;
			link->io.IOAddrLines =
			    io->flags & CISTPL_IO_LINES_MASK;
			link->io.BasePort1 = io->win[0].base;
			link->io.NumPorts1 = io->win[0].len;
			if (io->nwin > 1) {
				link->io.Attributes2 =
				    link->io.Attributes1;
				link->io.BasePort2 = io->win[1].base;
				link->io.NumPorts2 = io->win[1].len;
			}

			/* This reserves IO space but doesn't actually enable it */
			if (pcmcia_request_io(link->handle, &link->io) != 0)
				goto next_entry;
		}


		/* If we got this far, we're cool! */

		break;
		
	next_entry:
		if (link->io.NumPorts1)
			pcmcia_release_io(link->handle, &link->io);
		last_ret = pcmcia_get_next_tuple(handle, &tuple);
		if (last_ret  == CS_NO_MORE_ITEMS) {
			printk(KERN_ERR PFX "GetNextTuple(): No matching "
			       "CIS configuration.  Maybe you need the "
			       "ignore_cis_vcc=1 parameter.\n");
			goto cs_failed;
		}
	}

	/*
	 * Allocate an interrupt line.  Note that this does not assign
	 * a handler to the interrupt, unless the 'Handler' member of
	 * the irq structure is initialized.
	 */
	CS_CHECK(RequestIRQ, pcmcia_request_irq(link->handle, &link->irq));

	/* We initialize the hermes structure before completing PCMCIA
	 * configuration just in case the interrupt handler gets
	 * called. */
	mem = ioport_map(link->io.BasePort1, link->io.NumPorts1);
	if (!mem)
		goto cs_failed;

	hermes_struct_init(hw, mem, HERMES_16BIT_REGSPACING);

	/*
	 * This actually configures the PCMCIA socket -- setting up
	 * the I/O windows and the interrupt mapping, and putting the
	 * card and host interface into "Memory and IO" mode.
	 */
	CS_CHECK(RequestConfiguration,
		 pcmcia_request_configuration(link->handle, &link->conf));

	/* Ok, we have the configuration, prepare to register the netdev */
	dev->base_addr = link->io.BasePort1;
	dev->irq = link->irq.AssignedIRQ;
	SET_MODULE_OWNER(dev);
	card->node.major = card->node.minor = 0;

	SET_NETDEV_DEV(dev, &handle_to_dev(handle));
	/* Tell the stack we exist */
	if (register_netdev(dev) != 0) {
		printk(KERN_ERR PFX "register_netdev() failed\n");
		goto failed;
	}

	/* At this point, the dev_node_t structure(s) needs to be
	 * initialized and arranged in a linked list at link->dev. */
	strcpy(card->node.dev_name, dev->name);
	link->dev = &card->node; /* link->dev being non-NULL is also
                                    used to indicate that the
                                    net_device has been registered */
	link->state &= ~DEV_CONFIG_PENDING;

	/* Finally, report what we've done */
	printk(KERN_DEBUG "%s: index 0x%02x: Vcc %d.%d",
	       dev->name, link->conf.ConfigIndex,
	       link->conf.Vcc / 10, link->conf.Vcc % 10);
	if (link->conf.Vpp1)
		printk(", Vpp %d.%d", link->conf.Vpp1 / 10,
		       link->conf.Vpp1 % 10);
	printk(", irq %d", link->irq.AssignedIRQ);
	if (link->io.NumPorts1)
		printk(", io 0x%04x-0x%04x", link->io.BasePort1,
		       link->io.BasePort1 + link->io.NumPorts1 - 1);
	if (link->io.NumPorts2)
		printk(" & 0x%04x-0x%04x", link->io.BasePort2,
		       link->io.BasePort2 + link->io.NumPorts2 - 1);
	printk("\n");

	return;

 cs_failed:
	cs_error(link->handle, last_fn, last_ret);

 failed:
	orinoco_cs_release(link);
}				/* orinoco_cs_config */

/*
 * After a card is removed, orinoco_cs_release() will unregister the
 * device, and release the PCMCIA configuration.  If the device is
 * still open, this will be postponed until it is closed.
 */
static void
orinoco_cs_release(dev_link_t *link)
{
	struct net_device *dev = link->priv;
	struct orinoco_private *priv = netdev_priv(dev);
	unsigned long flags;

	/* We're committed to taking the device away now, so mark the
	 * hardware as unavailable */
	spin_lock_irqsave(&priv->lock, flags);
	priv->hw_unavailable++;
	spin_unlock_irqrestore(&priv->lock, flags);

	/* Don't bother checking to see if these succeed or not */
	pcmcia_release_configuration(link->handle);
	if (link->io.NumPorts1)
		pcmcia_release_io(link->handle, &link->io);
	if (link->irq.AssignedIRQ)
		pcmcia_release_irq(link->handle, &link->irq);
	link->state &= ~DEV_CONFIG;
	if (priv->hw.iobase)
		ioport_unmap(priv->hw.iobase);
}				/* orinoco_cs_release */

/*
 * The card status event handler.  Mostly, this schedules other stuff
 * to run after an event is received.
 */
static int
orinoco_cs_event(event_t event, int priority,
		       event_callback_args_t * args)
{
	dev_link_t *link = args->client_data;
	struct net_device *dev = link->priv;
	struct orinoco_private *priv = netdev_priv(dev);
	struct orinoco_pccard *card = priv->card;
	int err = 0;
	unsigned long flags;

	switch (event) {
	case CS_EVENT_CARD_REMOVAL:
		link->state &= ~DEV_PRESENT;
		if (link->state & DEV_CONFIG) {
			unsigned long flags;

			spin_lock_irqsave(&priv->lock, flags);
			netif_device_detach(dev);
			priv->hw_unavailable++;
			spin_unlock_irqrestore(&priv->lock, flags);
		}
		break;

	case CS_EVENT_CARD_INSERTION:
		link->state |= DEV_PRESENT | DEV_CONFIG_PENDING;
		orinoco_cs_config(link);
		break;

	case CS_EVENT_PM_SUSPEND:
		link->state |= DEV_SUSPEND;
		/* Fall through... */
	case CS_EVENT_RESET_PHYSICAL:
		/* Mark the device as stopped, to block IO until later */
		if (link->state & DEV_CONFIG) {
			/* This is probably racy, but I can't think of
                           a better way, short of rewriting the PCMCIA
                           layer to not suck :-( */
			if (! test_bit(0, &card->hard_reset_in_progress)) {
				spin_lock_irqsave(&priv->lock, flags);

				err = __orinoco_down(dev);
				if (err)
					printk(KERN_WARNING "%s: %s: Error %d downing interface\n",
					       dev->name,
					       event == CS_EVENT_PM_SUSPEND ? "SUSPEND" : "RESET_PHYSICAL",
					       err);
				
				netif_device_detach(dev);
				priv->hw_unavailable++;

				spin_unlock_irqrestore(&priv->lock, flags);
			}

			pcmcia_release_configuration(link->handle);
		}
		break;

	case CS_EVENT_PM_RESUME:
		link->state &= ~DEV_SUSPEND;
		/* Fall through... */
	case CS_EVENT_CARD_RESET:
		if (link->state & DEV_CONFIG) {
			/* FIXME: should we double check that this is
			 * the same card as we had before */
			pcmcia_request_configuration(link->handle, &link->conf);

			if (! test_bit(0, &card->hard_reset_in_progress)) {
				err = orinoco_reinit_firmware(dev);
				if (err) {
					printk(KERN_ERR "%s: Error %d re-initializing firmware\n",
					       dev->name, err);
					break;
				}
				
				spin_lock_irqsave(&priv->lock, flags);
				
				netif_device_attach(dev);
				priv->hw_unavailable--;
				
				if (priv->open && ! priv->hw_unavailable) {
					err = __orinoco_up(dev);
					if (err)
						printk(KERN_ERR "%s: Error %d restarting card\n",
						       dev->name, err);
					
				}

				spin_unlock_irqrestore(&priv->lock, flags);
			}
		}
		break;
	}

	return err;
}				/* orinoco_cs_event */

/********************************************************************/
/* Module initialization					    */
/********************************************************************/

/* Can't be declared "const" or the whole __initdata section will
 * become const */
static char version[] __initdata = DRIVER_NAME " " DRIVER_VERSION
	" (David Gibson <hermes@gibson.dropbear.id.au>, "
	"Pavel Roskin <proski@gnu.org>, et al)";

static struct pcmcia_device_id orinoco_cs_ids[] = {
	PCMCIA_DEVICE_MANF_CARD(0x000b, 0x7300),
	PCMCIA_DEVICE_MANF_CARD(0x0138, 0x0002),
	PCMCIA_DEVICE_MANF_CARD(0x0156, 0x0002),
	PCMCIA_DEVICE_MANF_CARD(0x01eb, 0x080a),
	PCMCIA_DEVICE_MANF_CARD(0x0261, 0x0002),
	PCMCIA_DEVICE_MANF_CARD(0x0268, 0x0001),
	PCMCIA_DEVICE_MANF_CARD(0x026f, 0x0305),
	PCMCIA_DEVICE_MANF_CARD(0x0274, 0x1613),
	PCMCIA_DEVICE_MANF_CARD(0x028a, 0x0002),
	PCMCIA_DEVICE_MANF_CARD(0x028a, 0x0673),
	PCMCIA_DEVICE_MANF_CARD(0x02aa, 0x0002),
	PCMCIA_DEVICE_MANF_CARD(0x02ac, 0x0002),
	PCMCIA_DEVICE_MANF_CARD(0x14ea, 0xb001),
	PCMCIA_DEVICE_MANF_CARD(0x50c2, 0x7300),
	PCMCIA_DEVICE_MANF_CARD(0x9005, 0x0021),
	PCMCIA_DEVICE_MANF_CARD(0xc250, 0x0002),
	PCMCIA_DEVICE_MANF_CARD(0xd601, 0x0002),
	PCMCIA_DEVICE_MANF_CARD(0xd601, 0x0005),
	PCMCIA_DEVICE_PROD_ID12("3Com", "3CRWE737A AirConnect Wireless LAN PC Card", 0x41240e5b, 0x56010af3),
	PCMCIA_DEVICE_PROD_ID123("Instant Wireless ", " Network PC CARD", "Version 01.02", 0x11d901af, 0x6e9bd926, 0x4b74baa0),
	PCMCIA_DEVICE_PROD_ID12("ACTIONTEC", "PRISM Wireless LAN PC Card", 0x393089da, 0xa71e69d5),
	PCMCIA_DEVICE_PROD_ID12("Avaya Communication", "Avaya Wireless PC Card", 0xd8a43b78, 0x0d341169),
	PCMCIA_DEVICE_PROD_ID12("BUFFALO", "WLI-PCM-L11G", 0x2decece3, 0xf57ca4b3),
	PCMCIA_DEVICE_PROD_ID12("Cabletron", "RoamAbout 802.11 DS", 0x32d445f5, 0xedeffd90),
	PCMCIA_DEVICE_PROD_ID12("corega K.K.", "Wireless LAN PCC-11", 0x5261440f, 0xa6405584),
	PCMCIA_DEVICE_PROD_ID12("corega K.K.", "Wireless LAN PCCA-11", 0x5261440f, 0xdf6115f9),
	PCMCIA_DEVICE_PROD_ID12("corega_K.K.", "Wireless_LAN_PCCB-11", 0x29e33311, 0xee7a27ae),
	PCMCIA_DEVICE_PROD_ID12("D", "Link DRC-650 11Mbps WLAN Card", 0x71b18589, 0xf144e3ac),
	PCMCIA_DEVICE_PROD_ID12("D", "Link DWL-650 11Mbps WLAN Card", 0x71b18589, 0xb6f1b0ab),
	PCMCIA_DEVICE_PROD_ID12("ELSA", "AirLancer MC-11", 0x4507a33a, 0xef54f0e3),
	PCMCIA_DEVICE_PROD_ID12("HyperLink", "Wireless PC Card 11Mbps", 0x56cc3f1a, 0x0bcf220c),
	PCMCIA_DEVICE_PROD_ID12("INTERSIL", "HFA384x/IEEE", 0x74c5e40d, 0xdb472a18),
	PCMCIA_DEVICE_PROD_ID12("Lucent Technologies", "WaveLAN/IEEE", 0x23eb9949, 0xc562e72a),
	PCMCIA_DEVICE_PROD_ID12("MELCO", "WLI-PCM-L11", 0x481e0094, 0x7360e410),
	PCMCIA_DEVICE_PROD_ID12("MELCO", "WLI-PCM-L11G", 0x481e0094, 0xf57ca4b3),
	PCMCIA_DEVICE_PROD_ID12("Microsoft", "Wireless Notebook Adapter MN-520", 0x5961bf85, 0x6eec8c01),
	PCMCIA_DEVICE_PROD_ID12("NCR", "WaveLAN/IEEE", 0x24358cd4, 0xc562e72a),
	PCMCIA_DEVICE_PROD_ID12("NETGEAR MA401RA Wireless PC", "Card", 0x0306467f, 0x9762e8f1),
	PCMCIA_DEVICE_PROD_ID12("PLANEX", "GeoWave/GW-CF110", 0x209f40ab, 0xd9715264),
	PCMCIA_DEVICE_PROD_ID12("PROXIM", "LAN PC CARD HARMONY 80211B", 0xc6536a5e, 0x090c3cd9),
	PCMCIA_DEVICE_PROD_ID12("PROXIM", "LAN PCI CARD HARMONY 80211B", 0xc6536a5e, 0x9f494e26),
	PCMCIA_DEVICE_PROD_ID12("SAMSUNG", "11Mbps WLAN Card", 0x43d74cb4, 0x579bd91b),
	PCMCIA_DEVICE_PROD_ID1("Symbol Technologies", 0x3f02b4d6),
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, orinoco_cs_ids);

static struct pcmcia_driver orinoco_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= DRIVER_NAME,
	},
	.attach		= orinoco_cs_attach,
	.event		= orinoco_cs_event,
	.detach		= orinoco_cs_detach,
	.id_table       = orinoco_cs_ids,
};

static int __init
init_orinoco_cs(void)
{
	printk(KERN_DEBUG "%s\n", version);

	return pcmcia_register_driver(&orinoco_driver);
}

static void __exit
exit_orinoco_cs(void)
{
	pcmcia_unregister_driver(&orinoco_driver);
	BUG_ON(dev_list != NULL);
}

module_init(init_orinoco_cs);
module_exit(exit_orinoco_cs);
