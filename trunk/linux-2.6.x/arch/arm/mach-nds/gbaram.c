/*
 *  linux/arch/arm/mach-nds/gbaram.c
 *
 *  Switching and detection functions for the GBA IO/ROM/RAM space.

 *  Copyright (C) 2007 Amadeus, Rick "Lick" Wong,
 *		       Chishm, Cory1492, Lazy1, Pepsiman and Viruseb 
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

#include <linux/config.h>
#include <linux/types.h>
#include <asm/arch/gbaram.h>
#include <asm/io.h>
#include <linux/kernel.h>	/* printk() */

//==========================================================================
void (*gba_set_ram)(void);
void (*gba_set_io) (void);
u32 gba_start;
u32 gba_length;

static void gba_dummy_set(void)
{
	/* nothing */
}

//==========================================================================

/*
 * Supercard SD/CF RAM/IO switching.
 */

/* Save the value at the switch address */
static u16 sc_save_value;

/* Set the mode of Supercard CF/SD to I/O */
static void sc_set_io(void)
{
	sc_save_value = readw(0x09FFFFFE);
	writew(0xA55A, 0x09FFFFFE);
	writew(0xA55A, 0x09FFFFFE);
	writew(0x0003, 0x09FFFFFE);
	writew(0x0003, 0x09FFFFFE);
}

/* Set the mode of Supercard CF/SD to RAM */
static void sc_set_ram(void)
{
	writew(0xA55A, 0x09FFFFFE);
	writew(0xA55A, 0x09FFFFFE);
	writew(0x0005, 0x09FFFFFE);
	writew(0x0005, 0x09FFFFFE);
	writew(sc_save_value, 0x09FFFFFE);
}

//==========================================================================

/*
 * M3 Adaptor SD/CF RAM/IO switching.
 */

/* Set the mode of M3 CF/SD to I/O */
static void m3_set_io(void)
{
	(void)readw(0x08e00002);
	(void)readw(0x0800000e);
	(void)readw(0x08801ffc);
	(void)readw(0x0800104a);
	(void)readw(0x08800612);
	(void)readw(0x08000000);
	(void)readw(0x08801b66);
	(void)readw(0x08800006);	// mode MEDIA
	(void)readw(0x0800080e);
	(void)readw(0x08000000);
	(void)readw(0x09000000);
}

/* Set the mode of M3 CF/SD to RAM */
static void m3_set_ram(void)
{
	(void)readw(0x08e00002);
	(void)readw(0x0800000e);
	(void)readw(0x08801ffc);
	(void)readw(0x0800104a);
	(void)readw(0x08800612);
	(void)readw(0x08000000);
	(void)readw(0x08801b66);
	(void)readw(0x0880000C);	// mode RAM (RW) 
	(void)readw(0x0800080e);
	(void)readw(0x08000000);
	(void)readw(0x09000000);
}

//==========================================================================

/*
 * Opera Memory Extension RAM/IO switching.
 */

/* Set the mode of Opera Extension to I/O */
static void op_set_io(void)
{
	writew(0, 0x08240000);
}

/* Set the mode of Opera Extension to RAM */
static void op_set_ram(void)
{
	writew(1, 0x08240000);
}

//==========================================================================

/*
 * EZ-4 and EZ-5 Tripple Pack Memory Extension RAM/IO switching.
 */

static inline void ez_OpenNorWrite(void)
{
	writew(0xd200, 0x9fe0000);
	writew(0x1500, 0x8000000);
	writew(0xd200, 0x8020000);
	writew(0x1500, 0x8040000);
	writew(0x1500, 0x9C40000);
	writew(0x1500, 0x9fc0000);
}

static inline void ez_CloseNorWrite(void)
{
	writew(0xd200, 0x9fe0000);
	writew(0x1500, 0x8000000);
	writew(0xd200, 0x8020000);
	writew(0x1500, 0x8040000);
	writew(0xd200, 0x9C40000);
	writew(0x1500, 0x9fc0000);
}

static inline void ez_SetRompage(u16 page)
{
	writew(0xd200, 0x9fe0000);
	writew(0x1500, 0x8000000);
	writew(0xd200, 0x8020000);
	writew(0x1500, 0x8040000);
	writew(page,   0x9880000);
	writew(0x1500, 0x9fc0000);
}

