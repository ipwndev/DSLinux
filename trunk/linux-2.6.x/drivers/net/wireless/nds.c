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
#include <asm/arch/shmemipc.h>
#include <asm/arch/wifi.h>

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
static void *stats_query_output;
static void *ap_query_output;
static u8 mode_query_output;
static DECLARE_WAIT_QUEUE_HEAD(ndswifi_wait);

static int wll_header_parse(struct sk_buff *skb, unsigned char *haddr)
{
	DEBUG(7, "Called: %s\n", __func__);
	memcpy(haddr, skb->mac.raw + 10, ETH_ALEN);
	return ETH_ALEN;
}

static int nds_start_xmit11(struct sk_buff *skb, struct net_device *dev)
{
	DEBUG(7, "Called: %s len(%d)\n", __func__, skb->len);

	shmemipc_lock();
	if (skb->len > sizeof(SHMEMIPC_BLOCK_ARM9->wifi.data)) {
		shmemipc_unlock();
		DEBUG(2, "Packet too big to send, dropping\n");
		return -E2BIG;
	}
	shmemipc_unlock();

	netif_stop_queue(dev);

	shmemipc_lock();
	SHMEMIPC_BLOCK_ARM9->wifi.type = SHMEMIPC_WIFI_TYPE_PACKET;
	memcpy(SHMEMIPC_BLOCK_ARM9->wifi.data, skb->data, skb->len);
#ifdef PAD_WITH_JUNK
	{
		int i;
		for (i = 0; i < 6; i++) {
			((u8 *) SHMEMIPC_BLOCK_ARM9->wifi.data)[skb->len + i] =
			    50 + i;
		}
	}
#endif
	SHMEMIPC_BLOCK_ARM9->wifi.length = skb->len;
	shmemipc_unlock();

	/* FIXME: this call seems to produce a "scheduling while atomic"
	 * error. shmemipc_flush() may sleep! */
	shmemipc_flush(SHMEMIPC_USER_WIFI);

	dev_kfree_skb(skb);

	return 0;
}

static struct net_device_stats *nds_get_stats(struct net_device *dev)
{
	struct nds_net_priv *local = (struct nds_net_priv *)dev->priv;

	DEBUG(7, "Called: %s\n", __func__);

	stats_query_complete = 0;
	stats_query_output = (void *)&local->stats;
	REG_IPCFIFOSEND = FIFO_WIFI_CMD(FIFO_WIFI_CMD_STATS_QUERY, 0);
	wait_event_interruptible(ndswifi_wait, stats_query_complete != 0);

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

	REG_IPCFIFOSEND = FIFO_WIFI_CMD(FIFO_WIFI_CMD_UP, 0);

	netif_start_queue(dev);

	return 0;
}

