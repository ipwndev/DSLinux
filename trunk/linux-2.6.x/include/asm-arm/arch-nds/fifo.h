/*
 * include/asm-armnommu/arch-nds/fifo.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __ASM_ARM_ARCH_FIFO_H
#define __ASM_ARM_ARCH_FIFO_H

#include <linux/kernel.h>
#include <linux/list.h>

/* 
 * Fifo commands are encoded as follows:
 * +--------------------------------------------------------------+
 * |3 bits FIFO_TYPE | 29 bits type data (subcommands, data, ...) |
 * +--------------------------------------------------------------+
 */
#define FIFO_FIRMWARE (0 << 29)
#define FIFO_BUTTONS  (1 << 29)
#define FIFO_TOUCH    (2 << 29)
#define FIFO_MIC      (3 << 29)
#define FIFO_WIFI     (4 << 29)
#define FIFO_SOUND    (5 << 29)
#define FIFO_POWER    (6 << 29)
#define FIFO_TIME     (7 << 29)
#define FIFO_GET_TYPE(x) (((x)) & 0xe0000000)
#define FIFO_GET_TYPE_DATA(x) ((x) & 0x1fffffff)

#define FIFO_HIGH_BITS  (1<<16)
#define FIFO_LOW_BITS   (1<<17)

/* 
 * Fifo commands for firmware dumper.
 * +-------------------------------------------------------------------------+
 * |3 bits FIFO_FIRMWARE | 5 bits FIFO_CMD_FIRMWARE_x | 24 bits command data |
 * +-------------------------------------------------------------------------+
 */
#define FIFO_FIRMWARE_CMD(c, d) (FIFO_FIRMWARE | ((c & 0x1f) << 24) | (d & 0x00ffffff))
#define FIFO_FIRMWARE_GET_CMD(c) ((c >> 24) & 0x1f)
#define FIFO_FIRMWARE_GET_DATA(d) (d & 0x00ffffff)
#define FIFO_FIRMWARE_DECODE_ADDRESS(a) ((a) + 0x02000000)

enum FIFO_FIRMWARE_CMDS {
	FIFO_FIRMWARE_CMD_BUFFER_ADDRESS,
	FIFO_FIRMWARE_CMD_READ
};
	
/* 
 * Fifo wifi commands are encoded as follows:
 * +-----------------------------------------------------------------+
 * |3 bits FIFO_WIFI | 5 bits FIFO_CMD_WIFI_x | 24 bits command data |
 * +-----------------------------------------------------------------+
 *  
 *  How command data is used depends on the command.
 *  Some commands, like recieve and transmit, send offsets into 0x02 RAM
 *  telling the other side where to read packet data from.
 *  Other commands, like those for setting WEP keys or the essid, further
 *  divide the command data into flags and actual data.
 */
#define FIFO_WIFI_CMD(c, d) (FIFO_WIFI | ((c & 0x1f) << 24) | (d & 0x00ffffff))
#define FIFO_WIFI_GET_CMD(c) ((c >> 24) & 0x1f)
#define FIFO_WIFI_GET_DATA(d) (d & 0x00ffffff)
#define FIFO_WIFI_DECODE_ADDRESS(a) ((a) + 0x02000000)

enum FIFO_WIFI_CMDS {
	FIFO_WIFI_CMD_UP,
	FIFO_WIFI_CMD_DOWN,
	FIFO_WIFI_CMD_MAC_QUERY,
	FIFO_WIFI_CMD_TX,
	FIFO_WIFI_CMD_TX_COMPLETE,
	FIFO_WIFI_CMD_RX,
	FIFO_WIFI_CMD_RX_COMPLETE,
	FIFO_WIFI_CMD_STATS_QUERY,
	FIFO_WIFI_CMD_SET_ESSID,
	FIFO_WIFI_CMD_SET_CHANNEL,
	FIFO_WIFI_CMD_SET_WEPKEY,
	FIFO_WIFI_CMD_SET_WEPKEYID,
	FIFO_WIFI_CMD_SET_WEPMODE,
	FIFO_WIFI_CMD_AP_QUERY,
	FIFO_WIFI_CMD_SCAN,
	FIFO_WIFI_CMD_SET_AP_MODE,
	FIFO_WIFI_CMD_GET_AP_MODE,
};

/* 
 * Sound driver commands
 * +-------------------------------------------------------+
 * |3 bits FIFO_SOUND | 1bit unused | 28 bits command data |
 * +-------------------------------------------------------+
 */
#define FIFO_SOUND_CHANNELS	(1<<24)
#define FIFO_SOUND_DMA_ADDRESS	(2<<24)
#define FIFO_SOUND_DMA_SIZE	(3<<24)
#define FIFO_SOUND_FORMAT	(4<<24)
#define FIFO_SOUND_RATE		(5<<24)
#define FIFO_SOUND_TRIGGER	(6<<24)
#define FIFO_SOUND_POWER	(7<<24)

/* FIFO registers */
#define NDS_REG_IPCFIFOSEND (*(volatile u32*) 0x04000188)
#define NDS_REG_IPCFIFORECV (*(volatile u32*) 0x04100000)
#define NDS_REG_IPCFIFOCNT  (*(volatile u16*) 0x04000184)

/* bits in NDS_REG_IPCFIFOCNT */
#define FIFO_SEND_FULL	(1 << 1)
#define FIFO_CLEAR	(1 << 3)
#define FIFO_EMPTY	(1 << 8)
#define FIFO_IRQ_ENABLE (1 << 10)
#define FIFO_ERROR	(1 << 14)
#define FIFO_ENABLE	(1 << 15)

/* FIFO callback functions. */
struct fifo_cb
{
	struct list_head list;
	u32 type ;
	union {
		void (*firmware_handler)(u8 cmd);
		void (*button_handler)(u32 state);
		void (*touch_handler)(u8 pressed, u8 x, u8 y);
		void (*time_handler)(u32 seconds);
		void (*wifi_handler)(u8 cmd, u32 data);
		/* ... */
	} handler;
};

#ifdef __DSLINUX_ARM7__
static inline void nds_fifo_send(u32 command)
{
	while (NDS_REG_IPCFIFOCNT & FIFO_SEND_FULL)
		; /* do nothing */
	NDS_REG_IPCFIFOSEND = command;
}
#else
static inline void nds_fifo_send(u32 command)
{
	int fifo_full = 0;

	while (NDS_REG_IPCFIFOCNT & FIFO_SEND_FULL)
		fifo_full = 1;
	
	NDS_REG_IPCFIFOSEND = command;
	
	if (fifo_full)
		printk(KERN_WARNING "fifo: detected send attempt while "
		    "fifo was full\n");
}
#endif /* __DSLINUX_ARM7__ */

/* Callback register functions. */
int register_fifocb(struct fifo_cb *fifo_cb);
int unregister_fifocb(struct fifo_cb *fifo_cb);

#endif /* __ASM_ARM_ARCH_FIFO_H */
