/* blkmem.c: Block access to memory spaces
 *
 * Copyright (C) 2000, 2001  Lineo Inc.  (www.lineo.com)   
 * Copyright (C) 1997, 1998  D. Jeff Dionne <jeff@lineo.ca>,
 *                           Kenneth Albanowski <kjahds@kjahds.com>,
 * Copyright (C) 2000-2002   Greg Ungerer <gerg@snapgear.com>
 *
 * Based z2ram - Amiga pseudo-driver to access 16bit-RAM in ZorroII space
 * Copyright (C) 1994 by Ingo Wilken (Ingo.Wilken@informatik.uni-oldenburg.de)
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
 */


#include <linux/config.h>
#include <linux/major.h>
#include <linux/malloc.h>

#define MAJOR_NR    BLKMEM_MAJOR

#include <linux/blk.h>
#include <linux/blkmem.h>
#if defined(MODULE)
#include <linux/module.h>
#endif
#include <asm/bitops.h>
#include <asm/delay.h>

#undef VERBOSE
#undef DEBUG

#define TRUE                  (1)
#define FALSE                 (0)

/*
 *	Timeout loop counter for FLASH operations.
 */
#define	FTIMEOUT	990000000


/*
 * Please, configure the ROMFS for your system here
 */

#ifdef __i960__
extern char __data_rom_start[];
#define FIXED_ROMARRAY __data_rom_start
#endif

#if defined( CONFIG_M68328 ) || defined ( CONFIG_M68EZ328 )
#include <asm/shglports.h>
#define CAT_ROMARRAY
#endif

#if defined( CONFIG_PILOT ) && defined( CONFIG_M68EZ328 )
# ifdef CONFIG_PILOT_INCROMFS
   extern char _flashstart[];
#  define FIXED_ROMARRAY _flashstart
# else
   #undef CAT_ROMARRAY /* we don't want this guy */
   extern char _ebss[], _etext[], _edata[], _sdata[];
#  ifdef CONFIG_RAMKERNEL
#   define FIXED_ROMARRAY _ebss
#  else
#   define FIXED_ROMARRAY ((_etext) + ((_edata) - (_sdata)))
#  endif
# endif
#endif

#if defined(CONFIG_MWI) 
  #undef CAT_ROMARRAY
  #if defined(CONFIG_ROMFS_FROM_ROM) && defined(CONFIG_RAMKERNEL)
    /* get romfs from flash beginning at address 8 */
    /* kernel is in ram */
    #define FIXED_ROMARRAY ((unsigned long)8)
  #elif defined(CONFIG_ROMFS_FROM_ROM) && defined(CONFIG_ROMKERNEL)
    /* get romfs from flash beginning after the data section in flash */
    /* kernel is in rom */
    extern char __romfs[];
    #define FIXED_ROMARRAY ((unsigned long) __romfs)
  #elif !defined(CONFIG_ROMFS_FROM_ROM) && defined(CONFIG_RAMKERNEL)
    /* romfs got copied after bss in ram */
    /* kernel is in ram */
    extern char _end[];
    #define FIXED_ROMARRAY ((unsigned long)_end)
  #elif !defined(CONFIG_ROMFS_FROM_ROM) && defined(CONFIG_ROMKERNEL)
    /* '>' prompted and loaded romfs after bss in ram */
    /* the same as in the upper case */
    extern char _end[];
    #define FIXED_ROMARRAY ((unsigned long)_end)
  #endif
#endif

#ifdef CONFIG_UCSIMM
#define CAT_ROMARRAY
#endif

#ifdef CONFIG_LEON_2
extern char _etext[];
#define FIXED_ROMARRAY	(_etext)
#endif

#if	defined(CONFIG_M68376) || defined(CONFIG_FR1000)
extern char _ramend[];
#define FIXED_ROMARRAY	(_ramend)
#endif

#if defined(CONFIG_M68EZ328ADS) || defined(CONFIG_ALMA_ANS) || \
		defined(CONFIG_CWEZ328)
#ifdef CONFIG_RAMKERNEL
extern char _flashstart[];
#define FIXED_ROMARRAY _flashstart
#else
#define CAT_ROMARRAY
#endif
#endif

#ifdef CONFIG_ARCH_ATMEL
//#define INTERNAL_ROMARRAY
/*
 * If nothing has been selected,  then assume we are on
 * the armulator or some similar non-spec platform
 */
 #if !defined(INTERNAL_ROMARRAY) && !defined(FIXED_ROMARRAY)
 #define FIXED_ROMARRAY FLASH_MEM_BASE
 #endif
#endif

#ifdef CONFIG_ARCH_TRIO
#define FIXED_ROMARRAY (char*)(3512*1024)
#endif

#ifdef CONFIG_ARCH_GBA
extern int	_etext;
extern int	_sdata;
extern int	_edata;
#endif

#ifdef CONFIG_COLDFIRE
#ifdef CONFIG_TELOS
#define CAT_ROMARRAY
#else
/*
 *	The ROMfs sits after the kernel bss segment.
 */
unsigned char *romarray;
extern char _ebss;

/*
 *	Stub out the LED functions for now.
 */
#define	SET_ALARM_LED(x)
#define	GET_COMM_STATUS_LED(x)
#define	SET_COMM_STATUS_LED(x)
#define	SET_COMM_ERROR_LED(x)
#endif /* not CONFIG_TELOS */
#endif /* CONFIG_COLDFIRE */

/* 2002-05-21 gc: */
#if defined( CONFIG_SM2010 ) 
#define CAT_ROMARRAY
#endif

/******* END OF BOARD-SPECIFIC CONFIGURATION ************/

/* Simple romfs, at internal, cat on the end of kernel, or seperate fixed adderess romfs. */

#ifdef INTERNAL_ROMARRAY
#include "romdisk.c"
#endif

#ifdef CAT_ROMARRAY
unsigned char *romarray;
extern char __data_rom_start[];
extern char _edata[];
extern char __data_start[];
#endif

#if defined(CONFIG_WATCHDOG)
extern void watchdog_disable(void);
extern void watchdog_enable(void);
#endif

#ifdef __H8300H__
extern char _rootimage[];
#endif

#ifdef FIXED_ROMARRAY
unsigned char *romarray = (char *)(FIXED_ROMARRAY);
#endif

/* If defined, ROOT_ARENA causes the root device to be the specified arena, useful with romfs */

/* Now defining ROOT_DEV in arch/setup.c */
/*#define ROOT_ARENA 0*/

struct arena_t;

typedef void (*xfer_func_t)(struct arena_t *, unsigned long address, unsigned long length, char * buffer);
typedef void (*erase_func_t)(struct arena_t *, unsigned long address);
typedef void (*program_func_t)(struct arena_t *, struct blkmem_program_t * prog);

#ifndef CONFIG_COLDFIRE
void program_main(struct arena_t *, struct blkmem_program_t *);
void read_spare(struct arena_t *, unsigned long, unsigned long, char *);
void write_spare(struct arena_t *, unsigned long, unsigned long, char *);
void erase_spare(struct arena_t *, unsigned long);
#endif

#if defined(CONFIG_ARNEWSH) || defined(CONFIG_M5206eC3)
void flash_amd8_pair_write(struct arena_t *, unsigned long, unsigned long, char *);
void flash_amd8_pair_erase(struct arena_t *, unsigned long);
#endif

#if defined(CONFIG_FLASH_SNAPGEAR)
#if defined(CONFIG_INTELFLASH)
/*
 *	SnapGear hardware with INTEL FLASH.
 */
static void flash_intel_writeall(struct arena_t *, struct blkmem_program_t *);
static void flash_intel_write(struct arena_t *, unsigned long, unsigned long, char *);
static void flash_intel_erase(struct arena_t *, unsigned long);
#define	flash_erase	flash_intel_erase
#define	flash_write	flash_intel_write
#define	flash_writeall	flash_intel_writeall
#else
#ifdef CONFIG_FLASH8BIT
/*
 *	SnapGear hardware with 8bit AMD FLASH.
 */
static void flash_amd8_writeall(struct arena_t *, struct blkmem_program_t *);
static void flash_amd8_write(struct arena_t *, unsigned long, unsigned long, char *);
static void flash_amd8_erase(struct arena_t *, unsigned long);
#define	flash_erase	flash_amd8_erase
#define	flash_write	flash_amd8_write
#define	flash_writeall	flash_amd8_writeall
#else
/*
 *	SnapGear hardware with 16bit AMD FLASH (this is the default).
 */
static void flash_amd16_writeall(struct arena_t *, struct blkmem_program_t *);
static void flash_amd16_write(struct arena_t *, unsigned long, unsigned long, char *);
static void flash_amd16_erase(struct arena_t *, unsigned long);
#define	flash_erase	flash_amd16_erase
#define	flash_write	flash_amd16_write
#define	flash_writeall	flash_amd16_writeall
#endif /* !COFNIG_FLASH8BIT */
#endif /* !CONFIG_INTELFLASH */
#endif /* CONFIG_FLASH_SNAPGEAR */

#if defined(CONFIG_HW_FEITH)
/*
 *	Feith hardware with 16bit AMD FLASH (this is the default).
 */
static void flash_amd16_writeall(struct arena_t *, struct blkmem_program_t *);
static void flash_amd16_write(struct arena_t *, unsigned long, unsigned long, char *);
static void flash_amd16_erase(struct arena_t *, unsigned long);
#define	flash_erase	flash_amd16_erase
#define	flash_write	flash_amd16_write
#define	flash_writeall	flash_amd16_writeall
#endif

#if defined(CONFIG_MWI) && defined(CONFIG_FLASH1MB)
static void flash_amd16_writeall(struct arena_t *, struct blkmem_program_t *);
static void flash_amd16_write(struct arena_t *, unsigned long, unsigned long, char *);
static void flash_amd16_erase(struct arena_t *, unsigned long);
#define	flash_erase	flash_amd16_erase
#define	flash_write	flash_amd16_write
#define	flash_writeall	flash_amd16_writeall
#endif

/* This array of structures defines the actual set of memory arenas, including
   access functions (if the memory isn't part of the main address space) */

struct arena_t {
	int rw;

	unsigned long address; /* Address of memory arena */
	unsigned long length;  /* Length of memory arena. If -1, try to get size from romfs header */

	program_func_t program_func; /* Function to program in one go */

	xfer_func_t read_func; /* Function to transfer data to main memory, or zero if none needed */
	xfer_func_t write_func; /* Function to transfer data from main memory, zero if none needed */

	erase_func_t erase_func; /* Function to erase a block of memory to zeros, or 0 if N/A */
	unsigned long blksize; /* Size of block that can be erased at one time, or 0 if N/A */
	unsigned long unitsize;
	unsigned char erasevalue; /* Contents of sectors when erased */

	/*unsigned int auto_erase_bits;
	unsigned int did_erase_bits;*/

} arena[] = {

#ifdef INTERNAL_ROMARRAY
	{0, (unsigned long)romarray, sizeof(romarray)},
#endif

#ifdef CAT_ROMARRAY
	{0, 0, -1},
#define FIXUP_ARENAS \
        arena[0].address = (unsigned long)__data_rom_start + (unsigned long)_edata - (unsigned long)__data_start;
#endif

#ifdef CONFIG_ARCH_GBA
	{0, 0, -1},
#define FIXUP_ARENAS \
	arena[0].address = ((unsigned long) &_etext) + \
		((unsigned long) &_edata) - ((unsigned long) &_sdata);
#endif

#ifdef FIXED_ROMARRAY
#ifndef CONFIG_ROOT_NFS
	{0, FIXED_ROMARRAY, -1},
#endif
#endif

#ifdef CONFIG_COLDFIRE
    /*
     * The ROM file-system is RAM resident on the ColdFire eval boards.
     * This arena is defined for access to it.
     */
    {0, 0, -1},

#ifndef CONFIG_ROMFS_FROM_ROM
  #define FIXUP_ARENAS 	arena[0].address = (unsigned long) &_ebss;
#else
  #define FIXUP_ARENAS	{ \
		register char *sp = (char *) arena[4].address; \
		register char *ep = sp + arena[4].length; \
		if (strncmp((char *) &_ebss, "-rom1fs-", 8) == 0) { \
			sp = (char *) &_ebss; \
		} else { \
			while (sp < ep && strncmp(sp, "-rom1fs-", 8)) \
				sp++; \
			if (sp >= ep) \
				sp = &_ebss; \
		} \
		arena[0].address = (unsigned long) sp; \
	}
