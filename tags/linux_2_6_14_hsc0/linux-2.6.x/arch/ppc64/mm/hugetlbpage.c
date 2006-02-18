/*
 * PPC64 (POWER4) Huge TLB Page Support for Kernel.
 *
 * Copyright (C) 2003 David Gibson, IBM Corporation.
 *
 * Based on the IA-32 version:
 * Copyright (C) 2002, Rohit Seth <rohit.seth@intel.com>
 */

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/pagemap.h>
#include <linux/smp_lock.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/sysctl.h>
#include <asm/mman.h>
#include <asm/pgalloc.h>
#include <asm/tlb.h>
#include <asm/tlbflush.h>
#include <asm/mmu_context.h>
#include <asm/machdep.h>
#include <asm/cputable.h>
#include <asm/tlb.h>

#include <linux/sysctl.h>

#define NUM_LOW_AREAS	(0x100000000UL >> SID_SHIFT)
#define NUM_HIGH_AREAS	(PGTABLE_RANGE >> HTLB_AREA_SHIFT)

/* Modelled after find_linux_pte() */
pte_t *huge_pte_offset(struct mm_struct *mm, unsigned long addr)
{
	pgd_t *pg;
	pud_t *pu;
	pmd_t *pm;
	pte_t *pt;

	BUG_ON(! in_hugepage_area(mm->context, addr));

	addr &= HPAGE_MASK;

	pg = pgd_offset(mm, addr);
	if (!pgd_none(*pg)) {
		pu = pud_offset(pg, addr);
		if (!pud_none(*pu)) {
			pm = pmd_offset(pu, addr);
			pt = (pte_t *)pm;
			BUG_ON(!pmd_none(*pm)
			       && !(pte_present(*pt) && pte_huge(*pt)));
			return pt;
		}
	}

	return NULL;
}

pte_t *huge_pte_alloc(struct mm_struct *mm, unsigned long addr)
{
	pgd_t *pg;
	pud_t *pu;
	pmd_t *pm;
	pte_t *pt;

	BUG_ON(! in_hugepage_area(mm->context, addr));

	addr &= HPAGE_MASK;

	pg = pgd_offset(mm, addr);
	pu = pud_alloc(mm, pg, addr);

	if (pu) {
		pm = pmd_alloc(mm, pu, addr);
		if (pm) {
			pt = (pte_t *)pm;
			BUG_ON(!pmd_none(*pm)
			       && !(pte_present(*pt) && pte_huge(*pt)));
			return pt;
		}
	}

	return NULL;
}

#define HUGEPTE_BATCH_SIZE	(HPAGE_SIZE / PMD_SIZE)

void set_huge_pte_at(struct mm_struct *mm, unsigned long addr,
		     pte_t *ptep, pte_t pte)
{
	int i;

	if (pte_present(*ptep)) {
		pte_clear(mm, addr, ptep);
		flush_tlb_pending();
	}

	for (i = 0; i < HUGEPTE_BATCH_SIZE; i++) {
		*ptep = __pte(pte_val(pte) & ~_PAGE_HPTEFLAGS);
		ptep++;
	}
}

pte_t huge_ptep_get_and_clear(struct mm_struct *mm, unsigned long addr,
			      pte_t *ptep)
{
	unsigned long old = pte_update(ptep, ~0UL);
	int i;

	if (old & _PAGE_HASHPTE)
		hpte_update(mm, addr, old, 0);

	for (i = 1; i < HUGEPTE_BATCH_SIZE; i++)
		ptep[i] = __pte(0);

	return __pte(old);
}

/*
 * This function checks for proper alignment of input addr and len parameters.
 */
int is_aligned_hugepage_range(unsigned long addr, unsigned long len)
{
	if (len & ~HPAGE_MASK)
		return -EINVAL;
	if (addr & ~HPAGE_MASK)
		return -EINVAL;
	if (! (within_hugepage_low_range(addr, len)
	       || within_hugepage_high_range(addr, len)) )
		return -EINVAL;
	return 0;
}

