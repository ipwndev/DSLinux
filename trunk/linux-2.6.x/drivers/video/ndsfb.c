/*
 *  linux/arch/armnommu/mach-nds/fb.c -- Nintendo DS frame buffer device
 *
 *      Copyright (C) 2005 Malcolm Parsons
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of this archive for
 *  more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/fb.h>
#include <linux/init.h>

#define DISPLAY_CR       (*(volatile u32*)0x04000000)
#define SUB_DISPLAY_CR   (*(volatile u32*)0x04001000)

#define MODE_0_2D      0x10000
#define MODE_1_2D      0x10001
#define MODE_2_2D      0x10002
#define MODE_3_2D      0x10003
#define MODE_4_2D      0x10004
#define MODE_5_2D      0x10005

#define DISPLAY_BG0_ACTIVE    (1 << 8)
#define DISPLAY_BG1_ACTIVE    (1 << 9)
#define DISPLAY_BG2_ACTIVE    (1 << 10)
#define DISPLAY_BG3_ACTIVE    (1 << 11)
#define DISPLAY_SPR_ACTIVE    (1 << 12)
#define DISPLAY_WIN0_ON       (1 << 13)
#define DISPLAY_WIN1_ON       (1 << 14)
#define DISPLAY_SPR_WIN_ON    (1 << 15)

#define BG_256_COLOR   (1<<7)

#define BG_RS_16x16    (0 << 14)
#define BG_RS_32x32    (1 << 14)
#define BG_RS_64x64    (2 << 14)
#define BG_RS_128x128  (3 << 14)

#define BG_BMP16_128x128 (BG_RS_16x16 | BG_256_COLOR | 1<<2)
#define BG_BMP16_256x256 (BG_RS_32x32 | BG_256_COLOR | 1<<2)
#define BG_BMP16_512x256 (BG_RS_64x64 | BG_256_COLOR | 1<<2)
#define BG_BMP16_512x512 (BG_RS_128x128 | BG_256_COLOR | 1<<2)

#define BG3_XDX        (*(volatile u16*)0x04000030)
#define BG3_XDY        (*(volatile u16*)0x04000032)
#define BG3_YDX        (*(volatile u16*)0x04000034)
#define BG3_YDY        (*(volatile u16*)0x04000036)
#define BG3_CX         (*(volatile u32*)0x04000038)
#define BG3_CY         (*(volatile u32*)0x0400003C)

#define SUB_BG3_XDX    (*(volatile u16*)0x04001030)
#define SUB_BG3_XDY    (*(volatile u16*)0x04001032)
#define SUB_BG3_YDX    (*(volatile u16*)0x04001034)
#define SUB_BG3_YDY    (*(volatile u16*)0x04001036)
#define SUB_BG3_CX     (*(volatile u32*)0x04001038)
#define SUB_BG3_CY     (*(volatile u32*)0x0400103C)

#define BG_CR           ((volatile u16*)0x04000008)
#define BG0_CR         (*(volatile u16*)0x04000008)
#define BG1_CR         (*(volatile u16*)0x0400000A)
#define BG2_CR         (*(volatile u16*)0x0400000C)
#define BG3_CR         (*(volatile u16*)0x0400000E)

#define SUB_BG_CR      ((volatile u16*)0x04001008)
#define SUB_BG0_CR     (*(volatile u16*)0x04001008)
#define SUB_BG1_CR     (*(volatile u16*)0x0400100A)
#define SUB_BG2_CR     (*(volatile u16*)0x0400100C)
#define SUB_BG3_CR     (*(volatile u16*)0x0400100E)

#define VRAM_CR         (*(volatile u32*)0x04000240)
#define VRAM_A_CR       (*(volatile u8*)0x04000240)
#define VRAM_B_CR       (*(volatile u8*)0x04000241)
#define VRAM_C_CR       (*(volatile u8*)0x04000242)
#define VRAM_D_CR       (*(volatile u8*)0x04000243)
#define VRAM_E_CR       (*(volatile u8*)0x04000244)
#define VRAM_F_CR       (*(volatile u8*)0x04000245)
#define VRAM_G_CR       (*(volatile u8*)0x04000246)
#define WRAM_CR         (*(volatile u8*)0x04000247)
#define VRAM_H_CR       (*(volatile u8*)0x04000248)
#define VRAM_I_CR       (*(volatile u8*)0x04000249)

#define VRAM_ENABLE   (1<<7)

#define VRAM_OFFSET(n)  ((n)<<3)

typedef enum
{
	VRAM_A_LCD = 0,
	VRAM_A_MAIN_BG  = 1,
	VRAM_A_MAIN_BG_0x6000000  = 1 | VRAM_OFFSET(0),
	VRAM_A_MAIN_BG_0x6020000  = 1 | VRAM_OFFSET(1),
	VRAM_A_MAIN_BG_0x6040000  = 1 | VRAM_OFFSET(2),
	VRAM_A_MAIN_BG_0x6060000  = 1 | VRAM_OFFSET(3),
	VRAM_A_MAIN_SPRITE = 2,
	VRAM_A_TEXTURE = 3,
	VRAM_A_TEXTURE_SLOT0 = 3 | VRAM_OFFSET(0),
	VRAM_A_TEXTURE_SLOT1 = 3 | VRAM_OFFSET(1),
	VRAM_A_TEXTURE_SLOT2 = 3 | VRAM_OFFSET(2),
	VRAM_A_TEXTURE_SLOT3 = 3 | VRAM_OFFSET(3)

}VRAM_A_TYPE;

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

static struct fb_var_screeninfo ndsfb_default __initdata = {
	.xres =		256,
	.yres =		192,
	.xres_virtual =	256,
	.yres_virtual =	256,
	.bits_per_pixel = 16,
	.red =		{ 0, 5, 0 },
      	.green =	{ 5, 5, 0 },
      	.blue =		{ 10, 5, 0 },
      	.transp =	{ 15, 1, 0 },
      	.activate =	FB_ACTIVATE_NOW,
      	.height =	-1,
      	.width =	-1,
      	.pixclock =	20000,
      	.left_margin =	64,
      	.right_margin =	64,
      	.upper_margin =	32,
      	.lower_margin =	32,
      	.hsync_len =	64,
      	.vsync_len =	2,
      	.vmode =	FB_VMODE_NONINTERLACED,
};

static struct fb_fix_screeninfo ndsfb_fix __initdata = {
	.id =		"Nintendo DS FB",
	.type =		FB_TYPE_PACKED_PIXELS,
	.visual =	FB_VISUAL_TRUECOLOR,
	.xpanstep =	1,
	.ypanstep =	1,
	.ywrapstep =	1,
	.accel =	FB_ACCEL_NONE,
};

static int ndsfb_enable __initdata = 1;	/* enabled by default */

    /*
     *  Interface used by the world
     */