#endif /* CONFIG_ROMFS_FROM_ROM */

#ifdef CONFIG_ARN5206
    /*
     *  The spare FLASH segment on the Arnewsh 5206 board.
     */
    {1,0xffe20000,0x20000,0,0,flash_amd8_pair_write,flash_amd8_pair_erase,0x8000,0x20000,0xff},
#endif

#if defined(CONFIG_ARN5307) || defined(CONFIG_M5206eC3)
    /*  pair of AM29LV004T flash for 1Mbyte total
     *  rom0 -- root file-system (actually in RAM)
     *  rom1 -- FLASH SA0   128K boot
     *  rom2 -- FLASH SA1-6 768k kernel & romfs
     *  rom3 -- FLASH SA7   64k spare
     *  rom4 -- FLASH SA8   16k spare
     *  rom5 -- FLASH SA9   16k spare
     *  rom6 -- FLASH SA10  32k spare
     */ 
    {1,0xffe00000,0x20000,0,0,flash_amd8_pair_write,flash_amd8_pair_erase,0x20000,0x20000,0xff},
    {1,0xffe20000,0xc0000,0,0,flash_amd8_pair_write,flash_amd8_pair_erase,0x20000,0xc0000,0xff},
    {1,0xffee0000,0x10000,0,0,flash_amd8_pair_write,flash_amd8_pair_erase,0x10000,0x10000,0xff},
    {1,0xffef0000,0x4000,0,0,flash_amd8_pair_write,flash_amd8_pair_erase,0x4000,0x4000,0xff},
    {1,0xffef4000,0x4000,0,0,flash_amd8_pair_write,flash_amd8_pair_erase,0x4000,0x4000,0xff},
    {1,0xffef8000,0x8000,0,0,flash_amd8_pair_write,flash_amd8_pair_erase,0x8000,0x8000,0xff},
#endif /* CONFIG_M5206eC3 */

#if defined(CONFIG_FLASH_SNAPGEAR)
#if defined(CONFIG_FLASH128K)
    /*
     *	SnapGear hardware with 128K FLASH.
     *	The following devices are supported:
     *		rom0 -- root file-system (actually in RAM)
     *		rom1 -- FLASH boot block (16k)
     *		rom2 -- FLASH boot arguments (16k)
     *		rom3 -- FLASH MAC addresses (16k)
     *		rom4 -- FLASH config file-system (64k)
     *		rom6 -- FLASH spare block (16k)
     *		rom5 -- FLASH the whole damn thing (128k)!
     */
    {1,0xf0000000,0x004000,flash_writeall, 0, 0, 0,    0x04000,0x004000,0xff},
    {1,0xf0004000,0x004000,0,0,flash_write,flash_erase,0x04000,0x004000,0xff},
    {1,0xf0008000,0x004000,0,0,flash_write,flash_erase,0x04000,0x004000,0xff},
    {1,0xf000c000,0x004000,0,0,flash_write,flash_erase,0x04000,0x004000,0xff},
    {1,0xf0010000,0x010000,flash_writeall,0,flash_write,flash_erase,0x04000,0x010000,0xff},
    {1,0xf0000000,0x020000,flash_writeall, 0, 0, 0,    0x04000,0x020000,0xff},
#elif defined(CONFIG_FLASH8MB)
    /*
     *	SnapGear hardware with 8MB FLASH.
     *	The following devices are supported:
     *		rom0 -- root file-system (actually in RAM)
     *		rom1 -- FLASH boot block (128k)
     *		rom2 -- FLASH boot arguments (128k)
     *		rom3 -- FLASH MAC addresses (128k)
     *		rom4 -- FLASH kernel+file-system binary (7mb)
     *		rom5 -- FLASH config file-system (512k)
     *		rom6 -- FLASH the whole damn thing (8Mb)!
     *		rom7 -- FLASH spare block (128k)
     */
    {1,0xf0000000,0x020000,flash_writeall, 0, 0, 0,    0x20000,0x020000,0xff},
    {1,0xf0020000,0x020000,0,0,flash_write,flash_erase,0x20000,0x020000,0xff},
    {1,0xf0040000,0x020000,0,0,flash_write,flash_erase,0x20000,0x020000,0xff},
    {1,0xf0100000,0x700000,flash_writeall, 0, 0, 0,    0x20000,0x700000,0xff},
    {1,0xf0080000,0x080000,0,0,flash_write,flash_erase,0x20000,0x080000,0xff},
    {1,0xf0000000,0x800000,flash_writeall, 0, 0, 0,    0x20000,0x800000,0xff},
    {1,0xf0060000,0x020000,flash_writeall,0,flash_write,flash_erase,0x20000,0x02000,0xff},
#elif defined(CONFIG_FLASH2MB) || defined(CONFIG_FLASH4MB)
    /*
     *	SnapGear hardware with 2MB/4MB FLASH.
     *	The following devices are supported:
     *		rom0 -- root file-system (actually in RAM)
     *		rom1 -- FLASH boot block (16k)
     *		rom2 -- FLASH boot arguments (8k)
     *		rom3 -- FLASH MAC addresses (8k)
     *		rom4 -- FLASH kernel+file-system binary (1920k)
     *		rom5 -- FLASH config file-system (64k)
     *		rom6 -- FLASH the whole damn thing (2Mb)!
     *		rom7 -- FLASH spare block (32k)
     *		rom8 -- FLASH2 kernel+file-system binary (1920k) (4MB only)
     *		rom9 -- FLASH2 the whole damn thing (2Mb)!
     */
    {1,0xf0000000,0x004000,flash_writeall, 0, 0, 0,    0x04000,0x004000,0xff},
    {1,0xf0004000,0x002000,0,0,flash_write,flash_erase,0x02000,0x002000,0xff},
    {1,0xf0006000,0x002000,0,0,flash_write,flash_erase,0x02000,0x002000,0xff},
    {1,0xf0020000,0x1e0000,flash_writeall, 0, 0, 0,    0x10000,0x1e0000,0xff},
    {1,0xf0010000,0x010000,0,0,flash_write,flash_erase,0x10000,0x010000,0xff},
    {1,0xf0000000,0x200000,flash_writeall, 0, 0, 0,    0x10000,0x200000,0xff},
    {1,0xf0008000,0x08000,flash_writeall,0,flash_write,flash_erase,0x08000,0x08000,0xff},
#if defined(CONFIG_FLASH4MB)
    {1,0xf0220000,0x1e0000,flash_writeall, 0, 0, 0,    0x10000,0x1e0000,0xff},
    {1,0xf0200000,0x200000,flash_writeall, 0, 0, 0,    0x10000,0x200000,0xff},
#endif
#else
    /*
     *	SnapGear hardware FLASH erase/program entry points (1MB FLASH).
     *	The following devices are supported:
     *		rom0 -- root file-system (actually in RAM)
     *		rom1 -- FLASH boot block (16k)
     *		rom2 -- FLASH boot arguments (8k)
     *		rom3 -- FLASH MAC addresses (8k)
     *		rom4 -- FLASH kernel+file-system binary (896k)
     *		rom5 -- FLASH config file-system (64k)
     *		rom6 -- FLASH the whole damn thing (1Mb)!
     *		rom7 -- FLASH spare block (32k)
     */
    {1,0xf0000000,0x04000,flash_writeall, 0, 0, 0,    0x04000,0x04000,0xff},
    {1,0xf0004000,0x02000,0,0,flash_write,flash_erase,0x02000,0x02000,0xff},
    {1,0xf0006000,0x02000,0,0,flash_write,flash_erase,0x02000,0x02000,0xff},
    {1,0xf0010000,0xe0000,flash_writeall, 0, 0, 0,    0x10000,0xe0000,0xff},
    {1,0xf00f0000,0x10000,0,0,flash_write,flash_erase,0x10000,0x10000,0xff},
    {1,0xf0000000,0x100000,flash_writeall, 0, 0, 0,  0x10000,0x100000,0xff},
    {1,0xf0008000,0x08000,flash_writeall,0,flash_write,flash_erase,0x08000,0x08000,0xff},
#if defined(CONFIG_EXTRA_FLASH1MB)
    /*
     *		rom8 -- FLASH extra. where the NETtel3540 stores the dsl image
     */
    {1,0xf0100000,0x100000,flash_writeall, 0, 0, 0,  0x10000,0x100000,0xff},
#endif /* CONFIG_EXTRA_FLASH1MB */
#endif /* CONFIG_FLASH2MB */
#endif /* CONFIG_FLASH_SNAPGEAR */

#if defined(CONFIG_HW_FEITH)

#if defined(CONFIG_CLEOPATRA) || defined(CONFIG_SCALES)
/*
 *	CLEOPATRA with 2MB/4MB FLASH erase/program entry points.
 *	The following devices are supported:
 *		rom0 -- root file-system (actually in RAM)
 *		rom1 -- FLASH boot block (16k)
 *		rom2 -- FLASH boot arguments (8k)
 *		rom3 -- FLASH MAC addresses (8k)
 *		rom4 -- FLASH kernel+file-system binary (1792k)
 *		rom5 -- FLASH config file-system (64k)
 *		rom6 -- FLASH the whole damn thing (2Mb)!
 *		rom7 -- FLASH spare block (32k)
 *		rom8 -- FLASH application config (128k)
 *		rom9 -- FLASH user block (1536k) (4MB only)
 *		rom10-- FLASH user block (512k) (4MB only)
 *		rom11-- FLASH user block (2048k) (6MB only)
 */
    {1,0xf0000000,0x004000,flash_writeall, 0, 0, 0,    0x04000,0x004000,0xff},
    {1,0xf0004000,0x002000,0,0,flash_write,flash_erase,0x02000,0x002000,0xff},
    {1,0xf0006000,0x002000,0,0,flash_write,flash_erase,0x02000,0x002000,0xff},
    {1,0xf0020000,0x1c0000,flash_writeall, 0, 0, 0,    0x10000,0x1c0000,0xff},
    {1,0xf0010000,0x010000,0,0,flash_write,flash_erase,0x10000,0x010000,0xff},
    {1,0xf0000000,0x200000,flash_writeall, 0, 0, 0,    0x10000,0x200000,0xff},
    {1,0xf0008000,0x08000,flash_writeall,0,flash_write,flash_erase,0x08000,0x08000,0xff},
    {1,0xf01e0000,0x020000,0,0,flash_write,flash_erase,0x10000,0x020000,0xff},
#if defined(CONFIG_FLASH4MB)
    {1,0xf0200000,0x180000,0,0,flash_write,flash_erase,0x10000,0x180000,0xff},
    {1,0xf0380000,0x080000,0,0,flash_write,flash_erase,0x10000,0x080000,0xff},
#endif

#if defined(CONFIG_FLASH6MB)
    {1,0xf0200000,0x180000,0,0,flash_write,flash_erase,0x10000,0x180000,0xff},
    {1,0xf0380000,0x080000,0,0,flash_write,flash_erase,0x10000,0x080000,0xff},
	 // old settings
    {1,0xf0400000,0x200000,0,0,flash_write,flash_erase,0x10000,0x200000,0xff},
	// new settings
