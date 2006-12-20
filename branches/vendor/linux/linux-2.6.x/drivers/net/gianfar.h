/*
 * drivers/net/gianfar.h
 *
 * Gianfar Ethernet Driver
 * Driver for FEC on MPC8540 and TSEC on MPC8540/MPC8560
 * Based on 8260_io/fcc_enet.c
 *
 * Author: Andy Fleming
 * Maintainer: Kumar Gala (kumar.gala@freescale.com)
 *
 * Copyright (c) 2002-2004 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 *  Still left to do:
 *      -Add support for module parameters
 *	-Add support for ethtool -s
 *	-Add patch for ethtool phys id
 */
#ifndef __GIANFAR_H
#define __GIANFAR_H

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/fsl_devices.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/crc32.h>
#include <linux/workqueue.h>
#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include "gianfar_phy.h"

/* The maximum number of packets to be handled in one call of gfar_poll */
#define GFAR_DEV_WEIGHT 64

/* Length for FCB */
#define GMAC_FCB_LEN 8

/* Default padding amount */
#define DEFAULT_PADDING 2

/* Number of bytes to align the rx bufs to */
#define RXBUF_ALIGNMENT 64

/* The number of bytes which composes a unit for the purpose of
 * allocating data buffers.  ie-for any given MTU, the data buffer
 * will be the next highest multiple of 512 bytes. */
#define INCREMENTAL_BUFFER_SIZE 512


#define MAC_ADDR_LEN 6

#define PHY_INIT_TIMEOUT 100000
#define GFAR_PHY_CHANGE_TIME 2

#define DEVICE_NAME "%s: Gianfar Ethernet Controller Version 1.1, "
#define DRV_NAME "gfar-enet"
extern const char gfar_driver_name[];
extern const char gfar_driver_version[];

/* These need to be powers of 2 for this driver */
#ifdef CONFIG_GFAR_NAPI
#define DEFAULT_TX_RING_SIZE	256
#define DEFAULT_RX_RING_SIZE	256
#else
#define DEFAULT_TX_RING_SIZE    64
#define DEFAULT_RX_RING_SIZE    64
#endif

#define GFAR_RX_MAX_RING_SIZE   256
#define GFAR_TX_MAX_RING_SIZE   256

#define DEFAULT_RX_BUFFER_SIZE  1536
#define TX_RING_MOD_MASK(size) (size-1)
#define RX_RING_MOD_MASK(size) (size-1)
#define JUMBO_BUFFER_SIZE 9728
#define JUMBO_FRAME_SIZE 9600

/* Latency of interface clock in nanoseconds */
/* Interface clock latency , in this case, means the
 * time described by a value of 1 in the interrupt
 * coalescing registers' time fields.  Since those fields
 * refer to the time it takes for 64 clocks to pass, the
 * latencies are as such:
 * GBIT = 125MHz => 8ns/clock => 8*64 ns / tick
 * 100 = 25 MHz => 40ns/clock => 40*64 ns / tick
 * 10 = 2.5 MHz => 400ns/clock => 400*64 ns / tick
 */
#define GFAR_GBIT_TIME  512
#define GFAR_100_TIME   2560
#define GFAR_10_TIME    25600

#define DEFAULT_TX_COALESCE 1
#define DEFAULT_TXCOUNT	16
#define DEFAULT_TXTIME	400

#define DEFAULT_RX_COALESCE 1
#define DEFAULT_RXCOUNT	16
#define DEFAULT_RXTIME	400

#define TBIPA_VALUE		0x1f
#define MIIMCFG_INIT_VALUE	0x00000007
#define MIIMCFG_RESET           0x80000000
#define MIIMIND_BUSY            0x00000001

/* MAC register bits */
#define MACCFG1_SOFT_RESET	0x80000000
#define MACCFG1_RESET_RX_MC	0x00080000
#define MACCFG1_RESET_TX_MC	0x00040000
#define MACCFG1_RESET_RX_FUN	0x00020000
#define	MACCFG1_RESET_TX_FUN	0x00010000
#define MACCFG1_LOOPBACK	0x00000100
#define MACCFG1_RX_FLOW		0x00000020
#define MACCFG1_TX_FLOW		0x00000010
#define MACCFG1_SYNCD_RX_EN	0x00000008
#define MACCFG1_RX_EN		0x00000004
#define MACCFG1_SYNCD_TX_EN	0x00000002
#define MACCFG1_TX_EN		0x00000001

