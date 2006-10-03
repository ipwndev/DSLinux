/********************************************************************************
  libvideogfx/graphics/basic/image.hh

  purpose:
    Data types for RGB and YUV images.

  notes:
   - Be careful when accessing the image data using more than one
     pointer at the same time (see bitmap.hh)

   - When extracting a bitmap from the image be sure to save a reference or
     a pointer to the bitmap and not use a bitmap object of its own:
     USE THIS: Bitmap<Pixel>* bm = &img.AskBitmap(...);
     NOT THIS: Bitmap<Pixel>  bm =  img.AskBitmap(...);
     The second version means that you extracted bitmap is independent of the
     image you took it from. So writing into the Bitmap will not have any
     effect on the image.

  to do:
   - Add Chroma411 format.
   - Add Image_YCbCr<T>.

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    18/Jul/2000 - Dirk Farin - new convenient bitmap access methods
    05/Nov/1999 - Dirk Farin - adapted comments to DOC++
    24/Aug/1999 - Dirk Farin - moved template instantiation into
      separate file to solve multiple defined functions
    20/Jul/1999 - Dirk Farin - moved 'border'-entry from
      ImageInfo_Alignment to ImageInfo_Base
    15/Jul/1999 - Dirk Farin - GetChromaSizes()
    12/Jul/1999 - Dirk Farin - class Image is now a base class. The
      most common user interface routines did move into the derived
      classes Image_RGB and Image_YUV.
    02/Jun/1999 - Dirk Farin - first implementation
 ********************************************************************************
    Copyright (C) 1999  Dirk Farin

    This program is distributed under GNU Public License (GPL) as
    outlined in the COPYING file that comes with the source distribution.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
********************************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_BASIC_IMAGE_HH
#define LIBVIDEOGFX_GRAPHICS_BASIC_IMAGE_HH

#include "libvideogfx/types.hh"

#include "bitmap.hh"


/// Chroma format of image.
enum ChromaFormat {
  /** Subsampling h:2 v:2 */ Chroma420,
  /** Subsampling h:2 v:1 */ Chroma422,
  /** No subsampling      */ Chroma444
};


/** Check if chroma is horizontally subsampled. Usage of the more general #ChromaSubH()# is recommended. */
inline bool IsSubH(ChromaFormat cf) { return cf != Chroma444; }
/** Check if chroma is vertically subsampled. Usage of the more general #ChromaSubV()# is recommended. */
inline bool IsSubV(ChromaFormat cf) { return cf == Chroma420; }
/** Get horizontal subsampling factor. */
inline int  ChromaSubH(ChromaFormat cf) { return (cf != Chroma444) ? 2 : 1; }
/** Get vertical subsampling factor. */
inline int  ChromaSubV(ChromaFormat cf) { return (cf == Chroma420) ? 2 : 1; }



// ------------------------ Image parameters -------------------------------

/** Base class for image parameters.
 */
class ImageInfo_Base
{
public:
  ImageInfo_Base()
    : width(0), height(0), border(0), has_alphamask(false) { }
  
  /// Image logical width.
  int  width;
  /// Image logical height.
  int  height;
  /** Add a border of width 'border' around the image that contains only unused data.
      This may be useful for some algorithms like filters that always consider a region
      around a pixel. */
  int  border;

  // --- Image structure information ---
  /// If image includes an alpha bitmap.
  bool has_alphamask;
};


/** Bitmap data alignment information. */
class ImageInfo_Alignment
{
public:
  /** Default aligmnent constructor. Default values are: not alignment, physical
      image size need not match logical image size. */
  ImageInfo_Alignment()
    : halign(1), valign(1),
      exact_size(false) { }

  /* See documentation "doc/bitmapdimensions.eps" for an explanation on
     what these values are defining, too. */

  /// Round width up to a multiple of this value.
  int  halign;
  /// Round height up to a multiple of this value.
  int  valign;

  /** If set to #true#: don't allow the alignment or the border to be greater than specified.
      {\bf Explanation}: As it is more efficient to keep and older bitmap if the new one
      is smaller than the old one, the old one is sometimes used instead of creating
      a new one. This does not work if you are depending on the exact memory layout of
      the image. So you can disable it by setting exact\_size to true. */
  bool exact_size;
};


/** Extra image parameters for YUV images. */
class ImageInfo_YUVExtraInfo
{
public:
  ImageInfo_YUVExtraInfo()
    : chroma(Chroma444), nocolor(false), reduced_chroma_size(false) { }

  /// Chroma format of the image.
  ChromaFormat chroma;
  /// Image is a greyscale image. Not bitmaps are allocated for the U- and V-channels.
  bool nocolor;
  /** Bitmap size of chroma planes will be reduced according to the
      value of 'chroma'. */
  bool reduced_chroma_size;
};


