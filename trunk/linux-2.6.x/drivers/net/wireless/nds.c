/*
 *   Driver for DS Linux
 *
 *       Copyright 2006 Bret Thaeler - bthaeler@aol.com
 *
 *	This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_arp.h>
#include <linux/ioport.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/rtnetlink.h>
#include <linux/wireless.h>
#include <net/iw_handler.h>

#include <asm/arch/fifo.h>
#include <asm/arch/wifi.h>
#include <asm/cacheflush.h>

#include <net/ieee80211.h>
#include "nds.h"

static char rcsid[] = "NDS Wireless <bthaeler@aol.com>";

MODULE_AUTHOR("Bret Thaeler <bthaeler@aol.com>");
MODULE_DESCRIPTION("Nintendo DS wireless LAN driver");
MODULE_LICENSE("GPL");

static int pc_debug = 0;
MODULE_PARM(pc_debug, "i");
#define DEBUG(n, args...) if (pc_debug>=(n)) printk(args)

#define DUMP_INPUT_PACKETS
#define DUMP_OUTPUT_PACKETS
#define REPORT_RAW_PACKET_STATS

static struct net_device *global_dev;
static u8 mac_query_complete;
static u8 stats_query_complete;
static u8 ap_query_complete;
static u8 scan_complete;
static u8 get_mode_completed;
static void *mac_query_output;
static u8 mode_query_output;
static DECLARE_WAIT_QUEUE_HEAD(ndswifi_wait);

/* Communication interface with ARM7. Each of these data structures is aligned
   on an ARM9 cache line, to allow independent cache invalidation. */
static volatile struct nds_tx_packet tx_packet __attribute__ ((aligned (32))) = {0,0,0};
static volatile struct nds_rx_packet rx_packet __attribute__ ((aligned (32)));
static volatile u32 arm7_stats[WIFI_STATS_MAX] __attribute__ ((aligned (32)));
static volatile Wifi_AccessPoint aplist[WIFI_MAX_AP] __attribute__ ((aligned (32)));
static volatile u8 txpaketbuffer[NDS_WIFI_MAX_PACKET_SIZE] __attribute__ ((aligned (32)));

#if defined(DUMP_INPUT_PACKETS) || defined(DUMP_OUTPUT_PACKETS)
static void nds_dump_packet(u8 *data, u16 len)
{
	u8 *c;
	char buff[2024], *c2;

	c = data;
	c2 = buff;
	while ((c - data) < len) {
		if (((*c) >> 4) > 9)
			*(c2++) = ((*c) >> 4) - 10 + 'A';
		else
			*(c2++) = ((*c) >> 4) + '0';

		if (((*c) & 0x0f) > 9)
			*(c2++) = ((*c) & 0x0f) - 10 + 'A';
		else
			*(c2++) = ((*c) & 0x0f) + '0';
		c++;
		if ((c - data) % 2 == 0)
			*(c2++) = ' ';
	}
	*c2 = '\0';
	printk("len(%d) %s\n", len, buff);
}
#endif

static inline void nds_init_rx_packet(volatile struct nds_rx_packet *rx_packet)
{
	int i;
	rx_packet->len = 0;
	for (i = 0; i < sizeof(rx_packet->data); i++)
		rx_packet->data[i] = 0;
}

#if 0
static int wll_header_parse(struct sk_buff *skb, unsigned char *haddr)
{
	DEBUG(7, "Called: %s\n", __func__);
	memcpy(haddr, skb->mac.raw + 10, ETH_ALEN);
	return ETH_ALEN;
}
#endif

static int nds_start_xmit11(struct sk_buff *skb, struct net_device *dev)
{
	DEBUG(7, "Called: %s len(%d)\n", __func__, skb->len);

	if (skb->len > NDS_WIFI_MAX_PACKET_SIZE) {
		printk(KERN_WARNING "%s: Packet too big to send, dropping\n",
		    dev->name);
		return -E2BIG;
	}

	/* only transmit one packet at a time */
	netif_stop_queue(dev);

#ifdef DUMP_OUTPUT_PACKETS
	if (pc_debug >= 9) {
		printk("output packet: ");
		nds_dump_packet((u8*)tx_packet.data, tx_packet.len);
	}
#endif
	/* wrap up packet information and send it to arm7 */
	tx_packet.len = skb->len;
	tx_packet.data = (void *)txpaketbuffer;
	memcpy((void *)txpaketbuffer, skb->data, skb->len);

	/* write data to memory before ARM7 gets hands on */
	/* here we are using "flush" because we don't need the data in the cache any more */
	dmac_flush_range((unsigned long)&tx_packet,
                        ((unsigned long)&tx_packet)+sizeof(tx_packet));
	dmac_flush_range((unsigned long)txpaketbuffer,
                        ((unsigned long)txpaketbuffer)+skb->len);
	dev_kfree_skb(skb);
	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_TX, (u32)(&tx_packet)));
	return 0;
}

