/********************************************************************************
    Copyright (C) 1999  Dirk Farin

    This program is distributed under GNU Public License (GPL) as
    outlined in the COPYING file that comes with the source distribution.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************************************/

#include "video12/modules/mcomp_sgl_mmx.hh"

#include <iostream.h>
#include <iomanip.h>


static void InitMMXRegs()
{
  uint64 mask1 = 0x0101010101010101LL;
  uint64 mask2 = 0xFEFEFEFEFEFEFEFELL;

  __asm__
    (
     "movq %0,%%mm0\n\t"
     "movq %1,%%mm7\n\t"
     : : "m" (mask1), "m" (mask2)
     );
}



// -------------------------- SINGLE PREDICTION ----------------------------------

static void LumaFF(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp;
        Pixel* dp;

  sp = mc->lastimg.y;
  dp = mc->currimg.y;

  for (int y=mc->blkheight;y>0;y--)
    {
      const uint64* spd = (const uint64*)sp;
            uint64* dpd = (uint64*)dp;

      dpd[ 0]= spd[ 0];
      dpd[ 1]= spd[ 1];

      dp += mc->bytesperline_lum;
      sp += mc->bytesperline_lum;
    }
}

static void LumaHF(MotionCompensation_SglMB::MCData* mc)
{
  InitMMXRegs();

  const Pixel* sp;
        Pixel* dp;

  sp = mc->lastimg.y;
  dp = mc->currimg.y;

  uint64 xshift = (((uint32)sp) & 7);
  sp -= xshift;

  xshift <<= 3;               // left bits to ignore
  uint64 xshift2 = 64-xshift; // right bits to ignore
  uint64 xshift3 = 64-8-xshift;

  for (int y=mc->blkheight;y>0;y--)
    {
      __asm__
        (
         // 24 pixel aligned einlesen (mm1 mm2 mm3)
         "movq   (%0),%%mm1\n\t"
         "movq  8(%0),%%mm2\n\t"
         "movq 16(%0),%%mm3\n\t"

         "movq %%mm2,%%mm4\n\t"
         "movq %%mm3,%%mm6\n\t"  // fuer spaeter

         "psrlq %2,%%mm1\n\t"
         "psrlq %2,%%mm2\n\t"

         "psllq %3,%%mm4\n\t"
         "psllq %3,%%mm3\n\t"

         "por  %%mm4,%%mm1\n\t"
         "por  %%mm3,%%mm2\n\t"

         // 16 pixel sind nun in (mm1,mm2)

         "movq %%mm1,%%mm3\n\t"
         "movq %%mm2,%%mm4\n\t"
         "movq %%mm2,%%mm5\n\t"

         "psrlq $8,%%mm3\n\t"
         "psrlq $8,%%mm4\n\t"

         "psllq $56,%%mm5\n\t"
         "psllq %4,%%mm6\n\t"

         "por  %%mm5,%%mm3\n\t"
         "por  %%mm6,%%mm4\n\t"

         // weitere 16 pixel (Startposition 1 Pixel weiter rechts) sind nun in (mm3,mm4)

         // Rounding berechnen (mm5,mm6)

         "movq %%mm1,%%mm5\n\t"
         "por  %%mm3,%%mm5\n\t"
         "pand %%mm0,%%mm5\n\t"
         "movq %%mm2,%%mm6\n\t"
         "por  %%mm4,%%mm6\n\t"
         "pand %%mm0,%%mm6\n\t"

         // Pixelwerte addieren

         "pand    %%mm7,%%mm1\n\t"
         "pand    %%mm7,%%mm2\n\t"
         "pand    %%mm7,%%mm3\n\t"
         "pand    %%mm7,%%mm4\n\t"
         "psrlq   $1,%%mm1\n\t"
         "psrlq   $1,%%mm2\n\t"
         "psrlq   $1,%%mm3\n\t"
         "psrlq   $1,%%mm4\n\t"
         "paddusb %%mm3,%%mm1\n\t"
         "paddusb %%mm4,%%mm2\n\t"

         // Rounding addieren

         "paddusb %%mm5,%%mm1\n\t"
         "paddusb %%mm6,%%mm2\n\t"

         // Abspeichern

         "movq %%mm1, (%1)\n\t"
         "movq %%mm2,8(%1)\n\t"

         : : "r"(sp),"r"(dp),"m"(xshift),"m"(xshift2),"m"(xshift3)
         );

      sp += mc->bytesperline_lum;
      dp += mc->bytesperline_lum;
    }
}

