/*
 *  linux/include/asm/arch/gbaram.h
 *
 *  Switching and detection functions for the GBA IO/ROM/RAM space.

 *  Copyright (C) 2007 Amadeus
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

#ifdef CONFIG_NDS_ROM8BIT

/* Set GBA area to RAM mode (if possible) */
extern void (*gba_set_ram)(void);

/* Set GBA area to IO mode (if possible) */
extern void (*gba_set_io) (void);

/* Start of the GBA RAM area (or 0) */
extern u32 gba_start;

/* Length in Bytes of the GBA RAM area */
extern u32 gba_length;

/* Try to activate GBA RAM and set the variables above */
extern int gba_activate_ram(void);


#endif