static struct net_device_stats *nds_get_stats(struct net_device *dev)
{
	struct nds_net_priv *local = (struct nds_net_priv *)dev->priv;

	DEBUG(7, "Called: %s\n", __func__);

	/* invalidate cache before we read data written by ARM7 */
	/* do this here, because of alignment & cache clean at nonaligned areas. */
	dmac_inv_range((unsigned long)&arm7_stats[0],
                       (unsigned long)&arm7_stats[WIFI_STATS_MAX]);

	stats_query_complete = 0;
	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_STATS_QUERY, 0));
	if(wait_event_interruptible_timeout(ndswifi_wait,
	    stats_query_complete != 0, WIFI_ARM7_TIMEOUT) == 0) {
		printk(KERN_WARNING "%s: timed out waiting for ARM7\n", __func__);
	}

#ifdef REPORT_RAW_PACKET_STATS
	local->stats.rx_packets = arm7_stats[WIFI_STATS_RXRAWPACKETS];
	local->stats.tx_packets = arm7_stats[WIFI_STATS_TXRAWPACKETS];
	local->stats.rx_bytes   = arm7_stats[WIFI_STATS_RXBYTES];
	local->stats.tx_bytes   = arm7_stats[WIFI_STATS_TXBYTES];
	local->stats.tx_dropped = arm7_stats[WIFI_STATS_TXQREJECT];
#else
	local->stats.rx_packets = arm7_stats[WIFI_STATS_RXPACKETS];
	local->stats.tx_packets = arm7_stats[WIFI_STATS_TXPACKETS];
	local->stats.rx_bytes   = arm7_stats[WIFI_STATS_RXDATABYTES];
	local->stats.tx_bytes   = arm7_stats[WIFI_STATS_TXDATABYTES];
	local->stats.tx_dropped = arm7_stats[WIFI_STATS_TXQREJECT];
#endif
	DEBUG(8, "WIFI_IE: 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG1]);
	DEBUG(8, "WIFI_IF: 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG2]);
/*
	DEBUG(8, "WIFI_REG(0x54): 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG3]);
	DEBUG(8, "WIFI_REG(0x5A): 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG4]);
*/
	DEBUG(8, "WIFI_REG(0xA0): 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG3]);
	DEBUG(8, "WIFI_REG(0xB8): 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG4]);
	DEBUG(8, "wifi state    : 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG5]);
	DEBUG(8, "Num interupts : 0x%08X\n", arm7_stats[WIFI_STATS_DEBUG6]);

	return &local->stats;
}

static int nds_set_mac_address(struct net_device *dev, void *p)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

static int nds_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	// struct nds_net_priv *local = (struct nds_net_priv *)dev->priv;
	int err = 0;
#if WIRELESS_EXT > 7
	// struct iwreq *wrq = (struct iwreq *) ifr;
#endif				/* WIRELESS_EXT > 7 */

	DEBUG(7, "Called: %s\n", __func__);

	DEBUG(2, "nds_ioctl dev=%p, ifr=%p, cmd = 0x%x\n", dev, ifr, cmd);
	/* Validate the command */
	switch (cmd) {
	default:
		DEBUG(0, "nds_ioctl cmd = 0x%x\n", cmd);
		err = -EOPNOTSUPP;
	}
	return err;
}

static int nds_change_mtu(struct net_device *dev, int new_mtu)
{
	DEBUG(7, "Called: %s\n", __func__);

	if (new_mtu < ETH_ZLEN || new_mtu > ETH_DATA_LEN)
		return -EINVAL;
	dev->mtu = new_mtu;
	return 0;
}

static int nds_open(struct net_device *dev)
{
	DEBUG(7, "Called: %s\n", __func__);

	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_UP, 0));

	netif_start_queue(dev);

	return 0;
}

static int nds_close(struct net_device *dev)
{
	DEBUG(7, "Called: %s\n", __func__);

	netif_stop_queue(dev);

	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_DOWN, 0));

	return 0;
}

#ifdef NODEF
static int nds_dev_config(struct net_device *dev, struct ifmap *map)
{
	DEBUG(7, "Called: %s\n", __func__);
	/* Dummy routine to satisfy device structure */
	DEBUG(1, "nds_dev_config(dev=%p,ifmap=%p)\n", dev, map);

	return 0;
}
#endif

static int nds_dev_init(struct net_device *dev)
{
	DEBUG(7, "Called: %s\n", __func__);

	/* get the mac addr */
	mac_query_complete = 0;
	mac_query_output = (void *)dev->dev_addr;
	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_MAC_QUERY, 0));
	wait_event_interruptible(ndswifi_wait, mac_query_complete != 0);
	return 0;
}