static void LumaFH(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp;
        Pixel* dp;

  sp = mc->lastimg.y;
  dp = mc->currimg.y;

  for (int y=0;y<mc->blkheight;y++)
    {
      for (int x=0;x<16;x++)
        {
          *dp++ = (*sp + sp[mc->bytesperline_lum] + 1)>>1;
          sp++;
        }
      sp += mc->bytesperline_lum-16;
      dp += mc->bytesperline_lum-16;
    }
}

static void LumaHH(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp;
        Pixel* dp;

  sp = mc->lastimg.y;
  dp = mc->currimg.y;

  for (int y=0;y<mc->blkheight;y++)
    {
      int v1 = *sp + sp[mc->bytesperline_lum];

      for (int x=0;x<8;x++)
        {
          sp++;
          int v2 = *sp + sp[mc->bytesperline_lum];
          *dp++ = (v1 + v2 +2)>>2;

          sp++;
          v1 = *sp + sp[mc->bytesperline_lum];
          *dp++ = (v1 + v2 +2)>>2;
        }
      sp += mc->bytesperline_lum-16;
      dp += mc->bytesperline_lum-16;
    }
}

static void ChromaFF(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp = mc->lastimg.cr;
        Pixel* dp = mc->currimg.cr;

  int h = mc->blkheight_chr;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp++ = *sp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }


  sp = mc->lastimg.cb;
  dp = mc->currimg.cb;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp++ = *sp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }
}

static void ChromaFH(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp = mc->lastimg.cr;
        Pixel* dp = mc->currimg.cr;

  int h = mc->blkheight_chr;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp++ = (*sp + sp[mc->bytesperline_chr]+1)>>1; sp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }


  sp = mc->lastimg.cb;
  dp = mc->currimg.cb;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp++ = (*sp + sp[mc->bytesperline_chr]+1)>>1; sp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }
}

static void ChromaHF(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp = mc->lastimg.cr;
        Pixel* dp = mc->currimg.cr;

  int h = mc->blkheight_chr;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp++ = (*sp + sp[1]+1)>>1; sp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }


  sp = mc->lastimg.cb;
  dp = mc->currimg.cb;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp++ = (*sp + sp[1]+1)>>1; sp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }
}

