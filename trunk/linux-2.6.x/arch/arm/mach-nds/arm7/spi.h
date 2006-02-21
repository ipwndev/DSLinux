#ifndef SPI_H
#define SPI_H

s32 touch_read_value(int measure, int retry , int range);

#define POWER0_SOUND_AMP	(1<<0)
#define POWER0_LOWER_BACKLIGHT	(1<<2)
#define POWER0_UPPER_BACKLIGHT	(1<<3)
#define POWER0_LED_BLINK	(1<<4)
#define POWER0_LED_FAST		(1<<5)
#define POWER0_SYSTEM_POWER	(1<<6)

enum power_reg {
	POWER_CONTROL,
	POWER_BATTERY,
	POWER_MIC_CONTROL,
	POWER_MIC_GAIN
};

u8 power_read(enum power_reg);
void power_write(enum power_reg, u8 val);

void read_firmware(u32 address, u8 * destination, int count);

#endif
