/*
 *  linux/arch/arm/kernel/signal.h
 *
 *  Copyright (C) 2005 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifdef CONFIG_ARCH_NDS
#define KERN_SIGRETURN_CODE	0x00000500
#else
#define KERN_SIGRETURN_CODE	0xffff0500
#endif

extern const unsigned long sigreturn_codes[4];
