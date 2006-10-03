/*********************************************************************
  yuv2rgb32mmx.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, Kapellenweg 15, 72070 Tuebingen, Germany,
     email: farindk@trick.informatik.uni-stuttgart.de

  modifications:
   13/Apr/00 - Dirk Farin
     - first implementation based on 16bit version
 *********************************************************************/

#ifndef DISPLAY_YUV2RGB32MMX_HH
#define DISPLAY_YUV2RGB32MMX_HH

#include "img2raw.hh"


class i2r_32bit_BGR_mmx : public Image2Raw_TransformYUV
{
public:
  virtual ~i2r_32bit_BGR_mmx() { }

  virtual void Transform(const Image_YUV<Pixel>&,uint8* mem,int firstline,int lastline);

  static bool s_CanConvert(const Image_YUV<Pixel>&,const RawImageSpec_RGB&);
  virtual bool CanConvert(const Image_YUV<Pixel>& i,const RawImageSpec_RGB& s) { return s_CanConvert(i,s); }

  virtual const char* TransformationName() { return "YUV to 32bit RGB, (BGR format), MMX accelerated"; }

private:
};

#endif
