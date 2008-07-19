/*
 *  Kernel Probes (KProbes)
 *  kernel/kprobes.c
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) IBM Corporation, 2002, 2004
 *
 * 2002-Oct	Created by Vamsi Krishna S <vamsi_krishna@in.ibm.com> Kernel
 *		Probes initial implementation (includes suggestions from
 *		Rusty Russell).
 * 2004-Aug	Updated by Prasanna S Panchamukhi <prasanna@in.ibm.com> with
 *		hlists and exceptions notifier as suggested by Andi Kleen.
 * 2004-July	Suparna Bhattacharya <suparna@in.ibm.com> added jumper probes
 *		interface to access function arguments.
 * 2004-Sep	Prasanna S Panchamukhi <prasanna@in.ibm.com> Changed Kprobes
 *		exceptions notifier to be first on the priority list.
 * 2005-May	Hien Nguyen <hien@us.ibm.com>, Jim Keniston
 *		<jkenisto@us.ibm.com> and Prasanna S Panchamukhi
 *		<prasanna@in.ibm.com> added function-return probes.
 */
#include <linux/kprobes.h>
#include <linux/spinlock.h>
#include <linux/hash.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleloader.h>
#include <asm-generic/sections.h>
#include <asm/cacheflush.h>
#include <asm/errno.h>
#include <asm/kdebug.h>

#define KPROBE_HASH_BITS 6
#define KPROBE_TABLE_SIZE (1 << KPROBE_HASH_BITS)

static struct hlist_head kprobe_table[KPROBE_TABLE_SIZE];
static struct hlist_head kretprobe_inst_table[KPROBE_TABLE_SIZE];

unsigned int kprobe_cpu = NR_CPUS;
static DEFINE_SPINLOCK(kprobe_lock);
static struct kprobe *curr_kprobe;

/*
 * kprobe->ainsn.insn points to the copy of the instruction to be
 * single-stepped. x86_64, POWER4 and above have no-exec support and
 * stepping on the instruction on a vmalloced/kmalloced/data page
 * is a recipe for disaster
 */
#define INSNS_PER_PAGE	(PAGE_SIZE/(MAX_INSN_SIZE * sizeof(kprobe_opcode_t)))

struct kprobe_insn_page {
	struct hlist_node hlist;
	kprobe_opcode_t *insns;		/* Page of instruction slots */
	char slot_used[INSNS_PER_PAGE];
	int nused;
};

static struct hlist_head kprobe_insn_pages;

/**
 * get_insn_slot() - Find a slot on an executable page for an instruction.
 * We allocate an executable page if there's no room on existing ones.
 */
kprobe_opcode_t __kprobes *get_insn_slot(void)
{
	struct kprobe_insn_page *kip;
	struct hlist_node *pos;

	hlist_for_each(pos, &kprobe_insn_pages) {
		kip = hlist_entry(pos, struct kprobe_insn_page, hlist);
		if (kip->nused < INSNS_PER_PAGE) {
			int i;
			for (i = 0; i < INSNS_PER_PAGE; i++) {
				if (!kip->slot_used[i]) {
					kip->slot_used[i] = 1;
					kip->nused++;
					return kip->insns + (i * MAX_INSN_SIZE);
				}
			}
			/* Surprise!  No unused slots.  Fix kip->nused. */
			kip->nused = INSNS_PER_PAGE;
		}
	}

	/* All out of space.  Need to allocate a new page. Use slot 0.*/
	kip = kmalloc(sizeof(struct kprobe_insn_page), GFP_KERNEL);
	if (!kip) {
		return NULL;
	}

	/*
	 * Use module_alloc so this page is within +/- 2GB of where the
	 * kernel image and loaded module images reside. This is required
	 * so x86_64 can correctly handle the %rip-relative fixups.
	 */
	kip->insns = module_alloc(PAGE_SIZE);
	if (!kip->insns) {
		kfree(kip);
		return NULL;
	}
	INIT_HLIST_NODE(&kip->hlist);
	hlist_add_head(&kip->hlist, &kprobe_insn_pages);
	memset(kip->slot_used, 0, INSNS_PER_PAGE);
	kip->slot_used[0] = 1;
	kip->nused = 1;
	return kip->insns;
}

