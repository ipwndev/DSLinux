
#define __DSLINUX_ARM7__

#include "asm/types.h"
#include "asm/arch/fifo.h"
#include "spi.h"

extern void swiDelay(u32 duration);

#define REG_SPI_CR      (*(volatile u16*)0x040001C0)
#define REG_SPI_DATA    (*(volatile u16*)0x040001C2)

#define SPI_4MHZ	(0<<0)
#define SPI_2MHZ	(1<<0)
#define SPI_1MHZ	(2<<0)
#define SPI_512KHZ	(3<<0)
#define SPI_BUSY	(1<<7)
#define SPI_POWER	(0<<8)
#define SPI_FIRMWARE	(1<<8)
#define SPI_TOUCH	(2<<8)
#define SPI_MIC	(2<<8)
#define SPI_8CLOCKS	(0<<10)
#define SPI_16CLOCKS	(1<<10)
#define SPI_SINGLE	(0<<11)
#define SPI_CONTINUOUS	(1<<11)
#define SPI_INTERRUPT	(1<<14)
#define SPI_ENABLE	(1<<15)

#define POWER_READ	(1<<7)
#define POWER_WRITE	(0<<7)
#define POWER_REG(n)	(n)

#define MIC_ON	(1)
#define MIC_OFF	(0)

#define MIC_POWER_OFFSET	(2)


#define WAIT_FOR_NOT_BUSY() {while (REG_SPI_CR & SPI_BUSY) swiDelay(1);}

static u16 touch_read(u32 command)
{
	u16 result;

	WAIT_FOR_NOT_BUSY();

	// Write the command and wait for it to complete
	REG_SPI_CR = SPI_ENABLE | SPI_CONTINUOUS | SPI_TOUCH | SPI_8CLOCKS | SPI_2MHZ ;
	REG_SPI_DATA = command;
	WAIT_FOR_NOT_BUSY();

	// Write the second command and clock in part of the data
	REG_SPI_DATA = 0;
	WAIT_FOR_NOT_BUSY();
	result = REG_SPI_DATA;

	// Clock in the rest of the data (last transfer)
	REG_SPI_CR = SPI_ENABLE | SPI_SINGLE | SPI_TOUCH | SPI_8CLOCKS | SPI_2MHZ ;
	REG_SPI_DATA = 0;
	WAIT_FOR_NOT_BUSY();

	// Return the result
	result = ((result & 0x7F) << 5) | (REG_SPI_DATA >> 3);

	REG_SPI_CR = 0;

	return result ;
}

// Code from devkitpro/libnds/source/arm7/touch.c
//---------------------------------------------------------------------------------
s32 touch_read_value(int measure, int retry , int range) {
//---------------------------------------------------------------------------------
	int i;
	s32 this_value=0, this_range;

	s32 last_value = touch_read(measure | 1);

	for ( i=0; i < retry; i++) {
		this_value = touch_read(measure | 1);
		if ((last_value - this_value) > 0)
			this_range = last_value - this_value;
		else
			this_range = this_value - last_value;
		if (this_range <= range) break;
	}

	if ( i == range) this_value = 0;
	return this_value;
}

u8 power_read(enum power_reg reg)
{
	u8 val;

	WAIT_FOR_NOT_BUSY();

	REG_SPI_CR = SPI_ENABLE | SPI_CONTINUOUS | SPI_POWER | SPI_8CLOCKS | SPI_1MHZ ;
	REG_SPI_DATA = POWER_READ | reg ;

	WAIT_FOR_NOT_BUSY();

	REG_SPI_CR = SPI_ENABLE | SPI_SINGLE | SPI_POWER | SPI_8CLOCKS | SPI_1MHZ ;
	REG_SPI_DATA = 0x0;

	WAIT_FOR_NOT_BUSY();

	val = REG_SPI_DATA ;

	REG_SPI_CR = 0 ;

	return val;
}