#define MACCFG2_INIT_SETTINGS	0x00007205
#define MACCFG2_FULL_DUPLEX	0x00000001
#define MACCFG2_IF              0x00000300
#define MACCFG2_MII             0x00000100
#define MACCFG2_GMII            0x00000200
#define MACCFG2_HUGEFRAME	0x00000020
#define MACCFG2_LENGTHCHECK	0x00000010

#define ECNTRL_INIT_SETTINGS	0x00001000
#define ECNTRL_TBI_MODE         0x00000020

#define MRBLR_INIT_SETTINGS	DEFAULT_RX_BUFFER_SIZE

#define MINFLR_INIT_SETTINGS	0x00000040

/* Init to do tx snooping for buffers and descriptors */
#define DMACTRL_INIT_SETTINGS   0x000000c3
#define DMACTRL_GRS             0x00000010
#define DMACTRL_GTS             0x00000008

#define TSTAT_CLEAR_THALT       0x80000000

/* Interrupt coalescing macros */
#define IC_ICEN			0x80000000
#define IC_ICFT_MASK		0x1fe00000
#define IC_ICFT_SHIFT		21
#define mk_ic_icft(x)		\
	(((unsigned int)x << IC_ICFT_SHIFT)&IC_ICFT_MASK)
#define IC_ICTT_MASK		0x0000ffff
#define mk_ic_ictt(x)		(x&IC_ICTT_MASK)

#define mk_ic_value(count, time) (IC_ICEN | \
				mk_ic_icft(count) | \
				mk_ic_ictt(time))

#define RCTRL_PAL_MASK		0x001f0000
#define RCTRL_VLEX		0x00002000
#define RCTRL_FILREN		0x00001000
#define RCTRL_GHTX		0x00000400
#define RCTRL_IPCSEN		0x00000200
#define RCTRL_TUCSEN		0x00000100
#define RCTRL_PRSDEP_MASK	0x000000c0
#define RCTRL_PRSDEP_INIT	0x000000c0
#define RCTRL_PROM		0x00000008
#define RCTRL_CHECKSUMMING	(RCTRL_IPCSEN \
		| RCTRL_TUCSEN | RCTRL_PRSDEP_INIT)
#define RCTRL_EXTHASH		(RCTRL_GHTX)
#define RCTRL_VLAN		(RCTRL_PRSDEP_INIT)


#define RSTAT_CLEAR_RHALT       0x00800000

#define TCTRL_IPCSEN		0x00004000
#define TCTRL_TUCSEN		0x00002000
#define TCTRL_VLINS		0x00001000
#define TCTRL_INIT_CSUM		(TCTRL_TUCSEN | TCTRL_IPCSEN)

#define IEVENT_INIT_CLEAR	0xffffffff
#define IEVENT_BABR		0x80000000
#define IEVENT_RXC		0x40000000
#define IEVENT_BSY		0x20000000
#define IEVENT_EBERR		0x10000000
#define IEVENT_MSRO		0x04000000
#define IEVENT_GTSC		0x02000000
#define IEVENT_BABT		0x01000000
#define IEVENT_TXC		0x00800000
#define IEVENT_TXE		0x00400000
#define IEVENT_TXB		0x00200000
#define IEVENT_TXF		0x00100000
#define IEVENT_LC		0x00040000
#define IEVENT_CRL		0x00020000
#define IEVENT_XFUN		0x00010000
#define IEVENT_RXB0		0x00008000
#define IEVENT_GRSC		0x00000100
#define IEVENT_RXF0		0x00000080
#define IEVENT_FIR		0x00000008
#define IEVENT_FIQ		0x00000004
#define IEVENT_DPE		0x00000002
#define IEVENT_PERR		0x00000001
#define IEVENT_RX_MASK          (IEVENT_RXB0 | IEVENT_RXF0)
#define IEVENT_TX_MASK          (IEVENT_TXB | IEVENT_TXF)
#define IEVENT_ERR_MASK         \
(IEVENT_RXC | IEVENT_BSY | IEVENT_EBERR | IEVENT_MSRO | \
 IEVENT_BABT | IEVENT_TXC | IEVENT_TXE | IEVENT_LC \
 | IEVENT_CRL | IEVENT_XFUN | IEVENT_DPE | IEVENT_PERR)

