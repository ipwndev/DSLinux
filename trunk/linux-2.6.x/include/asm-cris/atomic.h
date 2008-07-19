/* $Id$ */

#ifndef __ASM_CRIS_ATOMIC__
#define __ASM_CRIS_ATOMIC__

#include <asm/system.h>
#include <asm/arch/atomic.h>

/*
 * Atomic operations that C can't guarantee us.  Useful for
 * resource counting etc..
 */

typedef struct { volatile int counter; } atomic_t;

#define ATOMIC_INIT(i)  { (i) }

#define atomic_read(v) ((v)->counter)
#define atomic_set(v,i) (((v)->counter) = (i))

/* These should be written in asm but we do it in C for now. */

extern __inline__ void atomic_add(int i, volatile atomic_t *v)
{
	unsigned long flags;
	cris_atomic_save(v, flags);
	v->counter += i;
	cris_atomic_restore(v, flags);
}

extern __inline__ void atomic_sub(int i, volatile atomic_t *v)
{
	unsigned long flags;
	cris_atomic_save(v, flags);
	v->counter -= i;
	cris_atomic_restore(v, flags);
}

extern __inline__ int atomic_add_return(int i, volatile atomic_t *v)
{
	unsigned long flags;
	int retval;
	cris_atomic_save(v, flags);
	retval = (v->counter += i);
	cris_atomic_restore(v, flags);
	return retval;
}

#define atomic_add_negative(a, v)	(atomic_add_return((a), (v)) < 0)

extern __inline__ int atomic_sub_return(int i, volatile atomic_t *v)
{
	unsigned long flags;
	int retval;
	cris_atomic_save(v, flags);
	retval = (v->counter -= i);
	cris_atomic_restore(v, flags);
	return retval;
}

extern __inline__ int atomic_sub_and_test(int i, volatile atomic_t *v)
{
	int retval;
	unsigned long flags;
	cris_atomic_save(v, flags);
	retval = (v->counter -= i) == 0;
	cris_atomic_restore(v, flags);
	return retval;
}

extern __inline__ void atomic_inc(volatile atomic_t *v)
{
	unsigned long flags;
	cris_atomic_save(v, flags);
	(v->counter)++;
	cris_atomic_restore(v, flags);
}

extern __inline__ void atomic_dec(volatile atomic_t *v)
{
	unsigned long flags;
	cris_atomic_save(v, flags);
	(v->counter)--;
	cris_atomic_restore(v, flags);
}

extern __inline__ int atomic_inc_return(volatile atomic_t *v)
{
	unsigned long flags;
	int retval;
	cris_atomic_save(v, flags);
	retval = (v->counter)++;
	cris_atomic_restore(v, flags);
	return retval;
}

extern __inline__ int atomic_dec_return(volatile atomic_t *v)
{
	unsigned long flags;
	int retval;
	cris_atomic_save(v, flags);
	retval = (v->counter)--;
	cris_atomic_restore(v, flags);
	return retval;
}
extern __inline__ int atomic_dec_and_test(volatile atomic_t *v)
{
	int retval;
	unsigned long flags;
	cris_atomic_save(v, flags);
	retval = --(v->counter) == 0;
	cris_atomic_restore(v, flags);
	return retval;
}

extern __inline__ int atomic_inc_and_test(volatile atomic_t *v)
{
	int retval;
	unsigned long flags;
	cris_atomic_save(v, flags);
	retval = ++(v->counter) == 0;
	cris_atomic_restore(v, flags);
	return retval;
}

/* Atomic operations are already serializing */
#define smp_mb__before_atomic_dec()    barrier()
#define smp_mb__after_atomic_dec()     barrier()
#define smp_mb__before_atomic_inc()    barrier()
#define smp_mb__after_atomic_inc()     barrier()

#endif