void __kprobes free_insn_slot(kprobe_opcode_t *slot)
{
	struct kprobe_insn_page *kip;
	struct hlist_node *pos;

	hlist_for_each(pos, &kprobe_insn_pages) {
		kip = hlist_entry(pos, struct kprobe_insn_page, hlist);
		if (kip->insns <= slot &&
		    slot < kip->insns + (INSNS_PER_PAGE * MAX_INSN_SIZE)) {
			int i = (slot - kip->insns) / MAX_INSN_SIZE;
			kip->slot_used[i] = 0;
			kip->nused--;
			if (kip->nused == 0) {
				/*
				 * Page is no longer in use.  Free it unless
				 * it's the last one.  We keep the last one
				 * so as not to have to set it up again the
				 * next time somebody inserts a probe.
				 */
				hlist_del(&kip->hlist);
				if (hlist_empty(&kprobe_insn_pages)) {
					INIT_HLIST_NODE(&kip->hlist);
					hlist_add_head(&kip->hlist,
						&kprobe_insn_pages);
				} else {
					module_free(NULL, kip->insns);
					kfree(kip);
				}
			}
			return;
		}
	}
}

/* Locks kprobe: irqs must be disabled */
void __kprobes lock_kprobes(void)
{
	unsigned long flags = 0;

	/* Avoiding local interrupts to happen right after we take the kprobe_lock
	 * and before we get a chance to update kprobe_cpu, this to prevent
	 * deadlock when we have a kprobe on ISR routine and a kprobe on task
	 * routine
	 */
	local_irq_save(flags);

	spin_lock(&kprobe_lock);
	kprobe_cpu = smp_processor_id();

 	local_irq_restore(flags);
}

void __kprobes unlock_kprobes(void)
{
	unsigned long flags = 0;

	/* Avoiding local interrupts to happen right after we update
	 * kprobe_cpu and before we get a a chance to release kprobe_lock,
	 * this to prevent deadlock when we have a kprobe on ISR routine and
	 * a kprobe on task routine
	 */
	local_irq_save(flags);

	kprobe_cpu = NR_CPUS;
	spin_unlock(&kprobe_lock);

 	local_irq_restore(flags);
}

/* You have to be holding the kprobe_lock */
struct kprobe __kprobes *get_kprobe(void *addr)
{
	struct hlist_head *head;
	struct hlist_node *node;

	head = &kprobe_table[hash_ptr(addr, KPROBE_HASH_BITS)];
	hlist_for_each(node, head) {
		struct kprobe *p = hlist_entry(node, struct kprobe, hlist);
		if (p->addr == addr)
			return p;
	}
	return NULL;
}

/*
 * Aggregate handlers for multiple kprobes support - these handlers
 * take care of invoking the individual kprobe handlers on p->list
 */
static int __kprobes aggr_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	struct kprobe *kp;

	list_for_each_entry(kp, &p->list, list) {
		if (kp->pre_handler) {
			curr_kprobe = kp;
			if (kp->pre_handler(kp, regs))
				return 1;
		}
		curr_kprobe = NULL;
	}
	return 0;
}

static void __kprobes aggr_post_handler(struct kprobe *p, struct pt_regs *regs,
					unsigned long flags)
{
	struct kprobe *kp;

	list_for_each_entry(kp, &p->list, list) {
		if (kp->post_handler) {
			curr_kprobe = kp;
			kp->post_handler(kp, regs, flags);
			curr_kprobe = NULL;
		}
	}
	return;
}

static int __kprobes aggr_fault_handler(struct kprobe *p, struct pt_regs *regs,
					int trapnr)
{
	/*
	 * if we faulted "during" the execution of a user specified
	 * probe handler, invoke just that probe's fault handler
	 */
	if (curr_kprobe && curr_kprobe->fault_handler) {
		if (curr_kprobe->fault_handler(curr_kprobe, regs, trapnr))
			return 1;
	}
	return 0;
}

