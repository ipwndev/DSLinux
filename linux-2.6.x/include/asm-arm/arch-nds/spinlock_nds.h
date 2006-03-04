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
 * on UP, but we try to emulate it via crude macro overrides.
 * Needless to say, including <linux/spinlock.h> and this
 * file at the same time does not make sense at all.
 */

#ifndef __ASM_ARM_ARCH_SPINLOCK_NDS_H
#define __ASM_ARM_ARCH_SPINLOCK_NDS_H

typedef struct {
	volatile unsigned int lock;
} __nds_spinlock_t;

#define NDS_SPINLOCK_UNLOCKED	0
#define NDS_SPINLOCK_LOCKED	1

/*
 * Acquire a spin lock.
 *
 * We read the old value.  If it is zero, we may have
 * won the lock, so we try exclusively storing it by atomically
 * swapping the lock with the constant NDS_SPINLOCK_LOCKED.
 * If we get back NDS_SPINLOCK_UNLOCKED from memory during the
 * swap operation, we got the lock.
 *
 * This routine returns when the caller has got the lock.
 */
static inline void __spin_lock_nds(nds_spinlock_t *lock)
{
	unsigned long tmp;

	__asm__ __volatile__(
"spin:	ldr	%[tmp], [%[lock]]\n"
"	cmp	%[tmp], #NDS_SPINLOCK_UNLOCKED\n"
"	bne	spin\n"
"	ldr	%[tmp], #NDS_SPINLOCK_LOCKED\n"
"	swp	%[tmp], %[tmp], [%[lock]]\n"	/* atomic */
"	cmp	%[tmp], #NDS_SPINLOCK_UNLOCKED\n"
"	bne	spin\n"
	:
	: [lock] "r" (&lock->lock), [tmp] "r" (tmp));
}

/* We hold the lock exclusively, so just set it to NDS_SPINLOCK_UNLOCKED */
static inline void __spin_unlock_nds(nds_spinlock_t *lock)
{
	lock->lock = NDS_SPINLOCK_UNLOCKED;
}

/* Override Linux spinlock API */

#ifdef spin_lock
#undef spin_lock
#endif
#define spin_lock(x)	__spin_lock_nds(x)

#ifdef spin_unlock
#undef spin_unlock
#endif
#define spin_unlock(x)	__spin_unlock_nds(x)

#ifdef spinlock_t
#undef spinlock_t
#endif
#define spinlock_t	__nds_spinlock_t

#endif /* __ASM_ARM_ARCH_SPINLOCK_NDS_H */