static void ChromaHH(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp = mc->lastimg.cr;
        Pixel* dp = mc->currimg.cr;

  int h = mc->blkheight_chr;

  for (int y=0;y<h;y++)
    {
      int v1 = *sp + sp[mc->bytesperline_chr];

      for (int x=0;x<8;x++)
        {
          sp++;
          int v2 = *sp + sp[mc->bytesperline_chr];
          *dp++ = (v1 + v2+2)>>2;
          v1 = v2;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }


  sp = mc->lastimg.cb;
  dp = mc->currimg.cb;

  for (int y=0;y<h;y++)
    {
      int v1 = *sp + sp[mc->bytesperline_chr];

      for (int x=0;x<8;x++)
        {
          sp++;
          int v2 = *sp + sp[mc->bytesperline_chr];
          *dp++ = (v1 + v2+2)>>2;
          v1 = v2;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }
}





static void LumaFFadd(MotionCompensation_SglMB::MCData* mc)
{
  InitMMXRegs();

  const Pixel* sp;
        Pixel* dp;

  sp = mc->nextimg.y;
  dp = mc->currimg.y;

#if 1

  uint64 xshift = ((uint32)sp) & 7;

  if (xshift==0)
    {
      for (int y=mc->blkheight;y>0;y--)
        {
          // rp: mm1 mm4
          // dp: mm2 mm5
                  
          __asm__
            (
             "movq (%0),%%mm1\n\t"
             "movq (%1),%%mm2\n\t"
                     
             " movq %%mm1,%%mm3\n\t"
             "movq 8(%0),%%mm4\n\t"
             " por  %%mm2,%%mm3\n\t" // mm3 = rp[0] | dp[0]
             "movq 8(%1),%%mm5\n\t"
             " pand %%mm0,%%mm3\n\t" // mm3 = LSB of (rp[0] | dp[0]) = rounding[0]
                     
             "movq %%mm4,%%mm6\n\t"
             "por  %%mm5,%%mm6\n\t"
             "pand %%mm0,%%mm6\n\t"  // mm6 = LSB of (rp[1] | dp[1]) = rounding[1]
                     
             "pand %%mm7,%%mm1\n\t"     // mask out LSB
             "pand %%mm7,%%mm4\n\t"     // mask out LSB
             "psrlq $1,%%mm1\n\t"
             "pand %%mm7,%%mm2\n\t"     // mask out LSB
             "psrlq $1,%%mm2\n\t"
             "pand %%mm7,%%mm5\n\t"     // mask out LSB
             "psrlq $1,%%mm4\n\t"
             "paddusb  %%mm2, %%mm1\n\t"
             "psrlq $1,%%mm5\n\t"
             "paddusb  %%mm3, %%mm1\n\t"
             "movq   %%mm1,(%1)\n\t"      // dp[0] = (dp[0]>>1 + rp[0]>>1) + rounding[0]
                         
             "paddusb  %%mm5, %%mm4\n\t"
             "paddusb  %%mm6, %%mm4\n\t"
             "movq   %%mm4,8(%1)\n\t"     // dp[1] = (dp[1]>>1 + rp[1]>>1) + rounding[1]
                     
             : : "r"(sp),"r"(dp)
             );

          sp += mc->bytesperline_lum;
          dp += mc->bytesperline_lum;
        }
    }
  else
    {
      sp -= xshift;
      xshift <<= 3;               // left bits to ignore
      uint64 xshift2 = 64-xshift; // right bits to ignore
              
      for (int y=mc->blkheight;y>0;y--)
        {
          // rp: mm1 mm2 mm3   ->   mm1 mm3
          // dp:                    mm2 mm4
                  
          __asm__
            (
             // Read data aligned:
             // Read 3x 8bytes and shift data to correct position.
             // 16 pixels are in 

             "movq   (%0),%%mm1\n\t"  // rp[0] -> mm1
             "movq  8(%0),%%mm2\n\t"  // rp[1] -> mm2,mm4   (*1)
             " psrlq %2,%%mm1\n\t"    // note: shifting right because of little endian
             "movq 16(%0),%%mm3\n\t"  // rp[2] -> mm3
             " movq  %%mm2,%%mm4\n\t" // (*1)

             "psllq %3,%%mm4\n\t"
             "psrlq %2,%%mm2\n\t"
             " por   %%mm4,%%mm1\n\t" // combine first 8 pixels into mm1
             "psllq %3,%%mm3\n\t"
             "por   %%mm2,%%mm3\n\t"  // combine second 8 pixels into mm3


             "movq  (%1),%%mm2\n\t"   // dp[0]
             " movq %%mm1,%%mm5\n\t"

             "movq 8(%1),%%mm4\n\t"   // dp[1]
             " por  %%mm2,%%mm5\n\t"

             "pand %%mm0,%%mm5\n\t"   // mm5: rounding[0]
             " movq %%mm3,%%mm6\n\t"

             "por  %%mm4,%%mm6\n\t"
             " pand %%mm7,%%mm1\n\t"  // mask out LSB of rp[0]
             "pand %%mm0,%%mm6\n\t"   // mm6: rounding[1]
             " pand %%mm7,%%mm3\n\t"  // mask out LSB of rp[1]

             // calc mean

             "psrlq $1,%%mm1\n\t"
             " pand %%mm7,%%mm2\n\t"
             "psrlq $1,%%mm3\n\t"
             " pand %%mm7,%%mm4\n\t"
             "psrlq $1,%%mm2\n\t"
             " paddusb  %%mm5, %%mm1\n\t"   // rp[0] += rounding[0]
             "psrlq $1,%%mm4\n\t"
             " paddusb  %%mm6, %%mm3\n\t"   // rp[1] += rounding[1]
             "paddusb  %%mm2, %%mm1\n\t"    // rp[0] += dp[0]
             "movq   %%mm1, (%1)\n\t"
             " paddusb  %%mm4, %%mm3\n\t"   // rp[1] += dp[1]
             "movq   %%mm3,8(%1)\n\t"
                     
             : : "r"(sp),"r"(dp),"m"(xshift),"m"(xshift2)
             );

          sp += mc->bytesperline_lum;
          dp += mc->bytesperline_lum;
        }
    }

#else

  for (int y=0;y<mc->blkheight;y++)
    {
      for (int x=0;x<16;x++)
        {
          *dp = (*dp + *sp + 1)>>1;
          dp++;
          sp++;
        }
      sp += mc->bytesperline_lum-16;
      dp += mc->bytesperline_lum-16;
    }

#endif
}

static void LumaHFadd(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp;
        Pixel* dp;

  sp = mc->nextimg.y;
  dp = mc->currimg.y;

  for (int y=0;y<mc->blkheight;y++)
    {
      for (int x=0;x<16;x++)
        {
          *dp = (*dp + ((*sp + sp[1] + 1)>>1) + 1)>>1;
          dp++;
          sp++;
        }
      sp += mc->bytesperline_lum-16;
      dp += mc->bytesperline_lum-16;
    }
}

static void LumaFHadd(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp;
        Pixel* dp;

  sp = mc->nextimg.y;
  dp = mc->currimg.y;

  for (int y=0;y<mc->blkheight;y++)
    {
      for (int x=0;x<16;x++)
        {
          *dp = (*dp + ((*sp + sp[mc->bytesperline_lum] + 1)>>1) + 1)>>1;
          dp++;
          sp++;
        }
      sp += mc->bytesperline_lum-16;
      dp += mc->bytesperline_lum-16;
    }
}

static void LumaHHadd(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp;
        Pixel* dp;

  sp = mc->nextimg.y;
  dp = mc->currimg.y;

  for (int y=0;y<mc->blkheight;y++)
    {
      for (int x=0;x<16;x++)
        {
          *dp = (*dp + ((*sp + sp[1] + sp[mc->bytesperline_lum] + sp[mc->bytesperline_lum+1] +2)>>2)+1)>>1;
          dp++;
          sp++;
        }
      sp += mc->bytesperline_lum-16;
      dp += mc->bytesperline_lum-16;
    }
}

static void ChromaFFadd(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp = mc->nextimg.cr;
        Pixel* dp = mc->currimg.cr;

  int h = mc->blkheight_chr;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp = (*dp + *sp + 1)>>1;
          sp++;
          dp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }


  sp = mc->nextimg.cb;
  dp = mc->currimg.cb;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp = (*dp + *sp + 1)>>1;
          sp++;
          dp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }
}