/* The ***Param*** structs are for asking image attributes;
   the ***Spec*** are for setting the image attributes. */

/// Parameters of existing RGB images (size, existence of alpha bitmap)
class ImageParam     : public ImageInfo_Base  { };
/// Parameters of existing YUV images
class ImageParam_YUV : public ImageParam,     public ImageInfo_YUVExtraInfo
{
public:
  int GetChromaWidth()  const { return (width +ChromaSubH(chroma)-1)/ChromaSubH(chroma); }
  int GetChromaHeight() const { return (height+ChromaSubV(chroma)-1)/ChromaSubV(chroma); }

  /// Get size of chroma bitmaps.
  void GetChromaSizes(int& w,int &h) const
    {
      h = GetChromaHeight();
      w = GetChromaWidth();
    }
};


/// Specification of image parameters for RGB image creation.
class ImageSpec      : public ImageParam,     public ImageInfo_Alignment { };
/// Specification of image parameters for YUV image creation.
class ImageSpec_YUV  : public ImageParam_YUV, public ImageInfo_Alignment { };



/**
   Easy handling of RGB and YUV images. Both types of images can
   additionally contain an alpha mask. YUV type images support
   4:4:4, 4:2:2 and 4:2:0 chroma formats and greyscale only images.
   You can decide if you want the chroma planes to be the same
   size even though you are not using 4:4:4. This can help
   in later chroma format conversion as it can be done in place.
*/
template <class Pel> class Image
{
  friend void EnhanceImageWithBorder(Image<Pixel>&, int, bool);  // GCC 2.96 bug ???

public:
  virtual ~Image() { }

  enum BitmapChannel { Bitmap_Red = 0, Bitmap_Green = 1, Bitmap_Blue = 2,
		       Bitmap_Y   = 0, Bitmap_Cr    = 1, Bitmap_Cb   = 2,
	                               Bitmap_U     = 1, Bitmap_V    = 2,
		       Bitmap_Hue = 0, Bitmap_Saturation = 1, Bitmap_Brightness = 2,
                       Bitmap_Alpha=3
  };

  /// Get colorspace independent image parameters.
  void GetParam(ImageParam& p) const { p=d_param; }

  /** Get write access to a bitmap in the image. Please use the read-only variant of this method if you do
      not need write access. */
  Bitmap<Pel>&       AskBitmap      (BitmapChannel bm_id)       { return d_bm[bm_id]; }
  /// Get read-only access to a bitmap in the image.
  const Bitmap<Pel>& AskBitmap_const(BitmapChannel bm_id) const { return d_bm[bm_id]; }


  /** Replace a complete bitmap. Note that the new bitmap either has to be empty or has to
      be exactly the same size as the old one.
      Furthermore you are responsible that all alignments and the border size is sufficient
      for your application. This is not checked!
      
      If you insert or remove (by replacing a bitmap by an empty one) an alpha bitmap,
      the alphamask-flag in ImageParam will be set accordingly.
  */
  void ReplaceBitmap(BitmapChannel id,Bitmap<Pel>&);
  /// Set new image parameters.
  void SetParam(const ImageParam& param) { d_param=param; }

  /// Get write access to alpha bitmap.
        Pel*const* AskFrameA()             { return d_bm[Bitmap_Alpha].AskFrame(); }
  /// Get read-only access to alpha bitmap.
  const Pel*const* AskFrameA_const() const { return d_bm[Bitmap_Alpha].AskFrame_const(); }

  Bitmap<Pel>& AskBitmapA() { return d_bm[Bitmap_Alpha]; }
  const Bitmap<Pel>& AskBitmapA_const() const { return d_bm[Bitmap_Alpha]; }

  // --- hints ---

  /** Give the hint that the contents of the image is not used any more. This does not effect
      the logical behaviour of the image but can improve performance. */
  void Hint_ContentsIsNotUsedAnymore() { for (int i=0;i<4;i++) d_bm[i].Hint_ContentsIsNotUsedAnymore(); }


  // DEBUG

  int AskRefCntr() const { d_bm[0].AskRefCntr(); }

private:

protected:
  Image() { }
  Image(const Image<Pel>&);
  const Image<Pel>& operator=(const Image<Pel>&);

  void _Create(const ImageSpec&,bool bitmaps12,bool subh,bool subv);
  void _Destroy();

  Bitmap<Pel> d_bm[4];
  ImageParam  d_param;
};