static void flush_low_segments(void *parm)
{
	u16 areas = (unsigned long) parm;
	unsigned long i;

	asm volatile("isync" : : : "memory");

	BUILD_BUG_ON((sizeof(areas)*8) != NUM_LOW_AREAS);

	for (i = 0; i < NUM_LOW_AREAS; i++) {
		if (! (areas & (1U << i)))
			continue;
		asm volatile("slbie %0"
			     : : "r" ((i << SID_SHIFT) | SLBIE_C));
	}

	asm volatile("isync" : : : "memory");
}

static void flush_high_segments(void *parm)
{
	u16 areas = (unsigned long) parm;
	unsigned long i, j;

	asm volatile("isync" : : : "memory");

	BUILD_BUG_ON((sizeof(areas)*8) != NUM_HIGH_AREAS);

	for (i = 0; i < NUM_HIGH_AREAS; i++) {
		if (! (areas & (1U << i)))
			continue;
		for (j = 0; j < (1UL << (HTLB_AREA_SHIFT-SID_SHIFT)); j++)
			asm volatile("slbie %0"
				     :: "r" (((i << HTLB_AREA_SHIFT)
					     + (j << SID_SHIFT)) | SLBIE_C));
	}

	asm volatile("isync" : : : "memory");
}

static int prepare_low_area_for_htlb(struct mm_struct *mm, unsigned long area)
{
	unsigned long start = area << SID_SHIFT;
	unsigned long end = (area+1) << SID_SHIFT;
	struct vm_area_struct *vma;

	BUG_ON(area >= NUM_LOW_AREAS);

	/* Check no VMAs are in the region */
	vma = find_vma(mm, start);
	if (vma && (vma->vm_start < end))
		return -EBUSY;

	return 0;
}

static int prepare_high_area_for_htlb(struct mm_struct *mm, unsigned long area)
{
	unsigned long start = area << HTLB_AREA_SHIFT;
	unsigned long end = (area+1) << HTLB_AREA_SHIFT;
	struct vm_area_struct *vma;

	BUG_ON(area >= NUM_HIGH_AREAS);

	/* Check no VMAs are in the region */
	vma = find_vma(mm, start);
	if (vma && (vma->vm_start < end))
		return -EBUSY;

	return 0;
}

static int open_low_hpage_areas(struct mm_struct *mm, u16 newareas)
{
	unsigned long i;

	BUILD_BUG_ON((sizeof(newareas)*8) != NUM_LOW_AREAS);
	BUILD_BUG_ON((sizeof(mm->context.low_htlb_areas)*8) != NUM_LOW_AREAS);

	newareas &= ~(mm->context.low_htlb_areas);
	if (! newareas)
		return 0; /* The segments we want are already open */

	for (i = 0; i < NUM_LOW_AREAS; i++)
		if ((1 << i) & newareas)
			if (prepare_low_area_for_htlb(mm, i) != 0)
				return -EBUSY;

	mm->context.low_htlb_areas |= newareas;

	/* update the paca copy of the context struct */
	get_paca()->context = mm->context;

	/* the context change must make it to memory before the flush,
	 * so that further SLB misses do the right thing. */
	mb();
	on_each_cpu(flush_low_segments, (void *)(unsigned long)newareas, 0, 1);

	return 0;
}

static int open_high_hpage_areas(struct mm_struct *mm, u16 newareas)
{
	unsigned long i;

	BUILD_BUG_ON((sizeof(newareas)*8) != NUM_HIGH_AREAS);
	BUILD_BUG_ON((sizeof(mm->context.high_htlb_areas)*8)
		     != NUM_HIGH_AREAS);

	newareas &= ~(mm->context.high_htlb_areas);
	if (! newareas)
		return 0; /* The areas we want are already open */

	for (i = 0; i < NUM_HIGH_AREAS; i++)
		if ((1 << i) & newareas)
			if (prepare_high_area_for_htlb(mm, i) != 0)
				return -EBUSY;

	mm->context.high_htlb_areas |= newareas;

	/* update the paca copy of the context struct */
	get_paca()->context = mm->context;

	/* the context change must make it to memory before the flush,
	 * so that further SLB misses do the right thing. */
	mb();
	on_each_cpu(flush_high_segments, (void *)(unsigned long)newareas, 0, 1);

	return 0;
}

