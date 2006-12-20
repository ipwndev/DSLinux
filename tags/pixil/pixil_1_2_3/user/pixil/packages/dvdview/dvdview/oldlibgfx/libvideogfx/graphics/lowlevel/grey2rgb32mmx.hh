/*********************************************************************
  grey2rgb32mmx.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, Kapellenweg 15, 72070 Tuebingen, Germany,
     email: farindk@trick.informatik.uni-stuttgart.de

  modifications:
   08/Mar/2000 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef DISPLAY_GREY2RGB32MMX_HH
#define DISPLAY_GREY2RGB32MMX_HH

#include "img2raw.hh"


class i2r_grey_32bit_mmx : public Image2Raw_TransformYUV
{
public:
  virtual ~i2r_grey_32bit_mmx() { }

  virtual void Transform(const Image_YUV<Pixel>&,uint8* mem,int firstline,int lastline);

  static bool s_CanConvert(const Image_YUV<Pixel>&,const RawImageSpec_RGB&);
  virtual bool CanConvert(const Image_YUV<Pixel>& i,const RawImageSpec_RGB& s) { return s_CanConvert(i,s); }

  virtual const char* TransformationName() { return "grey to 32bit RGB, MMX accelerated"; }
};

#endif
