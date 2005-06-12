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

#define FIFO_BUTTONS  (1<<24)
#define FIFO_TOUCH    (1<<25)
#define FIFO_MIC      (1<<26)
#define FIFO_WIFI     (1<<27)
#define FIFO_SOUND    (1<<28)

struct fifo_cb
{
	u32 type ;
	union
	{
		void (*button_handler)( u8 state ) ;
		void (*touch_handler)( u8 pressed, u8 x, u8 y ) ;
		/* ... */
	} handler ;
	struct fifo_cb *next ;
};

int register_fifocb( struct fifo_cb *fifo_cb ) ;

#endif /* __ASM_ARM_ARCH_FIFO_H */
