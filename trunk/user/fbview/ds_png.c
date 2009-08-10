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

#include <stdio.h>
#include <malloc.h>
#include <png.h>

#include "ds_png.h"

/* Read a PNG file.  You may want to return an error code if the read
 * fails (depending upon the failure).  There are two "prototypes" given
 * here - one where we are given the filename, and we need to open the
 * file, and the other where we are given an open file (possibly with
 * some or all of the magic bytes read - see comments above).
 */

/**
 * load png file to fb_pixmap
 * @param filename name of bmp file on the disk to load
 * @param fb_pixmap pointer to the memory for pixmap
 * @param w width of loaded image
 * @param h height of loaded image
 * @return 0 if success, 1 if failure.
 */
int png2fb_pixmap(char * filename, fb_pixel ** fb_pixmap, Uint16 * w, Uint16 * h)  /* We need to open the file */
{
	/* Memory to place the raw image data */
	fb_pixel ** pixel;
	unsigned short i,j;
	
	/* png file headers */
	png_structp png_ptr;
	png_infop info_ptr;

	unsigned int sig_read = 0;
	FILE *fp;

	/* Opening the image file */	
	if ((fp = fopen(filename, "rb")) == NULL) return 1;
	
	/* Create and initialize the png_struct with the desired error handler
	 * functions.  If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters.  We also supply the
	 * the compiler header file version, so that we know if the application
	 * was compiled with a compatible version of the library.  REQUIRED
	 */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	/* if image file is corrupt, return an error */
	if (png_ptr == NULL) {
		fclose(fp);
		return -1;
	}
	
	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return 1;
	}
	
	/* Set error handling if you are using the setjmp/longjmp method (this is
	 * the normal method of doing things with libpng).  REQUIRED unless you
	 * set up your own error handlers in the png_create_read_struct() earlier.
	 */
	if (setjmp(png_jmpbuf(png_ptr))) {

		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);

		/* If we get here, we had a problem reading the file */
		return -1;
	}
	
	/* One of the following I/O initialization methods is REQUIRED
	 * Set up the input control if you are using standard C streams */
	png_init_io(png_ptr, fp);
	
	/* If we have already read some of the signature */
	png_set_sig_bytes(png_ptr, sig_read);
	
	/* If you have enough memory to read in the entire image at once,
	 * and you need to specify only transforms that can be controlled
	 * with one of the PNG_TRANSFORM_* bits (this presently excludes
	 * dithering, filling, setting background, and doing gamma
	 * adjustment), then you can read the entire image (including
	 * pixels) into the info structure with this call:
	 */
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_ALPHA | PNG_TRANSFORM_STRIP_16, NULL);
	
	/* At this point you have read the entire image */
	
	/* save width and height of image in the variables given by parameter */
	*w = info_ptr->width;
	*h = info_ptr->height;
	
	/* Allocate memory for the image */
	*fb_pixmap = (fb_pixel*)malloc((*w) * (*h) * pixelsize);
	if ((*fb_pixmap)==NULL)
	{
		/* error while allocating memory */
		fclose(fp);
		return -1;
	}
	
	/* pixel is an array of pointers to the pixel data for each row: */
	pixel = png_get_rows(png_ptr, info_ptr);
	
	/* Here we are reading pixel variable, and translate it into fb_pixmap.
	 * The simplest way is to use for loop.
	 */
	for (j = 0; j < (*h); j++)
	{
		for (i = 0; i < (*w); i++)
			fb_set_pixel((*fb_pixmap)+((i+j*(*w))*pixelsize), *((*pixel)+i*3+0), *((*pixel)+i*3+1), *((*pixel)+i*3+2));

		/* Go to next row */
		pixel++;
	}
	
	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	
	/* close the file */
	fclose(fp);
	
	/* that's it */
	return 0;
}

