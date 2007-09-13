/*********************************************************************
  color/colorspace.hh

  purpose:
    Routines for chroma format and colorspace convertions.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    30.Nov.1999 - Dirk Farin - implemented RGB2YUV_444()
    03/Aug/1999 - Dirk Farin - new functions: - TrTo???_DupSub()
                                              - Grey2RGB_Inplace()
    20/Jul/1999 - Dirk Farin - complete rewrite
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_COLOR_COLORSPACE_HH
#define LIBVIDEOGFX_GRAPHICS_COLOR_COLORSPACE_HH

#include "libvideogfx/graphics/basic/image.hh"


/* Apply interpolation filter with the kernel (1 3 3 1)//4 .
 * 'src' and 'dst' may be the same.
 */
void InterpolateH_Tap4(const Pixel*const* src,Pixel*const* dst,int dst_width,int height);
void InterpolateV_Tap4(const Pixel*const* src,Pixel*const* dst,int width,int dst_height);

void Tr420To444_Duplicate(Image_YUV<Pixel>& img);
void Tr420To422_Duplicate(Image_YUV<Pixel>& img);
void Tr422To444_Duplicate(Image_YUV<Pixel>& img);

void Tr422To420_Subsample(Image_YUV<Pixel>& img);
void Tr444To420_Subsample(Image_YUV<Pixel>& img);
void Tr444To422_Subsample(Image_YUV<Pixel>& img);

/* Transform any chroma format to the specified one (using simple pixel duplication/subsampling). */
void TrTo420_DupSub(Image_YUV<Pixel>& img);
void TrTo422_DupSub(Image_YUV<Pixel>& img);
void TrTo444_DupSub(Image_YUV<Pixel>& img);

void Tr422To444_Tap4(Image_YUV<Pixel>& img);


void YUV2RGB_444(const Image_YUV<Pixel>& src,Image_RGB<Pixel>& dst); // src and dst may be the same image.
void YUV2RGB_422(const Image_YUV<Pixel>& src,Image_RGB<Pixel>& dst); // src and dst may be the same image.
void YUV2RGB_420(const Image_YUV<Pixel>& src,Image_RGB<Pixel>& dst); // src and dst may be the same image.
void RGB2YUV_444(const Image_RGB<Pixel>& src,Image_YUV<Pixel>& dst); // src and dst may be the same image.

/* Transfer the bitmaps between the image types.
   Be extremely careful when using these functions as they do nothing except
   the bitmap transfer. Image size and the like must be adjusted by hand.
   For experienced users only!
*/
void TransferBitmapsRGB2YUV(Image_RGB<Pixel>&,Image_YUV<Pixel>&);
void TransferBitmapsYUV2RGB(Image_YUV<Pixel>&,Image_RGB<Pixel>&);

/* This also does colorspace convertion, but does some hacking to transfer the YUV-image bitmaps
   to the RGB-image. Thus the RGB image does not need to be initialized and the bitmaps in the YUV image
   will be removed from the image.
   The advantage of this function is that it is faster in some systems and does not require as much
   memory as the convertion is calculated "in place".

   Note that inplace calculation can be slower when you throw the bitmaps away after use.
   If you do not need them anymore it is perhaps a good idea to transfer them back to the YUV image.
*/
void YUV2RGB_444_Inplace(Image_YUV<Pixel>& src,Image_RGB<Pixel>& dst);

/* Convert greyscale YUV image to RGB.
   This function is VERY fast as no image data is actually being copied.
*/
void Grey2RGB_Inplace(Image_YUV<Pixel>& src,Image_RGB<Pixel>& dst);



void HSB2RGB(float h,float s,float b, float& r,float& g,float& b);  // input/output range: 0..1 
void RGB2YUV(uint8 r,uint8 g,uint8 b, uint8& y,uint8& u,uint8& v);


#if 0
class ColorConverter_RGB_YUV
{
public:
  ~ColorConverter_RGB_YUV() { }

  void SetCoefficients(...);

private:
  
};
#endif


#endif