//    {1,0xf0410000,0x040000,flash_writeall,0, flash_write,flash_erase,0x10000,0x040000,0xff},
//    {1,0xf0450000,0x1b0000,0,0,flash_write,flash_erase,0x10000,0x1b0000,0xff},

#endif
#endif /* CONFIG_CLEOPATRA || CONFIG_SCALES */

#if defined(CONFIG_CANCam)
/*
 *	CanCam with 8MB FLASH erase/program entry points.
 *	The following devices are supported:
 *		rom0 -- root file-system (actually in RAM)
 *		rom1 -- FLASH boot block (64k)
 *		rom2 -- FLASH boot arguments (64k)
 *		rom3 -- FLASH MAC addresses (64k)
 *		rom4 -- FLASH kernel+file-system binary (1728k)
 *		rom5 -- FLASH config file-system (64k)
 *		rom6 -- FLASH the whole damn thing (8Mb)!
 *		rom7 -- FLASH spare block (64k)
 *		rom8 -- FLASH user block (6MB)
 */
    {1,0xf0000000,0x010000,flash_writeall, 0, 0, 0,    0x10000,0x010000,0xff},
    {1,0xf0010000,0x010000,0,0,flash_write,flash_erase,0x10000,0x010000,0xff},
    {1,0xf0020000,0x010000,0,0,flash_write,flash_erase,0x10000,0x010000,0xff},
    {1,0xf0050000,0x1b0000,flash_writeall, 0, 0, 0,    0x10000,0x1b0000,0xff},
    {1,0xf0040000,0x010000,0,0,flash_write,flash_erase,0x10000,0x010000,0xff},
    {1,0xf0000000,0x800000,flash_writeall, 0, 0, 0,    0x10000,0x800000,0xff},
    {1,0xf0030000,0x010000,flash_writeall,0,flash_write,flash_erase,0x10000,0x10000,0xff},
    {1,0xf0200000,0x600000,0,0,flash_write,flash_erase,0x10000,0x600000,0xff},
#endif /* CONFIG_CanCam */
#endif /* CONFIG_HW_FEITH */

#endif /* CONFIG_COLDFIRE */



#if defined(CONFIG_MWI) && defined(CONFIG_FLASH1MB)
    /*
     *	MWI hardware FLASH erase/program entry points (1MB FLASH).
     *	The following devices are supported:
     *		rom0 -- root file-system (actually in RAM)
     *		rom1 -- FLASH boot block (16k)
     *		rom2 -- FLASH boot arguments (8k)
     *		rom3 -- FLASH MAC addresses (8k)
     *		rom4 -- FLASH kernel+file-system binary (896k)
     *		rom5 -- FLASH config file-system (64k)
     *		rom6 -- FLASH the whole damn thing (1Mb)!
     *		rom7 -- FLASH spare block (32k)
     */
    {1,0x000000, 0x100000, flash_writeall, 0, 0, 0,  0x10000, 0x100000, 0xff},
#if 0
    {1,0x000000, 0x004000, flash_writeall, 0, 0, 0,    0x04000, 0x04000, 0xff},
    {1,0x004000, 0x002000, 0, 0, flash_write, flash_erase, 0x02000, 0x02000, 0xff},
    {1,0x006000, 0x002000, 0, 0, flash_write, flash_erase, 0x02000, 0x02000, 0xff},
    {1,0x010000, 0x0e0000, flash_writeall, 0, 0, 0,    0x10000, 0xe0000, 0xff},
    {1,0x0f0000, 0x010000, 0, 0, flash_write, flash_erase, 0x10000, 0x10000, 0xff},
    {1,0x000000, 0x100000, flash_writeall, 0, 0, 0,  0x10000, 0x100000, 0xff},
    {1,0x008000, 0x008000, flash_writeall, 0, flash_write, flash_erase, 0x08000, 0x08000, 0xff},
#endif
#endif

#ifdef __H8300H__
	{0,_rootimage,-1},
#endif


#ifdef CONFIG_SHGLCORE

#ifdef CONFIG_SHGLCORE_2MEG
	{0, 0x0A0000, 0x200000-0x0A0000},	/* ROM FS */
	{1, SHGLCORE_FLASH_BANK_0_ADDR, 0x80000, 0, 0, write_spare, erase_spare, 0x10000, 0x80000, 0xff},
	{1, 0x000000, 0x200000, program_main, 0,0,0, 0x20000, 0x100000}, /* All main FLASH */
#else
	{0, 0x0A0000, 0x100000-0x0A0000},	/* ROM FS */
	{1, SHGLCORE_FLASH_BANK_0_ADDR, 0x80000, 0, 0, write_spare, erase_spare, 0x10000, 0x80000, 0xff},
	{1, 0x000000, 0x100000, program_main, 0,0,0, 0x20000, 0x100000}, /* All main FLASH */
#endif

#define FIXUP_ARENAS \
	extern unsigned long rom_length; \
	arena[0].length = (unsigned long)rom_length - 0xA0000; \
	arena[2].length = (unsigned long)rom_length;

#endif
};

#define arenas (sizeof(arena) / sizeof(struct arena_t))


static int blkmem_blocksizes[arenas];
static int blkmem_sizes[arenas];


#if defined(CONFIG_ARNEWSH) || defined(CONFIG_M5206eC3)

static struct semaphore spare_lock = MUTEX;

/*
 *	FLASH erase and programming routines for the AMD 8bit, odd/even
 *	FLASH pairs.
 */

void flash_amd8_pair_write(struct arena_t * a, unsigned long pos, unsigned long length, char * buffer)
{
  volatile unsigned short *address;
  unsigned short *wbuf, word;
  unsigned short result, prevresult;
  unsigned long flags, start;
  unsigned long fbase = a->address;
  int i;
  
#if 0
  printk("%s(%d): flash_amd8_pair_write(a=%x,pos=%x,length=%d,buf=%x)\n",
	__FILE__, __LINE__, (int) a, (int) pos, (int) length, (int) buffer);
#endif

  down(&spare_lock);

  start = jiffies;
  address = (unsigned volatile short *) (fbase + pos);
  wbuf = (unsigned short *) buffer;

  for (length >>= 1; (length > 0); length--, address++) {
  
    word = *wbuf++;

    if (*address != word) {
      save_flags(flags); cli();

      *((unsigned volatile short *) (fbase | (0x5555 << 1))) = 0xaaaa;
      *((unsigned volatile short *) (fbase | (0x2aaa << 1))) = 0x5555;
      *((unsigned volatile short *) (fbase | (0x5555 << 1))) = 0xa0a0;
      *address = word;
      udelay(1);

      /* Wait for write to complete, timeout after a few tries */
      for (prevresult = 0, i = 0; (i < 0x10000); i++) {
        result = *address;
        if ((result & 0x8080) == (word & 0x8080))
          break;
	if ((result & 0x4040) == (prevresult & 0x4040))
	  break;
	prevresult = result;
      }
 
      if (*address != word) {
          printk("%s(%d): FLASH write failed, address %p, write value %x"
		" (now %x, previous %x), count=%d\n", __FILE__, __LINE__,
		address, word, *address, prevresult, i);
	  printk("%s(%d): count=%d word=%x prevresult=%x result=%x\n",
                __FILE__, __LINE__, i, word, prevresult, result);
          *((unsigned volatile short *) fbase) = 0xf0f0; /* Reset */
      }
      restore_flags(flags);
    }
  }
  
  up(&spare_lock);
}

void flash_amd8_pair_erase(struct arena_t * a, unsigned long pos)
{
  unsigned volatile short *address;
  unsigned short result;
  unsigned long fbase = a->address;
  unsigned long flags;
  
#if 0
    printk("%s(%d): flash_amd8_pair_erase(): addr:%x, len:%x, pos:%x\n",
		__FILE__, __LINE__, a->address, a->length, pos);
#endif

  if (pos >= a->length)
    return;

  address = (unsigned volatile short *) (fbase + pos);

  /* Mutex all access to FLASH memory */
  down(&spare_lock);
  save_flags(flags); cli();

  /* Initiate erase of FLASH sector */
#ifdef CONFIG_ARN5206
  *((unsigned volatile short *) (fbase | (0x5555 << 1))) = 0xaaaa;
  *((unsigned volatile short *) (fbase | (0x2aaa << 1))) = 0x5555;
  *((unsigned volatile short *) (fbase | (0x5555 << 1))) = 0x8080;
  *((unsigned volatile short *) (fbase | (0x5555 << 1))) = 0xaaaa;
  *((unsigned volatile short *) (fbase | (0x2aaa << 1))) = 0x5555;
  *address = 0x3030;
#else
  *((unsigned volatile short *) (fbase | (0x0555 << 1))) = 0xaaaa;/*first*/
  *((unsigned volatile short *) (fbase | (0x02aa << 1))) = 0x5555;/*second*/
  *((unsigned volatile short *) (fbase | (0x0555 << 1))) = 0x8080;/*third*/
  *((unsigned volatile short *) (fbase | (0x0555 << 1))) = 0xaaaa;/*fourth*/
  *((unsigned volatile short *) (fbase | (0x02aa << 1))) = 0x5555;/*fifth*/
  *address = 0x3030;    /* sixth (0x30 to sector address) */
#endif

    for (;;) {
	result = *address;
/*	printk("result: %x\n", result);    */
	if ((result & 0x8080) == 0x8080)
	    break;    /* sucessful erase */
	if (((result & 0xff00) != 0xff00) && (result & 0x2000)) {
	     printk("%s(%d): erase failed: high byte, address %p\n",
		    __FILE__, __LINE__, address);
	     *((unsigned volatile short *) fbase) = 0xf0f0; /* Reset */
	    break;
	}    
	if (((result & 0x00ff) != 0x00ff) && (result & 0x0020)) {
	     printk("%s(%d): erase failed: low byte, address %p\n",
		    __FILE__, __LINE__, address);
	     *((unsigned volatile short *) fbase) = 0xf0f0; /* Reset */
	    break;
	}
    }

  restore_flags(flags);
  up(&spare_lock);
}
#endif    /* CONFIG_ARNEWSH || CONFIG_M5206eC3 */

/****************************************************************************/
/*                      SNAPGEAR FLASH SUPPORT                              */
/****************************************************************************/

#if defined(CONFIG_FLASH_SNAPGEAR) || defined(CONFIG_HW_FEITH) || (defined(CONFIG_MWI) && defined(CONFIG_FLASH1MB))

static struct semaphore spare_lock = MUTEX;

unsigned long flash_29lv800[] = { 0x4000, 0x2000, 0x2000, 0x8000 };

/*
 *	Support erasing and dumping to scratch FLASH segment.
 */
unsigned long	flash_dumpoffset = 0;

void flash_erasedump(void)
{
	flash_erase(arena + 7, 0);
}

void flash_writedump(char *buf, int len)
{
	flash_write(arena + 7, 0, len, (char *) buf);
}

#ifdef CONFIG_KEY
void flash_readext(char *buf, unsigned long len)
{
	memcpy(buf, (arena+2)->address, len);
}

void flash_writeext(char *buf, unsigned long len)
{
	flash_erase(arena+2, 0);
	flash_write(arena+2, 0, len, buf);
}
#endif

/****************************************************************************/
/*                            AMD 8bit FLASH                                */
/****************************************************************************/

#if defined(CONFIG_FLASH8BIT)

/*
 *	FLASH erase routine for the AMD 29LVX00 family devices.
 */

