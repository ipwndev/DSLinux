/*
 *  yuv2rgb16.cc
 */

#include <iostream.h>
#include <iomanip.h>

#include "yuv2rgb16.hh"


bool i2r_yuv_16bit::s_CanConvert(const Image_YUV<Pixel>& img,const RawImageSpec_RGB& spec)
{
  if (spec.resize_to_fixed || spec.resize_with_factor) return false;
  if (spec.bits_per_pixel != 16) return false;
  if (!spec.little_endian) return false;

  ImageParam_YUV param;
  img.GetParam(param);

  if (param.nocolor==true) return false;
  if (param.chroma !=Chroma420) return false;

  return true;
}

void i2r_yuv_16bit::Transform(const Image_YUV<Pixel>& img,uint8* mem,int firstline,int lastline)
{
  const uint32 rmask=d_spec.r_mask;
  const uint32 gmask=d_spec.g_mask;
  const uint32 bmask=d_spec.b_mask;

  uint32 rshift,gshift,bshift;

  rshift = d_spec.r_shift;  rshift -= 8-d_spec.r_bits;  rshift -= 8;  rshift = -rshift;
  gshift = d_spec.g_shift;  gshift -= 8-d_spec.g_bits;  gshift -= 8;  gshift = -gshift;
  bshift = d_spec.b_shift;  bshift -= 8-d_spec.b_bits;  bshift -= 8;  bshift = -bshift;

  ImageParam_YUV param;
  img.GetParam(param);

  assert(param.chroma==Chroma420);
  assert(firstline%2 == 0);

  const Pixel*const* yp = img.AskFrameY_const();
  const Pixel*const* up = img.AskFrameU_const();
  const Pixel*const* vp = img.AskFrameV_const();

  int chr_w, chr_h;

  //param.GetChromaSizes(chr_w,chr_h);

  bool fastversion = (rshift==0 &&  // red does not need shifting
                      bshift>=0);   // blue will be shifted right -> no need to mask it out

  if (fastversion)
    {
      for (int cy=firstline/2;cy<=lastline/2;cy++)
        {
          const Pixel*  yptr1 = yp[2*cy  ];
          const Pixel*  yptr2 = yp[2*cy+1];
          const Pixel*  uptr  = up[cy];
          const Pixel*  vptr  = vp[cy];
          uint16* membuf16a = ((uint16*)(mem+ 2*cy   *d_spec.bytes_per_line));
          uint16* membuf16b = ((uint16*)(mem+(2*cy+1)*d_spec.bytes_per_line));

          for (int cx=0;cx<chr_w;cx++)
            {
              int u=((int)*uptr++) -128;
              int v=((int)*vptr++) -128;
              
              int r0 = (int)(         + 409*v);
              int g0 = (int)( - 100*u - 208*v);
              int b0 = (int)( + 516*u        );

              int val;
              int yy=(((int)*yptr1++) -16)*298;

              int red   = r0+yy;
              if (red<=0) { val=0; } else if (red>0xff00) { val = rmask; } else { val = (red)&rmask; }
              int green = g0+yy;
              if (green<=0) { } else if (green>0xff00) { val|=gmask; } else { val |= (green>>gshift)&gmask; }
              int blue  = b0+yy;
              if (blue<=0) { } else if (blue>=0xff00) { val |= bmask; } else { val |= (blue>>bshift); }
              *membuf16a++ = (d_spec.little_endian ? ToLittleEndian((uint16)val) : ToBigEndian((uint16)val));

              yy=(((int)*yptr1++) -16)*298;
              red   = r0+yy;
              if (red<=0) { val=0; } else if (red>0xff00) { val = rmask; } else { val = (red)&rmask; }
              green = g0+yy;
              if (green<=0) { } else if (green>0xff00) { val|=gmask; } else { val |= (green>>gshift)&gmask; }
              blue  = b0+yy;
              if (blue<=0) { } else if (blue>=0xff00) { val |= bmask; } else { val |= (blue>>bshift); }
              *membuf16a++ = (d_spec.little_endian ? ToLittleEndian((uint16)val) : ToBigEndian((uint16)val));
              
              yy=(((int)*yptr2++) -16)*298;
              red   = r0+yy;
              if (red<=0) { val=0; } else if (red>0xff00) { val = rmask; } else { val = (red)&rmask; }
              green = g0+yy;
              if (green<=0) { } else if (green>0xff00) { val|=gmask; } else { val |= (green>>gshift)&gmask; }
              blue  = b0+yy;
              if (blue<=0) { } else if (blue>=0xff00) { val |= bmask; } else { val |= (blue>>bshift); }
              *membuf16b++ = (d_spec.little_endian ? ToLittleEndian((uint16)val) : ToBigEndian((uint16)val));
              
              yy=(((int)*yptr2++) -16)*298;
              red   = r0+yy;
              if (red<=0) { val=0; } else if (red>0xff00) { val = rmask; } else { val = (red)&rmask; }
              green = g0+yy;
              if (green<=0) { } else if (green>0xff00) { val|=gmask; } else { val |= (green>>gshift)&gmask; }
              blue  = b0+yy;
              if (blue<=0) { } else if (blue>=0xff00) { val |= bmask; } else { val |= (blue>>bshift); }
              *membuf16b++ = (d_spec.little_endian ? ToLittleEndian((uint16)val) : ToBigEndian((uint16)val));
            }
        }
    }
  else
    {
      for (int cy=firstline/2;cy<=lastline/2;cy++)
        {
          const Pixel*  yptr1 = yp[2*cy  ];
          const Pixel*  yptr2 = yp[2*cy+1];
          const Pixel*  uptr  = up[cy];
          const Pixel*  vptr  = vp[cy];
          uint16* membuf16a = ((uint16*)(mem+ 2*cy   *d_spec.bytes_per_line));
          uint16* membuf16b = ((uint16*)(mem+(2*cy+1)*d_spec.bytes_per_line));

          for (int cx=0;cx<chr_w;cx++)
            {
              int u=((int)*uptr++) -128;
              int v=((int)*vptr++) -128;
              
              int r0 = (int)(         + 409*v);
              int g0 = (int)( - 100*u - 208*v);
              int b0 = (int)( + 516*u        );
              
              int val;
              int yy=(((int)*yptr1++) -16)*298;
              int red   = r0+yy;
              if (red<=0) { val=0; } else if (red>0xff00) { val=rmask; } else { val = (red>>rshift)&rmask; }
              int green = g0+yy;
              if (green<=0) { } else if (green>0xff00) { val|=gmask; } else { val |= (green>>gshift)&gmask; }
              int blue  = b0+yy;
              if (blue<=0) { } else if (blue>=0xff00) { val |= bmask; } else { val |= (blue>>bshift)&bmask; }
              *membuf16a++ = (d_spec.little_endian ? ToLittleEndian((uint16)val) : ToBigEndian((uint16)val));
              
              yy=(((int)*yptr1++) -16)*298;
              red   = r0+yy;
              if (red<=0) { val=0; } else if (red>0xff00) { val=rmask; } else { val = (red>>rshift)&rmask; }
              green = g0+yy;
              if (green<=0) { } else if (green>0xff00) { val|=gmask; } else { val |= (green>>gshift)&gmask; }
              blue  = b0+yy;
              if (blue<=0) { } else if (blue>=0xff00) { val |= bmask; } else { val |= (blue>>bshift)&bmask; }
              *membuf16a++ = (d_spec.little_endian ? ToLittleEndian((uint16)val) : ToBigEndian((uint16)val));
              
              yy=(((int)*yptr2++) -16)*298;
              red   = r0+yy;
              if (red<=0) { val=0; } else if (red>0xff00) { val=rmask; } else { val = (red>>rshift)&rmask; }
              green = g0+yy;
              if (green<=0) { } else if (green>0xff00) { val|=gmask; } else { val |= (green>>gshift)&gmask; }
              blue  = b0+yy;
              if (blue<=0) { } else if (blue>=0xff00) { val |= bmask; } else { val |= (blue>>bshift)&bmask; }
              *membuf16b++ = (d_spec.little_endian ? ToLittleEndian((uint16)val) : ToBigEndian((uint16)val));
              
              yy=(((int)*yptr2++) -16)*298;
              red   = r0+yy;
              if (red<=0) { val=0; } else if (red>0xff00) { val=rmask; } else { val = (red>>rshift)&rmask; }
              green = g0+yy;
              if (green<=0) { } else if (green>0xff00) { val|=gmask; } else { val |= (green>>gshift)&gmask; }
              blue  = b0+yy;
              if (blue<=0) { } else if (blue>=0xff00) { val |= bmask; } else { val |= (blue>>bshift)&bmask; }
              *membuf16b++ = (d_spec.little_endian ? ToLittleEndian((uint16)val) : ToBigEndian((uint16)val));
            }
        }
    }
}
