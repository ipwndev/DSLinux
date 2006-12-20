/*
 *  img2raw.cc
 */

#include <iostream.h>

#include "img2raw.hh"
#include "yuv2rgb16.hh"

#if ENABLE_MMX
#include "yuv2rgb16mmx.hh"
#include "yuv2rgb32mmx.hh"
#include "grey2rgb32mmx.hh"
#include "grey2rgb16mmx.hh"
#endif


#define SHOWLINE(x,y) cout << #x ": " << x << y

void RawImageSpec_RGB::Debug_ShowParam() const
{
  SHOWLINE(bytes_per_line,endl);
  SHOWLINE(bits_per_pixel,endl);
  cout << "little_endian: " << (little_endian ? "little" : "big") << endl;
  SHOWLINE(r_mask," "); SHOWLINE(r_bits," "); SHOWLINE(r_shift,endl);
  SHOWLINE(g_mask," "); SHOWLINE(g_bits," "); SHOWLINE(g_shift,endl);
  SHOWLINE(b_mask," "); SHOWLINE(b_bits," "); SHOWLINE(b_shift,endl);

  if (resize_to_fixed) { cout << "resize to fixed: " << final_width << "x" << final_height << endl; }
  if (resize_with_factor) { cout << "resize with factor: " << resize_factor << endl; }
  if (force_to_greyscale) cout << "force to greyscale\n";
}


// --------------------------------------------------------------------------------------------


/* Convert greyscale to RGB components in 32bit entities in arbitrary order.
 */
class i2r_grey_32bit : public Image2Raw_TransformYUV
{
public:
  virtual ~i2r_grey_32bit() { }
  virtual void Transform(const Image_YUV<Pixel>&,uint8* mem,int firstline,int lastline);

  static bool s_CanConvert(const Image_YUV<Pixel>&,const RawImageSpec_RGB&);
  virtual bool CanConvert(const Image_YUV<Pixel>& i,const RawImageSpec_RGB& s) { return s_CanConvert(i,s); }

  virtual const char* TransformationName() { return "scalar grey to 32bit RGB"; }
};


bool i2r_grey_32bit::s_CanConvert(const Image_YUV<Pixel>& img,const RawImageSpec_RGB& spec)
{
  if (spec.resize_to_fixed || spec.resize_with_factor) return false;
  if (spec.bits_per_pixel != 32) return false;
  if (spec.r_bits != 8 || spec.g_bits != 8 || spec.b_bits != 8) return false;
  if (spec.r_shift%8   || spec.g_shift%8   || spec.b_shift%8)   return false;

  ImageParam_YUV param;
  img.GetParam(param);

  if (param.nocolor==false) return false;

  return true;
}

void i2r_grey_32bit::Transform(const Image_YUV<Pixel>& img,uint8* mem,int firstline,int lastline)
{
  ImageParam_YUV param;
  img.GetParam(param);

  const Pixel*const * pix_y = img.AskFrameY_const();

  for (int y=firstline;y<=lastline;y++)
    {
      uint32* membuf = (uint32*)(mem + d_spec.bytes_per_line*(y-firstline));

      for (int x=0;x<param.width;x++)
        {
	  uint32 val = pix_y[y][x];
	  val |= val<<16;
	  val |= val<< 8;

          *membuf++ = val;
        }
    }
}


// --------------------------------------------------------------------------------------------


/* Convert YUV 4:2:0 and place RGB components in 32bit entities in arbitrary order.
 */
class i2r_yuv_32bit : public Image2Raw_TransformYUV
{
public:
  virtual ~i2r_yuv_32bit() { }
  virtual void Transform(const Image_YUV<Pixel>&,uint8* mem,int firstline,int lastline);

  static bool s_CanConvert(const Image_YUV<Pixel>&,const RawImageSpec_RGB&);
  virtual bool CanConvert(const Image_YUV<Pixel>& i,const RawImageSpec_RGB& s) { return s_CanConvert(i,s); }

  virtual const char* TransformationName() { return "scalar YUV to 32bit RGB"; }
};

