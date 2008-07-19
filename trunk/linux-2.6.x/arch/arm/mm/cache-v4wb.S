/*
 *  linux/arch/arm/mm/cache-v4wb.S
 *
 *  Copyright (C) 1997-2002 Russell king
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/config.h>
#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/hardware.h>
#include <asm/page.h>
#include "proc-macros.S"

/*
 * The size of one data cache line.
 */
#define CACHE_DLINESIZE	32

/*
 * The total size of the data cache.
 */
#if defined(CONFIG_CPU_SA110)
# define CACHE_DSIZE	16384
#elif defined(CONFIG_CPU_SA1100)
# define CACHE_DSIZE	8192
#elif defined(CONFIG_ARCH_NDS)
# define CACHE_DSIZE	4096
#else
# error Unknown cache size
#endif

/*
 * This is the size at which it becomes more efficient to
 * clean the whole cache, rather than using the individual
 * cache line maintainence instructions.
 *
 *  Size  Clean (ticks) Dirty (ticks)
 *   4096   21  20  21    53  55  54
 *   8192   40  41  40   106 100 102
 *  16384   77  77  76   140 140 138
 *  32768  150 149 150   214 216 212 <---
 *  65536  296 297 296   351 358 361
 * 131072  591 591 591   656 657 651
 *  Whole  132 136 132   221 217 207 <---
 */
#define CACHE_DLIMIT	(CACHE_DSIZE * 4)

/*
 *	flush_user_cache_all()
 *
 *	Clean and invalidate all cache entries in a particular address
 *	space.
 */
ENTRY(v4wb_flush_user_cache_all)
	/* FALLTHROUGH */
/*
 *	flush_kern_cache_all()
 *
 *	Clean and invalidate the entire cache.
 */
ENTRY(v4wb_flush_kern_cache_all)
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0		@ invalidate I cache
	/* FALLTHROUGH */
/*
 *	flush_dcache_all()
 *
 *	Clean and invalidate the entire d-cache.
 */
ENTRY(v4wb_flush_dcache_all)
#ifdef CONFIG_ARCH_NDS				/* we can flush the D-cache line by line */
	mov	r0, #(CACHE_DSIZE/4)
1:	sub	r0, r0, #CACHE_DLINESIZE	@ next line
	mcr	p15, 0, r0, c7, c14, 2		@ clean and flush a D-cache line
	add	r0, r0, #0x40000000		@ next segment
	mcr	p15, 0, r0, c7, c14, 2		@ clean and flush a D-cache line
	add	r0, r0, #0x40000000		@ next segment
	mcr	p15, 0, r0, c7, c14, 2		@ clean and flush a D-cache line
	add	r0, r0, #0x40000000		@ next segment
	mcr	p15, 0, r0, c7, c14, 2		@ clean and flush a D-cache line
	adds	r0, r0, #0x40000000		@ next segment
	bne	1b				@ next cache line
#else
	mov	r0, #FLUSH_BASE
	add	r1, r0, #CACHE_DSIZE
1:	ldr	r2, [r0], #32
	cmp	r0, r1
	blo	1b
#endif
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ drain write buffer
	mov	pc, lr

/*
 *	flush_user_cache_range(start, end, flags)
 *
 *	Invalidate a range of cache entries in the specified
 *	address space.
 *
 *	- start - start address (inclusive, page aligned)
 *	- end	- end address (exclusive, page aligned)
 *	- flags	- vma_area_struct flags describing address space
 */
ENTRY(v4wb_flush_user_cache_range)
	tst	r2, #VM_EXEC			@ executable region?
	bne	v4wb_flush_cache_range		@ yes: flush i-cache and d-cache
	/* FALLTHROUGH */

/*
 *	dma_flush_range(start, end)
 *
 *	Clean and invalidate the specified virtual address range.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(v4wb_dma_flush_range)
	/* FALLTHROUGH */

/*
 *	flush_dcache_range(start, end)
 *
 *	Invalidate a range of D cache entries in the specified
 *	address space.
 *
 *	- start - start address (inclusive, page aligned)
 *	- end	- end address (exclusive, page aligned)
 */
ENTRY(v4wb_flush_dcache_range)
	sub	ip, r1, r0			@ calculate total size
	cmp	ip, #CACHE_DLIMIT		@ total size >= limit?
	bhs	v4wb_flush_dcache_all		@ yes: flush the complete d-cache
	bic	r0, r0, #CACHE_DLINESIZE - 1	@ safety first: adjust start pointer to cache line
1:
#ifdef CONFIG_ARCH_NDS				/* we can clean and invalidate at once */
	mcr	p15, 0, r0, c7, c14, 1		@ clean and flush a D-cache line
#else
	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
	mcr	p15, 0, r0, c7, c6, 1		@ invalidate D entry
#endif
	add	r0, r0, #CACHE_DLINESIZE
	cmp	r0, r1
	blo	1b
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ drain write buffer
	mov	pc, lr

/*
 *	flush_kern_dcache_page(void *page)
 *
 *	Ensure no D cache aliasing occurs, either with itself or
 *	the I cache
 *
 *	- addr	- page aligned address
 */
