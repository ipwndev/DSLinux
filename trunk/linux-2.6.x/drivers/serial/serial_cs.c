/*======================================================================

    A driver for PCMCIA serial devices

    serial_cs.c 1.134 2002/05/04 05:48:53

    The contents of this file are subject to the Mozilla Public
    License Version 1.1 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS
    IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
    implied. See the License for the specific language governing
    rights and limitations under the License.

    The initial developer of the original code is David A. Hinds
    <dahinds@users.sourceforge.net>.  Portions created by David A. Hinds
    are Copyright (C) 1999 David A. Hinds.  All Rights Reserved.

    Alternatively, the contents of this file may be used under the
    terms of the GNU General Public License version 2 (the "GPL"), in which
    case the provisions of the GPL are applicable instead of the
    above.  If you wish to allow the use of your version of this file
    only under the terms of the GPL and not to allow others to use
    your version of this file under the MPL, indicate your decision
    by deleting the provisions above and replace them with the notice
    and other provisions required by the GPL.  If you do not delete
    the provisions above, a recipient may use your version of this
    file under either the MPL or the GPL.
    
======================================================================*/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/serial_core.h>
#include <linux/major.h>
#include <asm/io.h>
#include <asm/system.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/ciscode.h>
#include <pcmcia/ds.h>
#include <pcmcia/cisreg.h>

#include "8250.h"

#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
module_param(pc_debug, int, 0644);
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KERN_DEBUG args)
static char *version = "serial_cs.c 1.134 2002/05/04 05:48:53 (David Hinds)";
#else
#define DEBUG(n, args...)
#endif

/*====================================================================*/

/* Parameters that can be set with 'insmod' */

/* Enable the speaker? */
static int do_sound = 1;
/* Skip strict UART tests? */
static int buggy_uart;

module_param(do_sound, int, 0444);
module_param(buggy_uart, int, 0444);

/*====================================================================*/

/* Table of multi-port card ID's */

struct multi_id {
	u_short manfid;
	u_short prodid;
	int multi;		/* 1 = multifunction, > 1 = # ports */
};

static struct multi_id multi_id[] = {
	{ MANFID_OMEGA,   PRODID_OMEGA_QSP_100,         4 },
	{ MANFID_QUATECH, PRODID_QUATECH_DUAL_RS232,    2 },
	{ MANFID_QUATECH, PRODID_QUATECH_DUAL_RS232_D1, 2 },
	{ MANFID_QUATECH, PRODID_QUATECH_QUAD_RS232,    4 },
	{ MANFID_SOCKET,  PRODID_SOCKET_DUAL_RS232,     2 },
	{ MANFID_INTEL,   PRODID_INTEL_DUAL_RS232,      2 },
	{ MANFID_NATINST, PRODID_NATINST_QUAD_RS232,    4 }
};
#define MULTI_COUNT (sizeof(multi_id)/sizeof(struct multi_id))

struct serial_info {
	dev_link_t		link;
	int			ndev;
	int			multi;
	int			slave;
	int			manfid;
	dev_node_t		node[4];
	int			line[4];
};

struct serial_cfg_mem {
	tuple_t tuple;
	cisparse_t parse;
	u_char buf[256];
};


static void serial_config(dev_link_t * link);
static int serial_event(event_t event, int priority,
			event_callback_args_t * args);

static dev_info_t dev_info = "serial_cs";

static dev_link_t *serial_attach(void);
static void serial_detach(dev_link_t *);

static dev_link_t *dev_list = NULL;

/*======================================================================

    After a card is removed, serial_remove() will unregister
    the serial device(s), and release the PCMCIA configuration.
    
======================================================================*/

static void serial_remove(dev_link_t *link)
{
	struct serial_info *info = link->priv;
	int i;

	link->state &= ~DEV_PRESENT;

	DEBUG(0, "serial_release(0x%p)\n", link);

	/*
	 * Recheck to see if the device is still configured.
	 */
	if (info->link.state & DEV_CONFIG) {
		for (i = 0; i < info->ndev; i++)
			serial8250_unregister_port(info->line[i]);

		info->link.dev = NULL;

		if (!info->slave) {
			pcmcia_release_configuration(info->link.handle);
			pcmcia_release_io(info->link.handle, &info->link.io);
			pcmcia_release_irq(info->link.handle, &info->link.irq);
		}

		info->link.state &= ~DEV_CONFIG;
	}
}

static void serial_suspend(dev_link_t *link)
{
	link->state |= DEV_SUSPEND;

	if (link->state & DEV_CONFIG) {
		struct serial_info *info = link->priv;
		int i;

		for (i = 0; i < info->ndev; i++)
			serial8250_suspend_port(info->line[i]);

		if (!info->slave)
			pcmcia_release_configuration(link->handle);
	}
}

