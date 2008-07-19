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

/*****************************************************************************/
#ifdef CONFIG_NDS_ROM8BIT
#define MAIN_MEM_BUFSIZE	256
static u16 cf_data_buf[MAIN_MEM_BUFSIZE] __attribute__ ((aligned (4)));
static void (*hw_ide_insw) (unsigned long port, u16 *dest, u32 count);
static void (*hw_ide_outsw) (unsigned long port, u16 *src, u32 count);
static void (*hw_ide_outb) (u8 value, unsigned long port);
#endif

/*****************************************************************************/
#if defined(CONFIG_IDE_NDS_SUPERCARD) && defined(CONFIG_NDS_ROM8BIT)
/* externals from sccf_s.S */
extern int sccf_detect_card(void);
extern u8  sccf_ide_inb(unsigned long port);
extern u16 sccf_ide_inw(unsigned long port);
extern u32 sccf_ide_inl(unsigned long port);

extern void sccf_ide_outb( u8 value, unsigned long port);
extern void sccf_ide_outw(u16 value, unsigned long port);
extern void sccf_ide_outl(u32 value, unsigned long port);

extern void sccf_ide_insw(unsigned long port, u16 *dest, u32 count);
extern void sccf_ide_outsw(unsigned long port, u16 *src, u32 count);
#endif

/*****************************************************************************/
#if defined(CONFIG_IDE_NDS_M3) && defined(CONFIG_NDS_ROM8BIT)
/* externals from m3cf_s.S */
extern int m3cf_detect_card(void);
extern u8  m3cf_ide_inb(unsigned long port);
extern u16 m3cf_ide_inw(unsigned long port);
extern u32 m3cf_ide_inl(unsigned long port);

extern void m3cf_ide_outb( u8 value, unsigned long port);
extern void m3cf_ide_outw(u16 value, unsigned long port);
extern void m3cf_ide_outl(u32 value, unsigned long port);

extern void m3cf_ide_insw(unsigned long port, u16 *dest, u32 count);
extern void m3cf_ide_outsw(unsigned long port, u16 *src, u32 count);
#endif

#if defined(CONFIG_IDE_NDS_M3) && !defined(CONFIG_NDS_ROM8BIT)
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

/*****************************************************************************/
#ifdef CONFIG_NDS_ROM8BIT
static void nds_ide_insw(unsigned long port, void *dest, u32 count)
{
	u16 *dst = (u16*)dest;
	do {
		u32 len = count;
		if (len > MAIN_MEM_BUFSIZE)
			len = MAIN_MEM_BUFSIZE;
		hw_ide_insw(port, cf_data_buf, len);
		memcpy(dst, cf_data_buf, len*sizeof(u16));
		count -= len;
		dst += len;
 	} while (count);
}

static void nds_ide_outsw(unsigned long port, void *source, u32 count)
{
	u16 *src = (u16*)source;
	do {
		u32 len = count;
		if (len > MAIN_MEM_BUFSIZE)
			len = MAIN_MEM_BUFSIZE;
		memcpy(cf_data_buf, src, len*sizeof(u16));
		hw_ide_outsw(port, cf_data_buf, len);
		count -= len;
		src += len;
 	} while (count);
}

static void nds_ide_insl(unsigned long port, void *dest, u32 count)
{
	nds_ide_insw(port, dest, count *2);	
}

static void nds_ide_outsl(unsigned long port, void *source, u32 count)
{
	nds_ide_outsw(port, source, count *2);	
}

static void nds_ide_outbsync(ide_drive_t *drive, u8 addr, unsigned long port)
{
	hw_ide_outb(addr, port);
}
#endif

/*****************************************************************************/