#ifdef WIRELESS_EXT
static struct iw_statistics *nds_get_wireless_stats(struct net_device *dev)
{
	DEBUG(7, "Called: %s\n", __func__);
	return NULL;
}

/*------------------------------------------------------------------*/
/*
 * Commit handler : called after a bunch of SET operations
 */
static int nds_config_commit(struct net_device *dev, struct iw_request_info *info,	/* NULL */
			     void *zwrq,	/* NULL */
			     char *extra)
{				/* NULL */
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get protocol name
 */
static int nds_get_name(struct net_device *dev,
			struct iw_request_info *info, char *cwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	strcpy(cwrq, "IEEE 802.11");
	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set frequency
 */
static int nds_set_freq(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_freq *fwrq, char *extra)
{
	DEBUG(7, "Called: %s m(%d) e(%d)\n", __func__, fwrq->m, fwrq->e);

	if (fwrq->e != 0)
		return -EINVAL;	/* not allowed */
	if (fwrq->m < 1 || fwrq->m > 13)
		return -EINVAL;	/* not allowed */
	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_SET_CHANNEL, (fwrq->m)));
	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get frequency
 */
static int nds_get_freq(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_freq *fwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set Mode of Operation
 */
static int nds_set_mode(struct net_device *dev,
			struct iw_request_info *info, __u32 * uwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);

	if (*uwrq != IW_MODE_ADHOC && *uwrq != IW_MODE_INFRA)
		return -EINVAL;

	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_SET_AP_MODE,
			  (*uwrq ==
			   IW_MODE_ADHOC) ? WIFI_AP_ADHOC : WIFI_AP_INFRA));
	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get Mode of Operation
 */
static int nds_get_mode(struct net_device *dev,
			struct iw_request_info *info, __u32 * uwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);

	get_mode_completed = 0;
	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_GET_AP_MODE, 0));
	wait_event_interruptible(ndswifi_wait, get_mode_completed != 0);
	*uwrq =
	    (mode_query_output ==
	     WIFI_AP_ADHOC) ? IW_MODE_ADHOC : IW_MODE_INFRA;
	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set Sensitivity
 */
