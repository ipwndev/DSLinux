/*
 * include/asm-arm/arch-nds/spinlock_nds.h
 *
 * Copyright 2006 Stefan Sperling <stsp@stsp.in-berlin.de>
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
 * This file implements low-level spinlock routines for dslinux.
 * 
 * Since the DS has an ARMv5 processor, the standard asm-arm/spinlock.h
 * will not help us because ARM cores < v6 are not SMP-capable.
 *
 * However, we have an SMP-like situation on the DS, because of
 * interrupt-based communication to the ARM7 core. We need spinlocks
 * for safe locking in interrupt-context on the kernel (ARM9) side.
 *
 * We cannot use the "real" Linux spinlock API, as it is all no-ops
 * on UP, but we try to emulate it without overriding definitions
 * from the standard API.
 * 
 * Note that only a subset of the Linux spinlock API is emulated,
 * namely:
 *
 *	SPIN_LOCK_UNLOCKED	-->	NDS_SPIN_LOCK_UNLOCKED
 * 	spin_lock_init()	-->	nds_spin_lock_init()
 * 	spin_lock()		-->	nds_spin_lock()
 * 	spin_unlock()		-->	nds_spin_unlock()
 *
 * That's all. We don't really need anything else for dslinux.
 */

#ifndef __ASM_ARM_ARCH_SPINLOCK_NDS_H
#define __ASM_ARM_ARCH_SPINLOCK_NDS_H

typedef struct {
	volatile unsigned int lock;
} nds_spinlock_t;

/* do not change these values! the asm code below relies on them! */
#define __NDS_SPINLOCK_STATE_UNLOCKED	0
#define __NDS_SPINLOCK_STATE_LOCKED	1

/* Initialize a spin lock dynamically */
static inline void nds_spin_lock_init(nds_spinlock_t *lock)
{
	lock->lock = __NDS_SPINLOCK_STATE_UNLOCKED;
}

/*
 * Acquire a spin lock.
 *
 * We try exclusively storing it by atomically swapping the lock with
 * the constant __NDS_SPINLOCK_STATE_LOCKED.
 * If we get back __NDS_SPINLOCK_STATE_UNLOCKED from memory during the
 * swap operation, we got the lock.
 *
 * This routine returns when the caller has got the lock.
 */
static inline void nds_spin_lock(nds_spinlock_t *lock)
{
	/* we assign this here to shut the compiler up about
	 * tmp not being initialised. */
	unsigned long tmp = __NDS_SPINLOCK_STATE_LOCKED;

	printk("locking...");

	/* we assign this here to shut the compiler up about
	 * tmp not being initialised. */
	__asm__ __volatile__(
"1:	ldr	%[tmp], =1\n"
"	swp	%[tmp], %[tmp], [%[lock]]\n"	/* atomic */
"	cmp	%[tmp], #0\n"
"	bne	1b"
	:
	: [lock] "r" (&lock->lock), [tmp] "r" (tmp), "r" (1)
	: "cc" );

	printk(" done\n");
}

/* We hold the lock exclusively, so just set it to
 * __NDS_SPINLOCK_STATE_UNLOCKED */
static inline void nds_spin_unlock(nds_spinlock_t *lock)
{
	/* we assign this here to shut the compiler up about
	 * tmp not being initialised. */
	unsigned long tmp = __NDS_SPINLOCK_STATE_UNLOCKED;

	printk("unlocking...");

	__asm__ __volatile__(
"	ldr	%[tmp], =0\n"
"	swp	%[tmp], %[tmp], [%[lock]]\n"	/* atomic */
	:
	: [lock] "r" (&lock->lock), [tmp] "r" (tmp)
	: "cc");

	printk(" done\n");
}

/* Linux spinlock API overrides */
#define NDS_SPIN_LOCK_UNLOCKED \
	(nds_spinlock_t) { \
		.lock = __NDS_SPINLOCK_STATE_UNLOCKED, \
	}

#endif /* __ASM_ARM_ARCH_SPINLOCK_NDS_H */
