/*
 * arch/armnommu/kernel/setup.c
 *
 * based on:
 *
 *   linux/arch/arm/kernel/setup.c
 *
 *   Copyright (C) 1995-1998 Russell King
 *
 *   This file obtains various parameters about the system that the kernel
 *   is running on.
 *
 * NET+ARM specific modification subejct to:
 *
 * Copyright (C) 2000, 2001 NETsilicon, Inc.
 * Copyright (C) 2000, 2001 Red Hat, Inc.
 *
 * This software is copyrighted by Red Hat. LICENSEE agrees that
 * it will not delete this copyright notice, trademarks or protective
 * notices from any copy made by LICENSEE.
 *
 * This software is provided "AS-IS" and any express or implied 
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall Red Hat
 * be liable for any indirect, consequential, or incidental damages,
 * loss of profits or revenue, loss of use or data, or interruption
 * of business, whether the alleged damages are labeled in contract,
 * tort, or indemnity.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author(s) : Joe deBlaquiere
 */

#include <linux/config.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/stddef.h>
#include <linux/unistd.h>
#include <linux/ptrace.h>
#include <linux/malloc.h>
#include <linux/user.h>
#include <linux/a.out.h>
#include <linux/tty.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/major.h>
#include <linux/utsname.h>
#include <linux/blk.h>

#include <asm/segment.h>
#include <asm/system.h>
#include <asm/hardware.h>
#include <asm/pgtable.h>
#include <asm/arch/mmu.h>
#include <asm/procinfo.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <asm/byteorder.h>

#ifdef	CONFIG_ARCH_NETARM
#include <asm/arch/netarm_mem_module.h>
#include <asm/arch/netarm_mmap.h>
#include <asm/arch/netarm_nvram.h>

#define NA_SAL_STATION_ADDR_1  0xFF8005C4 // SAL Station Address Register
#define NA_SAL_STATION_ADDR_2  0xFF8005C8
#define NA_SAL_STATION_ADDR_3  0xFF8005CC
extern void _netarm_led_blink(void);
extern void netarm_console_print(const char *b);
void	netarm_dump_hex8(unsigned long int val);
#endif

struct drive_info_struct { char dummy[32]; } drive_info;
struct screen_info screen_info;
struct processor processor;
char init_kernel_stack[4096];

extern const struct processor arm2_processor_functions;
extern const struct processor arm250_processor_functions;
extern const struct processor arm3_processor_functions;
extern const struct processor arm6_processor_functions;
extern const struct processor arm7_processor_functions;
extern const struct processor sa110_processor_functions;

struct armversions armidlist[] = {
#if defined(CONFIG_CPU_ARM2) || defined(CONFIG_CPU_ARM3)
	{ 0x41560200, 0xfffffff0, F_MEMC	, "ARM/VLSI",	"arm2"		, &arm2_processor_functions   },
	{ 0x41560250, 0xfffffff0, F_MEMC	, "ARM/VLSI",	"arm250"	, &arm250_processor_functions },
	{ 0x41560300, 0xfffffff0, F_MEMC|F_CACHE, "ARM/VLSI",	"arm3"		, &arm3_processor_functions   },
#endif
#if defined(CONFIG_CPU_ARM6) || defined(CONFIG_CPU_SA110)
	{ 0x41560600, 0xfffffff0, F_MMU|F_32BIT	, "ARM/VLSI",	"arm6"		, &arm6_processor_functions   },
	{ 0x41560610, 0xfffffff0, F_MMU|F_32BIT	, "ARM/VLSI",	"arm610"	, &arm6_processor_functions   },
	{ 0x41007000, 0xffffff00, F_MMU|F_32BIT , "ARM/VLSI",   "arm7"		, &arm7_processor_functions   },
	/* ARM710 does not follow the spec */
	{ 0x41007100, 0xfff8ff00, F_MMU|F_32BIT , "ARM/VLSI",   "arm710"	, &arm7_processor_functions   },
	{ 0x4401a100, 0xfffffff0, F_MMU|F_32BIT	, "DEC",	"sa110"		, &sa110_processor_functions  },
#endif
#if defined(CONFIG_CPU_ARM7)
	{ 0x41007000, 0xffffff00, F_32BIT , "ARM/Atmel",   "arm7"		, &arm7_processor_functions   },
	{ 0x41007100, 0xffffff00, F_32BIT , "ARM/APLIO",   "arm7"		, &arm7_processor_functions   },
	{ 0x41007200, 0xffffff00, F_32BIT , "ARM/NETsilicon",   "arm7"		, &arm7_processor_functions   },
	{ 0x41007300, 0xffffff00, F_32BIT , "ARM/GBA",   "arm7"	 		, &arm7_processor_functions   },
#endif
	{ 0x00000000, 0x00000000, 0		, "***",	"*unknown*"	, NULL }
	
};

