/*********************************************************************
  libvideogfx/graphics/lowlevel/yuv2rgb16.hh

  purpose:
    Transform YUV data into 16bit true color RGB raw data.
    Every bit organization in a 16bit field and endianess
    translation is supported.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    08/Aug/1999 - Dirk Farin - code imported from DVDview and
                               slightly modified
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_LOWLEVEL_YUV2RGB16_HH
#define LIBVIDEOGFX_GRAPHICS_LOWLEVEL_YUV2RGB16_HH

#include "img2raw.hh"


class i2r_yuv_16bit : public Image2Raw_TransformYUV
{
public:
  virtual ~i2r_yuv_16bit() { }

  virtual void Transform(const Image_YUV<Pixel>&,uint8* mem,int firstline,int lastline);

  static bool s_CanConvert(const Image_YUV<Pixel>&,const RawImageSpec_RGB&);
  virtual bool CanConvert(const Image_YUV<Pixel>& i,const RawImageSpec_RGB& s) { return s_CanConvert(i,s); }

  virtual const char* TransformationName() { return "scalar YUV to 16bit RGB"; }

private:
};

#endif
