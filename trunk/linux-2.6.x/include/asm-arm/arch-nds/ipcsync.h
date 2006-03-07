/*
 * include/asm-armnommu/arch-nds/ipcsync.h
 *
 * Copyright 2005 Stefan Sperling <stsp@stsp.in-berlin.de>
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

/* 
 * This file defines an interface to the inter-processor-communication
 * register of the Nintendo DS.
 */

#ifndef __ASM_ARM_ARCH_IPCSYNC_H
#define __ASM_ARM_ARCH_IPCSYNC_H

#include <linux/types.h>
#include <linux/list.h>

/* You should never need to touch this register directly. */
#define REG_IPCSYNC	(*(u16*)0x04000180)

/* Allows the other CPU to interrupt local CPU for ipcsync.
 * This is called during ipcsync initialization. */
static inline void ipcsync_allow_local_interrupt(void)
{
	REG_IPCSYNC |= (1 << 14);
}

/* You don't need this to use ipcsync. */
static inline void __ipcsync_disallow_local_interrupt(void)
{
	REG_IPCSYNC &= ~(1 << 14);
}

/* Use these to get local and remote status codes. */
static inline u8 ipcsync_get_local_status(void)
{
	return (REG_IPCSYNC & 0xf00);
}

static inline u8 ipcsync_get_remote_status(void)
{
	return (REG_IPCSYNC & 0xf);
}

/* You do not need this. See ipcsync_trigger_remote_interrupt() below. */
static inline void __ipcsync_set_local_status(u8 status)
{
	__ipcsync_disallow_local_interrupt();
	REG_IPCSYNC &= ~0xf00;
	REG_IPCSYNC |= ((status << 8) & 0xf00);
	ipcsync_allow_local_interrupt();
}

/* Triggers an interrupt with local status 'status' on the remote CPU. */
static inline void ipcsync_trigger_remote_interrupt(u8 status)
{
	__ipcsync_set_local_status(status);
	REG_IPCSYNC |= (1 << 13);
}

/* Interrupt types. We have 4 bits for the type, so up to 16 distinct types. */
/* These types are used by the shared memory ipc framework. */
#define SHMEMIPC_REQUEST_FLUSH	1
#define SHMEMIPC_FLUSH_COMPLETE	2

/* Callbacks will be called on ARM7 interrupt if their type matches the
 * one specified by the ARM7 in REG_IPCSYNC. */
struct ipcsync_cb {
	struct list_head list;
	u8 type; /* One of the types defined above. */
	union
	{
		void (*shmemipc_serve_flush_request)(void);
		void (*shmemipc_flush_complete)(void);
		/* ... */
	} handler;
};

int register_ipcsync_cb(struct ipcsync_cb *callback);
int unregister_ipcsync_cb(struct ipcsync_cb *callback);

#endif /* __ASM_ARM_ARCH_IPCSYNC_H */
