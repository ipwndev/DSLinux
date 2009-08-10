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

#define USE_GPM

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* gpm support */
#ifdef USE_GPM
#include "ds_gpm.h"
#include "ds_gpm.c"
#endif

/* header, that specifies variable sizes */
#include "types.h"

/* framebuffer support */
#include "framebuffer.h"

/* pictures manipulating functions */
#include "pixmap.h"

/* keyboard support */
#include "keyboard.h"

/* bitmap codec */
#include "ds_bmp.h"

/* png file codec */
#include "ds_png.h"

/* jpeg file codec */
#include "ds_jpeg.h"

/* i don't know why this programs has seg-faults on arm,
 * when not compiled as a one huge file? weird...
 * On my home pc everything works perfect,
 * and on mobile device not. :-(
 */
#include "framebuffer.c"
#include "pixmap.c"
#include "keyboard.c"
#include "ds_bmp.c"
#include "ds_png.c"
#include "ds_jpeg.c"

#define JUMP_SIZE 2

/** multimedia file name; f.e. "image.bmp" */
char * filename = NULL;

/** if != NULL then dumping bmp file there */
char * dump_filename = NULL;

/** pointer to codec function */
int (*image_codec)(char *, fb_pixel **, Uint16 *, Uint16 *);

/** bool for quiet mode (not viewing image) */
unsigned char quiet = 0;

/** scale value, 1.00 by default */
float scale = 1.00;

/** rotation value, allowed only 90 * x */
Sint16 rotate = 0;

/** bool for autoscalling mode - fit to screen size */
char autoscale = 0;

/** no gpm mode, if gpm library not included, then leave it */
#ifdef GPM_SUPPORT
char no_gpm = 0;
#else
char no_gpm = 1;
#endif

/**
 * functions decodes parameters
 * set config variables, filename and extension, and so on.
 * @return 0 if parameters are correct, 1 if not.
 */
int decode_params(int argc, char ** argv)
{
	char * ext_start;
	char * extension;
	char i;

	for (i = 1; i < argc; i++) {

		if (!strcmp(*(argv+i), "--help") || !strcmp(*(argv+i), "-help") || !strcmp(*(argv+i), "-h") || !strcmp(*(argv+i), "--h")) {
			printf ("fbview [-s float][-a][-r int][-d file][-m] file\n\n\
Parameters: \n\
-s --scale float        bitmap scale; 0.01 <= scale <= 10.00\n\
-a --autoscale          fit size of bitmap to screen size\n\
-r --rotate integer     rotate; rotate = {0, 90, 180, 270}\n\
-d --dump filename      dump buffer to bmp file\n\
-q --quiet              quiet mode\n\
-m --no-gpm             disable gpm support\n\
\n\
While viewing image use arrows or A, S, D, W buttons\n\
for scrolling and ESC button for quit\n\
\n\
You may change colour depth; use command:\n\
fbset -depth 16 -n\n\n");

return 1;

		} else

		if (!strcmp(*(argv+i), "--scale") || !strcmp(*(argv+i), "-s")) {
			if (argc <= i+1) {
				printf ("Bad parameter. After --scale option must follow float value.\n");
				return 1;
			}

			scale = atof(*(argv+(++i)));
			if (scale < 0.01 || scale > 10.00) {
				printf ("Scale value sholud be 0.01 <= scale <= 10.00\n");
				return 1;
			}
		} else

		if (!strcmp(*(argv+i), "--autoscale") || !strcmp(*(argv+i), "-a")) {
			autoscale = 1;
		} else

		if (!strcmp(*(argv+i), "--rotate") || !strcmp(*(argv+i), "-r")) {
			if (argc <= i+1) {
				printf ("Bad parameter. After --rotate option must follow integer value.\n");
				return 1;
			}

			rotate = atoi(*(argv+(++i)));
			if (rotate < 0) rotate = - rotate + 180;
			rotate %= 360;
			if (rotate % 90 != 0) {
				printf ("Only 90*x rotation supported\n");
				return 1;
			}
		} else

		if (!strcmp(*(argv+i), "--dump") || !strcmp(*(argv+i), "-d")) {
			if (argc <= i+1) {
				printf ("Bad parameter. After --dump option must follow file name.\n");
				return 1;
			}

			dump_filename = (char *)malloc(strlen(*(argv+(++i))) + 1);
			if (!dump_filename) {
				printf ("Error with memory allocation.\n");
				return 1;
			}

			strcpy (dump_filename, *(argv+i));
		} else

		if (!strcmp(*(argv+i), "--quiet") || !strcmp(*(argv+i), "-q")) {
			quiet = 1;
			no_gpm = 1;
		} else

		#ifdef GPM_SUPPORT
		if (!strcmp(*(argv+i), "--no-gpm") || !strcmp(*(argv+i), "-m")) {
			no_gpm = 1;
		} else
		#endif

		{
		/* any other parameter is source file */

			filename = (char *)malloc(strlen(*(argv+i)) + 1);
			if (!filename) {
				printf ("Error with memory allocation.\n");
				return 1;
			}
			strcpy (filename, *(argv+i));
		}
	};

	if (!filename) {
		printf ("No image file given \n");
		return 1;
	}

	ext_start = strrchr(filename, '.');
	if (ext_start == NULL) {
		printf ("Image file has no extension\n");
		return 1;
	}

	extension = (char *)malloc(strlen(ext_start));
	if (!extension) {
		printf ("Error with memory allocation.\n");
		return 1;
	}
	strcpy(extension, ext_start+1);

	for (i=0; *(extension+i); i++) *(extension+i) = toupper(*(extension+i));

	#ifdef BMP_SUPPORT
	if (strcmp(extension, "BMP") == 0) {
		printf ("File decoded as \"Bitmap Image File\".\n\n");
		image_codec = bmp2fb_pixmap;
	} else
	#endif
	#ifdef PNG_SUPPORT
	if (strcmp(extension, "PNG") == 0) {
		printf ("File decoded as \"PNG Image File\".\n\n");
		image_codec = png2fb_pixmap;
	} else
	#endif
	#ifdef JPEG_SUPPORT
		if (strcmp(extension, "JPG") == 0 || strcmp(extension, "JPEG") == 0) {
		printf ("File decoded as \"JPEG Image File\".\n\n");
		image_codec = jpeg2fb_pixmap;
	} else
	#endif
		image_codec = NULL;

	if (!image_codec) {
		printf ("Unknown image extension\n");
		return 1;
	}

	free(extension);
	return 0;
}