int prepare_hugepage_range(unsigned long addr, unsigned long len)
{
	int err;

	if ( (addr+len) < addr )
		return -EINVAL;

	if ((addr + len) < 0x100000000UL)
		err = open_low_hpage_areas(current->mm,
					  LOW_ESID_MASK(addr, len));
	else
		err = open_high_hpage_areas(current->mm,
					    HTLB_AREA_MASK(addr, len));
	if (err) {
		printk(KERN_DEBUG "prepare_hugepage_range(%lx, %lx)"
		       " failed (lowmask: 0x%04hx, highmask: 0x%04hx)\n",
		       addr, len,
		       LOW_ESID_MASK(addr, len), HTLB_AREA_MASK(addr, len));
		return err;
	}

	return 0;
}

struct page *
follow_huge_addr(struct mm_struct *mm, unsigned long address, int write)
{
	pte_t *ptep;
	struct page *page;

	if (! in_hugepage_area(mm->context, address))
		return ERR_PTR(-EINVAL);

	ptep = huge_pte_offset(mm, address);
	page = pte_page(*ptep);
	if (page)
		page += (address % HPAGE_SIZE) / PAGE_SIZE;

	return page;
}

int pmd_huge(pmd_t pmd)
{
	return 0;
}

struct page *
follow_huge_pmd(struct mm_struct *mm, unsigned long address,
		pmd_t *pmd, int write)
{
	BUG();
	return NULL;
}

/* Because we have an exclusive hugepage region which lies within the
 * normal user address space, we have to take special measures to make
 * non-huge mmap()s evade the hugepage reserved regions. */
unsigned long arch_get_unmapped_area(struct file *filp, unsigned long addr,
				     unsigned long len, unsigned long pgoff,
				     unsigned long flags)
{
	struct mm_struct *mm = current->mm;
	struct vm_area_struct *vma;
	unsigned long start_addr;

	if (len > TASK_SIZE)
		return -ENOMEM;

	if (addr) {
		addr = PAGE_ALIGN(addr);
		vma = find_vma(mm, addr);
		if (((TASK_SIZE - len) >= addr)
		    && (!vma || (addr+len) <= vma->vm_start)
		    && !is_hugepage_only_range(mm, addr,len))
			return addr;
	}
	if (len > mm->cached_hole_size) {
	        start_addr = addr = mm->free_area_cache;
	} else {
	        start_addr = addr = TASK_UNMAPPED_BASE;
	        mm->cached_hole_size = 0;
	}

full_search:
	vma = find_vma(mm, addr);
	while (TASK_SIZE - len >= addr) {
		BUG_ON(vma && (addr >= vma->vm_end));

		if (touches_hugepage_low_range(mm, addr, len)) {
			addr = ALIGN(addr+1, 1<<SID_SHIFT);
			vma = find_vma(mm, addr);
			continue;
		}
		if (touches_hugepage_high_range(mm, addr, len)) {
			addr = ALIGN(addr+1, 1UL<<HTLB_AREA_SHIFT);
			vma = find_vma(mm, addr);
			continue;
		}
		if (!vma || addr + len <= vma->vm_start) {
			/*
			 * Remember the place where we stopped the search:
			 */
			mm->free_area_cache = addr + len;
			return addr;
		}
		if (addr + mm->cached_hole_size < vma->vm_start)
		        mm->cached_hole_size = vma->vm_start - addr;
		addr = vma->vm_end;
		vma = vma->vm_next;
	}

	/* Make sure we didn't miss any holes */
	if (start_addr != TASK_UNMAPPED_BASE) {
		start_addr = addr = TASK_UNMAPPED_BASE;
		mm->cached_hole_size = 0;
		goto full_search;
	}
	return -ENOMEM;
}

