/*
 * NDS soundtest driver
 *
 * Copyright (c) 2006 Amadeus
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h> 
#include <linux/delay.h>
#include <linux/interrupt.h>

#include <asm/arch/fifo.h>
#include <asm/cacheflush.h>

#define TIMER1_DATA	(*(volatile u16*)0x04000104)
#define TIMER2_DATA	(*(volatile u16*)0x04000108)
#define TIMER1_CR	(*(volatile u16*)0x04000106)
#define TIMER2_CR	(*(volatile u16*)0x0400010A)
#define TIMER_ENABLE	(1<<7)
#define TIMER_IRQ_REQ	(1<<6)
#define TIMER_CASCADE	(TIMER_ENABLE|(1<<2))

#define SAMPLERATE 	80000

#define SINSIZE 32

static const short sintab[SINSIZE] = {
0, 6393, 12539, 18204, 23170, 27245, 30273, 32137, 32767, 32137,
30273, 27245, 23170, 18204, 12539, 6393, 0, -6393, -12539, -18204,
-23170, -27245, -30273, -32137, -32767, -32137, -30273, -27245, -23170, -18204,
-12539, -6393
};

#define BUFSIZE 64
#define INTSIZE 500

static short soundbuffer[BUFSIZE];

struct memdescr {
	volatile short *start;
	int size;
	int index;
	int sinindex;
};

static struct memdescr snddescr_left;
static struct memdescr snddescr_right;

static void soundtest_initsound(void)
{
	TIMER1_CR = 0;
	TIMER2_CR = 0;

	/* Enable Power */
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_POWER | 1);
	/* Wait some time to power up */
	msleep(15);

	/* First: set the number of channels. First channel is left. */
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_CHANNELS | 2);

 	/* Format is a simple set operation 1 = 16 bit LE */ 
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_FORMAT | 1);

	/* Next is buffer size (in bytes). Must be before ADDRESS */ 
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_DMA_SIZE | sizeof(soundbuffer));

	/* now set the address of the buffer */
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_DMA_ADDRESS |
	    (((u32) soundbuffer) & 0xffffff));

	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_RATE | SAMPLERATE);

	TIMER1_DATA = 0 - ((0x1000000 / SAMPLERATE)*2);
	TIMER2_DATA = 0 - INTSIZE;
}

/* 1 = start, 0 = stop */
static void soundtest_play(int start)
{
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_TRIGGER | start);

	/* send a dummy command */
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_CHANNELS | 2);

	/* wait until FIFO is empty */
	while (!(NDS_REG_IPCFIFOCNT & FIFO_EMPTY))
		;

	/* now we can start the timer and be shure that
           the timer interrupt is not comming to early. */
//	TIMER1_CR = TIMER_ENABLE;
//	TIMER2_CR = TIMER_CASCADE | TIMER_IRQ_REQ;
}

static void soundtest_write(struct memdescr *descr, int samples, int step)
{
	for (; samples; samples--) {
		descr->start[descr->index++] = sintab[descr->sinindex];
		if (descr->index >= descr->size)
			descr->index = 0;
		descr->sinindex += step;
		if (descr->sinindex >= SINSIZE)
			descr->sinindex -= SINSIZE;
 	}
}

#if 0
static irqreturn_t soundtest_interrupt(int irq, void *dev_id,
				     struct pt_regs *regs)
{
	soundtest_write( &snddescr_left, INTSIZE, 1);
	soundtest_write( &snddescr_right, INTSIZE, 2);

	return IRQ_HANDLED;
}
#endif

static void soundtest_fill(void)
{
	snddescr_left.start = &soundbuffer[0];
	snddescr_left.size  = BUFSIZE/2;
	snddescr_left.index = 0;
	snddescr_left.sinindex = 0;

	snddescr_right.start = &soundbuffer[BUFSIZE/2];
	snddescr_right.size  = BUFSIZE/2;
	snddescr_right.index = 0;
	snddescr_right.sinindex = 0;

	soundtest_write( &snddescr_left, BUFSIZE/2, 1);
	soundtest_write( &snddescr_right, BUFSIZE/2, 1);
	dmac_clean_range((unsigned long)&soundbuffer[0],(unsigned long)&soundbuffer[BUFSIZE]);
}

void debug_pin(int state)
{
	short pattern = state ? 0x7FFF : 0;
	short *p = soundbuffer;
	int i = BUFSIZE/2;
	for ( ; i; i--)
		*p++ = pattern;
	dmac_clean_range((unsigned long)&soundbuffer[0],(unsigned long)&soundbuffer[BUFSIZE]);
}

static int __init soundtest_init(void)
{
	soundtest_initsound();
	soundtest_fill();

#if 0
	/* use a high priority timer */
	if (request_irq(IRQ_TC2, soundtest_interrupt, SA_INTERRUPT, "NDS sound", 0)) {
		printk(KERN_ERR "cannot grab irq %d\n", IRQ_TC2);
		return -EBUSY;
	}
#endif

	soundtest_play(1);
	return 0;
}

static void __exit soundtest_exit(void)
{
}

module_init(soundtest_init);
module_exit(soundtest_exit);

