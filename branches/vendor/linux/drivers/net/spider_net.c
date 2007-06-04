/*
 * Network device driver for Cell Processor-Based Blade
 *
 * (C) Copyright IBM Corp. 2005
 *
 * Authors : Utz Bacher <utz.bacher@de.ibm.com>
 *           Jens Osterkamp <Jens.Osterkamp@de.ibm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/config.h>

#include <linux/compiler.h>
#include <linux/crc32.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/firmware.h>
#include <linux/if_vlan.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/mii.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/tcp.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <asm/bitops.h>
#include <asm/pci-bridge.h>
#include <net/checksum.h>

#include "spider_net.h"

MODULE_AUTHOR("Utz Bacher <utz.bacher@de.ibm.com> and Jens Osterkamp " \
	      "<Jens.Osterkamp@de.ibm.com>");
MODULE_DESCRIPTION("Spider Southbridge Gigabit Ethernet driver");
MODULE_LICENSE("GPL");

static int rx_descriptors = SPIDER_NET_RX_DESCRIPTORS_DEFAULT;
static int tx_descriptors = SPIDER_NET_TX_DESCRIPTORS_DEFAULT;

module_param(rx_descriptors, int, 0644);
module_param(tx_descriptors, int, 0644);

MODULE_PARM_DESC(rx_descriptors, "number of descriptors used " \
		 "in rx chains");
MODULE_PARM_DESC(tx_descriptors, "number of descriptors used " \
		 "in tx chain");

char spider_net_driver_name[] = "spidernet";

static struct pci_device_id spider_net_pci_tbl[] = {
	{ PCI_VENDOR_ID_TOSHIBA_2, PCI_DEVICE_ID_TOSHIBA_SPIDER_NET,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0UL },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, spider_net_pci_tbl);

/**
 * spider_net_read_reg - reads an SMMIO register of a card
 * @card: device structure
 * @reg: register to read from
 *
 * returns the content of the specified SMMIO register.
 */
static u32
spider_net_read_reg(struct spider_net_card *card, u32 reg)
{
	u32 value;

	value = readl(card->regs + reg);
	value = le32_to_cpu(value);

	return value;
}

/**
 * spider_net_write_reg - writes to an SMMIO register of a card
 * @card: device structure
 * @reg: register to write to
 * @value: value to write into the specified SMMIO register
 */
static void
spider_net_write_reg(struct spider_net_card *card, u32 reg, u32 value)
{
	value = cpu_to_le32(value);
	writel(value, card->regs + reg);
}

/**
 * spider_net_write_reg_sync - writes to an SMMIO register of a card
 * @card: device structure
 * @reg: register to write to
 * @value: value to write into the specified SMMIO register
 *
 * Unlike spider_net_write_reg, this will also make sure the
 * data arrives on the card by reading the reg again.
 */
static void
spider_net_write_reg_sync(struct spider_net_card *card, u32 reg, u32 value)
{
	value = cpu_to_le32(value);
	writel(value, card->regs + reg);
	(void)readl(card->regs + reg);
}

/**
 * spider_net_rx_irq_off - switch off rx irq on this spider card
 * @card: device structure
 *
 * switches off rx irq by masking them out in the GHIINTnMSK register
 */
static void
spider_net_rx_irq_off(struct spider_net_card *card)
{
	u32 regvalue;
	unsigned long flags;

	spin_lock_irqsave(&card->intmask_lock, flags);
	regvalue = spider_net_read_reg(card, SPIDER_NET_GHIINT0MSK);
	regvalue &= ~SPIDER_NET_RXINT;
	spider_net_write_reg_sync(card, SPIDER_NET_GHIINT0MSK, regvalue);
	spin_unlock_irqrestore(&card->intmask_lock, flags);
}

/** spider_net_write_phy - write to phy register
 * @netdev: adapter to be written to
 * @mii_id: id of MII
 * @reg: PHY register
 * @val: value to be written to phy register
 *
 * spider_net_write_phy_register writes to an arbitrary PHY
 * register via the spider GPCWOPCMD register. We assume the queue does
 * not run full (not more than 15 commands outstanding).
 **/
static void
spider_net_write_phy(struct net_device *netdev, int mii_id,
		     int reg, int val)
{
	struct spider_net_card *card = netdev_priv(netdev);
	u32 writevalue;

	writevalue = ((u32)mii_id << 21) |
		((u32)reg << 16) | ((u32)val);

	spider_net_write_reg(card, SPIDER_NET_GPCWOPCMD, writevalue);
}

/** spider_net_read_phy - read from phy register
 * @netdev: network device to be read from
 * @mii_id: id of MII
 * @reg: PHY register
 *
 * Returns value read from PHY register
 *
 * spider_net_write_phy reads from an arbitrary PHY
 * register via the spider GPCROPCMD register
 **/
static int
spider_net_read_phy(struct net_device *netdev, int mii_id, int reg)
{
	struct spider_net_card *card = netdev_priv(netdev);
	u32 readvalue;

	readvalue = ((u32)mii_id << 21) | ((u32)reg << 16);
	spider_net_write_reg(card, SPIDER_NET_GPCROPCMD, readvalue);

	/* we don't use semaphores to wait for an SPIDER_NET_GPROPCMPINT
	 * interrupt, as we poll for the completion of the read operation
	 * in spider_net_read_phy. Should take about 50 us */
	do {
		readvalue = spider_net_read_reg(card, SPIDER_NET_GPCROPCMD);
	} while (readvalue & SPIDER_NET_GPREXEC);

	readvalue &= SPIDER_NET_GPRDAT_MASK;

	return readvalue;
}

/**
 * spider_net_rx_irq_on - switch on rx irq on this spider card
 * @card: device structure
 *
 * switches on rx irq by enabling them in the GHIINTnMSK register
 */
static void
spider_net_rx_irq_on(struct spider_net_card *card)
{
	u32 regvalue;
	unsigned long flags;

	spin_lock_irqsave(&card->intmask_lock, flags);
	regvalue = spider_net_read_reg(card, SPIDER_NET_GHIINT0MSK);
	regvalue |= SPIDER_NET_RXINT;
	spider_net_write_reg_sync(card, SPIDER_NET_GHIINT0MSK, regvalue);
	spin_unlock_irqrestore(&card->intmask_lock, flags);
}

/**
 * spider_net_tx_irq_off - switch off tx irq on this spider card
 * @card: device structure
 *
 * switches off tx irq by masking them out in the GHIINTnMSK register
 */
static void
spider_net_tx_irq_off(struct spider_net_card *card)
{
	u32 regvalue;
	unsigned long flags;

	spin_lock_irqsave(&card->intmask_lock, flags);
	regvalue = spider_net_read_reg(card, SPIDER_NET_GHIINT0MSK);
	regvalue &= ~SPIDER_NET_TXINT;
	spider_net_write_reg_sync(card, SPIDER_NET_GHIINT0MSK, regvalue);
	spin_unlock_irqrestore(&card->intmask_lock, flags);
}

/**
 * spider_net_tx_irq_on - switch on tx irq on this spider card
 * @card: device structure
 *
 * switches on tx irq by enabling them in the GHIINTnMSK register
 */
static void
spider_net_tx_irq_on(struct spider_net_card *card)
{
	u32 regvalue;
	unsigned long flags;

	spin_lock_irqsave(&card->intmask_lock, flags);
	regvalue = spider_net_read_reg(card, SPIDER_NET_GHIINT0MSK);
	regvalue |= SPIDER_NET_TXINT;
	spider_net_write_reg_sync(card, SPIDER_NET_GHIINT0MSK, regvalue);
	spin_unlock_irqrestore(&card->intmask_lock, flags);
}

/**
 * spider_net_set_promisc - sets the unicast address or the promiscuous mode
 * @card: card structure
 *
 * spider_net_set_promisc sets the unicast destination address filter and
 * thus either allows for non-promisc mode or promisc mode
 */
static void
spider_net_set_promisc(struct spider_net_card *card)
{
	u32 macu, macl;
	struct net_device *netdev = card->netdev;

	if (netdev->flags & IFF_PROMISC) {
		/* clear destination entry 0 */
		spider_net_write_reg(card, SPIDER_NET_GMRUAFILnR, 0);
		spider_net_write_reg(card, SPIDER_NET_GMRUAFILnR + 0x04, 0);
		spider_net_write_reg(card, SPIDER_NET_GMRUA0FIL15R,
				     SPIDER_NET_PROMISC_VALUE);
	} else {
		macu = netdev->dev_addr[0];
		macu <<= 8;
		macu |= netdev->dev_addr[1];
		memcpy(&macl, &netdev->dev_addr[2], sizeof(macl));

		macu |= SPIDER_NET_UA_DESCR_VALUE;
		spider_net_write_reg(card, SPIDER_NET_GMRUAFILnR, macu);
		spider_net_write_reg(card, SPIDER_NET_GMRUAFILnR + 0x04, macl);
		spider_net_write_reg(card, SPIDER_NET_GMRUA0FIL15R,
				     SPIDER_NET_NONPROMISC_VALUE);
	}
}

/**
 * spider_net_get_mac_address - read mac address from spider card
 * @card: device structure
 *
 * reads MAC address from GMACUNIMACU and GMACUNIMACL registers
 */
static int
spider_net_get_mac_address(struct net_device *netdev)
{
	struct spider_net_card *card = netdev_priv(netdev);
	u32 macl, macu;

	macl = spider_net_read_reg(card, SPIDER_NET_GMACUNIMACL);
	macu = spider_net_read_reg(card, SPIDER_NET_GMACUNIMACU);

	netdev->dev_addr[0] = (macu >> 24) & 0xff;
	netdev->dev_addr[1] = (macu >> 16) & 0xff;
	netdev->dev_addr[2] = (macu >> 8) & 0xff;
	netdev->dev_addr[3] = macu & 0xff;
	netdev->dev_addr[4] = (macl >> 8) & 0xff;
	netdev->dev_addr[5] = macl & 0xff;

	if (!is_valid_ether_addr(&netdev->dev_addr[0]))
		return -EINVAL;

	return 0;
}