/*
 * This mmap-allocator allocates new areas top-down from below the
 * stack's low limit (the base):
 *
 * Because we have an exclusive hugepage region which lies within the
 * normal user address space, we have to take special measures to make
 * non-huge mmap()s evade the hugepage reserved regions.
 */
unsigned long
arch_get_unmapped_area_topdown(struct file *filp, const unsigned long addr0,
			  const unsigned long len, const unsigned long pgoff,
			  const unsigned long flags)
{
	struct vm_area_struct *vma, *prev_vma;
	struct mm_struct *mm = current->mm;
	unsigned long base = mm->mmap_base, addr = addr0;
	unsigned long largest_hole = mm->cached_hole_size;
	int first_time = 1;

	/* requested length too big for entire address space */
	if (len > TASK_SIZE)
		return -ENOMEM;

	/* dont allow allocations above current base */
	if (mm->free_area_cache > base)
		mm->free_area_cache = base;

	/* requesting a specific address */
	if (addr) {
		addr = PAGE_ALIGN(addr);
		vma = find_vma(mm, addr);
		if (TASK_SIZE - len >= addr &&
				(!vma || addr + len <= vma->vm_start)
				&& !is_hugepage_only_range(mm, addr,len))
			return addr;
	}

	if (len <= largest_hole) {
	        largest_hole = 0;
		mm->free_area_cache = base;
	}
try_again:
	/* make sure it can fit in the remaining address space */
	if (mm->free_area_cache < len)
		goto fail;

	/* either no address requested or cant fit in requested address hole */
	addr = (mm->free_area_cache - len) & PAGE_MASK;
	do {
hugepage_recheck:
		if (touches_hugepage_low_range(mm, addr, len)) {
			addr = (addr & ((~0) << SID_SHIFT)) - len;
			goto hugepage_recheck;
		} else if (touches_hugepage_high_range(mm, addr, len)) {
			addr = (addr & ((~0UL) << HTLB_AREA_SHIFT)) - len;
			goto hugepage_recheck;
		}

		/*
		 * Lookup failure means no vma is above this address,
		 * i.e. return with success:
		 */
 	 	if (!(vma = find_vma_prev(mm, addr, &prev_vma)))
			return addr;

		/*
		 * new region fits between prev_vma->vm_end and
		 * vma->vm_start, use it:
		 */
		if (addr+len <= vma->vm_start &&
		          (!prev_vma || (addr >= prev_vma->vm_end))) {
			/* remember the address as a hint for next time */
		        mm->cached_hole_size = largest_hole;
		        return (mm->free_area_cache = addr);
		} else {
			/* pull free_area_cache down to the first hole */
		        if (mm->free_area_cache == vma->vm_end) {
				mm->free_area_cache = vma->vm_start;
				mm->cached_hole_size = largest_hole;
			}
		}

		/* remember the largest hole we saw so far */
		if (addr + largest_hole < vma->vm_start)
		        largest_hole = vma->vm_start - addr;

		/* try just below the current vma->vm_start */
		addr = vma->vm_start-len;
	} while (len <= vma->vm_start);

fail:
	/*
	 * if hint left us with no space for the requested
	 * mapping then try again:
	 */
	if (first_time) {
		mm->free_area_cache = base;
		largest_hole = 0;
		first_time = 0;
		goto try_again;
	}
	/*
	 * A failed mmap() very likely causes application failure,
	 * so fall back to the bottom-up function here. This scenario
	 * can happen with large stack limits and large mmap()
	 * allocations.
	 */
	mm->free_area_cache = TASK_UNMAPPED_BASE;
	mm->cached_hole_size = ~0UL;
	addr = arch_get_unmapped_area(filp, addr0, len, pgoff, flags);
	/*
	 * Restore the topdown base:
	 */
	mm->free_area_cache = base;
	mm->cached_hole_size = ~0UL;

	return addr;
}

