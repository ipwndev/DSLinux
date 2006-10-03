/*********************************************************************
  fileio/writeppm.hh

  purpose:
    Functions to save RGB images into PPM P6-type (24bit binary)
    files and to save the luminance part of YUV images into PPM
    P5-type (8bit greyscale) files.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
   03/Aug/1999 - Dirk Farin - new functions: ReadImage_PPM()
                                             ReadImage_PPM5()
   29/Jul/1999 - Dirk Farin - files renamed to rw_ppm.*
                            - added ppm loading
   19/Jul/1999 - Dirk Farin - completely rewritten
   29/Jun/1999 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_FILEIO_RWPPM_HH
#define LIBVIDEOGFX_GRAPHICS_FILEIO_RWPPM_HH

#include <fstream.h>

#include "libvideogfx/graphics/basic/image.hh"


/* Reads PPM P5 and P6 types images.
 * If the 'srcimg' parameter is filled in, this image will be used for loading the file.
 *   It is a requirement that the image size is the same as the image file size.
 * If the 'spec' parameter is filled in, a new image will be created that uses most of
 *   the fields set in 'spec'. But width and height will be overwritten with the values from the file.
 * If none of both is specified, a completely new image without border and alignment is created.
 */
Image_RGB<Pixel> ReadImage_PPM (istream& stream,Image_RGB<Pixel>* srcimg=NULL,ImageSpec* spec=NULL);

Image_YUV<Pixel> ReadImage_PPM5(istream& stream,Image_YUV<Pixel>* srcimg=NULL,ImageSpec_YUV* spec=NULL);


/* Write RGB image tnto PPM P6-type file.
 */
void WriteImage_PPM6(const Image_RGB<Pixel>&,ostream& stream);

/* Write luminance part of YUV image into PPM P5-type file.
 * NOTE: the chrominance will simply be ignored.
 */
void WriteImage_PPM5(const Image_YUV<Pixel>&,ostream& stream);

#endif
