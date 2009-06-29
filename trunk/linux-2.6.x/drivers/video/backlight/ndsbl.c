/*
 *  linux/drivers/video/backlight/ndsbl.c -- Nintendo DS backlight control
 *
 *      Copyright (C) 2009 Ewan Meadows
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of this archive for
 *  more details.
 */
//TODO: DS/DS lite detection, get commands
#include <linux/init.h>
#include <linux/module.h>
#include <asm/arch/fifo.h>
#include <linux/backlight.h>
#include <linux/device.h>
#include <linux/err.h>

static struct backlight_properties ndsbl_properties;

static void read_from_arm7(u8 cmd, u32 data)
{
}

static struct fifo_cb power_callback = 
{
		.type = FIFO_POWER,
		.handler.power_handler = read_from_arm7
};

static int nds_set_brightness(struct backlight_device *bd, int brightness)
{
	//0 = Max BACKLIGHT_BRIGHTNESS
	//Detect DS Phat before trying to send command
	//POWER4 bits4-7 will always be 4 on Lite
	//4 on reg 0 = shutdown
	if (brightness > ndsbl_properties.max_brightness)
	{
	brightness = ndsbl_properties.max_brightness;
	};
	printk(KERN_ALERT "ndsbl: Setting brightness level %u\n", brightness);
	switch(brightness){
		case 0:
			nds_fifo_send(FIFO_POWER_CMD(FIFO_POWER_CMD_BACKLIGHT_BRIGHTNESS, 3));
			break;
		case 1:
			nds_fifo_send(FIFO_POWER_CMD(FIFO_POWER_CMD_BACKLIGHT_BRIGHTNESS, 2));
			break;
		case 2:
			nds_fifo_send(FIFO_POWER_CMD(FIFO_POWER_CMD_BACKLIGHT_BRIGHTNESS, 1));
			break;
		case 3:
			nds_fifo_send(FIFO_POWER_CMD(FIFO_POWER_CMD_BACKLIGHT_BRIGHTNESS, 0));
			break;
	}
	return 0;
}

static int nds_set_power(struct backlight_device *bd, int state)
{
		//0 = On for BACKLIGHT_POWER, >0 = off
		switch(state){
			case 0:
				nds_fifo_send(FIFO_POWER_CMD(FIFO_POWER_CMD_BACKLIGHT_POWER, 1));
				break;
			case 1:
			case 2:
			case 3:
				nds_fifo_send(FIFO_POWER_CMD(FIFO_POWER_CMD_BACKLIGHT_POWER, 0));
				break;
			default:
				nds_fifo_send(FIFO_POWER_CMD(FIFO_POWER_CMD_BACKLIGHT_POWER, 1));
				break;
			
		}
		return 0;	
};

static int nds_get_power(struct backlight_device *bd)
{
	printk(KERN_ALERT "ndsbl: Read power not implemented\n");
	return 0;
};

static int nds_get_brightness(struct backlight_device *bd)
{
	printk(KERN_ALERT "ndsbl: Read brightness not implemented\n");
	return 0;
};



static struct backlight_properties ndsbl_properties = 
{
	.owner		= THIS_MODULE,
	.get_power	= nds_get_power,
	.set_power	= nds_set_power,
	.get_brightness = nds_get_brightness,
	.set_brightness = nds_set_brightness,
};


static struct backlight_device *ndsbl_device;

static void ndsbl_platform_release(struct device *device)
{
}

static int __init ndsbl_probe(struct device *dev)
{	
	ndsbl_properties.max_brightness = 3;
	
	ndsbl_device = backlight_device_register ( "ndsbl", NULL, &ndsbl_properties);
	if (IS_ERR (ndsbl_device))
		return PTR_ERR (ndsbl_device);
	//Read and set initial values	
	register_fifocb(&power_callback);
	printk(KERN_ALERT "ndsbl: Nintendo DS backlight driver initilised OK\n");
	return 0;
};
	

static int ndsbl_remove(struct device *dev)
{
	backlight_device_unregister(ndsbl_device);
	return 0;
}

static struct device_driver ndsbl_driver = 
{
	.name		= "ndsbl",
	.bus		= &platform_bus_type,
	.probe		= ndsbl_probe,
	.remove		= ndsbl_remove,
};

static struct platform_device ndsbl_device0 = {
	.name = "ndsbl",
	.id = 0,
	.dev = {
		.release = ndsbl_platform_release,
		}
};

static int __init ndsbl_init(void)
	{
	int ret = 0;
	ret = driver_register(&ndsbl_driver);
	if (!ret) 
	{
		printk(KERN_ALERT "ndsbl: trying platform_device_register \n");
		ret = platform_device_register(&ndsbl_device0);
		if (ret) driver_unregister(&ndsbl_driver);
	};
	return ret; 

	}

static void __exit ndsbl_exit(void)
	{
	driver_unregister(&ndsbl_driver);
	}
module_init(ndsbl_init);
module_exit(ndsbl_exit);

MODULE_AUTHOR("Ewan Meadows");
MODULE_DESCRIPTION("Nintendo DS Backlight driver");
MODULE_LICENSE("GPLv2");