#if defined( CONFIG_ARCH_TRIO) 

static struct param_struct *params = (struct param_struct *) PARAMS_BASE;

u_long trio_romdisk_addr = 3*1024*1024;

#elif	defined(CONFIG_ARCH_NETARM)
struct param_struct netarm_params = { { {
	4096,					// page_size
#ifdef	CONFIG_NETARM_NET40_REV2
	(16*1024*1024)/4096,			// nr_pages
#else
	(32*1024*1024)/4096,			// nr_pages
#endif
	0,					// ramdisk_size	
	(FLAG_RDLOAD),					// flags

#ifdef	CONFIG_BLK_DEV_RAMDISK_BLKMEM
	MKDEV(BLKMEM_MAJOR,0),			// rootdev
#else
	MKDEV(JFFS_MAJOR,18),
#endif
	
	80,					// video_num_cols
	24,					// video_num_rows
	0,					// video_x
	0,					// video_y
	0,					// bytes_per_char_h
	0,					// bytes_per_char_v

	0,  					// initrd_start
	0,					// initrd_size

#if 0
	/* hardwired ramdisk address */
	0x89000	/ BLOCK_SIZE			// rd_start
#else
	0					// rd_start
#endif

}
}
};

static struct param_struct *params = &netarm_params;
#else
struct param_struct common_params = { { {
	PAGE_SIZE,		// page_size
	DRAM_SIZE/PAGE_SIZE,	// nr_pages
	0,			// ramdisk_size	
	0,			// flags
	0,			// rootdev
	80,			// video_num_cols
	24,			// video_num_rows
	0,			// video_x
	0,			// video_y
	0,			// bytes_per_char_h
	0,			// bytes_per_char_v
	0,			// initrd_start
	0,
	0
} } };

static struct param_struct *params = &common_params;

#endif

unsigned long arm_id;
unsigned int vram_half_sam;
int armidindex;
int ioebpresent;
int memc_ctrl_reg;
int number_ide_drives;
int number_mfm_drives;

extern int bytes_per_char_h;
extern int bytes_per_char_v;
extern int root_mountflags;
extern int _etext, _edata, _end;
extern unsigned long real_end_mem;

#ifdef CONFIG_ARCH_ATMEL
extern int atmel_console_initialized;
extern void rs_atmel_print(const char*);
#endif

/*
 * ram disk
 */
#ifdef CONFIG_BLK_DEV_RAM
extern int rd_doload;		/* 1 = load ramdisk, 0 = don't load */
extern int rd_prompt;		/* 1 = prompt for ramdisk, 0 = don't prompt */
extern int rd_image_start;	/* starting block # of image */


static void setup_ramdisk (struct param_struct *params)
{
#ifdef	CONFIG_BLK_DEV_RAMDISK_BLKMEM
#ifdef	CONFIG_BLK_DEV_RAMDISK_COMPILED_IN
#if 0
	rd_image_start	= params->u1.s.rd_start;
#else
	extern void __ramdisk_data;

	rd_image_start	= ((int)&__ramdisk_data) / BLOCK_SIZE ;
#endif
#endif
#endif
	rd_prompt	= ((params->u1.s.flags & FLAG_RDPROMPT) == 0) ? 0 : 1 ;
	rd_doload	= ((params->u1.s.flags & FLAG_RDLOAD) == 0) ? 0 : 1 ;
}
#else
#define setup_ramdisk(p)
#endif

/*
 * initial ram disk
 */
