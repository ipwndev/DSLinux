#define RTC_CR8         (* (volatile u8*) 0x04000138)
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

#include "asm/types.h"
#include "linux/timex.h"

extern void swiDelay( u32 duration );

static void rtcTransaction(u8 * command, u32 commandLen, u8 * result, u32 resultLen)
{
  u8 bit;
  u8 i;

  RTC_CR8 = CS_0 | SCK_1 | SIO_1;
  swiDelay(RTC_DELAY);
  RTC_CR8 = CS_1 | SCK_1 | SIO_1;
  swiDelay(RTC_DELAY);

  for(i=0; i < commandLen; i++){
    for (bit = 0; bit < 8; bit++) {
      RTC_CR8 = CS_1 | SCK_0 | SIO_out | (command[i]>>7);
      swiDelay(RTC_DELAY);

      RTC_CR8 = CS_1 | SCK_1 | SIO_out | (command[i]>>7);
      swiDelay(RTC_DELAY);

      command[i] = command[i] << 1;
    }
  }

  for(i=0; i < resultLen; i++) {
    result[i] = 0;
    for (bit = 0; bit < 8; bit++) {
      RTC_CR8 = CS_1 | SCK_0;
      swiDelay(RTC_DELAY);

      RTC_CR8 = CS_1 | SCK_1;
      swiDelay(RTC_DELAY);

      if (RTC_CR8 & SIO_in) result[i] |= (1 << bit);
    }
  }

  RTC_CR8 = CS_0 | SCK_1;
  swiDelay(RTC_DELAY);
}


static u8 BCDToInt(u8 data) {
  return((data & 0xF) + ((data & 0xF0)>>4)*10);
}

static u32 get_nds_seconds(u8 * time)
{
  u8 hours = 0;
  u8 i;

  for(i = 0; i < 7; i++) {
      time[i] = BCDToInt(time[i]);
  }

  hours = time[4];
  if (hours >= 52) {
      hours -= 52;
  }

  return( mktime(time[0]+2000,
                 time[1],
                 time[2],
                 hours,
                 time[5],
                 time[6]));

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