/**
 * spider_net_get_descr_status -- returns the status of a descriptor
 * @descr: descriptor to look at
 *
 * returns the status as in the dmac_cmd_status field of the descriptor
 */
static enum spider_net_descr_status
spider_net_get_descr_status(struct spider_net_descr *descr)
{
	u32 cmd_status;
	rmb();
	cmd_status = descr->dmac_cmd_status;
	rmb();
	cmd_status >>= SPIDER_NET_DESCR_IND_PROC_SHIFT;
	/* no need to mask out any bits, as cmd_status is 32 bits wide only
	 * (and unsigned) */
	return cmd_status;
}

/**
 * spider_net_set_descr_status -- sets the status of a descriptor
 * @descr: descriptor to change
 * @status: status to set in the descriptor
 *
 * changes the status to the specified value. Doesn't change other bits
 * in the status
 */
static void
spider_net_set_descr_status(struct spider_net_descr *descr,
			    enum spider_net_descr_status status)
{
	u32 cmd_status;
	/* read the status */
	mb();
	cmd_status = descr->dmac_cmd_status;
	/* clean the upper 4 bits */
	cmd_status &= SPIDER_NET_DESCR_IND_PROC_MASKO;
	/* add the status to it */
	cmd_status |= ((u32)status)<<SPIDER_NET_DESCR_IND_PROC_SHIFT;
	/* and write it back */
	descr->dmac_cmd_status = cmd_status;
	wmb();
}

/**
 * spider_net_free_chain - free descriptor chain
 * @card: card structure
 * @chain: address of chain
 *
 */
static void
spider_net_free_chain(struct spider_net_card *card,
		      struct spider_net_descr_chain *chain)
{
	struct spider_net_descr *descr;

	for (descr = chain->tail; !descr->bus_addr; descr = descr->next) {
		pci_unmap_single(card->pdev, descr->bus_addr,
				 SPIDER_NET_DESCR_SIZE, PCI_DMA_BIDIRECTIONAL);
		descr->bus_addr = 0;
	}
}

/**
 * spider_net_init_chain - links descriptor chain
 * @card: card structure
 * @chain: address of chain
 * @start_descr: address of descriptor array
 * @no: number of descriptors
 *
 * we manage a circular list that mirrors the hardware structure,
 * except that the hardware uses bus addresses.
 *
 * returns 0 on success, <0 on failure
 */
static int
spider_net_init_chain(struct spider_net_card *card,
		       struct spider_net_descr_chain *chain,
		       struct spider_net_descr *start_descr, int no)
{
	int i;
	struct spider_net_descr *descr;

	spin_lock_init(&card->chain_lock);

	descr = start_descr;
	memset(descr, 0, sizeof(*descr) * no);

	/* set up the hardware pointers in each descriptor */
	for (i=0; i<no; i++, descr++) {
		spider_net_set_descr_status(descr, SPIDER_NET_DESCR_NOT_IN_USE);

		descr->bus_addr =
			pci_map_single(card->pdev, descr,
				       SPIDER_NET_DESCR_SIZE,
				       PCI_DMA_BIDIRECTIONAL);

		if (descr->bus_addr == DMA_ERROR_CODE)
			goto iommu_error;

		descr->next = descr + 1;
		descr->prev = descr - 1;

	}
	/* do actual circular list */
	(descr-1)->next = start_descr;
	start_descr->prev = descr-1;

	descr = start_descr;
	for (i=0; i < no; i++, descr++) {
		descr->next_descr_addr = descr->next->bus_addr;
	}

	chain->head = start_descr;
	chain->tail = start_descr;

	return 0;

iommu_error:
	descr = start_descr;
	for (i=0; i < no; i++, descr++)
		if (descr->bus_addr)
			pci_unmap_single(card->pdev, descr->bus_addr,
					 SPIDER_NET_DESCR_SIZE, PCI_DMA_BIDIRECTIONAL);
	return -ENOMEM;
}

/**
 * spider_net_free_rx_chain_contents - frees descr contents in rx chain
 * @card: card structure
 *
 * returns 0 on success, <0 on failure
 */
static void
spider_net_free_rx_chain_contents(struct spider_net_card *card)
{
	struct spider_net_descr *descr;

	descr = card->rx_chain.head;
	while (descr->next != card->rx_chain.head) {
		if (descr->skb) {
			dev_kfree_skb(descr->skb);
			pci_unmap_single(card->pdev, descr->buf_addr,
					 SPIDER_NET_MAX_MTU,
					 PCI_DMA_BIDIRECTIONAL);
		}
		descr = descr->next;
	}
}

/**
 * spider_net_prepare_rx_descr - reinitializes a rx descriptor
 * @card: card structure
 * @descr: descriptor to re-init
 *
 * return 0 on succes, <0 on failure
 *
 * allocates a new rx skb, iommu-maps it and attaches it to the descriptor.
 * Activate the descriptor state-wise
 */
static int
spider_net_prepare_rx_descr(struct spider_net_card *card,
			    struct spider_net_descr *descr)
{
	int error = 0;
	int offset;
	int bufsize;

	/* we need to round up the buffer size to a multiple of 128 */
	bufsize = (SPIDER_NET_MAX_MTU + SPIDER_NET_RXBUF_ALIGN - 1) &
		(~(SPIDER_NET_RXBUF_ALIGN - 1));

	/* and we need to have it 128 byte aligned, therefore we allocate a
	 * bit more */
	/* allocate an skb */
	descr->skb = dev_alloc_skb(bufsize + SPIDER_NET_RXBUF_ALIGN - 1);
	if (!descr->skb) {
		if (net_ratelimit())
			if (netif_msg_rx_err(card))
				pr_err("Not enough memory to allocate "
					"rx buffer\n");
		return -ENOMEM;
	}
	descr->buf_size = bufsize;
	descr->result_size = 0;
	descr->valid_size = 0;
	descr->data_status = 0;
	descr->data_error = 0;

	offset = ((unsigned long)descr->skb->data) &
		(SPIDER_NET_RXBUF_ALIGN - 1);
	if (offset)
		skb_reserve(descr->skb, SPIDER_NET_RXBUF_ALIGN - offset);
	/* io-mmu-map the skb */
	descr->buf_addr = pci_map_single(card->pdev, descr->skb->data,
					 SPIDER_NET_MAX_MTU,
					 PCI_DMA_BIDIRECTIONAL);
	if (descr->buf_addr == DMA_ERROR_CODE) {
		dev_kfree_skb_any(descr->skb);
		if (netif_msg_rx_err(card))
			pr_err("Could not iommu-map rx buffer\n");
		spider_net_set_descr_status(descr, SPIDER_NET_DESCR_NOT_IN_USE);
	} else {
		descr->dmac_cmd_status = SPIDER_NET_DMAC_RX_CARDOWNED;
	}

	return error;
}

/**
 * spider_net_enable_rxctails - sets RX dmac chain tail addresses
 * @card: card structure
 *
 * spider_net_enable_rxctails sets the RX DMAC chain tail adresses in the
 * chip by writing to the appropriate register. DMA is enabled in
 * spider_net_enable_rxdmac.
 */
static void
spider_net_enable_rxchtails(struct spider_net_card *card)
{
	/* assume chain is aligned correctly */
	spider_net_write_reg(card, SPIDER_NET_GDADCHA ,
			     card->rx_chain.tail->bus_addr);
}

/**
 * spider_net_enable_rxdmac - enables a receive DMA controller
 * @card: card structure
 *
 * spider_net_enable_rxdmac enables the DMA controller by setting RX_DMA_EN
 * in the GDADMACCNTR register
 */
static void
spider_net_enable_rxdmac(struct spider_net_card *card)
{
	spider_net_write_reg(card, SPIDER_NET_GDADMACCNTR,
			     SPIDER_NET_DMA_RX_VALUE);
}

/**
 * spider_net_refill_rx_chain - refills descriptors/skbs in the rx chains
 * @card: card structure
 *
 * refills descriptors in all chains (last used chain first): allocates skbs
 * and iommu-maps them.
 */
static void
spider_net_refill_rx_chain(struct spider_net_card *card)
{
	struct spider_net_descr_chain *chain;
	int count = 0;
	unsigned long flags;

	chain = &card->rx_chain;

	spin_lock_irqsave(&card->chain_lock, flags);
	while (spider_net_get_descr_status(chain->head) ==
				SPIDER_NET_DESCR_NOT_IN_USE) {
		if (spider_net_prepare_rx_descr(card, chain->head))
			break;
		count++;
		chain->head = chain->head->next;
	}
	spin_unlock_irqrestore(&card->chain_lock, flags);

	/* could be optimized, only do that, if we know the DMA processing
	 * has terminated */
	if (count)
		spider_net_enable_rxdmac(card);
}

/**
 * spider_net_alloc_rx_skbs - allocates rx skbs in rx descriptor chains
 * @card: card structure
 *
 * returns 0 on success, <0 on failure
 */
static int
spider_net_alloc_rx_skbs(struct spider_net_card *card)
{
	int result;
	struct spider_net_descr_chain *chain;

	result = -ENOMEM;

	chain = &card->rx_chain;
	/* put at least one buffer into the chain. if this fails,
	 * we've got a problem. if not, spider_net_refill_rx_chain
	 * will do the rest at the end of this function */
	if (spider_net_prepare_rx_descr(card, chain->head))
		goto error;
	else
		chain->head = chain->head->next;

	/* this will allocate the rest of the rx buffers; if not, it's
	 * business as usual later on */
	spider_net_refill_rx_chain(card);
	return 0;

error:
	spider_net_free_rx_chain_contents(card);
	return result;
}

