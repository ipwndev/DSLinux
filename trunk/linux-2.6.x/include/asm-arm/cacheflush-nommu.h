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
 * Convert calls to our calling convention.
 */
#define flush_cache_all()	__cpuc_flush_kern_all()

#define flush_cache_mm(mm) __cpuc_flush_kern_all()
#define flush_cache_range(vma, start, end)	flush_cache_all()

#define flush_dcache_range(start,len)		flush_cache_all()
#define flush_dcache_page(page)			flush_cache_all()
#define flush_icache_user_range(vma,page,addr,len) 	flush_cache_all()

/*
 * Perform necessary cache operations to ensure that data previously
 * stored within this range of addresses can be executed by the CPU.
 */
#define flush_icache_range(s,e)		__cpuc_coherent_kern_range(s,e)
#define flush_icache_page(vma,pg)		do { } while (0)
#define flush_cache_page(vma, vmaddr, pfn)	do { } while (0)
#define flush_cache_user_range(vma,s,e)  flush_cache_range(vma,s,e)


#define flush_cache_vmap(start, end)		flush_cache_all()
#define flush_cache_vunmap(start, end)		flush_cache_all()

#define flush_dcache_mmap_lock(mapping) \
        write_lock_irq(&(mapping)->tree_lock)
#define flush_dcache_mmap_unlock(mapping) \
        write_unlock_irq(&(mapping)->tree_lock)

#define copy_to_user_page(vma, page, vaddr, dst, src, len) \
	memcpy(dst, src, len)
#define copy_from_user_page(vma, page, vaddr, dst, src, len) \
	memcpy(dst, src, len)


#endif