#define IMASK_INIT_CLEAR	0x00000000
#define IMASK_BABR              0x80000000
#define IMASK_RXC               0x40000000
#define IMASK_BSY               0x20000000
#define IMASK_EBERR             0x10000000
#define IMASK_MSRO		0x04000000
#define IMASK_GRSC              0x02000000
#define IMASK_BABT		0x01000000
#define IMASK_TXC               0x00800000
#define IMASK_TXEEN		0x00400000
#define IMASK_TXBEN		0x00200000
#define IMASK_TXFEN             0x00100000
#define IMASK_LC		0x00040000
#define IMASK_CRL		0x00020000
#define IMASK_XFUN		0x00010000
#define IMASK_RXB0              0x00008000
#define IMASK_GTSC              0x00000100
#define IMASK_RXFEN0		0x00000080
#define IMASK_FIR		0x00000008
#define IMASK_FIQ		0x00000004
#define IMASK_DPE		0x00000002
#define IMASK_PERR		0x00000001
#define IMASK_RX_DISABLED ~(IMASK_RXFEN0 | IMASK_BSY)
#define IMASK_DEFAULT  (IMASK_TXEEN | IMASK_TXFEN | IMASK_TXBEN | \
		IMASK_RXFEN0 | IMASK_BSY | IMASK_EBERR | IMASK_BABR | \
		IMASK_XFUN | IMASK_RXC | IMASK_BABT | IMASK_DPE \
		| IMASK_PERR)


/* Attribute fields */

/* This enables rx snooping for buffers and descriptors */
#ifdef CONFIG_GFAR_BDSTASH
#define ATTR_BDSTASH		0x00000800
#else
#define ATTR_BDSTASH		0x00000000
#endif

#ifdef CONFIG_GFAR_BUFSTASH
#define ATTR_BUFSTASH		0x00004000
#define STASH_LENGTH		64
#else
#define ATTR_BUFSTASH		0x00000000
#endif

#define ATTR_SNOOPING		0x000000c0
#define ATTR_INIT_SETTINGS      (ATTR_SNOOPING \
		| ATTR_BDSTASH | ATTR_BUFSTASH)

#define ATTRELI_INIT_SETTINGS   0x0


/* TxBD status field bits */
#define TXBD_READY		0x8000
#define TXBD_PADCRC		0x4000
#define TXBD_WRAP		0x2000
#define TXBD_INTERRUPT		0x1000
#define TXBD_LAST		0x0800
#define TXBD_CRC		0x0400
#define TXBD_DEF		0x0200
#define TXBD_HUGEFRAME		0x0080
#define TXBD_LATECOLLISION	0x0080
#define TXBD_RETRYLIMIT		0x0040
#define	TXBD_RETRYCOUNTMASK	0x003c
#define TXBD_UNDERRUN		0x0002
#define TXBD_TOE		0x0002

/* Tx FCB param bits */
#define TXFCB_VLN		0x80
#define TXFCB_IP		0x40
#define TXFCB_IP6		0x20
#define TXFCB_TUP		0x10
#define TXFCB_UDP		0x08
#define TXFCB_CIP		0x04
#define TXFCB_CTU		0x02
#define TXFCB_NPH		0x01
#define TXFCB_DEFAULT 		(TXFCB_IP|TXFCB_TUP|TXFCB_CTU|TXFCB_NPH)

/* RxBD status field bits */
#define RXBD_EMPTY		0x8000
#define RXBD_RO1		0x4000
#define RXBD_WRAP		0x2000
#define RXBD_INTERRUPT		0x1000
#define RXBD_LAST		0x0800
#define RXBD_FIRST		0x0400
#define RXBD_MISS		0x0100
#define RXBD_BROADCAST		0x0080
#define RXBD_MULTICAST		0x0040
#define RXBD_LARGE		0x0020
#define RXBD_NONOCTET		0x0010
#define RXBD_SHORT		0x0008
#define RXBD_CRCERR		0x0004
#define RXBD_OVERRUN		0x0002
#define RXBD_TRUNCATED		0x0001
#define RXBD_STATS		0x01ff

/* Rx FCB status field bits */
#define RXFCB_VLN		0x8000
#define RXFCB_IP		0x4000
#define RXFCB_IP6		0x2000
#define RXFCB_TUP		0x1000
#define RXFCB_CIP		0x0800
#define RXFCB_CTU		0x0400
#define RXFCB_EIP		0x0200
#define RXFCB_ETU		0x0100
#define RXFCB_PERR_MASK		0x000c
#define RXFCB_PERR_BADL3	0x0008

