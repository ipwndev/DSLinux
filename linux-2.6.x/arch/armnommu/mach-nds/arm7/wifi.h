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

/* match with drivers/net/wireless/nds.h */
enum WIFI_CMDS {
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

enum WIFI_AP_MODE {
	WIFI_AP_INFRA,
	WIFI_AP_ADHOC,
};

enum WEPMODES {
	WEPMODE_NONE = 0,
	WEPMODE_40BIT = 1,
	WEPMODE_128BIT = 3
};

enum WIFI_STATE {
	WIFI_STATE_UP = 0x0001,
	WIFI_STATE_ASSOCIATED = 0x0002,
	WIFI_STATE_ASSOCIATING = 0x0004,
	WIFI_STATE_CANNOTASSOCIATE = 0x0008,
	WIFI_STATE_AUTHENTICATED = 0x0010,
	WIFI_STATE_SAW_TX_ERR = 0x0020,

	WIFI_STATE_TXPENDING = 0x0100,
	WIFI_STATE_RXPENDING = 0x0200,
	WIFI_STATE_APQUERYPEND = 0x0400,
	WIFI_STATE_APQUERYSENDING = 0x0800,
	WIFI_STATE_STATSQUERYPEND = 0x1000,
	WIFI_STATE_STATSQUERYSEND = 0x2000,
};

/* match with drivers/net/wireless/nds.h */
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

/*
 * Memory layout
 *
 * 0x4000 - 0x5FFF (8192 bytes)
 *
 * Recieve buffer 0x4C20 -- 0x5F5F  (4928 bytes)
 *
 * Our MTU 1500 bytes
 * 802.11 headers: 48 bytes (40 + 8)
 * 
 * Transmit buffers 0x4000 - 0x4C1F (3104 bytes)
 * Each transmit buffer, 1552
 *
 * Transmit buffer1 0x4000 - 0x460F
 * Transmit buffer2 0x4610 - 0x4C1F
 */
#define RX_START_SETUP 0x4C20
#define RX_END_SETUP   0x5F60
#define RX_CSR_SETUP   0x0610
#define TX_MTU_BYTES   1500
#define TX1_SETUP_VAL  (0x8000 | ((0x4000 - 0x4000) >> 1))
#define TX1_MAC_START  0x4000
#define TX2_SETUP_VAL  (0x8000 | ((0x44DC - 0x4000) >> 1))
#define TX2_MAC_START  0x44DC

#define MAX_KEY_SIZE 13		// 128 (?) bits

#define WIFI_MAX_AP			32

#define WFLAG_PACKET_DATA		0x0001
#define WFLAG_PACKET_MGT		0x0002
#define WFLAG_PACKET_BEACON		0x0004
#define WFLAG_PACKET_CTRL		0x0008

#define WFLAG_PACKET_ALL		0xFFFF

#define WFLAG_APDATA_ADHOC			0x0001
#define WFLAG_APDATA_WEP			0x0002
#define WFLAG_APDATA_WPA			0x0004
#define WFLAG_APDATA_COMPATIBLE		0x0008
#define WFLAG_APDATA_EXTCOMPATIBLE	0x0010
#define WFLAG_APDATA_SHORTPREAMBLE	0x0020
#define WFLAG_APDATA_ACTIVE			0x8000

typedef struct WIFI_TXHEADER {
	u16 enable_flags;
	u16 unknown;
	u16 countup;
	u16 beaconfreq;
	u16 tx_rate;
	u16 tx_length;
} Wifi_TxHeader;

typedef struct WIFI_RXHEADER {
	u16 a;
	u16 b;
	u16 c;
	u16 d;
	u16 byteLength;
	u16 rssi_;
} Wifi_RxHeader;

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

typedef struct Wifi_Data_Struct {

	u16 curChannel, reqChannel;
	u16 curMode, reqMode;
	u16 authlevel, authctr;
	u8 curWepmode, reqWepMode;
	char ssid[34];
	u8 baserates[16];

	u16 state;
	u16 ap_query_bank;

	u8 MacAddr[6];
	u8 bssid[6];
	u8 apmac[6];
	u8 wepkey[4][MAX_KEY_SIZE + 1];
	u16 maxrate;
	u16 wepkeyid;

	u8 FlashData[512];

	u32 stats[WIFI_STATS_MAX];

	Wifi_AccessPoint aplist[WIFI_MAX_AP];
} Wifi_Data;

void wifi_init(void);
void wifi_open(void);
void wifi_close(void);
void wifi_mac_query(void);
void wifi_interupt(void);
void wifi_send_ether_packet(u16 length, u_char * data);
void wifi_stats_query(void);

void Wifi_SetChannel(int channel);
void Wifi_SetWepKey(int key, int off, u8 b1, u8 b2);
void Wifi_SetWepKeyID(int key);
void Wifi_SetWepMode(int wepmode);
void Wifi_SetSSID(int off, char b1, char b2);

void wifi_ap_query(u16 bank);
void wifi_start_scan(void);
void wifi_tx_q_complete(void);
void wifi_stats_query_complete(void);
void wifi_ap_query_complete(void);
