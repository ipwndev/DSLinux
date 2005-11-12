#ifndef SPI_H
#define SPI_H
u16 touchRead(u32 command);
void poweroff( void );
void read_firmware( u32 address, u16 * destination, int count);
#endif