bool i2r_yuv_32bit::s_CanConvert(const Image_YUV<Pixel>& img,const RawImageSpec_RGB& spec)
{
  if (spec.resize_to_fixed || spec.resize_with_factor) return false;
  if (spec.bits_per_pixel != 32 && spec.bits_per_pixel != 24) return false;
  if (spec.r_bits != 8 || spec.g_bits != 8 || spec.b_bits != 8) return false;
  if (spec.r_shift%8   || spec.g_shift%8   || spec.b_shift%8)   return false;

  ImageParam_YUV param;
  img.GetParam(param);

  if (param.nocolor==true) return false;
  if (param.chroma !=Chroma420) return false;

  return true;
}

static int* clip_0_255=NULL;
static int s_clip[1024];


static struct InitClip
{
  InitClip()
    {
      clip_0_255 = &s_clip[512];

      for (int i=-512;i<512;i++)
        {
          if (i<0)
            {
              clip_0_255[i]=0;
            }
          else if (i>255)
            {
              clip_0_255[i]=255;
            }
          else
            {
              clip_0_255[i]=i;
            }
        }
    }
} dummy_23874678;


void i2r_yuv_32bit::Transform(const Image_YUV<Pixel>& img,uint8* mem,int firstline,int lastline)
{
  int rpos,gpos,bpos;

  const int bytes_per_pixel = d_spec.bits_per_pixel/8;
  const int lastidx = bytes_per_pixel-1;

  rpos = lastidx-d_spec.r_shift/8;
  gpos = lastidx-d_spec.g_shift/8;
  bpos = lastidx-d_spec.b_shift/8;

  if (d_spec.little_endian)
    {
      rpos = lastidx-rpos;
      gpos = lastidx-gpos;
      bpos = lastidx-bpos;
    }


  ImageParam_YUV param;
  img.GetParam(param);

  assert(param.chroma==Chroma420);

  const Pixel*const * pix_y = img.AskFrameY_const();
  const Pixel*const * pix_u = img.AskFrameU_const();
  const Pixel*const * pix_v = img.AskFrameV_const();

  for (int y=firstline;y<=lastline;y+=2)
    {
      uint8* membuf8a = mem + d_spec.bytes_per_line*(y-firstline);
      uint8* membuf8b = mem + d_spec.bytes_per_line*y +d_spec.bytes_per_line;

      const Pixel* cbp = pix_u[y/2];
      const Pixel* crp = pix_v[y/2];
      const Pixel* y0p = pix_y[y  ];
      const Pixel* y1p = pix_y[y+1];

      for (int x=0;x<param.width;x+=2)
        {
          int cb=((int)*cbp++) -128;
          int cr=((int)*crp++) -128;

          int r0 = (int)(          + 409*cr);
          int g0 = (int)( - 100*cb - 208*cr);
          int b0 = (int)( + 516*cb         );
          
          int yy=(((int)*y0p++) -16)*298;
          int red   = (r0+yy)>>8; red  = clip_0_255[red];
          int green = (g0+yy)>>8; green= clip_0_255[green];
          int blue  = (b0+yy)>>8; blue = clip_0_255[blue];
          membuf8a[rpos] = red;
          membuf8a[gpos] = green;
          membuf8a[bpos] = blue;
          membuf8a+=bytes_per_pixel;

          yy=(((int)*y0p++) -16)*298;
          red   = (r0+yy)>>8; red  = clip_0_255[red];
          green = (g0+yy)>>8; green= clip_0_255[green];
          blue  = (b0+yy)>>8; blue = clip_0_255[blue];
          membuf8a[rpos] = red;
          membuf8a[gpos] = green;
          membuf8a[bpos] = blue;
          membuf8a+=bytes_per_pixel;
          
          yy=(((int)*y1p++) -16)*298;
          red   = (r0+yy)>>8; red  = clip_0_255[red];
          green = (g0+yy)>>8; green= clip_0_255[green];
          blue  = (b0+yy)>>8; blue = clip_0_255[blue];
          membuf8b[rpos] = red;
          membuf8b[gpos] = green;
          membuf8b[bpos] = blue;
          membuf8b+=bytes_per_pixel;
          
          yy=(((int)*y1p++) -16)*298;
          red   = (r0+yy)>>8; red  = clip_0_255[red];
          green = (g0+yy)>>8; green= clip_0_255[green];
          blue  = (b0+yy)>>8; blue = clip_0_255[blue];
          membuf8b[rpos] = red;
          membuf8b[gpos] = green;
          membuf8b[bpos] = blue;
          membuf8b+=bytes_per_pixel;
        }
    }
}