int ndsfb_init(void);
int ndsfb_setup(char *);

static int ndsfb_check_var(struct fb_var_screeninfo *var,
			 struct fb_info *info);
static int ndsfb_set_par(struct fb_info *info);
static int ndsfb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			 u_int transp, struct fb_info *info);
static int ndsfb_pan_display(struct fb_var_screeninfo *var,
			   struct fb_info *info);
static int ndsfb_mmap(struct fb_info *info, struct file *file,
		    struct vm_area_struct *vma);

static struct fb_ops ndsfb_ops = {
	.fb_check_var	= ndsfb_check_var,
	.fb_set_par	= ndsfb_set_par,
	.fb_setcolreg	= ndsfb_setcolreg,
	.fb_pan_display	= ndsfb_pan_display,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
	.fb_cursor	= soft_cursor,
	.fb_mmap	= ndsfb_mmap,
};

    /*
     *  Internal routines
     */

    /*
     *  Setting the video mode has been split into two parts.
     *  First part, xxxfb_check_var, must not write anything
     *  to hardware, it should only verify and adjust var.
     *  This means it doesn't alter par but it does use hardware
     *  data from it to check this var. 
     */

static int ndsfb_check_var(struct fb_var_screeninfo *var,
			 struct fb_info *info)
{
        /*
         *  FB_VMODE_CONUPDATE and FB_VMODE_SMOOTH_XPAN are equal!
         *  as FB_VMODE_SMOOTH_XPAN is only used internally
         */

#if 0
        if (var->vmode & FB_VMODE_CONUPDATE) {
                var->vmode |= FB_VMODE_YWRAP;
                var->xoffset = info->var.xoffset;
                var->yoffset = info->var.yoffset;
        }
#endif

