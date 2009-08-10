/*
 *  (C) Copyright 2008-2009 Kamil Kopec <kamil_kopec@poczta.onet.pl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/fb.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>

#include "types.h"
#include "framebuffer.h"

/* we don't use palette for 16-bit mode
 * for 8-bit palette is always 8
 * for 4-bit palette is 2
 * other pals are ridiculus */
#define fb_palette_colors (1<<var_info.bits_per_pixel)

/* variable represents screen memory */
fb_pixel * screen;

Uint32 buffer_size;

/* line of fb in bytes */
Uint32 fb_line_size;

/* palette allocated? */
Uint8 have_cmap = 0;

/* screen resolution */
Uint16 fb_xres, fb_yres;

/** framebuffer device descriptor */
Sint32 fb_dev;

/** struct with framebuffer informations */
struct fb_var_screeninfo var_info;

/** more framebuffer informations */
struct fb_fix_screeninfo fb_info;

/** pixelsize by default is 2 */
Uint8 pixelsize = 2;

/** old palette of framebuffer */
struct palette old_palette;

/** palette, that our application will use */
struct palette global_pal;

struct palette
{
	unsigned short *red;
	unsigned short *green;
	unsigned short *blue;
};


/**
 * Allocates memory for the palette.
 */
void alloc_palette(struct palette *pal)
{
	pal->red=malloc(sizeof(unsigned short)*fb_palette_colors);
	pal->green=malloc(sizeof(unsigned short)*fb_palette_colors);
	pal->blue=malloc(sizeof(unsigned short)*fb_palette_colors);

	if (!pal->red||!pal->green||!pal->blue) {
		/*internal("Cannot create palette.\n")*/;
	}
}

/**
 * Clears memory allocated for palette.
 */
void free_palette(struct palette *pal)
{
	free(pal->red);
	free(pal->green);
	free(pal->blue);
}

/**
 * Sets current palette to one given by parameter.
 */
void set_palette(struct palette *pal)
{
	struct fb_cmap cmap;
	int i;
	unsigned short *red=pal->red;
	unsigned short *green=pal->green;
	unsigned short *blue=pal->blue;
	__u16 *r, *g, *b, *t;

	r=malloc(fb_palette_colors*sizeof(__u16));
	g=malloc(fb_palette_colors*sizeof(__u16));
	b=malloc(fb_palette_colors*sizeof(__u16));
	t=malloc(fb_palette_colors*sizeof(__u16));

	if (!r||!g||!b||!t) {
		/*internal("Cannot allocate memory.\n")*/;
	}

	for (i = 0; i < fb_palette_colors; i++)
	{
	        r[i] = red[i];
	        g[i] = green[i];
	        b[i] = blue[i];
		/*fprintf(stderr, "%d %d %d\n", r[i], g[i], b[i]);*/
                /*fprintf(stderr, "%5x: %5x\t%5x\t%5x\t%5x\n",i,r[i],g[i],b[i],t[i]);*/

	}

	cmap.start = 0;
	cmap.len = fb_palette_colors;
	cmap.red = r;
	cmap.green = g;
	cmap.blue = b;
	cmap.transp = t;

	if ((ioctl(fb_dev, FBIOPUTCMAP, &cmap))==-1) {
		/*internal("Cannot set palette\n")*/;
	}

	free(r);free(g);free(b);free(t);
}

/**
 * Returns current allocated palette.
 */
void get_palette(struct palette *pal)
{
	struct fb_cmap cmap;
	int i;
	__u16 *r, *g, *b, *t;

	r=malloc(fb_palette_colors*sizeof(__u16));
	g=malloc(fb_palette_colors*sizeof(__u16));
	b=malloc(fb_palette_colors*sizeof(__u16));
	t=malloc(fb_palette_colors*sizeof(__u16));

	if (!r||!g||!b||!t) {
		/*internal("Cannot allocate memory.\n")*/;
	}

	cmap.start = 0;
	cmap.len = fb_palette_colors;
	cmap.red = r;
	cmap.green = g;
	cmap.blue = b;
	cmap.transp = t;

	if (ioctl(fb_dev, FBIOGETCMAP, &cmap)) {
		/*internal("Cannot get palette\n")*/;
	}

	for (i = 0; i < fb_palette_colors; i++)
	{
		/*printf("%d %d %d\n",r[i],g[i],b[i]);*/
	        pal->red[i] = r[i];
	        pal->green[i] = g[i];
	        pal->blue[i] = b[i];
	}

	free(r);free(g);free(b);free(t);
}


/* This is an empiric magic that ensures
 * Good white purity
 * Correct rounding and dithering prediction
 * And this is the cabbala:
 * 063 021 063 
 * 009 009 021
 * 255 085 255
 * 036 036 084
 */
