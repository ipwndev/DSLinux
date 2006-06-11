/*
 *  linux/arch/arm/mach-nds/arch.c
 *
 *  Copyright (C) 2004 SAMSUNG ELECTRONICS Co.,Ltd.
 *			      Hyok S. Choi (hyok.choi@samsung.com)
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/config.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysdev.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>

#include <asm/arch/power.h>
#include <asm/arch/fifo.h>

#define WAIT_CR 	0x04000204

extern struct sys_timer nds_timer;

extern void __init nds_init_irq(void);

static void poweroff(void)
{
	nds_fifo_send(FIFO_POWER);
}

extern void nds_machine_init(void)
{
	POWER_CR = POWER_2D | POWER_2D_SUB | POWER_LCD_TOP | POWER_LCD_BOTTOM | POWER_SWAP_LCDS ;

	/* Note: initial setup of wait_cr in head.S */

#ifdef CONFIG_NDS_FASTGBA
	// Switch to high speed:
	// bit 0-1     RAM-region access cycle control 0..3=10,8,6,18 cycles
	//     2-3     ROM 1st access cycle control    0..3=10,8,6,18 cycles
	//       4     ROM 2nd access cycle control    0..1=6,4 cycles
        writel( (readl(WAIT_CR) & 0xFFE0) | 0x001A, WAIT_CR);
#endif
	pm_power_off = poweroff;
}

MACHINE_START(NDS, "Nintendo DS")
	/* Maintainer: Malcolm Parsons <pepsiman@blueyonder.co.uk> */
	.phys_ram	= 0x02000000,
	.phys_io	= 0x04000000,
	.init_irq	= nds_init_irq,
	.timer		= &nds_timer,
	.init_machine	= nds_machine_init,
MACHINE_END
