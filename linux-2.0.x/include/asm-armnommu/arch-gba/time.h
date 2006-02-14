/****************************************************************************/

/*
 *	linux/include/asm-armnommu/arch-gba/time.h
 *	
 *	(C) Copyright 2001, Greg Ungerer (gerg@snapgear.com)
 */

/****************************************************************************/
#ifndef __ASM_ARCH_TIME_H
#define __ASM_ARCH_TIME_H
/****************************************************************************/

/*
 *	Timer registers of the GBA.
 */
#define	GBA_TIMER0_DATA	0x04000100		/* Timer 0 data reg */
#define	GBA_TIMER1_DATA	0x04000104		/* Timer 1 data reg */
#define	GBA_TIMER2_DATA	0x04000108		/* Timer 2 data reg */
#define	GBA_TIMER3_DATA	0x0400010c		/* Timer 3 data reg */

#define	GBA_TIMER0_CR	0x04000102		/* Timer 0 control reg */
#define	GBA_TIMER1_CR	0x04000106		/* Timer 1 control reg */
#define	GBA_TIMER2_CR	0x0400010a		/* Timer 2 control reg */
#define	GBA_TIMER3_CR	0x0400010e		/* Timer 3 control reg */


/*
 *	Timer control register flags.
 */
#define	GBA_TCR_CLK	0x0000			/* Use clock freq */
#define	GBA_TCR_CLK64	0x0001			/* Use clock/64 freq */
#define	GBA_TCR_CLK256	0x0002			/* Use clock/256 freq */
#define	GBA_TCR_CLK1024	0x0003			/* Use clock/1024 freq */
#define	GBA_TCR_CASCADE	0x0008			/* Cascade timer */
#define	GBA_TCR_IRQ	0x0040			/* Generate IRQ */
#define	GBA_TCR_ENB	0x0080			/* Enable timer */

/****************************************************************************/

/*
 *	Define the timer that we use as main Hz source.
 */
#define	KERNEL_TIMER_IRQ_NUM	1


extern __inline__ unsigned long gettimeoffset (void)
{
	/* FIXME: not right for the GBA */
	return(0);
}

/*
 * No need to reset the timer at every irq
 */
extern __inline__ int reset_timer(void)
{
	return (1);
}

/*
 *	No RTC to update.
 */
#define update_rtc() {}

/****************************************************************************/
#endif	/* __ASM_ARCH_TIME_H */