        /*
         *  Some very basic checks
         */
        if ( var->xres != 256 )
                return -EINVAL;
        if ( var->yres != 192 )
                return -EINVAL;

        if ( var->bits_per_pixel != 16 )
                return -EINVAL;

	var->xres_virtual = 256;
	var->yres_virtual = 256;

        if (var->xres_virtual < var->xoffset + var->xres)
                return -EINVAL;
        if (var->yres_virtual < var->yoffset + var->yres)
                return -EINVAL;

        switch (var->bits_per_pixel) {
                case 16:		/* RGBA 5551 */
                        var->red.offset = 0;
                        var->red.length = 5;
                        var->green.offset = 5;
                        var->green.length = 5;
                        var->blue.offset = 10;
                        var->blue.length = 5;
                        var->transp.offset = 15;
                        var->transp.length = 1;
                        break;
                default:
                        return -EINVAL;
        }
        var->red.msb_right = 0;
        var->green.msb_right = 0;
        var->blue.msb_right = 0;
        var->transp.msb_right = 0;

        return 0;
}

/* This routine actually sets the video mode. It's in here where we
 * the hardware state info->par and fix which can be affected by the 
 * change in par.
 */
static int ndsfb_set_par(struct fb_info *info)
{
	info->fix.line_length = 256 * 2;
	//info->fix.smem_start = info->screen_base ;
	//info->fix.smem_len = info->var->xres_virtual * info->var->yres_virtual * 2 ;

	if (info->par == 0)
	{
		DISPLAY_CR = MODE_5_2D | DISPLAY_BG3_ACTIVE;
		VRAM_A_CR = VRAM_ENABLE | VRAM_A_MAIN_BG_0x6000000 ;
		BG3_CR = BG_BMP16_256x256;

		BG3_XDX = 1 << 8;
		BG3_XDY = 0;
		BG3_YDX = 0;
		BG3_YDY = 1 << 8;
		BG3_CX  = (info->var.xoffset) << 8;
		BG3_CY  = (info->var.yoffset) << 8;
	}
	else
	{
		SUB_DISPLAY_CR = MODE_5_2D | DISPLAY_BG3_ACTIVE;
		VRAM_C_CR = VRAM_ENABLE | VRAM_C_SUB_BG_0x6200000 ;
		SUB_BG3_CR = BG_BMP16_256x256;

		SUB_BG3_XDX = 1 << 8;
		SUB_BG3_XDY = 0;
		SUB_BG3_YDX = 0;
		SUB_BG3_YDY = 1 << 8;
		SUB_BG3_CX  = (-info->var.xoffset) << 8;
		SUB_BG3_CY  = (-info->var.yoffset) << 8;
	}

        return 0;
}

    /*
     *  Set a single color register. The values supplied are already
     *  rounded down to the hardware's capabilities (according to the
     *  entries in the var structure). Return != 0 for invalid regno.
     */

static int ndsfb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			 u_int transp, struct fb_info *info)
{
	u32 v;

	if (regno >= 16)
		return 1;

#define CNVT_TOHW(val,width) ((((val)<<(width))+0x7FFF-(val))>>16)
	red = CNVT_TOHW(red, info->var.red.length);
	green = CNVT_TOHW(green, info->var.green.length);
	blue = CNVT_TOHW(blue, info->var.blue.length);
	transp = CNVT_TOHW(transp, info->var.transp.length);
#undef CNVT_TOHW

	/* Truecolor has hardware independent palette */

	v = (red << info->var.red.offset) |
	    (green << info->var.green.offset) |
	    (blue << info->var.blue.offset) |
	    (1 << info->var.transp.offset);
	((u32 *) (info->pseudo_palette))[regno] = v;

	return 0;
}

    /*
     *  Pan or Wrap the Display
     *
     *  This call looks only at xoffset, yoffset and the FB_VMODE_YWRAP flag
     */

