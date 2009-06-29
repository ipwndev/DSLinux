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

#include <asm/arch/power.h>
#include <asm/io.h>
#include <asm/arch/fifo.h>

#define	VCOUNT  	 (*(volatile u16*)0x04000006)
#define DISPLAY_CR       (*(volatile u32*)0x04000000)
#define SUB_DISPLAY_CR   (*(volatile u32*)0x04001000)

#define PALETTE        ((volatile u16*)0x05000000)
#define PALETTE_SUB    ((volatile u16*)0x05000400)

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

#define BG_BMP_BASE(base)  ((base) << 8)

#define BG_WRAP_ON     (1 << 13)

#define BG_RS_16x16    (0 << 14)
#define BG_RS_32x32    (1 << 14)
#define BG_RS_64x64    (2 << 14)
#define BG_RS_128x128  (3 << 14)

#define BG_BMP8_128x128 (BG_RS_16x16 | BG_256_COLOR)
#define BG_BMP8_256x256 (BG_RS_32x32 | BG_256_COLOR)
#define BG_BMP8_512x256 (BG_RS_64x64 | BG_256_COLOR)
#define BG_BMP8_512x512 (BG_RS_128x128 | BG_256_COLOR)
#define BG_BMP8_1024x512 0
#define BG_BMP8_512x1024 BIT(14)

#define BG_BMP16_128x128 (BG_RS_16x16 | BG_256_COLOR | 1<<2)
#define BG_BMP16_256x256 (BG_RS_32x32 | BG_256_COLOR | 1<<2)
#define BG_BMP16_512x256 (BG_RS_64x64 | BG_256_COLOR | 1<<2)
#define BG_BMP16_512x512 (BG_RS_128x128 | BG_256_COLOR | 1<<2)

#define BG2_XDX        (*(volatile u16*)0x04000020)
#define BG2_XDY        (*(volatile u16*)0x04000022)
#define BG2_YDX        (*(volatile u16*)0x04000024)
#define BG2_YDY        (*(volatile u16*)0x04000026)
#define BG2_CX         (*(volatile u32*)0x04000028)
#define BG2_CY         (*(volatile u32*)0x0400002C)

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
/* 8bit registers are accessed by readb/writeb */
#define VRAM_A_CR       0x04000240
#define VRAM_B_CR       0x04000241
#define VRAM_C_CR       0x04000242
#define VRAM_D_CR       0x04000243
#define VRAM_E_CR       0x04000244
#define VRAM_F_CR       0x04000245
#define VRAM_G_CR       0x04000246
#define WRAM_CR         0x04000247
#define VRAM_H_CR       0x04000248
#define VRAM_I_CR       0x04000249

#define VRAM_ENABLE   (1<<7)

#define VRAM_OFFSET(n)  ((n)<<3)

typedef enum {
	VRAM_A_LCD = 0,
	VRAM_A_MAIN_BG = 1,
	VRAM_A_MAIN_BG_0x6000000 = 1 | VRAM_OFFSET(0),
	VRAM_A_MAIN_BG_0x6020000 = 1 | VRAM_OFFSET(1),
	VRAM_A_MAIN_BG_0x6040000 = 1 | VRAM_OFFSET(2),
	VRAM_A_MAIN_BG_0x6060000 = 1 | VRAM_OFFSET(3),
	VRAM_A_MAIN_SPRITE = 2,
	VRAM_A_TEXTURE = 3,
	VRAM_A_TEXTURE_SLOT0 = 3 | VRAM_OFFSET(0),
	VRAM_A_TEXTURE_SLOT1 = 3 | VRAM_OFFSET(1),
	VRAM_A_TEXTURE_SLOT2 = 3 | VRAM_OFFSET(2),
	VRAM_A_TEXTURE_SLOT3 = 3 | VRAM_OFFSET(3)

} VRAM_A_TYPE;

