/*
 * ARM/ARM26 default IDE host driver
 *
 * Copyright (C) 2004 Bartlomiej Zolnierkiewicz
 * Based on code by: Russell King, Ian Molton and Alexander Schulz.
 *
 * May be copied or modified under the terms of the GNU General Public License.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ide.h>

#include <asm/mach-types.h>
#include <asm/irq.h>

#ifdef CONFIG_ARM26
# define IDE_ARM_HOST	(machine_is_a5k())
#else
# define IDE_ARM_HOST	(1)
#endif

#ifdef CONFIG_ARCH_CLPS7500
# include <asm/arch/hardware.h>
#
# define IDE_ARM_IO	(ISASLOT_IO + 0x1f0)
# define IDE_ARM_IRQ	IRQ_ISA_14
#else
# define IDE_ARM_IO	0x1f0
# define IDE_ARM_IRQ	IRQ_HARDDISK
#endif

#ifdef CONFIG_IDE_NDS_M3
static void M3_Unlock( void )
{
	volatile u16 tmp ;
	tmp = *(volatile u16 *)0x08000000 ;
	tmp = *(volatile u16 *)0x08E00002 ;
	tmp = *(volatile u16 *)0x0800000E ;
	tmp = *(volatile u16 *)0x08801FFC ;
	tmp = *(volatile u16 *)0x0800104A ;
	tmp = *(volatile u16 *)0x08800612 ;
	tmp = *(volatile u16 *)0x08000000 ;
	tmp = *(volatile u16 *)0x08801B66 ;
	tmp = *(volatile u16 *)0x08800006 ;
	tmp = *(volatile u16 *)0x08000000 ;
}
#endif

#ifdef CONFIG_IDE_NDS_SUPERCARD
static void Supercard_Unlock( void )
{
	*(volatile u16 *)0x09fffffe = 0xa55a ;
	*(volatile u16 *)0x09fffffe = 0xa55a ;
	*(volatile u16 *)0x09fffffe = 0x3 ;
	*(volatile u16 *)0x09fffffe = 0x3 ;
}
#endif

void __init ide_arm_init(void)
{
	if (IDE_ARM_HOST) {
		hw_regs_t hw;
		int i;

		memset(&hw, 0, sizeof(hw));
#ifdef CONFIG_IDE_NDS_SUPERCARD
		Supercard_Unlock();
#endif
#ifdef CONFIG_IDE_NDS_M3
		M3_Unlock();
		for (i=0; i<8; i++)
			hw.io_ports[i] = 0x08800000 + 0x20000*i;
		hw.io_ports[8] = 0x080C0000; // control
#else
		for (i=0; i<8; i++)
			hw.io_ports[i] = 0x09000000 + 0x20000*i;
		hw.io_ports[8] = 0x098C0000; // control
#endif
		hw.irq = IRQ_CART;
		ide_register_hw(&hw, NULL);
	}
}
