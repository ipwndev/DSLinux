#define RTC_CR8            0x04000138
#define READ_DATA_REG1     0x65
#define READ_STATUS_REG1   0x61
#define RTC_DELAY 48
#define CS_0    (1<<6)
#define CS_1    ((1<<6) | (1<<2))
#define SCK_0   (1<<5)
#define SCK_1   ((1<<5) | (1<<1))
#define SIO_0   (1<<4)
#define SIO_1   ((1<<4) | (1<<0))
#define SIO_out (1<<4)
#define SIO_in  (1)

/* rtc stores hours as bcd in bits 0-5, bit 6 is am/pm */
#define RTC_PM (1<<6)
#define HOUR_MASK 0x3f

#include "asm/types.h"
#include "asm/io.h"
#include "linux/timex.h"

#include "time.h"
#include "arm7.h"

static void rtcTransaction(u8 * command, u32 commandLen, u8 * result,
			   u32 resultLen)
{
	u8 bit;
	u8 i;

	writeb(CS_0 | SCK_1 | SIO_1, RTC_CR8);
	swiDelay(RTC_DELAY);
	writeb(CS_1 | SCK_1 | SIO_1, RTC_CR8);
	swiDelay(RTC_DELAY);

	for (i = 0; i < commandLen; i++) {
		for (bit = 0; bit < 8; bit++) {
			writeb(CS_1 | SCK_0 | SIO_out | (command[i] >> 7), RTC_CR8);
			swiDelay(RTC_DELAY);

			writeb(CS_1 | SCK_1 | SIO_out | (command[i] >> 7), RTC_CR8);
			swiDelay(RTC_DELAY);

			command[i] = command[i] << 1;
		}
	}

	for (i = 0; i < resultLen; i++) {
		result[i] = 0;
		for (bit = 0; bit < 8; bit++) {
			writeb(CS_1 | SCK_0, RTC_CR8);
			swiDelay(RTC_DELAY);

			writeb(CS_1 | SCK_1, RTC_CR8);
			swiDelay(RTC_DELAY);

			if (readb(RTC_CR8) & SIO_in)
				result[i] |= (1 << bit);
		}
	}

	writeb(CS_0 | SCK_1, RTC_CR8);
	swiDelay(RTC_DELAY);
}

static u8 BCDToInt(u8 data)
{
	return ((data & 0xF) + ((data & 0xF0) >> 4) * 10);
}

static u32 get_nds_seconds(u8 * time)
{
	u8 hours = 0;
	u8 i;

	hours = BCDToInt(time[4] & HOUR_MASK);

	if ((time[4] & RTC_PM) && (hours < 12)) {
		hours += 12;
	}

	for (i = 0; i < 7; i++) {
		time[i] = BCDToInt(time[i]);
	}

	return (mktime(time[0] + 2000,
		       time[1], time[2], hours, time[5], time[6]));

}

u32 nds_get_time7(void)
{
	u8 command;
	u8 time[8];
	unsigned int seconds;

	command = READ_DATA_REG1;
	rtcTransaction(&command, 1, &(time[1]), 7);

	command = READ_STATUS_REG1;
	rtcTransaction(&command, 1, &(time[0]), 1);

	seconds = get_nds_seconds(&(time[1]));

	return seconds;

}
