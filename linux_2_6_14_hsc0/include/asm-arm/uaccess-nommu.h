/*
 *  linux/include/asm-arm/uaccess-nommu.h
 *
 *  Copyright (C) 2003 Hyok S. Choi, Samsung Electronics Co.,Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _ASMARM_UACCESS_NOMMU_H
#define _ASMARM_UACCESS_NOMMU_H

/*
 * Note that this is actually 0x1,0000,0000
 */
#define KERNEL_DS	0x00000000
/* uClinux has only one addr space. */
#define USER_DS		PAGE_OFFSET

#define get_ds()	(KERNEL_DS)
#define get_fs()	(USER_DS)	/* uClinux has only one addr space. */

static inline void set_fs (mm_segment_t fs)
{ /* nothing to do here for uClinux */
}

#define segment_eq(a,b)	((a) == (b))

/*
 * assuming __range_ok & __addr_ok always succeed.
 */
#define __addr_ok(addr) 			1
#define __range_ok(addr,size) 	0

#define access_ok(type,addr,size)	(__range_ok(addr,size) == 0)

#endif /* _ASMARM_UACCESS-NOMMU_H */