static int __kprobes aggr_break_handler(struct kprobe *p, struct pt_regs *regs)
{
	struct kprobe *kp = curr_kprobe;
	if (curr_kprobe && kp->break_handler) {
		if (kp->break_handler(kp, regs)) {
			curr_kprobe = NULL;
			return 1;
		}
	}
	curr_kprobe = NULL;
	return 0;
}

struct kretprobe_instance __kprobes *get_free_rp_inst(struct kretprobe *rp)
{
	struct hlist_node *node;
	struct kretprobe_instance *ri;
	hlist_for_each_entry(ri, node, &rp->free_instances, uflist)
		return ri;
	return NULL;
}

static struct kretprobe_instance __kprobes *get_used_rp_inst(struct kretprobe
							      *rp)
{
	struct hlist_node *node;
	struct kretprobe_instance *ri;
	hlist_for_each_entry(ri, node, &rp->used_instances, uflist)
		return ri;
	return NULL;
}

void __kprobes add_rp_inst(struct kretprobe_instance *ri)
{
	/*
	 * Remove rp inst off the free list -
	 * Add it back when probed function returns
	 */
	hlist_del(&ri->uflist);

	/* Add rp inst onto table */
	INIT_HLIST_NODE(&ri->hlist);
	hlist_add_head(&ri->hlist,
			&kretprobe_inst_table[hash_ptr(ri->task, KPROBE_HASH_BITS)]);

	/* Also add this rp inst to the used list. */
	INIT_HLIST_NODE(&ri->uflist);
	hlist_add_head(&ri->uflist, &ri->rp->used_instances);
}

void __kprobes recycle_rp_inst(struct kretprobe_instance *ri)
{
	/* remove rp inst off the rprobe_inst_table */
	hlist_del(&ri->hlist);
	if (ri->rp) {
		/* remove rp inst off the used list */
		hlist_del(&ri->uflist);
		/* put rp inst back onto the free list */
		INIT_HLIST_NODE(&ri->uflist);
		hlist_add_head(&ri->uflist, &ri->rp->free_instances);
	} else
		/* Unregistering */
		kfree(ri);
}

struct hlist_head __kprobes *kretprobe_inst_table_head(struct task_struct *tsk)
{
	return &kretprobe_inst_table[hash_ptr(tsk, KPROBE_HASH_BITS)];
}

/*
 * This function is called from exit_thread or flush_thread when task tk's
 * stack is being recycled so that we can recycle any function-return probe
 * instances associated with this task. These left over instances represent
 * probed functions that have been called but will never return.
 */
void __kprobes kprobe_flush_task(struct task_struct *tk)
{
        struct kretprobe_instance *ri;
        struct hlist_head *head;
	struct hlist_node *node, *tmp;
	unsigned long flags = 0;

	spin_lock_irqsave(&kprobe_lock, flags);
        head = kretprobe_inst_table_head(current);
        hlist_for_each_entry_safe(ri, node, tmp, head, hlist) {
                if (ri->task == tk)
                        recycle_rp_inst(ri);
        }
	spin_unlock_irqrestore(&kprobe_lock, flags);
}

/*
 * This kprobe pre_handler is registered with every kretprobe. When probe
 * hits it will set up the return probe.
 */
static int __kprobes pre_handler_kretprobe(struct kprobe *p,
					   struct pt_regs *regs)
{
	struct kretprobe *rp = container_of(p, struct kretprobe, kp);

	/*TODO: consider to only swap the RA after the last pre_handler fired */
	arch_prepare_kretprobe(rp, regs);
	return 0;
}

static inline void free_rp_inst(struct kretprobe *rp)
{
	struct kretprobe_instance *ri;
	while ((ri = get_free_rp_inst(rp)) != NULL) {
		hlist_del(&ri->uflist);
		kfree(ri);
	}
}

/*
 * Keep all fields in the kprobe consistent
 */
static inline void copy_kprobe(struct kprobe *old_p, struct kprobe *p)
{
	memcpy(&p->opcode, &old_p->opcode, sizeof(kprobe_opcode_t));
	memcpy(&p->ainsn, &old_p->ainsn, sizeof(struct arch_specific_insn));
}