// --------------------------------------------------------------------------------------------

/* Convert YUV 4:2:2 and place RGB components in 32bit entities in arbitrary order.
 */
class i2r_yuv422_32bit : public Image2Raw_TransformYUV
{
public:
  virtual ~i2r_yuv422_32bit() { }
  virtual void Transform(const Image_YUV<Pixel>&,uint8* mem,int firstline,int lastline);

  static bool s_CanConvert(const Image_YUV<Pixel>&,const RawImageSpec_RGB&);
  virtual bool CanConvert(const Image_YUV<Pixel>& i,const RawImageSpec_RGB& s) { return s_CanConvert(i,s); }

  virtual const char* TransformationName() { return "scalar 4:2:2-YUV to 32bit RGB"; }
};

bool i2r_yuv422_32bit::s_CanConvert(const Image_YUV<Pixel>& img,const RawImageSpec_RGB& spec)
{
  if (spec.resize_to_fixed || spec.resize_with_factor) return false;
  if (spec.bits_per_pixel != 32) return false;
  if (spec.r_bits != 8 || spec.g_bits != 8 || spec.b_bits != 8) return false;
  if (spec.r_shift%8   || spec.g_shift%8   || spec.b_shift%8)   return false;

  ImageParam_YUV param;
  img.GetParam(param);

  if (param.nocolor==true) return false;
  if (param.chroma !=Chroma422) return false;

  return true;
}

void i2r_yuv422_32bit::Transform(const Image_YUV<Pixel>& img,uint8* mem,int firstline,int lastline)
{
  int rpos,gpos,bpos;

  rpos = 3-d_spec.r_shift/8;
  gpos = 3-d_spec.g_shift/8;
  bpos = 3-d_spec.b_shift/8;

  if (d_spec.little_endian)
    {
      rpos = 3-rpos;
      gpos = 3-gpos;
      bpos = 3-bpos;
    }


  ImageParam_YUV param;
  img.GetParam(param);

  assert(param.chroma==Chroma422);

  const Pixel*const * pix_y = img.AskFrameY_const();
  const Pixel*const * pix_u = img.AskFrameU_const();
  const Pixel*const * pix_v = img.AskFrameV_const();

  for (int y=firstline;y<=lastline;y++)
    {
      uint8* membuf8a = mem + d_spec.bytes_per_line*(y-firstline);

      const Pixel* cbp = pix_u[y];
      const Pixel* crp = pix_v[y];
      const Pixel* y0p = pix_y[y];

      for (int x=0;x<param.width;x+=2)
        {
          int cb=((int)*cbp++) -128;
          int cr=((int)*crp++) -128;

          int r0 = (int)(          + 409*cr);
          int g0 = (int)( - 100*cb - 208*cr);
          int b0 = (int)( + 516*cb         );
          
          int yy=(((int)*y0p++) -16)*298;
          int red   = (r0+yy)>>8; red  = clip_0_255[red];
          int green = (g0+yy)>>8; green= clip_0_255[green];
          int blue  = (b0+yy)>>8; blue = clip_0_255[blue];
          membuf8a[rpos] = red;
          membuf8a[gpos] = green;
          membuf8a[bpos] = blue;
          membuf8a+=4;

          yy=(((int)*y0p++) -16)*298;
          red   = (r0+yy)>>8; red  = clip_0_255[red];
          green = (g0+yy)>>8; green= clip_0_255[green];
          blue  = (b0+yy)>>8; blue = clip_0_255[blue];
          membuf8a[rpos] = red;
          membuf8a[gpos] = green;
          membuf8a[bpos] = blue;
          membuf8a+=4;
        }
    }
}

// --------------------------------------------------------------------------------------------

/* Convert YUV 4:4:4 and place RGB components in 32bit entities in arbitrary order.
 */
class i2r_yuv444_32bit : public Image2Raw_TransformYUV
{
public:
  virtual ~i2r_yuv444_32bit() { }
  virtual void Transform(const Image_YUV<Pixel>&,uint8* mem,int firstline,int lastline);