static void ChromaFHadd(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp = mc->nextimg.cr;
        Pixel* dp = mc->currimg.cr;

  int h = mc->blkheight_chr;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp = (*dp + ((*sp + sp[mc->bytesperline_chr]+1)>>1) + 1)>>1;
          dp++;
          sp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }


  sp = mc->nextimg.cb;
  dp = mc->currimg.cb;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp = (*dp + ((*sp + sp[mc->bytesperline_chr]+1)>>1) + 1)>>1;
          dp++;
          sp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }
}

static void ChromaHFadd(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp = mc->nextimg.cr;
        Pixel* dp = mc->currimg.cr;

  int h = mc->blkheight_chr;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp = (*dp + ((*sp + sp[1]+1)>>1) + 1)>>1; dp++; sp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }


  sp = mc->nextimg.cb;
  dp = mc->currimg.cb;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp = (*dp + ((*sp + sp[1]+1)>>1) + 1)>>1; dp++; sp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }
}

static void ChromaHHadd(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp = mc->nextimg.cr;
        Pixel* dp = mc->currimg.cr;

  int h = mc->blkheight_chr;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp = (*dp + ((*sp + sp[1] + sp[mc->bytesperline_chr] + sp[mc->bytesperline_chr+1]+2)>>2)+1)>>1;
          sp++; dp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }


  sp = mc->nextimg.cb;
  dp = mc->currimg.cb;

  for (int y=0;y<h;y++)
    {
      for (int x=0;x<8;x++)
        {
          *dp = (*dp + ((*sp + sp[1] + sp[mc->bytesperline_chr] + sp[mc->bytesperline_chr+1]+2)>>2)+1)>>1;
          sp++; dp++;
        }
      sp += mc->bytesperline_chr-8;
      dp += mc->bytesperline_chr-8;
    }
}