/**
 * spider_net_release_tx_descr - processes a used tx descriptor
 * @card: card structure
 * @descr: descriptor to release
 *
 * releases a used tx descriptor (unmapping, freeing of skb)
 */
static void
spider_net_release_tx_descr(struct spider_net_card *card,
			    struct spider_net_descr *descr)
{
	struct sk_buff *skb;

	/* unmap the skb */
	skb = descr->skb;
	pci_unmap_single(card->pdev, descr->buf_addr, skb->len,
			 PCI_DMA_BIDIRECTIONAL);

	dev_kfree_skb_any(skb);

	/* set status to not used */
	spider_net_set_descr_status(descr, SPIDER_NET_DESCR_NOT_IN_USE);
}

/**
 * spider_net_release_tx_chain - processes sent tx descriptors
 * @card: adapter structure
 * @brutal: if set, don't care about whether descriptor seems to be in use
 *
 * releases the tx descriptors that spider has finished with (if non-brutal)
 * or simply release tx descriptors (if brutal)
 */
static void
spider_net_release_tx_chain(struct spider_net_card *card, int brutal)
{
	struct spider_net_descr_chain *tx_chain = &card->tx_chain;
	enum spider_net_descr_status status;

	spider_net_tx_irq_off(card);

	/* no lock for chain needed, if this is only executed once at a time */
again:
	for (;;) {
		status = spider_net_get_descr_status(tx_chain->tail);
		switch (status) {
		case SPIDER_NET_DESCR_CARDOWNED:
			if (!brutal) goto out;
			/* fallthrough, if we release the descriptors
			 * brutally (then we don't care about
			 * SPIDER_NET_DESCR_CARDOWNED) */
		case SPIDER_NET_DESCR_RESPONSE_ERROR:
		case SPIDER_NET_DESCR_PROTECTION_ERROR:
		case SPIDER_NET_DESCR_FORCE_END:
			if (netif_msg_tx_err(card))
				pr_err("%s: forcing end of tx descriptor "
				       "with status x%02x\n",
				       card->netdev->name, status);
			card->netdev_stats.tx_dropped++;
			break;

		case SPIDER_NET_DESCR_COMPLETE:
			card->netdev_stats.tx_packets++;
			card->netdev_stats.tx_bytes +=
				tx_chain->tail->skb->len;
			break;

		default: /* any other value (== SPIDER_NET_DESCR_NOT_IN_USE) */
			goto out;
		}
		spider_net_release_tx_descr(card, tx_chain->tail);
		tx_chain->tail = tx_chain->tail->next;
	}
out:
	netif_wake_queue(card->netdev);

	if (!brutal) {
		/* switch on tx irqs (while we are still in the interrupt
		 * handler, so we don't get an interrupt), check again
		 * for done descriptors. This results in fewer interrupts */
		spider_net_tx_irq_on(card);
		status = spider_net_get_descr_status(tx_chain->tail);
		switch (status) {
			case SPIDER_NET_DESCR_RESPONSE_ERROR:
			case SPIDER_NET_DESCR_PROTECTION_ERROR:
			case SPIDER_NET_DESCR_FORCE_END:
			case SPIDER_NET_DESCR_COMPLETE:
				goto again;
			default:
				break;
		}
	}

}

/**
 * spider_net_get_multicast_hash - generates hash for multicast filter table
 * @addr: multicast address
 *
 * returns the hash value.
 *
 * spider_net_get_multicast_hash calculates a hash value for a given multicast
 * address, that is used to set the multicast filter tables
 */
static u8
spider_net_get_multicast_hash(struct net_device *netdev, __u8 *addr)
{
	/* FIXME: an addr of 01:00:5e:00:00:01 must result in 0xa9,
	 * ff:ff:ff:ff:ff:ff must result in 0xfd */
	u32 crc;
	u8 hash;

	crc = crc32_be(~0, addr, netdev->addr_len);

	hash = (crc >> 27);
	hash <<= 3;
	hash |= crc & 7;

	return hash;
}

/**
 * spider_net_set_multi - sets multicast addresses and promisc flags
 * @netdev: interface device structure
 *
 * spider_net_set_multi configures multicast addresses as needed for the
 * netdev interface. It also sets up multicast, allmulti and promisc
 * flags appropriately
 */
static void
spider_net_set_multi(struct net_device *netdev)
{
	struct dev_mc_list *mc;
	u8 hash;
	int i;
	u32 reg;
	struct spider_net_card *card = netdev_priv(netdev);
	unsigned long bitmask[SPIDER_NET_MULTICAST_HASHES / BITS_PER_LONG] =
		{0, };

	spider_net_set_promisc(card);

	if (netdev->flags & IFF_ALLMULTI) {
		for (i = 0; i < SPIDER_NET_MULTICAST_HASHES; i++) {
			set_bit(i, bitmask);
		}
		goto write_hash;
	}

	/* well, we know, what the broadcast hash value is: it's xfd
	hash = spider_net_get_multicast_hash(netdev, netdev->broadcast); */
	set_bit(0xfd, bitmask);

	for (mc = netdev->mc_list; mc; mc = mc->next) {
		hash = spider_net_get_multicast_hash(netdev, mc->dmi_addr);
		set_bit(hash, bitmask);
	}

write_hash:
	for (i = 0; i < SPIDER_NET_MULTICAST_HASHES / 4; i++) {
		reg = 0;
		if (test_bit(i * 4, bitmask))
			reg += 0x08;
		reg <<= 8;
		if (test_bit(i * 4 + 1, bitmask))
			reg += 0x08;
		reg <<= 8;
		if (test_bit(i * 4 + 2, bitmask))
			reg += 0x08;
		reg <<= 8;
		if (test_bit(i * 4 + 3, bitmask))
			reg += 0x08;

		spider_net_write_reg(card, SPIDER_NET_GMRMHFILnR + i * 4, reg);
	}
}

/**
 * spider_net_disable_rxdmac - disables the receive DMA controller
 * @card: card structure
 *
 * spider_net_disable_rxdmac terminates processing on the DMA controller by
 * turing off DMA and issueing a force end
 */
static void
spider_net_disable_rxdmac(struct spider_net_card *card)
{
	spider_net_write_reg(card, SPIDER_NET_GDADMACCNTR,
			     SPIDER_NET_DMA_RX_FEND_VALUE);
}

/**
 * spider_net_stop - called upon ifconfig down
 * @netdev: interface device structure
 *
 * always returns 0
 */
int
spider_net_stop(struct net_device *netdev)
{
	struct spider_net_card *card = netdev_priv(netdev);

	netif_poll_disable(netdev);
	netif_carrier_off(netdev);
	netif_stop_queue(netdev);

	/* disable/mask all interrupts */
	spider_net_write_reg(card, SPIDER_NET_GHIINT0MSK, 0);
	spider_net_write_reg(card, SPIDER_NET_GHIINT1MSK, 0);
	spider_net_write_reg(card, SPIDER_NET_GHIINT2MSK, 0);

	/* free_irq(netdev->irq, netdev);*/
	free_irq(to_pci_dev(netdev->class_dev.dev)->irq, netdev);

	spider_net_write_reg(card, SPIDER_NET_GDTDMACCNTR,
			     SPIDER_NET_DMA_TX_FEND_VALUE);

	/* turn off DMA, force end */
	spider_net_disable_rxdmac(card);

	/* release chains */
	spider_net_release_tx_chain(card, 1);

	spider_net_free_chain(card, &card->tx_chain);
	spider_net_free_chain(card, &card->rx_chain);

	return 0;
}

/**
 * spider_net_get_next_tx_descr - returns the next available tx descriptor
 * @card: device structure to get descriptor from
 *
 * returns the address of the next descriptor, or NULL if not available.
 */
static struct spider_net_descr *
spider_net_get_next_tx_descr(struct spider_net_card *card)
{
	/* check, if head points to not-in-use descr */
	if ( spider_net_get_descr_status(card->tx_chain.head) ==
	     SPIDER_NET_DESCR_NOT_IN_USE ) {
		return card->tx_chain.head;
	} else {
		return NULL;
	}
}

/**
 * spider_net_set_txdescr_cmdstat - sets the tx descriptor command field
 * @descr: descriptor structure to fill out
 * @skb: packet to consider
 *
 * fills out the command and status field of the descriptor structure,
 * depending on hardware checksum settings. This function assumes a wmb()
 * has executed before.
 */
static void
spider_net_set_txdescr_cmdstat(struct spider_net_descr *descr,
			       struct sk_buff *skb)
{
	if (skb->ip_summed != CHECKSUM_HW) {
		descr->dmac_cmd_status = SPIDER_NET_DMAC_CMDSTAT_NOCS;
		return;
	}

	/* is packet ip?
	 * if yes: tcp? udp? */
	if (skb->protocol == htons(ETH_P_IP)) {
		if (skb->nh.iph->protocol == IPPROTO_TCP) {
			descr->dmac_cmd_status = SPIDER_NET_DMAC_CMDSTAT_TCPCS;
		} else if (skb->nh.iph->protocol == IPPROTO_UDP) {
			descr->dmac_cmd_status = SPIDER_NET_DMAC_CMDSTAT_UDPCS;
		} else { /* the stack should checksum non-tcp and non-udp
			    packets on his own: NETIF_F_IP_CSUM */
			descr->dmac_cmd_status = SPIDER_NET_DMAC_CMDSTAT_NOCS;
		}
	}
}

