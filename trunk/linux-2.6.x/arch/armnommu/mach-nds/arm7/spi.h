#ifndef SPI_H
#define SPI_H
u16 touch_read(u32 command);
s32 touch_read_value(int measure, int retry , int range);
u8 power_read(void);
void power_write(u8 val);
void read_firmware(u32 address, u8 * destination, int count);
#endif