static void MC_420_DBL_FFFF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFF(mc); LumaFFadd(mc); }
static void MC_420_DBL_FHFF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFF(mc); LumaFHadd(mc); }
static void MC_420_DBL_HFFF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFF(mc); LumaHFadd(mc); }
static void MC_420_DBL_HHFF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFF(mc); LumaHHadd(mc); }
static void MC_420_DBL_FFFH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFH(mc); LumaFFadd(mc); }
static void MC_420_DBL_FHFH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFH(mc); LumaFHadd(mc); }
static void MC_420_DBL_HFFH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFH(mc); LumaHFadd(mc); }
static void MC_420_DBL_HHFH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFH(mc); LumaHHadd(mc); }
static void MC_420_DBL_FFHF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHF(mc); LumaFFadd(mc); }
static void MC_420_DBL_FHHF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHF(mc); LumaFHadd(mc); }
static void MC_420_DBL_HFHF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHF(mc); LumaHFadd(mc); }
static void MC_420_DBL_HHHF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHF(mc); LumaHHadd(mc); }
static void MC_420_DBL_FFHH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHH(mc); LumaFFadd(mc); }
static void MC_420_DBL_FHHH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHH(mc); LumaFHadd(mc); }
static void MC_420_DBL_HFHH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHH(mc); LumaHFadd(mc); }
static void MC_420_DBL_HHHH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHH(mc); LumaHHadd(mc); }
              
static void MC_420_DBL_FFFF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFF(mc); ChromaFFadd(mc); }
static void MC_420_DBL_FHFF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFF(mc); ChromaFHadd(mc); }
static void MC_420_DBL_HFFF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFF(mc); ChromaHFadd(mc); }
static void MC_420_DBL_HHFF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFF(mc); ChromaHHadd(mc); }
static void MC_420_DBL_FFFH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFH(mc); ChromaFFadd(mc); }
static void MC_420_DBL_FHFH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFH(mc); ChromaFHadd(mc); }
static void MC_420_DBL_HFFH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFH(mc); ChromaHFadd(mc); }
static void MC_420_DBL_HHFH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFH(mc); ChromaHHadd(mc); }
static void MC_420_DBL_FFHF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHF(mc); ChromaFFadd(mc); }
static void MC_420_DBL_FHHF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHF(mc); ChromaFHadd(mc); }
static void MC_420_DBL_HFHF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHF(mc); ChromaHFadd(mc); }
static void MC_420_DBL_HHHF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHF(mc); ChromaHHadd(mc); }
static void MC_420_DBL_FFHH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHH(mc); ChromaFFadd(mc); }
static void MC_420_DBL_FHHH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHH(mc); ChromaFHadd(mc); }
static void MC_420_DBL_HFHH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHH(mc); ChromaHFadd(mc); }
static void MC_420_DBL_HHHH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHH(mc); ChromaHHadd(mc); }