ENTRY(v4wb_flush_kern_dcache_page)
	add	r1, r0, #PAGE_SZ
	/* fall through */

/*
 *	coherent_kern_range(start, end)
 *
 *	Ensure coherency between the Icache and the Dcache in the
 *	region described by start.  If you have non-snooping
 *	Harvard caches, you need to implement this function.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(v4wb_coherent_kern_range)
	/* fall through */

/*
 *	coherent_user_range(start, end)
 *
 *	Ensure coherency between the Icache and the Dcache in the
 *	region described by start.  If you have non-snooping
 *	Harvard caches, you need to implement this function.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(v4wb_coherent_user_range)
	/* fall through */
/*
 *	flush_cache_range(start, end)
 *
 *	Invalidate a range of I+D cache entries in the specified
 *	address space.
 *
 *	- start - start address (inclusive, page aligned)
 *	- end	- end address (exclusive, page aligned)
 */
ENTRY(v4wb_flush_cache_range)
	sub	ip, r1, r0			@ calculate total size
	cmp	ip, #CACHE_DLIMIT		@ total size >= limit?
	bhs	v4wb_flush_kern_cache_all	@ yes: flush the complete i+d-cache
	bic	r0, r0, #CACHE_DLINESIZE - 1	@ safety first: adjust start pointer to cache line
2:
#ifdef CONFIG_ARCH_NDS				/* we can clean and invalidate at once */
	mcr	p15, 0, r0, c7, c14, 1		@ clean and flush a D-cache line
#else
	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
	mcr	p15, 0, r0, c7, c6, 1		@ invalidate D entry
#endif
	mcr	p15, 0, r0, c7, c5, 1		@ invalidate I entry
	add	r0, r0, #CACHE_DLINESIZE
	cmp	r0, r1
	blo	2b
	mov	ip, #0
	mcr	p15, 0, ip, c7, c10, 4		@ drain write buffer
	mov	pc, lr
/*
 *	dma_inv_range(start, end)
 *
 *	Invalidate (discard) the specified virtual address range.
 *	May not write back any entries.  If 'start' or 'end'
 *	are not cache line aligned, those lines must be written
 *	back.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(v4wb_dma_inv_range)
	tst	r0, #CACHE_DLINESIZE - 1
	bic	r0, r0, #CACHE_DLINESIZE - 1
	mcrne	p15, 0, r0, c7, c10, 1		@ clean D entry

	tst	r1, #CACHE_DLINESIZE - 1
	bicne	r2, r1, #CACHE_DLINESIZE - 1
	mcrne	p15, 0, r2, c7, c10, 1		@ clean D entry

1:	mcr	p15, 0, r0, c7, c6, 1		@ invalidate D entry
	add	r0, r0, #CACHE_DLINESIZE
	cmp	r0, r1
	blo	1b
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ drain write buffer
	mov	pc, lr

/*
 *	dma_clean_range(start, end)
 *
 *	Clean (write back) the specified virtual address range.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(v4wb_dma_clean_range)
	sub	ip, r1, r0			@ calculate total size
	cmp	ip, #CACHE_DLIMIT		@ total size >= limit?
	bhs	v4wb_dma_clean_all		@ yes: clean the complete d-cache
	bic	r0, r0, #CACHE_DLINESIZE - 1
1:	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
	add	r0, r0, #CACHE_DLINESIZE
	cmp	r0, r1
	blo	1b
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ drain write buffer
	mov	pc, lr

ENTRY(v4wb_dma_clean_all)
#ifdef CONFIG_ARCH_NDS				/* we can clean the D-cache line by line */
	mov	r0, #(CACHE_DSIZE/4)
1:	sub	r0, r0, #CACHE_DLINESIZE	@ next line
	mcr	p15, 0, r0, c7, c10, 2		@ clean a D-cache line
	add	r0, r0, #0x40000000		@ next segment
	mcr	p15, 0, r0, c7, c10, 2		@ clean a D-cache line
	add	r0, r0, #0x40000000		@ next segment
	mcr	p15, 0, r0, c7, c10, 2		@ clean a D-cache line
	add	r0, r0, #0x40000000		@ next segment
	mcr	p15, 0, r0, c7, c10, 2		@ clean a D-cache line
	adds	r0, r0, #0x40000000		@ next segment
	bne	1b				@ next cache line
#else
	mov	r0, #FLUSH_BASE
	add	r1, r0, #CACHE_DSIZE
1:	ldr	r2, [r0], #32
	cmp	r0, r1
	blo	1b
#endif
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ drain write buffer
	mov	pc, lr



	__INITDATA

	.type	v4wb_cache_fns, #object
ENTRY(v4wb_cache_fns)
	.long	v4wb_flush_kern_cache_all
	.long	v4wb_flush_user_cache_all
	.long	v4wb_flush_user_cache_range
	.long	v4wb_coherent_kern_range
	.long	v4wb_coherent_user_range
	.long	v4wb_flush_kern_dcache_page
	.long	v4wb_dma_inv_range
	.long	v4wb_dma_clean_range
	.long	v4wb_dma_flush_range
	.size	v4wb_cache_fns, . - v4wb_cache_fns
