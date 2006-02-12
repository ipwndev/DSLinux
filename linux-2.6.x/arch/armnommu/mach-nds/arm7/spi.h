#ifndef SPI_H
#define SPI_H
u16 touchRead(u32 command);
s32 readTouchValue(int measure, int retry , int range);
u8 power_read(void);
void power_write(u8 val);
void read_firmware(u32 address, u8 * destination, int count);
#endif