MotionCompensation_SglMB_MMX::MotionCompensation_SglMB_MMX()
{
  sglluma[0]   = LumaFF;
  sglluma[MC_Last_HalfH] = LumaHF;
  sglluma[MC_Last_HalfV] = LumaFH;
  sglluma[MC_Last_HalfH | MC_Last_HalfV] = LumaHH;

  sglchroma420[0] = ChromaFF;
  sglchroma420[MC_Last_HalfH] = ChromaHF;
  sglchroma420[MC_Last_HalfV] = ChromaFH;
  sglchroma420[MC_Last_HalfH | MC_Last_HalfV] = ChromaHH;

  for (int i=0;i<4;i++)
    {
      sglchroma422[i] = ChromaFF;
      sglchroma444[i] = ChromaFF;
    }

  dblluma[ 0] = MC_420_DBL_FFFF_Luma;
  dblluma[ 1] = MC_420_DBL_FFFH_Luma;
  dblluma[ 2] = MC_420_DBL_FFHF_Luma;
  dblluma[ 3] = MC_420_DBL_FFHH_Luma;
  dblluma[ 4] = MC_420_DBL_FHFF_Luma;
  dblluma[ 5] = MC_420_DBL_FHFH_Luma;
  dblluma[ 6] = MC_420_DBL_FHHF_Luma;
  dblluma[ 7] = MC_420_DBL_FHHH_Luma;
  dblluma[ 8] = MC_420_DBL_HFFF_Luma;
  dblluma[ 9] = MC_420_DBL_HFFH_Luma;
  dblluma[10] = MC_420_DBL_HFHF_Luma;
  dblluma[11] = MC_420_DBL_HFHH_Luma;
  dblluma[12] = MC_420_DBL_HHFF_Luma;
  dblluma[13] = MC_420_DBL_HHFH_Luma;
  dblluma[14] = MC_420_DBL_HHHF_Luma;
  dblluma[15] = MC_420_DBL_HHHH_Luma;

  dblchroma420[ 0] = MC_420_DBL_FFFF_Chroma;
  dblchroma420[ 1] = MC_420_DBL_FFFH_Chroma;
  dblchroma420[ 2] = MC_420_DBL_FFHF_Chroma;
  dblchroma420[ 3] = MC_420_DBL_FFHH_Chroma;
  dblchroma420[ 4] = MC_420_DBL_FHFF_Chroma;
  dblchroma420[ 5] = MC_420_DBL_FHFH_Chroma;
  dblchroma420[ 6] = MC_420_DBL_FHHF_Chroma;
  dblchroma420[ 7] = MC_420_DBL_FHHH_Chroma;
  dblchroma420[ 8] = MC_420_DBL_HFFF_Chroma;
  dblchroma420[ 9] = MC_420_DBL_HFFH_Chroma;
  dblchroma420[10] = MC_420_DBL_HFHF_Chroma;
  dblchroma420[11] = MC_420_DBL_HFHH_Chroma;
  dblchroma420[12] = MC_420_DBL_HHFF_Chroma;
  dblchroma420[13] = MC_420_DBL_HHFH_Chroma;
  dblchroma420[14] = MC_420_DBL_HHHF_Chroma;
  dblchroma420[15] = MC_420_DBL_HHHH_Chroma;
}

MotionCompensation_SglMB::MCompFunc*const*
MotionCompensation_SglMB_MMX::AskMCompFunc_Sgl_Luma() const
{
  return sglluma;
}

MotionCompensation_SglMB::MCompFunc*const*
MotionCompensation_SglMB_MMX::AskMCompFunc_Dbl_Luma() const
{
  return dblluma;
}

MotionCompensation_SglMB::MCompFunc*const*
MotionCompensation_SglMB_MMX::AskMCompFunc_Sgl_Chroma(uint2 chroma) const
{
  return sglchroma420;
}

MotionCompensation_SglMB::MCompFunc*const*
MotionCompensation_SglMB_MMX::AskMCompFunc_Dbl_Chroma(uint2 chroma) const
{
  return dblchroma420;
}

