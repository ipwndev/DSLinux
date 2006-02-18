/*****************************************************************************
 *                                                                           *
 * File: cxgb2.c                                                             *
 * $Revision$                                                         *
 * $Date$                                              *
 * Description:                                                              *
 *  Chelsio 10Gb Ethernet Driver.                                            *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License, version 2, as       *
 * published by the Free Software Foundation.                                *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with this program; if not, write to the Free Software Foundation, Inc.,   *
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED    *
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF      *
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.                     *
 *                                                                           *
 * http://www.chelsio.com                                                    *
 *                                                                           *
 * Copyright (c) 2003 - 2005 Chelsio Communications, Inc.                    *
 * All rights reserved.                                                      *
 *                                                                           *
 * Maintainers: maintainers@chelsio.com                                      *
 *                                                                           *
 * Authors: Dimitrios Michailidis   <dm@chelsio.com>                         *
 *          Tina Yang               <tainay@chelsio.com>                     *
 *          Felix Marti             <felix@chelsio.com>                      *
 *          Scott Bardone           <sbardone@chelsio.com>                   *
 *          Kurt Ottaway            <kottaway@chelsio.com>                   *
 *          Frank DiMambro          <frank@chelsio.com>                      *
 *                                                                           *
 * History:                                                                  *
 *                                                                           *
 ****************************************************************************/

#include "common.h"
#include <linux/config.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <asm/uaccess.h>

#include "cpl5_cmd.h"
#include "regs.h"
#include "gmac.h"
#include "cphy.h"
#include "sge.h"
#include "espi.h"

#ifdef work_struct
#include <linux/tqueue.h>
#define INIT_WORK INIT_TQUEUE
#define schedule_work schedule_task
#define flush_scheduled_work flush_scheduled_tasks

static inline void schedule_mac_stats_update(struct adapter *ap, int secs)
{
	mod_timer(&ap->stats_update_timer, jiffies + secs * HZ);
}

static inline void cancel_mac_stats_update(struct adapter *ap)
{
	del_timer_sync(&ap->stats_update_timer);
	flush_scheduled_tasks();
}

/*
 * Stats update timer for 2.4.  It schedules a task to do the actual update as
 * we need to access MAC statistics in process context.
 */
static void mac_stats_timer(unsigned long data)
{
	struct adapter *ap = (struct adapter *)data;

	schedule_task(&ap->stats_update_task);
}
#else
#include <linux/workqueue.h>

static inline void schedule_mac_stats_update(struct adapter *ap, int secs)
{
	schedule_delayed_work(&ap->stats_update_task, secs * HZ);
}

static inline void cancel_mac_stats_update(struct adapter *ap)
{
	cancel_delayed_work(&ap->stats_update_task);
}
#endif

#define MAX_CMDQ_ENTRIES 16384
#define MAX_CMDQ1_ENTRIES 1024
#define MAX_RX_BUFFERS 16384
#define MAX_RX_JUMBO_BUFFERS 16384
#define MAX_TX_BUFFERS_HIGH	16384U
#define MAX_TX_BUFFERS_LOW	1536U
#define MIN_FL_ENTRIES 32

#define PORT_MASK ((1 << MAX_NPORTS) - 1)

#define DFLT_MSG_ENABLE (NETIF_MSG_DRV | NETIF_MSG_PROBE | NETIF_MSG_LINK | \
			 NETIF_MSG_TIMER | NETIF_MSG_IFDOWN | NETIF_MSG_IFUP |\
			 NETIF_MSG_RX_ERR | NETIF_MSG_TX_ERR)

/*
 * The EEPROM is actually bigger but only the first few bytes are used so we
 * only report those.
 */
#define EEPROM_SIZE 32

MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR("Chelsio Communications");
MODULE_LICENSE("GPL");

static int dflt_msg_enable = DFLT_MSG_ENABLE;

MODULE_PARM(dflt_msg_enable, "i");
MODULE_PARM_DESC(dflt_msg_enable, "Chelsio T1 message enable bitmap");


static const char pci_speed[][4] = {
	"33", "66", "100", "133"
};

/*
 * Setup MAC to receive the types of packets we want.
 */
static void t1_set_rxmode(struct net_device *dev)
{
	struct adapter *adapter = dev->priv;
	struct cmac *mac = adapter->port[dev->if_port].mac;
	struct t1_rx_mode rm;

	rm.dev = dev;
	rm.idx = 0;
	rm.list = dev->mc_list;
	mac->ops->set_rx_mode(mac, &rm);
}

static void link_report(struct port_info *p)
{
	if (!netif_carrier_ok(p->dev))
        	printk(KERN_INFO "%s: link down\n", p->dev->name);
	else {
		const char *s = "10Mbps";

		switch (p->link_config.speed) {
			case SPEED_10000: s = "10Gbps"; break;
			case SPEED_1000:  s = "1000Mbps"; break;
			case SPEED_100:   s = "100Mbps"; break;
		}

        printk(KERN_INFO "%s: link up, %s, %s-duplex\n",
		       p->dev->name, s,
		       p->link_config.duplex == DUPLEX_FULL ? "full" : "half");
	}
}

void t1_link_changed(struct adapter *adapter, int port_id, int link_stat,
			int speed, int duplex, int pause)
{
	struct port_info *p = &adapter->port[port_id];

	if (link_stat != netif_carrier_ok(p->dev)) {
		if (link_stat)
			netif_carrier_on(p->dev);
		else
			netif_carrier_off(p->dev);
		link_report(p);

	}
}