struct txbd8
{
	u16	status;	/* Status Fields */
	u16	length;	/* Buffer length */
	u32	bufPtr;	/* Buffer Pointer */
};

struct txfcb {
	u8	vln:1,
		ip:1,
		ip6:1,
		tup:1,
		udp:1,
		cip:1,
		ctu:1,
		nph:1;
	u8	reserved;
	u8	l4os;	/* Level 4 Header Offset */
	u8	l3os; 	/* Level 3 Header Offset */
	u16	phcs;	/* Pseudo-header Checksum */
	u16	vlctl;	/* VLAN control word */
};

struct rxbd8
{
	u16	status;	/* Status Fields */
	u16	length;	/* Buffer Length */
	u32	bufPtr;	/* Buffer Pointer */
};

struct rxfcb {
	u16	vln:1,
		ip:1,
		ip6:1,
		tup:1,
		cip:1,
		ctu:1,
		eip:1,
		etu:1;
	u8	rq;	/* Receive Queue index */
	u8	pro;	/* Layer 4 Protocol */
	u16	reserved;
	u16	vlctl;	/* VLAN control word */
};

struct rmon_mib
{
	u32	tr64;	/* 0x.680 - Transmit and Receive 64-byte Frame Counter */
	u32	tr127;	/* 0x.684 - Transmit and Receive 65-127 byte Frame Counter */
	u32	tr255;	/* 0x.688 - Transmit and Receive 128-255 byte Frame Counter */
	u32	tr511;	/* 0x.68c - Transmit and Receive 256-511 byte Frame Counter */
	u32	tr1k;	/* 0x.690 - Transmit and Receive 512-1023 byte Frame Counter */
	u32	trmax;	/* 0x.694 - Transmit and Receive 1024-1518 byte Frame Counter */
	u32	trmgv;	/* 0x.698 - Transmit and Receive 1519-1522 byte Good VLAN Frame */
	u32	rbyt;	/* 0x.69c - Receive Byte Counter */
	u32	rpkt;	/* 0x.6a0 - Receive Packet Counter */
	u32	rfcs;	/* 0x.6a4 - Receive FCS Error Counter */
	u32	rmca;	/* 0x.6a8 - Receive Multicast Packet Counter */
	u32	rbca;	/* 0x.6ac - Receive Broadcast Packet Counter */
	u32	rxcf;	/* 0x.6b0 - Receive Control Frame Packet Counter */
	u32	rxpf;	/* 0x.6b4 - Receive Pause Frame Packet Counter */
	u32	rxuo;	/* 0x.6b8 - Receive Unknown OP Code Counter */
	u32	raln;	/* 0x.6bc - Receive Alignment Error Counter */
	u32	rflr;	/* 0x.6c0 - Receive Frame Length Error Counter */
	u32	rcde;	/* 0x.6c4 - Receive Code Error Counter */
	u32	rcse;	/* 0x.6c8 - Receive Carrier Sense Error Counter */
	u32	rund;	/* 0x.6cc - Receive Undersize Packet Counter */
	u32	rovr;	/* 0x.6d0 - Receive Oversize Packet Counter */
	u32	rfrg;	/* 0x.6d4 - Receive Fragments Counter */
	u32	rjbr;	/* 0x.6d8 - Receive Jabber Counter */
	u32	rdrp;	/* 0x.6dc - Receive Drop Counter */
	u32	tbyt;	/* 0x.6e0 - Transmit Byte Counter Counter */
	u32	tpkt;	/* 0x.6e4 - Transmit Packet Counter */
	u32	tmca;	/* 0x.6e8 - Transmit Multicast Packet Counter */
	u32	tbca;	/* 0x.6ec - Transmit Broadcast Packet Counter */
	u32	txpf;	/* 0x.6f0 - Transmit Pause Control Frame Counter */
	u32	tdfr;	/* 0x.6f4 - Transmit Deferral Packet Counter */
	u32	tedf;	/* 0x.6f8 - Transmit Excessive Deferral Packet Counter */
	u32	tscl;	/* 0x.6fc - Transmit Single Collision Packet Counter */
	u32	tmcl;	/* 0x.700 - Transmit Multiple Collision Packet Counter */
	u32	tlcl;	/* 0x.704 - Transmit Late Collision Packet Counter */
	u32	txcl;	/* 0x.708 - Transmit Excessive Collision Packet Counter */
	u32	tncl;	/* 0x.70c - Transmit Total Collision Counter */
	u8	res1[4];
	u32	tdrp;	/* 0x.714 - Transmit Drop Frame Counter */
	u32	tjbr;	/* 0x.718 - Transmit Jabber Frame Counter */
	u32	tfcs;	/* 0x.71c - Transmit FCS Error Counter */
	u32	txcf;	/* 0x.720 - Transmit Control Frame Counter */
	u32	tovr;	/* 0x.724 - Transmit Oversize Frame Counter */
	u32	tund;	/* 0x.728 - Transmit Undersize Frame Counter */
	u32	tfrg;	/* 0x.72c - Transmit Fragments Frame Counter */
	u32	car1;	/* 0x.730 - Carry Register One */
	u32	car2;	/* 0x.734 - Carry Register Two */
	u32	cam1;	/* 0x.738 - Carry Mask Register One */
	u32	cam2;	/* 0x.73c - Carry Mask Register Two */
};

