/*********************************************************************
  blkcmp.hh

  purpose:
    Block-compare code including KNI (MMX2) instruction set optimized
    version.

  notes:
    !!! Don't forget to call EMMS() before using any floating-point
    arithmetic when using the MMX-version !!!

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
   11/Jul/2000 - Dirk Farin
     - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_GRAPHICS_MOTVEC_BLKCMP_KNI_HH
#define LIBVIDEOGFX_GRAPHICS_MOTVEC_BLKCMP_KNI_HH

#include "libvideogfx/graphics/basic/bitmap.hh"


#if ENABLE_MMX
inline int ComputeSAD_w16(const Pixel*const* blkrows_1,int h0_1,
			  const Pixel*const* blkrows_2,int h0_2,
			  int blk_height,
			  int max_error_threshold)
{
  volatile uint32 error;

  __asm__
    (
     "pxor %mm0,%mm0\n\t"
     );

  for (int dv=0; dv<blk_height; dv++) {
    const Pixel*const p1 = &blkrows_1[dv][h0_1];
    const Pixel*const p2 = &blkrows_2[dv][h0_2];

    __asm__ __volatile__ 
      (
       "movq   (%0),%%mm1\n\t"
       "psadbw (%1),%%mm1\n\t"
       "paddw  %%mm1,%%mm0\n\t"
	 
       "movq   8(%0),%%mm2\n\t"
       "psadbw 8(%1),%%mm2\n\t"
       "paddw  %%mm2,%%mm0\n\t"

       "movd   %%mm0,(%2)\n\t"

       : : "r" (p1), "r" (p2), "r" (&error)
       );

    if (error>max_error_threshold)
      return error+1;
  }

  return error;
}

inline void EMMS()
{
  __asm__
    (
     "emms\n\t"
     );
}
#else
inline int ComputeSAD_w16(const Pixel*const* blkrows_1,int h0_1,
			  const Pixel*const* blkrows_2,int h0_2,
			  int blk_height,
			  int max_error_threshold)
{
  uint32 error=0;

  for (int dv=0; dv<blk_height; dv++) {
    const Pixel* p1 = &blkrows_1[dv][h0_1];
    const Pixel* p2 = &blkrows_2[dv][h0_2];

    for (int dh=0; dh<16; dh+=4) {
      int diff1 = abs(p1[0]-p2[0]);
      int diff2 = abs(p1[1]-p2[1]);
      int diff3 = abs(p1[2]-p2[2]);
      int diff4 = abs(p1[3]-p2[3]);
      diff1 += diff2;
      diff3 += diff4;
      diff1 += diff3;
      error += diff1;
      p1 += 4;
      p2 += 4;
    }

    if (error>max_error_threshold)
      return error+1;
  }

  return error;
}

inline void EMMS() { }
#endif

#endif
