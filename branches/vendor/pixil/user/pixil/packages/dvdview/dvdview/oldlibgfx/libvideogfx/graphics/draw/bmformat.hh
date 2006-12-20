#/*********************************************************************
  libvideogfx/graphics/draw/bmformat.hh

  purpose:
    Functions for extending borders, change format of bitmaps...

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    04/Jul/2000 - Dirk Farin - bitmap format conversion and helpers
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_DRAW_BMFORMAT_HH
#define LIBVIDEOGFX_GRAPHICS_DRAW_BMFORMAT_HH

#include "libvideogfx/graphics/basic/bitmap.hh"
#include "libvideogfx/graphics/basic/image.hh"


template <class A,class B> void ConvertBitmap(const Bitmap<A>& src,Bitmap<B>& dst);

/* Copy a number of lines at the inner border of the image. */
template <class T> void CopyInnerBorder(const Image<T>& src,Image<T>& dst,int hwidth,int vwidth);

/* Creates a border of the specified with around the image and copies the border
   pixel values into it. */
template <class T> void ExtrudeIntoBorder(Image<T>&);
template <class T> void ExtrudeIntoBorder(Bitmap<T>&);

#define CopyImageIntoBorder  ExtrudeIntoBorder      // OBSOLET
#define CopyBitmapIntoBorder ExtrudeIntoBorder      // OBSOLET

template <class T> void SetBitmapBorder(Bitmap<T>&,const T& val);


template <class T> void RemoveColor(Image_YUV<T>&);
template <class T> void ConvertGreyToRGB(const Image_YUV<T>&,Image_RGB<T>&);

#endif