#ifdef CONFIG_BLK_DEV_INITRD
static void setup_initrd (struct param_struct *params, unsigned long memory_end)
{
	ROOT_DEV = MKDEV(RAMDISK_MAJOR,0);

	initrd_start = params->u1.s.initrd_start;
	initrd_end   = params->u1.s.initrd_start + params->u1.s.initrd_size;

	if (initrd_end > memory_end) {
		printk ("initrd extends beyond end of memory "
			"(0x%08lx > 0x%08lx) - disabling initrd\n",
			initrd_end, memory_end);
		initrd_start = 0;
	}
}
#else
#define setup_initrd(p,m)
#endif

static inline void check_ioeb_present(void)
{
#ifndef CONFIG_ARCH_TRIO
#ifndef CONFIG_ARCH_ATMEL
#ifndef CONFIG_ARCH_NETARM
#ifndef CONFIG_ARCH_GBA
	if (((*IOEB_BASE) & 15) == 5)
		armidlist[armidindex].features |= F_IOEB;
#endif
#endif
#endif
#endif
}

static void get_processor_type (void)
{
	int i;

	for (armidindex = 0; ; armidindex ++)
		if (!((armidlist[armidindex].id ^ arm_id) &
		      armidlist[armidindex].mask))
			break;

	if (armidlist[armidindex].id == 0) {
#ifndef CONFIG_ARCH_NETARM
#ifndef CONFIG_ARCH_ATMEL
#ifndef CONFIG_ARCH_TRIO
#ifndef CONFIG_ARCH_GBA
		for (i = 0; i < 3200; i++)
			((unsigned long *)SCREEN2_BASE)[i] = 0x77113322;
#endif
#endif
#endif
#endif

		while (1);
	}

	processor = *armidlist[armidindex].proc;

#ifdef CONFIG_ARCH_ATMEL

	switch((*((unsigned long *)SF_CHIP_ID)>>28)&0x7) {
		    case 1:
			i='F'; break;
		    case 2:
			i='C'; break;
		    case 3:
			i='S'; break;
		    case 4:
			i='R'; break;
		    default:
			i='M'; break;
	}
	printk("Found an Atmel AT91%c40xxx %s processor\n", // with %ldk kernel stack\n", 
		i,
		armidlist[armidindex].name
		//, ((*((unsigned long *)SF_CHIP_ID)>>16) & 0xf)
		);
	printk("Atmel AT91 series microcontroller support (c) 2000,2001 Lineo Inc.\n");
#else
	printk("Found processor [%s] [%s]\n", armidlist[armidindex].name, armidlist[armidindex].manu);
#endif
	
}

#define COMMAND_LINE_SIZE 256

#ifdef	CONFIG_ARCH_ATMEL
static char command_line[COMMAND_LINE_SIZE] = "root=/dev/rom0"; /* { 0, }; */
#else
static char command_line[COMMAND_LINE_SIZE] = { 0, };
#endif
       char saved_command_line[COMMAND_LINE_SIZE];

#ifdef CONFIG_NETARM_EEPROM
#ifdef	CONFIG_ETHER_NETARM
static int parse_decimal(char ch)
{
	if ('0' >  ch) return -1;
	if ('9' >= ch) return ch - '0' ;
	return -1;
}

static unsigned int compute_checksum(NA_dev_board_params_t *pParams)
{
	int i;
	unsigned int *pInt = (unsigned int *)pParams ;
	unsigned int sum = 0;
	for ( i = 0 ; i < NETARM_NVRAM_DWORD_COUNT ; i++ )
	{
		sum += pInt[i];
	}
	
	if ( sum != 0 ) return -1 ;
	return 0 ;
}
#endif
#endif

