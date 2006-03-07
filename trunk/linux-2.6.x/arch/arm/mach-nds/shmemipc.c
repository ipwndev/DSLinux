/*
 * Copyright (c) 2005 Stefan Sperling <stsp@stsp.in-berlin.de>
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
 *
 */

/* 
 * This file belongs to the inter-processor-communication framework
 * using shared memory. It uses ipcsync callbacks for communication.
 * Data is transfered via shared memory. See shmemipc.h for a detailed
 * explanation of what this driver is supposed to do.
 *
 * This driver is logically stacked between ipcsync and device drivers
 * using shared memory to communicate with devices behind the ARM7.
 */

#include <linux/module.h>
#include <linux/init.h> 
#include <linux/version.h>
#include <asm/errno.h>
#include <asm/semaphore.h>
#include <asm/arch/shmemipc.h>
#include <linux/completion.h>

#define SHMEMIPC_VERSION_CODE KERNEL_VERSION(0, 2, 0)
/* 
 * Version history:
 * 0.2.0: use completion object to wait until flush is complete
 *        use linux lists
 * 0.1.0: initial version
 */

static int shmemipc_debug = 0;
#define DEBUG(args...) if (shmemipc_debug) printk(args)

/* A flush request times out after SHMEMIPC_FLUSH_TIMEOUT jiffies */
#define SHMEMIPC_FLUSH_TIMEOUT 500

static LIST_HEAD(cblist);
static DECLARE_MUTEX(cblist_mtx);
static DECLARE_MUTEX(flush_mtx);
static DECLARE_COMPLETION(shmemipc_flush_completed);

spinlock_t shmemipc_lock = SPIN_LOCK_UNLOCKED;
unsigned long shmemipc_irq_flags;

static void map_to_arm7(struct shmemipc_block *block);
static void map_to_arm9(struct shmemipc_block *block);

static inline u8 shmemipc_get_block_state(void)
{
	return (REG_WRAMCNT & 0x03);
}

static inline void shmemipc_set_block_state(u8 state)
{
	REG_WRAMCNT &= ~0x03;
	REG_WRAMCNT |= (state & 0x03);
}

#define SHMEMIPC_BLOCK_MAP_BOTH_ARM9	0
#define SHMEMIPC_BLOCK_MAP_NORMAL	1 /* ARM7: block 0, ARM9: block 1 */
#define SHMEMIPC_BLOCK_MAP_REVERSE	2 /* ARM7: block 1, ARM9: block 0 */
#define SHMEMIPC_BLOCK_MAP_BOTH_ARM7	3


static inline int is_block_arm7(struct shmemipc_block *block)
{
	return block == SHMEMIPC_BLOCK_ARM7;
}

static inline int is_block_arm9(struct shmemipc_block *block)
{
	return block == SHMEMIPC_BLOCK_ARM9;
}

/* Map the specified block to the ARM7, if it isn't mapped to it already.
 * Must be called with shmemipc_lock held. */
static void map_to_arm7(struct shmemipc_block *block)
{
	DEBUG("map_to_arm7(%p): before: %i\n", block, (int)shmemipc_get_block_state());
	switch (shmemipc_get_block_state()) {
		case SHMEMIPC_BLOCK_MAP_BOTH_ARM9:
			if (is_block_arm7(block))
				shmemipc_set_block_state(SHMEMIPC_BLOCK_MAP_NORMAL);
			else if (is_block_arm9(block))
				shmemipc_set_block_state(SHMEMIPC_BLOCK_MAP_REVERSE);
			break;	
		case SHMEMIPC_BLOCK_MAP_NORMAL:
			if (is_block_arm9(block))
				shmemipc_set_block_state(SHMEMIPC_BLOCK_MAP_BOTH_ARM7);
			break;
		case SHMEMIPC_BLOCK_MAP_REVERSE:
			if (is_block_arm7(block))
				shmemipc_set_block_state(SHMEMIPC_BLOCK_MAP_BOTH_ARM7);
			break;
		case SHMEMIPC_BLOCK_MAP_BOTH_ARM7:
			break;	
	}
	DEBUG("map_to_arm7(%p): after: %i\n", block, (int)shmemipc_get_block_state());
}

/* Map the specified block to the ARM9, if it isn't mapped to it already.
 * Must be called with shmemipc_lock held. */
static void map_to_arm9(struct shmemipc_block *block)
{
	DEBUG("map_to_arm9(%p): before: %i\n", block, (int)shmemipc_get_block_state());
	switch (shmemipc_get_block_state()) {
		case SHMEMIPC_BLOCK_MAP_BOTH_ARM7:
			if (is_block_arm9(block))
				shmemipc_set_block_state(SHMEMIPC_BLOCK_MAP_NORMAL);
			else if (is_block_arm7(block))
				shmemipc_set_block_state(SHMEMIPC_BLOCK_MAP_REVERSE);
			break;	
		case SHMEMIPC_BLOCK_MAP_NORMAL:
			if (is_block_arm7(block))
				shmemipc_set_block_state(SHMEMIPC_BLOCK_MAP_BOTH_ARM9);
			break;
		case SHMEMIPC_BLOCK_MAP_REVERSE:
			if (is_block_arm9(block))
				shmemipc_set_block_state(SHMEMIPC_BLOCK_MAP_BOTH_ARM9);
			break;
		case SHMEMIPC_BLOCK_MAP_BOTH_ARM9:
			break;	
	}
	DEBUG("map_to_arm9(%p): after: %i\n", block, (int)shmemipc_get_block_state());
}

