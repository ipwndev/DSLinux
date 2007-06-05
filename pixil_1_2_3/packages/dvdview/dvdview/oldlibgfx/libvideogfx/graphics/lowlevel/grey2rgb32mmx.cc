/*
 *  grey2rgb32mmx.cc
 */

#include "grey2rgb32mmx.hh"


bool i2r_grey_32bit_mmx::s_CanConvert(const Image_YUV<Pixel>& img,const RawImageSpec_RGB& spec)
{
  if (spec.resize_to_fixed || spec.resize_with_factor) return NULL;
  if (spec.bits_per_pixel != 32) return NULL;
  if (spec.r_bits != 8 || spec.g_bits != 8 || spec.b_bits != 8) return NULL;
  if ((spec.r_shift%8) || (spec.g_shift%8) || (spec.b_shift%8)) return NULL;

  ImageParam_YUV param;
  img.GetParam(param);

  if (param.nocolor==false) return false;

  int w = (param.width+7) & ~7;

  if (spec.bytes_per_line < 4*w) return false;

  return true;
}

void i2r_grey_32bit_mmx::Transform(const Image_YUV<Pixel>& img,uint8* mem,int firstline,int lastline)
{
  ImageParam_YUV param;
  img.GetParam(param);

  assert(param.nocolor);

  const Pixel*const* pix_y  = img.AskFrameY_const();

  const int h = param.height;
  const int w = param.width;

        uint8* dp;
  const uint8* sp;

  uint8* dpstart=mem;

  //assert(w%8 == 0);

  for (int y=firstline;y<=lastline;y++)
    {
      sp = pix_y[y];

      dp=dpstart;
      dpstart+=d_spec.bytes_per_line;

      for (int x=0;x<w;x+=8)
        {
          __asm__ __volatile__
            (
	     "movq        (%0),%%mm0\n\t"   // 8 Pixel nach mm0  (ABCDEFGH)
	     "movq        %%mm0,%%mm4\n\t"  //         und nach mm4
	     "punpckhbw   %%mm0,%%mm0\n\t"  // AABBCCDD in mm0,mm2
	     " punpcklbw  %%mm4,%%mm4\n\t"  // EEFFGGHH in mm4,mm6
	     "movq        %%mm0,%%mm2\n\t"  //
             " movq       %%mm4,%%mm6\n\t"  //
	     "punpckhbw   %%mm0,%%mm0\n\t"  // AAAABBBB in mm0
	     " punpckhbw  %%mm4,%%mm4\n\t"  // EEEEFFFF in mm4
	     "movq        %%mm4,8(%1)\n\t"  // EF->mem
             " punpcklbw  %%mm2,%%mm2\n\t"  // CCCCDDDD in mm2
	     "movq        %%mm0,24(%1)\n\t" // AB->mem
	     " punpcklbw  %%mm6,%%mm6\n\t"  // GGGGHHHH in mm6
	     "movq        %%mm2,16(%1)\n\t" // CD->mem
	     "movq        %%mm6,(%1)\n\t"   // GH->mem
             : : "r" (sp), "r" (dp)
             );

	  sp += 8;
	  dp += 32;
        }
    }

  __asm__
    (
     "emms\n\t"
     );
}
