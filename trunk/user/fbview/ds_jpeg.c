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
#include <jpeglib.h>
#include <setjmp.h>
#include <stdlib.h>

#include "ds_jpeg.h"

#define my_error_exit NULL

/* error handler structure */
struct my_error_mgr {
	/* "public" fields */
	struct jpeg_error_mgr pub;
	
	/* for return to caller */
	jmp_buf setjmp_buffer;
};

/** global variable to the error handler structure */
typedef struct my_error_mgr * my_error_ptr;

/**
 * load jpeg file to fb_pixmap
 * @param filename name of bmp file on the disk to load
 * @param fb_pixmap pointer to the memory for pixmap
 * @param w width of loaded image
 * @param h height of loaded image
 * @return 0 if success, 1 if failure.
 */
int jpeg2fb_pixmap(char * filename, fb_pixel ** fb_pixmap, Uint16 * w, Uint16 * h)
{
	int i, j;
	
	/* This struct contains the JPEG decompression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	*/

	struct jpeg_decompress_struct cinfo;

	/* We use our private extension JPEG error handler.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct my_error_mgr jerr;

	/* source file */
	FILE * infile;

	/* Output row buffer */
	JSAMPARRAY buffer;

	/* physical row width in output buffer */
	int row_stride;
	
	/* In this example we want to open the input file before doing anything else,
	* so that the setjmp() error recovery below can assume the file is open.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to read binary files.
	*/
	if ((infile = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		return 1;
	}
	
	/* Step 1: allocate and initialize JPEG decompression object */
	
	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */

	if (setjmp(jerr.setjmp_buffer))
	{
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return 1;
	}

	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);
	
	/* Step 2: specify data source (eg, a file) */
	jpeg_stdio_src(&cinfo, infile);
	
	/* Step 3: read file parameters with jpeg_read_header() */
	(void) jpeg_read_header(&cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	*   (a) suspension is not possible with the stdio data source, and
	*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	* See libjpeg.doc for more info.
	*/
	
	/* Step 4: set parameters for decompression */
	
	/* In this example, we don't need to change any of the defaults set by
	* jpeg_read_header(), so we do nothing here.
	*/
	
	/* Step 5: Start decompressor */
	
	(void) jpeg_start_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/
	
	/* We may need to do some setup of our own at this point before reading
	* the data. After jpeg_start_decompress() we have the correct scaled
	* output image dimensions available, as well as the output colormap
	* if we asked for color quantization.
	* In this example, we need to make an output work buffer of the right size.
	*/

	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	
	/* Step 6: while (scan lines remain to be read) */
	
	/* Here we use the library's state variable cinfo.output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/
	*w = cinfo.output_width;
	*h = cinfo.output_height;
	
	*fb_pixmap = (fb_pixel*)malloc((*w) * (*h) * pixelsize);
	if ((*fb_pixmap)==NULL)
	{
		/* error while allocating memory */
		fclose(infile);
		(void)jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		return 1;
	}
	
	j = 0;
	
	/* We are reading each row of image, one by one */
	while (cinfo.output_scanline < cinfo.output_height) {

		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could ask for
		* more than one scanline at a time if that's more convenient.
		*/
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);

		/* Assume put_scanline_someplace wants a pointer and sample count. */
		
		for (i = 0; i < cinfo.output_width; i++) {
			/* Some of pointer magic, that allow me to create pixel in right framebuffer format */
			fb_set_pixel((*fb_pixmap)+((i+j*(*w))*pixelsize), *((*buffer)+i*3+0), *((*buffer)+i*3+1), *((*buffer)+i*3+2));
		}
		
		j++;
	}
	
	/* Step 7: Finish decompression */
	
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/
	(void) jpeg_finish_decompress(&cinfo);
	
	/* Step 8: Release JPEG decompression object */
	
	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);
	
	/* After finish_decompress, we can close the input file.
	* Here we postpone it until after no more JPEG errors are possible,
	* so as to simplify the setjmp error logic above.  (Actually, I don't
	* think that jpeg_destroy can do an error exit, but why assume anything...)
	*/
	fclose(infile);
	
	/* At this point you may want to check to see whether any corrupt-data
	* warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	*/
	
	/* And we're done! */
	return 0;
}

