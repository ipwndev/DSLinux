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

static struct shmemipc_cb *cbhead = NULL;

static DECLARE_MUTEX(cbhead_mtx);
static DECLARE_MUTEX(flush_mtx);

struct shmemipc_cb;

static inline int is_block_arm7(struct shmemipc_block *block)
{
	return block == SHMEMIPC_BLOCK_ARM7;
}

static inline int is_block_arm9(struct shmemipc_block *block)
{
	return block == SHMEMIPC_BLOCK_ARM9;
}

/* Map the specified block to the ARM7, if it isn't mapped to it already. */
void map_to_arm7(struct shmemipc_block *block)
{
	//printk("map_to_arm7(%p): before: %i\n", block, (int)shmemipc_get_block_state());
	shmemipc_lock();
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
	shmemipc_unlock();
	//printk("map_to_arm7(%p): after: %i\n", block, (int)shmemipc_get_block_state());
}

/* Map the specified block to the ARM9, if it isn't mapped to it already. */
void map_to_arm9(struct shmemipc_block *block)
{
	//printk("map_to_arm9(%p): before: %i\n", block, (int)shmemipc_get_block_state());
	shmemipc_lock();
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
	shmemipc_unlock();
	//printk("map_to_arm9(%p): after: %i\n", block, (int)shmemipc_get_block_state());
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

	//printk("shmemipc_flush(): called by user %i\n", (int)user);
	
	down(&flush_mtx); /* released in shmemipc_flush_complete() */
	shmemipc_lock();
	block->user = user;
	map_to_arm7(block);
	shmemipc_unlock();
	ipcsync_trigger_remote_interrupt(SHMEMIPC_REQUEST_FLUSH);
	
	return 0;
}

static void dispatch_interrupt(u8 type)
{
	struct shmemipc_block* block = NULL;
	struct shmemipc_cb *cb;

	block = SHMEMIPC_BLOCK;

	//printk("shmemipc: dispatching interrupt type %i for user %i\n", (int)type, (int)block->user);

	for (cb = cbhead; cb; cb = cb->next) {
		switch (block->user) {
			case SHMEMIPC_USER_SOUND:
				break;
			case SHMEMIPC_USER_WIFI:
				break;
			case SHMEMIPC_USER_FIRMWARE:
				cb->handler.firmware_callback(type);
				break;
		}
	}
}

static void shmemipc_flush_complete(void)
{
	/* The ARM7 is done flushing our block. */
	shmemipc_lock();
	map_to_arm9(SHMEMIPC_BLOCK_ARM9);
	dispatch_interrupt(SHMEMIPC_FLUSH_COMPLETE);
	shmemipc_unlock();
	up(&flush_mtx);
}

static void shmemipc_serve_flush_request(void)
{
	/* The ARM7 wants us to flush its block */
	shmemipc_lock();
	map_to_arm9(SHMEMIPC_BLOCK_ARM7);
	dispatch_interrupt(SHMEMIPC_REQUEST_FLUSH);
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
	down(&cbhead_mtx);
	cb->next = cbhead;
	cbhead = cb;
	up(&cbhead_mtx);
	return 0;
}

int unregister_shmemipc_cb(struct shmemipc_cb *shmemipc_cb)
{
	struct shmemipc_cb *cb;

	down(&cbhead_mtx);
	for (cb = cbhead; cb; cb = cb->next) {
		if (cb->next == NULL) { /* shmemipc_cb not found in list */
			up(&cbhead_mtx);
			return -EINVAL;
		} else if (cb->next == shmemipc_cb) {
			cb->next = cb->next->next;
			break;
		}
	}
	up(&cbhead_mtx);
	return 0;
}

#define SHMEMIPC_VERSION_CODE KERNEL_VERSION(0, 1, 0)
static int __init shmemipc_init(void)
{
	char *b;

	printk("shmemipc: Driver version %d.%d.%d loaded\n",
	    (SHMEMIPC_VERSION_CODE >> 16) & 0xff,
	    (SHMEMIPC_VERSION_CODE >>  8) & 0xff,
	    (SHMEMIPC_VERSION_CODE      ) & 0xff);
	
	register_ipcsync_cb(&request_flush_callback);
	register_ipcsync_cb(&flush_complete_callback);

	/* Initialize the whole shared memory area to zero. */
	map_to_arm9(SHMEMIPC_BLOCK_0);
	map_to_arm9(SHMEMIPC_BLOCK_1);
	b = (char*)SHMEMIPC_BLOCK_0;
	while (b < (char*)SHMEMIPC_BLOCK_1_END)
		*b++ = '\0';

	map_to_arm7(SHMEMIPC_BLOCK_ARM7);
	map_to_arm9(SHMEMIPC_BLOCK_ARM9);

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