static inline void ez_SetNandControl(u16 control)
{
	writew(0xd200, 0x9fe0000);
	writew(0x1500, 0x8000000);
	writew(0xd200, 0x8020000);
	writew(0x1500, 0x8040000);
	writew(control,0x9400000);
	writew(0x1500, 0x9fc0000);
}

/* This is from the EZ-4 driver.
   Obviously, this is the combination of
   OpenNorWrite() and NandControl(1).
*/
static inline void ez_SD_Enable(void)
{
	writew(0xd200, 0x9fe0000);
	writew(0x1500, 0x8000000);
	writew(0xd200, 0x8020000);
	writew(0x1500, 0x8040000);
	writew(0x0001, 0x9400000);
	writew(0x1500, 0x9C40000);
	writew(0x1500, 0x9fc0000);
}

/* This is from the EZ-4 driver.
   Obviously, this is the combination of
   CloseNorWrite() and NandControl(0).
*/
static inline void ez_SD_Disable(void)
{
	writew(0xd200, 0x9fe0000);
	writew(0x1500, 0x8000000);
	writew(0xd200, 0x8020000);
	writew(0x1500, 0x8040000);
	writew(0x0000, 0x9400000);
	writew(0xd200, 0x9C40000);
	writew(0x1500, 0x9fc0000);
}

/* Set the mode of EZ to I/O */
static void ez_set_io(void)
{
	/* seems that nothing is needed for the EZ-4 */
}

/* Set the mode of EZ to PSRAM */
static void ez_set_ram(void)
{
	/* seems that nothing is needed for the EZ-4 */
}

//==========================================================================

/*
 * G6
 */

/* Set the mode of G6 to I/O */
static void g6_set_io(void)
{
	(void)readw(0x09000000);
	(void)readw(0x09FFFFE0);
	(void)readw(0x09FFFFEC);
	(void)readw(0x09FFFFEC);
	(void)readw(0x09FFFFEC);
	(void)readw(0x09FFFFFC);
	(void)readw(0x09FFFFFC);
	(void)readw(0x09FFFFFC);
	(void)readw(0x09FFFF4A);
	(void)readw(0x09FFFF4A);
	(void)readw(0x09FFFF4A);
	(void)readw(0x09200006);
	(void)readw(0x09FFFFF0);
	(void)readw(0x09FFFFE8);
}

/* Set the mode of G6 to RAM */
static void g6_set_ram(void)
{
	(void)readw(0x09000000);
	(void)readw(0x09FFFFE0);
	(void)readw(0x09FFFFEC);
	(void)readw(0x09FFFFEC);
	(void)readw(0x09FFFFEC);
	(void)readw(0x09FFFFFC);
	(void)readw(0x09FFFFFC);
	(void)readw(0x09FFFFFC);
	(void)readw(0x09FFFF4A);
	(void)readw(0x09FFFF4A);
	(void)readw(0x09FFFF4A);
	(void)readw(0x0920000C);
	(void)readw(0x09FFFFF0);
	(void)readw(0x09FFFFE8);
}

//==========================================================================
/*
 * Test if the memory area is working RAM.
 * @param	addr	Start address of area
 * @param	length	Area length in bytes
 * @return	0 if error, 1 if RAM successfully tested
 *		The data cache is supposed to be OFF for gba area here.
 */
static int gba_testram(u32 addr, u32 length)
{
	register volatile u32* p = (volatile u32*) addr;
	register u32 len  = length >> 2;
	register u32 seed = 0x12345678 + addr;
	/* write to memory */
	for (; len; len--)
		*p++ = seed--;
	p = (volatile u32*) addr;
	len = length >> 2;
	seed = 0x12345678 + addr;
	/* readback written values */
	for (; len; len--)
		if (*p++ != seed--)
			return 0;
	return 1;		
}

//==========================================================================

/*
 * try to activate GBA RAM memory area.
 * @param	set_ram		function to activate RAM
 * @param	set_io		function to switch area to IO
 * @param	start		start address of RAM
 * @param	length		length of RAM
 * @return			0 if error, 1 if successfull, RAM is activated
 *		The data cache is supposed to be OFF for gba area here.
 */
static int gba_testcard( void (*set_ram)(void), void (*set_io)(void), u32 start, u32 length)
{
	/* try to activate RAM */
	set_ram();
	/* test address range in RAM */
	if (gba_testram(start, 256)) {
		/* success! */
		gba_set_ram = set_ram;
		gba_set_io  = set_io;
		gba_start   = start;
		gba_length  = length;
		return 1;
	}
	/* failure */
	return 0;
}