/**
 * spider_net_prepare_tx_descr - fill tx descriptor with skb data
 * @card: card structure
 * @descr: descriptor structure to fill out
 * @skb: packet to use
 *
 * returns 0 on success, <0 on failure.
 *
 * fills out the descriptor structure with skb data and len. Copies data,
 * if needed (32bit DMA!)
 */
static int
spider_net_prepare_tx_descr(struct spider_net_card *card,
			    struct spider_net_descr *descr,
			    struct sk_buff *skb)
{
	descr->buf_addr = pci_map_single(card->pdev, skb->data,
					 skb->len, PCI_DMA_BIDIRECTIONAL);
	if (descr->buf_addr == DMA_ERROR_CODE) {
		if (netif_msg_tx_err(card))
			pr_err("could not iommu-map packet (%p, %i). "
				  "Dropping packet\n", skb->data, skb->len);
		return -ENOMEM;
	}

	descr->buf_size = skb->len;
	descr->skb = skb;
	descr->data_status = 0;

	/* make sure the above values are in memory before we change the
	 * status */
	wmb();

	spider_net_set_txdescr_cmdstat(descr,skb);

	return 0;
}

/**
 * spider_net_kick_tx_dma - enables TX DMA processing
 * @card: card structure
 * @descr: descriptor address to enable TX processing at
 *
 * spider_net_kick_tx_dma writes the current tx chain head as start address
 * of the tx descriptor chain and enables the transmission DMA engine
 */
static void
spider_net_kick_tx_dma(struct spider_net_card *card,
		       struct spider_net_descr *descr)
{
	/* this is the only descriptor in the output chain.
	 * Enable TX DMA */

	spider_net_write_reg(card, SPIDER_NET_GDTDCHA,
			     descr->bus_addr);

	spider_net_write_reg(card, SPIDER_NET_GDTDMACCNTR,
			     SPIDER_NET_DMA_TX_VALUE);
}

/**
 * spider_net_xmit - transmits a frame over the device
 * @skb: packet to send out
 * @netdev: interface device structure
 *
 * returns 0 on success, <0 on failure
 */
static int
spider_net_xmit(struct sk_buff *skb, struct net_device *netdev)
{
	struct spider_net_card *card = netdev_priv(netdev);
	struct spider_net_descr *descr;
	int result;

	descr = spider_net_get_next_tx_descr(card);

	if (!descr) {
		netif_stop_queue(netdev);

		descr = spider_net_get_next_tx_descr(card);
		if (!descr)
			goto error;
		else
			netif_start_queue(netdev);
	}

	result = spider_net_prepare_tx_descr(card, descr, skb);
	if (result)
		goto error;

	card->tx_chain.head = card->tx_chain.head->next;

	/* make sure the status from spider_net_prepare_tx_descr is in
	 * memory before we check out the previous descriptor */
	wmb();

	if (spider_net_get_descr_status(descr->prev) !=
	    SPIDER_NET_DESCR_CARDOWNED)
		spider_net_kick_tx_dma(card, descr);

	return NETDEV_TX_OK;

error:
	card->netdev_stats.tx_dropped++;
	return NETDEV_TX_LOCKED;
}

/**
 * spider_net_do_ioctl - called for device ioctls
 * @netdev: interface device structure
 * @ifr: request parameter structure for ioctl
 * @cmd: command code for ioctl
 *
 * returns 0 on success, <0 on failure. Currently, we have no special ioctls.
 * -EOPNOTSUPP is returned, if an unknown ioctl was requested
 */
static int
spider_net_do_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
	switch (cmd) {
	default:
		return -EOPNOTSUPP;
	}
}

/**
 * spider_net_pass_skb_up - takes an skb from a descriptor and passes it on
 * @descr: descriptor to process
 * @card: card structure
 *
 * returns 1 on success, 0 if no packet was passed to the stack
 *
 * iommu-unmaps the skb, fills out skb structure and passes the data to the
 * stack. The descriptor state is not changed.
 */
static int
spider_net_pass_skb_up(struct spider_net_descr *descr,
		       struct spider_net_card *card)
{
	struct sk_buff *skb;
	struct net_device *netdev;
	u32 data_status, data_error;

	data_status = descr->data_status;
	data_error = descr->data_error;

	netdev = card->netdev;

	/* check for errors in the data_error flag */
	if ((data_error & SPIDER_NET_DATA_ERROR_MASK) &&
	    netif_msg_rx_err(card))
		pr_err("error in received descriptor found, "
		       "data_status=x%08x, data_error=x%08x\n",
		       data_status, data_error);

	/* prepare skb, unmap descriptor */
	skb = descr->skb;
	pci_unmap_single(card->pdev, descr->buf_addr, SPIDER_NET_MAX_MTU,
			 PCI_DMA_BIDIRECTIONAL);

	/* the cases we'll throw away the packet immediately */
	if (data_error & SPIDER_NET_DESTROY_RX_FLAGS)
		return 0;

	skb->dev = netdev;
	skb_put(skb, descr->valid_size);

	/* the card seems to add 2 bytes of junk in front
	 * of the ethernet frame */
#define SPIDER_MISALIGN		2
	skb_pull(skb, SPIDER_MISALIGN);
	skb->protocol = eth_type_trans(skb, netdev);

	/* checksum offload */
	if (card->options.rx_csum) {
		if ( (data_status & SPIDER_NET_DATA_STATUS_CHK_MASK) &&
		     (!(data_error & SPIDER_NET_DATA_ERROR_CHK_MASK)) )
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
			skb->ip_summed = CHECKSUM_NONE;
	} else {
		skb->ip_summed = CHECKSUM_NONE;
	}

	if (data_status & SPIDER_NET_VLAN_PACKET) {
		/* further enhancements: HW-accel VLAN
		 * vlan_hwaccel_receive_skb
		 */
	}

	/* pass skb up to stack */
	netif_receive_skb(skb);

	/* update netdevice statistics */
	card->netdev_stats.rx_packets++;
	card->netdev_stats.rx_bytes += skb->len;

	return 1;
}

/**
 * spider_net_decode_descr - processes an rx descriptor
 * @card: card structure
 *
 * returns 1 if a packet has been sent to the stack, otherwise 0
 *
 * processes an rx descriptor by iommu-unmapping the data buffer and passing
 * the packet up to the stack
 */
static int
spider_net_decode_one_descr(struct spider_net_card *card)
{
	enum spider_net_descr_status status;
	struct spider_net_descr *descr;
	struct spider_net_descr_chain *chain;
	int result;

	chain = &card->rx_chain;
	descr = chain->tail;

	status = spider_net_get_descr_status(descr);

	if (status == SPIDER_NET_DESCR_CARDOWNED) {
		/* nothing in the descriptor yet */
		return 0;
	}

	if (status == SPIDER_NET_DESCR_NOT_IN_USE) {
		/* not initialized yet, I bet chain->tail == chain->head
		 * and the ring is empty */
		spider_net_refill_rx_chain(card);
		return 0;
	}

	/* descriptor definitively used -- move on head */
	chain->tail = descr->next;

	result = 0;
	if ( (status == SPIDER_NET_DESCR_RESPONSE_ERROR) ||
	     (status == SPIDER_NET_DESCR_PROTECTION_ERROR) ||
	     (status == SPIDER_NET_DESCR_FORCE_END) ) {
		if (netif_msg_rx_err(card))
			pr_err("%s: dropping RX descriptor with state %d\n",
			       card->netdev->name, status);
		card->netdev_stats.rx_dropped++;
		goto refill;
	}

	if ( (status != SPIDER_NET_DESCR_COMPLETE) &&
	     (status != SPIDER_NET_DESCR_FRAME_END) ) {
		if (netif_msg_rx_err(card))
			pr_err("%s: RX descriptor with state %d\n",
			       card->netdev->name, status);
		goto refill;
	}

	/* ok, we've got a packet in descr */
	result = spider_net_pass_skb_up(descr, card);
refill:
	spider_net_set_descr_status(descr, SPIDER_NET_DESCR_NOT_IN_USE);
	/* change the descriptor state: */
	spider_net_refill_rx_chain(card);

	return result;
}

/**
 * spider_net_poll - NAPI poll function called by the stack to return packets
 * @netdev: interface device structure
 * @budget: number of packets we can pass to the stack at most
 *
 * returns 0 if no more packets available to the driver/stack. Returns 1,
 * if the quota is exceeded, but the driver has still packets.
 *
 * spider_net_poll returns all packets from the rx descriptors to the stack
 * (using netif_receive_skb). If all/enough packets are up, the driver
 * reenables interrupts and returns 0. If not, 1 is returned.
 */
static int
spider_net_poll(struct net_device *netdev, int *budget)
{
	struct spider_net_card *card = netdev_priv(netdev);
	int packets_to_do, packets_done = 0;
	int no_more_packets = 0;

	packets_to_do = min(*budget, netdev->quota);

	while (packets_to_do) {
		if (spider_net_decode_one_descr(card)) {
			packets_done++;
			packets_to_do--;
		} else {
			/* no more packets for the stack */
			no_more_packets = 1;
			break;
		}
	}

	netdev->quota -= packets_done;
	*budget -= packets_done;

	/* if all packets are in the stack, enable interrupts and return 0 */
	/* if not, return 1 */
	if (no_more_packets) {
		netif_rx_complete(netdev);
		spider_net_rx_irq_on(card);
		return 0;
	}

	return 1;
}

/**
 * spider_net_vlan_rx_reg - initializes VLAN structures in the driver and card
 * @netdev: interface device structure
 * @grp: vlan_group structure that is registered (NULL on destroying interface)
 */
static void
spider_net_vlan_rx_reg(struct net_device *netdev, struct vlan_group *grp)
{
	/* further enhancement... yet to do */
	return;
}

/**
 * spider_net_vlan_rx_add - adds VLAN id to the card filter
 * @netdev: interface device structure
 * @vid: VLAN id to add
 */
