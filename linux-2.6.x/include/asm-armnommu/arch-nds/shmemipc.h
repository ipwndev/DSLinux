/*
 * include/asm-armnommu/arch-nds/shmemipc.h
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
 *
 *
 * This file belongs to the framework for inter-processor-communication
 * used in dslinux. The DS has two processors, an arm7 and an arm9.
 * Each has dedicated tasks. The arm7 controls certain hardware devices
 * like the touchscreen, wifi and sound chips. The Linux kernel itself
 * runs only on the arm9, so this is not SMP!
 *
 * Think of the arm7 as a co-processor for certain tasks. Some drivers use
 * shared memory to pass data between the CPUs.
 * 
 * There are 3 ways for the 2 CPUs to communicate:
 *
 *  1. A hardware FIFO. See fifo.c for details.
 *     This is used by the touchscreen driver, for example.
 *  
 *  2. A shared memory region.
 *     Quoting http://neimod.com/dstek/dstek2.xml#Memory%20Map:
 *     
 *     Shared IWRAM. The name shared IWRAM (consists of 2 16kb blocks)
 *     can be misleading, as only one CPU has access to it.
 *     However, using WRAMCNT each 16k block can be assigned to a
 *     specific CPU. When processing is done for example, the block can
 *     be assigned to the other CPU quickly for processing, without copying
 *     it via main memory or the IPC fifo.
 *  
 *  3. A synchronization register.
 *     From http://neimod.com/dstek/dstek2.xml#Interprocessor%20Communication:
 *
 *     The ARM7 can trigger an interrupt on the ARM9 (and vice versa) by
 *     writing a 1 to bit 13 of the ARM7's REG_IPCSYNC. But, the ARM9 must
 *     allow this to happen by enabling bit 14 of ARM9's REG_IPCSYNC,
 *     and enable bit 16 of REG_IE.
 *
 * 
 * The shared memory framework uses the shared memory region for data to
 * be transferred, and the ipcsync for communication between the processors.
 * It is logically stacked between ipcsync and device drivers.
 * ipsync is below it and device drivers using shared memory to communicate
 * with devices behind the ARM7 are on top of it.
 *
 * The idea is as follows:
 *
 * The ARM7 writes to block 0 and reads from it when requested by the ARM9.
 * The ARM9 writes to block 1 and reads from it when requested by the ARM7.
 * 
 * Details:
 *
 * A driver that wants to communicate to the ARM7 via shared memory locks the
 * block (SHMEMIPC_BLOCK) and writes data to it.
 * Once the driver wants to send the data over to the ARM7, it calls
 * shmemipc_flush(). This will lock the block and send a SHMEMIPC_REQUEST_FLUSH
 * interrupt to the ARM7. The interrupt has a type specific to the driver, so
 * the ARM7 knows what area of the memory block it should read from, and which
 * device is the destination. Having read the data from the buffer the ARM7
 * will interrupt the kernel with SHMEMIPC_FLUSH_COMPLETE. The interrupt
 * handler will release the lock on the block. The transfer is complete.
 *
 * Communication from the ARM7 to the ARM9 works the same way. But because we
 * can only re-map blocks when running on the ARM9 and the ARM7 does not run
 * a multi-threaded kernel but a simple endless-loop program, the implementation
 * details vary between the two different CPUs.
 *
 * We might be sharing the ARM7 interrupt with other subsystems of the kernel.
 * The dispatcher in ipcsync.c takes care of this though.
 * We only get interrupts destined for us.
 *
 */

#ifndef __ASM_ARM_ARCH_SHMEMIPC_H
#define __ASM_ARM_ARCH_SHMEMIPC_H

#include <asm/arch/ipcsync.h>

struct shmemipc_block;

/* These are the two 16k shared memory blocks. */
#define SHMEMIPC_BLOCK_0	((struct shmemipc_block*) 0x037F8000)
#define SHMEMIPC_BLOCK_0_END	((void*) 0x037FBFFF)
#define SHMEMIPC_BLOCK_1	((struct shmemipc_block*) 0x037FC000)
#define SHMEMIPC_BLOCK_1_END	((void*) 0x037FFFFF)
 
/* The ARM7 writes to BLOCK_ARM7 and reads from it when requested by the ARM9.
 * The ARM9 writes to BLOCK_ARM9 and reads from it when requested by the ARM7.
 */
#define SHMEMIPC_BLOCK_ARM7 SHMEMIPC_BLOCK_0
#define SHMEMIPC_BLOCK_ARM9 SHMEMIPC_BLOCK_1

/* A shortcut for use in device drivers that don't want to clutter
 * their code with the fact that they run on ARM9.
 */
#define SHMEMIPC_BLOCK SHMEMIPC_BLOCK_ARM9

/* WRAM Control Register. This controls the mapping of blocks to CPUs.
 * Only bits 0 and 1 are interesting. We can only access this register from
 * the ARM9.  The intial value for this register is BOTH_BLOCKS_ARM7.
 */
#define REG_WRAMCNT			(*(u8*)0x04000247)

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

#if 0
/* Data structure for sound data user. */
struct shmemipc_sound_block {
};
#endif

/* Data structure for firmware dumper */
#define SHMEMIPC_FIRMWARE_DATA_SIZE	512
struct shmemipc_firmware_block {
	loff_t from;
	size_t len;
	u_char *destination;
	u_char data[SHMEMIPC_FIRMWARE_DATA_SIZE];
};


struct shmemipc_wifi_block {
	u16 type;
	u16 length;
	u_char data[1600];
};

/* Drivers _must_ use these macros to lock the memory block for access!
 * Unfortunately, we cannot use spinlocks because spinlock.h will bail
 * when we include it ("SMP not supported").  */
#define shmemipc_lock()		local_irq_disable()
#define shmemipc_unlock()	local_irq_enable()

/* The user ID is used to tell the other side who is responsible
 * for flushing data. Every driver using the shared memory framework
 * needs an ID here. */
#define SHMEMIPC_USER_SOUND	1
#define SHMEMIPC_USER_WIFI	2
#define SHMEMIPC_USER_FIRMWARE	3

struct shmemipc_block {
	u8 user;
	struct shmemipc_firmware_block firmware;
	struct shmemipc_wifi_block wifi;
};

/* Callbacks will be called on ARM7 interrupts if their user
 * matches the one specified by the ARM7 in REG_IPCSYNC. They
 * get one argument which identifies the type of interrupt that
 * occurred (SHEMEMIPC_FLUSHREQUEST or SHMEMIPC_FLUSH_COMPLETE,
 * defined in ipcsync.h) */
struct shmemipc_cb {
	u8 user; /* One of the users defined above. */
	union
	{
		/* ... */
		void (*firmware_callback)(u8 type);
		void (*wifi_callback)(u8 type);
	} handler;
	struct shmemipc_cb *next;
};


int register_shmemipc_cb(struct shmemipc_cb *callback);

/* This function may sleep. Don't call in interrupt! 
 * Don't call with interrupts disabled! */
int shmemipc_flush(u8 user);

void map_to_arm7(struct shmemipc_block *block);
void map_to_arm9(struct shmemipc_block *block);

#endif /* __ASM_ARM_ARCH_SHMEMIPC_H */