static void serial_resume(dev_link_t *link)
{
	link->state &= ~DEV_SUSPEND;

	if (DEV_OK(link)) {
		struct serial_info *info = link->priv;
		int i;

		if (!info->slave)
			pcmcia_request_configuration(link->handle, &link->conf);

		for (i = 0; i < info->ndev; i++)
			serial8250_resume_port(info->line[i]);
	}
}

/*======================================================================

    serial_attach() creates an "instance" of the driver, allocating
    local data structures for one device.  The device is registered
    with Card Services.

======================================================================*/

static dev_link_t *serial_attach(void)
{
	struct serial_info *info;
	client_reg_t client_reg;
	dev_link_t *link;
	int ret;

	DEBUG(0, "serial_attach()\n");

	/* Create new serial device */
	info = kmalloc(sizeof (*info), GFP_KERNEL);
	if (!info)
		return NULL;
	memset(info, 0, sizeof (*info));
	link = &info->link;
	link->priv = info;

	link->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
	link->io.NumPorts1 = 8;
	link->irq.Attributes = IRQ_TYPE_EXCLUSIVE;
	link->irq.IRQInfo1 = IRQ_LEVEL_ID;
	link->conf.Attributes = CONF_ENABLE_IRQ;
	if (do_sound) {
		link->conf.Attributes |= CONF_ENABLE_SPKR;
		link->conf.Status = CCSR_AUDIO_ENA;
	}
	link->conf.IntType = INT_MEMORY_AND_IO;

	/* Register with Card Services */
	link->next = dev_list;
	dev_list = link;
	client_reg.dev_info = &dev_info;
	client_reg.Version = 0x0210;
	client_reg.event_callback_args.client_data = link;
	ret = pcmcia_register_client(&link->handle, &client_reg);
	if (ret != CS_SUCCESS) {
		cs_error(link->handle, RegisterClient, ret);
		serial_detach(link);
		return NULL;
	}

	return link;
}

/*======================================================================

    This deletes a driver "instance".  The device is de-registered
    with Card Services.  If it has been released, all local data
    structures are freed.  Otherwise, the structures will be freed
    when the device is released.

======================================================================*/

static void serial_detach(dev_link_t * link)
{
	struct serial_info *info = link->priv;
	dev_link_t **linkp;
	int ret;

	DEBUG(0, "serial_detach(0x%p)\n", link);

	/* Locate device structure */
	for (linkp = &dev_list; *linkp; linkp = &(*linkp)->next)
		if (*linkp == link)
			break;
	if (*linkp == NULL)
		return;

	/*
	 * Ensure any outstanding scheduled tasks are completed.
	 */
	flush_scheduled_work();

	/*
	 * Ensure that the ports have been released.
	 */
	serial_remove(link);

	if (link->handle) {
		ret = pcmcia_deregister_client(link->handle);
		if (ret != CS_SUCCESS)
			cs_error(link->handle, DeregisterClient, ret);
	}

	/* Unlink device structure, free bits */
	*linkp = link->next;
	kfree(info);
}

/*====================================================================*/

static int setup_serial(client_handle_t handle, struct serial_info * info,
			kio_addr_t iobase, int irq)
{
	struct uart_port port;
	int line;

	memset(&port, 0, sizeof (struct uart_port));
	port.iobase = iobase;
	port.irq = irq;
	port.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST | UPF_SHARE_IRQ;
	port.uartclk = 1843200;
	port.dev = &handle_to_dev(handle);
	if (buggy_uart)
		port.flags |= UPF_BUGGY_UART;
	line = serial8250_register_port(&port);
	if (line < 0) {
		printk(KERN_NOTICE "serial_cs: serial8250_register_port() at "
		       "0x%04lx, irq %d failed\n", (u_long)iobase, irq);
		return -EINVAL;
	}

	info->line[info->ndev] = line;
	sprintf(info->node[info->ndev].dev_name, "ttyS%d", line);
	info->node[info->ndev].major = TTY_MAJOR;
	info->node[info->ndev].minor = 0x40 + line;
	if (info->ndev > 0)
		info->node[info->ndev - 1].next = &info->node[info->ndev];
	info->ndev++;

	return 0;
}

/*====================================================================*/

static int
first_tuple(client_handle_t handle, tuple_t * tuple, cisparse_t * parse)
{
	int i;
	i = pcmcia_get_first_tuple(handle, tuple);
	if (i != CS_SUCCESS)
		return CS_NO_MORE_ITEMS;
	i = pcmcia_get_tuple_data(handle, tuple);
	if (i != CS_SUCCESS)
		return i;
	return pcmcia_parse_tuple(handle, tuple, parse);
}

static int
next_tuple(client_handle_t handle, tuple_t * tuple, cisparse_t * parse)
{
	int i;
	i = pcmcia_get_next_tuple(handle, tuple);
	if (i != CS_SUCCESS)
		return CS_NO_MORE_ITEMS;
	i = pcmcia_get_tuple_data(handle, tuple);
	if (i != CS_SUCCESS)
		return i;
	return pcmcia_parse_tuple(handle, tuple, parse);
}

