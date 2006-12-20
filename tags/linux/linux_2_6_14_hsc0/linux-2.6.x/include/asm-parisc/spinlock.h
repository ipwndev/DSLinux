#ifndef __ASM_SPINLOCK_H
#define __ASM_SPINLOCK_H

#include <asm/system.h>
#include <asm/processor.h>
#include <asm/spinlock_types.h>

/* Note that PA-RISC has to use `1' to mean unlocked and `0' to mean locked
 * since it only has load-and-zero. Moreover, at least on some PA processors,
 * the semaphore address has to be 16-byte aligned.
 */

static inline int __raw_spin_is_locked(raw_spinlock_t *x)
{
	volatile unsigned int *a = __ldcw_align(x);
	return *a == 0;
}

#define __raw_spin_lock_flags(lock, flags) __raw_spin_lock(lock)
#define __raw_spin_unlock_wait(x) \
		do { cpu_relax(); } while (__raw_spin_is_locked(x))

static inline void __raw_spin_lock(raw_spinlock_t *x)
{
	volatile unsigned int *a;

	mb();
	a = __ldcw_align(x);
	while (__ldcw(a) == 0)
		while (*a == 0);
	mb();
}

static inline void __raw_spin_unlock(raw_spinlock_t *x)
{
	volatile unsigned int *a;
	mb();
	a = __ldcw_align(x);
	*a = 1;
	mb();
}

static inline int __raw_spin_trylock(raw_spinlock_t *x)
{
	volatile unsigned int *a;
	int ret;

	mb();
	a = __ldcw_align(x);
        ret = __ldcw(a) != 0;
	mb();

	return ret;
}

/*
 * Read-write spinlocks, allowing multiple readers
 * but only one writer.
 */

#define __raw_read_trylock(lock) generic__raw_read_trylock(lock)

/* read_lock, read_unlock are pretty straightforward.  Of course it somehow
 * sucks we end up saving/restoring flags twice for read_lock_irqsave aso. */

static  __inline__ void __raw_read_lock(raw_rwlock_t *rw)
{
	unsigned long flags;
	local_irq_save(flags);
	__raw_spin_lock(&rw->lock);

	rw->counter++;

	__raw_spin_unlock(&rw->lock);
	local_irq_restore(flags);
}

static  __inline__ void __raw_read_unlock(raw_rwlock_t *rw)
{
	unsigned long flags;
	local_irq_save(flags);
	__raw_spin_lock(&rw->lock);

	rw->counter--;

	__raw_spin_unlock(&rw->lock);
	local_irq_restore(flags);
}

/* write_lock is less trivial.  We optimistically grab the lock and check
 * if we surprised any readers.  If so we release the lock and wait till
 * they're all gone before trying again
 *
 * Also note that we don't use the _irqsave / _irqrestore suffixes here.
 * If we're called with interrupts enabled and we've got readers (or other
 * writers) in interrupt handlers someone fucked up and we'd dead-lock
 * sooner or later anyway.   prumpf */

static  __inline__ void __raw_write_lock(raw_rwlock_t *rw)
{
retry:
	__raw_spin_lock(&rw->lock);

	if(rw->counter != 0) {
		/* this basically never happens */
		__raw_spin_unlock(&rw->lock);

		while (rw->counter != 0)
			cpu_relax();

		goto retry;
	}

	/* got it.  now leave without unlocking */
	rw->counter = -1; /* remember we are locked */
}

/* write_unlock is absolutely trivial - we don't have to wait for anything */

static  __inline__ void __raw_write_unlock(raw_rwlock_t *rw)
{
	rw->counter = 0;
	__raw_spin_unlock(&rw->lock);
}

static  __inline__ int __raw_write_trylock(raw_rwlock_t *rw)
{
	__raw_spin_lock(&rw->lock);
	if (rw->counter != 0) {
		/* this basically never happens */
		__raw_spin_unlock(&rw->lock);

		return 0;
	}

	/* got it.  now leave without unlocking */
	rw->counter = -1; /* remember we are locked */
	return 1;
}

static __inline__ int __raw_is_read_locked(raw_rwlock_t *rw)
{
	return rw->counter > 0;
}

static __inline__ int __raw_is_write_locked(raw_rwlock_t *rw)
{
	return rw->counter < 0;
}

#endif /* __ASM_SPINLOCK_H */