typedef enum {
	VRAM_B_LCD = 0,
	VRAM_B_MAIN_BG = 1 | VRAM_OFFSET(1),
	VRAM_B_MAIN_BG_0x6000000 = 1 | VRAM_OFFSET(0),
	VRAM_B_MAIN_BG_0x6020000 = 1 | VRAM_OFFSET(1),
	VRAM_B_MAIN_BG_0x6040000 = 1 | VRAM_OFFSET(2),
	VRAM_B_MAIN_BG_0x6060000 = 1 | VRAM_OFFSET(3),
	VRAM_B_MAIN_SPRITE = 2,
	VRAM_B_TEXTURE = 3 | VRAM_OFFSET(1),
	VRAM_B_TEXTURE_SLOT0 = 3 | VRAM_OFFSET(0),
	VRAM_B_TEXTURE_SLOT1 = 3 | VRAM_OFFSET(1),
	VRAM_B_TEXTURE_SLOT2 = 3 | VRAM_OFFSET(2),
	VRAM_B_TEXTURE_SLOT3 = 3 | VRAM_OFFSET(3)

} VRAM_B_TYPE;

typedef enum {
	VRAM_C_LCD = 0,
	VRAM_C_MAIN_BG = 1 | VRAM_OFFSET(2),
	VRAM_C_MAIN_BG_0x6000000 = 1 | VRAM_OFFSET(0),
	VRAM_C_MAIN_BG_0x6020000 = 1 | VRAM_OFFSET(1),
	VRAM_C_MAIN_BG_0x6040000 = 1 | VRAM_OFFSET(2),
	VRAM_C_MAIN_BG_0x6060000 = 1 | VRAM_OFFSET(3),
	VRAM_C_ARM7 = 2,
	VRAM_C_SUB_BG = 4,
	VRAM_C_SUB_BG_0x6200000 = 4 | VRAM_OFFSET(0),
	VRAM_C_SUB_BG_0x6220000 = 4 | VRAM_OFFSET(1),
	VRAM_C_SUB_BG_0x6240000 = 4 | VRAM_OFFSET(2),
	VRAM_C_SUB_BG_0x6260000 = 4 | VRAM_OFFSET(3),
	VRAM_C_TEXTURE = 3 | VRAM_OFFSET(2),
	VRAM_C_TEXTURE_SLOT0 = 3 | VRAM_OFFSET(0),
	VRAM_C_TEXTURE_SLOT1 = 3 | VRAM_OFFSET(1),
	VRAM_C_TEXTURE_SLOT2 = 3 | VRAM_OFFSET(2),
	VRAM_C_TEXTURE_SLOT3 = 3 | VRAM_OFFSET(3)

} VRAM_C_TYPE;

static struct fb_var_screeninfo ndsfb_default __initdata = {
	.xres = 256,
	.yres = 192,
	.xres_virtual = 256,
	.yres_virtual = 512,
	.bits_per_pixel = 8,
	.red = {0, 5, 0},
	.green = {5, 5, 0},
	.blue = {10, 5, 0},
	.transp = {15, 1, 0},
	.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE,
	.height = -1,
	.width = -1,
	.pixclock = 20000,
	.left_margin = 64,
	.right_margin = 64,
	.upper_margin = 32,
	.lower_margin = 32,
	.hsync_len = 64,
	.vsync_len = 2,
	.vmode = FB_VMODE_NONINTERLACED,
};

static struct fb_fix_screeninfo ndsfb_fix __initdata = {
	.id = "Nintendo DS FB",
	.type = FB_TYPE_PACKED_PIXELS,
	.visual = FB_VISUAL_PSEUDOCOLOR,
	.xpanstep = 1,
	.ypanstep = 1,
	.ywrapstep = 1,
	.accel = FB_ACCEL_NONE,
};

static int ndsfb_enable __initdata = 1;	/* enabled by default */

static int opencount = 0;
    /*
     *  Interface used by the world
     */
int ndsfb_init(void);
int ndsfb_setup(char *);

static int ndsfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info);
static int ndsfb_set_par(struct fb_info *info);
static int ndsfb_setcolreg(u_int regno, u_int red, u_int green, u_int blue,
			   u_int transp, struct fb_info *info);
static int ndsfb_pan_display(struct fb_var_screeninfo *var,
			     struct fb_info *info);
static int ndsfb_mmap(struct fb_info *info, struct file *file,
		      struct vm_area_struct *vma);
static int ndsfb_blank(int blank_mode, struct fb_info *info);
static int ndsfb_open(struct fb_info *info, int user);
static int ndsfb_release(struct fb_info *info, int user);
static int ndsfb_ioctl(struct inode *inode, struct file *file,unsigned int cmd, unsigned long arg, struct fb_info *info);

