#define __DSLINUX_ARM7__
#include "asm/types.h"
#include "spi.h"
#include "microphone.h"
#include "arm7.h"
#include "asm/arch/fifo.h"

//same as SOUND_FREQ(n)
#define MIC_FREQ(n)		(0 - (0x1000000 / (n)))

static u8 *mic_buffer8  = 0;  //pointer to the buffer (8-bit) 
static u16 *mic_buffer16 = 0; //pointer to the buffer (16-bit)
static u32 buffer_size = 0;  //size of the buffer
static u32 current_ptr = 0;  //current pointer in the buffer
static u32 mic_rate = 0;  //sample rate.
static u8 mic_format = 0;
static u32 period_size = 0; //number of bytes to read before signaling arm9

/********************
 *mic_on()
 ********************/
void mic_on()
{
	mic_on_off(1); // or power_write(POWER_MIC_CONTROL, 1);
}

/********************
 *mic_off()
 ********************/
void mic_off()
{
	mic_on_off(0); // or power_write(POWER_MIC_CONTROL, 0);
}

/********************
 *mic_set_power()
 ********************/
void mic_set_power(u8 power) {
	if (power == 0) 
	{
		mic_off();
		power_write(POWER_MIC_GAIN, 0);
	}
	else 
	{
		mic_on();
		power_write(POWER_MIC_GAIN, power - 1);
	}
}

/********************
 *mic_set_address()
 ********************/
void mic_set_address(u32 buffer)
{
	mic_buffer8 = (u8*)buffer;
	mic_buffer16 = (u16*)buffer;
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
 *mic_set_format()
 ********************/
void mic_set_format(u8 format) 
{
	mic_format = format;
}

/********************
 *mic_set_period_size()
 ********************/
void mic_set_period_size(u32 size)
{
	period_size = size;
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
	mic_buffer8 = 0;
	mic_buffer16 = 0;
	return current_ptr;
}

/********************
 *mic_timer_handler()
 ********************/
void mic_timer_handler(void)
{
	if (mic_format & MIC_FORMAT_8BIT)
	{
		if (mic_buffer8) 
		{
			mic_buffer8[current_ptr] = mic_read8();
			if (mic_format & MIC_FORMAT_SIGNED)
				mic_buffer8[current_ptr] = mic_buffer8[current_ptr] ^ 0x80;
			current_ptr++;
		}
	}
	else 
	{
		if (mic_buffer16)
		{
			mic_buffer16[current_ptr/2] = mic_read12();
			if (mic_format & MIC_FORMAT_SIGNED)
				mic_buffer16[current_ptr/2] = (mic_buffer16[current_ptr/2] - 2048) << 4;
			current_ptr+=2;
		}
	}
	if ((period_size > 0) && (current_ptr % period_size == 0))
	{
		if (mic_format & MIC_FORMAT_8BIT)
			nds_fifo_send(FIFO_MIC | FIFO_MIC_HAVEDATA /*| mic_buffer8[current_ptr]*/);
		else
			nds_fifo_send(FIFO_MIC | FIFO_MIC_HAVEDATA /*| mic_buffer16[current_ptr/2]*/);
	}
	if (current_ptr >= buffer_size)
	{
		//buffer wraps around.
		current_ptr = 0;
	}
}