/*
* Add the new probe to old_p->list. Fail if this is the
* second jprobe at the address - two jprobes can't coexist
*/
static int __kprobes add_new_kprobe(struct kprobe *old_p, struct kprobe *p)
{
        struct kprobe *kp;

	if (p->break_handler) {
		list_for_each_entry(kp, &old_p->list, list) {
			if (kp->break_handler)
				return -EEXIST;
		}
		list_add_tail(&p->list, &old_p->list);
	} else
		list_add(&p->list, &old_p->list);
	return 0;
}

/*
 * Fill in the required fields of the "manager kprobe". Replace the
 * earlier kprobe in the hlist with the manager kprobe
 */
static inline void add_aggr_kprobe(struct kprobe *ap, struct kprobe *p)
{
	copy_kprobe(p, ap);
	ap->addr = p->addr;
	ap->pre_handler = aggr_pre_handler;
	ap->post_handler = aggr_post_handler;
	ap->fault_handler = aggr_fault_handler;
	ap->break_handler = aggr_break_handler;

	INIT_LIST_HEAD(&ap->list);
	list_add(&p->list, &ap->list);

	INIT_HLIST_NODE(&ap->hlist);
	hlist_del(&p->hlist);
	hlist_add_head(&ap->hlist,
		&kprobe_table[hash_ptr(ap->addr, KPROBE_HASH_BITS)]);
}

/*
 * This is the second or subsequent kprobe at the address - handle
 * the intricacies
 * TODO: Move kcalloc outside the spinlock
 */
static int __kprobes register_aggr_kprobe(struct kprobe *old_p,
					  struct kprobe *p)
{
	int ret = 0;
	struct kprobe *ap;

	if (old_p->pre_handler == aggr_pre_handler) {
		copy_kprobe(old_p, p);
		ret = add_new_kprobe(old_p, p);
	} else {
		ap = kcalloc(1, sizeof(struct kprobe), GFP_ATOMIC);
		if (!ap)
			return -ENOMEM;
		add_aggr_kprobe(ap, old_p);
		copy_kprobe(ap, p);
		ret = add_new_kprobe(ap, p);
	}
	return ret;
}

/* kprobe removal house-keeping routines */
static inline void cleanup_kprobe(struct kprobe *p, unsigned long flags)
{
	arch_disarm_kprobe(p);
	hlist_del(&p->hlist);
	spin_unlock_irqrestore(&kprobe_lock, flags);
	arch_remove_kprobe(p);
}

static inline void cleanup_aggr_kprobe(struct kprobe *old_p,
		struct kprobe *p, unsigned long flags)
{
	list_del(&p->list);
	if (list_empty(&old_p->list)) {
		cleanup_kprobe(old_p, flags);
		kfree(old_p);
	} else
		spin_unlock_irqrestore(&kprobe_lock, flags);
}

static int __kprobes in_kprobes_functions(unsigned long addr)
{
	if (addr >= (unsigned long)__kprobes_text_start
		&& addr < (unsigned long)__kprobes_text_end)
		return -EINVAL;
	return 0;
}

int __kprobes register_kprobe(struct kprobe *p)
{
	int ret = 0;
	unsigned long flags = 0;
	struct kprobe *old_p;

	if ((ret = in_kprobes_functions((unsigned long) p->addr)) != 0)
		return ret;
	if ((ret = arch_prepare_kprobe(p)) != 0)
		goto rm_kprobe;

	spin_lock_irqsave(&kprobe_lock, flags);
	old_p = get_kprobe(p->addr);
	p->nmissed = 0;
	if (old_p) {
		ret = register_aggr_kprobe(old_p, p);
		goto out;
	}

	arch_copy_kprobe(p);
	INIT_HLIST_NODE(&p->hlist);
	hlist_add_head(&p->hlist,
		       &kprobe_table[hash_ptr(p->addr, KPROBE_HASH_BITS)]);

  	arch_arm_kprobe(p);

out:
	spin_unlock_irqrestore(&kprobe_lock, flags);
rm_kprobe:
	if (ret == -EEXIST)
		arch_remove_kprobe(p);
	return ret;
}

