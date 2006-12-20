/*********************************************************************
  libvideogfx/graphics/filters/resize.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    03/Aug/1999 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_FILTERS_RESIZE_HH
#define LIBVIDEOGFX_GRAPHICS_FILTERS_RESIZE_HH

#include "libvideogfx/graphics/basic/image.hh"

void HalfSize_Subsample(const Image_YUV<Pixel>& img,Image_YUV<Pixel>& dest);

#endif
