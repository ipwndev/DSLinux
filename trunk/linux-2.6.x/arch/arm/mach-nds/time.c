/*
 *  linux/arch/arm/mach-nds/time.c
 *
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

/*
 *  *  Timer registers of the NDS.
 *   */
#define NDS_TIMER0_DATA 0x04000100      /* Timer 0 data reg */
#define NDS_TIMER1_DATA 0x04000104      /* Timer 1 data reg */
#define NDS_TIMER2_DATA 0x04000108      /* Timer 2 data reg */
#define NDS_TIMER3_DATA 0x0400010c      /* Timer 3 data reg */

#define NDS_TIMER0_CR   0x04000102      /* Timer 0 control reg */
#define NDS_TIMER1_CR   0x04000106      /* Timer 1 control reg */
#define NDS_TIMER2_CR   0x0400010a      /* Timer 2 control reg */
#define NDS_TIMER3_CR   0x0400010e      /* Timer 3 control reg */


/*
 *  *  Timer control register flags.
 *   */
#define NDS_TCR_CLK 0x0000          /* Use clock freq */
#define NDS_TCR_CLK64   0x0001          /* Use clock/64 freq */
#define NDS_TCR_CLK256  0x0002          /* Use clock/256 freq */
#define NDS_TCR_CLK1024 0x0003          /* Use clock/1024 freq */
#define NDS_TCR_CASCADE 0x0004          /* Cascade timer */
#define NDS_TCR_IRQ 0x0040          /* Generate IRQ */
#define NDS_TCR_ENB 0x0080          /* Enable timer */

/* use timer 0 for ticks */
#define KERNEL_TIMER 0

#include <asm/mach/time.h>

static irqreturn_t nds_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	timer_tick(regs);
	return IRQ_HANDLED;
}

static struct irqaction nds_timer_irq = {
	.name    = "NDS Timer Tick",
	.flags   = 0,	/* using low priority for all standard timer tasks */
	.handler = nds_timer_interrupt
};
static unsigned long setup_timer (void)
{
	volatile unsigned short	*tcp;

	tcp = (volatile unsigned short *) NDS_TIMER0_DATA;
	*tcp = (-0x02000000)/(1024*HZ);
	tcp = (volatile unsigned short *) NDS_TIMER0_CR;
	*tcp = (NDS_TCR_CLK1024 | NDS_TCR_ENB | NDS_TCR_IRQ);

	return mktime(2000, 1, 1, 0, 0, 0);
}

void nds_time_init(void)
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

	setup_irq(KERNEL_TIMER_IRQ_NUM, &nds_timer_irq);
	xtime.tv_sec = setup_timer();
}

/* Used in arch.c - ignore 'defined but not used' warning! (stsp) */
struct sys_timer nds_timer = {
	.init = nds_time_init,
};
