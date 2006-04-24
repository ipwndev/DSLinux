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

#include <asm/arch/fifo.h>

#define SAMPLERATE 	44100
#define BUFSIZE 	(SAMPLERATE)  /* 1 second */

static short int soundbuffer[BUFSIZE];

static void soundtest_initsound(void)
{
	/* Enable Power */
	REG_IPCFIFOSEND	= FIFO_SOUND | FIFO_SOUND_POWER | 1;
	/* Wait some time to power up */
	msleep(15);

	/* First: set the number of channels. First channel is left. */
	REG_IPCFIFOSEND = FIFO_SOUND | FIFO_SOUND_CHANNELS | 1;

 	/* Format is a simple set operation 1 = 16 bit LE */ 
	REG_IPCFIFOSEND = FIFO_SOUND | FIFO_SOUND_FORMAT | 1;

	/* Next is buffer size (in bytes). Must be before ADDRESS */ 
	REG_IPCFIFOSEND = FIFO_SOUND | FIFO_SOUND_DMA_SIZE | sizeof(soundbuffer);

	/* now set the address of the buffer */
	REG_IPCFIFOSEND = FIFO_SOUND | FIFO_SOUND_DMA_ADDRESS |
	    (((u32) soundbuffer) & 0xffffff);

	REG_IPCFIFOSEND = FIFO_SOUND | FIFO_SOUND_RATE | SAMPLERATE;
}

/* 1 = start, 0 = stop */
static void soundtest_play(int start)
{
	REG_IPCFIFOSEND = FIFO_SOUND | FIFO_SOUND_TRIGGER | start;
}

static void soundtest_fill(void)
{
	int i;
	int data = 0;
	int freq = 440;
	int samples_per_period = SAMPLERATE / freq;
	int incr = 0x20000 / samples_per_period;

	/* Fill the buffer with data */
	for (i = 0; i < BUFSIZE; i++){
		soundbuffer[i] = (short int)data;
		if (incr > 0) {
			if ((data + incr) > 32767) {
				incr = 0 - incr;
			}
		} else {
			if ((data + incr) < -32768) {
				incr = 0 - incr;
			}
		}	
		data += incr;
	}
}

static int __init soundtest_init(void)
{
	soundtest_initsound();
	soundtest_fill();
	soundtest_play(1);
	return 0;
}

static void __exit soundtest_exit(void)
{
}

module_init(soundtest_init);
module_exit(soundtest_exit);