void __kprobes unregister_kprobe(struct kprobe *p)
{
	unsigned long flags;
	struct kprobe *old_p;

	spin_lock_irqsave(&kprobe_lock, flags);
	old_p = get_kprobe(p->addr);
	if (old_p) {
		if (old_p->pre_handler == aggr_pre_handler)
			cleanup_aggr_kprobe(old_p, p, flags);
		else
			cleanup_kprobe(p, flags);
	} else
		spin_unlock_irqrestore(&kprobe_lock, flags);
}

static struct notifier_block kprobe_exceptions_nb = {
	.notifier_call = kprobe_exceptions_notify,
	.priority = 0x7fffffff /* we need to notified first */
};

int __kprobes register_jprobe(struct jprobe *jp)
{
	/* Todo: Verify probepoint is a function entry point */
	jp->kp.pre_handler = setjmp_pre_handler;
	jp->kp.break_handler = longjmp_break_handler;

	return register_kprobe(&jp->kp);
}

void __kprobes unregister_jprobe(struct jprobe *jp)
{
	unregister_kprobe(&jp->kp);
}

#ifdef ARCH_SUPPORTS_KRETPROBES

int __kprobes register_kretprobe(struct kretprobe *rp)
{
	int ret = 0;
	struct kretprobe_instance *inst;
	int i;

	rp->kp.pre_handler = pre_handler_kretprobe;

	/* Pre-allocate memory for max kretprobe instances */
	if (rp->maxactive <= 0) {
#ifdef CONFIG_PREEMPT
		rp->maxactive = max(10, 2 * NR_CPUS);
#else
		rp->maxactive = NR_CPUS;
#endif
	}
	INIT_HLIST_HEAD(&rp->used_instances);
	INIT_HLIST_HEAD(&rp->free_instances);
	for (i = 0; i < rp->maxactive; i++) {
		inst = kmalloc(sizeof(struct kretprobe_instance), GFP_KERNEL);
		if (inst == NULL) {
			free_rp_inst(rp);
			return -ENOMEM;
		}
		INIT_HLIST_NODE(&inst->uflist);
		hlist_add_head(&inst->uflist, &rp->free_instances);
	}

	rp->nmissed = 0;
	/* Establish function entry probe point */
	if ((ret = register_kprobe(&rp->kp)) != 0)
		free_rp_inst(rp);
	return ret;
}

#else /* ARCH_SUPPORTS_KRETPROBES */

int __kprobes register_kretprobe(struct kretprobe *rp)
{
	return -ENOSYS;
}

#endif /* ARCH_SUPPORTS_KRETPROBES */

void __kprobes unregister_kretprobe(struct kretprobe *rp)
{
	unsigned long flags;
	struct kretprobe_instance *ri;

	unregister_kprobe(&rp->kp);
	/* No race here */
	spin_lock_irqsave(&kprobe_lock, flags);
	free_rp_inst(rp);
	while ((ri = get_used_rp_inst(rp)) != NULL) {
		ri->rp = NULL;
		hlist_del(&ri->uflist);
	}
	spin_unlock_irqrestore(&kprobe_lock, flags);
}

static int __init init_kprobes(void)
{
	int i, err = 0;

	/* FIXME allocate the probe table, currently defined statically */
	/* initialize all list heads */
	for (i = 0; i < KPROBE_TABLE_SIZE; i++) {
		INIT_HLIST_HEAD(&kprobe_table[i]);
		INIT_HLIST_HEAD(&kretprobe_inst_table[i]);
	}

	err = arch_init_kprobes();
	if (!err)
		err = register_die_notifier(&kprobe_exceptions_nb);

	return err;
}

__initcall(init_kprobes);

EXPORT_SYMBOL_GPL(register_kprobe);
EXPORT_SYMBOL_GPL(unregister_kprobe);
EXPORT_SYMBOL_GPL(register_jprobe);
EXPORT_SYMBOL_GPL(unregister_jprobe);
EXPORT_SYMBOL_GPL(jprobe_return);
EXPORT_SYMBOL_GPL(register_kretprobe);
EXPORT_SYMBOL_GPL(unregister_kretprobe);