static inline int is_valid(u8 user)
{
	return (user == SHMEMIPC_USER_SOUND ||
	    user == SHMEMIPC_USER_WIFI ||
	    user == SHMEMIPC_USER_FIRMWARE);
}

int shmemipc_flush(u8 user)
{
	struct shmemipc_block *block = SHMEMIPC_BLOCK_ARM9;

	if (!is_valid(user))
		return -EINVAL;

	DEBUG("shmemipc_flush(): called by user %i\n", (int)user);
	
	down(&flush_mtx); /* released in shmemipc_flush_complete() */
	shmemipc_lock();
	block->user = user;
	map_to_arm7(block);
	DEBUG("shmemipc: requesting flush and waiting for completion\n");
	shmemipc_unlock();

	ipcsync_trigger_remote_interrupt(SHMEMIPC_REQUEST_FLUSH);
	/* This is dodgy... we may get the reply from ARM7 before we sleep. */
	if (wait_for_completion_interruptible_timeout(
	    &shmemipc_flush_completed, SHMEMIPC_FLUSH_TIMEOUT) == 0) {
		printk(KERN_WARNING "shmemipc: flush request timed out\n");
	}
	
	return 0;
}

static void dispatch_interrupt(u8 type, struct shmemipc_block *block)
{
	struct list_head *p;
	struct shmemipc_cb *cb;

	DEBUG("shmemipc: dispatching interrupt type %i for user %i\n", (int)type, (int)block->user);

	list_for_each(p, &cblist) {
		cb = list_entry(p, struct shmemipc_cb, list);
		switch (block->user) {
			case SHMEMIPC_USER_SOUND:
				break;
			case SHMEMIPC_USER_WIFI:
				cb->handler.wifi_callback(type);
				break;
			case SHMEMIPC_USER_FIRMWARE:
				cb->handler.firmware_callback(type);
				break;
		}
	}
}

static void shmemipc_flush_complete(void)
{
	DEBUG("%s called\n", __func__);
	/* The ARM7 is done flushing our block. */
	shmemipc_lock();
	map_to_arm9(SHMEMIPC_BLOCK_ARM9);
	dispatch_interrupt(SHMEMIPC_FLUSH_COMPLETE, SHMEMIPC_BLOCK_ARM9);
	shmemipc_unlock();
	complete(&shmemipc_flush_completed);
	DEBUG("shmemipc: flush completed\n");
	up(&flush_mtx);		/* taken in shmemipc_flush() */
}

static void shmemipc_serve_flush_request(void)
{
	DEBUG("shmemipc: serving flush request for ARM7\n");

	/* The ARM7 wants us to flush its block */
	shmemipc_lock();
	map_to_arm9(SHMEMIPC_BLOCK_ARM7);
	dispatch_interrupt(SHMEMIPC_REQUEST_FLUSH, SHMEMIPC_BLOCK_ARM7);
	map_to_arm7(SHMEMIPC_BLOCK_ARM7);
	shmemipc_unlock();
	ipcsync_trigger_remote_interrupt(SHMEMIPC_FLUSH_COMPLETE);
}

static struct ipcsync_cb request_flush_callback = {
	.type = SHMEMIPC_REQUEST_FLUSH,
	.handler.shmemipc_serve_flush_request = shmemipc_serve_flush_request
};

static struct ipcsync_cb flush_complete_callback = {
	.type = SHMEMIPC_FLUSH_COMPLETE,
	.handler.shmemipc_flush_complete = shmemipc_flush_complete
};

int register_shmemipc_cb(struct shmemipc_cb *cb)
{
	down(&cblist_mtx);
	list_add(&cb->list, &cblist);
	up(&cblist_mtx);
	return 0;
}

int unregister_shmemipc_cb(struct shmemipc_cb *cb)
{
	down(&cblist_mtx);
	list_del(&cb->list);
	up(&cblist_mtx);
	return 0;
}

static int __init shmemipc_init(void)
{
	char *b;

	printk("shmemipc: Driver version %d.%d.%d loaded\n",
	    (SHMEMIPC_VERSION_CODE >> 16) & 0xff,
	    (SHMEMIPC_VERSION_CODE >>  8) & 0xff,
	    (SHMEMIPC_VERSION_CODE      ) & 0xff);
	
	/* Initialize the whole shared memory area to zero. */
	map_to_arm9(SHMEMIPC_BLOCK_0);
	map_to_arm9(SHMEMIPC_BLOCK_1);
	b = (char*)SHMEMIPC_BLOCK_0;
	while (b < (char*)SHMEMIPC_BLOCK_1_END)
		*b++ = '\0';

	map_to_arm7(SHMEMIPC_BLOCK_ARM7);
	map_to_arm9(SHMEMIPC_BLOCK_ARM9);

	register_ipcsync_cb(&request_flush_callback);
	register_ipcsync_cb(&flush_complete_callback);

	return 0;
}

static void __exit shmemipc_exit(void)
{
	unregister_ipcsync_cb(&request_flush_callback);
	unregister_ipcsync_cb(&flush_complete_callback);
}

module_init(shmemipc_init);
module_exit(shmemipc_exit);

MODULE_AUTHOR("Stefan Sperling <stsp@stsp.in-berlin.de>");
MODULE_DESCRIPTION("Nintendo DS inter-processor communication via "
    "shared memory driver");
MODULE_LICENSE("GPL");