/**
 * Tries to generate the best palette for the current framebuffer mode.
 */
static void generate_palette(struct palette *palette)
{
        int a;

	switch (var_info.bits_per_pixel)
	{
		case 4:
               	for (a=0;a<fb_palette_colors;a++)
                {
       	                palette->red[a]=(a&8)?65535:0;
               	        palette->green[a]=((a>>1)&3)*(65535/3);
                       	palette->blue[a]=(a&1)?65535:0;
		}
		break;
		case 8:
                for (a=0;a<fb_palette_colors;a++){
                       	palette->red[a]=((a>>5)&7)*(65535/7);
                        palette->green[a]=((a>>2)&7)*(65535/7);
       	                palette->blue[a]=(a&3)*(65535/3);
                }
		break;
		case 16:
                for (a=0;a<fb_palette_colors;a++){
			/*
                       	palette->red[a]=((a>>10)&31)*(65535/31);
                        palette->green[a]=((a>>5)&31)*(65535/31);
       	                palette->blue[a]=(a&31)*(65535/31);
			*/
                        palette->red[a]=
                        palette->green[a]=
                        palette->blue[a]=(((a&31)*255)/31)*257;
                }
		break;
		case 24:
                for (a=0;a<fb_palette_colors;a++){
			/*
                       	palette->red[a]=((a>>11)&31)*(65535/31);
                        palette->green[a]=((a>>5)&63)*(65535/63);
       	                palette->blue[a]=(a&31)*(65535/31);
			*/
                        palette->green[a]=(((a&63)*255)/64)*257;
                        palette->red[a]=
                        palette->blue[a]=(((a&31)*255)/32)*257;
                }
		break;
                default:
                for (a=0;a<fb_palette_colors;a++){
                        palette->red[a]=
                        palette->green[a]=
                        palette->blue[a]=a*257;
                        /* stuff it in both high and low byte */
                }
	}
};

/**
 * function causes that textcursor doesn't blink.
 */
inline void fb_hide_cursor(void)
{
	printf("\033[10000B\033[10000C\033[?25l");
	fflush(stdout);
};

/**
 * function causes that textcursor starts to blink.
 */
inline void fb_show_cursor(void)
{
	printf("\033[10000D\033[?25h");
	fflush(stdout);
};


/*
Function uses for debugging.
gives info about framebuffer state.
void fb_dump_info()
{
	printf ("xres: %d\n", var_info.xres);
	printf ("yres: %d\n", var_info.yres);
	printf ("xres_virtual: %d\n", var_info.xres_virtual);
	printf ("yres_virtual: %d\n", var_info.yres_virtual);
	printf ("xoffset: %d\n", var_info.xoffset);
	printf ("yoffset: %d\n", var_info.yoffset);
	printf ("bits_per_pixel: %d\n", var_info.bits_per_pixel);
	printf ("grayscale: %d\n", var_info.grayscale);
};
*/

/**
 * Initializes the framebuffer.
 * @return 0 if success, 1 if failure
 */
int fb_init()
{
	/* opening device file */
	if (!(fb_dev = open("/dev/fb0", O_RDWR)))
	{
		printf ("Unable to read framebuffer device: /dev/fb0\n");
		return 1;
	};

	/* reading screen informations */
	if (ioctl (fb_dev, FBIOGET_VSCREENINFO, &var_info))
	{
		printf ("Unable to read screen info.\n");
		printf ("Do you have read+write privilages to /dev/fb0?\n");
		close(fb_dev);
		return 2;
	};

/*	if ((ioctl (fb_dev, FBIOGET_FSCREENINFO, &fb_info))==-1)
	{
		printf ("Unable to read screen info.\n");
		printf ("Do you have read+write privilages to /dev/fb0?\n");
		close(fb_dev);
		return 2;
	}
*/
	var_info.xoffset = 0;
	var_info.yoffset = 0;

	if (ioctl (fb_dev, FBIOPUT_VSCREENINFO, &var_info))
	{
		printf ("Unable to change screen data\n");
		printf ("Do you have write privilages to /dev/fb0?\n");
		printf ("Does your video card support double buffering?\n");
		close(fb_dev);
		return 3;
	};

	switch (var_info.bits_per_pixel)
	{
		case 4: case 8:
			pixelsize = 1;
			break;
		case 5: case 15: case 16:
			pixelsize = 2;
			break;
		case 24:
			pixelsize = 3;
			break;
		case 32:
			pixelsize = 4;
			break;
		default:
			printf ("Unknown pixel format");
			return 3;
	};

	/* buffer_size = sizeof(screen) */
	buffer_size = var_info.xres * var_info.yres * var_info.bits_per_pixel / 8;

	/* mmap screen for framebuffer device */
	if (!(screen = (Uint8 *) mmap (NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_dev, 0)))
	{
		printf ("Unable to map shared memory\n");
		printf ("Make sure, that you have compiled shared memory support inot your kernel.\n");
		close(fb_dev);
		return 4;
	};

	/* do we need to allocate new palette? */
	if (var_info.bits_per_pixel < 1<<24)
	{
		have_cmap=1;
		alloc_palette(&old_palette);
		get_palette(&old_palette);

		alloc_palette(&global_pal);
		generate_palette(&global_pal);
		set_palette(&global_pal);
	}

	/* store resolutions data */
	fb_xres = var_info.xres;
	fb_yres = var_info.yres;
	fb_line_size = var_info.xres * pixelsize;

	/* hide console cursor */
	fb_hide_cursor();

	return 0;
}