static void
spider_net_vlan_rx_add(struct net_device *netdev, uint16_t vid)
{
	/* further enhancement... yet to do */
	/* add vid to card's VLAN filter table */
	return;
}

/**
 * spider_net_vlan_rx_kill - removes VLAN id to the card filter
 * @netdev: interface device structure
 * @vid: VLAN id to remove
 */
static void
spider_net_vlan_rx_kill(struct net_device *netdev, uint16_t vid)
{
	/* further enhancement... yet to do */
	/* remove vid from card's VLAN filter table */
}

/**
 * spider_net_get_stats - get interface statistics
 * @netdev: interface device structure
 *
 * returns the interface statistics residing in the spider_net_card struct
 */
static struct net_device_stats *
spider_net_get_stats(struct net_device *netdev)
{
	struct spider_net_card *card = netdev_priv(netdev);
	struct net_device_stats *stats = &card->netdev_stats;
	return stats;
}

/**
 * spider_net_change_mtu - changes the MTU of an interface
 * @netdev: interface device structure
 * @new_mtu: new MTU value
 *
 * returns 0 on success, <0 on failure
 */
static int
spider_net_change_mtu(struct net_device *netdev, int new_mtu)
{
	/* no need to re-alloc skbs or so -- the max mtu is about 2.3k
	 * and mtu is outbound only anyway */
	if ( (new_mtu < SPIDER_NET_MIN_MTU ) ||
		(new_mtu > SPIDER_NET_MAX_MTU) )
		return -EINVAL;
	netdev->mtu = new_mtu;
	return 0;
}

/**
 * spider_net_set_mac - sets the MAC of an interface
 * @netdev: interface device structure
 * @ptr: pointer to new MAC address
 *
 * Returns 0 on success, <0 on failure. Currently, we don't support this
 * and will always return EOPNOTSUPP.
 */
static int
spider_net_set_mac(struct net_device *netdev, void *p)
{
	struct spider_net_card *card = netdev_priv(netdev);
	u32 macl, macu, regvalue;
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	/* switch off GMACTPE and GMACRPE */
	regvalue = spider_net_read_reg(card, SPIDER_NET_GMACOPEMD);
	regvalue &= ~((1 << 5) | (1 << 6));
	spider_net_write_reg(card, SPIDER_NET_GMACOPEMD, regvalue);

	/* write mac */
	macu = (addr->sa_data[0]<<24) + (addr->sa_data[1]<<16) +
		(addr->sa_data[2]<<8) + (addr->sa_data[3]);
	macl = (addr->sa_data[4]<<8) + (addr->sa_data[5]);
	spider_net_write_reg(card, SPIDER_NET_GMACUNIMACU, macu);
	spider_net_write_reg(card, SPIDER_NET_GMACUNIMACL, macl);

	/* switch GMACTPE and GMACRPE back on */
	regvalue = spider_net_read_reg(card, SPIDER_NET_GMACOPEMD);
	regvalue |= ((1 << 5) | (1 << 6));
	spider_net_write_reg(card, SPIDER_NET_GMACOPEMD, regvalue);

	spider_net_set_promisc(card);

	/* look up, whether we have been successful */
	if (spider_net_get_mac_address(netdev))
		return -EADDRNOTAVAIL;
	if (memcmp(netdev->dev_addr,addr->sa_data,netdev->addr_len))
		return -EADDRNOTAVAIL;

	return 0;
}

/**
 * spider_net_enable_txdmac - enables a TX DMA controller
 * @card: card structure
 *
 * spider_net_enable_txdmac enables the TX DMA controller by setting the
 * descriptor chain tail address
 */
static void
spider_net_enable_txdmac(struct spider_net_card *card)
{
	/* assume chain is aligned correctly */
	spider_net_write_reg(card, SPIDER_NET_GDTDCHA,
			     card->tx_chain.tail->bus_addr);
}

/**
 * spider_net_handle_error_irq - handles errors raised by an interrupt
 * @card: card structure
 * @status_reg: interrupt status register 0 (GHIINT0STS)
 *
 * spider_net_handle_error_irq treats or ignores all error conditions
 * found when an interrupt is presented
 */
static void
spider_net_handle_error_irq(struct spider_net_card *card, u32 status_reg)
{
	u32 error_reg1, error_reg2;
	u32 i;
	int show_error = 1;

	error_reg1 = spider_net_read_reg(card, SPIDER_NET_GHIINT1STS);
	error_reg2 = spider_net_read_reg(card, SPIDER_NET_GHIINT2STS);

	/* check GHIINT0STS ************************************/
	if (status_reg)
		for (i = 0; i < 32; i++)
			if (status_reg & (1<<i))
				switch (i)
	{
	/* let error_reg1 and error_reg2 evaluation decide, what to do
	case SPIDER_NET_PHYINT:
	case SPIDER_NET_GMAC2INT:
	case SPIDER_NET_GMAC1INT:
	case SPIDER_NET_GIPSINT:
	case SPIDER_NET_GFIFOINT:
	case SPIDER_NET_DMACINT:
	case SPIDER_NET_GSYSINT:
		break; */

	case SPIDER_NET_GPWOPCMPINT:
		/* PHY write operation completed */
		show_error = 0;
		break;
	case SPIDER_NET_GPROPCMPINT:
		/* PHY read operation completed */
		/* we don't use semaphores, as we poll for the completion
		 * of the read operation in spider_net_read_phy. Should take
		 * about 50 us */
		show_error = 0;
		break;
	case SPIDER_NET_GPWFFINT:
		/* PHY command queue full */
		if (netif_msg_intr(card))
			pr_err("PHY write queue full\n");
		show_error = 0;
		break;

	/* case SPIDER_NET_GRMDADRINT: not used. print a message */
	/* case SPIDER_NET_GRMARPINT: not used. print a message */
	/* case SPIDER_NET_GRMMPINT: not used. print a message */

	case SPIDER_NET_GDTDEN0INT:
		/* someone has set TX_DMA_EN to 0 */
		show_error = 0;
		break;

	case SPIDER_NET_GDDDEN0INT: /* fallthrough */
	case SPIDER_NET_GDCDEN0INT: /* fallthrough */
	case SPIDER_NET_GDBDEN0INT: /* fallthrough */
	case SPIDER_NET_GDADEN0INT:
		/* someone has set RX_DMA_EN to 0 */
		show_error = 0;
		break;

	/* RX interrupts */
	case SPIDER_NET_GDDFDCINT:
	case SPIDER_NET_GDCFDCINT:
	case SPIDER_NET_GDBFDCINT:
	case SPIDER_NET_GDAFDCINT:
	/* case SPIDER_NET_GDNMINT: not used. print a message */
	/* case SPIDER_NET_GCNMINT: not used. print a message */
	/* case SPIDER_NET_GBNMINT: not used. print a message */
	/* case SPIDER_NET_GANMINT: not used. print a message */
	/* case SPIDER_NET_GRFNMINT: not used. print a message */
		show_error = 0;
		break;

	/* TX interrupts */
	case SPIDER_NET_GDTFDCINT:
		show_error = 0;
		break;
	case SPIDER_NET_GTTEDINT:
		show_error = 0;
		break;
	case SPIDER_NET_GDTDCEINT:
		/* chain end. If a descriptor should be sent, kick off
		 * tx dma
		if (card->tx_chain.tail == card->tx_chain.head)
			spider_net_kick_tx_dma(card);
		show_error = 0; */
		break;

	/* case SPIDER_NET_G1TMCNTINT: not used. print a message */
	/* case SPIDER_NET_GFREECNTINT: not used. print a message */
	}

	/* check GHIINT1STS ************************************/
	if (error_reg1)
		for (i = 0; i < 32; i++)
			if (error_reg1 & (1<<i))
				switch (i)
	{
	case SPIDER_NET_GTMFLLINT:
		if (netif_msg_intr(card))
			pr_err("Spider TX RAM full\n");
		show_error = 0;
		break;
	case SPIDER_NET_GRMFLLINT:
		if (netif_msg_intr(card))
			pr_err("Spider RX RAM full, incoming packets "
			       "might be discarded !\n");
		netif_rx_schedule(card->netdev);
		spider_net_enable_rxchtails(card);
		spider_net_enable_rxdmac(card);
		break;

	/* case SPIDER_NET_GTMSHTINT: problem, print a message */
	case SPIDER_NET_GDTINVDINT:
		/* allrighty. tx from previous descr ok */
		show_error = 0;
		break;
	/* case SPIDER_NET_GRFDFLLINT: print a message down there */
	/* case SPIDER_NET_GRFCFLLINT: print a message down there */
	/* case SPIDER_NET_GRFBFLLINT: print a message down there */
	/* case SPIDER_NET_GRFAFLLINT: print a message down there */

	/* chain end */
	case SPIDER_NET_GDDDCEINT: /* fallthrough */
	case SPIDER_NET_GDCDCEINT: /* fallthrough */
	case SPIDER_NET_GDBDCEINT: /* fallthrough */
	case SPIDER_NET_GDADCEINT:
		if (netif_msg_intr(card))
			pr_err("got descriptor chain end interrupt, "
			       "restarting DMAC %c.\n",
			       'D'+i-SPIDER_NET_GDDDCEINT);
		spider_net_refill_rx_chain(card);
		show_error = 0;
		break;

	/* invalid descriptor */
	case SPIDER_NET_GDDINVDINT: /* fallthrough */
	case SPIDER_NET_GDCINVDINT: /* fallthrough */
	case SPIDER_NET_GDBINVDINT: /* fallthrough */
	case SPIDER_NET_GDAINVDINT:
		/* could happen when rx chain is full */
		spider_net_refill_rx_chain(card);
		show_error = 0;
		break;

	/* case SPIDER_NET_GDTRSERINT: problem, print a message */
	/* case SPIDER_NET_GDDRSERINT: problem, print a message */
	/* case SPIDER_NET_GDCRSERINT: problem, print a message */
	/* case SPIDER_NET_GDBRSERINT: problem, print a message */
	/* case SPIDER_NET_GDARSERINT: problem, print a message */
	/* case SPIDER_NET_GDSERINT: problem, print a message */
	/* case SPIDER_NET_GDTPTERINT: problem, print a message */
	/* case SPIDER_NET_GDDPTERINT: problem, print a message */
	/* case SPIDER_NET_GDCPTERINT: problem, print a message */
	/* case SPIDER_NET_GDBPTERINT: problem, print a message */
	/* case SPIDER_NET_GDAPTERINT: problem, print a message */
	default:
		show_error = 1;
		break;
	}

	/* check GHIINT2STS ************************************/
	if (error_reg2)
		for (i = 0; i < 32; i++)
			if (error_reg2 & (1<<i))
				switch (i)
	{
	/* there is nothing we can (want  to) do at this time. Log a
	 * message, we can switch on and off the specific values later on
	case SPIDER_NET_GPROPERINT:
	case SPIDER_NET_GMCTCRSNGINT:
	case SPIDER_NET_GMCTLCOLINT:
	case SPIDER_NET_GMCTTMOTINT:
	case SPIDER_NET_GMCRCAERINT:
	case SPIDER_NET_GMCRCALERINT:
	case SPIDER_NET_GMCRALNERINT:
	case SPIDER_NET_GMCROVRINT:
	case SPIDER_NET_GMCRRNTINT:
	case SPIDER_NET_GMCRRXERINT:
	case SPIDER_NET_GTITCSERINT:
	case SPIDER_NET_GTIFMTERINT:
	case SPIDER_NET_GTIPKTRVKINT:
	case SPIDER_NET_GTISPINGINT:
	case SPIDER_NET_GTISADNGINT:
	case SPIDER_NET_GTISPDNGINT:
	case SPIDER_NET_GRIFMTERINT:
	case SPIDER_NET_GRIPKTRVKINT:
	case SPIDER_NET_GRISPINGINT:
	case SPIDER_NET_GRISADNGINT:
	case SPIDER_NET_GRISPDNGINT:
		break;
	*/
		default:
			break;
	}

	if ((show_error) && (netif_msg_intr(card)))
		pr_err("Got error interrupt, GHIINT0STS = 0x%08x, "
		       "GHIINT1STS = 0x%08x, GHIINT2STS = 0x%08x\n",
		       status_reg, error_reg1, error_reg2);

	/* clear interrupt sources */
	spider_net_write_reg(card, SPIDER_NET_GHIINT1STS, error_reg1);
	spider_net_write_reg(card, SPIDER_NET_GHIINT2STS, error_reg2);
}

