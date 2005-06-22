/*
 * NDS touchscreen driver
 *
 * Copyright (c) 2005 Anthony Zawacki
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h> 
#include <linux/interrupt.h> 
#include <linux/version.h>
#include <linux/kbd_kern.h>
//#include <linux/config.h>
#include <asm/arch/fifo.h>

#include <asm/irq.h>
#include <asm/io.h>

extern const unsigned short master_Palette[16];
extern const unsigned char keyboard_Tiles[6016];
extern const unsigned short keyboard_Map_Unpressed[480];
extern const unsigned short keyboard_Map_Pressed[480];
extern const unsigned short keyboard_Map_Qwerty_Lower[480];
extern const unsigned short keyboard_Map_Qwerty_Upper[480];
extern const unsigned short qwertyKeyMap[32*24];

/*
 * Useful defines
 */
#ifdef FALSE
#undef FALSE
#endif
#define FALSE                           (1 == 0)

#ifdef TRUE
#undef TRUE
#endif
#define TRUE                            (1 == 1)

#ifndef NULL
#define NULL                            (void *) 0
#endif

#define SUB_DISPLAY_CR	(*(volatile u32*)0x04001000)

#define MODE_0_2D      0x10000

#define DISPLAY_BG0_ACTIVE    (1 << 8)
#define DISPLAY_BG1_ACTIVE    (1 << 9)

#define SUB_BG0_CR     (*(volatile u16*)0x04001008)
#define SUB_BG1_CR     (*(volatile u16*)0x0400100A)

#define BG_COLOR_16        0x0

#define BG_PRIORITY(n) (n)

#define BG_TILE_BASE(base) ((base) << 2)
#define BG_MAP_BASE(base)  ((base) << 8)

#define SCREEN_BASE_BLOCK_SUB(n)  (((n)*0x800)+0x6200000)

#define PALETTE_SUB	((volatile u16*)0x05000400)
#define BG_GFX_SUB	((u16*)0x6200000)

#define VRAM_C_CR	(*(volatile u8*)0x04000242)
#define VRAM_ENABLE	(1<<7)
#define VRAM_OFFSET(n)	((n)<<3)
typedef enum
{
	VRAM_C_LCD = 0,
	VRAM_C_MAIN_BG  = 1 | VRAM_OFFSET(2),
	VRAM_C_MAIN_BG_0x6000000  = 1 | VRAM_OFFSET(0),
	VRAM_C_MAIN_BG_0x6020000  = 1 | VRAM_OFFSET(1),
	VRAM_C_MAIN_BG_0x6040000  = 1 | VRAM_OFFSET(2),
	VRAM_C_MAIN_BG_0x6060000  = 1 | VRAM_OFFSET(3),
	VRAM_C_ARM7 = 2,
	VRAM_C_SUB_BG  = 4,
	VRAM_C_SUB_BG_0x6200000  = 4 | VRAM_OFFSET(0),
	VRAM_C_SUB_BG_0x6220000  = 4 | VRAM_OFFSET(1),
	VRAM_C_SUB_BG_0x6240000  = 4 | VRAM_OFFSET(2),
	VRAM_C_SUB_BG_0x6260000  = 4 | VRAM_OFFSET(3),
	VRAM_C_TEXTURE = 3 | VRAM_OFFSET(2),
	VRAM_C_TEXTURE_SLOT0 = 3 | VRAM_OFFSET(0),
	VRAM_C_TEXTURE_SLOT1 = 3 | VRAM_OFFSET(1),
	VRAM_C_TEXTURE_SLOT2 = 3 | VRAM_OFFSET(2),
	VRAM_C_TEXTURE_SLOT3 = 3 | VRAM_OFFSET(3)

}VRAM_C_TYPE;


#define DRIVER_DESC "Nintendo DS touchscreen driver"
#define NDS_TS_VERSION_CODE KERNEL_VERSION(0, 0, 1)

MODULE_AUTHOR("Anthony Zawacki aka PhoenixRising (pr@dspassme.com)");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 192
#define MAX_KEYBOARDS CONFIG_TOUCHSCREEN_NDS_MAX_KEYBOARDS
#define MAX_KEYCODES CONFIG_TOUCHSCREEN_NDS_MAX_KEYCODES
#define DEFAULT_MODE CONFIG_TOUCHSCREEN_NDS_DEFAULT_MODE

static int mode = DEFAULT_MODE;
static int debug_level = CONFIG_TOUCHSCREEN_NDS_DEBUG_LEVEL;

MODULE_PARM(mode, "i");
MODULE_PARM_DESC(mode, "Determines if the driver returns absolute x/y coordinates (mode 2), key codes for a keyboard (mode 1), or is currently disabled (mode 0.)  Default is 1 (key codes for a keyboard.)");
MODULE_PARM(debug_level, "i");
MODULE_PARM_DESC(debug_level, "Specifes the amount of debug information that will be printed by the module.  0 (default value) is no debugging information, 3 is maximum verbosity.");

#define DISABLED_MODE 0
#define KEYBOARD_MODE 1
#define MOUSE_MODE 2

struct input_dev ndstouch_dev;

struct keycode_item {
  u8 x1;		/* Left side of the key code start */
  u8 y1;		/* Top side of the key code start */
  u8 x2;		/* Right side of the key code end */
  u8 y2;		/* Bottom side of the key code end */
  unsigned int keycode;	/* Key code to return from this area */
};

static int current_keyboard = 0;
static char keyboard_loaded[MAX_KEYBOARDS];
static char keyboard_gfx[MAX_KEYBOARDS][SCREEN_WIDTH * SCREEN_HEIGHT];
static char keyboard_pal[MAX_KEYBOARDS][256];
static struct keycode_item keycodes[MAX_KEYBOARDS][MAX_KEYCODES];