static void flash_amd8_erase(struct arena_t *a, unsigned long pos)
{
  unsigned volatile char *address;
  unsigned long fbase = a->address;
  unsigned long flags;
  unsigned char status;
  int i;
  
#if 0
  printk("%s(%d): flash_amd8_erase(a=%x,pos=%x)\n", __FILE__, __LINE__,
    (int) a, (int) pos);
#endif

  if (pos >= a->length)
    return;

  address = (unsigned volatile char *) (fbase + pos);

  /* Mutex all access to FLASH memory */
  down(&spare_lock);
  save_flags(flags); cli();

#if defined(CONFIG_WATCHDOG)
  watchdog_disable();
#endif

  /* Erase this sector */
#if CONFIG_FLASH128K
  *((volatile unsigned char *) (fbase | 0x555)) = 0xaa;
  *((volatile unsigned char *) (fbase | 0x2aa)) = 0x55;
  *((volatile unsigned char *) (fbase | 0x555)) = 0x80;
  *((volatile unsigned char *) (fbase | 0x555)) = 0xaa;
  *((volatile unsigned char *) (fbase | 0x2aa)) = 0x55;
#else
  *((volatile unsigned char *) (fbase | 0xaaa)) = 0xaa;
  *((volatile unsigned char *) (fbase | 0x555)) = 0x55;
  *((volatile unsigned char *) (fbase | 0xaaa)) = 0x80;
  *((volatile unsigned char *) (fbase | 0xaaa)) = 0xaa;
  *((volatile unsigned char *) (fbase | 0x555)) = 0x55;
#endif

  *address = 0x30;

  for (i = 0; (i < FTIMEOUT); i++) {
    status = *address;
    if ((status & 0x80) || (status & 0x20))
      break;
  }

  if (*address != 0xff) {
     printk("%s(%d): FLASH erase failed, address %p iteration=%d status=%x\n",
		__FILE__, __LINE__, address, i, status);
     *((unsigned volatile char *) fbase) = 0xf0; /* Reset */
  }

#if defined(CONFIG_WATCHDOG)
  watchdog_enable();
#endif

  restore_flags(flags);
  up(&spare_lock);
}

/*
 *	FLASH programming routine for the 29LVX00 device family.
 */

static void flash_amd8_write(struct arena_t * a, unsigned long pos, unsigned long length, char * buffer)
{
  volatile unsigned char *address;
  unsigned long flags, fbase = a->address;
  unsigned char *wbuf, status;
  int i;
  
#if 0
  printk("%s(%d): flash_amd8_write(a=%x,pos=%x,length=%d,buf=%x)\n",
	__FILE__, __LINE__, (int) a, (int) pos, (int) length, (int) buffer);
#endif

  down(&spare_lock);

#if defined(CONFIG_WATCHDOG)
  watchdog_disable();
#endif

  address = (unsigned volatile char *) (fbase + pos);
  wbuf = (unsigned char *) buffer;

  for (; (length > 0); length--, address++, wbuf++) {
  
    if (*address != *wbuf) {
      save_flags(flags); cli();

#if CONFIG_FLASH128K
      *((volatile unsigned char *) (fbase | 0x555)) = 0xaa;
      *((volatile unsigned char *) (fbase | 0x2aa)) = 0x55;
      *((volatile unsigned char *) (fbase | 0x555)) = 0xa0;
#else
      *((volatile unsigned char *) (fbase | 0xaaa)) = 0xaa;
      *((volatile unsigned char *) (fbase | 0x555)) = 0x55;
      *((volatile unsigned char *) (fbase | 0xaaa)) = 0xa0;
#endif
      *address = *wbuf;

      for (i = 0; (i < FTIMEOUT); i++) {
	status = *address;
	if (status == *wbuf) {
	  /* Program complete */
	  break;
	}
      }

      if (*address != *wbuf) {
          printk("%s(%d): FLASH write failed i=%d, address %p -> %x(%x)\n",
		__FILE__, __LINE__, i, address, *wbuf, *address);
          *((unsigned volatile char *) fbase) = 0xf0; /* Reset */
      }

      restore_flags(flags);
    }
  }

#if defined(CONFIG_WATCHDOG)
  watchdog_enable();
#endif

  up(&spare_lock);
}


/*
 *	Program a complete FLASH image. This runs from DRAM, so no
 *	need to worry about writing to what we are running from...
 */

static void flash_amd8_writeall(struct arena_t * a, struct blkmem_program_t * prog)
{
  unsigned long		base, offset, erased;
  unsigned long		ptr, min, max;
  unsigned char		*w, status;
  int			failures;
  int			i, j, k, l;

#if defined(CONFIG_WATCHDOG)
  watchdog_disable();
#endif
  
#if 0
  printk("FLASH: programming");
#endif
  failures = 0;
  erased = 0;

  cli();

  for (i = 0; (i < prog->blocks); i++) {

#if 0
    printk("%s(%d): block=%d address=%x pos=%x length=%x range=%x-%x\n",
      __FILE__, __LINE__, i, (int) a->address, (int) prog->block[i].pos,
      (int) prog->block[i].length, (int) (prog->block[i].pos + a->address),
      (int) (prog->block[i].pos + prog->block[i].length - 1 + a->address));
#endif

    /*
     *	Erase FLASH sectors for this program block...
     */
    for (l = prog->block[i].pos / a->blksize;
	l <= ((prog->block[i].pos+prog->block[i].length-1) / a->blksize);
	l++) {

      if (erased & (0x1 << l))
	continue;

      ptr = l * a->blksize;
      offset = ptr % a->unitsize;
      base = ptr - offset;
	
      base += a->address;
      ptr += a->address;
      j = 0;

flash_redo:

#if 0
      printk("%s(%d): ERASE BLOCK sector=%d base=%x offset=%x ptr=%x\n",
	  __FILE__, __LINE__, l, (int) base, (int) offset, (int) ptr);
#endif

      /* Erase this sector */
#if CONFIG_FLASH128K
      *((volatile unsigned char *) (base | 0x555)) = 0xaa;
      *((volatile unsigned char *) (base | 0x2aa)) = 0x55;
      *((volatile unsigned char *) (base | 0x555)) = 0x80;
      *((volatile unsigned char *) (base | 0x555)) = 0xaa;
      *((volatile unsigned char *) (base | 0x2aa)) = 0x55;
#else
      *((volatile unsigned char *) (base | 0xaaa)) = 0xaa;
      *((volatile unsigned char *) (base | 0x555)) = 0x55;
      *((volatile unsigned char *) (base | 0xaaa)) = 0x80;
      *((volatile unsigned char *) (base | 0xaaa)) = 0xaa;
      *((volatile unsigned char *) (base | 0x555)) = 0x55;
#endif
      *((volatile unsigned char *) ptr) = 0x30;

      for (k = 0; (k < FTIMEOUT); k++) {
	status = *((volatile unsigned char *) ptr);
	if ((status & 0x80) || (status & 0x20))
	  break;
      }
      if ((!(status & 0x80) && (status & 0x20)) || k >= FTIMEOUT) {
	printk("FLASH: (%d) erase failed\n", __LINE__);
	/* Reset FLASH unit */
	*((volatile unsigned char *) base) = 0xf0;
	failures++;
	/* Continue (with unerased sector) */
      }

      /*
       *  The 29LV800 part has a bunch of small segments at the bottom.
       *  These need to be erased individually, ugh...
       */
      if (a->length > flash_29lv800[0]) {
	if ((ptr & 0x1fffff) < 0x10000) {
	  ptr += flash_29lv800[j++];
	  goto flash_redo;
	}
      }

      erased |= (0x1 << l);
    }

    /*
     *	Program FLASH with the block data...
     */
    min = prog->block[i].pos+a->address;
    max = prog->block[i].pos+prog->block[i].length+a->address;
    w = (unsigned char *) prog->block[i].data;

#if 0
    printk("%s(%d): PROGRAM BLOCK min=%x max=%x\n", __FILE__, __LINE__,
      (int) min, (int) max);
#endif

    for (ptr = min; (ptr < max); ptr++, w++) {
      
      offset = (ptr - a->address) % a->unitsize;
      base = ptr - offset;

#if 0
      printk("%s(%d): PROGRAM base=%x offset=%x ptr=%x value=%x\n",
	  __FILE__, __LINE__, (int) base, (int) offset, (int) ptr, (int) *w);
#endif

#if CONFIG_FLASH128K
      *((volatile unsigned char *) (base | 0x555)) = 0xaa;
      *((volatile unsigned char *) (base | 0x2aa)) = 0x55;
      *((volatile unsigned char *) (base | 0x555)) = 0xa0;
#else
      *((volatile unsigned char *) (base | 0xaaa)) = 0xaa;
      *((volatile unsigned char *) (base | 0x555)) = 0x55;
      *((volatile unsigned char *) (base | 0xaaa)) = 0xa0;
#endif
      *((volatile unsigned char *) ptr) = *w;

      for (j = 0; (j < FTIMEOUT); j++) {
	status = *((volatile unsigned char *) ptr);
	if (status == *w) {
	  /* Program complete */
	  break;
	}
      }

      status = *((volatile unsigned char *) ptr);
      if (status != *w) {
	printk("FLASH: (%d) write failed, addr=%x val=%x status=%x cnt=%d\n",
		__LINE__, (int) ptr, *w, status, j);
	/* Reset FLASH unit */
	*((volatile unsigned char *) ptr) = 0xf0;
	failures++;
      }
    }
  }

  if (failures > 0) {
    printk("FLASH: %d failures programming FLASH!\n", failures);
    return;
  }

#if 0
  printk("\nFLASH: programming successful!\n");
#endif
  if (prog->reset) {
    printk("FLASH: rebooting...\n\n");
    HARD_RESET_NOW();
  }
}

/****************************************************************************/
/*                            AMD 16bit FLASH                               */
/****************************************************************************/

#endif

#ifdef CONFIG_FLASH16BIT

/*
 *	FLASH erase routine for the AMD 29LVX00 family devices.
 */

static void flash_amd16_erase(struct arena_t *a, unsigned long pos)
{
  unsigned volatile short *address;
  unsigned long fbase = a->address;
  unsigned long flags;
  unsigned short status;
  int i;

#if 0
  printk("%s(%d): flash_amd16_erase(a=%x,pos=%x)\n", __FILE__, __LINE__,
    (int) a, (int) pos);
#endif

  if (pos >= a->length)
    return;

  address = (unsigned volatile short *) (fbase + pos);

  /* Mutex all access to FLASH memory */
  down(&spare_lock);
  save_flags(flags); cli();

#if defined(CONFIG_WATCHDOG)
  watchdog_disable();
#endif

  /* Erase this sector */
  /* FIX: check which byte lane the value needs to be on */
  *((volatile unsigned short *) (fbase | (0x555 << 1))) = 0xaaaa;
  *((volatile unsigned short *) (fbase | (0x2aa << 1))) = 0x5555;
  *((volatile unsigned short *) (fbase | (0x555 << 1))) = 0x8080;
  *((volatile unsigned short *) (fbase | (0x555 << 1))) = 0xaaaa;
  *((volatile unsigned short *) (fbase | (0x2aa << 1))) = 0x5555;
  *address = 0x3030;

  for (i = 0; (i < FTIMEOUT); i++) {
    status = *address;
    if ((status & 0x0080) || (status & 0x0020))
      break;
  }

  if (*address != 0xffff) {
     printk("%s(%d): FLASH erase failed, address %p iteration=%d status=%x\n",
		__FILE__, __LINE__, address, i, status);
     *((unsigned volatile short *) fbase) = 0xf0f0; /* Reset */
  }

#if defined(CONFIG_WATCHDOG)
  watchdog_enable();
#endif

  restore_flags(flags);
  up(&spare_lock);
}

/*
 *	FLASH programming routine for the 29LVX00 device family.
 */

