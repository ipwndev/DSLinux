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
#include <asm/arch/fifo.h>

#include <asm/irq.h>
#include <asm/io.h>

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

#define DRIVER_DESC "Nintendo DS touchscreen driver"
#define NDS_TS_VERSION_CODE KERNEL_VERSION(0, 0, 1)

MODULE_AUTHOR("Anthony Zawacki aka PhoenixRising (pr@dspassme.com)");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

#define SCREEN_WIDTH CONFIG_INPUT_TSDEV_SCREEN_X
#define SCREEN_HEIGHT CONFIG_INPUT_TSDEV_SCREEN_Y
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

static void load_keyboard_graphic(int gfx_id)
{
  if ((gfx_id >= 0) && (gfx_id < MAX_KEYBOARDS) && keyboard_loaded[gfx_id]) {
    if (debug_level) {
      printk(KERN_INFO "nds_tc.c: Loading keyboard graphic %d.\n", gfx_id);
    }
    current_keyboard = gfx_id;
  } else {
    printk(KERN_ERR "nds_ts.c:  Cannot load keyboard graphic %d.  Either "\
	   "the keyboard graphic is not loaded or gfx_id >= %d, which is "\
	   "too large.", gfx_id, MAX_KEYBOARDS);
  }
}

void touchscreen_event(u8 touched, u8 x, u8 y)
{
  int i = 0;
  struct input_dev* dev = &ndstouch_dev;

  switch(mode) {
  case KEYBOARD_MODE:
    break;
    for(i = 0; i < MAX_KEYCODES; ++i) {
      if ((x >= keycodes[current_keyboard][i].x1) &&
	  (x <= keycodes[current_keyboard][i].x2) &&
	  (y >= keycodes[current_keyboard][i].y1) &&
	  (y <= keycodes[current_keyboard][i].y2)) {
	input_report_key(dev, keycodes[current_keyboard][i].keycode & 0xFFFF,
			 touched);
	/* Check to see if the upper bits indicate a keyboard change */
	if (keycodes[current_keyboard][i].keycode & ~0xFFFF) {
	  /* Which keyboard is desired? */
	  load_keyboard_graphic(keycodes[current_keyboard][i].keycode >> 16);
	}
	/*
	 * We don't break in case the user has defined multiple scan
	 * codes for a keypress area on the keyboard.  Perhaps a macro
	 * button of some sort?
	 */
      }
    }
  case MOUSE_MODE:
    if (touched) {
      input_report_abs(dev, ABS_X, x);
      input_report_abs(dev, ABS_Y, y);
    }
    break;
  case DISABLED_MODE:
    break;
  }
}

static struct fifo_cb my_callback;

static int __init touchscreen_init(void)
{
  printk(KERN_INFO "nds_ts:  Driver version %d.%d.%d loaded\n",
	 (NDS_TS_VERSION_CODE >> 16) & 0xff,
	 (NDS_TS_VERSION_CODE >>  8) & 0xff,
	 (NDS_TS_VERSION_CODE      ) & 0xff);

  my_callback.type = FIFO_TOUCH;
  my_callback.handler.touch_handler = touchscreen_event;

  switch(mode) {
  case KEYBOARD_MODE:
    load_keyboard_graphic(0);
    break;
  case MOUSE_MODE:
  case DISABLED_MODE:
    break;
  }

  ndstouch_dev.name="Nintendo DS Touchscreen";
  ndstouch_dev.evbit[0] = BIT(EV_KEY);
  ndstouch_dev.keybit[LONG(BTN_0)] = BIT(BTN_0);

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

