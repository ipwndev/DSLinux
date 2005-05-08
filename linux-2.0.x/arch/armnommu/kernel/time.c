/*
 *  linux/arch/arm/kernel/time.c
 *
 *  Copyright (C) 1991, 1992, 1995  Linus Torvalds
 *  Modifications for ARM (C) 1994, 1995, 1996,1997 Russell King
 *
 * This file contains the ARM-specific time handling details:
 * reading the RTC at bootup, etc...
 *
 * 1994-07-02  Alan Modra
 *             fixed set_rtc_mmss, fixed time.year for >= 2000, new mktime
 * 1997-09-10  Updated NTP code according to technical memorandum Jan '96
 *             "A Kernel Model for Precision Timekeeping" by Dave Mills
 */
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/interrupt.h>

#include <asm/segment.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <linux/timex.h>
#include <asm/hardware.h>

#ifndef BCD_TO_BIN
#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)
#endif

#ifndef BIN_TO_BCD
#define BIN_TO_BCD(val) ((val)=(((val)/10)<<4) + (val)%10)
#endif

/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines were long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */
static inline unsigned long mktime(unsigned int year, unsigned int mon,
	unsigned int day, unsigned int hour,
	unsigned int min, unsigned int sec)
{
	if (0 >= (int) (mon -= 2)) {	/* 1..12 -> 11,12,1..10 */
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}
	return (((
		    (unsigned long)(year/4 - year/100 + year/400 + 367*mon/12 + day) +
		      year*365 - 719499
		    )*24 + hour /* now have hours */
		   )*60 + min /* now have minutes */
		  )*60 + sec; /* finally seconds */
}

#include <asm/arch/time.h>

static unsigned long do_gettimeoffset(void)
{
	
	return gettimeoffset ();
}

void do_gettimeofday(struct timeval *tv)
{
	unsigned long flags;

	save_flags_cli (flags);
	*tv = xtime;
	tv->tv_usec += do_gettimeoffset();
	if (tv->tv_usec >= 1000000) {
		tv->tv_usec -= 1000000;
		tv->tv_sec++;
	}
	restore_flags(flags);
}

void do_settimeofday(struct timeval *tv)
{
	cli ();
	/* This is revolting. We need to set the xtime.tv_usec
	 * correctly. However, the value in this location is
	 * is value at the last tick.
	 * Discover what correction gettimeofday
	 * would have done, and then undo it!
	 */
	tv->tv_usec -= do_gettimeoffset();

	if (tv->tv_usec < 0) {
		tv->tv_usec += 1000000;
		tv->tv_sec--;
	}

	xtime = *tv;
	time_adjust = 0;		/* stop active adjtime() */
	time_status |= STA_UNSYNC;
	time_state = TIME_ERROR;	/* p. 24, (a) */
	time_maxerror = NTP_PHASE_LIMIT;
	time_esterror = NTP_PHASE_LIMIT;
	sti ();
}

/*
 * timer_interrupt() needs to keep up the real-time clock,
 * as well as call the "do_timer()" routine every clocktick.
 */
static void timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
        if (reset_timer ()) {
		do_timer(regs);
	}

	update_rtc ();
}

static struct irqaction irq_kernel_timer = { timer_interrupt, 0, 0, "timer", NULL, NULL};

extern int setup_arm_irq(int, struct irqaction *);

#ifdef CONFIG_ARCH_ATMEL
unsigned long setup_timer (void)
{
	volatile struct atmel_timers* tt = (struct atmel_timers*) (TIMER_BASE);
	volatile struct atmel_timer_channel* tc = &tt->chans[KERNEL_TIMER].ch;
	unsigned long v;

	/* No SYNC */
	tt->bcr = 0;  

#if (KERNEL_TIMER==0)
	/* program no external (=internal) clock generation signal on XC0 */
	v = tt->bmr;
	v = (v & ~(3 << 0)) | (1 << 0);
	tt->bmr = v;
#ifdef CONFIG_ARCH_ATMEL_EB55
	APMC_BASE->APMC_PCER = (1 << TC0_ID) ;
#endif
#elif (KERNEL_TIMER==1)
	/* program no external (=internal) clock generation signal on XC1 */
	v = tt->bmr;
	v = (v & ~(3 << 2)) | (1 << 2);
	tt->bmr = v;
#ifdef CONFIG_ARCH_ATMEL_EB55
	APMC_BASE->APMC_PCER = (1 << TC1_ID) ;
#endif
#elif (KERNEL_TIMER==2)
	/* program no external (=internal) clock generation signal on XC2 */
	v = tt->bmr;
	v = (v & ~(3 << 4)) | (1 << 4);
	tt->bmr = v;
#ifdef CONFIG_ARCH_ATMEL_EB55
	APMC_BASE->APMC_PCER = (1 << TC2_ID) ;
#endif
#else
#error Weird -- KERNEL_TIMER does not seem to be defined...
#endif
	tc->ccr = 2;  /* disable the channel */

	/* select ACLK/128 as inupt frequensy for TC1 and enable CPCTRG */
	tc->cmr = 3 | (1 << 14);

	tc->idr = ~0ul;  /* disable all interrupt */
	tc->rc = ((ARM_CLK/128)/HZ - 1);   /* load the count limit into the CR register */
	tc->ier = TC_CPCS;  /* enable CPCS interrupt */

	/* enable the channel */
	tc->ccr = TC_SWTRG|TC_CLKEN;

	return mktime(2000, 1, 1, 0, 0, 0);
}
#endif

#if defined(CONFIG_ARCH_GBA)
unsigned long setup_timer (void)
{
	volatile unsigned short	*tcp;

	/*
	 *	GBA timers are not very programmable, so cascade 2
	 *	timers to generate a 64 Hz clock.
	 */
	tcp = (volatile unsigned short *) GBA_TIMER0_CR;
	*tcp = (GBA_TCR_CLK1024 | GBA_TCR_ENB);
	tcp = (volatile unsigned short *) GBA_TIMER1_CR;
	*tcp = (GBA_TCR_CLK256 | GBA_TCR_CASCADE | GBA_TCR_ENB |GBA_TCR_IRQ);

#if 1
	/*
	 *	FIXME: can't get the timers to work (at least with the
	 *	boycott/advance emulator. VBLANK interrupt does seem to
	 *	work though, so using that for now.
	 */
{
	*((unsigned short *) 0x04000004) = 0x0008;
	setup_arm_irq(IRQ_VBLANK, &irq_kernel_timer);
}
#endif
#if 0
{
	volatile unsigned short *td0, *td1;
	td0 = (volatile unsigned short *) GBA_TIMER0_DATA;
	td1 = (volatile unsigned short *) GBA_TIMER1_DATA;
	/*
	 *	DEBUG: test to see if the timer is working...
	 */
	for (;;)
		printk("TIMER0=%04x  TIMER1=%04x\n", *td0, *td1);
}
#endif
}
#endif /* CONFIG_ARCH_BGA */

void time_init(void)
{
#if (KERNEL_TIMER==0)
#   define KERNEL_TIMER_IRQ_NUM	IRQ_TC0
#elif (KERNEL_TIMER==1)
#   define KERNEL_TIMER_IRQ_NUM	IRQ_TC1
#elif (KERNEL_TIMER==2)
#   define KERNEL_TIMER_IRQ_NUM	IRQ_TC2
#else
#error Wierd -- KERNEL_TIMER is not defined or something....
#endif

	setup_arm_irq(KERNEL_TIMER_IRQ_NUM, &irq_kernel_timer);
	xtime.tv_sec = setup_timer();
	xtime.tv_usec = 0;
}