static void flash_amd16_write(struct arena_t * a, unsigned long pos, unsigned long length, char * buffer)
{
  volatile unsigned short *address;
  unsigned long flags, fbase = a->address;
  unsigned short *wbuf, status;
  int i;

  printk("%s(%d): flash_amd16_write(a=%x,pos=%x,length=%d,buf=%x)\n",
	__FILE__, __LINE__, (int) a, (int) pos, (int) length, (int) buffer);

  down(&spare_lock);

#if defined(CONFIG_WATCHDOG)
  watchdog_disable();
#endif

  address = (unsigned volatile short *) (fbase + pos);
  wbuf = (unsigned short *) buffer;

  for (length >>= 1; (length > 0); length--, address++, wbuf++) {

    if (*address != *wbuf) {
      save_flags(flags); cli();

      *((volatile unsigned short *) (fbase | (0x555 << 1))) = 0xaaaa;
      *((volatile unsigned short *) (fbase | (0x2aa << 1))) = 0x5555;
      *((volatile unsigned short *) (fbase | (0x555 << 1))) = 0xa0a0;
      *address = *wbuf;

      for (i = 0; (i < FTIMEOUT); i++) {
	status = *address;
	if (status == *wbuf) {
	  /* Program complete */
	  break;
	}
      }

      if (*address != *wbuf) {
          printk("%s(%d): FLASH write failed i=%d, address %p -> %x(%x)\n",
		__FILE__, __LINE__, i, address, *wbuf, *address);
          *((unsigned volatile short *) fbase) = 0xf0f0; /* Reset */
      }

      restore_flags(flags);
    }
  }

#if defined(CONFIG_WATCHDOG)
  watchdog_enable();
#endif

  up(&spare_lock);
}


/*
 *	Program a complete FLASH image. This runs from DRAM, so no
 *	need to worry about writing to what we are running from...
 */

static void flash_amd16_writeall(struct arena_t * a, struct blkmem_program_t * prog)
{
  unsigned long		base, offset, erased;
  unsigned long		ptr, min, max;
  unsigned short	*w, status;
  int			failures;
  int			i, j, k, l;

#if defined(CONFIG_WATCHDOG)
  watchdog_disable();
#endif

  printk("FLASH: programming");

  failures = 0;
  erased = 0;

  cli();

  for (i = 0; (i < prog->blocks); i++) {

    printk("%s(%d): block=%d address=%x pos=%x length=%x range=%x-%x\n",
      __FILE__, __LINE__, i, (int) a->address, (int) prog->block[i].pos,
      (int) prog->block[i].length, (int) (prog->block[i].pos + a->address),
      (int) (prog->block[i].pos + prog->block[i].length - 1 + a->address));

#if 0
    if (prog->block[i].length > 0x100000)
      break;
#endif

    /*
     *	Erase FLASH sectors for this program block...
     */
    for (l = prog->block[i].pos / a->blksize;
	l <= ((prog->block[i].pos+prog->block[i].length-1) / a->blksize);
	l++) {

      if (erased & (0x1 << l))
	continue;

      ptr = l * a->blksize;
      offset = ptr % a->unitsize;
      base = ptr - offset;

      base += a->address;
      ptr += a->address;
      j = 0;

flash_redo:

      printk("%s(%d): ERASE BLOCK sector=%d base=%x offset=%x ptr=%x\n",
	  __FILE__, __LINE__, l, (int) base, (int) offset, (int) ptr);

      /* Erase this sector */
      /* FIX: check which byte lane the value needs to be on */
      *((volatile unsigned short *) (base | (0x555 << 1))) = 0xaaaa;
      *((volatile unsigned short *) (base | (0x2aa << 1))) = 0x5555;
      *((volatile unsigned short *) (base | (0x555 << 1))) = 0x8080;
      *((volatile unsigned short *) (base | (0x555 << 1))) = 0xaaaa;
      *((volatile unsigned short *) (base | (0x2aa << 1))) = 0x5555;
      *((volatile unsigned short *) ptr) = 0x3030;

      for (k = 0; (k < FTIMEOUT); k++) {
	status = *((volatile unsigned short *) ptr);
	if ((status & 0x0080) || (status & 0x0020))
	  break;
      }
      if ((!(status & 0x0080) && (status & 0x0020)) || k >= FTIMEOUT) {
	printk("FLASH: (%d) erase failed\n", __LINE__);
	/* Reset FLASH unit */
	*((volatile unsigned char *) base) = 0xf0f0;
	failures++;
	/* Continue (with unerased sector) */
      }

      /*
       *  The 29LV800 part has a bunch of small segments at the bottom.
       *  These need to be erased individually, ugh...
       */
      if (a->length > flash_29lv800[0]) {
	if ((ptr & 0x1fffff) < 0x10000) {
	  ptr += flash_29lv800[j++];
	  goto flash_redo;
	}
      }

      erased |= (0x1 << l);
    }

    /*
     *	Program FLASH with the block data...
     */
    min = prog->block[i].pos+a->address;
    max = prog->block[i].pos+prog->block[i].length+a->address;
    w = (unsigned short *) prog->block[i].data;

    printk("%s(%d): PROGRAM BLOCK min=%x max=%x\n", __FILE__, __LINE__,
      (int) min, (int) max);

    for (ptr = min; (ptr < max); ptr += 2, w++) {

      offset = (ptr - a->address) % a->unitsize;
      base = ptr - offset;

      printk("%s(%d): PROGRAM base=%x offset=%x ptr=%x value=%x\n",
	  __FILE__, __LINE__, (int) base, (int) offset, (int) ptr, (int) *w);

      *((volatile unsigned short *) (base | (0x555 << 1))) = 0xaaaa;
      *((volatile unsigned short *) (base | (0x2aa << 1))) = 0x5555;
      *((volatile unsigned short *) (base | (0x555 << 1))) = 0xa0a0;
      *((volatile unsigned short *) ptr) = *w;

      for (j = 0; (j < FTIMEOUT); j++) {
	status = *((volatile unsigned short *) ptr);
	if (status == *w) {
	  /* Program complete */
	  break;
	}
      }

      status = *((volatile unsigned short *) ptr);
      if (status != *w) {
	printk("FLASH: (%d) write failed, addr=%x val=%x status=%x cnt=%d\n",
		__LINE__, (int) ptr, *w, status, j);
	/* Reset FLASH unit */
	*((volatile unsigned short *) ptr) = 0xf0f0;
	failures++;
      }
    }
  }

  if (failures > 0) {
    printk("FLASH: %d failures programming FLASH!\n", failures);
    return;
  }

  printk("\nFLASH: programming successful!\n");

  if (prog->reset) {
    printk("FLASH: rebooting...\n\n");
    HARD_RESET_NOW();
  }
}

#endif /* !CONFIG_FLASH8BIT */

/****************************************************************************/
/*                             INTEL FLASH                                  */
/****************************************************************************/

/*
 *	FLASH erase routine for the Intel Strata FLASH.
 */

void flash_intel_erase(struct arena_t *a, unsigned long pos)
{
  unsigned volatile char *address;
  unsigned long fbase = a->address;
  unsigned long flags;
  unsigned char status;
  int i;
  
#if 0
  printk("%s(%d): flash_intel_erase(a=%x,pos=%x)\n", __FILE__, __LINE__,
    (int) a, (int) pos);
#endif

  if (pos >= a->length)
    return;

  address = (volatile unsigned char *) (fbase + pos);

  /* Mutex all access to FLASH memory */
  down(&spare_lock);
  save_flags(flags); cli();

#if defined(CONFIG_WATCHDOG)
  watchdog_disable();
#endif

  /* Erase this sector */
  *address = 0x20;
  *address = 0xd0;

  for (i = 0; (i < FTIMEOUT); i++) {
    status = *address;
    if (status & 0x80)
      break;
  }

  /* Restore FLASH to normal read mode */
  *address = 0xff;

  if (*address != 0xff) {
     printk("FLASH: (%d): erase failed, address %p iteration=%d "
		"status=%x\n", __LINE__, address, i, (int) status);
  }

#if defined(CONFIG_WATCHDOG)
  watchdog_enable();
#endif

  restore_flags(flags);
  up(&spare_lock);
}


/*
 *	FLASH programming routine for Intel Strata FLASH.
 */

void flash_intel_write(struct arena_t * a, unsigned long pos, unsigned long length, char * buffer)
{
  unsigned long		flags, ptr, min, max;
  unsigned char		*lp, *lp0, status;
  int			failures, j, k, l;
  
#if 0
  printk("%s(%d): flash_intel_write(a=%x,pos=%x,length=%d,buf=%x)\n",
	__FILE__, __LINE__, (int) a, (int) pos, (int) length, (int) buffer);
#endif

  down(&spare_lock);

#if defined(CONFIG_WATCHDOG)
  watchdog_disable();
#endif

  min = (a->address + pos);
  max = min + length;
  lp = (unsigned char *) buffer;

  for (ptr = min; (ptr < max); ptr += l) {

      save_flags(flags); cli();

      /* Determine write size */
      lp0 = lp;
      j = max - ptr;
      l = (j < 32) ? j : 32;
      if ((ptr & ~0x1f) != ptr) {
	j = 32 - (ptr & 0x1f);
	l = (l < j) ? l : j;
      }

      /* Program next buffer bytes */
      for (j = 0; (j < FTIMEOUT); j++) {
	*((volatile unsigned char *) ptr) = 0xe8;
	status = *((volatile unsigned char *) ptr);
	if (status & 0x80)
	  break;
      }
      if ((status & 0x80) == 0)
	goto writealldone;

      *((volatile unsigned char *) ptr) = (l-1);
      for (j = 0; (j < l); j++)
	*((volatile unsigned char *) (ptr+j)) = *lp++;

      *((volatile unsigned char *) ptr) = 0xd0;

      for (j = 0; (j < FTIMEOUT); j++) {
	status = *((volatile unsigned char *) ptr);
	if (status & 0x80) {
	  /* Program complete */
	  break;
	}
      }

writealldone:
      /* Restore FLASH to normal read mode */
      *((volatile unsigned char *) ptr) = 0xff;

      for (k = 0; (k < l); k++, lp0++) {
      	status = *((volatile unsigned char *) (ptr+k));
        if (status != *lp0) {
		printk("FLASH: (%d): write failed, addr=%08x wrote=%02x "
		"read=%02x cnt=%d len=%d\n",
		__LINE__, (int) (ptr+k), (int) *lp0, (int) status, j, l);
		failures++;
	}
      }

      restore_flags(flags);
    }

#if defined(CONFIG_WATCHDOG)
  watchdog_enable();
#endif

  up(&spare_lock);
}


#if 0
/*
 *	Slow FLASH programming routine for Intel Strata FLASH.
 */
void flash_intel_writeslow(struct arena_t * a, unsigned long pos, unsigned long length, char * buffer)
{
  volatile unsigned char *address;
  unsigned long flags, fbase = a->address;
  unsigned char *lbuf, status;
  int i;

#if 0
  printk("%s(%d): flash_intel_write(a=%x,pos=%x,length=%d,buf=%x)\n",
	__FILE__, __LINE__, (int) a, (int) pos, (int) length, (int) buffer);
#endif

  down(&spare_lock);

#if defined(CONFIG_WATCHDOG)
  watchdog_disable();
#endif

  address = (unsigned volatile char *) (fbase + pos);
  lbuf = (unsigned char *) buffer;

  for (; (length > 0); length--, address++, lbuf++) {
  
    if (*address != *lbuf) {
      save_flags(flags); cli();

      *address = 0x40;
      *address = *lbuf;

      for (i = 0; (i < FTIMEOUT); i++) {
	status = *address;
	if (status & 0x80) {
	  /* Program complete */
	  break;
	}
      }

      /* Restore FLASH to normal read mode */
      *address = 0xff;

      if (*address != *lbuf) {
          printk("FLASH: (%d): write failed i=%d, address %p -> %x(%x)\n",
		__LINE__, i, address, (int) *lbuf, (int) *address);
      }

      restore_flags(flags);
    }
  }

#if defined(CONFIG_WATCHDOG)
  watchdog_enable();
#endif

  up(&spare_lock);
}
#endif


