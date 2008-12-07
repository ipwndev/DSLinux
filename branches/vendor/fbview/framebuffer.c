//#define debug

#include <linux/fb.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

//#ifdef debug
#include <stdio.h>
//#endif

#include "types.h"
#include "framebuffer.h"

// we don't use palette for 16-bit mode
// for 8-bit palette is always 8
// for 4-bit palette is 2
// other pals are ridiculus
#define fb_palette_colors (1<<var_info.bits_per_pixel)

fb_pixel * screen;
Uint32 buffer_size;
Uint32 fb_line_size;		// line of fb in bytes
Uint8 have_cmap = 0;		// palette allocated?

Uint16 fb_xres, fb_yres;		// screen resolution

Sint32 fb_dev;
struct fb_var_screeninfo var_info;
struct fb_fix_screeninfo fb_info;
Uint8 pixelsize;
struct palette old_palette;
struct palette global_pal;


struct palette
{
	unsigned short *red;
	unsigned short *green;
	unsigned short *blue;
};

void alloc_palette(struct palette *pal)
{
	pal->red=malloc(sizeof(unsigned short)*fb_palette_colors);
	pal->green=malloc(sizeof(unsigned short)*fb_palette_colors);
	pal->blue=malloc(sizeof(unsigned short)*fb_palette_colors);

	if (!pal->red||!pal->green||!pal->blue) {
		/*internal("Cannot create palette.\n")*/;
	}
}


void free_palette(struct palette *pal)
{
	free(pal->red);
	free(pal->green);
	free(pal->blue);
}


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
}

inline void fb_hide_cursor(void)
{
	printf("\033[10000B\033[10000C\033[?25l");
	fflush(stdout);
}

inline void fb_show_cursor(void)
{
	printf("\033[10000D\033[?25h");
	fflush(stdout);
}

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

int fb_init()
{
	if (!(fb_dev = open("/dev/fb0", O_RDWR)))
	{
		printf ("Unable to read framebuffer device: /dev/fb0\n");
		return 1;
	};

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
//	var_info.bits_per_pixel = 32;
//	var_info.yres_virtual = var_info.yres * 2;
//	if (ioctl (fb_dev, FBIOPAN_DISPLAY, &var_info))
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

	buffer_size = var_info.xres * var_info.yres * var_info.bits_per_pixel / 8;
	if (!(screen = (Uint8 *) mmap (NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_dev, 0)))
	{
		printf ("Unable to map shared memory\n");
		printf ("Make sure, that you have compiled shared memory support inot your kernel.\n");
		close(fb_dev);
		return 4;
	};

	// do we need to allocate new palette?
	if (/*fb_info.visual==FB_VISUAL_PSEUDOCOLOR && */var_info.bits_per_pixel < 1<<24)
	{
		have_cmap=1;
		alloc_palette(&old_palette);
		get_palette(&old_palette);

		alloc_palette(&global_pal);
		generate_palette(&global_pal);
		set_palette(&global_pal);
	}

	fb_xres = var_info.xres;
	fb_yres = var_info.yres;
	fb_line_size = var_info.xres * pixelsize;

	#ifdef debug
	fb_dump_info();
	#else
	fb_hide_cursor();
	#endif

	return 0;
}

int fb_uninit()
{
	munmap(screen, buffer_size);
	if (have_cmap)
	{
		set_palette(&old_palette);
		free_palette(&old_palette);
		free_palette(&global_pal);
	}
	close(fb_dev);

	#ifndef debug
	fb_show_cursor();
	#endif

	return 0;
}

void fb_clear_screen(fb_pixel * screen)
{
	memset(screen, 0, buffer_size/*fb_xres*fb_yres*pixelsize*/);
}

inline void fb_setpixel(fb_pixel * dest, Uint8 R, Uint8 G, Uint8 B)
{
	switch (pixelsize) {
	case 1:
		*(dest) = 0xe0 & R | 0x1c & (G>>3) | 0x03 & (B>>6);
		break;
	case 2:
		*(dest+1) = 0x80 | 0xec & (B>>1) | 0x03 & (G>>6);
		*(dest) = 0xe0 & (G<<2) | 0x1F & (R>>3);
		break;
	case 3: case 4:
		*(dest) = B;
		*(dest+1) = G;
		*(dest+2) = R;
	};
}