//==========================================================================
/*
 * Special detection function for the EZ cards.
 */
int ez_detect(void)
{
	u32 len;

	// EZ4:switch to OS mode
	ez_SetRompage(0x8000);
	// enable writing
	ez_OpenNorWrite();

	// detect EZ4 PSRAM starting at 0x08400000
	if (!gba_testram(0x08000000, 256) && gba_testram(0x08400000, 256))
		goto ez_ok;

	// EZ5 3in1 has no OS mode
	ez_CloseNorWrite();
	// EZ5:switch PSRAM start to 0x08400000
	ez_SetRompage(352);
	// enable writing
	ez_OpenNorWrite();

	// detect EZ5 3in1 PSRAM starting at 0x08400000
	// EZ5 has mirrored PSRAM from 0x08000000 to 0x08400000
	if (gba_testram(0x08400000, 256))
		goto ez_ok;

	// nothing found
	return 0;

ez_ok:	// test for end address (128MB PSRAM or only 64 MB?)
	len = 0x00800000;
	if (gba_testram(0x08400000+len, 256))
		len = 0x01000000;
	// fill in result values
	gba_set_ram = ez_set_ram;
	gba_set_io  = ez_set_io;
	gba_start   = 0x08400000;
	gba_length  = len;
	return 1;
}

//==========================================================================
int gba_activate_ram(void)
{
	/* Test EZ first */
	if (ez_detect()) goto activated;

	/* test supercard CF/SD */
	if (gba_testcard(sc_set_ram, sc_set_io, 0x08000000, 0x02000000)) goto activated;
	/* test M3 adaptor CF/SD */
	if (gba_testcard(m3_set_ram, m3_set_io, 0x08000000, 0x02000000)) goto activated;
	/* test Opera Memory Extension */
	if (gba_testcard(op_set_ram, op_set_io, 0x09000000, 0x00800000)) goto activated;
	/* test G6 */
	if (gba_testcard(g6_set_ram, g6_set_io, 0x08000000, 0x02000000)) goto activated;

	/* insert other adaptors here */

	/* None found, allocate dummy functions for ram/io switching */
	gba_set_ram = gba_dummy_set;
	gba_set_io  = gba_dummy_set;
	gba_start   = 0;
	gba_length  = 0;
	return 0;

activated:

// Turbo mode detection does break DLDI driver for M3 ...
// For Supercard (lite), and maybe other SDRAM memory, Turbo mode is not working
// because of the slow interface. The only cards where Turbo mode MAY be working
// is the opera expansion and the 3in1 expansion because they are using PSRAM.
#if 0
	{
		/* the TURBO mode: try to lower the access time to GBA ROM space */
		unsigned short wait_value;
#define WAIT_CR 	0x04000204

		/* read the original WAIT_CR value */
		wait_value = readw(WAIT_CR);
		printk(KERN_INFO "WAIT_CR was:    0x%X\n", (int)wait_value);

		// Switch to high speed:
		// bit 0-1     RAM-region access cycle control 0..3=10,8,6,18 cycles
		//     2-3     ROM 1st access cycle control    0..3=10,8,6,18 cycles
		//       4     ROM 2nd access cycle control    0..1=6,4 cycles
		// reset value is 00000b

		// try to speed up 2nd access rom cycle (data cache block moves)
        	writew( wait_value | 0x0010, WAIT_CR);
		if (gba_testram(gba_start, 0x1000)) {
			wait_value |= 0x0010;
		}
		// try to speed up 1nd access rom cycle most
        	writew( wait_value | 0x0008, WAIT_CR);
		if (gba_testram(gba_start, 0x1000)) {
			wait_value |= 0x0008;
		} else {
			// try to speed up 1nd access rom cycle
        		writew( wait_value | 0x0004, WAIT_CR);
			if (gba_testram(gba_start, 0x1000)) {
				wait_value |= 0x0004;
			}
		}
		// set highest speed value
        	writew( wait_value, WAIT_CR);
		printk(KERN_INFO "WAIT_CR set to: 0x%X\n", (int)wait_value);
	}
#endif

	/* Activate the data cache for GBA ROM space so CONFIG_NDS_ROM8BIT can work */
	{
		/* this is the same pattern used in head.S */
		u32 pattern = 0x82;
		__asm__ __volatile__(                           
		"mcr	p15, 0, %0, c2, c0, 0"
		: : "r" (pattern) );
	}

	return 1;
}