static void link_start(struct port_info *p)
{
	struct cmac *mac = p->mac;

	mac->ops->reset(mac);
	if (mac->ops->macaddress_set)
		mac->ops->macaddress_set(mac, p->dev->dev_addr);
	t1_set_rxmode(p->dev);
	t1_link_start(p->phy, mac, &p->link_config);
	mac->ops->enable(mac, MAC_DIRECTION_RX | MAC_DIRECTION_TX);
}

static void enable_hw_csum(struct adapter *adapter)
{
	if (adapter->flags & TSO_CAPABLE)
		t1_tp_set_ip_checksum_offload(adapter, 1); /* for TSO only */
	t1_tp_set_tcp_checksum_offload(adapter, 1);
}

/*
 * Things to do upon first use of a card.
 * This must run with the rtnl lock held.
 */
static int cxgb_up(struct adapter *adapter)
{
	int err = 0;

	if (!(adapter->flags & FULL_INIT_DONE)) {
		err = t1_init_hw_modules(adapter);
		if (err)
			goto out_err;

		enable_hw_csum(adapter);
		adapter->flags |= FULL_INIT_DONE;
	}

	t1_interrupts_clear(adapter);
	if ((err = request_irq(adapter->pdev->irq,
			       t1_select_intr_handler(adapter), SA_SHIRQ,
			       adapter->name, adapter))) {
		goto out_err;
	}
	t1_sge_start(adapter->sge);
	t1_interrupts_enable(adapter);
 out_err:
	return err;
}

/*
 * Release resources when all the ports have been stopped.
 */
static void cxgb_down(struct adapter *adapter)
{
	t1_sge_stop(adapter->sge);
	t1_interrupts_disable(adapter);
	free_irq(adapter->pdev->irq, adapter);
}

static int cxgb_open(struct net_device *dev)
{
	int err;
	struct adapter *adapter = dev->priv;
	int other_ports = adapter->open_device_map & PORT_MASK;

	if (!adapter->open_device_map && (err = cxgb_up(adapter)) < 0)
		return err;

	__set_bit(dev->if_port, &adapter->open_device_map);
	link_start(&adapter->port[dev->if_port]);
	netif_start_queue(dev);
	if (!other_ports && adapter->params.stats_update_period)
		schedule_mac_stats_update(adapter,
					  adapter->params.stats_update_period);
	return 0;
}

static int cxgb_close(struct net_device *dev)
{
	struct adapter *adapter = dev->priv;
	struct port_info *p = &adapter->port[dev->if_port];
	struct cmac *mac = p->mac;

	netif_stop_queue(dev);
	mac->ops->disable(mac, MAC_DIRECTION_TX | MAC_DIRECTION_RX);
	netif_carrier_off(dev);

	clear_bit(dev->if_port, &adapter->open_device_map);
	if (adapter->params.stats_update_period &&
	    !(adapter->open_device_map & PORT_MASK)) {
		/* Stop statistics accumulation. */
		smp_mb__after_clear_bit();
		spin_lock(&adapter->work_lock);   /* sync with update task */
		spin_unlock(&adapter->work_lock);
		cancel_mac_stats_update(adapter);
	}

	if (!adapter->open_device_map)
		cxgb_down(adapter);
	return 0;
}

static struct net_device_stats *t1_get_stats(struct net_device *dev)
{
	struct adapter *adapter = dev->priv;
	struct port_info *p = &adapter->port[dev->if_port];
	struct net_device_stats *ns = &p->netstats;
	const struct cmac_statistics *pstats;

	/* Do a full update of the MAC stats */
	pstats = p->mac->ops->statistics_update(p->mac,
						      MAC_STATS_UPDATE_FULL);

	ns->tx_packets = pstats->TxUnicastFramesOK +
		pstats->TxMulticastFramesOK + pstats->TxBroadcastFramesOK;

	ns->rx_packets = pstats->RxUnicastFramesOK +
		pstats->RxMulticastFramesOK + pstats->RxBroadcastFramesOK;

	ns->tx_bytes = pstats->TxOctetsOK;
	ns->rx_bytes = pstats->RxOctetsOK;

	ns->tx_errors = pstats->TxLateCollisions + pstats->TxLengthErrors +
		pstats->TxUnderrun + pstats->TxFramesAbortedDueToXSCollisions;
	ns->rx_errors = pstats->RxDataErrors + pstats->RxJabberErrors +
		pstats->RxFCSErrors + pstats->RxAlignErrors +
		pstats->RxSequenceErrors + pstats->RxFrameTooLongErrors +
		pstats->RxSymbolErrors + pstats->RxRuntErrors;

	ns->multicast  = pstats->RxMulticastFramesOK;
	ns->collisions = pstats->TxTotalCollisions;

	/* detailed rx_errors */
	ns->rx_length_errors = pstats->RxFrameTooLongErrors +
		pstats->RxJabberErrors;
	ns->rx_over_errors   = 0;
	ns->rx_crc_errors    = pstats->RxFCSErrors;
	ns->rx_frame_errors  = pstats->RxAlignErrors;
	ns->rx_fifo_errors   = 0;
	ns->rx_missed_errors = 0;

	/* detailed tx_errors */
	ns->tx_aborted_errors   = pstats->TxFramesAbortedDueToXSCollisions;
	ns->tx_carrier_errors   = 0;
	ns->tx_fifo_errors      = pstats->TxUnderrun;
	ns->tx_heartbeat_errors = 0;
	ns->tx_window_errors    = pstats->TxLateCollisions;
	return ns;
}