static int nds_set_sens(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get Sensitivity
 */
static int nds_get_sens(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get range info
 */
static int nds_get_range(struct net_device *dev,
			 struct iw_request_info *info,
		         union iwreq_data *wrqu, char *extra)
{
	struct iw_range *range = (struct iw_range *)extra;

	DEBUG(7, "Called: %s\n", __func__);

	/* Set the length (very important for backward compatibility) */
	wrqu->data.length = sizeof(*range);

	/* Set all the info we don't care or don't know about to zero */
	memset(range, 0, sizeof(*range));

	/* Set the Wireless Extension versions */
	range->we_version_compiled	= WIRELESS_EXT;
	range->we_version_source	= 1;
	range->throughput		= 2 * 1000 * 1000;     /* ~2 Mb/s */
	/* FIXME: study the code to fill in more fields... */
	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set AP address
 */
static int nds_set_wap(struct net_device *dev,
		       struct iw_request_info *info,
		       struct sockaddr *awrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get AP address
 */
static int nds_get_wap(struct net_device *dev,
		       struct iw_request_info *info,
		       struct sockaddr *awrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get AP List
 * Note : this is deprecated in favor of IWSCAN
 */
/*
static int nds_get_aplist(struct net_device *dev,
			   struct iw_request_info *info,
			   struct iw_point *dwrq,
			   char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}
*/

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : Initiate Scan
 */
#if WIRELESS_EXT > 17
static int nds_set_scan(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *vwrq, char *extra)
#else
static int nds_set_scan(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *vwrq, char *extra)
#endif
{
	DEBUG(7, "Called: %s\n", __func__);

	scan_complete = 0;
	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_SCAN, 0));
	wait_event_interruptible(ndswifi_wait, scan_complete != 0);

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : Read Scan Results
 */
static int nds_get_scan(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *dwrq, char *extra)
{
	int i;
	char *current_ev = extra;
	struct iw_event iwe;

	DEBUG(7, "Called: %s\n", __func__);

	/* invalidate cache before we read data written by ARM7 */
	/* do this here, because of alignment & cache clean at nonaligned areas. */
	dmac_inv_range((unsigned long)&aplist[0],
                      ((unsigned long)&aplist[WIFI_MAX_AP])); 

	/* stop ap list updates */
	ap_query_complete = 0;
	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_AP_QUERY, 1));
	wait_event_interruptible(ndswifi_wait, ap_query_complete != 0);

	for (i = 0; i < WIFI_MAX_AP; i++) {
		if (!aplist[i].channel)
			continue;

		iwe.cmd = SIOCGIWAP;
		iwe.u.ap_addr.sa_family = ARPHRD_ETHER;
		memcpy(iwe.u.ap_addr.sa_data, (void*)aplist[i].bssid,
		       ETH_ALEN);
		current_ev =
		    iwe_stream_add_event(current_ev,
					 extra + IW_SCAN_MAX_DATA, &iwe,
					 IW_EV_ADDR_LEN);

		iwe.cmd = SIOCGIWESSID;
		iwe.u.data.flags = 1;
		iwe.u.data.length = aplist[i].ssid_len;
		current_ev = iwe_stream_add_point(current_ev,
						  extra +
						  IW_SCAN_MAX_DATA,
						  &iwe, (char*)aplist[i].ssid);

		iwe.cmd = SIOCGIWMODE;
		iwe.u.mode =
		    (aplist[i].flags & WFLAG_APDATA_ADHOC) ? 1 : 2;
		current_ev =
		    iwe_stream_add_event(current_ev,
					 extra + IW_SCAN_MAX_DATA, &iwe,
					 IW_EV_UINT_LEN);

		iwe.cmd = SIOCGIWFREQ;
		iwe.u.freq.m = aplist[i].channel;
		iwe.u.freq.e = 0;
		current_ev = iwe_stream_add_event(current_ev,
						  extra +
						  IW_SCAN_MAX_DATA,
						  &iwe, IW_EV_FREQ_LEN);

		iwe.cmd = SIOCGIWENCODE;
		if (aplist[i].flags & WFLAG_APDATA_WEP)
			iwe.u.data.flags =
			    IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
		else
			iwe.u.data.flags = IW_ENCODE_DISABLED;
		iwe.u.data.length = 0;
		current_ev = iwe_stream_add_point(current_ev,
						  extra +
						  IW_SCAN_MAX_DATA,
						  &iwe, NULL);
	}

	/* restart aplist updates */
	ap_query_complete = 0;
	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_AP_QUERY, 0));
	wait_event_interruptible(ndswifi_wait, ap_query_complete != 0);

	/* Length of data */
	dwrq->length = (current_ev - extra);
	dwrq->flags = 0;	/* FIXME: set properly these flags */

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set ESSID
 */
static int nds_set_essid(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_point *dwrq, char *extra)
{
	struct nds_net_priv *local = (struct nds_net_priv *)dev->priv;
	char *c;
	int i, j;

	/* Terminate the string */
	i = dwrq->length;
	if (i > IW_ESSID_MAX_SIZE)
		i = IW_ESSID_MAX_SIZE;
	memcpy(local->ssid, extra, i);
	local->ssid[i] = '\0';

	DEBUG(7, "Called: %s essid(%s)\n", __func__, local->ssid);

	c = local->ssid;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			u32 tmp;

			tmp = *(c++) << 8;
			tmp |= *(c++);
			nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_SET_ESSID,
				/* Send the essid in pieces of 2 chars each.
				 * Encoded as follows:
				 *          #essid      offset     2 chars */
					  ((i << 20) | (j << 16) | tmp)));
			if (!*(c - 1) || !*(c - 2)) {
				i = 4;
				break;
			}
		}
	}

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get ESSID
 */