static unsigned long htlb_get_low_area(unsigned long len, u16 segmask)
{
	unsigned long addr = 0;
	struct vm_area_struct *vma;

	vma = find_vma(current->mm, addr);
	while (addr + len <= 0x100000000UL) {
		BUG_ON(vma && (addr >= vma->vm_end)); /* invariant */

		if (! __within_hugepage_low_range(addr, len, segmask)) {
			addr = ALIGN(addr+1, 1<<SID_SHIFT);
			vma = find_vma(current->mm, addr);
			continue;
		}

		if (!vma || (addr + len) <= vma->vm_start)
			return addr;
		addr = ALIGN(vma->vm_end, HPAGE_SIZE);
		/* Depending on segmask this might not be a confirmed
		 * hugepage region, so the ALIGN could have skipped
		 * some VMAs */
		vma = find_vma(current->mm, addr);
	}

	return -ENOMEM;
}

static unsigned long htlb_get_high_area(unsigned long len, u16 areamask)
{
	unsigned long addr = 0x100000000UL;
	struct vm_area_struct *vma;

	vma = find_vma(current->mm, addr);
	while (addr + len <= TASK_SIZE_USER64) {
		BUG_ON(vma && (addr >= vma->vm_end)); /* invariant */

		if (! __within_hugepage_high_range(addr, len, areamask)) {
			addr = ALIGN(addr+1, 1UL<<HTLB_AREA_SHIFT);
			vma = find_vma(current->mm, addr);
			continue;
		}

		if (!vma || (addr + len) <= vma->vm_start)
			return addr;
		addr = ALIGN(vma->vm_end, HPAGE_SIZE);
		/* Depending on segmask this might not be a confirmed
		 * hugepage region, so the ALIGN could have skipped
		 * some VMAs */
		vma = find_vma(current->mm, addr);
	}

	return -ENOMEM;
}

unsigned long hugetlb_get_unmapped_area(struct file *file, unsigned long addr,
					unsigned long len, unsigned long pgoff,
					unsigned long flags)
{
	int lastshift;
	u16 areamask, curareas;

	if (len & ~HPAGE_MASK)
		return -EINVAL;

	if (!cpu_has_feature(CPU_FTR_16M_PAGE))
		return -EINVAL;

	if (test_thread_flag(TIF_32BIT)) {
		curareas = current->mm->context.low_htlb_areas;

		/* First see if we can do the mapping in the existing
		 * low areas */
		addr = htlb_get_low_area(len, curareas);
		if (addr != -ENOMEM)
			return addr;

		lastshift = 0;
		for (areamask = LOW_ESID_MASK(0x100000000UL-len, len);
		     ! lastshift; areamask >>=1) {
			if (areamask & 1)
				lastshift = 1;

			addr = htlb_get_low_area(len, curareas | areamask);
			if ((addr != -ENOMEM)
			    && open_low_hpage_areas(current->mm, areamask) == 0)
				return addr;
		}
	} else {
		curareas = current->mm->context.high_htlb_areas;

		/* First see if we can do the mapping in the existing
		 * high areas */
		addr = htlb_get_high_area(len, curareas);
		if (addr != -ENOMEM)
			return addr;

		lastshift = 0;
		for (areamask = HTLB_AREA_MASK(TASK_SIZE_USER64-len, len);
		     ! lastshift; areamask >>=1) {
			if (areamask & 1)
				lastshift = 1;

			addr = htlb_get_high_area(len, curareas | areamask);
			if ((addr != -ENOMEM)
			    && open_high_hpage_areas(current->mm, areamask) == 0)
				return addr;
		}
	}
	printk(KERN_DEBUG "hugetlb_get_unmapped_area() unable to open"
	       " enough areas\n");
	return -ENOMEM;
}