static u32 get_msglevel(struct net_device *dev)
{
	struct adapter *adapter = dev->priv;

	return adapter->msg_enable;
}

static void set_msglevel(struct net_device *dev, u32 val)
{
	struct adapter *adapter = dev->priv;

	adapter->msg_enable = val;
}

static char stats_strings[][ETH_GSTRING_LEN] = {
        "TxOctetsOK",
        "TxOctetsBad",
        "TxUnicastFramesOK",
        "TxMulticastFramesOK",
        "TxBroadcastFramesOK",
        "TxPauseFrames",
        "TxFramesWithDeferredXmissions",
        "TxLateCollisions",
        "TxTotalCollisions",
        "TxFramesAbortedDueToXSCollisions",
        "TxUnderrun",
        "TxLengthErrors",
        "TxInternalMACXmitError",
        "TxFramesWithExcessiveDeferral",
        "TxFCSErrors",

        "RxOctetsOK",
        "RxOctetsBad",
        "RxUnicastFramesOK",
        "RxMulticastFramesOK",
        "RxBroadcastFramesOK",
        "RxPauseFrames",
        "RxFCSErrors",
        "RxAlignErrors",
        "RxSymbolErrors",
        "RxDataErrors",
        "RxSequenceErrors",
        "RxRuntErrors",
        "RxJabberErrors",
        "RxInternalMACRcvError",
        "RxInRangeLengthErrors",
        "RxOutOfRangeLengthField",
        "RxFrameTooLongErrors",

	"TSO",
	"VLANextractions",
	"VLANinsertions",
	"RxCsumGood",
	"TxCsumOffload",
	"RxDrops"

	"respQ_empty",
	"respQ_overflow",
	"freelistQ_empty",
	"pkt_too_big",
	"pkt_mismatch",
	"cmdQ_full0",
	"cmdQ_full1",
	"tx_ipfrags",
	"tx_reg_pkts",
	"tx_lso_pkts",
	"tx_do_cksum",
	
	"espi_DIP2ParityErr",
	"espi_DIP4Err",
	"espi_RxDrops",
	"espi_TxDrops",
	"espi_RxOvfl",
	"espi_ParityErr"
};
 
#define T2_REGMAP_SIZE (3 * 1024)

static int get_regs_len(struct net_device *dev)
{
	return T2_REGMAP_SIZE;
}

static void get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct adapter *adapter = dev->priv;

	strcpy(info->driver, DRV_NAME);
	strcpy(info->version, DRV_VERSION);
	strcpy(info->fw_version, "N/A");
	strcpy(info->bus_info, pci_name(adapter->pdev));
}

static int get_stats_count(struct net_device *dev)
{
	return ARRAY_SIZE(stats_strings);
}

static void get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	if (stringset == ETH_SS_STATS)
		memcpy(data, stats_strings, sizeof(stats_strings));
}

static void get_stats(struct net_device *dev, struct ethtool_stats *stats,
		      u64 *data)
{
	struct adapter *adapter = dev->priv;
	struct cmac *mac = adapter->port[dev->if_port].mac;
	const struct cmac_statistics *s;
	const struct sge_port_stats *ss;
	const struct sge_intr_counts *t;

	s = mac->ops->statistics_update(mac, MAC_STATS_UPDATE_FULL);
	ss = t1_sge_get_port_stats(adapter->sge, dev->if_port);
	t = t1_sge_get_intr_counts(adapter->sge);

        *data++ = s->TxOctetsOK;
        *data++ = s->TxOctetsBad;
        *data++ = s->TxUnicastFramesOK;
        *data++ = s->TxMulticastFramesOK;
        *data++ = s->TxBroadcastFramesOK;
        *data++ = s->TxPauseFrames;
        *data++ = s->TxFramesWithDeferredXmissions;
        *data++ = s->TxLateCollisions;
        *data++ = s->TxTotalCollisions;
        *data++ = s->TxFramesAbortedDueToXSCollisions;
        *data++ = s->TxUnderrun;
        *data++ = s->TxLengthErrors;
        *data++ = s->TxInternalMACXmitError;
        *data++ = s->TxFramesWithExcessiveDeferral;
        *data++ = s->TxFCSErrors;

        *data++ = s->RxOctetsOK;
        *data++ = s->RxOctetsBad;
        *data++ = s->RxUnicastFramesOK;
        *data++ = s->RxMulticastFramesOK;
        *data++ = s->RxBroadcastFramesOK;
        *data++ = s->RxPauseFrames;
        *data++ = s->RxFCSErrors;
        *data++ = s->RxAlignErrors;
        *data++ = s->RxSymbolErrors;
        *data++ = s->RxDataErrors;
        *data++ = s->RxSequenceErrors;
        *data++ = s->RxRuntErrors;
        *data++ = s->RxJabberErrors;
        *data++ = s->RxInternalMACRcvError;
        *data++ = s->RxInRangeLengthErrors;
        *data++ = s->RxOutOfRangeLengthField;
        *data++ = s->RxFrameTooLongErrors;

	*data++ = ss->tso;
	*data++ = ss->vlan_xtract;
	*data++ = ss->vlan_insert;
	*data++ = ss->rx_cso_good;
	*data++ = ss->tx_cso;
	*data++ = ss->rx_drops;

	*data++ = (u64)t->respQ_empty;
	*data++ = (u64)t->respQ_overflow;
	*data++ = (u64)t->freelistQ_empty;
	*data++ = (u64)t->pkt_too_big;
	*data++ = (u64)t->pkt_mismatch;
	*data++ = (u64)t->cmdQ_full[0];
	*data++ = (u64)t->cmdQ_full[1];
	*data++ = (u64)t->tx_ipfrags;
	*data++ = (u64)t->tx_reg_pkts;
	*data++ = (u64)t->tx_lso_pkts;
	*data++ = (u64)t->tx_do_cksum;
}

