/*
 * NDS buttons driver
 *
 * Copyright (c) 2005 Malcolm Parsons
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

#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/fifo.h>

#define REG_KEYINPUT 	0x04000130
static struct input_dev ndsbutton_dev;

#if 0
	BTN_A, BTN_B, BTN_SELECT, BTN_START,
       	BTN_0, BTN_1, BTN_2, BTN_3,
	BTN_THUMBR, BTN_THUMBL
};
#else
static short ndsbuttons[] = { 
	KEY_ENTER, KEY_SPACE, BTN_SELECT, BTN_START,
       	KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN,
	#ifndef CONFIG_NDS_SWAP_LR
	KEY_RIGHTCTRL, KEY_LEFTSHIFT
	#else
	KEY_RIGHTSHIFT, KEY_LEFTCTRL
	#endif
};
static short ndsbuttons2[] = { 
	KEY_PAGEUP, KEY_PAGEDOWN
};
#endif

static irqreturn_t ndsbutton_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	int i;
	u16 state = readw(REG_KEYINPUT);

	for (i = 0 ; i < (sizeof(ndsbuttons)/sizeof(ndsbuttons[0])) ; i++)
		input_report_key(&ndsbutton_dev, ndsbuttons[i], !((state >> i) & 1));

        input_sync(&ndsbutton_dev);

	return IRQ_HANDLED ;
}

static void ndsbutton_xkeys(u32 state)
{
	int i;
	for (i = 0 ; i < (sizeof(ndsbuttons2)/sizeof(ndsbuttons2[0])) ; i++)
	{
		input_report_key(&ndsbutton_dev, ndsbuttons2[i], !((state >> i) & 1));
	}
       input_report_switch(&ndsbutton_dev, SW_0, ((state >> 7 )&1)); 
       input_sync(&ndsbutton_dev);
}

static struct fifo_cb ndsbutton_fifocb = {
	.type = FIFO_BUTTONS,
	.handler.button_handler = ndsbutton_xkeys
};

static int __init ndsbutton_init(void)
{
	int i;

        if (request_irq(IRQ_VBLANK, ndsbutton_interrupt, SA_SHIRQ, "button", &ndsbutton_dev)) {
                printk(KERN_ERR "button.c: Can't allocate irq %d\n", IRQ_KEYPAD);
                return -EBUSY;
        }

	*(volatile u16*) 0x04000004 |= 1 << 3 ;

        ndsbutton_dev.evbit[0] = BIT(EV_KEY) | BIT(EV_REP) | BIT(EV_SW);
	ndsbutton_dev.swbit[0] = BIT(SW_0);
	for ( i = 0 ; i < (sizeof(ndsbuttons)/sizeof(ndsbuttons[0])) ; i++ )
		ndsbutton_dev.keybit[LONG(ndsbuttons[i])] |= BIT(ndsbuttons[i]);
	for ( i = 0 ; i < (sizeof(ndsbuttons2)/sizeof(ndsbuttons2[0])) ; i++ )
		ndsbutton_dev.keybit[LONG(ndsbuttons2[i])] |= BIT(ndsbuttons2[i]);
        
        input_register_device(&ndsbutton_dev);

	register_fifocb( &ndsbutton_fifocb ) ;

	return 0;
}

static void __exit ndsbutton_exit(void)
{
        input_unregister_device(&ndsbutton_dev);
        free_irq(IRQ_VBLANK, &ndsbutton_dev);
}

module_init(ndsbutton_init);
module_exit(ndsbutton_exit);

