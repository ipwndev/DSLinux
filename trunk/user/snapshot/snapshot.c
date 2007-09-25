
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define ERROR (-1)
#define OK 0

#define WIDTH 	256
#define HEIGHT 	192

 /* The png_jmpbuf() macro, used in error handling, became available in
  * libpng version 1.0.6.  If you want to be able to run your code with older
  * versions of libpng, you must define the macro yourself (but only if it
  * is not already defined by libpng!).
  */

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

/* write a png file */
static int write_png(char *file_name, char *framebuffer)
{
   FILE *fp;
   int bdev = -1;
   png_structp png_ptr;
   png_infop info_ptr;

   /* open the file */
   fp = fopen(file_name, "w");
   if (fp == NULL)
      return (ERROR);

   /* open the framebuffer */
   bdev = open(framebuffer, O_RDONLY);
   if (bdev < 0) {
      fclose(fp);
      return (ERROR);
   }

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also check that
    * the library version is compatible with the one used at compile time,
    * in case we are using dynamically linked libraries.  REQUIRED.
    */
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

   if (png_ptr == NULL)
   {
      fclose(fp);
      close(bdev);
      return (ERROR);
   }

   /* Allocate/initialize the image information data.  REQUIRED */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      close(bdev);
      png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
      return (ERROR);
   }

   /* Set error handling.  REQUIRED if you aren't supplying your own
    * error handling functions in the png_create_write_struct() call.
    */
   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* If we get here, we had a problem reading the file */
      fclose(fp);
      close(bdev);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return (ERROR);
   }

   /* set up the output control if you are using standard C streams */
   png_init_io(png_ptr, fp);

   /* Set the image information here.  Width and height are up to 2^31,
    * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
    * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
    * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
    * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
    * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
    * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
    */
    png_set_IHDR(png_ptr, info_ptr, WIDTH, HEIGHT, 8, PNG_COLOR_TYPE_RGB,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

   /* Write the file header information.  REQUIRED */
   png_write_info(png_ptr, info_ptr);

   /* The easiest way to write the image (you may have a different memory
    * layout, however, so choose what fits your needs best).  You need to
    * use the first method if you aren't handling interlacing yourself.
    */
   {
   	png_uint_32 x, y;
   	png_byte image[HEIGHT][WIDTH*3];
   	png_bytep row_pointers[HEIGHT];
   	for (y = 0; y < HEIGHT; y++)
     	    row_pointers[y] = &image[y][0];

	/* read the image */
   	for (y = 0; y < HEIGHT; y++) {
	    png_bytep p = row_pointers[y];	    
	    for (x = 0; x < WIDTH; x++) {
		png_byte r,g,b;
		unsigned short value;
		read(bdev, &value, sizeof(value));
		r = (value << 3) & 0xf8;
		g = (value >> 2) & 0xf8;
		b = (value >> 7) & 0xf8;
		*p++ = r;
		*p++ = g;
		*p++ = b;
	    }
	}
	/* write out the image data by one or more scanlines */
   	for (y = 0; y < HEIGHT; y++)
   	{
      	    png_write_rows(png_ptr, &row_pointers[y], 1);
   	}	
   }
   /* It is REQUIRED to call this to finish writing the rest of the file */
   png_write_end(png_ptr, info_ptr);

   /* clean up after the write, and free any memory allocated */
   png_destroy_write_struct(&png_ptr, &info_ptr);

   /* close the file */
   fclose(fp);
   close(bdev);

   /* that's it */
   return (OK);
}


int main (int argc, char *argv[])
{
	if (argc < 3) {
		// print out help text
		printf("Simple program to dump the contents of the\n");
		printf("(16bit) DSLINUX framebuffer into a PNG file.\n");
		printf("\n");
		printf("Usage: snapshot /dev/fb1 file.png\n");
		return EXIT_FAILURE;
	}
	if (write_png(argv[2], argv[1]))
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}









