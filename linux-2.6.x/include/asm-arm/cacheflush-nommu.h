/*
 *  linux/include/asm-arm/cacheflush-nommu.h
 *
 *  Copyright (C) 2003 Hyok S. Choi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _ASMARM_CACHEFLUSH_NOMMU_H
#define _ASMARM_CACHEFLUSH_NOMMU_H

/*
 * No cache flushing is required when address mappings are
 * changed, because the caches on ARM without MMU are physically
 * addressed.
 */
#define flush_cache_all()		do { } while (0)
#define flush_cache_mm(mm)		do { } while (0)
#define flush_cache_range(vma, a, b)	do { } while (0)
#define flush_cache_page(vma, p, pfn)	do { } while (0)
#define flush_cache_vmap(start, end)	do { } while (0)
#define flush_cache_vunmap(start, end)	do { } while (0)
#define flush_icache_page(vma, page)	do { } while (0)

/*
 * We have no D-cache aliasing on ARM, so this call does nothing
 */
#define flush_dcache_page(page)		do { } while (0)

/* flush_icache_range() must
 * - clean the data cache
 * - drain the write buffer
 * - invalidate the instruction cache. */
#define flush_icache_range(s,e)		  __cpuc_coherent_kern_range(s,e)

/*
 * copy_to_user_page() is a special case, because we must clean
 * the dcache+drain the write buffer+invalidate the instruction cache.
 * ARM has no snooping caches...
 */
#define copy_to_user_page(vma, page, vaddr, dst, src, len) 		\
	do { 								\
		memcpy(dst, src, len);					\
		flush_icache_range((unsigned long)(dst),		\
		                  ((unsigned long)(dst))+(len)); 	\
	} while (0)

/* No cache problems if reading from user space... */
#define copy_from_user_page(vma, page, vaddr, dst, src, len) 		\
	memcpy(dst, src, len)

/*
 * Write any modified data cache blocks out to memory and invalidate them.
 * Drain the write buffer.
 * Does not invalidate/update the corresponding instruction cache blocks.
 */
#define flush_dcache_range(start,stop)	      dmac_flush_range(start,stop)

/* Flush the range of user (defined by vma->vm_mm) address space
 * starting at 'addr' for 'len' bytes from the cache.  The range does
 * not straddle a page boundary, the unique physical page containing
 * the range is 'page'.  This seems to be used mainly for invalidating
 * an address range following a poke into the program text through the
 * ptrace() call from another process (e.g. for BRK instruction
 * insertion). */

#define flush_icache_user_range(vma,page,addr,len)			\
		flush_icache_range((unsigned long)(addr),		\
		                  ((unsigned long)(addr))+(len))
/*
 * flush_cache_user_range is used when we want to ensure that the
 * Harvard caches are synchronised for the user space address range.
 * This is used for the ARM private sys_cacheflush system call.
 */
#define flush_cache_user_range(vma,s,e)   	  flush_icache_range(s,e)

/*
 * Some locking stuff....
 */
#define flush_dcache_mmap_lock(mapping) \
        write_lock_irq(&(mapping)->tree_lock)
#define flush_dcache_mmap_unlock(mapping) \
        write_unlock_irq(&(mapping)->tree_lock)

#endif