/**
 * spider_net_interrupt - interrupt handler for spider_net
 * @irq: interupt number
 * @ptr: pointer to net_device
 * @regs: PU registers
 *
 * returns IRQ_HANDLED, if interrupt was for driver, or IRQ_NONE, if no
 * interrupt found raised by card.
 *
 * This is the interrupt handler, that turns off
 * interrupts for this device and makes the stack poll the driver
 */
static irqreturn_t
spider_net_interrupt(int irq, void *ptr, struct pt_regs *regs)
{
	struct net_device *netdev = ptr;
	struct spider_net_card *card = netdev_priv(netdev);
	u32 status_reg;

	status_reg = spider_net_read_reg(card, SPIDER_NET_GHIINT0STS);

	if (!status_reg)
		return IRQ_NONE;

	if (status_reg & SPIDER_NET_TXINT)
		spider_net_release_tx_chain(card, 0);

	if (status_reg & SPIDER_NET_RXINT ) {
		spider_net_rx_irq_off(card);
		netif_rx_schedule(netdev);
	}

	/* we do this after rx and tx processing, as we want the tx chain
	 * processed to see, whether we should restart tx dma processing */
	spider_net_handle_error_irq(card, status_reg);

	/* clear interrupt sources */
	spider_net_write_reg(card, SPIDER_NET_GHIINT0STS, status_reg);

	return IRQ_HANDLED;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
/**
 * spider_net_poll_controller - artificial interrupt for netconsole etc.
 * @netdev: interface device structure
 *
 * see Documentation/networking/netconsole.txt
 */
static void
spider_net_poll_controller(struct net_device *netdev)
{
	disable_irq(netdev->irq);
	spider_net_interrupt(netdev->irq, netdev, NULL);
	enable_irq(netdev->irq);
}
#endif /* CONFIG_NET_POLL_CONTROLLER */

/**
 * spider_net_init_card - initializes the card
 * @card: card structure
 *
 * spider_net_init_card initializes the card so that other registers can
 * be used
 */
static void
spider_net_init_card(struct spider_net_card *card)
{
	spider_net_write_reg(card, SPIDER_NET_CKRCTRL,
			     SPIDER_NET_CKRCTRL_STOP_VALUE);

	spider_net_write_reg(card, SPIDER_NET_CKRCTRL,
			     SPIDER_NET_CKRCTRL_RUN_VALUE);
}

/**
 * spider_net_enable_card - enables the card by setting all kinds of regs
 * @card: card structure
 *
 * spider_net_enable_card sets a lot of SMMIO registers to enable the device
 */
static void
spider_net_enable_card(struct spider_net_card *card)
{
	int i;
	/* the following array consists of (register),(value) pairs
	 * that are set in this function. A register of 0 ends the list */
	u32 regs[][2] = {
		{ SPIDER_NET_GRESUMINTNUM, 0 },
		{ SPIDER_NET_GREINTNUM, 0 },

		/* set interrupt frame number registers */
		/* clear the single DMA engine registers first */
		{ SPIDER_NET_GFAFRMNUM, SPIDER_NET_GFXFRAMES_VALUE },
		{ SPIDER_NET_GFBFRMNUM, SPIDER_NET_GFXFRAMES_VALUE },
		{ SPIDER_NET_GFCFRMNUM, SPIDER_NET_GFXFRAMES_VALUE },
		{ SPIDER_NET_GFDFRMNUM, SPIDER_NET_GFXFRAMES_VALUE },
		/* then set, what we really need */
		{ SPIDER_NET_GFFRMNUM, SPIDER_NET_FRAMENUM_VALUE },

		/* timer counter registers and stuff */
		{ SPIDER_NET_GFREECNNUM, 0 },
		{ SPIDER_NET_GONETIMENUM, 0 },
		{ SPIDER_NET_GTOUTFRMNUM, 0 },

		/* RX mode setting */
		{ SPIDER_NET_GRXMDSET, SPIDER_NET_RXMODE_VALUE },
		/* TX mode setting */
		{ SPIDER_NET_GTXMDSET, SPIDER_NET_TXMODE_VALUE },
		/* IPSEC mode setting */
		{ SPIDER_NET_GIPSECINIT, SPIDER_NET_IPSECINIT_VALUE },

		{ SPIDER_NET_GFTRESTRT, SPIDER_NET_RESTART_VALUE },

		{ SPIDER_NET_GMRWOLCTRL, 0 },
		{ SPIDER_NET_GTESTMD, 0 },

		{ SPIDER_NET_GMACINTEN, 0 },

		/* flow control stuff */
		{ SPIDER_NET_GMACAPAUSE, SPIDER_NET_MACAPAUSE_VALUE },
		{ SPIDER_NET_GMACTXPAUSE, SPIDER_NET_TXPAUSE_VALUE },

		{ SPIDER_NET_GMACBSTLMT, SPIDER_NET_BURSTLMT_VALUE },
		{ 0, 0}
	};

	i = 0;
	while (regs[i][0]) {
		spider_net_write_reg(card, regs[i][0], regs[i][1]);
		i++;
	}

	/* clear unicast filter table entries 1 to 14 */
	for (i = 1; i <= 14; i++) {
		spider_net_write_reg(card,
				     SPIDER_NET_GMRUAFILnR + i * 8,
				     0x00080000);
		spider_net_write_reg(card,
				     SPIDER_NET_GMRUAFILnR + i * 8 + 4,
				     0x00000000);
	}

	spider_net_write_reg(card, SPIDER_NET_GMRUA0FIL15R, 0x08080000);

	spider_net_write_reg(card, SPIDER_NET_ECMODE, SPIDER_NET_ECMODE_VALUE);

	/* set chain tail adress for RX chains and
	 * enable DMA */
	spider_net_enable_rxchtails(card);
	spider_net_enable_rxdmac(card);

	spider_net_write_reg(card, SPIDER_NET_GRXDMAEN, SPIDER_NET_WOL_VALUE);

	/* set chain tail adress for TX chain */
	spider_net_enable_txdmac(card);

	spider_net_write_reg(card, SPIDER_NET_GMACLENLMT,
			     SPIDER_NET_LENLMT_VALUE);
	spider_net_write_reg(card, SPIDER_NET_GMACMODE,
			     SPIDER_NET_MACMODE_VALUE);
	spider_net_write_reg(card, SPIDER_NET_GMACOPEMD,
			     SPIDER_NET_OPMODE_VALUE);

	/* set interrupt mask registers */
	spider_net_write_reg(card, SPIDER_NET_GHIINT0MSK,
			     SPIDER_NET_INT0_MASK_VALUE);
	spider_net_write_reg(card, SPIDER_NET_GHIINT1MSK,
			     SPIDER_NET_INT1_MASK_VALUE);
	spider_net_write_reg(card, SPIDER_NET_GHIINT2MSK,
			     SPIDER_NET_INT2_MASK_VALUE);
}

/**
 * spider_net_open - called upon ifonfig up
 * @netdev: interface device structure
 *
 * returns 0 on success, <0 on failure
 *
 * spider_net_open allocates all the descriptors and memory needed for
 * operation, sets up multicast list and enables interrupts
 */
int
spider_net_open(struct net_device *netdev)
{
	struct spider_net_card *card = netdev_priv(netdev);
	int result;

	result = -ENOMEM;
	if (spider_net_init_chain(card, &card->tx_chain,
			  card->descr, tx_descriptors))
		goto alloc_tx_failed;
	if (spider_net_init_chain(card, &card->rx_chain,
			  card->descr + tx_descriptors, rx_descriptors))
		goto alloc_rx_failed;

	/* allocate rx skbs */
	if (spider_net_alloc_rx_skbs(card))
		goto alloc_skbs_failed;

	spider_net_set_multi(netdev);

	/* further enhancement: setup hw vlan, if needed */

	result = -EBUSY;
	if (request_irq(netdev->irq, spider_net_interrupt,
			     SA_SHIRQ, netdev->name, netdev))
		goto register_int_failed;

	spider_net_enable_card(card);

	netif_start_queue(netdev);
	netif_carrier_on(netdev);
	netif_poll_enable(netdev);

	return 0;

register_int_failed:
	spider_net_free_rx_chain_contents(card);
alloc_skbs_failed:
	spider_net_free_chain(card, &card->rx_chain);
alloc_rx_failed:
	spider_net_free_chain(card, &card->tx_chain);
alloc_tx_failed:
	return result;
}

/**
 * spider_net_setup_phy - setup PHY
 * @card: card structure
 *
 * returns 0 on success, <0 on failure
 *
 * spider_net_setup_phy is used as part of spider_net_probe. Sets
 * the PHY to 1000 Mbps
 **/
static int
spider_net_setup_phy(struct spider_net_card *card)
{
	struct mii_phy *phy = &card->phy;

	spider_net_write_reg(card, SPIDER_NET_GDTDMASEL,
			     SPIDER_NET_DMASEL_VALUE);
	spider_net_write_reg(card, SPIDER_NET_GPCCTRL,
			     SPIDER_NET_PHY_CTRL_VALUE);
	phy->mii_id = 1;
	phy->dev = card->netdev;
	phy->mdio_read = spider_net_read_phy;
	phy->mdio_write = spider_net_write_phy;

	mii_phy_probe(phy, phy->mii_id);

	if (phy->def->ops->setup_forced)
		phy->def->ops->setup_forced(phy, SPEED_1000, DUPLEX_FULL);

	/* the following two writes could be moved to sungem_phy.c */
	/* enable fiber mode */
	spider_net_write_phy(card->netdev, 1, MII_NCONFIG, 0x9020);
	/* LEDs active in both modes, autosense prio = fiber */
	spider_net_write_phy(card->netdev, 1, MII_NCONFIG, 0x945f);

	/* switch off fibre autoneg */
	spider_net_write_phy(card->netdev, 1, MII_NCONFIG, 0xfc01);
	spider_net_write_phy(card->netdev, 1, 0x0b, 0x0004);

	phy->def->ops->read_link(phy);
	pr_info("Found %s with %i Mbps, %s-duplex.\n", phy->def->name,
		phy->speed, phy->duplex==1 ? "Full" : "Half");

	return 0;
}

/**
 * spider_net_download_firmware - loads firmware into the adapter
 * @card: card structure
 * @firmware: firmware pointer
 *
 * spider_net_download_firmware loads the firmware opened by
 * spider_net_init_firmware into the adapter.
 */
static void
spider_net_download_firmware(struct spider_net_card *card,
			     const struct firmware *firmware)
{
	int sequencer, i;
	u32 *fw_ptr = (u32 *)firmware->data;

	/* stop sequencers */
	spider_net_write_reg(card, SPIDER_NET_GSINIT,
			     SPIDER_NET_STOP_SEQ_VALUE);

	for (sequencer = 0; sequencer < 6; sequencer++) {
		spider_net_write_reg(card,
				     SPIDER_NET_GSnPRGADR + sequencer * 8, 0);
		for (i = 0; i < SPIDER_NET_FIRMWARE_LEN; i++) {
			spider_net_write_reg(card, SPIDER_NET_GSnPRGDAT +
					     sequencer * 8, *fw_ptr);
			fw_ptr++;
		}
	}

	spider_net_write_reg(card, SPIDER_NET_GSINIT,
			     SPIDER_NET_RUN_SEQ_VALUE);
}

/**
 * spider_net_init_firmware - reads in firmware parts
 * @card: card structure
 *
 * Returns 0 on success, <0 on failure
 *
 * spider_net_init_firmware opens the sequencer firmware and does some basic
 * checks. This function opens and releases the firmware structure. A call
 * to download the firmware is performed before the release.
 *
 * Firmware format
 * ===============
 * spider_fw.bin is expected to be a file containing 6*1024*4 bytes, 4k being
 * the program for each sequencer. Use the command
 *    tail -q -n +2 Seq_code1_0x088.txt Seq_code2_0x090.txt              \
 *         Seq_code3_0x098.txt Seq_code4_0x0A0.txt Seq_code5_0x0A8.txt   \
 *         Seq_code6_0x0B0.txt | xxd -r -p -c4 > spider_fw.bin
 *
 * to generate spider_fw.bin, if you have sequencer programs with something
 * like the following contents for each sequencer:
 *    <ONE LINE COMMENT>
 *    <FIRST 4-BYTES-WORD FOR SEQUENCER>
 *    <SECOND 4-BYTES-WORD FOR SEQUENCER>
 *     ...
 *    <1024th 4-BYTES-WORD FOR SEQUENCER>
 */
static int
spider_net_init_firmware(struct spider_net_card *card)
{
	const struct firmware *firmware;
	int err = -EIO;

	if (request_firmware(&firmware,
			     SPIDER_NET_FIRMWARE_NAME, &card->pdev->dev) < 0) {
		if (netif_msg_probe(card))
			pr_err("Couldn't read in sequencer data file %s.\n",
			       SPIDER_NET_FIRMWARE_NAME);
		firmware = NULL;
		goto out;
	}

	if (firmware->size != 6 * SPIDER_NET_FIRMWARE_LEN * sizeof(u32)) {
		if (netif_msg_probe(card))
			pr_err("Invalid size of sequencer data file %s.\n",
			       SPIDER_NET_FIRMWARE_NAME);
		goto out;
	}

	spider_net_download_firmware(card, firmware);

	err = 0;
out:
	release_firmware(firmware);

	return err;
}

/**
 * spider_net_workaround_rxramfull - work around firmware bug
 * @card: card structure
 *
 * no return value
 **/
static void
spider_net_workaround_rxramfull(struct spider_net_card *card)
{
	int i, sequencer = 0;

	/* cancel reset */
	spider_net_write_reg(card, SPIDER_NET_CKRCTRL,
			     SPIDER_NET_CKRCTRL_RUN_VALUE);

	/* empty sequencer data */
	for (sequencer = 0; sequencer < 6; sequencer++) {
		spider_net_write_reg(card, SPIDER_NET_GSnPRGDAT +
				     sequencer * 8, 0x0);
		for (i = 0; i < SPIDER_NET_FIRMWARE_LEN; i++) {
			spider_net_write_reg(card, SPIDER_NET_GSnPRGDAT +
					     sequencer * 8, 0x0);
		}
	}

	/* set sequencer operation */
	spider_net_write_reg(card, SPIDER_NET_GSINIT, 0x000000fe);

	/* reset */
	spider_net_write_reg(card, SPIDER_NET_CKRCTRL,
			     SPIDER_NET_CKRCTRL_STOP_VALUE);
}

/**
 * spider_net_tx_timeout_task - task scheduled by the watchdog timeout
 * function (to be called not under interrupt status)
 * @data: data, is interface device structure
 *
 * called as task when tx hangs, resets interface (if interface is up)
 */
static void
spider_net_tx_timeout_task(void *data)
{
	struct net_device *netdev = data;
	struct spider_net_card *card = netdev_priv(netdev);

	if (!(netdev->flags & IFF_UP))
		goto out;

	netif_device_detach(netdev);
	spider_net_stop(netdev);

	spider_net_workaround_rxramfull(card);
	spider_net_init_card(card);

	if (spider_net_setup_phy(card))
		goto out;
	if (spider_net_init_firmware(card))
		goto out;

	spider_net_open(netdev);
	spider_net_kick_tx_dma(card, card->tx_chain.head);
	netif_device_attach(netdev);

out:
	atomic_dec(&card->tx_timeout_task_counter);
}

/**
 * spider_net_tx_timeout - called when the tx timeout watchdog kicks in.
 * @netdev: interface device structure
 *
 * called, if tx hangs. Schedules a task that resets the interface
 */
static void
spider_net_tx_timeout(struct net_device *netdev)
{
	struct spider_net_card *card;

	card = netdev_priv(netdev);
	atomic_inc(&card->tx_timeout_task_counter);
	if (netdev->flags & IFF_UP)
		schedule_work(&card->tx_timeout_task);
	else
		atomic_dec(&card->tx_timeout_task_counter);
}

/**
 * spider_net_setup_netdev_ops - initialization of net_device operations
 * @netdev: net_device structure
 *
 * fills out function pointers in the net_device structure
 */
static void
spider_net_setup_netdev_ops(struct net_device *netdev)
{
	netdev->open = &spider_net_open;
	netdev->stop = &spider_net_stop;
	netdev->hard_start_xmit = &spider_net_xmit;
	netdev->get_stats = &spider_net_get_stats;
	netdev->set_multicast_list = &spider_net_set_multi;
	netdev->set_mac_address = &spider_net_set_mac;
	netdev->change_mtu = &spider_net_change_mtu;
	netdev->do_ioctl = &spider_net_do_ioctl;
	/* tx watchdog */
	netdev->tx_timeout = &spider_net_tx_timeout;
	netdev->watchdog_timeo = SPIDER_NET_WATCHDOG_TIMEOUT;
	/* NAPI */
	netdev->poll = &spider_net_poll;
	netdev->weight = SPIDER_NET_NAPI_WEIGHT;
	/* HW VLAN */
	netdev->vlan_rx_register = &spider_net_vlan_rx_reg;
	netdev->vlan_rx_add_vid = &spider_net_vlan_rx_add;
	netdev->vlan_rx_kill_vid = &spider_net_vlan_rx_kill;
#ifdef CONFIG_NET_POLL_CONTROLLER
	/* poll controller */
	netdev->poll_controller = &spider_net_poll_controller;
#endif /* CONFIG_NET_POLL_CONTROLLER */
	/* ethtool ops */
	netdev->ethtool_ops = &spider_net_ethtool_ops;
}

/**
 * spider_net_setup_netdev - initialization of net_device
 * @card: card structure
 *
 * Returns 0 on success or <0 on failure
 *
 * spider_net_setup_netdev initializes the net_device structure
 **/
static int
spider_net_setup_netdev(struct spider_net_card *card)
{
	int result;
	struct net_device *netdev = card->netdev;
	struct device_node *dn;
	struct sockaddr addr;
	u8 *mac;

	SET_MODULE_OWNER(netdev);
	SET_NETDEV_DEV(netdev, &card->pdev->dev);

	pci_set_drvdata(card->pdev, netdev);
	spin_lock_init(&card->intmask_lock);
	netdev->irq = card->pdev->irq;

	card->options.rx_csum = SPIDER_NET_RX_CSUM_DEFAULT;

	spider_net_setup_netdev_ops(netdev);

	netdev->features = 0;
	/* some time: NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX |
	 *		NETIF_F_HW_VLAN_FILTER */

	netdev->irq = card->pdev->irq;

	dn = pci_device_to_OF_node(card->pdev);
	if (!dn)
		return -EIO;

	mac = (u8 *)get_property(dn, "local-mac-address", NULL);
	if (!mac)
		return -EIO;
	memcpy(addr.sa_data, mac, ETH_ALEN);

	result = spider_net_set_mac(netdev, &addr);
	if ((result) && (netif_msg_probe(card)))
		pr_err("Failed to set MAC address: %i\n", result);

	result = register_netdev(netdev);
	if (result) {
		if (netif_msg_probe(card))
			pr_err("Couldn't register net_device: %i\n",
				  result);
		return result;
	}

	if (netif_msg_probe(card))
		pr_info("Initialized device %s.\n", netdev->name);

	return 0;
}

/**
 * spider_net_alloc_card - allocates net_device and card structure
 *
 * returns the card structure or NULL in case of errors
 *
 * the card and net_device structures are linked to each other
 */
static struct spider_net_card *
spider_net_alloc_card(void)
{
	struct net_device *netdev;
	struct spider_net_card *card;
	size_t alloc_size;

	alloc_size = sizeof (*card) +
		sizeof (struct spider_net_descr) * rx_descriptors +
		sizeof (struct spider_net_descr) * tx_descriptors;
	netdev = alloc_etherdev(alloc_size);
	if (!netdev)
		return NULL;

	card = netdev_priv(netdev);
	card->netdev = netdev;
	card->msg_enable = SPIDER_NET_DEFAULT_MSG;
	INIT_WORK(&card->tx_timeout_task, spider_net_tx_timeout_task, netdev);
	init_waitqueue_head(&card->waitq);
	atomic_set(&card->tx_timeout_task_counter, 0);

	return card;
}

/**
 * spider_net_undo_pci_setup - releases PCI ressources
 * @card: card structure
 *
 * spider_net_undo_pci_setup releases the mapped regions
 */
static void
spider_net_undo_pci_setup(struct spider_net_card *card)
{
	iounmap(card->regs);
	pci_release_regions(card->pdev);
}

/**
 * spider_net_setup_pci_dev - sets up the device in terms of PCI operations
 * @card: card structure
 * @pdev: PCI device
 *
 * Returns the card structure or NULL if any errors occur
 *
 * spider_net_setup_pci_dev initializes pdev and together with the
 * functions called in spider_net_open configures the device so that
 * data can be transferred over it
 * The net_device structure is attached to the card structure, if the
 * function returns without error.
 **/
static struct spider_net_card *
spider_net_setup_pci_dev(struct pci_dev *pdev)
{
	struct spider_net_card *card;
	unsigned long mmio_start, mmio_len;

	if (pci_enable_device(pdev)) {
		pr_err("Couldn't enable PCI device\n");
		return NULL;
	}

	if (!(pci_resource_flags(pdev, 0) & IORESOURCE_MEM)) {
		pr_err("Couldn't find proper PCI device base address.\n");
		goto out_disable_dev;
	}

	if (pci_request_regions(pdev, spider_net_driver_name)) {
		pr_err("Couldn't obtain PCI resources, aborting.\n");
		goto out_disable_dev;
	}

	pci_set_master(pdev);

	card = spider_net_alloc_card();
	if (!card) {
		pr_err("Couldn't allocate net_device structure, "
			  "aborting.\n");
		goto out_release_regions;
	}
	card->pdev = pdev;

	/* fetch base address and length of first resource */
	mmio_start = pci_resource_start(pdev, 0);
	mmio_len = pci_resource_len(pdev, 0);

	card->netdev->mem_start = mmio_start;
	card->netdev->mem_end = mmio_start + mmio_len;
	card->regs = ioremap(mmio_start, mmio_len);

	if (!card->regs) {
		pr_err("Couldn't obtain PCI resources, aborting.\n");
		goto out_release_regions;
	}

	return card;

out_release_regions:
	pci_release_regions(pdev);
out_disable_dev:
	pci_disable_device(pdev);
	pci_set_drvdata(pdev, NULL);
	return NULL;
}

/**
 * spider_net_probe - initialization of a device
 * @pdev: PCI device
 * @ent: entry in the device id list
 *
 * Returns 0 on success, <0 on failure
 *
 * spider_net_probe initializes pdev and registers a net_device
 * structure for it. After that, the device can be ifconfig'ed up
 **/
static int __devinit
spider_net_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	int err = -EIO;
	struct spider_net_card *card;

	card = spider_net_setup_pci_dev(pdev);
	if (!card)
		goto out;

	spider_net_workaround_rxramfull(card);
	spider_net_init_card(card);

	err = spider_net_setup_phy(card);
	if (err)
		goto out_undo_pci;

	err = spider_net_init_firmware(card);
	if (err)
		goto out_undo_pci;

	err = spider_net_setup_netdev(card);
	if (err)
		goto out_undo_pci;

	return 0;

out_undo_pci:
	spider_net_undo_pci_setup(card);
	free_netdev(card->netdev);
out:
	return err;
}