/*
 *	Program a complete FLASH image into an Intel Strata FLASH part.
 *	This runs from DRAM, so no need to worry about writing to what
 *	we are running from...
 */

void flash_intel_writeall(struct arena_t * a, struct blkmem_program_t * prog)
{
  unsigned long		erased[16];
  unsigned long		base, offset, ptr, min, max;
  unsigned char		*lp, *lp0, status;
  int			failures, i, j, k, l;

#if defined(CONFIG_WATCHDOG)
  watchdog_disable();
#endif

#if 0
  printk("FLASH: programming");
#endif
  failures = 0;
  memset(&erased[0], 0, sizeof(erased));

  cli();
  
  for (i = 0; (i < prog->blocks); i++) {

#if 0
    printk("%s(%d): block=%d address=%x pos=%x length=%x range=%x-%x\n",
      __FILE__, __LINE__, i, (int) a->address, (int) prog->block[i].pos,
      (int) prog->block[i].length, (int) (prog->block[i].pos + a->address),
      (int) (prog->block[i].pos + prog->block[i].length - 1 + a->address));
#endif

    /*
     *	Erase FLASH sectors for this program block...
     */
    for (l = prog->block[i].pos / a->blksize;
	l <= ((prog->block[i].pos+prog->block[i].length-1) / a->blksize);
	l++) {

      if (erased[(l / 32)] & (0x1 << (l % 32)))
	continue;

      ptr = l * a->blksize;
      offset = ptr % a->unitsize;
      base = ptr - offset;

      base += a->address;
      ptr += a->address;
      j = 0;

#if 0
      printk("%s(%d): ERASE BLOCK sector=%d ptr=%x\n",
	  __FILE__, __LINE__, l, (int) ptr);
#endif

      /* Erase this sector */
      *((volatile unsigned char *) ptr) = 0x20;
      *((volatile unsigned char *) ptr) = 0xd0;

      for (k = 0; (k < FTIMEOUT); k++) {
	status = *((volatile unsigned char *) ptr);
	if (status & 0x80) {
	  /* Erase complete */
	  status = *((volatile unsigned char *) ptr);
	  break;
	}
      }

      /* Restore FLASH to normal read mode */
      *((volatile unsigned char *) ptr) = 0xff;

      if (k >= FTIMEOUT) {
	  printk("FLASH: (%d) erase failed, status=%08x\n",
		__LINE__, (int) status);
	  failures++;
	  /* Continue (with unerased sector) */
	  break;
      }

      erased[(l / 32)] |= (0x1 << (l % 32));
    }

    /*
     *	Program FLASH with the block data...
     */
    min = prog->block[i].pos+a->address;
    max = prog->block[i].pos+prog->block[i].length+a->address;
    lp = (unsigned char *) prog->block[i].data;

#if 0
    printk("%s(%d): PROGRAM BLOCK min=%x max=%x\n", __FILE__, __LINE__,
      (int) min, (int) max);
#endif


    for (ptr = min; (ptr < max); ptr += l) {

      /* Determine write size */
      lp0 = lp;
      j = max - ptr;
      l = (j < 32) ? j : 32;
      if ((ptr & ~0x1f) != ptr) {
	j = 32 - (ptr & 0x1f);
	l = (l < j) ? l : j;
      }

#if 0
      printk("%s(%d): PROGRAM ptr=%x l=%d\n", __FILE__, __LINE__, (int) ptr, l);
#endif

      /* Program next buffer bytes */
      for (j = 0; (j < FTIMEOUT); j++) {
	*((volatile unsigned char *) ptr) = 0xe8;
	status = *((volatile unsigned char *) ptr);
	if (status & 0x80)
	  break;
      }
      if ((status & 0x80) == 0)
	goto intelwritealldone;

      *((volatile unsigned char *) ptr) = (l-1);
      for (j = 0; (j < l); j++)
	*((volatile unsigned char *) (ptr+j)) = *lp++;

      *((volatile unsigned char *) ptr) = 0xd0;

      for (j = 0; (j < FTIMEOUT); j++) {
	status = *((volatile unsigned char *) ptr);
	if (status & 0x80) {
	  /* Program complete */
	  break;
	}
      }

intelwritealldone:
      /* Restore FLASH to normal read mode */
      *((volatile unsigned char *) ptr) = 0xff;

      for (k = 0; (k < l); k++, lp0++) {
      	status = *((volatile unsigned char *) (ptr+k));
        if (status != *lp0) {
		printk("FLASH: (%d): write failed, addr=%08x wrote=%02x "
		"read=%02x cnt=%d len=%d\n",
		__LINE__, (int) (ptr+k), (int) *lp0, (int) status, j, l);
		failures++;
	}
      }
    }
  }

  if (failures > 0) {
    printk("FLASH: %d failures programming FLASH!\n", failures);
    return;
  }

#if 0
  printk("\nFLASH: programming successful!\n");
#endif
  if (prog->reset) {
    printk("FLASH: rebooting...\n\n");
    HARD_RESET_NOW();
  }
}

#endif /* CONFIG_FLASH_SNAPGEAR */

/****************************************************************************/
/*                          SHGLCORE FLASH                                  */
/****************************************************************************/

#ifdef CONFIG_SHGLCORE
static struct semaphore spare_lock = MUTEX;

void read_spare(struct arena_t * a, unsigned long pos, unsigned long length, char * buffer)
{

#ifdef DEBUG
  printk("rsl\n");
#endif

  /* Mutex all access to FLASH */
  down(&spare_lock);

#ifdef DEBUG
  printk("rsld\n");
#endif

  /* Just copy the data into target buffer */
  memcpy( buffer, (void*)(a->address+pos), length);

  /* Release MUTEX */
  up(&spare_lock);
  
#ifdef DEBUG
  printk("rsud\n");
#endif
}

void write_spare(struct arena_t * a, unsigned long pos, unsigned long length, char * buffer)
{
  unsigned long start;
  unsigned char c;
  volatile unsigned char * address;
  unsigned char result;
  unsigned long fbase = a->address;
  unsigned long flags;
  
#if 0
  for(i = pos / a->blksize; i <= ((pos+length-1) / a->blksize); i++) {
    if (test_bit(i, &a->auto_erase_bits)) {
      /* erase sector start */
      printk("Autoerase of sector %d\n", i);
      erase_spare(a, i * a->blksize);
      clear_bit(i, &a->auto_erase_bits);
    }
  }
#endif

#ifdef DEBUG
  printk("wsl\n");
#endif
  
  down(&spare_lock);
  
#ifdef DEBUG
  printk("wsld\n");
#endif
  
  start = jiffies;
  
  address = (unsigned volatile char*)(fbase+pos);
  
  while (length>0) {
  
    c = *buffer++;
    
    /*printk("Checking spare_flash program of byte %lx, at address %p, value %x (%c), current %x (%c)\n", pos, address, c, c, *address, *address);*/

    if (*address != c) {
  
      /*printk("Starting spare_flash program of byte %lx, at address %p\n", pos, address);*/
      
      
      if (c & ~*address) {
        printk("Unable to write byte at %p (impossible bit transition in %x, actual %x)\n", address, c, *address);
        /*continue;*/
      }

	save_flags(flags); cli();

      *(unsigned volatile char *)(fbase | 0x5555)=0x0aa;
      *(unsigned volatile char *)(fbase | 0x2aaa)=0x055;
      *(unsigned volatile char *)(fbase | 0x5555)=0x0a0;
      
      *address = c;
               
      for(;;) {
        result = *address;
        /*printk("Read value %x (%c)\n", result, result);*/
        if ((result & 0x80) == (c & 0x80))
          break;
        if (result & 0x20) {
          printk("timeout of FLASH write at address %p of value %x (actual %x)\n", address, c, *address);
          *(unsigned volatile char *)(fbase)=0x0f0; /* Reset */
          break;
        }
      }
      
      restore_flags(flags);

      /*printk("Completed spare_flash program of byte %lx, at address %p\n", pos, address);*/
        
#if 0
      if (jiffies != start) {
        /*printk("Spare_flash rescheduling in write\n");*/
        current->state = TASK_INTERRUPTIBLE;
        current->timeout = jiffies;
        schedule();
        current->timeout = 0;
        /*schedule();*/
        start = jiffies;
      }
#endif
    }

    address++;
    length--;
  }
  
  up(&spare_lock);
  
#ifdef DEBUG
  printk("wsud\n");
#endif
}

void erase_spare(struct arena_t * a, unsigned long pos)
{
  unsigned long fbase = a->address;
  int delay;
  unsigned volatile char * address;
  unsigned long flags;
  
  if (pos >= a->length)
    return;
  
  /* Mutex all access to FLASH memory */
  
#ifdef DEBUG
  printk("esl\n");
#endif
  
  down(&spare_lock);

#ifdef DEBUG
  printk("esld\n");
#endif

  address = (unsigned volatile char*)(fbase + pos);

  printk("Starting spare_flash erase of byte %lx, at address %p\n", pos, address);
  
  save_flags(flags); cli();

again:

  delay = HZ/4+1;
  
  /* Initiate erase of FLASH sector */
  
  *(unsigned volatile char *)(fbase | 0x5555)=0x0aa;
  *(unsigned volatile char *)(fbase | 0x2aaa)=0x055;
  *(unsigned volatile char *)(fbase | 0x5555)=0x080;
  *(unsigned volatile char *)(fbase | 0x5555)=0x0aa;
  *(unsigned volatile char *)(fbase | 0x2aaa)=0x055;
                       
  *address = 0x030;
  
  /* Delay until erase is complete */
     
  for (;;) {
    unsigned char result;
#ifdef original_spare_erase_delay
    struct wait_queue *wait = NULL;
#ifdef DEBUG
    printk("Spare_flash erase delaying for %d ticks, status is %x\n", delay, (unsigned int)*address);
#endif
    
    current->timeout = jiffies + delay;
#if 0    
    current->state = TASK_INTERRUPTIBLE;
    schedule();
    current->timeout = 0;
#endif
    interruptible_sleep_on(&wait);
#endif
    udelay(100000);
    
    result = *address;
    if (result & 0x80)
       break;
    if (result & 0x20) {
       printk("timeout of Spare_flash erase of address %p\n", address);
       *(unsigned volatile char *)(fbase)=0x0f0; /* Reset */
       printk("Sleeping a second and retrying\n");

       udelay(1000000);
       
       goto again;
    }
  }
  
  restore_flags(flags);

#ifdef DEBUG
  printk("Completed spare_flash erase of byte %lx, at address %p\n", pos, address);
#endif
  
  up(&spare_lock);

#ifdef DEBUG
  printk("esud\n");
#endif

}

#define VSP(X) (*(volatile unsigned short *)(X))
#define VSC(X) (*(volatile unsigned char *)(X))

#define SCSR  VSP(0xfffc0c)
#define SCSR_TDRE (1<<8)

#define SCDR  VSP(0xfffC0e)

#define print_char(x) ({			\
	while (!(SCSR & SCSR_TDRE))		\
		;				\
	SCDR = (x);				\
})

#define print_hexdigit(x) ({			\
	int digit = (x) & 0xf;			\
	if (digit>9)				\
		print_char('a'+digit-10);	\
	else					\
		print_char('0'+digit);		\
						\
})

