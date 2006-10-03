/*********************************************************************
  fileio/rw_uyvy.hh

  purpose:
    Functions for loading and saving of UYVY files of any size.
    The image to write the image into will be created by
    ReadImage_UYVY() according to the specification given in
    the spec-argument. The chroma and nocolors fields need not
    be set. But note that the size of the saved image must be
    equal to the values in 'spec'.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
   15/Jul/1999 - Dirk Farin - completely rewritten
   18/Jun/9999 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_FILEIO_RW_UYVY_HH
#define LIBVIDEOGFX_GRAPHICS_FILEIO_RW_UYVY_HH

#include <fstream.h>

#include "libvideogfx/graphics/basic/image.hh"


// Return true if the image file size matches the size specified in sizespec.
bool CheckImageSize (ifstream& istr,const ImageInfo_Base& sizespec);

void ReadImage_UYVY (Image_YUV<Pixel>&,ifstream& istr,const ImageSpec_YUV& spec);
void WriteImage_UYVY(Image_YUV<Pixel>&,ofstream& ostr);

#endif