/**
 * spider_net_remove - removal of a device
 * @pdev: PCI device
 *
 * Returns 0 on success, <0 on failure
 *
 * spider_net_remove is called to remove the device and unregisters the
 * net_device
 **/
static void __devexit
spider_net_remove(struct pci_dev *pdev)
{
	struct net_device *netdev;
	struct spider_net_card *card;

	netdev = pci_get_drvdata(pdev);
	card = netdev_priv(netdev);

	wait_event(card->waitq,
		   atomic_read(&card->tx_timeout_task_counter) == 0);

	unregister_netdev(netdev);

	/* switch off card */
	spider_net_write_reg(card, SPIDER_NET_CKRCTRL,
			     SPIDER_NET_CKRCTRL_STOP_VALUE);
	spider_net_write_reg(card, SPIDER_NET_CKRCTRL,
			     SPIDER_NET_CKRCTRL_RUN_VALUE);

	spider_net_undo_pci_setup(card);
	free_netdev(netdev);
}

static struct pci_driver spider_net_driver = {
	.owner		= THIS_MODULE,
	.name		= spider_net_driver_name,
	.id_table	= spider_net_pci_tbl,
	.probe		= spider_net_probe,
	.remove		= __devexit_p(spider_net_remove)
};

/**
 * spider_net_init - init function when the driver is loaded
 *
 * spider_net_init registers the device driver
 */