/*====================================================================*/

static int simple_config(dev_link_t *link)
{
	static kio_addr_t base[5] = { 0x3f8, 0x2f8, 0x3e8, 0x2e8, 0x0 };
	static int size_table[2] = { 8, 16 };
	client_handle_t handle = link->handle;
	struct serial_info *info = link->priv;
	struct serial_cfg_mem *cfg_mem;
	tuple_t *tuple;
	u_char *buf;
	cisparse_t *parse;
	cistpl_cftable_entry_t *cf;
	config_info_t config;
	int i, j, try;
	int s;

	cfg_mem = kmalloc(sizeof(struct serial_cfg_mem), GFP_KERNEL);
	if (!cfg_mem)
		return -1;

	tuple = &cfg_mem->tuple;
	parse = &cfg_mem->parse;
	cf = &parse->cftable_entry;
	buf = cfg_mem->buf;

	/* If the card is already configured, look up the port and irq */
	i = pcmcia_get_configuration_info(handle, &config);
	if ((i == CS_SUCCESS) && (config.Attributes & CONF_VALID_CLIENT)) {
		kio_addr_t port = 0;
		if ((config.BasePort2 != 0) && (config.NumPorts2 == 8)) {
			port = config.BasePort2;
			info->slave = 1;
		} else if ((info->manfid == MANFID_OSITECH) &&
			   (config.NumPorts1 == 0x40)) {
			port = config.BasePort1 + 0x28;
			info->slave = 1;
		}
		if (info->slave) {
			kfree(cfg_mem);
			return setup_serial(handle, info, port, config.AssignedIRQ);
		}
	}
	link->conf.Vcc = config.Vcc;

	/* First pass: look for a config entry that looks normal. */
	tuple->TupleData = (cisdata_t *) buf;
	tuple->TupleOffset = 0;
	tuple->TupleDataMax = 255;
	tuple->Attributes = 0;
	tuple->DesiredTuple = CISTPL_CFTABLE_ENTRY;
	/* Two tries: without IO aliases, then with aliases */
	for (s = 0; s < 2; s++) {
		for (try = 0; try < 2; try++) {
			i = first_tuple(handle, tuple, parse);
			while (i != CS_NO_MORE_ITEMS) {
				if (i != CS_SUCCESS)
					goto next_entry;
				if (cf->vpp1.present & (1 << CISTPL_POWER_VNOM))
					link->conf.Vpp1 = link->conf.Vpp2 =
					    cf->vpp1.param[CISTPL_POWER_VNOM] / 10000;
				if ((cf->io.nwin > 0) && (cf->io.win[0].len == size_table[s]) &&
					    (cf->io.win[0].base != 0)) {
					link->conf.ConfigIndex = cf->index;
					link->io.BasePort1 = cf->io.win[0].base;
					link->io.IOAddrLines = (try == 0) ?
					    16 : cf->io.flags & CISTPL_IO_LINES_MASK;
					i = pcmcia_request_io(link->handle, &link->io);
					if (i == CS_SUCCESS)
						goto found_port;
				}
next_entry:
				i = next_tuple(handle, tuple, parse);
			}
		}
	}
	/* Second pass: try to find an entry that isn't picky about
	   its base address, then try to grab any standard serial port
	   address, and finally try to get any free port. */
	i = first_tuple(handle, tuple, parse);
	while (i != CS_NO_MORE_ITEMS) {
		if ((i == CS_SUCCESS) && (cf->io.nwin > 0) &&
		    ((cf->io.flags & CISTPL_IO_LINES_MASK) <= 3)) {
			link->conf.ConfigIndex = cf->index;
			for (j = 0; j < 5; j++) {
				link->io.BasePort1 = base[j];
				link->io.IOAddrLines = base[j] ? 16 : 3;
				i = pcmcia_request_io(link->handle, &link->io);
				if (i == CS_SUCCESS)
					goto found_port;
			}
		}
		i = next_tuple(handle, tuple, parse);
	}

      found_port:
	if (i != CS_SUCCESS) {
		printk(KERN_NOTICE
		       "serial_cs: no usable port range found, giving up\n");
		cs_error(link->handle, RequestIO, i);
		kfree(cfg_mem);
		return -1;
	}

	i = pcmcia_request_irq(link->handle, &link->irq);
	if (i != CS_SUCCESS) {
		cs_error(link->handle, RequestIRQ, i);
		link->irq.AssignedIRQ = 0;
	}
	if (info->multi && (info->manfid == MANFID_3COM))
		link->conf.ConfigIndex &= ~(0x08);
	i = pcmcia_request_configuration(link->handle, &link->conf);
	if (i != CS_SUCCESS) {
		cs_error(link->handle, RequestConfiguration, i);
		kfree(cfg_mem);
		return -1;
	}
	kfree(cfg_mem);
	return setup_serial(handle, info, link->io.BasePort1, link->irq.AssignedIRQ);
}

