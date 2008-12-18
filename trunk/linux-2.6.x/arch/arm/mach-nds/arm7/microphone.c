#define __DSLINUX_ARM7__
#include "asm/types.h"
#include "spi.h"
#include "microphone.h"
#include "arm7.h"

//same as SOUND_FREQ(n)
#define MIC_FREQ(n)		(0 - (0x1000000 / (n)))

static u8 *mic_buffer  = 0;  //pointer to the buffer 
static u32 buffer_size = 0;  //size of the buffer
static u32 current_ptr = 0;  //current pointer in the buffer
static u32 mic_rate = 0;  //sample rate.

/********************
 *mic_on()
 ********************/
void mic_on()
{
	mic_on_off(1);
}

/********************
 *mic_off()
 ********************/
void mic_off()
{
	mic_on_off(0);
}

/********************
 *mic_set_address()
 ********************/
void mic_set_address(u32 buffer)
{
	mic_buffer = (u8*)buffer;
}

/********************
 *mic_set_size()
 ********************/
void mic_set_size(u32 size)
{
	buffer_size = size;
}

/********************
 *mic_set_rate()
 ********************/
void mic_set_rate(u32 rate)
{
	mic_rate = rate; 
}

/********************
 *mic_start()
 ********************/
void mic_start()
{
	// Setup timer3, since timer0 is used for wifi, 
	// and timers 1 and 2 are for sound.
	REG_TM3CNT_L = MIC_FREQ(mic_rate);
	REG_TM3CNT_H = NDS_TCR_ENB | NDS_TCR_CLK | NDS_TCR_IRQ;
	NDS_IE |= IRQ_TIMER3;
}

/*********************
 *mic_stop
 *********************/
int mic_stop(void)
{
	//turn off the timer.
	REG_TM3CNT_H &= ~NDS_TCR_ENB;
	//disable the timer irq.
	NDS_IE &= ~IRQ_TIMER3;
	mic_buffer = 0;
	return current_ptr;
}

/********************
 *mic_timer_handler()
 ********************/
void mic_timer_handler(void)
{
	mic_buffer[current_ptr] = mic_read8() /*^ 0x80*/;
	current_ptr++; 
	if (current_ptr >= buffer_size)
	{
		//buffer wraps around.
		current_ptr = 0;
	}
}