  static bool s_CanConvert(const Image_YUV<Pixel>&,const RawImageSpec_RGB&);
  virtual bool CanConvert(const Image_YUV<Pixel>& i,const RawImageSpec_RGB& s) { return s_CanConvert(i,s); }

  virtual const char* TransformationName() { return "scalar 4:4:4-YUV to 32bit RGB"; }
};

bool i2r_yuv444_32bit::s_CanConvert(const Image_YUV<Pixel>& img,const RawImageSpec_RGB& spec)
{
  if (spec.resize_to_fixed || spec.resize_with_factor) return false;
  if (spec.bits_per_pixel != 32) return false;
  if (spec.r_bits != 8 || spec.g_bits != 8 || spec.b_bits != 8) return false;
  if (spec.r_shift%8   || spec.g_shift%8   || spec.b_shift%8)   return false;

  ImageParam_YUV param;
  img.GetParam(param);

  if (param.nocolor==true) return false;
  if (param.chroma !=Chroma444) return false;

  return true;
}

void i2r_yuv444_32bit::Transform(const Image_YUV<Pixel>& img,uint8* mem,int firstline,int lastline)
{
  int rpos,gpos,bpos;

  rpos = 3-d_spec.r_shift/8;
  gpos = 3-d_spec.g_shift/8;
  bpos = 3-d_spec.b_shift/8;

  if (d_spec.little_endian)
    {
      rpos = 3-rpos;
      gpos = 3-gpos;
      bpos = 3-bpos;
    }


  ImageParam_YUV param;
  img.GetParam(param);

  assert(param.chroma==Chroma444);

  const Pixel*const * pix_y = img.AskFrameY_const();
  const Pixel*const * pix_u = img.AskFrameU_const();
  const Pixel*const * pix_v = img.AskFrameV_const();

  for (int y=firstline;y<=lastline;y++)
    {
      uint8* membuf8a = mem + d_spec.bytes_per_line*(y-firstline);

      const Pixel* cbp = pix_u[y];
      const Pixel* crp = pix_v[y];
      const Pixel* y0p = pix_y[y];

      for (int x=0;x<param.width;x++)
        {
          int cb=((int)*cbp++) -128;
          int cr=((int)*crp++) -128;

          int r0 = (int)(          + 409*cr);
          int g0 = (int)( - 100*cb - 208*cr);
          int b0 = (int)( + 516*cb         );
          
          int yy=(((int)*y0p++) -16)*298;
          int red   = (r0+yy)>>8; red  = clip_0_255[red];
          int green = (g0+yy)>>8; green= clip_0_255[green];
          int blue  = (b0+yy)>>8; blue = clip_0_255[blue];
          membuf8a[rpos] = red;
          membuf8a[gpos] = green;
          membuf8a[bpos] = blue;
          membuf8a+=4;
        }
    }
}

// --------------------------------------------------------------------------------------------

/* Place RGB components in 32bit entities in arbitrary order.
   For the special cases that can be handled by the classes i2r_xrgb and i2r_xbgr you
   should use them as they are a bit faster.
 */
class i2r_32bit : public Image2Raw_TransformRGB
{
public:
  virtual ~i2r_32bit() { }
  virtual void Transform(const Image_RGB<Pixel>&,uint8* mem,int firstline,int lastline);

  static bool s_CanConvert(const Image_RGB<Pixel>&,const RawImageSpec_RGB&);
  virtual bool CanConvert(const Image_RGB<Pixel>& i,const RawImageSpec_RGB& s) { return s_CanConvert(i,s); }

  virtual const char* TransformationName() { return "scalar 32bit RGB"; }
};

bool i2r_32bit::s_CanConvert(const Image_RGB<Pixel>& img,const RawImageSpec_RGB& spec)
{
  if (spec.resize_to_fixed || spec.resize_with_factor) return false;
  if (spec.bits_per_pixel != 32 && spec.bits_per_pixel != 24) return false;
  if (spec.r_bits != 8 || spec.g_bits != 8 || spec.b_bits != 8) return false;
  if (spec.r_shift%8   || spec.g_shift%8   || spec.b_shift%8)   return false;

  return true;
}