void power_write( enum power_reg reg, u8 val )
{
	WAIT_FOR_NOT_BUSY();

	// Write the command and wait for it to complete
	REG_SPI_CR = SPI_ENABLE | SPI_CONTINUOUS | SPI_POWER | SPI_8CLOCKS | SPI_1MHZ ;
	REG_SPI_DATA = POWER_WRITE | reg ;

	WAIT_FOR_NOT_BUSY();

	// Write the data
	REG_SPI_CR = SPI_ENABLE | SPI_SINGLE | SPI_POWER | SPI_8CLOCKS | SPI_1MHZ ;
	REG_SPI_DATA = val;

	REG_SPI_CR = 0 ;
}

void read_firmware(u32 address, u8 * destination, int count)
{
	int i;

	WAIT_FOR_NOT_BUSY();

	REG_SPI_CR = SPI_ENABLE | SPI_CONTINUOUS | SPI_FIRMWARE | SPI_8CLOCKS | SPI_4MHZ ;
	REG_SPI_DATA = 3;
	WAIT_FOR_NOT_BUSY();

	REG_SPI_DATA = (address >> 16) & 255;
	WAIT_FOR_NOT_BUSY();

	REG_SPI_DATA = (address >> 8) & 255;
	WAIT_FOR_NOT_BUSY();

	REG_SPI_DATA = (address) & 255;
	WAIT_FOR_NOT_BUSY();

	for (i = 0; i < count; i++) {
		REG_SPI_DATA = 0;
		WAIT_FOR_NOT_BUSY();

		destination[i] = REG_SPI_DATA;
	}

	REG_SPI_CR = 0;
}

/*
 * mic_on_off() : Turns on/off the Microphone Amp. 
 * Code based on neimod's example.
 */
void mic_on_off(u8 control) {
	WAIT_FOR_NOT_BUSY();

	REG_SPI_CR = SPI_ENABLE | SPI_POWER | SPI_1MHZ | SPI_CONTINUOUS;
	REG_SPI_DATA = MIC_POWER_OFFSET;

	WAIT_FOR_NOT_BUSY();

	REG_SPI_CR = SPI_ENABLE | SPI_POWER | SPI_1MHZ;
	REG_SPI_DATA = control;
}

/*
 * mic_read8(): Reads a byte from the microphone
 * Code based on neimod's example. 
 */
u8 mic_read8() {
	u16 result, result2;

	WAIT_FOR_NOT_BUSY();

	REG_SPI_CR = SPI_ENABLE | SPI_MIC | SPI_2MHZ | SPI_CONTINUOUS;
	REG_SPI_DATA = 0xEC;  // Touchscreen command format for AUX

	WAIT_FOR_NOT_BUSY();

	REG_SPI_DATA = 0x00;

	WAIT_FOR_NOT_BUSY();

	result = REG_SPI_DATA;
	REG_SPI_CR = SPI_ENABLE | SPI_MIC | SPI_2MHZ;
	REG_SPI_DATA = 0x00;

	WAIT_FOR_NOT_BUSY();

	result2 = REG_SPI_DATA;

	return (((result & 0x7F) << 1) | ((result2>>7)&1));

}


/*
 * mic_read12() : Read a short from the microphone. 
 * Code based on neimod's example.
 */
u16 mic_read12() {

	static u16 result, result2;

	WAIT_FOR_NOT_BUSY();

	REG_SPI_CR = SPI_ENABLE | SPI_MIC | SPI_2MHZ | SPI_CONTINUOUS;
	
	REG_SPI_DATA = 0xE4;  // Touchscreen command format for AUX, 12bit

	WAIT_FOR_NOT_BUSY();

	REG_SPI_DATA = 0x00;

	WAIT_FOR_NOT_BUSY();

	result = REG_SPI_DATA;
	REG_SPI_CR = SPI_ENABLE | SPI_TOUCH | SPI_2MHZ;
	REG_SPI_DATA = 0x00;

	WAIT_FOR_NOT_BUSY();

	result2 = REG_SPI_DATA;

	return (((result & 0x7F) << 5) | ((result2>>3)&0x1F));
}

