/*********************************************************************
  libvideogfx/graphics/filters/linear.hh

  purpose:
    linear filters:
    - LowPass ( 0 1 0 / 1 4 1 / 0 1 0 )
    - LowPass ( 1 1 1 / 1 1 1 / 1 1 1 )

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    21/Jul/1999 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_FILTERS_LINEAR_HH
#define LIBVIDEOGFX_GRAPHICS_FILTERS_LINEAR_HH

#include "libvideogfx/graphics/basic/image.hh"
#include "libvideogfx/containers/array.hh"


/* Low-pass filtering is done using this kernel:

   1   /  0  1  0  \
   - * |  1  4  1  |
   8   \  0  1  0  /
*/
void LowPass_5pt(const Image_YUV<Pixel>& img,Image_YUV<Pixel>& dest);


/* Low-pass filtering is done using this kernel:

   1   /  1  1  1  \
   - * |  1  1  1  |
   9   \  1  1  1  /
*/
void LowPass_3x3mean(const Image_YUV<Pixel>& img,Image_YUV<Pixel>& dest);


template <class Pel> void ConvolveH(const Bitmap<Pel>& src,Bitmap<Pel>& dst,
				    const Array<float>& filter,bool useborder=true);
template <class Pel> void ConvolveV(const Bitmap<Pel>& src,Bitmap<Pel>& dst,
				    const Array<float>& filter,bool useborder=true);
template <class Pel> void ConvolveHV(const Bitmap<Pel>& src,Bitmap<Pel>& dst,
				     const Array<float>& filter,bool useborder=true);

void NormalizeFilter(Array<float>& filter); // Make coefficients sum up to 1.0 .


void CreateGaussFilter     (Array<float>& filter,float sigma,float cutoffval=0.01,bool normalize=true);
void CreateGaussDerivFilter(Array<float>& filter,float sigma,float cutoffval=0.01);

#endif