struct gfar_extra_stats {
	u64 kernel_dropped;
	u64 rx_large;
	u64 rx_short;
	u64 rx_nonoctet;
	u64 rx_crcerr;
	u64 rx_overrun;
	u64 rx_bsy;
	u64 rx_babr;
	u64 rx_trunc;
	u64 eberr;
	u64 tx_babt;
	u64 tx_underrun;
	u64 rx_skbmissing;
	u64 tx_timeout;
};

#define GFAR_RMON_LEN ((sizeof(struct rmon_mib) - 16)/sizeof(u32))
#define GFAR_EXTRA_STATS_LEN (sizeof(struct gfar_extra_stats)/sizeof(u64))

/* Number of stats in the stats structure (ignore car and cam regs)*/
#define GFAR_STATS_LEN (GFAR_RMON_LEN + GFAR_EXTRA_STATS_LEN)

#define GFAR_INFOSTR_LEN 32

struct gfar_stats {
	u64 extra[GFAR_EXTRA_STATS_LEN];
	u64 rmon[GFAR_RMON_LEN];
};


struct gfar {
	u32	tsec_id;	/* 0x.000 - Controller ID register */
	u8	res1[12];
	u32	ievent;		/* 0x.010 - Interrupt Event Register */
	u32	imask;		/* 0x.014 - Interrupt Mask Register */
	u32	edis;		/* 0x.018 - Error Disabled Register */
	u8	res2[4];
	u32	ecntrl;		/* 0x.020 - Ethernet Control Register */
	u32	minflr;		/* 0x.024 - Minimum Frame Length Register */
	u32	ptv;		/* 0x.028 - Pause Time Value Register */
	u32	dmactrl;	/* 0x.02c - DMA Control Register */
	u32	tbipa;		/* 0x.030 - TBI PHY Address Register */
	u8	res3[88];
	u32	fifo_tx_thr;	/* 0x.08c - FIFO transmit threshold register */
	u8	res4[8];
	u32	fifo_tx_starve;	/* 0x.098 - FIFO transmit starve register */
	u32	fifo_tx_starve_shutoff;	/* 0x.09c - FIFO transmit starve shutoff register */
	u8	res5[4];
	u32	fifo_rx_pause;	/* 0x.0a4 - FIFO receive pause threshold register */
	u32	fifo_rx_alarm;	/* 0x.0a8 - FIFO receive alarm threshold register */
	u8	res6[84];
	u32	tctrl;		/* 0x.100 - Transmit Control Register */
	u32	tstat;		/* 0x.104 - Transmit Status Register */
	u32	dfvlan;		/* 0x.108 - Default VLAN Control word */
	u32	tbdlen;		/* 0x.10c - Transmit Buffer Descriptor Data Length Register */
	u32	txic;		/* 0x.110 - Transmit Interrupt Coalescing Configuration Register */
	u32	tqueue;		/* 0x.114 - Transmit queue control register */
	u8	res7[40];
	u32	tr03wt;		/* 0x.140 - TxBD Rings 0-3 round-robin weightings */
	u32	tr47wt;		/* 0x.144 - TxBD Rings 4-7 round-robin weightings */
	u8	res8[52];
	u32	tbdbph;		/* 0x.17c - Tx data buffer pointer high */
	u8	res9a[4];
	u32	tbptr0;		/* 0x.184 - TxBD Pointer for ring 0 */
	u8	res9b[4];
	u32	tbptr1;		/* 0x.18c - TxBD Pointer for ring 1 */
	u8	res9c[4];
	u32	tbptr2;		/* 0x.194 - TxBD Pointer for ring 2 */
	u8	res9d[4];
	u32	tbptr3;		/* 0x.19c - TxBD Pointer for ring 3 */
	u8	res9e[4];
	u32	tbptr4;		/* 0x.1a4 - TxBD Pointer for ring 4 */
	u8	res9f[4];
	u32	tbptr5;		/* 0x.1ac - TxBD Pointer for ring 5 */
	u8	res9g[4];
	u32	tbptr6;		/* 0x.1b4 - TxBD Pointer for ring 6 */
	u8	res9h[4];
	u32	tbptr7;		/* 0x.1bc - TxBD Pointer for ring 7 */
	u8	res9[64];
	u32	tbaseh;		/* 0x.200 - TxBD base address high */
	u32	tbase0;		/* 0x.204 - TxBD Base Address of ring 0 */
	u8	res10a[4];
	u32	tbase1;		/* 0x.20c - TxBD Base Address of ring 1 */
	u8	res10b[4];
	u32	tbase2;		/* 0x.214 - TxBD Base Address of ring 2 */
	u8	res10c[4];
	u32	tbase3;		/* 0x.21c - TxBD Base Address of ring 3 */
	u8	res10d[4];
	u32	tbase4;		/* 0x.224 - TxBD Base Address of ring 4 */
	u8	res10e[4];
	u32	tbase5;		/* 0x.22c - TxBD Base Address of ring 5 */
	u8	res10f[4];
	u32	tbase6;		/* 0x.234 - TxBD Base Address of ring 6 */
	u8	res10g[4];
	u32	tbase7;		/* 0x.23c - TxBD Base Address of ring 7 */
	u8	res10[192];
	u32	rctrl;		/* 0x.300 - Receive Control Register */
	u32	rstat;		/* 0x.304 - Receive Status Register */
	u8	res12[8];
	u32	rxic;		/* 0x.310 - Receive Interrupt Coalescing Configuration Register */
	u32	rqueue;		/* 0x.314 - Receive queue control register */
	u8	res13[24];
	u32	rbifx;		/* 0x.330 - Receive bit field extract control register */
	u32	rqfar;		/* 0x.334 - Receive queue filing table address register */
	u32	rqfcr;		/* 0x.338 - Receive queue filing table control register */
	u32	rqfpr;		/* 0x.33c - Receive queue filing table property register */
	u32	mrblr;		/* 0x.340 - Maximum Receive Buffer Length Register */
	u8	res14[56];
	u32	rbdbph;		/* 0x.37c - Rx data buffer pointer high */
	u8	res15a[4];
	u32	rbptr0;		/* 0x.384 - RxBD pointer for ring 0 */
	u8	res15b[4];
	u32	rbptr1;		/* 0x.38c - RxBD pointer for ring 1 */
	u8	res15c[4];
	u32	rbptr2;		/* 0x.394 - RxBD pointer for ring 2 */
	u8	res15d[4];
	u32	rbptr3;		/* 0x.39c - RxBD pointer for ring 3 */
	u8	res15e[4];
	u32	rbptr4;		/* 0x.3a4 - RxBD pointer for ring 4 */
	u8	res15f[4];
	u32	rbptr5;		/* 0x.3ac - RxBD pointer for ring 5 */
	u8	res15g[4];
	u32	rbptr6;		/* 0x.3b4 - RxBD pointer for ring 6 */
	u8	res15h[4];
	u32	rbptr7;		/* 0x.3bc - RxBD pointer for ring 7 */
	u8	res16[64];
	u32	rbaseh;		/* 0x.400 - RxBD base address high */
	u32	rbase0;		/* 0x.404 - RxBD base address of ring 0 */
	u8	res17a[4];
	u32	rbase1;		/* 0x.40c - RxBD base address of ring 1 */
	u8	res17b[4];
	u32	rbase2;		/* 0x.414 - RxBD base address of ring 2 */
	u8	res17c[4];
	u32	rbase3;		/* 0x.41c - RxBD base address of ring 3 */
	u8	res17d[4];
	u32	rbase4;		/* 0x.424 - RxBD base address of ring 4 */
	u8	res17e[4];
	u32	rbase5;		/* 0x.42c - RxBD base address of ring 5 */
	u8	res17f[4];
	u32	rbase6;		/* 0x.434 - RxBD base address of ring 6 */
	u8	res17g[4];
	u32	rbase7;		/* 0x.43c - RxBD base address of ring 7 */
	u8	res17[192];
	u32	maccfg1;	/* 0x.500 - MAC Configuration 1 Register */
	u32	maccfg2;	/* 0x.504 - MAC Configuration 2 Register */
	u32	ipgifg;		/* 0x.508 - Inter Packet Gap/Inter Frame Gap Register */
	u32	hafdup;		/* 0x.50c - Half Duplex Register */
	u32	maxfrm;		/* 0x.510 - Maximum Frame Length Register */
	u8	res18[12];
	u32	miimcfg;	/* 0x.520 - MII Management Configuration Register */
	u32	miimcom;	/* 0x.524 - MII Management Command Register */
	u32	miimadd;	/* 0x.528 - MII Management Address Register */
	u32	miimcon;	/* 0x.52c - MII Management Control Register */
	u32	miimstat;	/* 0x.530 - MII Management Status Register */
	u32	miimind;	/* 0x.534 - MII Management Indicator Register */
	u8	res19[4];
	u32	ifstat;		/* 0x.53c - Interface Status Register */
	u32	macstnaddr1;	/* 0x.540 - Station Address Part 1 Register */
	u32	macstnaddr2;	/* 0x.544 - Station Address Part 2 Register */
	u32	mac01addr1;	/* 0x.548 - MAC exact match address 1, part 1 */
	u32	mac01addr2;	/* 0x.54c - MAC exact match address 1, part 2 */
	u32	mac02addr1;	/* 0x.550 - MAC exact match address 2, part 1 */
	u32	mac02addr2;	/* 0x.554 - MAC exact match address 2, part 2 */
	u32	mac03addr1;	/* 0x.558 - MAC exact match address 3, part 1 */
	u32	mac03addr2;	/* 0x.55c - MAC exact match address 3, part 2 */
	u32	mac04addr1;	/* 0x.560 - MAC exact match address 4, part 1 */
	u32	mac04addr2;	/* 0x.564 - MAC exact match address 4, part 2 */
	u32	mac05addr1;	/* 0x.568 - MAC exact match address 5, part 1 */
	u32	mac05addr2;	/* 0x.56c - MAC exact match address 5, part 2 */
	u32	mac06addr1;	/* 0x.570 - MAC exact match address 6, part 1 */
	u32	mac06addr2;	/* 0x.574 - MAC exact match address 6, part 2 */
	u32	mac07addr1;	/* 0x.578 - MAC exact match address 7, part 1 */
	u32	mac07addr2;	/* 0x.57c - MAC exact match address 7, part 2 */
	u32	mac08addr1;	/* 0x.580 - MAC exact match address 8, part 1 */
	u32	mac08addr2;	/* 0x.584 - MAC exact match address 8, part 2 */
	u32	mac09addr1;	/* 0x.588 - MAC exact match address 9, part 1 */
	u32	mac09addr2;	/* 0x.58c - MAC exact match address 9, part 2 */
	u32	mac10addr1;	/* 0x.590 - MAC exact match address 10, part 1*/
	u32	mac10addr2;	/* 0x.594 - MAC exact match address 10, part 2*/
	u32	mac11addr1;	/* 0x.598 - MAC exact match address 11, part 1*/
	u32	mac11addr2;	/* 0x.59c - MAC exact match address 11, part 2*/
	u32	mac12addr1;	/* 0x.5a0 - MAC exact match address 12, part 1*/
	u32	mac12addr2;	/* 0x.5a4 - MAC exact match address 12, part 2*/
	u32	mac13addr1;	/* 0x.5a8 - MAC exact match address 13, part 1*/
	u32	mac13addr2;	/* 0x.5ac - MAC exact match address 13, part 2*/
	u32	mac14addr1;	/* 0x.5b0 - MAC exact match address 14, part 1*/
	u32	mac14addr2;	/* 0x.5b4 - MAC exact match address 14, part 2*/
	u32	mac15addr1;	/* 0x.5b8 - MAC exact match address 15, part 1*/
	u32	mac15addr2;	/* 0x.5bc - MAC exact match address 15, part 2*/
	u8	res20[192];
	struct rmon_mib	rmon;	/* 0x.680-0x.73c */
	u32	rrej;		/* 0x.740 - Receive filer rejected packet counter */
	u8	res21[188];
	u32	igaddr0;	/* 0x.800 - Indivdual/Group address register 0*/
	u32	igaddr1;	/* 0x.804 - Indivdual/Group address register 1*/
	u32	igaddr2;	/* 0x.808 - Indivdual/Group address register 2*/
	u32	igaddr3;	/* 0x.80c - Indivdual/Group address register 3*/
	u32	igaddr4;	/* 0x.810 - Indivdual/Group address register 4*/
	u32	igaddr5;	/* 0x.814 - Indivdual/Group address register 5*/
	u32	igaddr6;	/* 0x.818 - Indivdual/Group address register 6*/
	u32	igaddr7;	/* 0x.81c - Indivdual/Group address register 7*/
	u8	res22[96];
	u32	gaddr0;		/* 0x.880 - Group address register 0 */
	u32	gaddr1;		/* 0x.884 - Group address register 1 */
	u32	gaddr2;		/* 0x.888 - Group address register 2 */
	u32	gaddr3;		/* 0x.88c - Group address register 3 */
	u32	gaddr4;		/* 0x.890 - Group address register 4 */
	u32	gaddr5;		/* 0x.894 - Group address register 5 */
	u32	gaddr6;		/* 0x.898 - Group address register 6 */
	u32	gaddr7;		/* 0x.89c - Group address register 7 */
	u8	res23a[352];
	u32	fifocfg;	/* 0x.a00 - FIFO interface config register */
	u8	res23b[252];
	u8	res23c[248];
	u32	attr;		/* 0x.bf8 - Attributes Register */
	u32	attreli;	/* 0x.bfc - Attributes Extract Length and Extract Index Register */
	u8	res24[1024];

};