static int multi_config(dev_link_t * link)
{
	client_handle_t handle = link->handle;
	struct serial_info *info = link->priv;
	struct serial_cfg_mem *cfg_mem;
	tuple_t *tuple;
	u_char *buf;
	cisparse_t *parse;
	cistpl_cftable_entry_t *cf;
	config_info_t config;
	int i, rc, base2 = 0;

	cfg_mem = kmalloc(sizeof(struct serial_cfg_mem), GFP_KERNEL);
	if (!cfg_mem)
		return -1;
	tuple = &cfg_mem->tuple;
	parse = &cfg_mem->parse;
	cf = &parse->cftable_entry;
	buf = cfg_mem->buf;

	i = pcmcia_get_configuration_info(handle, &config);
	if (i != CS_SUCCESS) {
		cs_error(handle, GetConfigurationInfo, i);
		rc = -1;
		goto free_cfg_mem;
	}
	link->conf.Vcc = config.Vcc;

	tuple->TupleData = (cisdata_t *) buf;
	tuple->TupleOffset = 0;
	tuple->TupleDataMax = 255;
	tuple->Attributes = 0;
	tuple->DesiredTuple = CISTPL_CFTABLE_ENTRY;

	/* First, look for a generic full-sized window */
	link->io.NumPorts1 = info->multi * 8;
	i = first_tuple(handle, tuple, parse);
	while (i != CS_NO_MORE_ITEMS) {
		/* The quad port cards have bad CIS's, so just look for a
		   window larger than 8 ports and assume it will be right */
		if ((i == CS_SUCCESS) && (cf->io.nwin == 1) &&
		    (cf->io.win[0].len > 8)) {
			link->conf.ConfigIndex = cf->index;
			link->io.BasePort1 = cf->io.win[0].base;
			link->io.IOAddrLines =
			    cf->io.flags & CISTPL_IO_LINES_MASK;
			i = pcmcia_request_io(link->handle, &link->io);
			base2 = link->io.BasePort1 + 8;
			if (i == CS_SUCCESS)
				break;
		}
		i = next_tuple(handle, tuple, parse);
	}

	/* If that didn't work, look for two windows */
	if (i != CS_SUCCESS) {
		link->io.NumPorts1 = link->io.NumPorts2 = 8;
		info->multi = 2;
		i = first_tuple(handle, tuple, parse);
		while (i != CS_NO_MORE_ITEMS) {
			if ((i == CS_SUCCESS) && (cf->io.nwin == 2)) {
				link->conf.ConfigIndex = cf->index;
				link->io.BasePort1 = cf->io.win[0].base;
				link->io.BasePort2 = cf->io.win[1].base;
				link->io.IOAddrLines =
				    cf->io.flags & CISTPL_IO_LINES_MASK;
				i = pcmcia_request_io(link->handle, &link->io);
				base2 = link->io.BasePort2;
				if (i == CS_SUCCESS)
					break;
			}
			i = next_tuple(handle, tuple, parse);
		}
	}

	if (i != CS_SUCCESS) {
		cs_error(link->handle, RequestIO, i);
		rc = -1;
		goto free_cfg_mem;
	}

	i = pcmcia_request_irq(link->handle, &link->irq);
	if (i != CS_SUCCESS) {
		printk(KERN_NOTICE
		       "serial_cs: no usable port range found, giving up\n");
		cs_error(link->handle, RequestIRQ, i);
		link->irq.AssignedIRQ = 0;
	}
	/* Socket Dual IO: this enables irq's for second port */
	if (info->multi && (info->manfid == MANFID_SOCKET)) {
		link->conf.Present |= PRESENT_EXT_STATUS;
		link->conf.ExtStatus = ESR_REQ_ATTN_ENA;
	}
	i = pcmcia_request_configuration(link->handle, &link->conf);
	if (i != CS_SUCCESS) {
		cs_error(link->handle, RequestConfiguration, i);
		rc = -1;
		goto free_cfg_mem;
	}

	/* The Oxford Semiconductor OXCF950 cards are in fact single-port:
	   8 registers are for the UART, the others are extra registers */
	if (info->manfid == MANFID_OXSEMI) {
		if (cf->index == 1 || cf->index == 3) {
			setup_serial(handle, info, base2, link->irq.AssignedIRQ);
			outb(12, link->io.BasePort1 + 1);
		} else {
			setup_serial(handle, info, link->io.BasePort1, link->irq.AssignedIRQ);
			outb(12, base2 + 1);
		}
		rc = 0;
		goto free_cfg_mem;
	}

	setup_serial(handle, info, link->io.BasePort1, link->irq.AssignedIRQ);
	/* The Nokia cards are not really multiport cards */
	if (info->manfid == MANFID_NOKIA) {
		rc = 0;
		goto free_cfg_mem;
	}
	for (i = 0; i < info->multi - 1; i++)
		setup_serial(handle, info, base2 + (8 * i),
				link->irq.AssignedIRQ);
	rc = 0;
free_cfg_mem:
	kfree(cfg_mem);
	return rc;
}