void __init ide_arm_init(void)
{
	if (IDE_ARM_HOST) {
		hw_regs_t hw;
		int i;

		memset(&hw, 0, sizeof(hw));

#if defined(CONFIG_IDE_NDS_SUPERCARD) && defined(CONFIG_NDS_ROM8BIT)
		if (sccf_detect_card()) {
			ide_hwif_t *hwif;

			printk( KERN_INFO "Supercard CF detected\n");
			
			/* setup register addresses */
			for (i=0; i<8; i++)
				hw.io_ports[i] = 0x09000000 + 0x20000*i;
			hw.io_ports[8] = 0x098C0000; // control
			/* setup IRQ */
			hw.irq = IRQ_CART;
			/* register hardware addresses */
			ide_register_hw(&hw, &hwif);
			/* modify IO functions */
			hw_ide_insw     = sccf_ide_insw;
			hw_ide_outsw    = sccf_ide_outsw;
			hw_ide_outb	= sccf_ide_outb;
			hwif->OUTB	= sccf_ide_outb;
			hwif->OUTBSYNC	= nds_ide_outbsync;
			hwif->OUTW	= sccf_ide_outw;
			hwif->OUTL	= sccf_ide_outl;
			hwif->OUTSW	= nds_ide_outsw;
			hwif->OUTSL	= nds_ide_outsl;
			hwif->INB	= sccf_ide_inb;
			hwif->INW	= sccf_ide_inw;
			hwif->INL	= sccf_ide_inl;
			hwif->INSW	= nds_ide_insw;
			hwif->INSL	= nds_ide_insl;
			/* all OK */
			return;
		}
#endif

#if defined(CONFIG_IDE_NDS_M3) && defined(CONFIG_NDS_ROM8BIT)
		if (m3cf_detect_card()) {
			ide_hwif_t *hwif;

			printk( KERN_INFO "M3 CF detected\n");

			/* setup register addresses */
			for (i=0; i<8; i++)
				hw.io_ports[i] = 0x08800000 + 0x20000*i;
			hw.io_ports[8] = 0x080C0000; // control
			/* setup IRQ */
			hw.irq = IRQ_CART;
			/* register hardware addresses */
			ide_register_hw(&hw, &hwif);
			/* modify IO functions */
			hw_ide_insw     = m3cf_ide_insw;
			hw_ide_outsw    = m3cf_ide_outsw;
			hw_ide_outb	= m3cf_ide_outb;
			hwif->OUTB	= m3cf_ide_outb;
			hwif->OUTBSYNC	= nds_ide_outbsync;
			hwif->OUTW	= m3cf_ide_outw;
			hwif->OUTL	= m3cf_ide_outl;
			hwif->OUTSW	= nds_ide_outsw;
			hwif->OUTSL	= nds_ide_outsl;
			hwif->INB	= m3cf_ide_inb;
			hwif->INW	= m3cf_ide_inw;
			hwif->INL	= m3cf_ide_inl;
			hwif->INSW	= nds_ide_insw;
			hwif->INSL	= nds_ide_insl;
			/* all OK */
			return;
		}
#endif

#ifndef CONFIG_NDS_ROM8BIT

#ifdef CONFIG_IDE_NDS_M3
		M3_Unlock();
		for (i=0; i<8; i++)
			hw.io_ports[i] = 0x08800000 + 0x20000*i;
		hw.io_ports[8] = 0x080C0000; // control
#endif
#ifdef CONFIG_IDE_NDS_MAX_MEDIA_PLAYER
		for (i=0; i<8; i++)
			hw.io_ports[i] = 0x08000000 + 0x20000*i;
		hw.io_ports[8] = 0x080E0000; // control
#endif
#if defined(CONFIG_IDE_NDS_SUPERCARD) || defined(CONFIG_IDE_NDS_GBAMP)
		for (i=0; i<8; i++)
			hw.io_ports[i] = 0x09000000 + 0x20000*i;
		hw.io_ports[8] = 0x098C0000; // control
#endif
		hw.irq = IRQ_CART;
		ide_register_hw(&hw, NULL);
#endif
	}
}
