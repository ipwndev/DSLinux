/*
 * include/asm-armnommu/arch-nds/fifo.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __ASM_ARM_ARCH_FIFO_H
#define __ASM_ARM_ARCH_FIFO_H

#define FIFO_BUTTONS  (1<<28)
#define FIFO_TOUCH    (2<<28)
#define FIFO_MIC      (3<<28)
#define FIFO_WIFI     (4<<28)
#define FIFO_SOUND    (5<<28)
#define FIFO_POWER    (6<<28)
#define FIFO_TIME     (7<<28)

#define FIFO_HIGH_BITS  (1<<16)
#define FIFO_LOW_BITS   (1<<17)

#define FIFO_SOUND_CHANNELS	(1<<24)
#define FIFO_SOUND_DMA_ADDRESS	(2<<24)
#define FIFO_SOUND_DMA_SIZE	(3<<24)
#define FIFO_SOUND_FORMAT	(4<<24)
#define FIFO_SOUND_RATE		(5<<24)
#define FIFO_SOUND_TRIGGER	(6<<24)

#define REG_IPCFIFOSEND (*(volatile u32*) 0x04000188)
#define REG_IPCFIFORECV (*(volatile u32*) 0x04100000)
#define REG_IPCFIFOCNT  (*(volatile u16*) 0x04000184)

struct fifo_cb
{
	u32 type ;
	union
	{
		void (*button_handler)( u32 state ) ;
		void (*touch_handler)( u8 pressed, u8 x, u8 y ) ;
		void (*time_handler)( u32 seconds ) ;
		/* ... */
	} handler ;
	struct fifo_cb *next ;
};

int register_fifocb( struct fifo_cb *fifo_cb ) ;
int unregister_fifocb( struct fifo_cb *fifo_cb ) ;

#endif /* __ASM_ARM_ARCH_FIFO_H */
