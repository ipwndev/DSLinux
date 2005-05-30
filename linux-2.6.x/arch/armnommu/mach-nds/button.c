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

struct input_dev ndsbutton_dev;

#if 0
static short ndsbuttons[] = { 
	BTN_A, BTN_B, BTN_SELECT, BTN_START,
       	BTN_0, BTN_1, BTN_2, BTN_3,
	BTN_THUMBR, BTN_THUMBL
};
#else
static short ndsbuttons[] = { 
	KEY_ENTER, BTN_B, BTN_SELECT, BTN_START,
       	KEY_L, KEY_S, BTN_2, BTN_3,
	BTN_THUMBR, BTN_THUMBL
};
#endif

static irqreturn_t ndsbutton_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	int i;
	u16 state = *(volatile u16*) 0x04000130 ;

	for (i = 0 ; i < 10 ; i++)
		input_report_key(&ndsbutton_dev, ndsbuttons[i], !((state >> i) & 1));

        input_sync(&ndsbutton_dev);

	return IRQ_HANDLED ;
}

static int __init ndsbutton_init(void)
{
	int i;

        if (request_irq(IRQ_KEYPAD, ndsbutton_interrupt, 0, "button", NULL)) {
                printk(KERN_ERR "button.c: Can't allocate irq %d\n", IRQ_KEYPAD);
                return -EBUSY;
        }

	*(volatile u16*) 0x04000132 = 0x3ff | 1 << 14 ;

        ndsbutton_dev.evbit[0] = BIT(EV_KEY);
	for ( i = 0 ; i < 10 ; i++ )
		ndsbutton_dev.keybit[LONG(ndsbuttons[i])] |= BIT(ndsbuttons[i]);
        
        input_register_device(&ndsbutton_dev);

	return 0;
}

static void __exit ndsbutton_exit(void)
{
        input_unregister_device(&ndsbutton_dev);
        free_irq(IRQ_KEYPAD, ndsbutton_interrupt);
}

module_init(ndsbutton_init);
module_exit(ndsbutton_exit);

