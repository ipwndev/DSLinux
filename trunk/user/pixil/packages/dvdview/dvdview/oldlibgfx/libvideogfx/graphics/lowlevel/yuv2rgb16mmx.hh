/*********************************************************************
  yuv2rgb16mmx.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, Kapellenweg 15, 72070 Tuebingen, Germany,
     email: farindk@trick.informatik.uni-stuttgart.de

  modifications:
   01/Feb/99 - Dirk Farin
     - interface definition
 *********************************************************************/

#ifndef DISPLAY_YUV2RGB16MMX_HH
#define DISPLAY_YUV2RGB16MMX_HH

#include "img2raw.hh"


class i2r_16bit_mmx : public Image2Raw_TransformYUV
{
public:
  virtual ~i2r_16bit_mmx() { }

  virtual void Transform(const Image_YUV<Pixel>&,uint8* mem,int firstline,int lastline);

  static bool s_CanConvert(const Image_YUV<Pixel>&,const RawImageSpec_RGB&);
  virtual bool CanConvert(const Image_YUV<Pixel>& i,const RawImageSpec_RGB& s) { return s_CanConvert(i,s); }

  virtual const char* TransformationName() { return "YUV to 16bit RGB, MMX accelerated"; }
};

#endif
