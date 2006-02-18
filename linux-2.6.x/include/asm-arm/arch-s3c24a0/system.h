/*
 * include/asm-arm/arch-s3c24a0/system.h
 * 
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/config.h>
#include <asm/arch/hardware.h>

static inline 
void arch_idle(void)
{
	/* TODO */
	cpu_do_idle(/*0*/);
}

static inline 
void arch_reset(char mode)
{
	if (mode == 's') {
		/* Jump into ROM at address 0 */
		cpu_reset(0);
	} else {
		WTCNT = 0x100;
		WTDAT = 0x100;
		WTCON = 0x8021;
	}
}