/** RGB image. */
template <class Pel> class Image_RGB : public Image<Pel>
{
public:
  Image_RGB() { }
  Image_RGB(const Image_RGB<Pel>& img) : Image<Pel>(img) { }
  ~Image_RGB() { }

  /// Create new RGB image according to the specifications.
  void Create(const ImageSpec&);
  /// Free the image.
  void Destroy() { _Destroy(); }

  // --- shortcuts ---

  /// Get write access to the red color channel bitmap.
  Pel*const* AskFrameR() { return d_bm[Bitmap_Red  ].AskFrame(); }
  /// Get write access to the green color channel bitmap.
  Pel*const* AskFrameG() { return d_bm[Bitmap_Green].AskFrame(); }
  /// Get write access to the blue color channel bitmap.
  Pel*const* AskFrameB() { return d_bm[Bitmap_Blue ].AskFrame(); }

  /// Get read-only access to the red color channel bitmap.
  const Pel*const* AskFrameR_const() const { return d_bm[Bitmap_Red  ].AskFrame_const(); }
  /// Get read-only access to the green color channel bitmap.
  const Pel*const* AskFrameG_const() const { return d_bm[Bitmap_Green].AskFrame_const(); }
  /// Get read-only access to the blue color channel bitmap.
  const Pel*const* AskFrameB_const() const { return d_bm[Bitmap_Blue ].AskFrame_const(); }

  Bitmap<Pel>& AskBitmapR() { return d_bm[Bitmap_Red  ]; }
  Bitmap<Pel>& AskBitmapG() { return d_bm[Bitmap_Green]; }
  Bitmap<Pel>& AskBitmapB() { return d_bm[Bitmap_Blue ]; }

  const Bitmap<Pel>& AskBitmapR_const() { return d_bm[Bitmap_Red  ]; }
  const Bitmap<Pel>& AskBitmapG_const() { return d_bm[Bitmap_Green]; }
  const Bitmap<Pel>& AskBitmapB_const() { return d_bm[Bitmap_Blue ]; }
};


/** YUV image. */
template <class Pel> class Image_YUV : public Image<Pel>
{
public:
  Image_YUV() { }
  Image_YUV(const Image_YUV<Pel>& img) : Image<Pel>(img) { d_info_yuvextra=img.d_info_yuvextra; }
  ~Image_YUV() { }

  /// Create new RGB image according to the specifications.
  void Create(const ImageSpec_YUV&);
  /// Free the image.
  void Destroy() { _Destroy(); }

  /// Get image parameters including the YUV specific image parameters.
  void GetParam(ImageParam_YUV& p) const { Image<Pel>::GetParam(p); ((ImageInfo_YUVExtraInfo&)p)=d_info_yuvextra; }

  /** This method not only alters the chroma format but also checks that all bitmaps
      are the right size. That means that if you change the size of bitmaps because
      of a chroma convertion, you have to call this functions {\em after} replacing the
      bitmaps. */
  void SetChromaFormat(ChromaFormat cf);
  /// Set image parameters.
  void SetParam(const ImageParam_YUV& param) { d_info_yuvextra=param; d_param=param; }

  // shortcuts

  /// Get write access to the luminance color channel bitmap.
  Pel*const* AskFrameY() { return d_bm[Bitmap_Y].AskFrame(); }
  /// Get write access to the U-chrominance color channel bitmap.
  Pel*const* AskFrameU() { return d_bm[Bitmap_U].AskFrame(); }
  /// Get write access to the V-chrominance color channel bitmap.
  Pel*const* AskFrameV() { return d_bm[Bitmap_V].AskFrame(); }

  /// Get read-only access to the luminance color channel bitmap.
  const Pel*const* AskFrameY_const() const { return d_bm[Bitmap_Y].AskFrame_const(); }
  /// Get read-only access to the U-chrominance color channel bitmap.
  const Pel*const* AskFrameU_const() const { return d_bm[Bitmap_U].AskFrame_const(); }
  /// Get read-only access to the V-chrominance color channel bitmap.
  const Pel*const* AskFrameV_const() const { return d_bm[Bitmap_V].AskFrame_const(); }

  Bitmap<Pel>& AskBitmapY() { return d_bm[Bitmap_Y]; }
  Bitmap<Pel>& AskBitmapU() { return d_bm[Bitmap_U]; }
  Bitmap<Pel>& AskBitmapV() { return d_bm[Bitmap_V]; }

  const Bitmap<Pel>& AskBitmapY_const() { return d_bm[Bitmap_Y]; }
  const Bitmap<Pel>& AskBitmapU_const() { return d_bm[Bitmap_U]; }
  const Bitmap<Pel>& AskBitmapV_const() { return d_bm[Bitmap_V]; }

private:
  ImageInfo_YUVExtraInfo d_info_yuvextra;
};

#endif
