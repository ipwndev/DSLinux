/* 
 * Copyright (C) 2002 Jeff Dike (jdike@karaya.com)
 * Licensed under the GPL
 */

#ifndef __TLB_H__
#define __TLB_H__

#include "um_mmu.h"

struct host_vm_op {
	enum { NONE, MMAP, MUNMAP, MPROTECT } type;
	union {
		struct {
			unsigned long addr;
			unsigned long len;
			unsigned int r:1;
			unsigned int w:1;
			unsigned int x:1;
			int fd;
			__u64 offset;
		} mmap;
		struct {
			unsigned long addr;
			unsigned long len;
		} munmap;
		struct {
			unsigned long addr;
			unsigned long len;
			unsigned int r:1;
			unsigned int w:1;
			unsigned int x:1;
		} mprotect;
	} u;
};

extern void mprotect_kernel_vm(int w);
extern void force_flush_all(void);
extern void fix_range_common(struct mm_struct *mm, unsigned long start_addr,
                             unsigned long end_addr, int force,
			     int (*do_ops)(union mm_context *,
					   struct host_vm_op *, int, int,
					   void **));
extern int flush_tlb_kernel_range_common(unsigned long start,
					 unsigned long end);

#endif
