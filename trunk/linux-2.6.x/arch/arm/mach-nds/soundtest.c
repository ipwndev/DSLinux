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

#define SAMPLERATE 	44000

#define SINSIZE 100

static const short sintab[SINSIZE] = {
0, 2057, 4107, 6140, 8149, 10126, 12062, 13952, 15786, 17557,
19260, 20886, 22431, 23886, 25247, 26509, 27666, 28714, 29648, 30466,
31163, 31738, 32187, 32509, 32702, 32767, 32702, 32509, 32187, 31738,
31163, 30466, 29648, 28714, 27666, 26509, 25247, 23886, 22431, 20886,
19260, 17557, 15786, 13952, 12062, 10126, 8149, 6140, 4107, 2057,
0, -2057, -4107, -6140, -8149, -10126, -12062, -13952, -15786, -17557,
-19260, -20886, -22431, -23886, -25247, -26509, -27666, -28714, -29648, -30466,
-31163, -31738, -32187, -32509, -32702, -32767, -32702, -32509, -32187, -31738,
-31163, -30466, -29648, -28714, -27666, -26509, -25247, -23886, -22431, -20886,
-19260, -17557, -15786, -13952, -12062, -10126, -8149, -6140, -4107, -2057
};

static short soundbuffer[SINSIZE*2];

static void soundtest_initsound(void)
{
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
}

/* 1 = start, 0 = stop */
static void soundtest_play(int start)
{
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_TRIGGER | start);
}

static void soundtest_fill(void)
{
	int i;

	for (i = 0; i < SINSIZE; i++) {
		soundbuffer[i] = sintab[i];		
	}
	for (i = 0; i < (SINSIZE/2); i++) {
		soundbuffer[i+SINSIZE] = sintab[i*2];		
	}
	for (i = 0; i < (SINSIZE/2); i++) {
		soundbuffer[i+SINSIZE+(SINSIZE/2)] = sintab[i*2];		
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

