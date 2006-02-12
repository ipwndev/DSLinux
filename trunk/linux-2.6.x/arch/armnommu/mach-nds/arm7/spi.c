
#include "asm/types.h"
#include "asm/arch/fifo.h"

extern void swiDelay(u32 duration);
extern void swiWaitForVBlank(void);

#define DISP_SR		(*(volatile u16*)0x04000004)

#define DISP_VBLANK_IRQ	(1 << 3)

#define REG_SPI_CR      (*(volatile u16*)0x040001C0)
#define REG_SPI_DATA    (*(volatile u16*)0x040001C2)

#define REG_SPI_ENABLE   0x8000
#define SPI_BUSY     0x80

u16 touchRead(u32 command)
{
	u16 result;
	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	// Write the command and wait for it to complete
	REG_SPI_CR = REG_SPI_ENABLE | 0xA01;
	REG_SPI_DATA = command;
	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	// Write the second command and clock in part of the data
	REG_SPI_DATA = 0;
	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);
	result = REG_SPI_DATA;

	// Clock in the rest of the data (last transfer)
	REG_SPI_CR = REG_SPI_ENABLE | 0x201;
	REG_SPI_DATA = 0;
	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	// Return the result
	return ((result & 0x7F) << 5) | (REG_SPI_DATA >> 3);
}

// Code from devkitpro/libnds/source/arm7/touch.c
//---------------------------------------------------------------------------------
s32 readTouchValue(int measure, int retry , int range) {
	//---------------------------------------------------------------------------------
	int i;
	s32 this_value=0, this_range;

	s32 last_value = touchRead(measure | 1);

	for ( i=0; i < retry; i++) {
		this_value = touchRead(measure | 1);
		if ((last_value - this_value) > 0)
			this_range = last_value - this_value;
		else
			this_range = this_value - last_value;
		if (this_range <= range) break;
	}

	if ( i == range) this_value = 0;
	return this_value;

}

u8 power_read(void)
{
	u8 val;

	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	REG_SPI_CR = REG_SPI_ENABLE | (1<<1) | (1<<11) ;
	REG_SPI_DATA = 0x80;

	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	REG_SPI_CR = REG_SPI_ENABLE | (1<<1) ;
	REG_SPI_DATA = 0x0;

	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	val = REG_SPI_DATA ;

	REG_SPI_CR = 0 ;

	return val;
}

void power_write( u8 val )
{
	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	// Write the command and wait for it to complete
	REG_SPI_CR = REG_SPI_ENABLE | (1<<1) | (1<<11) ;
	REG_SPI_DATA = 0x00;

	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	// Write the data
	REG_SPI_CR = REG_SPI_ENABLE | (1<<1) ;
	REG_SPI_DATA = val;

	REG_SPI_CR = 0 ;
}

void read_firmware(u32 address, u8 * destination, int count)
{
	int i;

	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	REG_SPI_CR = 0x8900;
	REG_SPI_DATA = 3;
	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	REG_SPI_DATA = (address >> 16) & 255;
	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	REG_SPI_DATA = (address >> 8) & 255;
	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	REG_SPI_DATA = (address) & 255;
	while (REG_SPI_CR & SPI_BUSY)
		swiDelay(1);

	for (i = 0; i < count; i++) {
		REG_SPI_DATA = 0;
		while (REG_SPI_CR & SPI_BUSY)
			swiDelay(1);

		destination[i] = REG_SPI_DATA;
	}
	REG_SPI_CR = 0x0000;
}
