/*********************************************************************
  libvideogfx/graphics/lowlevel/img2raw.hh

  purpose:
    Classes that allow image data stored in Image_*<Pixel> objects
    in either RGB or YUV format to be converted to a format the
    hardware directly understands. This can be (for example) the
    data ordering required for X11 XImages. Only RGB outputs
    are supported so far.

    Special hardware oriented convertion routines should be
    integrated here. The appropriate transformation is selected
    automagically.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    18/Jul/2000 - Dirk Farin - complete reimplementation
    29/Jul/1999 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_LOWLEVEL_IMG2RAW_HH
#define LIBVIDEOGFX_GRAPHICS_LOWLEVEL_IMG2RAW_HH

#include "libvideogfx/graphics/basic/image.hh"


void CalcBitsShift(uint32 mask,int& bits,int& shift);   /* mask=000011111000 -> bits=5; shift=3 */

/** Image manipulation operators that act on the fly.
    None of them is supported yet.
*/
struct RawImageOperator
{
  RawImageOperator() : resize_to_fixed(false), resize_with_factor(false), force_to_greyscale(false) { }

  bool resize_to_fixed;
  int  final_width,final_height;

  bool   resize_with_factor;
  float resize_factor;

  bool   force_to_greyscale; // REDUNDANT ?
};


/** RGB-output data format specification
 */
struct RawImageSpec_RGB : public RawImageOperator
{
  int  bytes_per_line;
  int  bits_per_pixel;
  bool little_endian;

  int r_mask,r_bits,r_shift;
  int g_mask,g_bits,g_shift;
  int b_mask,b_bits,b_shift;

  // example:   mask: 0xFF0000, bits: 8, shift = 16

  void SetRGBMasks(uint32 r,uint32 g,uint32 b)
    {
      CalcBitsShift(r_mask=r,r_bits,r_shift);
      CalcBitsShift(g_mask=g,g_bits,g_shift);
      CalcBitsShift(b_mask=b,b_bits,b_shift);
    }

  void Debug_ShowParam() const;
};


/** Image to raw-format converter class.
 */
class Image2Raw
{
public:
   Image2Raw();
  ~Image2Raw();

  void SetOutputSpec(const RawImageSpec_RGB& spec) { d_spec=spec; }

  void SetZoomFactor(float f=2)    { d_spec.resize_factor=f; d_spec.resize_with_factor = (f!=1.0); }
  void SetGrayscale(bool flag=true) { d_spec.force_to_greyscale = flag; }

  void TransformRGB(const Image_RGB<Pixel>&,uint8* mem,int firstline=0,int lastline=-1);
  void TransformYUV(const Image_YUV<Pixel>&,uint8* mem,int firstline=0,int lastline=-1);

private:
  RawImageSpec_RGB  d_spec;

  class Image2Raw_TransformYUV* d_last_yuv_transform;
  class Image2Raw_TransformRGB* d_last_rgb_transform;
};


// ---------------------------------------- only for implementation use ----------------------


class Image2Raw_TransformRGB
{
public:
  virtual ~Image2Raw_TransformRGB() { }
  virtual bool CanConvert(const Image_RGB<Pixel>&,const RawImageSpec_RGB&) = 0;
  void SetSpec(const RawImageSpec_RGB& spec) { d_spec=spec; }
  virtual void Transform(const Image_RGB<Pixel>&,uint8* mem,int firstline,int lastline) { assert(0); }

  virtual const char* TransformationName() = 0;

  static Image2Raw_TransformRGB* SelectTransform(const Image_RGB<Pixel>&,const RawImageSpec_RGB&);

protected:
  RawImageSpec_RGB d_spec;
};


class Image2Raw_TransformYUV
{
public:
  virtual ~Image2Raw_TransformYUV() { }
  virtual bool CanConvert(const Image_YUV<Pixel>&,const RawImageSpec_RGB&) = 0;
  void SetSpec(const RawImageSpec_RGB& spec) { d_spec=spec; }
  virtual void Transform(const Image_YUV<Pixel>&,uint8* mem,int firstline,int lastline) { assert(0); }

  virtual const char* TransformationName() = 0;

protected:
  RawImageSpec_RGB d_spec;
};

#endif