/**
 * Uninitializes the framebuffer
 * @return always 0
 */
int fb_uninit()
{
	/* unmmap screen of framebuffer device */
	munmap(screen, buffer_size);

	/* have we changed the palette? */
	if (have_cmap) {
		/* if yes then restore the old one */
		set_palette(&old_palette);
		free_palette(&old_palette);
		free_palette(&global_pal);
	}
	close(fb_dev);

	/* show console cursor */
	fb_show_cursor();

	return 0;
}

/**
 * Set black colour to all pixels of the screen.
 * @param screen pointer to pixmap with width and height same as screen.
 * Usually is it variable screen.
 */
void fb_clear_screen(fb_pixel * screen)
{
	memset(screen, 0, buffer_size/*fb_xres*fb_yres*pixelsize*/);
}

/**
 * Paints a part of image on the screen. Id image is smaller than screen, function paints entire image.
 * @param fb_pixel pixmap of image to paint on the screen
 * @param width width od image
 * @param height height of image
 * @param pos_x position of the image to paint. Pixel pointed by pos_x will be drawn in the top, left corner.
 * @param pos_y position of the image to paint. Pixel pointed by pos_y will be drawn in the top, left corner.
 */
void fb_pixmap_render(fb_pixel * fb_pixmap, Uint16 width , Uint16 height, Uint16 pos_x, Uint16 pos_y)
{
	/* This function is invoked many, many times,
	 * so it is written as fast, as it was possible
	 */
	Uint32 j;
	Uint32 copy_mem_size;
	Uint32 bmp_line_size;
	Uint32 fb_line_size;
	Uint16 max_j;
	
	fb_pixel * tmp_screen;
	fb_pixel * tmp_bmp;
	
	bmp_line_size = width*pixelsize;
	fb_line_size = fb_xres*pixelsize;
	tmp_bmp = fb_pixmap + pos_y*bmp_line_size + pos_x*pixelsize - bmp_line_size;
	tmp_screen = screen - fb_line_size;
	copy_mem_size = (fb_xres > width ? bmp_line_size : fb_line_size);
	max_j = (fb_yres > height ? height : fb_yres);
	
	for(j = 0; j <  max_j ;j++)
		memcpy(tmp_screen+=fb_line_size, tmp_bmp+=bmp_line_size, copy_mem_size);
}

/**
 * Sets colour of pixel on the pixmap.
 * @param dest memory adress of pixel
 * @param R red value [0; 255];
 * @param G green value [0; 255];
 * @param B blue value [0; 255];
 */
inline void fb_set_pixel(fb_pixel * dest, Uint8 R, Uint8 G, Uint8 B)
{
	switch (pixelsize) {
	case 1:
		*(dest) = 0xe0 & R | 0x1c & (G>>3) | 0x03 & (B>>6);
		break;
	case 2:
		*(dest+1) = 0x80 | 0x7c & (B>>1) | 0x03 & (G>>6);
		*(dest) = 0xe0 & (G<<2) | 0x1f & (R>>3);
		break;
	case 3: case 4:
		*(dest) = B;
		*(dest+1) = G;
		*(dest+2) = R;
	};
}

/**
 * Reads colour of pixel and saves it to RGB variables.
 * @param dest memory adress of pixel
 * @param R red value [0; 255];
 * @param G green value [0; 255];
 * @param B blue value [0; 255];
 */
inline void fb_get_pixel(fb_pixel * dest, Uint8 * R, Uint8 * G, Uint8 * B)
{
	switch (pixelsize) {
	case 1:
		*R = *(dest) & 0xe0;
		*G = (*(dest) & 0x1c) << 3;
		*B = (*(dest) & 0x03) << 6;
		break;
	case 2:
		*R = ((*dest) & 0x1f) << 3;
		*G = (((*(dest+1)) & 0x03) << 6) | (((*dest) & 0xe0) >> 2);
		*B = ((*(dest+1)) & 0x7c) << 1;
		break;
	case 3: case 4:
		*B = *(dest);
		*G = *(dest+1);
		*R = *(dest+2);
	};
}