#define print_num(x) ({				\
	unsigned long num = (x);		\
	print_hexdigit(num >> 28);		\
	print_hexdigit(num >> 24);		\
	print_hexdigit(num >> 20);		\
	print_hexdigit(num >> 16);		\
	print_hexdigit(num >> 12);		\
	print_hexdigit(num >> 8);		\
	print_hexdigit(num >> 4);		\
	print_hexdigit(num >> 0);		\
})

/* Note: sub_program_main must not reference _any_ data or code outside of itself,
   or leave interrupts enabled, due to the fact that it is probably erasing
   & reloading the kernel. */

#define SET_SHORT(x,y) VSP((x)) = (y)
/*#define SET_SHORT(x,y) ({})*/ /*print_char('>');print_num(x);*/ /*printk("%8.8lx <= %04x\n", (x), (y))*/

#define SET_CHAR(x,y) VSC((x)) = (y)
/*#define SET_CHAR(x,y) ({})*/ /*print_char('>');print_num(x);*/ /*printk("%8.8lx <= %02x\n", (x), (y))*/

#define GET_SHORT(x) VSP((x))
/*#define GET_SHORT(x) ({0;})*/ /*({print_char('<');print_num(x);0;})*/ /*(printk("%8.8lx => ....\n", (x)),0)*/

#define GET_CHAR(x) VSC((x))
/*#define GET_CHAR(x) ({0;})*/ /*({print_char('<');print_num(x);0;})*/ /*(printk("%8.8lx => ..\n", (x)),0)*/


void sub_program_main(struct arena_t * a, struct blkmem_program_t * prog)
{
  volatile int i,l;
  unsigned long base, offset, ptr, min, max;
  unsigned char * c;
  unsigned int erased = 0;
  int failures;
  int retry;
  
  cli();

  retry = 0;

again:

  SET_ALARM_LED(1);
  
  retry++;
  
  if (retry>5) {
  	goto give_up;
  }

    print_char('\r');
    print_char('\n');
    print_char('R');
    print_char('0' + retry);

  failures = 0;
  erased = 0;
  
/*  for(i=prog->blocks-1;i>=0;i--) {*/
  for(i=0;i<prog->blocks;i++) {

    SET_COMM_STATUS_LED(!GET_COMM_STATUS_LED());

    print_char('\r');
    print_char('\n');
    print_num(prog->block[i].pos+a->address);
    print_char('-');
    print_num(prog->block[i].pos+prog->block[i].length-1+a->address);
    print_char('\r');
    print_char('\n');

    if(prog->block[i].length > 0xE0000)
      break;

    for(l=prog->block[i].pos / a->blksize; l <= ((prog->block[i].pos+prog->block[i].length-1) / a->blksize); l++) {
      if (!test_bit(l, &erased)) {
        
 	print_char('E');
 	print_char('0' + l / 10);
 	print_char('0' + l % 10);
 	print_char('\r');
 	print_char('\n');
 	
 	if (l <  1)
 	  break;
 	/*if (l >= 8)
 	  break;*/

	ptr = l * a->blksize;
	offset = ptr % a->unitsize;
	base = ptr - offset;
	
	base += a->address;
	ptr += a->address;
	
	print_char('b');
	print_char('a');
	print_char('s');
	print_char('e');
	print_char(' ');
	print_num(base);
	print_char('\r');
	print_char('\n');
	print_char('o');
	print_char('f');
	print_char('f');
	print_char(' ');
	print_num(offset);
	print_char('\r');
	print_char('\n');
	print_char('p');
	print_char('t');
	print_char('r');
	print_char(' ');
	print_num(ptr);
	print_char('\r');
	print_char('\n');

        set_bit(l, &erased);

        if (ptr <  0x020000)
          break;
        /*if (ptr >= 0x100000)
          break;*/
        
        print_num(ptr);
        
 	SET_COMM_ERROR_LED(1);
 	
        /* Erase even half of sector */
        SET_SHORT( (base | (0x5555 << 1)), 0xaa00);
        SET_SHORT( (base | (0x2aaa << 1)), 0x5500);
        SET_SHORT( (base | (0x5555 << 1)), 0x8000);
        SET_SHORT( (base | (0x5555 << 1)), 0xaa00);
        SET_SHORT( (base | (0x2aaa << 1)), 0x5500);

        SET_SHORT( ptr, 0x3000);
#ifdef original_erase_logic
        while (!(GET_SHORT(ptr) & 0x8000))
          ;
#else
	for (;;) {
		unsigned int status = GET_SHORT(ptr);
		if (status & 0x8000) {
			/* Erase complete */
			break;
		}
		if (status & 0x2000) {
			/* Check again */
			status = GET_SHORT(ptr);
			if (status & 0x8000) {
				/* Erase complete */
				break;
			}
			
			/* Erase failed */
			print_char('F');
			
			/* Reset FLASH unit */
			SET_SHORT( base, 0xf000);
			
			failures++;
			
			/* Continue (with unerased sector) */
			break;
		}
        }
#endif

        print_char(':');

        /* Erase odd half of sector */
        SET_SHORT( (base | (0x5555 << 1)), 0x00aa);
        SET_SHORT( (base | (0x2aaa << 1)), 0x0055);
        SET_SHORT( (base | (0x5555 << 1)), 0x0080);
        SET_SHORT( (base | (0x5555 << 1)), 0x00aa);
        SET_SHORT( (base | (0x2aaa << 1)), 0x0055);

        SET_SHORT( ptr, 0x0030);
#ifdef original_erase_logic
        while (!(GET_SHORT(ptr) & 0x0080))
          ;
#else
	for (;;) {
		unsigned int status = GET_SHORT(ptr);
		if (status & 0x0080) {
			/* Erase complete */
			break;
		}
		if (status & 0x0020) {
		
			/* Check again */			
			status = GET_SHORT(ptr);
			if (status & 0x0080) {
				/* Erase complete */
				break;
			}

			/* Erase failed */
			print_char('F');
			
			/* Reset FLASH unit */
			SET_SHORT( base, 0x00f0);
			
			failures++;
			
			/* Continue (with unerased sector) */
			break;
		}
        }
#endif

        print_char(':');
          
#if 0
        probe = (volatile unsigned short*)(fbase + a->blksize * l);
        *probe = 0x3000;
        while (!(*probe & 0x8000))
          ;
          
        print_char('.');
        
        /* Erase odd half of sector */
        *(unsigned volatile short *)(fbase | (0x5555 << 1))=0x00aa;
        *(unsigned volatile short *)(fbase | (0x2aaa << 1))=0x0055;
        *(unsigned volatile short *)(fbase | (0x5555 << 1))=0x0080;
        *(unsigned volatile short *)(fbase | (0x5555 << 1))=0x00aa;
        *(unsigned volatile short *)(fbase | (0x2aaa << 1))=0x0055;

        probe = (volatile unsigned short*)(fbase + a->blksize * l);
        *probe = 0x0030;
        while (!(*probe & 0x0080))
          break;
          
        print_char('.');
#endif

 	SET_COMM_ERROR_LED(0);
      }
    }
    
    
    min = prog->block[i].pos+a->address;
    max = prog->block[i].pos+prog->block[i].length+a->address;
    for(ptr=min, c=prog->block[i].data; ptr<max; ptr++, c++) {
      
      offset = (ptr-a->address) % a->unitsize;
      base = ptr - offset;

      if (ptr <  0x020000)
        break;
      /*if (ptr >= 0x100000)
        break;*/

      /*print_char('.');*/
      
#if 0
      if ((fbase & 1) == 0) { /* Even bank */
        *(unsigned volatile short *)(fbase | (0x5555 << 1))=0xaa00;
        *(unsigned volatile short *)(fbase | (0x2aaa << 1))=0x5500;
        *(unsigned volatile short *)(fbase | (0x5555 << 1))=0xa000;
        *(unsigned volatile short *)(fbase + (b & ~1))     =*c << 8;
      } else { /* Odd bank */
        *(unsigned volatile short *)(fbase | (0x5555 << 1))=0x00aa;
        *(unsigned volatile short *)(fbase | (0x2aaa << 1))=0x0055;
        *(unsigned volatile short *)(fbase | (0x5555 << 1))=0x00a0;
        *(unsigned volatile short *)(fbase + (b & ~1))     =*c;
      }
            
      probe = (volatile unsigned char*)(fbase + b);
      while (*probe != *c)
          break;
#endif

      if ((ptr & 1) == 0) { /* Even bank */
        SET_SHORT( (base | (0x5555 << 1)), 0xaa00);
        SET_SHORT( (base | (0x2aaa << 1)), 0x5500);
        SET_SHORT( (base | (0x5555 << 1)), 0xa000);
        SET_SHORT( (ptr & ~1),      *c << 8);
      } else { /* Odd bank */
        SET_SHORT( (base | (0x5555 << 1)), 0x00aa);
        SET_SHORT( (base | (0x2aaa << 1)), 0x0055);
        SET_SHORT( (base | (0x5555 << 1)), 0x00a0);
        SET_SHORT( (ptr & ~1),      *c);
      }
      
#ifdef original_write_logic
      while (GET_CHAR(ptr) != *c)
        ;
#else
	for (;;) {
		unsigned char status = GET_CHAR(ptr);
		if ((status & 0x80) == (*c & 0x80)) {
			/* Program complete */
			break;
		}
		if (status & 0x20) {
			/* Check again */
			status = GET_CHAR(ptr);
			if ((status & 0x80) == (*c & 0x80)) {
				/* Program complete */
				break;
			}
			
			/* Program failed */
			print_char('F');
			
			/* Reset FLASH unit */
			if ((ptr & 1) == 0) { /* Even bank */
				SET_SHORT( base, 0xf000);
			} else { /* Odd bank */
				SET_SHORT( base, 0x00f0);
			}
			
			failures++;
			
			/* Continue */
			break;
		}
        }
#endif
      
      /*print_char(' ');*/
    }
  }
  
  if (failures > 0) {
  	/* There were failures erasing the FLASH, so go back to the beginning and
  	try it all again -- for lack of anything better to do. */
  	
  	print_char('!');
  	goto again;
  }

give_up:

  SET_ALARM_LED(1);
  
  
  HARD_RESET_NOW();
  
  i = 1;
  while(i)
    ;
  
}

void program_main(struct arena_t * a, struct blkmem_program_t * prog)
{
  int len;
  void (*code)(struct arena_t*, struct blkmem_program_t *);
  
  printk("program_main entered, blocks = %d\n", prog->blocks);
  
  len = &program_main-&sub_program_main;
  code = kmalloc(len, GFP_KERNEL);
  memcpy(code, &sub_program_main, len);
  
  code(a, prog);

  kfree(code);
  
  /*sub_program_main(a, prog);*/
}
#endif /* CONFIG_SHGLCORE */

/****************************************************************************/

int general_program_func(struct inode * inode, struct file * file, struct arena_t * a, struct blkmem_program_t * prog)
{
  int i,block;
  unsigned int erased = 0;

  /* Mandatory flush of all dirty buffers */
  fsync_dev(inode->i_rdev);
  invalidate_buffers(inode->i_rdev);

  for(i=0;i<prog->blocks;i++) {
    int min= prog->block[i].pos / a->blksize;
    int max = (prog->block[i].pos + prog->block[i].length - 1) / a->blksize;
    for(block=min; block <= max; block++) {
      if (!test_bit(block, &erased)) {
	printk("Erase of sector at pos %lx of arena %d (address %p)\n", block * a->blksize, MINOR(inode->i_rdev), (void*)(a->address+block*a->blksize));
    
        /* Invoke erase function */
        a->erase_func(a, block * a->blksize);
        set_bit(block, &erased);
      }
    }
    
    printk("Write of %lu bytes at pos %lu (data at %p)\n", prog->block[i].length, prog->block[i].pos, prog->block[i].data);
    
    a->write_func(a, prog->block[i].pos, prog->block[i].length, prog->block[i].data);
    
    schedule();
  }

  if (prog->reset)
  	HARD_RESET_NOW();
  return 0;
}