void i2r_32bit::Transform(const Image_RGB<Pixel>& img,uint8* mem,int firstline,int lastline)
{
  int rpos,gpos,bpos;

  const int bytes_per_pixel = d_spec.bits_per_pixel/8;
  const int lastidx = bytes_per_pixel-1;

  rpos = lastidx-d_spec.r_shift/8;
  gpos = lastidx-d_spec.g_shift/8;
  bpos = lastidx-d_spec.b_shift/8;

  if (d_spec.little_endian)
    {
      rpos = lastidx-rpos;
      gpos = lastidx-gpos;
      bpos = lastidx-bpos;
    }

  ImageParam param;
  img.GetParam(param);

  const Pixel*const* r = img.AskFrameR_const();
  const Pixel*const* g = img.AskFrameG_const();
  const Pixel*const* b = img.AskFrameB_const();

  for (int y=firstline;y<=lastline;y++)
    {
      uint8* p = mem+y*d_spec.bytes_per_line;

      int yy = y-firstline;

      for (int x=0;x<param.width;x++)
	{
	  p[rpos] = r[yy][x];
	  p[gpos] = g[yy][x];
	  p[bpos] = b[yy][x];
	  p+=bytes_per_pixel;
	}
    }
}

// --------------------------------------------------------------------------------------------

/* Place RGB components in 16bit entities in arbitrary order.
   For the special cases that can be handled by the classes i2r_xrgb and i2r_xbgr you
   should use them as they are a bit faster.
 */
class i2r_16bit : public Image2Raw_TransformRGB
{
public:
  virtual ~i2r_16bit() { }
  virtual void Transform(const Image_RGB<Pixel>&,uint8* mem,int firstline,int lastline);

  static bool s_CanConvert(const Image_RGB<Pixel>&,const RawImageSpec_RGB&);
  virtual bool CanConvert(const Image_RGB<Pixel>& i,const RawImageSpec_RGB& s) { return s_CanConvert(i,s); }

  virtual const char* TransformationName() { return "scalar 16bit RGB"; }
};

bool i2r_16bit::s_CanConvert(const Image_RGB<Pixel>& img,const RawImageSpec_RGB& spec)
{
  int rshift = spec.r_shift+(spec.r_bits-8);
  int gshift = spec.g_shift+(spec.g_bits-8);
  int bshift = spec.b_shift+(spec.b_bits-8);

  if (rshift<0 || gshift<0) return false;
  if (bshift>0) return false;

  if (spec.resize_to_fixed || spec.resize_with_factor) return false;
  if (spec.bits_per_pixel != 16) return false;

  return true;
}

void i2r_16bit::Transform(const Image_RGB<Pixel>& img,uint8* mem,int firstline,int lastline)
{
  int rpos,gpos,bpos;

  ImageParam param;
  img.GetParam(param);

  const Pixel*const* r = img.AskFrameR_const();
  const Pixel*const* g = img.AskFrameG_const();
  const Pixel*const* b = img.AskFrameB_const();

  int rshift = d_spec.r_shift+(d_spec.r_bits-8);
  int gshift = d_spec.g_shift+(d_spec.g_bits-8);
  int bshift = d_spec.b_shift+(d_spec.b_bits-8);

  bshift = -bshift;

  for (int y=firstline;y<=lastline;y++)
    {
      uint16* p = (uint16*)(mem+y*d_spec.bytes_per_line);

      int yy = y-firstline;

      for (int x=0;x<param.width;x++)
	{
	  uint16 val;

	  val  = (r[yy][x]<<rshift) & d_spec.r_mask;
	  val |= (g[yy][x]<<gshift) & d_spec.g_mask;
	  val |= (b[yy][x]>>bshift); // & d_spec.b_mask;

	  *p++ = (d_spec.little_endian ? ToLittleEndian((uint16)val) : ToBigEndian((uint16)val));
	}
    }
}

// --------------------------------------------------------------------------------------------


void CalcBitsShift(uint32 mask,int& bits,int& shift)
{
  assert(mask!=0);

  shift=0;
  while ((mask&1)==0) { shift++; mask>>=1; }
  bits=0;
  while (mask&1) { bits++; mask>>=1; }

  assert(mask==0); // This may fail if there are more than one continuous sequences if ones (like 000111000011100).
}