static struct fb_ops ndsfb_ops = {
	.fb_check_var = ndsfb_check_var,
	.fb_set_par = ndsfb_set_par,
	.fb_setcolreg = ndsfb_setcolreg,
	.fb_pan_display = ndsfb_pan_display,
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
	.fb_cursor = soft_cursor,
	.fb_mmap = ndsfb_mmap,
	.fb_blank = ndsfb_blank,
	.fb_open = ndsfb_open,
	.fb_release = ndsfb_release,
	.fb_ioctl	= ndsfb_ioctl
};

extern void ndstouch_open_fb1(void);
extern void ndstouch_close_fb1(void);

#if 0
static irqreturn_t ndsfb_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	struct fb_info *info = dev_get_drvdata(dev_id);

	if (1) {
		ndsfb_set_par(info);
	}

	return IRQ_HANDLED;
}
#endif

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
int scale=0;int xwidth=256;int yheight=192;
static int ndsfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{

	/*
	 *  Some very basic checks
	 */

	if ((var->xres == var->xres_virtual) && (var->yres == var->yres_virtual))
	{


		if (info->par != 0) {
			var->xres_virtual = 256;
			var->yres_virtual = 256;

			if (var->bits_per_pixel != 16 && var->bits_per_pixel != 8)
				return -EINVAL;

			if (var->xres_virtual < var->xoffset + var->xres)
				return -EINVAL;
			if (var->yres_virtual < var->yoffset + var->yres)
				return -EINVAL;
		}
		else
		{
			scale=1;
			xwidth=var->xres;
			yheight=var->yres;
			var->xres_virtual = var->xres > 256 ? 512 : 256;
			var->yres_virtual = var->xres_virtual > 256 ? 256 : 512;
		}
	}
	else
	{
		/*
		 *  FB_VMODE_CONUPDATE and FB_VMODE_SMOOTH_XPAN are equal!
		 *  as FB_VMODE_SMOOTH_XPAN is only used internally
		 */

		if (var->vmode & FB_VMODE_CONUPDATE) {
			var->vmode |= FB_VMODE_YWRAP;
			var->xoffset = info->var.xoffset;
			var->yoffset = info->var.yoffset;
		}

		scale=0;
		if (var->xres != 256)
			return -EINVAL;
		if (var->yres != 192)
			return -EINVAL;
		if (info->par != 0) {
			var->xres_virtual = 256;
			var->yres_virtual = 256;
		} else {
			if (var->xres_virtual > 256) {
				var->xres_virtual = 512;
				var->yres_virtual = 256;
			} else {
				var->xres_virtual = 256;
				var->yres_virtual = var->yres_virtual > 256 ? 512 : 256;
			}
		}

		if (var->bits_per_pixel != 16 && var->bits_per_pixel != 8)
		return -EINVAL;

		if (var->xres_virtual < var->xoffset + var->xres)
			return -EINVAL;
		if (var->yres_virtual < var->yoffset + var->yres)
			return -EINVAL;

	}





	switch (var->bits_per_pixel) {
	case 8:
		var->red.length = 5;
		var->green.length = 5;
		var->blue.length = 5;
		var->transp.length = 0;
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
	if (info->var.bits_per_pixel == 16)
		info->fix.visual = FB_VISUAL_TRUECOLOR;
	else
		info->fix.visual = FB_VISUAL_PSEUDOCOLOR;

	info->fix.line_length =
	    info->var.xres_virtual * (info->var.bits_per_pixel / 8);
	info->fix.smem_start = (unsigned)info->screen_base;
	info->fix.smem_len = info->var.xres_virtual * info->var.yres_virtual *
	    (info->var.bits_per_pixel / 8);

	if (info->par == 0) {
		if (info->var.yres_virtual == 512) {
			DISPLAY_CR =
			    MODE_5_2D | DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE;
			if (info->fix.visual == FB_VISUAL_TRUECOLOR) {
				writeb( VRAM_ENABLE | VRAM_B_MAIN_BG_0x6020000, VRAM_B_CR);
				BG3_CR = BG_BMP16_256x256 | BG_BMP_BASE(8);
			} else {
				BG3_CR = BG_BMP8_256x256 | BG_BMP_BASE(4);
			}

			BG3_XDX = 1 << 8;
			BG3_XDY = 0;
			BG3_YDX = 0;
			BG3_YDY = 1 << 8;
			BG3_CX = (info->var.xoffset) << 8;
			BG3_CY = (info->var.yoffset - 256) << 8;
		} else {
			DISPLAY_CR = MODE_5_2D | DISPLAY_BG2_ACTIVE;
		}
		writeb(VRAM_ENABLE | VRAM_A_MAIN_BG_0x6000000, VRAM_A_CR);
		if (info->var.xres_virtual == 512)
			if (info->fix.visual == FB_VISUAL_TRUECOLOR)
				BG2_CR = BG_BMP16_512x256 | BG_BMP_BASE(0);
			else
				BG2_CR = BG_BMP8_512x256 | BG_BMP_BASE(0);
		else if (info->fix.visual == FB_VISUAL_TRUECOLOR)
			BG2_CR = BG_BMP16_256x256 | BG_BMP_BASE(0);
		else
			BG2_CR = BG_BMP8_256x256 | BG_BMP_BASE(0);

		if (scale==1){
			BG2_XDX = ((xwidth / 256) << 8) | (xwidth % 256) ; 
			BG2_YDY = ((yheight / 192) << 8) | ((yheight % 192) + (yheight % 192) / 3) ;
			BG2_XDY = 0;
			BG2_YDX = 0;
			BG2_CX  = 0;
			BG2_CY  = 0;
		}
		else
		{
			BG2_XDX = 1 << 8;
			BG2_YDY = 1 << 8;
			BG2_XDY = 0;
			BG2_YDX = 0;
			BG2_CX = (info->var.xoffset) << 8;
			BG2_CY = (info->var.yoffset) << 8;
		}

	} else {
		SUB_DISPLAY_CR = MODE_5_2D | DISPLAY_BG3_ACTIVE;
		writeb(VRAM_ENABLE | VRAM_C_SUB_BG_0x6200000, VRAM_C_CR);

		if (info->fix.visual == FB_VISUAL_TRUECOLOR)
			SUB_BG3_CR = BG_BMP16_256x256;
		else
			SUB_BG3_CR = BG_BMP8_256x256;

		if (info->var.vmode & FB_VMODE_YWRAP)
			SUB_BG3_CR |= BG_WRAP_ON;

		SUB_BG3_XDX = 1 << 8;
		SUB_BG3_XDY = 0;
		SUB_BG3_YDX = 0;
		SUB_BG3_YDY = 1 << 8;
		SUB_BG3_CX = (info->var.xoffset) << 8;
		SUB_BG3_CY = (info->var.yoffset) << 8;
	}

	return 0;
}

static int ndsfb_blank(int blank_mode, struct fb_info *info)
{
	/*
	 * This is now handled by drivers/video/backlight/ndsbl.c
	 */
	return 0;
}

static int ndsfb_get_vblank(struct fb_vblank *vblank)
{
	memset(vblank, 0, sizeof(*vblank));
	vblank->flags = FB_VBLANK_HAVE_VCOUNT;
	vblank->vcount = readw(VCOUNT);
	return 0;
}

    /*
     * ioctl
     */

static int ndsfb_ioctl(struct inode *inode, struct file *file,
			  unsigned int cmd, unsigned long arg,
			  struct fb_info *info)
{
	void __user *argp = (void __user *)arg;
 
	switch (cmd) {
		case FBIOGET_VBLANK:
			{
				struct fb_vblank vblank;
				int err;

				err = ndsfb_get_vblank(&vblank);
				if (err)
					return err;
				if (copy_to_user(argp, &vblank, sizeof(vblank)))
					return -EFAULT;
				return 0;
			}
	}
	return -ENOTTY;
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

	if (regno >= 256)
		return 1;

#define CNVT_TOHW(val,width) ((((val)<<(width))+0x7FFF-(val))>>16)
	red = CNVT_TOHW(red, info->var.red.length);
	green = CNVT_TOHW(green, info->var.green.length);
	blue = CNVT_TOHW(blue, info->var.blue.length);
	transp = CNVT_TOHW(transp, info->var.transp.length);
#undef CNVT_TOHW

	/* Truecolor has hardware independent palette */

	if (info->fix.visual == FB_VISUAL_TRUECOLOR) {
		if (regno >= 16)
			return 1;
		v = (red << info->var.red.offset) |
		    (green << info->var.green.offset) |
		    (blue << info->var.blue.offset) |
		    (1 << info->var.transp.offset);
		((u32 *) (info->pseudo_palette))[regno] = v;
	} else {
		v = (red << info->var.red.offset) |
		    (green << info->var.green.offset) |
		    (blue << info->var.blue.offset);

		if (info->par == 0)
			PALETTE[regno] = v;
		else
			PALETTE_SUB[regno] = v;
	}

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
		    || var->yoffset >= info->var.yres_virtual || var->xoffset)
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

	ndsfb_set_par(info);

	return 0;
}

    /*
     *  We can't do a proper mmap without an mmu, so just give
     *  a pointer to the vram.
     */