static int nds_close(struct net_device *dev)
{
	DEBUG(7, "Called: %s\n", __func__);

	netif_stop_queue(dev);

	REG_IPCFIFOSEND = FIFO_WIFI_CMD(FIFO_WIFI_CMD_DOWN, 0);

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
	REG_IPCFIFOSEND = FIFO_WIFI_CMD(FIFO_WIFI_CMD_MAC_QUERY, 0);
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
	REG_IPCFIFOSEND = FIFO_WIFI_CMD(FIFO_WIFI_CMD_SET_CHANNEL, (fwrq->m));
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

	REG_IPCFIFOSEND =
	    FIFO_WIFI_CMD(FIFO_WIFI_CMD_SET_AP_MODE,
			  (*uwrq ==
			   IW_MODE_ADHOC) ? WIFI_AP_ADHOC : WIFI_AP_INFRA);
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
	REG_IPCFIFOSEND = FIFO_WIFI_CMD(FIFO_WIFI_CMD_GET_AP_MODE, 0);
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
			 struct iw_point *dwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);
	return -ENETDOWN;
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
static int nds_set_scan(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *vwrq, char *extra)
{
	DEBUG(7, "Called: %s\n", __func__);

	scan_complete = 0;
	REG_IPCFIFOSEND = FIFO_WIFI_CMD(FIFO_WIFI_CMD_SCAN, 0);
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
	Wifi_AccessPoint aplist[16];
	int i, j;
	char *current_ev = extra;
	struct iw_event iwe;

	DEBUG(7, "Called: %s\n", __func__);

	for (j = 0; j < 2; j++) {
		ap_query_complete = 0;
		ap_query_output = (void *)aplist;
		REG_IPCFIFOSEND = FIFO_WIFI_CMD(FIFO_WIFI_CMD_AP_QUERY, j);
		wait_event_interruptible(ndswifi_wait, ap_query_complete != 0);

		for (i = 0; i < 16; i++) {
			if (!aplist[i].channel)
				continue;

			iwe.cmd = SIOCGIWAP;
			iwe.u.ap_addr.sa_family = ARPHRD_ETHER;
			memcpy(iwe.u.ap_addr.sa_data, aplist[i].bssid,
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
							  &iwe, aplist[i].ssid);

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
	}

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
			REG_IPCFIFOSEND =
			    FIFO_WIFI_CMD((FIFO_WIFI_CMD_SET_ESSID1 + i),
					  ((j << 16) | tmp));
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
					REG_IPCFIFOSEND =
					    FIFO_WIFI_CMD((FIFO_WIFI_CMD_SET_WEPKEY0 + (index * 2) + i), ((j << 16) | tmp));
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
		REG_IPCFIFOSEND =
		    FIFO_WIFI_CMD(FIFO_WIFI_CMD_SET_WEPKEYID, index);
	}

	/* Read the flags */
	if ((dwrq->flags & IW_ENCODE_MODE) == IW_ENCODE_DISABLED) {
		local->enable = 0;
		REG_IPCFIFOSEND =
		    FIFO_WIFI_CMD(FIFO_WIFI_CMD_SET_WEPMODE, WEPMODE_NONE);
	}
	if ((dwrq->flags & IW_ENCODE_MODE) == IW_ENCODE_ENABLED) {
		local->enable = 1;
		REG_IPCFIFOSEND =
		    FIFO_WIFI_CMD(FIFO_WIFI_CMD_SET_WEPMODE, local->key_size);
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

static void nds_cmd_from_arm7(u8 cmd, u8 offset, u16 data)
{
	DEBUG(7, "Called: %s cmd(%d) offset(%d) data(0x%x)\n", __func__, cmd,
	      offset, data);

	switch (cmd) {
	case FIFO_WIFI_CMD_MAC_QUERY:
		((u16 *) mac_query_output)[offset] = data;
		if (offset == 2) {
			mac_query_complete = 1;
			wake_up_interruptible(&ndswifi_wait);
		}
		break;
	case FIFO_WIFI_CMD_TX_COMPLETE:
		if (global_dev)
			netif_wake_queue(global_dev);
		break;
	case FIFO_WIFI_CMD_SCAN:
		scan_complete = 1;
		wake_up_interruptible(&ndswifi_wait);
		break;
	case FIFO_WIFI_CMD_GET_AP_MODE:
		mode_query_output = data;
		get_mode_completed = 1;
		break;
	}
}

static void nds_wifi_ipc_packet(void)
{
	/* packet recieved */
	struct sk_buff *skb;
	unsigned char *skbp;
	Wifi_RxHeader *rx_hdr;
	struct ieee80211_hdr *hdr_80211;
	int rc;

#ifdef DUMP_INPUT_PACKETS
	if (pc_debug >= 9) {
		u8 *c;
		char buff[2024], *c2;

		c = SHMEMIPC_BLOCK_ARM7->wifi.data;
		c2 = buff;
		while ((c - SHMEMIPC_BLOCK_ARM7->wifi.data) <
		       SHMEMIPC_BLOCK_ARM7->wifi.length) {
			if (((*c) >> 4) > 9)
				*(c2++) = ((*c) >> 4) - 10 + 'A';
			else
				*(c2++) = ((*c) >> 4) + '0';

			if (((*c) & 0x0f) > 9)
				*(c2++) = ((*c) & 0x0f) - 10 + 'A';
			else
				*(c2++) = ((*c) & 0x0f) + '0';
			c++;
			if ((c - SHMEMIPC_BLOCK_ARM7->wifi.data) % 2 == 0)
				*(c2++) = ' ';
		}
		*c2 = '\0';
		DEBUG(9, "len(%d): %s\n",
		      SHMEMIPC_BLOCK_ARM7->wifi.length, buff);
	}
#endif

	rx_hdr = (Wifi_RxHeader *) SHMEMIPC_BLOCK_ARM7->wifi.data;
	hdr_80211 =
	    (struct ieee80211_hdr *)(SHMEMIPC_BLOCK_ARM7->wifi.
				      data + sizeof(Wifi_RxHeader));

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

#ifdef DUMP_OUTPUT_PACKETS
				if (pc_debug >= 9) {
					u8 *c;
					char buff[300], *c2;
					int l = (14 + len - 8 - hdrlen);
					if (l > 80)
						l = 80;

					c = skbp;
					c2 = buff;
					while ((c - skbp) < l) {
						if (((*c) >> 4) > 9)
							*(c2++)
							    =
							    ((*c) >> 4) - 10 +
							    'A';
						else
							*(c2++)
							    = ((*c) >> 4) + '0';

						if (((*c) & 0x0f) > 9)
							*(c2++)
							    =
							    ((*c) & 0x0f) - 10 +
							    'A';
						else
							*(c2++)
							    =
							    ((*c) & 0x0f) + '0';
						c++;
						if ((c - skbp) % 2 == 0)
							*(c2++)
							    = ' ';
					}
					*c2 = '\0';
					DEBUG(9,
					      "len(%d): proto(%d) %s\n",
					      (14 + len - 8 -
					       hdrlen),
					      ntohs(skb->protocol), buff);
				}
#endif
				rc = netif_rx(skb);
				if (rc != NET_RX_SUCCESS)
					DEBUG(3, "netif_rx return(%d)\n", rc);
			}
		}

	}
}