/*======================================================================

    serial_config() is scheduled to run after a CARD_INSERTION event
    is received, to configure the PCMCIA socket, and to make the
    serial device available to the system.

======================================================================*/

void serial_config(dev_link_t * link)
{
	client_handle_t handle = link->handle;
	struct serial_info *info = link->priv;
	struct serial_cfg_mem *cfg_mem;
	tuple_t *tuple;
	u_char *buf;
	cisparse_t *parse;
	cistpl_cftable_entry_t *cf;
	int i, last_ret, last_fn;

	DEBUG(0, "serial_config(0x%p)\n", link);

	cfg_mem = kmalloc(sizeof(struct serial_cfg_mem), GFP_KERNEL);
	if (!cfg_mem)
		goto failed;

	tuple = &cfg_mem->tuple;
	parse = &cfg_mem->parse;
	cf = &parse->cftable_entry;
	buf = cfg_mem->buf;

	tuple->TupleData = (cisdata_t *) buf;
	tuple->TupleOffset = 0;
	tuple->TupleDataMax = 255;
	tuple->Attributes = 0;
	/* Get configuration register information */
	tuple->DesiredTuple = CISTPL_CONFIG;
	last_ret = first_tuple(handle, tuple, parse);
	if (last_ret != CS_SUCCESS) {
		last_fn = ParseTuple;
		goto cs_failed;
	}
	link->conf.ConfigBase = parse->config.base;
	link->conf.Present = parse->config.rmask[0];

	/* Configure card */
	link->state |= DEV_CONFIG;

	/* Is this a compliant multifunction card? */
	tuple->DesiredTuple = CISTPL_LONGLINK_MFC;
	tuple->Attributes = TUPLE_RETURN_COMMON | TUPLE_RETURN_LINK;
	info->multi = (first_tuple(handle, tuple, parse) == CS_SUCCESS);

	/* Is this a multiport card? */
	tuple->DesiredTuple = CISTPL_MANFID;
	if (first_tuple(handle, tuple, parse) == CS_SUCCESS) {
		info->manfid = parse->manfid.manf;
		for (i = 0; i < MULTI_COUNT; i++)
			if ((info->manfid == multi_id[i].manfid) &&
			    (parse->manfid.card == multi_id[i].prodid))
				break;
		if (i < MULTI_COUNT)
			info->multi = multi_id[i].multi;
	}

	/* Another check for dual-serial cards: look for either serial or
	   multifunction cards that ask for appropriate IO port ranges */
	tuple->DesiredTuple = CISTPL_FUNCID;
	if ((info->multi == 0) &&
	    ((first_tuple(handle, tuple, parse) != CS_SUCCESS) ||
	     (parse->funcid.func == CISTPL_FUNCID_MULTI) ||
	     (parse->funcid.func == CISTPL_FUNCID_SERIAL))) {
		tuple->DesiredTuple = CISTPL_CFTABLE_ENTRY;
		if (first_tuple(handle, tuple, parse) == CS_SUCCESS) {
			if ((cf->io.nwin == 1) && (cf->io.win[0].len % 8 == 0))
				info->multi = cf->io.win[0].len >> 3;
			if ((cf->io.nwin == 2) && (cf->io.win[0].len == 8) &&
			    (cf->io.win[1].len == 8))
				info->multi = 2;
		}
	}

	if (info->multi > 1)
		multi_config(link);
	else
		simple_config(link);

	if (info->ndev == 0)
		goto failed;

	if (info->manfid == MANFID_IBM) {
		conf_reg_t reg = { 0, CS_READ, 0x800, 0 };
		last_ret = pcmcia_access_configuration_register(link->handle, &reg);
		if (last_ret) {
			last_fn = AccessConfigurationRegister;
			goto cs_failed;
		}
		reg.Action = CS_WRITE;
		reg.Value = reg.Value | 1;
		last_ret = pcmcia_access_configuration_register(link->handle, &reg);
		if (last_ret) {
			last_fn = AccessConfigurationRegister;
			goto cs_failed;
		}
	}

	link->dev = &info->node[0];
	link->state &= ~DEV_CONFIG_PENDING;
	kfree(cfg_mem);
	return;

 cs_failed:
	cs_error(link->handle, last_fn, last_ret);
 failed:
	serial_remove(link);
	link->state &= ~DEV_CONFIG_PENDING;
	kfree(cfg_mem);
}

/*======================================================================

    The card status event handler.  Mostly, this schedules other
    stuff to run after an event is received.  A CARD_REMOVAL event
    also sets some flags to discourage the serial drivers from
    talking to the ports.
    
======================================================================*/

