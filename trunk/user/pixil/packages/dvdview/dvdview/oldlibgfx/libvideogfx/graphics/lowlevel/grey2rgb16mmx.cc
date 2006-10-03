/*
 *  grey2rgb16mmx.cc
 */

#include "grey2rgb16mmx.hh"


bool i2r_grey_16bit_mmx::s_CanConvert(const Image_YUV<Pixel>& img,const RawImageSpec_RGB& spec)
{
  if (spec.resize_to_fixed || spec.resize_with_factor) return NULL;
  if (spec.bits_per_pixel != 16) return NULL;
  if (!spec.little_endian) return NULL;

  ImageParam_YUV param;
  img.GetParam(param);

  if (param.nocolor==false) return false;

  int w = (param.width+7) & ~7;
  if (spec.bytes_per_line < w) return false;

  return true;
}

void i2r_grey_16bit_mmx::Transform(const Image_YUV<Pixel>& img,uint8* mem,int firstline,int lastline)
{
  uint64 constants[6];

  constants[0] = 16-d_spec.r_bits-d_spec.r_shift;
  constants[1] = 16-d_spec.g_bits-d_spec.g_shift;
  constants[2] = 16-d_spec.b_bits-d_spec.b_shift;

  assert(constants[0]==0); // Assume that R is aligned to the very left.

  uint64 mask;
  mask = d_spec.r_mask; constants[3] = mask | (mask<<16) | (mask<<32) | (mask<<48);
  mask = d_spec.g_mask; constants[4] = mask | (mask<<16) | (mask<<32) | (mask<<48);
  mask = d_spec.b_mask; constants[5] = mask | (mask<<16) | (mask<<32) | (mask<<48);


  ImageParam_YUV param;
  img.GetParam(param);

  //assert(param.nocolor);

  const Pixel*const* pix_y  = img.AskFrameY_const();

  const int h = param.height;
  const int w = param.width;

        uint8* dp;
  const uint8* sp;

  uint8* dpstart=mem;

  assert(w%8 == 0);

  for (int y=firstline;y<=lastline;y++)
    {
      sp = pix_y[y];

      dp=dpstart;
      dpstart+=d_spec.bytes_per_line;

      for (int x=0;x<w;x+=8)
        {
          __asm__ __volatile__
            (
	     "movq        (%0),%%mm1\n\t"   // 8 Pixel nach mm1  (ABCDEFGH)
	     " pxor       %%mm2,%%mm2\n\t"
	     "movq        %%mm1,%%mm4\n\t"  //         und nach mm4
	     " punpckhbw  %%mm1,%%mm2\n\t"  // A0B0C0D0 in mm2,mm4,mm6
	     "pxor        %%mm3,%%mm3\n\t"
	     " punpcklbw  %%mm4,%%mm3\n\t"  // E0F0G0H0 in mm3,mm5,mm7
	     "movq        %%mm2,%%mm4\n\t"
	     " movq       %%mm3,%%mm5\n\t"
             "psrlq       1*8(%2),%%mm5\n\t" // G nach rechts schieben
	     " movq       %%mm2,%%mm6\n\t"
             "psrlq       1*8(%2),%%mm4\n\t" // G nach rechts schieben
	     " movq       %%mm3,%%mm7\n\t"
             "psrlq       2*8(%2),%%mm6\n\t" // B nach rechts schieben
	     " pand       3*8(%2),%%mm2\n\t"
             "psrlq       2*8(%2),%%mm7\n\t" // B nach rechts schieben
	     " pand       3*8(%2),%%mm3\n\t"
	     "pand        4*8(%2),%%mm4\n\t"
	     " pand       4*8(%2),%%mm5\n\t"
	     "por         %%mm4,%%mm2\n\t"
	     " pand       5*8(%2),%%mm6\n\t"
	     "por         %%mm5,%%mm3\n\t"
	     " pand       5*8(%2),%%mm7\n\t"
	     "por         %%mm6,%%mm2\n\t"
	     " movq       %%mm2,8(%1)\n\t"
	     "por         %%mm7,%%mm3\n\t"
	     " movq       %%mm3,(%1)\n\t"
             : : "r" (sp), "r" (dp), "r" (&constants[0])
             );

	  sp += 8;
	  dp += 16;
        }
    }

  __asm__
    (
     "emms\n\t"
     );
}
