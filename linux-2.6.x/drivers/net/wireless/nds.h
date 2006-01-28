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

/* match with arch/armnommu/mach-nds/arm7/wifi.h */
enum WIFI_FIFO_CMDS {
	WIFI_CMD_UP,
	WIFI_CMD_DOWN,
	WIFI_CMD_MAC_QUERY,
	WIFI_CMD_TX_COMPLETE,
	WIFI_CMD_STATS_QUERY,
	WIFI_CMD_SET_ESSID1,
	WIFI_CMD_SET_ESSID2,
	WIFI_CMD_SET_ESSID3,
	WIFI_CMD_SET_ESSID4,
	WIFI_CMD_SET_CHANNEL,
	WIFI_CMD_SET_WEPKEY0,
	WIFI_CMD_SET_WEPKEY0A,
	WIFI_CMD_SET_WEPKEY1,
	WIFI_CMD_SET_WEPKEY1A,
	WIFI_CMD_SET_WEPKEY2,
	WIFI_CMD_SET_WEPKEY2A,
	WIFI_CMD_SET_WEPKEY3,
	WIFI_CMD_SET_WEPKEY3A,
	WIFI_CMD_SET_WEPKEYID,
	WIFI_CMD_SET_WEPMODE,
	WIFI_CMD_AP_QUERY,
	WIFI_CMD_SCAN,
};

/* match with arch/armnommu/mach-nds/arm7/wifi.h */
enum {
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

enum WEPMODES {
	WEPMODE_NONE = 0,
	WEPMODE_40BIT = 1,
	WEPMODE_128BIT = 3
};

#define MAX_KEY_SIZE 13		// 128 (?) bits
#define MIN_KEY_SIZE  5		// 40 bits RC4 - WEP

struct nds_net_priv {
	struct net_device_stats stats;

	char ssid[IW_ESSID_MAX_SIZE + 1];	// 0-32byte data, zero

	/* wep stuff */
	u16 current_index;
	u16 key_size;
	u16 enable;
	u8 key_key[4][MAX_KEY_SIZE];
};

#define WFLAG_APDATA_ADHOC			0x0001
#define WFLAG_APDATA_WEP			0x0002
#define WFLAG_APDATA_WPA			0x0004
#define WFLAG_APDATA_COMPATIBLE		0x0008
#define WFLAG_APDATA_EXTCOMPATIBLE	0x0010
#define WFLAG_APDATA_SHORTPREAMBLE	0x0020
#define WFLAG_APDATA_ACTIVE			0x8000

typedef struct WIFI_ACCESSPOINT {
	char ssid[33];		// 0-32byte data, zero
	char ssid_len;
	u8 bssid[6];
	u8 macaddr[6];
	u16 maxrate;		// max rate is measured in steps of 1/2Mbit - 5.5Mbit will be represented as 11, or 0x0B
	u32 timectr;
	u16 rssi;
	u16 flags;
/*
	u32 spinlock;
*/
	u8 channel;
	u8 rssi_past[8];
	u8 base_rates[16];	// terminated by a 0 entry
} Wifi_AccessPoint;

typedef struct WIFI_RXHEADER {
	u16 a;
	u16 b;
	u16 c;
	u16 d;
	u16 byteLength;
	u16 rssi_;
} Wifi_RxHeader;