static u16* const bgmap = (u16*)SCREEN_BASE_BLOCK_SUB(4);
static u16* const fgmap = (u16*)SCREEN_BASE_BLOCK_SUB(5);

static void draw_keyboard(int gfx_id)
{
	int i;

	VRAM_C_CR = VRAM_ENABLE | VRAM_C_SUB_BG_0x6200000 ;

	SUB_DISPLAY_CR = MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG1_ACTIVE ;

	SUB_BG0_CR = BG_COLOR_16 | BG_MAP_BASE(4) | BG_PRIORITY(1);
	SUB_BG1_CR = BG_COLOR_16 | BG_MAP_BASE(5) | BG_PRIORITY(0);

	for( i = 0 ; i < 16 ; i++ )
	{
		PALETTE_SUB[i] = master_Palette[i];
	}
	memcpy( BG_GFX_SUB, keyboard_Tiles, sizeof(keyboard_Tiles) );
	memset( bgmap, 0, 32 * 24 * 2 );
	memset( fgmap, 0, 32 * 24 * 2 );
	memcpy( bgmap + 32 * 9, keyboard_Map_Unpressed, 32 * 15 * 2 );
	memcpy( fgmap + 32 * 9, keyboard_Map_Qwerty_Lower, 32 * 15 * 2 );
}

static void update_keyboard(u16 pressedKey)
{
	int x,y ;
	u16 key ;
	struct kbd_struct *kbd = kbd_table + fg_console;
	int shift_pressed = 0 ;
	int caps_pressed = 0 ;
	unsigned short keysym ;
	unsigned char type ;

	if ( kbd->slockstate & ( 1<<KG_SHIFTL | 1<<KG_SHIFTR | 1<<KG_SHIFT ) )
		shift_pressed = 1 ;

	if ( vc_kbd_led( kbd, VC_CAPSLOCK ) )
		caps_pressed = 1 ;

	for ( y = 9 ; y < 24 ; y++ )
	{
		for ( x = 0 ; x < 32 ; x++ )
		{
			key = qwertyKeyMap[ y * 32 + x ] ;
			if ( key != KEY_RESERVED )
			{
				keysym = plain_map[key];
				type = KTYP(keysym) ;
				type -= 0xf0;
				if ( key == pressedKey || 
						( type == KT_SLOCK &&
						  ( 1 << (keysym & 0xff) ) & kbd->slockstate ) )
				{
					bgmap[ y * 32 + x ] = keyboard_Map_Pressed[ (y-9) * 32 + x ] ;
				}
				else
					bgmap[ y * 32 + x ] = keyboard_Map_Unpressed[ (y-9) * 32 + x ] ;
				if ( shift_pressed || ( caps_pressed && type == KT_LETTER ) )
					fgmap[ y * 32 + x ] = keyboard_Map_Qwerty_Upper[ (y-9) * 32 + x ] ;
				else
					fgmap[ y * 32 + x ] = keyboard_Map_Qwerty_Lower[ (y-9) * 32 + x ] ;
			}
		}
	}
}

void touchscreen_event(u8 touched, u8 x, u8 y)
{
	int i = 0;
	struct input_dev* dev = &ndstouch_dev;
	static int wasTouched = FALSE ;
	static u16 currentKey = KEY_RESERVED ;
	u16 key ;

	switch(mode) {
		case KEYBOARD_MODE:
			/* convert to tile indexes */
			x /= 8;
			y /= 8;

			if ( touched )
			{
				key = qwertyKeyMap[y * 32 + x] ;
				if ( !wasTouched && key != KEY_RESERVED )
				{
					currentKey = key ;
					input_report_key(dev, currentKey, 1 ) ;
				}

				wasTouched = TRUE ;
			}
			else if ( wasTouched )
			{
				if ( currentKey != KEY_RESERVED )
				{
					input_report_key(dev, currentKey, 0 ) ;
					currentKey = KEY_RESERVED ;
				}
				wasTouched = FALSE ;
			}

			input_sync(dev);

			update_keyboard( currentKey ) ;

			break;
		case MOUSE_MODE:
			if (touched) {
				input_report_abs(dev, ABS_X, x);
				input_report_abs(dev, ABS_Y, y);
				input_sync(dev);
			}
			break;
		case DISABLED_MODE:
			break;
	}
}

static struct fifo_cb my_callback = {
	.type = FIFO_TOUCH,
	.handler.touch_handler = touchscreen_event
};

static int __init touchscreen_init(void)
{
	int i ;
	printk(KERN_INFO "nds_ts:  Driver version %d.%d.%d loaded\n",
			(NDS_TS_VERSION_CODE >> 16) & 0xff,
			(NDS_TS_VERSION_CODE >>  8) & 0xff,
			(NDS_TS_VERSION_CODE      ) & 0xff);

	switch(mode) {
		case KEYBOARD_MODE:
			draw_keyboard(0);
			break;
		case MOUSE_MODE:
		case DISABLED_MODE:
			break;
	}

	ndstouch_dev.name="Nintendo DS Touchscreen";
	ndstouch_dev.evbit[0] = BIT(EV_KEY);
	for ( i = 0 ; i < sizeof(qwertyKeyMap)/sizeof(qwertyKeyMap[0]) ; i++)
	{
		if ( qwertyKeyMap[i] != KEY_RESERVED )
			ndstouch_dev.keybit[LONG(qwertyKeyMap[i])] |= BIT(qwertyKeyMap[i]);
	}

	input_register_device(&ndstouch_dev);

	register_fifocb(&my_callback);

	return 0;
}

static void __exit touchscreen_exit(void)
{
  input_unregister_device(&ndstouch_dev);
}

module_init(touchscreen_init);
module_exit(touchscreen_exit);

