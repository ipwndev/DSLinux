#ifndef __PPC64_MMU_CONTEXT_H
#define __PPC64_MMU_CONTEXT_H

#include <linux/config.h>
#include <linux/kernel.h>	
#include <linux/mm.h>	
#include <asm/mmu.h>	
#include <asm/cputable.h>

/*
 * Copyright (C) 2001 PPC 64 Team, IBM Corp
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

/*
 * Every architecture must define this function. It's the fastest
 * way of searching a 140-bit bitmap where the first 100 bits are
 * unlikely to be set. It's guaranteed that at least one of the 140
 * bits is cleared.
 */
static inline int sched_find_first_bit(unsigned long *b)
{
	if (unlikely(b[0]))
		return __ffs(b[0]);
	if (unlikely(b[1]))
		return __ffs(b[1]) + 64;
	return __ffs(b[2]) + 128;
}

static inline void enter_lazy_tlb(struct mm_struct *mm, struct task_struct *tsk)
{
}

#define NO_CONTEXT	0
#define MAX_CONTEXT	(0x100000-1)

extern int init_new_context(struct task_struct *tsk, struct mm_struct *mm);
extern void destroy_context(struct mm_struct *mm);

extern void switch_stab(struct task_struct *tsk, struct mm_struct *mm);
extern void switch_slb(struct task_struct *tsk, struct mm_struct *mm);

/*
 * switch_mm is the entry point called from the architecture independent
 * code in kernel/sched.c
 */
static inline void switch_mm(struct mm_struct *prev, struct mm_struct *next,
			     struct task_struct *tsk)
{
	if (!cpu_isset(smp_processor_id(), next->cpu_vm_mask))
		cpu_set(smp_processor_id(), next->cpu_vm_mask);

	/* No need to flush userspace segments if the mm doesnt change */
	if (prev == next)
		return;

#ifdef CONFIG_ALTIVEC
	if (cpu_has_feature(CPU_FTR_ALTIVEC))
		asm volatile ("dssall");
#endif /* CONFIG_ALTIVEC */

	if (cpu_has_feature(CPU_FTR_SLB))
		switch_slb(tsk, next);
	else
		switch_stab(tsk, next);
}

#define deactivate_mm(tsk,mm)	do { } while (0)

/*
 * After we have set current->mm to a new value, this activates
 * the context for the new mm so we see the new mappings.
 */
static inline void activate_mm(struct mm_struct *prev, struct mm_struct *next)
{
	unsigned long flags;

	local_irq_save(flags);
	switch_mm(prev, next, current);
	local_irq_restore(flags);
}

#endif /* __PPC64_MMU_CONTEXT_H */
