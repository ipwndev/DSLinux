
#include <stdio.h>
#include <malloc.h>
#include <png.h>

#include "ds_png.h"
#include "framebuffer.h"
#include "keyboard.h"


/* Read a PNG file.  You may want to return an error code if the read
 * fails (depending upon the failure).  There are two "prototypes" given
 * here - one where we are given the filename, and we need to open the
 * file, and the other where we are given an open file (possibly with
 * some or all of the magic bytes read - see comments above).
 */

unsigned int width;
unsigned int height;

unsigned int png_position_x;
unsigned int png_position_y;

fb_pixel * png_load(char *file_name)  /* We need to open the file */
{
   fb_pixel * bmp_buffer;
   fb_pixel ** pixel;
   unsigned short i,j;

   png_structp png_ptr;
   png_infop info_ptr;
   unsigned int sig_read = 0;
   int bit_depth, color_type, interlace_type;
   FILE *fp;

   if ((fp = fopen(file_name, "rb")) == NULL)
      return NULL;

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also supply the
    * the compiler header file version, so that we know if the application
    * was compiled with a compatible version of the library.  REQUIRED
    */
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

   if (png_ptr == NULL)
   {
      fclose(fp);
      return NULL;
   }

   /* Allocate/initialize the memory for image information.  REQUIRED. */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      return NULL;
   }

   /* Set error handling if you are using the setjmp/longjmp method (this is
    * the normal method of doing things with libpng).  REQUIRED unless you
    * set up your own error handlers in the png_create_read_struct() earlier.
    */

   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      fclose(fp);
      /* If we get here, we had a problem reading the file */
      return NULL;
   }

   /* One of the following I/O initialization methods is REQUIRED */
   /* Set up the input control if you are using standard C streams */
   png_init_io(png_ptr, fp);


   /* If we have already read some of the signature */
   png_set_sig_bytes(png_ptr, sig_read);

   /*
    * If you have enough memory to read in the entire image at once,
    * and you need to specify only transforms that can be controlled
    * with one of the PNG_TRANSFORM_* bits (this presently excludes
    * dithering, filling, setting background, and doing gamma
    * adjustment), then you can read the entire image (including
    * pixels) into the info structure with this call:
    */
   png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_ALPHA | PNG_TRANSFORM_STRIP_16, NULL);

   /* At this point you have read the entire image */


/*After you have called png_read_png(), you can retrieve the image data with

   row_pointers = png_get_rows(png_ptr, info_ptr);

where row_pointers is an array of pointers to the pixel data for each row:

   png_bytep row_pointers[height];

If you know your image size and pixel size ahead of time, you can allocate row_pointers prior to calling png_read_png() with

   row_pointers = png_malloc(png_ptr,
      height*sizeof(png_bytep));
   for (int i=0; i<height, i++)
      row_pointers[i]=png_malloc(png_ptr,
         width*pixel_size);
   png_set_rows(png_ptr, info_ptr, &row_pointers);

Alternatively you could allocate your image in one big block and define row_pointers[i] to point into the proper places in your block.

If you use png_set_rows(), the application is responsible for freeing row_pointers (and row_pointers[i], if they were separately allocated).

If you don't allocate row_pointers ahead of time, png_read_png() will do it, and it'll be free'ed when you call png_destroy_*().*/

    width = info_ptr->width;
    height = info_ptr->height;
    png_position_x = 0;
    png_position_y = 0;

    bmp_buffer = (fb_pixel*)malloc(width * height * pixelsize);
    if (bmp_buffer == NULL)
    {
	// error while allocating memory
	fclose(fp);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	return NULL;
    }

    pixel = png_get_rows(png_ptr, info_ptr);

    for (j = 0; j < height; j++)
    {
	for (i = 0; i < width; i++) {
	    fb_setpixel(bmp_buffer+((i+j*width)*pixelsize), *((*pixel)+i*3+0), *((*pixel)+i*3+1), *((*pixel)+i*3+2));
	}
        pixel++;
    }

   /* clean up after the read, and free any memory allocated - REQUIRED */
   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

   /* close the file */
   fclose(fp);

   /* that's it */
   return bmp_buffer;
}



void png_render(fb_pixel * bmp_buffer)
{
    Uint32 j;
    Uint32 copy_mem_size;
    Uint32 bmp_line_size;
    Uint32 fb_line_size;
    Uint16 max_j;

    fb_pixel * tmp_screen;
    fb_pixel * tmp_bmp;

    bmp_line_size = width*pixelsize;
    fb_line_size = fb_xres*pixelsize;
    tmp_bmp = bmp_buffer + png_position_y*bmp_line_size + png_position_x*pixelsize - bmp_line_size;
    tmp_screen = screen - fb_line_size;
    copy_mem_size = (fb_xres > width ? bmp_line_size : fb_line_size);
    max_j = (fb_yres > height ? height : fb_yres);

    for(j = 0; j <  max_j ;j++)
	memcpy(tmp_screen+=fb_line_size, tmp_bmp+=bmp_line_size, copy_mem_size);
}



/*
void png_free(fb_pixel * bmp_buffer)
{
    if (bmp_buffer) free(bmp_buffer);
}
*/



int png_play(char * filename)
{
    char ch;

    if (fb_init())
    {
	printf ("Unable to init framebuffer device\n");
	return 2;
    }

    fb_pixel * bmp_buffer;

    if ((bmp_buffer = png_load(filename)) == NULL)
    {
	fb_uninit();
	printf ("Error while reading png file\n");
	return 1;
    }

    fb_clear_screen(screen);

    png_render(bmp_buffer);

    init_keyboard();

    ch=0;
    while (1)
    {
	if (!kbhit()) 
	{
	    ch = readch();
	    if (ch == KEY_ESC) break;
	    if (ch == KEY_UP && png_position_y >= JUMP_SIZE) png_position_y-=JUMP_SIZE;
	    if (ch == KEY_DOWN && fb_yres <= (height-png_position_y-JUMP_SIZE)) png_position_y+=JUMP_SIZE;
	    if (ch == KEY_LEFT && png_position_x >= JUMP_SIZE) png_position_x-=JUMP_SIZE;
	    if (ch == KEY_RIGHT && fb_xres <= (width-png_position_x-JUMP_SIZE)) png_position_x+=JUMP_SIZE;
	    ch = 0;
	    png_render(bmp_buffer);
	}
    }

    close_keyboard();

    fflush(stdin);

    fb_clear_screen(screen);

    if (bmp_buffer) free(bmp_buffer);//png_free(bmp_buffer);
    fb_uninit();
    return 0;
}