static int __init spider_net_init(void)
{
	if (rx_descriptors < SPIDER_NET_RX_DESCRIPTORS_MIN) {
		rx_descriptors = SPIDER_NET_RX_DESCRIPTORS_MIN;
		pr_info("adjusting rx descriptors to %i.\n", rx_descriptors);
	}
	if (rx_descriptors > SPIDER_NET_RX_DESCRIPTORS_MAX) {
		rx_descriptors = SPIDER_NET_RX_DESCRIPTORS_MAX;
		pr_info("adjusting rx descriptors to %i.\n", rx_descriptors);
	}
	if (tx_descriptors < SPIDER_NET_TX_DESCRIPTORS_MIN) {
		tx_descriptors = SPIDER_NET_TX_DESCRIPTORS_MIN;
		pr_info("adjusting tx descriptors to %i.\n", tx_descriptors);
	}
	if (tx_descriptors > SPIDER_NET_TX_DESCRIPTORS_MAX) {
		tx_descriptors = SPIDER_NET_TX_DESCRIPTORS_MAX;
		pr_info("adjusting tx descriptors to %i.\n", tx_descriptors);
	}

	return pci_register_driver(&spider_net_driver);
}

/**
 * spider_net_cleanup - exit function when driver is unloaded
 *
 * spider_net_cleanup unregisters the device driver
 */
static void __exit spider_net_cleanup(void)
{
	pci_unregister_driver(&spider_net_driver);
}

module_init(spider_net_init);
module_exit(spider_net_cleanup);