Image2Raw::Image2Raw()
  : d_last_yuv_transform(NULL),
    d_last_rgb_transform(NULL)
{
}


Image2Raw::~Image2Raw()
{
  if (d_last_yuv_transform) delete d_last_yuv_transform;
  if (d_last_rgb_transform) delete d_last_rgb_transform;
}


void Image2Raw::TransformRGB(const Image_RGB<Pixel>& img,uint8* mem,int firstline,int lastline)
{
  Image2Raw_TransformRGB* t=NULL;

  if (d_last_rgb_transform)
    {
      if (d_last_rgb_transform->CanConvert(img,d_spec))
	{
	  // cout << "reusing old transform\n";
	  t = d_last_rgb_transform;
	  goto found2;
	}
      else
	{
	  delete d_last_rgb_transform;
	  d_last_rgb_transform=NULL;
	}
    }

  //cout << "searching for transform to match:\n";
  //d_spec.Debug_ShowParam();

  if (i2r_32bit::s_CanConvert(img,d_spec)) { t = new i2r_32bit; goto found; }
  if (i2r_16bit::s_CanConvert(img,d_spec)) { t = new i2r_16bit; goto found; }

  throw "no suitable colorspace transformation found!\n";
  assert(0); // no transformation found;
  return;

found:
  //cout << "RGB transformation used: " << t->TransformationName() << endl;
found2:
  t->SetSpec(d_spec);

  if (lastline==-1)
    {
      ImageParam param;
      img.GetParam(param);
      lastline=param.height-1;
    }
  t->Transform(img,mem,firstline,lastline);

  d_last_rgb_transform = t;
}

void Image2Raw::TransformYUV(const Image_YUV<Pixel>& img,uint8* mem,int firstline,int lastline)
{
  Image2Raw_TransformYUV* t=NULL;

  if (d_last_yuv_transform)
    {
      if (d_last_yuv_transform->CanConvert(img,d_spec))
	{
	  // cout << "reusing old transform\n";
	  t = d_last_yuv_transform;
	  goto found2;
	}
      else
	{
	  delete d_last_yuv_transform;
	  d_last_yuv_transform=NULL;
	}
    }

  //cout << "searching for transform to match:\n";
  //d_spec.Debug_ShowParam();

#if ENABLE_MMX
  if (i2r_32bit_BGR_mmx ::s_CanConvert(img,d_spec)) { t = new i2r_32bit_BGR_mmx;  goto found; }
  if (i2r_grey_32bit_mmx::s_CanConvert(img,d_spec)) { t = new i2r_grey_32bit_mmx; goto found; }
  if (i2r_16bit_mmx::s_CanConvert(img,d_spec)) { t = new i2r_16bit_mmx; goto found; }
  if (i2r_grey_16bit_mmx::s_CanConvert(img,d_spec)) { t = new i2r_grey_16bit_mmx; goto found; }

#endif

  if (i2r_yuv_32bit   ::s_CanConvert(img,d_spec)) { t = new i2r_yuv_32bit;    goto found; }
  if (i2r_grey_32bit  ::s_CanConvert(img,d_spec)) { t = new i2r_grey_32bit;   goto found; }
  if (i2r_yuv422_32bit::s_CanConvert(img,d_spec)) { t = new i2r_yuv422_32bit; goto found; }
  if (i2r_yuv444_32bit::s_CanConvert(img,d_spec)) { t = new i2r_yuv444_32bit; goto found; }
  if (i2r_yuv_16bit   ::s_CanConvert(img,d_spec)) { t = new i2r_yuv_16bit;    goto found; }
  

  throw "no suitable colorspace transformation found!\n";
  assert(0); // no transformation found;
  return;

found:
  //cout << "YUV transformation used: " << t->TransformationName() << endl;
found2:
  t->SetSpec(d_spec);

  if (lastline==-1)
    {
      ImageParam_YUV param;
      img.GetParam(param);
      lastline=param.height-1;
    }
  t->Transform(img,mem,firstline,lastline);

  d_last_yuv_transform = t;
}