static int ndsfb_pan_display(struct fb_var_screeninfo *var,
			   struct fb_info *info)
{
	if (var->vmode & FB_VMODE_YWRAP) {
		if (var->yoffset < 0
		    || var->yoffset >= info->var.yres_virtual
		    || var->xoffset)
			return -EINVAL;
	} else {
		if (var->xoffset + var->xres > info->var.xres_virtual ||
		    var->yoffset + var->yres > info->var.yres_virtual)
			return -EINVAL;
	}
	info->var.xoffset = var->xoffset;
	info->var.yoffset = var->yoffset;
	if (var->vmode & FB_VMODE_YWRAP)
		info->var.vmode |= FB_VMODE_YWRAP;
	else
		info->var.vmode &= ~FB_VMODE_YWRAP;
	return 0;
}

    /*
     *  Most drivers don't need their own mmap function 
     */

static int ndsfb_mmap(struct fb_info *info, struct file *file,
		    struct vm_area_struct *vma)
{
	return -EINVAL;
}

int __init ndsfb_setup(char *options)
{
	char *this_opt;

	ndsfb_enable = 1;

	if (!options || !*options)
		return 1;

	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!*this_opt)
			continue;
		if (!strncmp(this_opt, "disable", 7))
			ndsfb_enable = 0;
	}
	return 1;
}

    /*
     *  Initialisation
     */

static void ndsfb_platform_release(struct device *device)
{
	// This is called when the reference count goes to zero.
}

static int __init ndsfb_probe(struct device *device)
{
	struct platform_device *dev = to_platform_device(device);
	struct fb_info *info;
	int retval = -ENOMEM;

	info = framebuffer_alloc(sizeof(u32) * 256, &dev->dev);
	if (!info)
		goto err;

	info->screen_base = dev->id == 0 ? (void*)0x06000000 : (void*)0x06200000 ;
	info->fbops = &ndsfb_ops;

	retval = fb_find_mode(&info->var, info, NULL,
			      NULL, 0, NULL, 16);

	if (!retval || (retval == 4))
		info->var = ndsfb_default;
	info->fix = ndsfb_fix;
	info->pseudo_palette = info->par;
	info->par = dev->id;
	info->flags = FBINFO_FLAG_DEFAULT;

	retval = fb_alloc_cmap(&info->cmap, 256, 0);
	if (retval < 0)
		goto err1;

	retval = register_framebuffer(info);
	if (retval < 0)
		goto err2;
	dev_set_drvdata(&dev->dev, info);

	printk(KERN_INFO
	       "fb%d: Nintendo DS frame buffer device\n",
	       info->node);
	return 0;
err2:
	fb_dealloc_cmap(&info->cmap);
err1:
	framebuffer_release(info);
err:
	return retval;
}

static int ndsfb_remove(struct device *device)
{
	struct fb_info *info = dev_get_drvdata(device);

	if (info) {
		unregister_framebuffer(info);
		framebuffer_release(info);
	}
	return 0;
}

static struct device_driver ndsfb_driver = {
	.name	= "ndsfb",
	.bus	= &platform_bus_type,
	.probe	= ndsfb_probe,
	.remove = ndsfb_remove,
};

static struct platform_device ndsfb_device0 = {
	.name	= "ndsfb",
	.id	= 0,
	.dev	= {
		.release = ndsfb_platform_release,
	}
};

static struct platform_device ndsfb_device1 = {
	.name	= "ndsfb",
	.id	= 1,
	.dev	= {
		.release = ndsfb_platform_release,
	}
};

int __init ndsfb_init(void)
{
	int ret = 0;

#ifndef MODULE
	char *option = NULL;

	if (fb_get_options("ndsfb", &option))
		return -ENODEV;
	ndsfb_setup(option);
#endif

	if (!ndsfb_enable)
		return -ENXIO;

	ret = driver_register(&ndsfb_driver);

	if (!ret) {
		ret = platform_device_register(&ndsfb_device0);
		if (ret)
			driver_unregister(&ndsfb_driver);
		else
			ret = platform_device_register(&ndsfb_device1);
	}
	return ret;
}

module_init(ndsfb_init);

