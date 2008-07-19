/*********************************************************************
  libvideogfx/graphics/filters/nonlin.hh

  purpose:
    Nonlinear filters:
    - Median YUV 3x3 - only luminance information is used to choose the
                       pixel to be copied. The color information will
		       simply be copied accordingly.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    24/Aug/1999 - Dirk Farin - functions are not template functions
    21/Jul/1999 - Dirk Farin - cleanup and speed improvements of median filter
    29/Jun/1999 - Dirk Farin - first implementation of median filter
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_FILTERS_NONLIN_HH
#define LIBVIDEOGFX_GRAPHICS_FILTERS_NONLIN_HH

#include "libvideogfx/graphics/basic/image.hh"

template <class Pel> void Median_YUV_3x3(const Image_YUV<Pel>& img,Image_YUV<Pel>& dest);

#endif