static inline void reg_block_dump(struct adapter *ap, void *buf,
				  unsigned int start, unsigned int end)
{
	u32 *p = buf + start;

	for ( ; start <= end; start += sizeof(u32))
		*p++ = readl(ap->regs + start);
}

static void get_regs(struct net_device *dev, struct ethtool_regs *regs,
		     void *buf)
{
	struct adapter *ap = dev->priv;

	/*
	 * Version scheme: bits 0..9: chip version, bits 10..15: chip revision
	 */
	regs->version = 2;

	memset(buf, 0, T2_REGMAP_SIZE);
	reg_block_dump(ap, buf, 0, A_SG_RESPACCUTIMER);
}

static int get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct adapter *adapter = dev->priv;
	struct port_info *p = &adapter->port[dev->if_port];

	cmd->supported = p->link_config.supported;
	cmd->advertising = p->link_config.advertising;

	if (netif_carrier_ok(dev)) {
		cmd->speed = p->link_config.speed;
		cmd->duplex = p->link_config.duplex;
	} else {
		cmd->speed = -1;
		cmd->duplex = -1;
	}

        cmd->port = (cmd->supported & SUPPORTED_TP) ? PORT_TP : PORT_FIBRE;
        cmd->phy_address = p->phy->addr;
        cmd->transceiver = XCVR_EXTERNAL;
        cmd->autoneg = p->link_config.autoneg;
        cmd->maxtxpkt = 0;
        cmd->maxrxpkt = 0;
	return 0;
}

static int speed_duplex_to_caps(int speed, int duplex)
{
	int cap = 0;

	switch (speed) {
	case SPEED_10:
		if (duplex == DUPLEX_FULL)
			cap = SUPPORTED_10baseT_Full;
		else
			cap = SUPPORTED_10baseT_Half;
		break;
	case SPEED_100:
		if (duplex == DUPLEX_FULL)
			cap = SUPPORTED_100baseT_Full;
		else
			cap = SUPPORTED_100baseT_Half;
		break;
	case SPEED_1000:
		if (duplex == DUPLEX_FULL)
			cap = SUPPORTED_1000baseT_Full;
		else
			cap = SUPPORTED_1000baseT_Half;
		break;
	case SPEED_10000:
		if (duplex == DUPLEX_FULL)
			cap = SUPPORTED_10000baseT_Full;
	}
	return cap;
}

#define ADVERTISED_MASK (ADVERTISED_10baseT_Half | ADVERTISED_10baseT_Full | \
		      ADVERTISED_100baseT_Half | ADVERTISED_100baseT_Full | \
		      ADVERTISED_1000baseT_Half | ADVERTISED_1000baseT_Full | \
		      ADVERTISED_10000baseT_Full)

static int set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct adapter *adapter = dev->priv;
	struct port_info *p = &adapter->port[dev->if_port];
	struct link_config *lc = &p->link_config;

	if (!(lc->supported & SUPPORTED_Autoneg))
		return -EOPNOTSUPP;             /* can't change speed/duplex */

	if (cmd->autoneg == AUTONEG_DISABLE) {
		int cap = speed_duplex_to_caps(cmd->speed, cmd->duplex);

		if (!(lc->supported & cap) || cmd->speed == SPEED_1000)
			return -EINVAL;
		lc->requested_speed = cmd->speed;
		lc->requested_duplex = cmd->duplex;
		lc->advertising = 0;
	} else {
		cmd->advertising &= ADVERTISED_MASK;
		if (cmd->advertising & (cmd->advertising - 1))
			cmd->advertising = lc->supported;
		cmd->advertising &= lc->supported;
		if (!cmd->advertising)
			return -EINVAL;
		lc->requested_speed = SPEED_INVALID;
		lc->requested_duplex = DUPLEX_INVALID;
		lc->advertising = cmd->advertising | ADVERTISED_Autoneg;
	}
	lc->autoneg = cmd->autoneg;
	if (netif_running(dev))
		t1_link_start(p->phy, p->mac, lc);
	return 0;
}

static void get_pauseparam(struct net_device *dev,
			   struct ethtool_pauseparam *epause)
{
	struct adapter *adapter = dev->priv;
	struct port_info *p = &adapter->port[dev->if_port];

	epause->autoneg = (p->link_config.requested_fc & PAUSE_AUTONEG) != 0;
	epause->rx_pause = (p->link_config.fc & PAUSE_RX) != 0;
	epause->tx_pause = (p->link_config.fc & PAUSE_TX) != 0;
}

static int set_pauseparam(struct net_device *dev,
			  struct ethtool_pauseparam *epause)
{
	struct adapter *adapter = dev->priv;
	struct port_info *p = &adapter->port[dev->if_port];
	struct link_config *lc = &p->link_config;