static int nds_get_essid(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_point *dwrq, char *extra)
{
	struct nds_net_priv *local = (struct nds_net_priv *)dev->priv;

	DEBUG(7, "Called: %s\n", __func__);

	strcpy(extra, local->ssid);
	dwrq->length = strlen(local->ssid) + 1;
	dwrq->flags = 1;	/* active */

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set Nickname
 */
static int nds_set_nick(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *dwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get Nickname
 */
static int nds_get_nick(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *dwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set Bit-Rate
 */
static int nds_set_rate(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get Bit-Rate
 */
static int nds_get_rate(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set RTS threshold
 */
static int nds_set_rts(struct net_device *dev,
		       struct iw_request_info *info,
		       struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get RTS threshold
 */
static int nds_get_rts(struct net_device *dev,
		       struct iw_request_info *info,
		       struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get Fragmentation threshold
 */
static int nds_get_frag(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set Fragmentation threshold
 */
static int nds_set_frag(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set Tx-Power
 */
static int nds_set_txpow(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get Tx-Power
 */
static int nds_get_txpow(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set Retry limits
 */
static int nds_set_retry(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get Retry limits
 */
static int nds_get_retry(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set Encryption Key
 */
static int nds_set_encode(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_point *dwrq, char *extra)
{
	int index;
	struct nds_net_priv *local = (struct nds_net_priv *)dev->priv;

	DEBUG(7, "Called: %s length(%d) flags(0x%04X)\n", __func__,
	      dwrq->length, dwrq->flags);

	/* Basic checking: do we have a key to set ?  */
	if (dwrq->length > 0) {
		int key_len;
		index = (dwrq->flags & IW_ENCODE_INDEX) - 1;
		/* Check the size of the key */
		if (dwrq->length > MAX_KEY_SIZE) {
			return -EINVAL;
		}
		/* Check the index (none -> use current) */
		if ((index < 0) || (index > 3))
			index = local->current_index;
		/* Set the length */
		if (dwrq->length > MIN_KEY_SIZE) {
			key_len = MAX_KEY_SIZE;
			local->key_size = WEPMODE_128BIT;
		} else {
			local->key_size = WEPMODE_40BIT;
			if (dwrq->length > 0)
				key_len = MIN_KEY_SIZE;
			else
				/* Disable the key */
				key_len = 0;
		}
		/* Check if the key is not marked as invalid */
		if (!(dwrq->flags & IW_ENCODE_NOKEY)) {
			u8 *k;
			int i, j;
			u32 tmp;

			/* Cleanup */
			memset(local->key_key[index], 0, MAX_KEY_SIZE);
			/* Copy the key in the driver */
			memcpy(local->key_key[index], extra, dwrq->length);
			/* Send the key to the card */
			k = local->key_key[index];
			for (i = 0; i < 2; i++) {
				for (j = 0; j < 4; j++) {
					tmp = *(k++) << 8;
					tmp |= *(k++);
					nds_fifo_send(FIFO_WIFI_CMD(
					    FIFO_WIFI_CMD_SET_WEPKEY,
					    ((index << 20) | (i << 18)
					     | (j << 16) | tmp)));
					if ((k - local->key_key[index]) >=
					    key_len) {
						i = 2;
						break;
					}
				}
			}
		}
		/* WE specify that if a valid key is set, encryption
		 * should be enabled (user may turn it off later)
		 * This is also how "iwconfig ethX key on" works */
/*
		if((index == current_index) && (key.len > 0) &&
		   (local->config.authType == AUTH_OPEN)) {
			local->config.authType = AUTH_ENCRYPT;
			set_bit (FLAG_COMMIT, &local->flags);
		}
*/
	}

	/* Do we want to just set the transmit key index ? */
	index = (dwrq->flags & IW_ENCODE_INDEX) - 1;
	if ((index >= 0) && index < 4) {
		local->current_index = index;
		nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_SET_WEPKEYID, index));
	}

	/* Read the flags */
	if ((dwrq->flags & IW_ENCODE_MODE) == IW_ENCODE_DISABLED) {
		local->enable = 0;
		nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_SET_WEPMODE,
		    WEPMODE_NONE));
	}
	if ((dwrq->flags & IW_ENCODE_MODE) == IW_ENCODE_ENABLED) {
		local->enable = 1;
		nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_SET_WEPMODE,
		    local->key_size));
	}
	return 0;		/* Call commit handler */
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get Encryption Key
 */
static int nds_get_encode(struct net_device *dev,
			  struct iw_request_info *info,
			  struct iw_point *dwrq, char *extra)
{
	struct nds_net_priv *local = (struct nds_net_priv *)dev->priv;

	DEBUG(7, "Called: %s\n", __func__);

	if (local->enable) {
		dwrq->flags = IW_ENCODE_ENABLED;
	} else {
		dwrq->flags = IW_ENCODE_DISABLED;
	}

	dwrq->flags |= (local->current_index + 1) & IW_ENCODE_INDEX;

	if (local->key_size == WEPMODE_128BIT) {
		dwrq->length = MAX_KEY_SIZE;
		memcpy(extra, local->key_key[local->current_index],
		       dwrq->length);
	} else if (local->key_size == WEPMODE_40BIT) {
		dwrq->length = MIN_KEY_SIZE;
		memcpy(extra, local->key_key[local->current_index],
		       dwrq->length);
	}

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : set Power Management
 */
static int nds_set_power(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Wireless Handler : get Power Management
 */
static int nds_get_power(struct net_device *dev,
			 struct iw_request_info *info,
			 struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
}

/*------------------------------------------------------------------*/
/*
 * Structures to export the Wireless Handlers
 */

static const struct iw_priv_args nds_private_args[] = {
/*{ cmd,         set_args,                            get_args, name } */
/*
  { AIROIOCTL, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | sizeof (aironet_ioctl),
    IW_PRIV_TYPE_BYTE | 2047, "ndsioctl" },
  { AIROIDIFC, IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED | sizeof (aironet_ioctl),
    IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, "ndsidifc" },
*/
};

static const iw_handler nds_handler[] = {
	(iw_handler) nds_config_commit,	/* SIOCSIWCOMMIT */
	(iw_handler) nds_get_name,	/* SIOCGIWNAME */
	(iw_handler) NULL,	/* SIOCSIWNWID */
	(iw_handler) NULL,	/* SIOCGIWNWID */
	(iw_handler) nds_set_freq,	/* SIOCSIWFREQ */
	(iw_handler) nds_get_freq,	/* SIOCGIWFREQ */
	(iw_handler) nds_set_mode,	/* SIOCSIWMODE */
	(iw_handler) nds_get_mode,	/* SIOCGIWMODE */
	(iw_handler) nds_set_sens,	/* SIOCSIWSENS */
	(iw_handler) nds_get_sens,	/* SIOCGIWSENS */
	(iw_handler) NULL,	/* SIOCSIWRANGE */
	(iw_handler) nds_get_range,	/* SIOCGIWRANGE */
	(iw_handler) NULL,	/* SIOCSIWPRIV */
	(iw_handler) NULL,	/* SIOCGIWPRIV */
	(iw_handler) NULL,	/* SIOCSIWSTATS */
	(iw_handler) NULL,	/* SIOCGIWSTATS */
	iw_handler_set_spy,	/* SIOCSIWSPY */
	iw_handler_get_spy,	/* SIOCGIWSPY */
	iw_handler_set_thrspy,	/* SIOCSIWTHRSPY */
	iw_handler_get_thrspy,	/* SIOCGIWTHRSPY */
	(iw_handler) nds_set_wap,	/* SIOCSIWAP */
	(iw_handler) nds_get_wap,	/* SIOCGIWAP */
	(iw_handler) NULL,	/* -- hole -- */
	(iw_handler) NULL /*nds_get_aplist */ ,	/* SIOCGIWAPLIST */
	(iw_handler) nds_set_scan,	/* SIOCSIWSCAN */
	(iw_handler) nds_get_scan,	/* SIOCGIWSCAN */
	(iw_handler) nds_set_essid,	/* SIOCSIWESSID */
	(iw_handler) nds_get_essid,	/* SIOCGIWESSID */
	(iw_handler) nds_set_nick,	/* SIOCSIWNICKN */
	(iw_handler) nds_get_nick,	/* SIOCGIWNICKN */
	(iw_handler) NULL,	/* -- hole -- */
	(iw_handler) NULL,	/* -- hole -- */
	(iw_handler) nds_set_rate,	/* SIOCSIWRATE */
	(iw_handler) nds_get_rate,	/* SIOCGIWRATE */
	(iw_handler) nds_set_rts,	/* SIOCSIWRTS */
	(iw_handler) nds_get_rts,	/* SIOCGIWRTS */
	(iw_handler) nds_set_frag,	/* SIOCSIWFRAG */
	(iw_handler) nds_get_frag,	/* SIOCGIWFRAG */
	(iw_handler) nds_set_txpow,	/* SIOCSIWTXPOW */
	(iw_handler) nds_get_txpow,	/* SIOCGIWTXPOW */
	(iw_handler) nds_set_retry,	/* SIOCSIWRETRY */
	(iw_handler) nds_get_retry,	/* SIOCGIWRETRY */
	(iw_handler) nds_set_encode,	/* SIOCSIWENCODE */
	(iw_handler) nds_get_encode,	/* SIOCGIWENCODE */
	(iw_handler) nds_set_power,	/* SIOCSIWPOWER */
	(iw_handler) nds_get_power,	/* SIOCGIWPOWER */
};

/* Note : don't describe AIROIDIFC and AIROOLDIDIFC in here.
 * We want to force the use of the ioctl code, because those can't be
 * won't work the iw_handler code (because they simultaneously read
 * and write data and iw_handler can't do that).
 * Note that it's perfectly legal to read/write on a single ioctl command,
 * you just can't use iwpriv and need to force it via the ioctl handler.
 * Jean II */
static const iw_handler nds_private_handler[] = {
	NULL,			/* SIOCIWFIRSTPRIV */
};

static const struct iw_handler_def nds_handler_def = {
	.num_standard = sizeof(nds_handler) / sizeof(iw_handler),
	.num_private = sizeof(nds_private_handler) / sizeof(iw_handler),
	.num_private_args =
	    sizeof(nds_private_args) / sizeof(struct iw_priv_args),
	.standard = (iw_handler *) nds_handler,
	.private = (iw_handler *) nds_private_handler,
	.private_args = (struct iw_priv_args *)nds_private_args,
	
};
#endif

static void wifi_setup(struct net_device *dev)
{
	DEBUG(7, "Called: %s\n", __func__);

/*
	dev->hard_header_parse  = wll_header_parse;
*/
	dev->hard_start_xmit = &nds_start_xmit11;
/*
    dev->set_config = &nds_dev_config;
*/
	dev->get_stats = &nds_get_stats;
	dev->set_mac_address = &nds_set_mac_address;
	dev->do_ioctl = &nds_ioctl;
#ifdef WIRELESS_EXT
	dev->get_wireless_stats = nds_get_wireless_stats;
	dev->wireless_handlers = (struct iw_handler_def *)&nds_handler_def;
#endif				/* WIRELESS_EXT */
	dev->change_mtu = &nds_change_mtu;
	dev->init = &nds_dev_init;
	dev->open = &nds_open;
	dev->stop = &nds_close;
}

static void nds_wifi_receive_packet(void)
{
	struct sk_buff *skb;
	unsigned char *skbp;
	Wifi_RxHeader *rx_hdr;
	struct ieee80211_hdr *hdr_80211;
	int rc;

	if (!global_dev)
		return;

	/* invalidate cache before we read data written by ARM7 */
	dmac_inv_range((unsigned long)&rx_packet,
                      ((unsigned long)&rx_packet)+sizeof(rx_packet));

#ifdef DUMP_INPUT_PACKETS
	if (pc_debug >= 9) {
		printk("input packet: ");
		nds_dump_packet((u8*)rx_packet.data, rx_packet.len);
	}
#endif
	rx_hdr = (Wifi_RxHeader*)(u32)&rx_packet.data;
	hdr_80211 = (struct ieee80211_hdr *)((u32)&rx_packet.data
			+ sizeof(Wifi_RxHeader));

	if ((hdr_80211->frame_ctl & 0x01CF) == IEEE80211_FTYPE_DATA) {
		if ((((u16 *) global_dev->dev_addr)[0] ==
		     ((u16 *) hdr_80211->addr1)[0]
		     && ((u16 *) global_dev->dev_addr)[1] ==
		     ((u16 *) hdr_80211->addr1)[1]
		     && ((u16 *) global_dev->dev_addr)[2] ==
		     ((u16 *) hdr_80211->addr1)[2])
		    || (((u16 *) hdr_80211->addr1)[0] == 0xFFFF
			&& ((u16 *) hdr_80211->addr1)[1] ==
			0xFFFF && ((u16 *) hdr_80211->addr1)[2] == 0xFFFF)) {
			/* hdrlen == 802.11 header length  bytes */
			int base2, hdrlen;
			base2 = 22;
			hdrlen = 24;
			// looks like WEP IV and IVC are removed from RX packets

			// check for LLC/SLIP header...
			if (((u16 *) rx_hdr)[base2 - 4 + 0] ==
			    0xAAAA
			    && ((u16 *) rx_hdr)[base2 - 4 +
						1] == 0x0003
			    && ((u16 *) rx_hdr)[base2 - 4 + 2] == 0) {
				// mb = sgIP_memblock_allocHW(14,len-8-hdrlen);
				// Wifi_RxRawReadPacket(base2,(len-8-hdrlen)&(~1),((u16 *)mb->datastart)+7);^M
				/*
				 * 14 (ether header) 
				 * + byte_length
				 *  - (ieee hdr 24 bytes) 
				 *  - 8 bytes LLC
				 */
				int len = rx_hdr->byteLength;
				if (!
				    (skb =
				     dev_alloc_skb(14 + len -
						   8 - hdrlen + 2))) {
					/* priv->stats.rx_dropped++; */
					return;
				}
				skb->dev = global_dev;
				skb_reserve(skb, 2);
				skbp = skb_put(skb, 14 + len - 8 - hdrlen);
				memcpy(skbp + 14, &(((u16 *)
						     rx_hdr)
						    [base2]),
				       (len - 8 - hdrlen));
				memcpy(skbp, hdr_80211->addr1, ETH_ALEN);	// copy dest

				if (hdr_80211->
				    frame_ctl & IEEE80211_FCTL_FROMDS) {
					memcpy(skbp + ETH_ALEN,
					       hdr_80211->addr3, ETH_ALEN);
				} else {
					memcpy(skbp + ETH_ALEN,
					       hdr_80211->addr2, ETH_ALEN);
				}
				((u16 *) skbp)[6] = ((u16 *)
						     rx_hdr)[(hdrlen / 2) + 6 +
							     3];

				skb->protocol = eth_type_trans(skb, global_dev);

				rc = netif_rx(skb);
				if (rc != NET_RX_SUCCESS)
					DEBUG(3, "netif_rx return(%d)\n", rc);
			}
		}

	}

	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_RX_COMPLETE, 0)); 	
}

/**
 * Wifi interrupt handler from ARM7.
 */
static void nds_cmd_from_arm7(u8 cmd, u32 data)
{
	u8 offset;

	DEBUG(7, "Called: %s cmd(%d)  data(0x%x)\n", __func__, cmd, data);

	switch (cmd) {
	case FIFO_WIFI_CMD_MAC_QUERY:
		offset = (data >> 16) & 0x03;
		((u16 *) mac_query_output)[offset] = (data & 0xffff);
		if (offset == 2) {
			mac_query_complete = 1;
			wake_up_interruptible(&ndswifi_wait);
		}
		break;
	case FIFO_WIFI_CMD_TX_COMPLETE:
		if (global_dev)
			netif_wake_queue(global_dev);
		break;
	case FIFO_WIFI_CMD_RX:
		/* this is now the only long interrupt handler.
		   It's a good candidate for deferred execution. */
		nds_wifi_receive_packet();
		break;
	case FIFO_WIFI_CMD_SCAN:
		scan_complete = 1;
		wake_up_interruptible(&ndswifi_wait);
		break;
	case FIFO_WIFI_CMD_GET_AP_MODE:
		mode_query_output = data;
		get_mode_completed = 1;
		break;
	case FIFO_WIFI_CMD_STATS_QUERY:
		stats_query_complete = 1;
		wake_up_interruptible(&ndswifi_wait);
		break;
	case FIFO_WIFI_CMD_AP_QUERY:
		ap_query_complete = 1;
		wake_up_interruptible(&ndswifi_wait);
		break;
	}
}

static struct fifo_cb nds_cmd_fifocb = {
	.type = FIFO_WIFI,
	.handler.wifi_handler = nds_cmd_from_arm7
};

static int __init init_nds(void)
{
	u8 *p = NULL;
	int i;
	int rc;
	struct net_device *dev;
	struct nds_net_priv *local;

	DEBUG(7, "Called: %s\n", __func__);
	printk(KERN_INFO "%s\n", rcsid);
	DEBUG(1, "NDS wireless  init_module \n");

	/* need to talk to the device, to set it up */
	register_fifocb(&nds_cmd_fifocb);

	/* Initialise stats buffer and send address to ARM7 */
	for (i = 0; i < WIFI_STATS_MAX; i++)
		arm7_stats[i] = 0;
	DEBUG(5, "%s: sending stats buffer address 0x%p\n", __func__, arm7_stats);
	/* write data to memory before ARM7 gets hands on */
	dmac_flush_range((unsigned long)&arm7_stats[0],
                         (unsigned long)&arm7_stats[WIFI_STATS_MAX]);
	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_STATS_QUERY, (u32)arm7_stats));

	/* Initialise AP list buffer and send address to ARM7 */
	p = (u8*)aplist;
	for (i = 0; i < (sizeof(Wifi_AccessPoint) * WIFI_MAX_AP); i++)
		*p++ = 0;
	p = NULL;
	DEBUG(5, "%s: sending aplist buffer address 0x%p\n", __func__, aplist);
	/* write data to memory before ARM7 gets hands on */
	dmac_clean_range((unsigned long)&aplist[0],
                         (unsigned long)&aplist[WIFI_MAX_AP]);
	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_AP_QUERY, (u32)aplist));

	/* Allocate space for private device-specific data */
	dev = alloc_etherdev(sizeof(struct nds_net_priv));
	strcpy(dev->name, "nds");
	/* my router needs 1492 */
	dev->mtu = 1492;
	wifi_setup(dev);

	if (!dev)
		goto fail_alloc_dev;

	local = dev->priv;

	netif_stop_queue(dev);

	rtnl_lock();
	rc = register_netdevice(dev);
	rtnl_unlock();
	if (rc != 0) {
		printk("nds_config register_netdevice() failed: %d\n", rc);
		return 1;
	}

	global_dev = dev;

	/* Initialise packet receive buffer and send address to ARM7 */
	nds_init_rx_packet(&rx_packet);
	DEBUG(5, "%s: sending rx buffer address 0x%p\n", __func__, &rx_packet);
	/* write data to memory before ARM7 gets hands on */
	dmac_clean_range((unsigned long)&rx_packet,
                        ((unsigned long)&rx_packet)+sizeof(rx_packet));
	nds_fifo_send(FIFO_WIFI_CMD(FIFO_WIFI_CMD_RX, (u32)&rx_packet));
	
	return 0;

      fail_alloc_dev:

	return 1;
}				/* init_nds */

static void __exit exit_nds(void)
{
	DEBUG(7, "Called: %s\n", __func__);

	DEBUG(0, "NDS wireless: cleanup_module\n");

	unregister_fifocb(&nds_cmd_fifocb);

	global_dev = 0;

}				/* exit_nds */

module_init(init_nds);
module_exit(exit_nds);