static void complete_request(void * data);

static struct tq_struct complete_tq = {0, 0, complete_request, NULL};

static void
delay_request( void )
{
  queue_task(&complete_tq, &tq_scheduler);
}

static void
complete_request( void * data)
{
  unsigned long start;
  unsigned long len;
  struct arena_t * a = arena + DEVICE_NR(CURRENT_DEV);

  for(;;) {
  
  /* sectors are 512 bytes */
  start = CURRENT->sector << 9;
  len  = CURRENT->current_nr_sectors << 9;

  /*printk("blkmem: re-request %d\n", CURRENT->cmd);*/
    
  if ( CURRENT->cmd == READ ) {
    /*printk("BMre-Read: %lx:%lx > %p\n", a->address + start, len, CURRENT->buffer);*/
    a->read_func(a, start, len, CURRENT->buffer);
  }
  else if (CURRENT->cmd == WRITE) {
    /*printk("BMre-Write: %p > %lx:%lx\n", CURRENT->buffer, a->address + start, len);*/
    a->write_func(a, start, len, CURRENT->buffer);
  }
  /*printk("ending blkmem request\n");*/
  end_request( TRUE );
  
  INIT_REQUEST;
  }
  
#if 0
  if (CURRENT)
    do_blkmem_request(); /* Process subsequent requests */
#endif
}


static void
do_blkmem_request( void )
{
  unsigned long start;
  unsigned long len;
  struct arena_t * a = arena + DEVICE_NR(CURRENT_DEV);
#if 0
  printk( KERN_ERR DEVICE_NAME ": request\n");
#endif
  while ( TRUE ) {
    INIT_REQUEST;
    
    /* sectors are 512 bytes */
    start = CURRENT->sector << 9;
    len  = CURRENT->current_nr_sectors << 9;

    if ((start + len) > a->length) {
      printk( KERN_ERR DEVICE_NAME ": bad access: block=%ld, count=%ld (pos=%lx, len=%lx)\n",
	      CURRENT->sector,
	      CURRENT->current_nr_sectors,
	      start+len,
	      a->length);
      end_request( FALSE );
      continue;
    }

    /*printk("blkmem: request %d\n", CURRENT->cmd);*/
    
    if ( ( CURRENT->cmd != READ ) 
	 && ( CURRENT->cmd != WRITE ) 
	 ) {
      printk( KERN_ERR DEVICE_NAME ": bad command: %d\n", CURRENT->cmd );
      end_request( FALSE );
      continue;
    }
    
    if ( CURRENT->cmd == READ ) {
      /*printk("BMRead %d, pid %d: %lx:%lx > %p\n", DEVICE_NR(CURRENT_DEV), current->pid, a->address + start, len, CURRENT->buffer);*/
      if (a->read_func) {
        delay_request();
        return;
        /*a->read_func(a, start, len, CURRENT->buffer);*/
      }
      else
        memcpy( CURRENT->buffer, (void*)(a->address + start), len );
    }
    else if (CURRENT->cmd == WRITE) {
      /*printk("BMWrite %d: %p > %lx:%lx\n", DEVICE_NR(CURRENT_DEV), CURRENT->buffer, a->address + start, len);*/
      if (a->write_func) {
        delay_request();
        return;
        /*a->write_func(a, start, len, CURRENT->buffer);*/
      } else
        memcpy( (void*)(a->address + start), CURRENT->buffer, len );
    }
    /*printk("ending blkmem request\n");*/
    end_request( TRUE );
  }
}


#ifdef MAGIC_ROM_PTR
static int
blkmem_romptr( struct inode *inode, struct file *filp, struct vm_area_struct * vma)
{
  struct arena_t * a = arena + MINOR(inode->i_rdev);
  
  if (a->read_func)
    return -ENOSYS; /* Can't do it, as this arena isn't in the main address space */

  vma->vm_start = a->address + vma->vm_offset;
  return 0;
}
#else
static int
blkmem_romptr( struct inode *inode, struct file *filp, struct vm_area_struct * vma)
{
  return 0;
}
#endif

static int blkmem_ioctl (struct inode *inode, struct file *file,
                         unsigned int cmd, unsigned long arg)
{
  struct arena_t * a = arena + MINOR(inode->i_rdev);
  int err;

  switch (cmd) {
  case BMGETSIZES:   /* Return device size in sectors */
  
    if (!arg)
      return -EINVAL;
    
    err = verify_area(VERIFY_WRITE, (unsigned long *) arg, sizeof(unsigned long));
    if (err)
      return err;

    put_user(a->blksize ? (a->length / a->blksize) : 0, (unsigned long *) arg);
    
    break;

  case BMGETSIZEB:   /* Return device size in bytes */
  
    if (!arg)
      return -EINVAL;
    
    err = verify_area(VERIFY_WRITE, (unsigned long *) arg, sizeof(unsigned long));
    if (err)
      return err;

    put_user(a->length, (unsigned long *) arg);
    
    break;

  case BMSERASE:
    if (a->erase_func) {

#if 0
      printk("Erase of sector at pos %lx of arena %d (address %p)\n", arg, MINOR(inode->i_rdev), (void*)(a->address+arg));
#endif
      if (arg >= a->length)
        return -EINVAL;
    
      /* Mandatory flush of all dirty buffers */
      fsync_dev(inode->i_rdev);
      
      /* Invalidate any buffers pointing to the section that will be erased */
      invalidate_buffers_by_byte(inode->i_rdev, arg * a->blksize, a->blksize);

      /* Invoke erase function */
      a->erase_func(a, arg);
    } else
      return -EINVAL;
    break;
    
  case BMSGSIZE:
  
    if (!arg)
      return -EINVAL;
  
    err = verify_area(VERIFY_WRITE, (unsigned long*)arg, sizeof(unsigned long));
    if (err)
      return err;
      
    put_user(a->blksize, (unsigned long*)arg);
    break;

  case BMSGERASEVALUE:
    err = verify_area(VERIFY_WRITE, (unsigned char*)arg, sizeof(unsigned char));
    if (err)
      return err;
      
    *(unsigned char*)arg = a->erasevalue;
    break;

  case BMPROGRAM:
  {
    struct blkmem_program_t * prog;
    int i;
    if (!arg)
      return -EINVAL;
  
    prog = (struct blkmem_program_t*)arg;
    
    /* Extensive checks to verify that the programming data makes sense */
    
    err = verify_area(VERIFY_READ, prog, sizeof(struct blkmem_program_t));
    if (err)
      return err;

    if ((prog->magic1 != BMPROGRAM_MAGIC_1) ||
        (prog->magic2 != BMPROGRAM_MAGIC_2))
      return -EINVAL;

    err = verify_area(VERIFY_READ, prog, sizeof(struct blkmem_program_t) + sizeof(prog->block[0]) * prog->blocks);
    if (err)
      return err;

    for(i=0;i<prog->blocks;i++)
      if(prog->block[i].magic3 != BMPROGRAM_MAGIC_3)
        return -EINVAL;

    for(i=0;i<prog->blocks;i++)
      if ((prog->block[i].pos > a->length) ||
          ((prog->block[i].pos+prog->block[i].length-1) > a->length))
        return -EINVAL;

    for(i=0;i<prog->blocks;i++) {
      err = verify_area(VERIFY_READ, prog->block[i].data, prog->block[i].length);
      if (err)
        return err;
    }

    if (a->program_func) {
      a->program_func(a, prog);
    } else {
      return general_program_func(inode, file, a, prog);
    }

    break;
  }

  default:
    return -EINVAL;
  }
  return 0;
}

static int
blkmem_open( struct inode *inode, struct file *filp )
{
  int device;
  struct arena_t * a;

  device = DEVICE_NR( inode->i_rdev );
#ifdef DEBUG
  printk( KERN_ERR DEVICE_NAME ": open: %d\n", device );
  printk( "MINOR: %d\tarenas: %d\n", MINOR(device), arenas);
#endif

  if ((MINOR(device) < 0) || (MINOR(device) >= arenas))  {
    printk("arena open of %d failed!\n", MINOR(device));
    return -ENODEV;
  }
  
  a = &arena[MINOR(device)];

#ifdef DEBUG
  printk("Open of blkmem arena %d at %lx, length %lx\n", MINOR(device), a->address, a->length);
#endif

#if defined(MODULE)
  MOD_INC_USE_COUNT;
#endif
  return 0;
}

static void 
blkmem_release( struct inode *inode, struct file *filp )
{
#if 0
  printk( KERN_ERR DEVICE_NAME ": release: %d\n", current_device );
#endif

  fsync_dev( inode->i_rdev );

#if defined(MODULE)
  MOD_DEC_USE_COUNT;
#endif

  return;
}

static struct file_operations blkmem_fops =
{
  NULL,                   /* lseek - default */
  block_read,             /* read - general block-dev read */
  block_write,            /* write - general block-dev write */
  NULL,                   /* readdir - bad */
  NULL,                   /* poll */
  blkmem_ioctl,           /* ioctl */
  NULL,
  blkmem_open,            /* open */
  blkmem_release,         /* release */
  block_fsync,            /* fsync */
  NULL,			  /* fasync */
  NULL,			  /* check media change */
  NULL,			  /* revalidate */
#ifdef MAGIC_ROM_PTR
  blkmem_romptr,          /* romptr */
#else
  NULL
#endif
};

int
blkmem_init( void )
{
  int i;
  if ( register_blkdev( MAJOR_NR, DEVICE_NAME, &blkmem_fops )) {
    printk( KERN_ERR DEVICE_NAME ": Unable to get major %d\n",
            MAJOR_NR );
    return -EBUSY;
  }

#ifdef FIXUP_ARENAS
  {
  FIXUP_ARENAS
  }
#endif

  for(i=0;i<arenas;i++) {
    if (arena[i].length == -1) {
      /* Since genromfs fills the header in with network order longs, we need to read it as such. */
      arena[i].length = ntohl(*(volatile unsigned long *)(arena[i].address + 8));
    }
    blkmem_blocksizes[i] = 1024;
    blkmem_sizes[i] = (arena[i].length + (1 << 10) - 1) >> 10; /* Round up */
    arena[i].length = blkmem_sizes[i] << 10;
  }

  printk("Blkmem copyright 1998,1999 D. Jeff Dionne\n"
	 "Blkmem copyright 1998 Kenneth Albanowski\n"
	 "Blkmem %d disk images:\n", (int) arenas);

  for(i=0;i<arenas;i++) {
    printk("%d: %lX-%lX (%s)\n", i,
	arena[i].address, arena[i].address+arena[i].length-1,
    	arena[i].rw 
    		? "RW"
    		: "RO"
    );
  }

  read_ahead[ MAJOR_NR ] = 0;
  blk_dev[ MAJOR_NR ].request_fn = DEVICE_REQUEST;
  blksize_size[ MAJOR_NR ] = blkmem_blocksizes;
  blk_size[ MAJOR_NR ] = blkmem_sizes;
  
#ifdef ROOT_ARENA
  ROOT_DEV = MKDEV(MAJOR_NR,ROOT_ARENA);
#endif

  return 0;
}

#if defined(MODULE)

int
init_module( void )
{
  int error;
    
  error = blkmem_init();
  if ( error == 0 ) {
    printk( KERN_INFO DEVICE_NAME ": loaded as module\n" );
  }
    
  return error;
}

void
cleanup_module( void )
{
  if ( unregister_blkdev( MAJOR_NR, DEVICE_NAME ) != 0 )
    printk( KERN_ERR DEVICE_NAME ": unregister of device failed\n");

  if ( current_device != -1 ) {
    /* free whatever we resources remain here */
  }

  return;
}
#endif