	if (epause->autoneg == AUTONEG_DISABLE)
		lc->requested_fc = 0;
	else if (lc->supported & SUPPORTED_Autoneg)
		lc->requested_fc = PAUSE_AUTONEG;
	else
		return -EINVAL;

	if (epause->rx_pause)
		lc->requested_fc |= PAUSE_RX;
	if (epause->tx_pause)
		lc->requested_fc |= PAUSE_TX;
	if (lc->autoneg == AUTONEG_ENABLE) {
		if (netif_running(dev))
			t1_link_start(p->phy, p->mac, lc);
	} else {
		lc->fc = lc->requested_fc & (PAUSE_RX | PAUSE_TX);
		if (netif_running(dev))
			p->mac->ops->set_speed_duplex_fc(p->mac, -1, -1,
							 lc->fc);
	}
	return 0;
}

static u32 get_rx_csum(struct net_device *dev)
{
	struct adapter *adapter = dev->priv;

	return (adapter->flags & RX_CSUM_ENABLED) != 0;
}

static int set_rx_csum(struct net_device *dev, u32 data)
{
	struct adapter *adapter = dev->priv;

	if (data)
		adapter->flags |= RX_CSUM_ENABLED;
	else
		adapter->flags &= ~RX_CSUM_ENABLED;
	return 0;
}

static int set_tso(struct net_device *dev, u32 value)
{
	struct adapter *adapter = dev->priv;

	if (!(adapter->flags & TSO_CAPABLE))
		return value ? -EOPNOTSUPP : 0;
	return ethtool_op_set_tso(dev, value);
}

static void get_sge_param(struct net_device *dev, struct ethtool_ringparam *e)
{
	struct adapter *adapter = dev->priv;
	int jumbo_fl = t1_is_T1B(adapter) ? 1 : 0;

	e->rx_max_pending = MAX_RX_BUFFERS;
	e->rx_mini_max_pending = 0;
	e->rx_jumbo_max_pending = MAX_RX_JUMBO_BUFFERS;
	e->tx_max_pending = MAX_CMDQ_ENTRIES;

	e->rx_pending = adapter->params.sge.freelQ_size[!jumbo_fl];
	e->rx_mini_pending = 0;
	e->rx_jumbo_pending = adapter->params.sge.freelQ_size[jumbo_fl];
	e->tx_pending = adapter->params.sge.cmdQ_size[0];
}

static int set_sge_param(struct net_device *dev, struct ethtool_ringparam *e)
{
	struct adapter *adapter = dev->priv;
	int jumbo_fl = t1_is_T1B(adapter) ? 1 : 0;

	if (e->rx_pending > MAX_RX_BUFFERS || e->rx_mini_pending ||
	    e->rx_jumbo_pending > MAX_RX_JUMBO_BUFFERS ||
	    e->tx_pending > MAX_CMDQ_ENTRIES ||
	    e->rx_pending < MIN_FL_ENTRIES ||
	    e->rx_jumbo_pending < MIN_FL_ENTRIES ||
	    e->tx_pending < (adapter->params.nports + 1) * (MAX_SKB_FRAGS + 1))
		return -EINVAL;

	if (adapter->flags & FULL_INIT_DONE)
        return -EBUSY;

	adapter->params.sge.freelQ_size[!jumbo_fl] = e->rx_pending;
	adapter->params.sge.freelQ_size[jumbo_fl] = e->rx_jumbo_pending;
	adapter->params.sge.cmdQ_size[0] = e->tx_pending;
	adapter->params.sge.cmdQ_size[1] = e->tx_pending > MAX_CMDQ1_ENTRIES ?
		MAX_CMDQ1_ENTRIES : e->tx_pending;
	return 0;
}

static int set_coalesce(struct net_device *dev, struct ethtool_coalesce *c)
{
	struct adapter *adapter = dev->priv;

	/*
	 * If RX coalescing is requested we use NAPI, otherwise interrupts.
	 * This choice can be made only when all ports and the TOE are off.
	 */
	if (adapter->open_device_map == 0)
		adapter->params.sge.polling = c->use_adaptive_rx_coalesce;

	if (adapter->params.sge.polling) {
		adapter->params.sge.rx_coalesce_usecs = 0;
	} else {
		adapter->params.sge.rx_coalesce_usecs = c->rx_coalesce_usecs;
	}
 	adapter->params.sge.coalesce_enable = c->use_adaptive_rx_coalesce;
	adapter->params.sge.sample_interval_usecs = c->rate_sample_interval;
	t1_sge_set_coalesce_params(adapter->sge, &adapter->params.sge);
	return 0;
}

static int get_coalesce(struct net_device *dev, struct ethtool_coalesce *c)
{
	struct adapter *adapter = dev->priv;

	c->rx_coalesce_usecs = adapter->params.sge.rx_coalesce_usecs;
	c->rate_sample_interval = adapter->params.sge.sample_interval_usecs;
	c->use_adaptive_rx_coalesce = adapter->params.sge.coalesce_enable;
	return 0;
}

static int get_eeprom_len(struct net_device *dev)
{
    return EEPROM_SIZE;
}

#define EEPROM_MAGIC(ap) \
	(PCI_VENDOR_ID_CHELSIO | ((ap)->params.chip_version << 16))