unsigned long get_fb_unmapped_area(struct file *filp, unsigned long addr,
				   unsigned long x1, unsigned long x2,
				   unsigned long x3)
{
	return addr;
}

static int ndsfb_mmap(struct fb_info *info, struct file *file,
		      struct vm_area_struct *vma)
{
	vma->vm_start = info->fix.smem_start;

	return 0;
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


static int ndsfb_open(struct fb_info *info, int user)
{
	struct platform_device *dev = to_platform_device(info->device);
	if (dev->id == 1) {
		if (!opencount++)
			ndstouch_open_fb1();
	}
	return 0;
}

static int ndsfb_release(struct fb_info *info, int user)
{
	// This is called when the reference count goes to zero.
	struct platform_device *dev = to_platform_device(info->device);
	if (dev->id == 1) {
		if (!--opencount)
			ndstouch_close_fb1();
	}
	return 0;
}

    /*
     *  Initialisation
     */

static void ndsfb_platform_release(struct device *device)
{
}

static int __init ndsfb_probe(struct device *device)
{
	struct platform_device *dev = to_platform_device(device);
	struct fb_info *info;
	int retval = -ENOMEM;

	info = framebuffer_alloc(sizeof(u32) * 256, &dev->dev);
	if (!info)
		goto err;

	info->screen_base =
	    dev->id == 0 ? (void *)0x06000000 : (void *)0x06200000;
	info->fbops = &ndsfb_ops;

	retval = fb_find_mode(&info->var, info, NULL, NULL, 0, NULL, 16);

	if (!retval || (retval == 4))
		info->var = ndsfb_default;
	info->fix = ndsfb_fix;
	info->pseudo_palette = info->par;
	info->par = (void *)dev->id;
	info->flags = FBINFO_FLAG_DEFAULT
	    | FBINFO_PARTIAL_PAN_OK
	    | FBINFO_HWACCEL_XPAN | FBINFO_HWACCEL_YPAN | FBINFO_HWACCEL_YWRAP;

	retval = fb_alloc_cmap(&info->cmap, 256, 0);
	if (retval < 0)
		goto err1;

	retval = register_framebuffer(info);
	if (retval < 0)
		goto err2;
	dev_set_drvdata(&dev->dev, info);

	//request_irq(IRQ_VBLANK, ndsfb_interrupt, SA_SHIRQ, "ndsfb", device);

	printk(KERN_INFO "fb%d: Nintendo DS frame buffer device\n", info->node);
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
		//free_irq(IRQ_VBLANK, device);
	}
	return 0;
}

static struct device_driver ndsfb_driver = {
	.name = "ndsfb",
	.bus = &platform_bus_type,
	.probe = ndsfb_probe,
	.remove = ndsfb_remove,
};

static struct platform_device ndsfb_device0 = {
	.name = "ndsfb",
	.id = 0,
	.dev = {
		.release = ndsfb_platform_release,
		}
};

static struct platform_device ndsfb_device1 = {
	.name = "ndsfb",
	.id = 1,
	.dev = {
		.release = ndsfb_platform_release,
		}
};

int __init ndsfb_init(void)
{
	int ret = 0;
	struct fb_info *info;

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

	if (!ret) {
		info = dev_get_drvdata(&ndsfb_device1.dev);
		fb_set_var(info, &info->var);
		fb_prepare_logo(info);
		fb_show_logo(info);
	}

	return ret;
}

module_init(ndsfb_init);