int hash_huge_page(struct mm_struct *mm, unsigned long access,
		   unsigned long ea, unsigned long vsid, int local)
{
	pte_t *ptep;
	unsigned long va, vpn;
	pte_t old_pte, new_pte;
	unsigned long rflags, prpn;
	long slot;
	int err = 1;

	spin_lock(&mm->page_table_lock);

	ptep = huge_pte_offset(mm, ea);

	/* Search the Linux page table for a match with va */
	va = (vsid << 28) | (ea & 0x0fffffff);
	vpn = va >> HPAGE_SHIFT;

	/*
	 * If no pte found or not present, send the problem up to
	 * do_page_fault
	 */
	if (unlikely(!ptep || pte_none(*ptep)))
		goto out;

/* 	BUG_ON(pte_bad(*ptep)); */

	/* 
	 * Check the user's access rights to the page.  If access should be
	 * prevented then send the problem up to do_page_fault.
	 */
	if (unlikely(access & ~pte_val(*ptep)))
		goto out;
	/*
	 * At this point, we have a pte (old_pte) which can be used to build
	 * or update an HPTE. There are 2 cases:
	 *
	 * 1. There is a valid (present) pte with no associated HPTE (this is 
	 *	the most common case)
	 * 2. There is a valid (present) pte with an associated HPTE. The
	 *	current values of the pp bits in the HPTE prevent access
	 *	because we are doing software DIRTY bit management and the
	 *	page is currently not DIRTY. 
	 */


	old_pte = *ptep;
	new_pte = old_pte;

	rflags = 0x2 | (! (pte_val(new_pte) & _PAGE_RW));
 	/* _PAGE_EXEC -> HW_NO_EXEC since it's inverted */
	rflags |= ((pte_val(new_pte) & _PAGE_EXEC) ? 0 : HW_NO_EXEC);

	/* Check if pte already has an hpte (case 2) */
	if (unlikely(pte_val(old_pte) & _PAGE_HASHPTE)) {
		/* There MIGHT be an HPTE for this pte */
		unsigned long hash, slot;

		hash = hpt_hash(vpn, 1);
		if (pte_val(old_pte) & _PAGE_SECONDARY)
			hash = ~hash;
		slot = (hash & htab_hash_mask) * HPTES_PER_GROUP;
		slot += (pte_val(old_pte) & _PAGE_GROUP_IX) >> 12;

		if (ppc_md.hpte_updatepp(slot, rflags, va, 1, local) == -1)
			pte_val(old_pte) &= ~_PAGE_HPTEFLAGS;
	}

	if (likely(!(pte_val(old_pte) & _PAGE_HASHPTE))) {
		unsigned long hash = hpt_hash(vpn, 1);
		unsigned long hpte_group;

		prpn = pte_pfn(old_pte);

repeat:
		hpte_group = ((hash & htab_hash_mask) *
			      HPTES_PER_GROUP) & ~0x7UL;

		/* Update the linux pte with the HPTE slot */
		pte_val(new_pte) &= ~_PAGE_HPTEFLAGS;
		pte_val(new_pte) |= _PAGE_HASHPTE;

		/* Add in WIMG bits */
		/* XXX We should store these in the pte */
		rflags |= _PAGE_COHERENT;

		slot = ppc_md.hpte_insert(hpte_group, va, prpn,
					  HPTE_V_LARGE, rflags);

		/* Primary is full, try the secondary */
		if (unlikely(slot == -1)) {
			pte_val(new_pte) |= _PAGE_SECONDARY;
			hpte_group = ((~hash & htab_hash_mask) *
				      HPTES_PER_GROUP) & ~0x7UL; 
			slot = ppc_md.hpte_insert(hpte_group, va, prpn,
						  HPTE_V_LARGE |
						  HPTE_V_SECONDARY,
						  rflags);
			if (slot == -1) {
				if (mftb() & 0x1)
					hpte_group = ((hash & htab_hash_mask) *
						      HPTES_PER_GROUP)&~0x7UL;

				ppc_md.hpte_remove(hpte_group);
				goto repeat;
                        }
		}

		if (unlikely(slot == -2))
			panic("hash_huge_page: pte_insert failed\n");

		pte_val(new_pte) |= (slot<<12) & _PAGE_GROUP_IX;

		/* 
		 * No need to use ldarx/stdcx here because all who
		 * might be updating the pte will hold the
		 * page_table_lock
		 */
		*ptep = new_pte;
	}

	err = 0;

 out:
	spin_unlock(&mm->page_table_lock);

	return err;
}