void nds_wifi_ipc_stats(void)
{
	/* stats recieved */
	struct net_device_stats *s =
	    (struct net_device_stats *)stats_query_output;
	u32 *arm7_stats = (u32 *) SHMEMIPC_BLOCK_ARM7->wifi.data;

	DEBUG(7, "Called: %s\n", __func__);

#ifdef REPORT_RAW_PACKET_STATS
	s->rx_packets = arm7_stats[WIFI_STATS_RXRAWPACKETS];
	s->tx_packets = arm7_stats[WIFI_STATS_TXRAWPACKETS];
	s->rx_bytes = arm7_stats[WIFI_STATS_RXBYTES];
	s->tx_bytes = arm7_stats[WIFI_STATS_TXBYTES];
	s->tx_dropped = arm7_stats[WIFI_STATS_TXQREJECT];
#else
	s->rx_packets = arm7_stats[WIFI_STATS_RXPACKETS];
	s->tx_packets = arm7_stats[WIFI_STATS_TXPACKETS];
	s->rx_bytes = arm7_stats[WIFI_STATS_RXDATABYTES];
	s->tx_bytes = arm7_stats[WIFI_STATS_TXDATABYTES];
	s->tx_dropped = arm7_stats[WIFI_STATS_TXQREJECT];
#endif
	DEBUG(8, "WIFI_IE: 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG1]);
	DEBUG(8, "WIFI_IF: 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG2]);
/*
	DEBUG(8, "WIFI_REG(0x54): 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG3]);
	DEBUG(8, "WIFI_REG(0x5A): 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG4]);
*/
	DEBUG(8, "WIFI_REG(0xA0): 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG3]);
	DEBUG(8, "WIFI_REG(0xB8): 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG4]);
	DEBUG(8, "wifi state: 0x%04X\n", arm7_stats[WIFI_STATS_DEBUG5]);
	DEBUG(8, "Num interupts: 0x%08X\n", arm7_stats[WIFI_STATS_DEBUG6]);

	stats_query_complete = 1;
	wake_up_interruptible(&ndswifi_wait);
}

static void nds_wifi_shmemipc_isr(u8 type)
{
	shmemipc_lock();
	DEBUG(7, "Called: %s type(%s) wifi_type(%d)\n", __func__,
	      type == SHMEMIPC_REQUEST_FLUSH ? "flush" : "complete",
	      SHMEMIPC_BLOCK_ARM7->wifi.type);

	switch (type) {
	case SHMEMIPC_REQUEST_FLUSH:
		if (SHMEMIPC_BLOCK_ARM7->wifi.type == SHMEMIPC_WIFI_TYPE_PACKET
		    && global_dev) {
			nds_wifi_ipc_packet();
		} else if (SHMEMIPC_BLOCK_ARM7->wifi.type ==
			   SHMEMIPC_WIFI_TYPE_STATS) {
			nds_wifi_ipc_stats();
		} else if (SHMEMIPC_BLOCK_ARM7->wifi.type ==
			   SHMEMIPC_WIFI_TYPE_AP_LIST) {
			memcpy(ap_query_output, SHMEMIPC_BLOCK_ARM7->wifi.data,
			       16 * sizeof(Wifi_AccessPoint));
			ap_query_complete = 1;
			wake_up_interruptible(&ndswifi_wait);
		}
		break;
	case SHMEMIPC_FLUSH_COMPLETE:
		if (SHMEMIPC_BLOCK_ARM9->wifi.type == SHMEMIPC_WIFI_TYPE_PACKET) {
			/* TX copy complete, do nothing */
		}
		break;
	}
	shmemipc_unlock();
}

static struct fifo_cb nds_cmd_fifocb = {
	.type = FIFO_WIFI,
	.handler.wifi_handler = nds_cmd_from_arm7
};

static struct shmemipc_cb nds_shmem_cb = {
	.user = SHMEMIPC_USER_WIFI,
	.handler.wifi_callback = nds_wifi_shmemipc_isr
};

static int __init init_nds(void)
{
	int rc;
	struct net_device *dev;
	struct nds_net_priv *local;

	DEBUG(7, "Called: %s\n", __func__);

	DEBUG(1, "%s\n", rcsid);
	DEBUG(1, "NDS wireless  init_module \n");

	/* need to talk to the device, to set it up */
	register_fifocb(&nds_cmd_fifocb);

	/* setup shmem so we can get blocks back from arm7 */
	register_shmemipc_cb(&nds_shmem_cb);

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