static int
serial_event(event_t event, int priority, event_callback_args_t * args)
{
	dev_link_t *link = args->client_data;
	struct serial_info *info = link->priv;

	DEBUG(1, "serial_event(0x%06x)\n", event);

	switch (event) {
	case CS_EVENT_CARD_REMOVAL:
		serial_remove(link);
		break;

	case CS_EVENT_CARD_INSERTION:
		link->state |= DEV_PRESENT | DEV_CONFIG_PENDING;
		serial_config(link);
		break;

	case CS_EVENT_PM_SUSPEND:
		serial_suspend(link);
		break;

	case CS_EVENT_RESET_PHYSICAL:
		if ((link->state & DEV_CONFIG) && !info->slave)
			pcmcia_release_configuration(link->handle);
		break;

	case CS_EVENT_PM_RESUME:
		serial_resume(link);
		break;

	case CS_EVENT_CARD_RESET:
		if (DEV_OK(link) && !info->slave)
			pcmcia_request_configuration(link->handle, &link->conf);
		break;
	}
	return 0;
}

static struct pcmcia_device_id serial_ids[] = {
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x0057, 0x0021),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x0089, 0x110a),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x0104, 0x000a),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x0105, 0xea15),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x0109, 0x0501),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x0138, 0x110a),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x0140, 0x000a),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x0143, 0x3341),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x0143, 0xc0ab),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x016c, 0x0081),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x021b, 0x0101),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x08a1, 0xc0ab),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x0105, 0x0d0a),
	PCMCIA_PFC_DEVICE_MANF_CARD(1, 0x0105, 0x0e0a),
	PCMCIA_PFC_DEVICE_PROD_ID123(1, "MEGAHERTZ", "CC/XJEM3288", "DATA/FAX/CELL ETHERNET MODEM", 0xf510db04, 0x04cd2988, 0x46a52d63),
	PCMCIA_PFC_DEVICE_PROD_ID123(1, "MEGAHERTZ", "CC/XJEM3336", "DATA/FAX/CELL ETHERNET MODEM", 0xf510db04, 0x0143b773, 0x46a52d63),
	PCMCIA_PFC_DEVICE_PROD_ID123(1, "MEGAHERTZ", "EM1144T", "PCMCIA MODEM", 0xf510db04, 0x856d66c8, 0xbd6c43ef),
	PCMCIA_PFC_DEVICE_PROD_ID123(1, "MEGAHERTZ", "XJEM1144/CCEM1144", "PCMCIA MODEM", 0xf510db04, 0x52d21e1e, 0xbd6c43ef),
	PCMCIA_PFC_DEVICE_PROD_ID13(1, "Xircom", "CEM28", 0x2e3ee845, 0x0ea978ea),
	PCMCIA_PFC_DEVICE_PROD_ID13(1, "Xircom", "CEM33", 0x2e3ee845, 0x80609023),
	PCMCIA_PFC_DEVICE_PROD_ID13(1, "Xircom", "CEM56", 0x2e3ee845, 0xa650c32a),
	PCMCIA_PFC_DEVICE_PROD_ID13(1, "Xircom", "REM10", 0x2e3ee845, 0x76df1d29),
	PCMCIA_PFC_DEVICE_PROD_ID13(1, "Xircom", "XEM5600", 0x2e3ee845, 0xf1403719),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "AnyCom", "Fast Ethernet + 56K COMBO", 0x578ba6e7, 0xb0ac62c4),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "D-Link", "DME336T", 0x1a424a1c, 0xb23897ff),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "Gateway 2000", "XJEM3336", 0xdd9989be, 0x662c394c),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "Grey Cell", "GCS3000", 0x2a151fac, 0x48b932ae),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "Linksys", "EtherFast 10&100 + 56K PC Card (PCMLM56)", 0x0733cc81, 0xb3765033),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "LINKSYS", "PCMLM336", 0xf7cb0b07, 0x7a821b58),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "MEGAHERTZ", "XJEM1144/CCEM1144", 0xf510db04, 0x52d21e1e),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "Ositech", "Trumpcard:Jack of Diamonds Modem+Ethernet", 0xc2f80cd, 0x656947b9),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "Ositech", "Trumpcard:Jack of Hearts Modem+Ethernet", 0xc2f80cd, 0xdc9ba5ed),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "PCMCIAs", "ComboCard", 0xdcfe12d3, 0xcd8906cc),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "PCMCIAs", "LanModem", 0xdcfe12d3, 0xc67c648f),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "TDK", "GlobalNetworker 3410/3412", 0x1eae9475, 0xd9a93bed),
	PCMCIA_PFC_DEVICE_PROD_ID12(1, "Xircom", "CreditCard Ethernet+Modem II", 0x2e3ee845, 0xeca401bf),
	PCMCIA_MFC_DEVICE_MANF_CARD(0, 0x0104, 0x0070),
	PCMCIA_MFC_DEVICE_MANF_CARD(1, 0x0101, 0x0562),
	PCMCIA_MFC_DEVICE_MANF_CARD(1, 0x0104, 0x0070),
	PCMCIA_MFC_DEVICE_MANF_CARD(1, 0x016c, 0x0020),
	PCMCIA_MFC_DEVICE_PROD_ID123(1, "APEX DATA", "MULTICARD", "ETHERNET-MODEM", 0x11c2da09, 0x7289dc5d, 0xaad95e1f),
	PCMCIA_MFC_DEVICE_PROD_ID12(1, "IBM", "Home and Away 28.8 PC Card       ", 0xb569a6e5, 0x5bd4ff2c),
	PCMCIA_MFC_DEVICE_PROD_ID12(1, "IBM", "Home and Away Credit Card Adapter", 0xb569a6e5, 0x4bdf15c3),
	PCMCIA_MFC_DEVICE_PROD_ID12(1, "IBM", "w95 Home and Away Credit Card ", 0xb569a6e5, 0xae911c15),
	PCMCIA_MFC_DEVICE_PROD_ID1(1, "Motorola MARQUIS", 0xf03e4e77),
	PCMCIA_MFC_DEVICE_PROD_ID2(1, "FAX/Modem/Ethernet Combo Card ", 0x1ed59302),
	PCMCIA_DEVICE_MANF_CARD(0x0089, 0x0301),
	PCMCIA_DEVICE_MANF_CARD(0x0101, 0x0039),
	PCMCIA_DEVICE_MANF_CARD(0x0104, 0x0006),
	PCMCIA_DEVICE_MANF_CARD(0x0105, 0x410a),
	PCMCIA_DEVICE_MANF_CARD(0x010b, 0x0d50),
	PCMCIA_DEVICE_MANF_CARD(0x010b, 0x0d51),
	PCMCIA_DEVICE_MANF_CARD(0x010b, 0x0d52),
	PCMCIA_DEVICE_MANF_CARD(0x010b, 0x0d53),
	PCMCIA_DEVICE_MANF_CARD(0x010b, 0xd180),
	PCMCIA_DEVICE_MANF_CARD(0x0137, 0x000e),
	PCMCIA_DEVICE_MANF_CARD(0x0137, 0x001b),
	PCMCIA_DEVICE_MANF_CARD(0x0137, 0x0025),
	PCMCIA_DEVICE_MANF_CARD(0x0137, 0x0045),
	PCMCIA_DEVICE_MANF_CARD(0x0137, 0x0052),
	PCMCIA_DEVICE_PROD_ID134("ADV", "TECH", "COMpad-32/85", 0x67459937, 0x916d02ba, 0x8fbe92ae),
	PCMCIA_DEVICE_PROD_ID124("GATEWAY2000", "CC3144", "PCMCIA MODEM", 0x506bccae, 0xcb3685f1, 0xbd6c43ef),
	PCMCIA_DEVICE_PROD_ID14("MEGAHERTZ", "PCMCIA MODEM", 0xf510db04, 0xbd6c43ef),
	PCMCIA_DEVICE_PROD_ID124("TOSHIBA", "T144PF", "PCMCIA MODEM", 0xb4585a1a, 0x7271409c, 0xbd6c43ef),
	PCMCIA_DEVICE_PROD_ID123("FUJITSU", "FC14F ", "MBH10213", 0x6ee5a3d8, 0x30ead12b, 0xb00f05a0),
	PCMCIA_DEVICE_PROD_ID13("MEGAHERTZ", "V.34 PCMCIA MODEM", 0xf510db04, 0xbb2cce4a),
	PCMCIA_DEVICE_PROD_ID12("Brain Boxes", "Bluetooth PC Card", 0xee138382, 0xd4ce9b02),
	PCMCIA_DEVICE_PROD_ID12("CIRRUS LOGIC", "FAX MODEM", 0xe625f451, 0xcecd6dfa),
	PCMCIA_DEVICE_PROD_ID12("COMPAQ", "PCMCIA 28800 FAX/DATA MODEM", 0xa3a3062c, 0x8cbd7c76),
	PCMCIA_DEVICE_PROD_ID12("COMPAQ", "PCMCIA 33600 FAX/DATA MODEM", 0xa3a3062c, 0x5a00ce95),
	PCMCIA_DEVICE_PROD_ID12("Computerboards, Inc.", "PCM-COM422", 0xd0b78f51, 0x7e2d49ed),
	PCMCIA_DEVICE_PROD_ID12("Dr. Neuhaus", "FURY CARD 14K4", 0x76942813, 0x8b96ce65),
	PCMCIA_DEVICE_PROD_ID12("Intelligent", "ANGIA FAX/MODEM", 0xb496e65e, 0xf31602a6),
	PCMCIA_DEVICE_PROD_ID12("Intel", "MODEM 2400+", 0x816cc815, 0x412729fb),
	PCMCIA_DEVICE_PROD_ID12("IOTech Inc ", "PCMCIA Dual RS-232 Serial Port Card", 0x3bd2d898, 0x92abc92f),
	PCMCIA_DEVICE_PROD_ID12("MACRONIX", "FAX/MODEM", 0x668388b3, 0x3f9bdf2f),
	PCMCIA_DEVICE_PROD_ID12("Multi-Tech", "MT1432LT", 0x5f73be51, 0x0b3e2383),
	PCMCIA_DEVICE_PROD_ID12("Multi-Tech", "MT2834LT", 0x5f73be51, 0x4cd7c09e),
	PCMCIA_DEVICE_PROD_ID12("OEM      ", "C288MX     ", 0xb572d360, 0xd2385b7a),
	PCMCIA_DEVICE_PROD_ID12("PCMCIA   ", "C336MX     ", 0x99bcafe9, 0xaa25bcab),
	PCMCIA_DEVICE_PROD_ID12("Quatech Inc", "PCMCIA Dual RS-232 Serial Port Card", 0xc4420b35, 0x92abc92f),
	PCMCIA_PFC_DEVICE_CIS_PROD_ID12(1, "PCMCIA", "EN2218-LAN/MODEM", 0x281f1c5d, 0x570f348e, "PCMLM28.cis"),
	PCMCIA_PFC_DEVICE_CIS_PROD_ID12(1, "PCMCIA", "UE2218-LAN/MODEM", 0x281f1c5d, 0x6fdcacee, "PCMLM28.cis"),
	PCMCIA_PFC_DEVICE_CIS_PROD_ID12(1, "Psion Dacom", "Gold Card V34 Ethernet", 0xf5f025c2, 0x338e8155, "PCMLM28.cis"),
	PCMCIA_PFC_DEVICE_CIS_PROD_ID12(1, "Psion Dacom", "Gold Card V34 Ethernet GSM", 0xf5f025c2, 0x4ae85d35, "PCMLM28.cis"),
	PCMCIA_PFC_DEVICE_CIS_PROD_ID12(1, "LINKSYS", "PCMLM28", 0xf7cb0b07, 0x66881874, "PCMLM28.cis"),
	PCMCIA_MFC_DEVICE_CIS_PROD_ID12(1, "DAYNA COMMUNICATIONS", "LAN AND MODEM MULTIFUNCTION", 0x8fdf8f89, 0xdd5ed9e8, "DP83903.cis"),
	PCMCIA_MFC_DEVICE_CIS_PROD_ID4(1, "NSC MF LAN/Modem", 0x58fc6056, "DP83903.cis"),
	PCMCIA_MFC_DEVICE_CIS_MANF_CARD(1, 0x0101, 0x0556, "3CCFEM556.cis"),
	PCMCIA_MFC_DEVICE_CIS_MANF_CARD(1, 0x0175, 0x0000, "DP83903.cis"),
	PCMCIA_MFC_DEVICE_CIS_MANF_CARD(1, 0x0101, 0x0035, "3CXEM556.cis"),
	PCMCIA_MFC_DEVICE_CIS_MANF_CARD(1, 0x0101, 0x003d, "3CXEM556.cis"),
	PCMCIA_DEVICE_CIS_MANF_CARD(0x0192, 0x0710, "SW_7xx_SER.cis"),	/* Sierra Wireless AC710/AC750 GPRS Network Adapter R1 */
	PCMCIA_DEVICE_CIS_PROD_ID12("MultiTech", "PCMCIA 56K DataFax", 0x842047ee, 0xc2efcf03, "MT5634ZLX.cis"),
	PCMCIA_DEVICE_CIS_PROD_ID12("ADVANTECH", "COMpad-32/85B-4", 0x96913a85, 0xcec8f102, "COMpad4.cis"),
	PCMCIA_DEVICE_CIS_PROD_ID123("ADVANTECH", "COMpad-32/85", "1.0", 0x96913a85, 0x8fbe92ae, 0x0877b627, "COMpad2.cis"),
	PCMCIA_DEVICE_CIS_PROD_ID2("RS-COM 2P", 0xad20b156, "RS-COM-2P.cis"),
	/* too generic */
	/* PCMCIA_MFC_DEVICE_MANF_CARD(0, 0x0160, 0x0002), */
	/* PCMCIA_MFC_DEVICE_MANF_CARD(1, 0x0160, 0x0002), */
	PCMCIA_DEVICE_FUNC_ID(2),
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, serial_ids);

static struct pcmcia_driver serial_cs_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= "serial_cs",
	},
	.attach		= serial_attach,
	.event		= serial_event,
	.detach		= serial_detach,
	.id_table	= serial_ids,
};

static int __init init_serial_cs(void)
{
	return pcmcia_register_driver(&serial_cs_driver);
}

static void __exit exit_serial_cs(void)
{
	pcmcia_unregister_driver(&serial_cs_driver);
	BUG_ON(dev_list != NULL);
}

module_init(init_serial_cs);
module_exit(exit_serial_cs);

MODULE_LICENSE("GPL");