/* Struct stolen almost completely (and shamelessly) from the FCC enet source
 * (Ok, that's not so true anymore, but there is a family resemblence)
 * The GFAR buffer descriptors track the ring buffers.  The rx_bd_base
 * and tx_bd_base always point to the currently available buffer.
 * The dirty_tx tracks the current buffer that is being sent by the
 * controller.  The cur_tx and dirty_tx are equal under both completely
 * empty and completely full conditions.  The empty/ready indicator in
 * the buffer descriptor determines the actual condition.
 */
struct gfar_private {
	/* pointers to arrays of skbuffs for tx and rx */
	struct sk_buff ** tx_skbuff;
	struct sk_buff ** rx_skbuff;

	/* indices pointing to the next free sbk in skb arrays */
	u16 skb_curtx;
	u16 skb_currx;

	/* index of the first skb which hasn't been transmitted
	 * yet. */
	u16 skb_dirtytx;

	/* Configuration info for the coalescing features */
	unsigned char txcoalescing;
	unsigned short txcount;
	unsigned short txtime;
	unsigned char rxcoalescing;
	unsigned short rxcount;
	unsigned short rxtime;

	/* GFAR addresses */
	struct rxbd8 *rx_bd_base;	/* Base addresses of Rx and Tx Buffers */
	struct txbd8 *tx_bd_base;
	struct rxbd8 *cur_rx;           /* Next free rx ring entry */
	struct txbd8 *cur_tx;	        /* Next free ring entry */
	struct txbd8 *dirty_tx;		/* The Ring entry to be freed. */
	struct gfar *regs;	/* Pointer to the GFAR memory mapped Registers */
	u32 *hash_regs[16];
	int hash_width;
	struct gfar *phyregs;
	struct work_struct tq;
	struct timer_list phy_info_timer;
	struct net_device_stats stats; /* linux network statistics */
	struct gfar_extra_stats extra_stats;
	spinlock_t lock;
	unsigned int rx_buffer_size;
	unsigned int rx_stash_size;
	unsigned int tx_ring_size;
	unsigned int rx_ring_size;

	unsigned char vlan_enable:1,
		rx_csum_enable:1,
		extended_hash:1;
	unsigned short padding;
	struct vlan_group *vlgrp;
	/* Info structure initialized by board setup code */
	unsigned int interruptTransmit;
	unsigned int interruptReceive;
	unsigned int interruptError;
	struct gianfar_platform_data *einfo;

	struct gfar_mii_info *mii_info;
	int oldspeed;
	int oldduplex;
	int oldlink;

	uint32_t msg_enable;
};

extern inline u32 gfar_read(volatile unsigned *addr)
{
	u32 val;
	val = in_be32(addr);
	return val;
}

extern inline void gfar_write(volatile unsigned *addr, u32 val)
{
	out_be32(addr, val);
}

extern struct ethtool_ops *gfar_op_array[];

#endif /* __GIANFAR_H */