/* -------------------------------------------------------------- */

/**
 * function that is running while viewing image.
 * There is the main loop of the application.
 * @return always 0.
 */
int fb_pixmap_play(fb_pixel * fb_pixmap, Uint16 width, Uint16 height)
{
	char ch;
	float gpm_x, gpm_y;
	Uint16 pos_x = 0;
	Uint16 pos_y = 0;
	
	fb_clear_screen(screen);
	fb_pixmap_render(fb_pixmap, width, height, pos_x, pos_y);
	
	init_keyboard();
	#ifdef GPM_SUPPORT
	/* the easiest way to flush mouse input. */
	if (!no_gpm) gpm_is_moved();
	#endif

	ch=0;
	while (1)
	{
		if (kbhit())
		{
		ch = readch();
	
		if (ch == DS_KEY_ESC) break;
		if ((ch == DS_KEY_UP || ch == DS_ARROW_UP) && pos_y >= JUMP_SIZE) pos_y-=JUMP_SIZE;
		if ((ch == DS_KEY_DOWN || ch == DS_ARROW_DOWN) && fb_yres <= (height - pos_y-JUMP_SIZE)) pos_y+=JUMP_SIZE;
		if ((ch == DS_KEY_LEFT || ch == DS_ARROW_LEFT) && pos_x >= JUMP_SIZE) pos_x-=JUMP_SIZE;
		if ((ch == DS_KEY_RIGHT || ch == DS_ARROW_RIGHT) && fb_xres <= (width - pos_x-JUMP_SIZE)) pos_x+=JUMP_SIZE;
		ch = 0;
		fb_pixmap_render(fb_pixmap, width, height, pos_x, pos_y);
		}
	
		#ifdef GPM_SUPPORT
		if (!no_gpm && gpm_is_moved()) {
			gpm_get_position(&gpm_x, &gpm_y);
			if (height > fb_yres) pos_y = (height - fb_yres) * gpm_y;
			if (width > fb_xres) pos_x = (width - fb_xres) * gpm_x;
			fb_pixmap_render(fb_pixmap, width, height, pos_x, pos_y);
		}
		#endif
	}

	close_keyboard();
	fflush(stdin);
	
	fb_clear_screen(screen);
	return 0;
}

/* -------------------------------------------------------------- */

int main(int argc, char ** argv)
{
	/* image sizes and map of pixels */
	Uint16 width;
	Uint16 height;
	fb_pixel * fb_pixmap = NULL;
	
	printf ("fbview-1.03 - GPL frame buffer image viewer.\n");
	printf ("Supportet file formats: BMP-24bit, PNG, JPEG\n");
	printf ("written by galactic, 2008, 2009\n\n");
	
	if (decode_params(argc, argv)) {
		return 1;
	}
	
	/* if quiet mode, then there is no need to load framebuffer */
	if (!quiet && fb_init()) {
		printf ("Unable to init framebuffer device\n");
		return 1;
	}
	
	#ifdef GPM_SUPPORT
	if (!no_gpm) {
		if (gpm_init()) {
		printf ("Unable to init gpm device. no-gpm mode forced.\n");
		no_gpm = 1;
		}
	}
	#endif
	
	/* decode image using suitable codec */
	if ((* image_codec)(filename, &fb_pixmap, &width, &height)) {
		printf ("Error while loading image file.\n");
		if (!quiet) fb_uninit();
		if (filename) free(filename);
		if (dump_filename) free(dump_filename);
		return 1;
	}
	
	/* if image is not loaded (=NULL), then show message and exit. */
	if (fb_pixmap == NULL) {
		printf ("Image file loaded not correctly.\n");
		if (!quiet) fb_uninit();
		if (filename) free(filename);
		if (dump_filename) free(dump_filename);
		return 1;
	}
	
	/* execute image manipulation functions */
	if (autoscale) scale = ((float)fb_xres / fb_yres > (float)width / height) ? (float)fb_yres / height : (float)fb_xres / width;
	if (scale != 1.00) fb_pixmap_scale(&fb_pixmap, &width, &height, scale);
	if (rotate != 0) fb_pixmap_rotate(&fb_pixmap, &width, &height, rotate);
	if (dump_filename != NULL) fb_pixmap2bmp(dump_filename, fb_pixmap, width, height);
	
	/* if not quiet mode, then run viewing */
	if (!quiet) {
		fb_pixmap_play(fb_pixmap, width, height);
	
		#ifdef GPM_SUPPORT
		if (!no_gpm) gpm_uninit();
		#endif
	
		fb_uninit();
	}
	
	/* clean the trash */
	if (filename) free(filename);
	if (dump_filename) free(dump_filename);
	free(fb_pixmap);
	
	return 0;
}