void setup_arch(char **cmdline_p,
	unsigned long * memory_start_p, unsigned long * memory_end_p)
{
	static unsigned char smptrap = 0;
	unsigned long memory_start, memory_end;
	char c = ' ', *to = command_line, *from;
	int len = 0;

	if (smptrap == 1)
		return;
	smptrap = 1;

#if CONFIG_GBATXT
	{
		extern void register_console(void (*proc)(const char *));
		extern void gbatxt_console_print(const char * b);
		extern void gbatxt_console_init(void);

		gbatxt_console_init();
		register_console(gbatxt_console_print);
		printk("uClinux/GBA\n");
		printk("GameBoyAdvance port by Greg Ungerer (gerg@napgearcom)\n");
	}
#endif

	get_processor_type ();
	check_ioeb_present ();
	processor._proc_init ();

#ifdef CONFIG_CONSOLE	
	bytes_per_char_h  = params->u1.s.bytes_per_char_h;
	bytes_per_char_v  = params->u1.s.bytes_per_char_v;
#endif
	from		  = params->commandline;
	ROOT_DEV	  = to_kdev_t (params->u1.s.rootdev);
	ORIG_X		  = params->u1.s.video_x;
	ORIG_Y		  = params->u1.s.video_y;
	ORIG_VIDEO_COLS	  = params->u1.s.video_num_cols;
	ORIG_VIDEO_LINES  = params->u1.s.video_num_rows;

	setup_ramdisk (params);

	if (!(params->u1.s.flags & FLAG_READONLY))
		root_mountflags &= ~MS_RDONLY;

	/* If the kernel lives inside of our ram space (i.e. it isn't executing
	 * in place in flash) then we need to exclude it from the memory that
	 * the kernel gets to manage, since the kernel binary needs some space.
	 */
	memory_end = ((unsigned long)DRAM_BASE) + ((unsigned long)DRAM_SIZE);
	if (((unsigned long)&_end < (unsigned long)DRAM_BASE) ||
		((unsigned long)&_end > (unsigned long)memory_end)) {
	    memory_start = (unsigned long)DRAM_BASE;
	} else {
	    memory_start = (unsigned long)&_end;
	    printk("kernel binary is in RAM -- reserving %ldk for the kernel\n",
		    ((unsigned long)&_end - (unsigned long)DRAM_BASE)>>10);
	}

#ifdef	CONFIG_ARCH_NETARM
	netarm_console_print("setup_arch : MEM start ");
	netarm_dump_hex8((unsigned long)memory_start);
	netarm_console_print(" end ");
	netarm_dump_hex8((unsigned long)memory_end);
	netarm_console_print("\n");

#ifdef CONFIG_NETARM_EEPROM
#ifdef CONFIG_ETHER_NETARM
	/* copy MAC address from NVRAM to Ethernet controller */

	{
		NA_dev_board_params_t *pParams;
		unsigned int *pUint;
		unsigned int uItmp;
		unsigned int serno;
		unsigned int csum;
		
		pParams = (NA_dev_board_params_t *)(NETARM_MMAP_EEPROM_BASE) ;

		csum = compute_checksum(pParams) ;
		
		if (csum == 0)
		{
			/* valid checksum... parse chars */
			int i,j ;
		
			serno = 0;
			for ( i = 0 ; i < 8 ; i++ )
			{
				j = parse_decimal(pParams->serialNumber[i]);
				if (j < 0)
				{
					/* invalid digits - pretend csum bad */
					csum = 1 ;
					break;
				}
				serno *= 10 ;
				serno += j;
			}
		}

		if (csum != 0)
		{
			/* use default serial number */
			serno = 99335 ;
		}
		/* multiply serno by 8 to calc MAC */
		serno <<= 3 ;
		
		uItmp = serno & 0xFF ;
		uItmp <<= 8 ;
		uItmp += ( serno >> 8 ) & 0xFF ;
		pUint = (unsigned int *)NA_SAL_STATION_ADDR_3 ;
		*pUint = uItmp;

		uItmp = ( serno >> 16 ) & 0xFF ;
		uItmp <<= 8 ;
		uItmp += NETARM_OUI_BYTE3 ;
		pUint = (unsigned int *)NA_SAL_STATION_ADDR_2 ;
		*pUint = uItmp;

		uItmp = NETARM_OUI_BYTE2 ;
		uItmp <<= 8 ;
		uItmp += NETARM_OUI_BYTE1 ;
		pUint = (unsigned int *)NA_SAL_STATION_ADDR_1 ;
		*pUint = uItmp;
	}
#endif
#endif

#endif

	init_task.mm->start_code = TASK_SIZE;
	init_task.mm->end_code	 = TASK_SIZE + (unsigned long) &_etext;
	init_task.mm->end_data	 = TASK_SIZE + (unsigned long) &_edata;
	init_task.mm->brk	 = TASK_SIZE + (unsigned long) &_end;

	/* Save unparsed command line copy for /proc/cmdline */
	memcpy(saved_command_line, from, COMMAND_LINE_SIZE);
	saved_command_line[COMMAND_LINE_SIZE-1] = '\0';

	for (;;) {
		if (c == ' ' &&
		    from[0] == 'm' &&
		    from[1] == 'e' &&
		    from[2] == 'm' &&
		    from[3] == '=') {
			memory_end = simple_strtoul(from+4, &from, 0);
			if (*from == 'K' || *from == 'k') {
				memory_end = memory_end << 10;
				from++;
			} else if (*from == 'M' || *from == 'm') {
				memory_end = memory_end << 20;
				from++;
			}
			memory_end = memory_end + PAGE_OFFSET;
		}
#if defined( CONFIG_ARCH_TRIO)
		else if (c == ' ' &&
				from[0] == 'r' &&
				from[1] == 'o' &&
				from[2] == 'm' &&
				from[3] == '=') {
				trio_romdisk_addr = simple_strtoul(from+4, &from, 0);
				if (*from == 'K' || *from == 'k') {
					trio_romdisk_addr = trio_romdisk_addr << 10;
					from++;
				} else if (*from == 'M' || *from == 'm') {
					trio_romdisk_addr = trio_romdisk_addr << 20;
					from++;
				}
			}
#endif						
		c = *from++;
		if (!c)
			break;
		if (COMMAND_LINE_SIZE <= ++len)
			break;
		*to++ = c;
	}

	*to = '\0';
	*cmdline_p = /* command_line */ "root=/dev/rom0";

#if defined(CONFIG_CHR_DEV_FLASH) || defined(CONFIG_BLK_DEV_FLASH)
	/* we need to initialize the Flashrom device here since we might
	 * do things with flash early on in the boot
	 */
	flash_probe();
#endif

	*memory_start_p = memory_start;
	*memory_end_p = memory_end;

	setup_initrd (params, memory_end);

	strcpy (system_utsname.machine, armidlist[armidindex].name);
#ifdef CONFIG_SERIAL_TRIO
	{
		extern void register_console(void (*proc)(const char *));
		extern void console_print_trio(const char * b);
		register_console(console_print_trio);
	}

#elif defined(CONFIG_CONSOLE_ON_SC28L91)
	{
		extern void register_console(void (*proc)(const char *));
		extern void console_print_sc28l91(const char * b);
		register_console(console_print_sc28l91);
	}
#elif defined(CONFIG_CONSOLE_ON_ATMEL)
	{
		extern void register_console(void (*proc)(const char *));
		extern void console_print_atmel(const char * b);
		atmel_console_initialized = 0;
		register_console(console_print_atmel);
	}
#elif CONFIG_ARCH_NETARM
	{
		extern void register_console(void (*proc)(const char *));
		extern void netarm_console_print(const char * b);
		register_console(netarm_console_print);
		
		printk("NET+ARM console enabled\n");
	}
#endif
}

#define ISSET(bit) (armidlist[armidindex].features & bit)

int get_cpuinfo(char * buffer)
{
	int len;

	len = sprintf (buffer,  "CPU:\n"
				"Type\t\t: %s\n"
				"Revision\t: %d\n"
				"Manufacturer\t: %s\n"
				"32bit modes\t: %s\n"
				"BogoMips\t: %lu.%02lu\n",
				armidlist[armidindex].name,
				(int)arm_id & 15,
				armidlist[armidindex].manu,
				ISSET (F_32BIT) ? "yes" : "no",
				(loops_per_sec+2500) / 500000,
				((loops_per_sec+2500) / 5000) % 100);
	len += sprintf (buffer + len,
				"\nHardware:\n"
				"Mem System\t: %s\n"
				"IOEB\t\t: %s\n",
				ISSET(F_MEMC)  ? "MEMC" : 
				ISSET(F_MMU)   ? "MMU"  : "*unknown*",
				ISSET(F_IOEB)  ? "present" : "absent"
				);
	return len;
}