static int get_eeprom(struct net_device *dev, struct ethtool_eeprom *e,
		      u8 *data)
{
	int i;
	u8 buf[EEPROM_SIZE] __attribute__((aligned(4)));
	struct adapter *adapter = dev->priv;

	e->magic = EEPROM_MAGIC(adapter);
	for (i = e->offset & ~3; i < e->offset + e->len; i += sizeof(u32))
		t1_seeprom_read(adapter, i, (u32 *)&buf[i]);
	memcpy(data, buf + e->offset, e->len);
	return 0;
}

static struct ethtool_ops t1_ethtool_ops = {
	.get_settings      = get_settings,
	.set_settings      = set_settings,
	.get_drvinfo       = get_drvinfo,
	.get_msglevel      = get_msglevel,
	.set_msglevel      = set_msglevel,
	.get_ringparam     = get_sge_param,
	.set_ringparam     = set_sge_param,
	.get_coalesce      = get_coalesce,
	.set_coalesce      = set_coalesce,
	.get_eeprom_len    = get_eeprom_len,
	.get_eeprom        = get_eeprom,
	.get_pauseparam    = get_pauseparam,
	.set_pauseparam    = set_pauseparam,
	.get_rx_csum       = get_rx_csum,
	.set_rx_csum       = set_rx_csum,
	.get_tx_csum       = ethtool_op_get_tx_csum,
	.set_tx_csum       = ethtool_op_set_tx_csum,
	.get_sg            = ethtool_op_get_sg,
	.set_sg            = ethtool_op_set_sg,
	.get_link          = ethtool_op_get_link,
	.get_strings       = get_strings,
	.get_stats_count   = get_stats_count,
	.get_ethtool_stats = get_stats,
	.get_regs_len      = get_regs_len,
	.get_regs          = get_regs,
	.get_tso           = ethtool_op_get_tso,
	.set_tso           = set_tso,
};

static void cxgb_proc_cleanup(struct adapter *adapter,
					struct proc_dir_entry *dir)
{
	const char *name;
	name = adapter->name;
	remove_proc_entry(name, dir);
}
//#define chtoe_setup_toedev(adapter) NULL
#define update_mtu_tab(adapter)
#define write_smt_entry(adapter, idx)

static int t1_ioctl(struct net_device *dev, struct ifreq *req, int cmd)
{
        struct adapter *adapter = dev->priv;
        struct mii_ioctl_data *data = if_mii(req);

	switch (cmd) {
        case SIOCGMIIPHY:
                data->phy_id = adapter->port[dev->if_port].phy->addr;
                /* FALLTHRU */
        case SIOCGMIIREG: {
		struct cphy *phy = adapter->port[dev->if_port].phy;
		u32 val;

		if (!phy->mdio_read)
            return -EOPNOTSUPP;
		phy->mdio_read(adapter, data->phy_id, 0, data->reg_num & 0x1f,
			       &val);
                data->val_out = val;
                break;
	}
        case SIOCSMIIREG: {
		struct cphy *phy = adapter->port[dev->if_port].phy;

                if (!capable(CAP_NET_ADMIN))
                    return -EPERM;
		if (!phy->mdio_write)
            return -EOPNOTSUPP;
		phy->mdio_write(adapter, data->phy_id, 0, data->reg_num & 0x1f,
			        data->val_in);
                break;
	}

	default:
		return -EOPNOTSUPP;
	}
	return 0;
}

static int t1_change_mtu(struct net_device *dev, int new_mtu)
{
	int ret;
	struct adapter *adapter = dev->priv;
	struct cmac *mac = adapter->port[dev->if_port].mac;

	if (!mac->ops->set_mtu)
        return -EOPNOTSUPP;
	if (new_mtu < 68)
        return -EINVAL;
	if ((ret = mac->ops->set_mtu(mac, new_mtu)))
		return ret;
	dev->mtu = new_mtu;
	return 0;
}

static int t1_set_mac_addr(struct net_device *dev, void *p)
{
	struct adapter *adapter = dev->priv;
	struct cmac *mac = adapter->port[dev->if_port].mac;
	struct sockaddr *addr = p;

	if (!mac->ops->macaddress_set)
		return -EOPNOTSUPP;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
	mac->ops->macaddress_set(mac, dev->dev_addr);
	return 0;
}

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
static void vlan_rx_register(struct net_device *dev,
				   struct vlan_group *grp)
{
	struct adapter *adapter = dev->priv;

	spin_lock_irq(&adapter->async_lock);
	adapter->vlan_grp = grp;
	t1_set_vlan_accel(adapter, grp != NULL);
	spin_unlock_irq(&adapter->async_lock);
}

static void vlan_rx_kill_vid(struct net_device *dev, unsigned short vid)
{
	struct adapter *adapter = dev->priv;

	spin_lock_irq(&adapter->async_lock);
	if (adapter->vlan_grp)
		adapter->vlan_grp->vlan_devices[vid] = NULL;
	spin_unlock_irq(&adapter->async_lock);
}
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
static void t1_netpoll(struct net_device *dev)
{
	unsigned long flags;
	struct adapter *adapter = dev->priv;

	local_irq_save(flags);
        t1_select_intr_handler(adapter)(adapter->pdev->irq, adapter, NULL);
	local_irq_restore(flags);
}
#endif

/*
 * Periodic accumulation of MAC statistics.  This is used only if the MAC
 * does not have any other way to prevent stats counter overflow.
 */
