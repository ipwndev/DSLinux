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

#include <linux/types.h>

enum WIFI_STATS {
	WIFI_STATS_RXPACKETS = 0,
	WIFI_STATS_RXBYTES,
	WIFI_STATS_RXDATABYTES,

	WIFI_STATS_TXPACKETS,
	WIFI_STATS_TXBYTES,
	WIFI_STATS_TXDATABYTES,
	WIFI_STATS_TXQREJECT,

	WIFI_STATS_TXRAWPACKETS,
	WIFI_STATS_RXRAWPACKETS,
	WIFI_STATS_RXOVERRUN,

	WIFI_STATS_DEBUG1,
	WIFI_STATS_DEBUG2,
	WIFI_STATS_DEBUG3,
	WIFI_STATS_DEBUG4,
	WIFI_STATS_DEBUG5,
	WIFI_STATS_DEBUG6,

	WIFI_STATS_HW_1B0,
	WIFI_STATS_HW_1B4,
	WIFI_STATS_HW_1B8,
	WIFI_STATS_HW_1BC,
	WIFI_STATS_HW_1C0,
	WIFI_STATS_HW_1D0,
	WIFI_STATS_HW_1D4,
	WIFI_STATS_HW_1D8,
	WIFI_STATS_HW_1DC,

	WIFI_STATS_MAX
};

enum WIFI_AP_MODE {
	WIFI_AP_INFRA,
	WIFI_AP_ADHOC,
};

/* Ethernet MTU is 1500 */
#define NDS_WIFI_MAX_PACKET_SIZE 1600

#define WIFI_ARM7_TIMEOUT	100
#define WIFI_MAX_AP		32

/* Used for transmission to arm7 */
struct nds_tx_packet {
	u16 len;	
	u_char *data;
	void *skb;
};
struct nds_rx_packet {
	u16 len;	
	u_char data[NDS_WIFI_MAX_PACKET_SIZE];
};