static void mac_stats_task(void *data)
{
	int i;
	struct adapter *adapter = data;

	for_each_port(adapter, i) {
		struct port_info *p = &adapter->port[i];

		if (netif_running(p->dev))
			p->mac->ops->statistics_update(p->mac,
						       MAC_STATS_UPDATE_FAST);
	}

	/* Schedule the next statistics update if any port is active. */
	spin_lock(&adapter->work_lock);
	if (adapter->open_device_map & PORT_MASK)
		schedule_mac_stats_update(adapter,
					  adapter->params.stats_update_period);
	spin_unlock(&adapter->work_lock);
}

/*
 * Processes elmer0 external interrupts in process context.
 */
static void ext_intr_task(void *data)
{
	struct adapter *adapter = data;

	elmer0_ext_intr_handler(adapter);

	/* Now reenable external interrupts */
	spin_lock_irq(&adapter->async_lock);
	adapter->slow_intr_mask |= F_PL_INTR_EXT;
	writel(F_PL_INTR_EXT, adapter->regs + A_PL_CAUSE);
	writel(adapter->slow_intr_mask | F_PL_INTR_SGE_DATA,
                   adapter->regs + A_PL_ENABLE);
	spin_unlock_irq(&adapter->async_lock);
}

/*
 * Interrupt-context handler for elmer0 external interrupts.
 */
void t1_elmer0_ext_intr(struct adapter *adapter)
{
	/*
	 * Schedule a task to handle external interrupts as we require
	 * a process context.  We disable EXT interrupts in the interim
	 * and let the task reenable them when it's done.
	 */
	adapter->slow_intr_mask &= ~F_PL_INTR_EXT;
	writel(adapter->slow_intr_mask | F_PL_INTR_SGE_DATA,
                   adapter->regs + A_PL_ENABLE);
	schedule_work(&adapter->ext_intr_handler_task);
}

void t1_fatal_err(struct adapter *adapter)
{
	if (adapter->flags & FULL_INIT_DONE) {
		t1_sge_stop(adapter->sge);
		t1_interrupts_disable(adapter);
	}
	CH_ALERT("%s: encountered fatal error, operation suspended\n",
		 adapter->name);
}

static int __devinit init_one(struct pci_dev *pdev,
			      const struct pci_device_id *ent)
{
	static int version_printed;

	int i, err, pci_using_dac = 0;
	unsigned long mmio_start, mmio_len;
	const struct board_info *bi;
	struct adapter *adapter = NULL;
	struct port_info *pi;

	if (!version_printed) {
		printk(KERN_INFO "%s - version %s\n", DRV_DESCRIPTION,
		       DRV_VERSION);
		++version_printed;
	}

	err = pci_enable_device(pdev);
	if (err)
        return err;

	if (!(pci_resource_flags(pdev, 0) & IORESOURCE_MEM)) {
		CH_ERR("%s: cannot find PCI device memory base address\n",
		       pci_name(pdev));
		err = -ENODEV;
		goto out_disable_pdev;
	}

	if (!pci_set_dma_mask(pdev, DMA_64BIT_MASK)) {
		pci_using_dac = 1;

		if (pci_set_consistent_dma_mask(pdev, DMA_64BIT_MASK)) {
			CH_ERR("%s: unable to obtain 64-bit DMA for"
			       "consistent allocations\n", pci_name(pdev));
			err = -ENODEV;
			goto out_disable_pdev;
		}

	} else if ((err = pci_set_dma_mask(pdev, DMA_32BIT_MASK)) != 0) {
		CH_ERR("%s: no usable DMA configuration\n", pci_name(pdev));
		goto out_disable_pdev;
	}

	err = pci_request_regions(pdev, DRV_NAME);
	if (err) {
		CH_ERR("%s: cannot obtain PCI resources\n", pci_name(pdev));
		goto out_disable_pdev;
	}

	pci_set_master(pdev);

    mmio_start = pci_resource_start(pdev, 0);
	mmio_len = pci_resource_len(pdev, 0);
	bi = t1_get_board_info(ent->driver_data);

	for (i = 0; i < bi->port_number; ++i) {
		struct net_device *netdev;

		netdev = alloc_etherdev(adapter ? 0 : sizeof(*adapter));
		if (!netdev) {
			err = -ENOMEM;
			goto out_free_dev;
		}

		SET_MODULE_OWNER(netdev);
		SET_NETDEV_DEV(netdev, &pdev->dev);

		if (!adapter) {
			adapter = netdev->priv;
			adapter->pdev = pdev;
			adapter->port[0].dev = netdev;  /* so we don't leak it */

			adapter->regs = ioremap(mmio_start, mmio_len);
			if (!adapter->regs) {
				CH_ERR("%s: cannot map device registers\n",
				       pci_name(pdev));
				err = -ENOMEM;
				goto out_free_dev;
			}

			if (t1_get_board_rev(adapter, bi, &adapter->params)) {
				err = -ENODEV;	  /* Can't handle this chip rev */
				goto out_free_dev;
			}

			adapter->name = pci_name(pdev);
			adapter->msg_enable = dflt_msg_enable;
			adapter->mmio_len = mmio_len;

			init_MUTEX(&adapter->mib_mutex);
			spin_lock_init(&adapter->tpi_lock);
			spin_lock_init(&adapter->work_lock);
			spin_lock_init(&adapter->async_lock);

			INIT_WORK(&adapter->ext_intr_handler_task,
				  ext_intr_task, adapter);
			INIT_WORK(&adapter->stats_update_task, mac_stats_task,
				  adapter);
#ifdef work_struct
			init_timer(&adapter->stats_update_timer);
			adapter->stats_update_timer.function = mac_stats_timer;
			adapter->stats_update_timer.data =
				(unsigned long)adapter;
#endif

			pci_set_drvdata(pdev, netdev);
		}

		pi = &adapter->port[i];
		pi->dev = netdev;
		netif_carrier_off(netdev);
		netdev->irq = pdev->irq;
		netdev->if_port = i;
		netdev->mem_start = mmio_start;
		netdev->mem_end = mmio_start + mmio_len - 1;
		netdev->priv = adapter;
		netdev->features |= NETIF_F_SG | NETIF_F_IP_CSUM;
		netdev->features |= NETIF_F_LLTX;

		adapter->flags |= RX_CSUM_ENABLED | TCP_CSUM_CAPABLE;
		if (pci_using_dac)
			netdev->features |= NETIF_F_HIGHDMA;
		if (vlan_tso_capable(adapter)) {
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
			adapter->flags |= VLAN_ACCEL_CAPABLE;
			netdev->features |=
				NETIF_F_HW_VLAN_TX | NETIF_F_HW_VLAN_RX;
			netdev->vlan_rx_register = vlan_rx_register;
			netdev->vlan_rx_kill_vid = vlan_rx_kill_vid;
#endif
			adapter->flags |= TSO_CAPABLE;
			netdev->features |= NETIF_F_TSO;
		}

		netdev->open = cxgb_open;
		netdev->stop = cxgb_close;
		netdev->hard_start_xmit = t1_start_xmit;
		netdev->hard_header_len += (adapter->flags & TSO_CAPABLE) ?
			sizeof(struct cpl_tx_pkt_lso) :
			sizeof(struct cpl_tx_pkt);
		netdev->get_stats = t1_get_stats;
		netdev->set_multicast_list = t1_set_rxmode;
		netdev->do_ioctl = t1_ioctl;
		netdev->change_mtu = t1_change_mtu;
		netdev->set_mac_address = t1_set_mac_addr;
#ifdef CONFIG_NET_POLL_CONTROLLER
		netdev->poll_controller = t1_netpoll;
#endif
		netdev->weight = 64;

        SET_ETHTOOL_OPS(netdev, &t1_ethtool_ops);
	}

	if (t1_init_sw_modules(adapter, bi) < 0) {
		err = -ENODEV;
		goto out_free_dev;
	}

	/*
	 * The card is now ready to go.  If any errors occur during device
	 * registration we do not fail the whole card but rather proceed only
	 * with the ports we manage to register successfully.  However we must
	 * register at least one net device.
	 */
	for (i = 0; i < bi->port_number; ++i) {
		err = register_netdev(adapter->port[i].dev);
		if (err)
			CH_WARN("%s: cannot register net device %s, skipping\n",
				pci_name(pdev), adapter->port[i].dev->name);
		else {
			/*
			 * Change the name we use for messages to the name of
			 * the first successfully registered interface.
			 */
			if (!adapter->registered_device_map)
				adapter->name = adapter->port[i].dev->name;

                __set_bit(i, &adapter->registered_device_map);
		}
	}
	if (!adapter->registered_device_map) {
		CH_ERR("%s: could not register any net devices\n",
		       pci_name(pdev));
		goto out_release_adapter_res;
	}

	printk(KERN_INFO "%s: %s (rev %d), %s %dMHz/%d-bit\n", adapter->name,
	       bi->desc, adapter->params.chip_revision,
	       adapter->params.pci.is_pcix ? "PCIX" : "PCI",
	       adapter->params.pci.speed, adapter->params.pci.width);
	return 0;

 out_release_adapter_res:
	t1_free_sw_modules(adapter);
 out_free_dev:
	if (adapter) {
		if (adapter->regs) iounmap(adapter->regs);
		for (i = bi->port_number - 1; i >= 0; --i)
			if (adapter->port[i].dev) {
				cxgb_proc_cleanup(adapter, proc_root_driver);
				kfree(adapter->port[i].dev);
			}
	}
	pci_release_regions(pdev);
 out_disable_pdev:
	pci_disable_device(pdev);
	pci_set_drvdata(pdev, NULL);
	return err;
}

static inline void t1_sw_reset(struct pci_dev *pdev)
{
	pci_write_config_dword(pdev, A_PCICFG_PM_CSR, 3);
	pci_write_config_dword(pdev, A_PCICFG_PM_CSR, 0);
}

static void __devexit remove_one(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);

	if (dev) {
		int i;
		struct adapter *adapter = dev->priv;

		for_each_port(adapter, i)
			if (test_bit(i, &adapter->registered_device_map))
				unregister_netdev(adapter->port[i].dev);

		t1_free_sw_modules(adapter);
		iounmap(adapter->regs);
		while (--i >= 0)
			if (adapter->port[i].dev) {
				cxgb_proc_cleanup(adapter, proc_root_driver);
				kfree(adapter->port[i].dev);
			}
		pci_release_regions(pdev);
		pci_disable_device(pdev);
		pci_set_drvdata(pdev, NULL);
		t1_sw_reset(pdev);
	}
}

static struct pci_driver driver = {
	.name     = DRV_NAME,
	.id_table = t1_pci_tbl,
	.probe    = init_one,
	.remove   = __devexit_p(remove_one),
};

static int __init t1_init_module(void)
{
	return pci_module_init(&driver);
}

static void __exit t1_cleanup_module(void)
{
	pci_unregister_driver(&driver);
}

module_init(t1_init_module);
module_exit(t1_cleanup_module);
