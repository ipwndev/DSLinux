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

#include "video12/modules/mcomp_sgl_simple.hh"

#include <iostream.h>
#include <iomanip.h>

// -------------------------- SINGLE PREDICTION ----------------------------------

static void LumaFF(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp;
        Pixel* dp;

  sp = mc->lastimg.y;
  dp = mc->currimg.y;

  for (int y=mc->blkheight;y>0;y--)
    {
#if 0
      uint64* lsp=(uint64*)sp;
      uint64* ldp=(uint64*)dp;

      uint64 t1=lsp[0];
      uint64 t2=lsp[1];
      ldp[0] = t1;
      ldp[1] = t2;
#endif

#if 1
      dp[ 0]= sp[ 0];
      dp[ 1]= sp[ 1];
      dp[ 2]= sp[ 2];
      dp[ 3]= sp[ 3];
      dp[ 4]= sp[ 4];
      dp[ 5]= sp[ 5];
      dp[ 6]= sp[ 6];
      dp[ 7]= sp[ 7];
      dp[ 8]= sp[ 8];
      dp[ 9]= sp[ 9];
      dp[10]= sp[10];
      dp[11]= sp[11];
      dp[12]= sp[12];
      dp[13]= sp[13];
      dp[14]= sp[14];
      dp[15]= sp[15];
#endif
      dp += mc->bytesperline_lum;
      sp += mc->bytesperline_lum;
    }
}

#if 0

static void MC16xXX_420_SGL_FF_Chroma(MotionCompensation_SglMB::MCData* mc)
{
  const int y0    = p->y0 >>1;
  const int x0    = p->x0 >>1;
  const int yend  = y0 + p->blkheight_chr;
  const int x1    = x0 + ((p->sglpred->h/2)>>1);
  const int yoffs = ((p->sglpred->v/2)>>1);

  for (int y=y0;y<yend;y++)
    {
      const Pixel* rp = &p->sglpred->cb[y+yoffs][x1];
            Pixel* dp = &p->current.cb[y][x0];

      dp[0]= rp[0];
      dp[1]= rp[1];
      dp[2]= rp[2];
      dp[3]= rp[3];
      dp[4]= rp[4];
      dp[5]= rp[5];
      dp[6]= rp[6];
      dp[7]= rp[7];

      rp = &p->sglpred->cr[y+yoffs][x1];
      dp = &p->current.cr[y][x0];

      dp[0]= rp[0];
      dp[1]= rp[1];
      dp[2]= rp[2];
      dp[3]= rp[3];
      dp[4]= rp[4];
      dp[5]= rp[5];
      dp[6]= rp[6];
      dp[7]= rp[7];
    }
}


static void MC16xXX_420_SGL_HF_Luma(MotionCompensation_SglMB::MCData* mc)
{
  const int yend  = p->y0 + p->blkheight;
  const int x1    = p->x0 + (p->sglpred->h>>1);
  const int dy    = (p->sglpred->v>>1);

  for (int y=p->y0;y<yend;y++)
    {
      int y1 = y+dy;
      const Pixel* rp = &p->sglpred->y[y1][x1];
            Pixel* dp = &p->current.y [y ][p->x0];

      dp[ 0]=(rp[ 0]+rp[ 1]+MV_MEAN_CONST)>>1;
      dp[ 1]=(rp[ 1]+rp[ 2]+MV_MEAN_CONST)>>1;
      dp[ 2]=(rp[ 2]+rp[ 3]+MV_MEAN_CONST)>>1;
      dp[ 3]=(rp[ 3]+rp[ 4]+MV_MEAN_CONST)>>1;
      dp[ 4]=(rp[ 4]+rp[ 5]+MV_MEAN_CONST)>>1;
      dp[ 5]=(rp[ 5]+rp[ 6]+MV_MEAN_CONST)>>1;
      dp[ 6]=(rp[ 6]+rp[ 7]+MV_MEAN_CONST)>>1;
      dp[ 7]=(rp[ 7]+rp[ 8]+MV_MEAN_CONST)>>1;
      dp[ 8]=(rp[ 8]+rp[ 9]+MV_MEAN_CONST)>>1;
      dp[ 9]=(rp[ 9]+rp[10]+MV_MEAN_CONST)>>1;
      dp[10]=(rp[10]+rp[11]+MV_MEAN_CONST)>>1;
      dp[11]=(rp[11]+rp[12]+MV_MEAN_CONST)>>1;
      dp[12]=(rp[12]+rp[13]+MV_MEAN_CONST)>>1;
      dp[13]=(rp[13]+rp[14]+MV_MEAN_CONST)>>1;
      dp[14]=(rp[14]+rp[15]+MV_MEAN_CONST)>>1;
      dp[15]=(rp[15]+rp[16]+MV_MEAN_CONST)>>1;
    }
}

static void MC16xXX_420_SGL_HF_Chroma(MotionCompensation_Common::PredData* p)
{
  const int y0    = p->y0 >>1;
  const int x0    = p->x0 >>1;
  const int yend  = y0 + p->blkheight_chr;
  const int x1    = x0 + ((p->sglpred->h/2)>>1);
  const int yoffs = ((p->sglpred->v/2)>>1);

  for (int y=y0;y<yend;y++)
    {
      const Pixel* rp = &p->sglpred->cb[y+yoffs][x1];
            Pixel* dp = &p->current.cb[y][x0];

      dp[ 0]=(rp[ 0]+rp[ 1]+MV_MEAN_CONST)>>1;
      dp[ 1]=(rp[ 1]+rp[ 2]+MV_MEAN_CONST)>>1;
      dp[ 2]=(rp[ 2]+rp[ 3]+MV_MEAN_CONST)>>1;
      dp[ 3]=(rp[ 3]+rp[ 4]+MV_MEAN_CONST)>>1;
      dp[ 4]=(rp[ 4]+rp[ 5]+MV_MEAN_CONST)>>1;
      dp[ 5]=(rp[ 5]+rp[ 6]+MV_MEAN_CONST)>>1;
      dp[ 6]=(rp[ 6]+rp[ 7]+MV_MEAN_CONST)>>1;
      dp[ 7]=(rp[ 7]+rp[ 8]+MV_MEAN_CONST)>>1;

      rp = &p->sglpred->cr[y+yoffs][x1];
      dp = &p->current.cr[y][x0];

      dp[ 0]=(rp[ 0]+rp[ 1]+MV_MEAN_CONST)>>1;
      dp[ 1]=(rp[ 1]+rp[ 2]+MV_MEAN_CONST)>>1;
      dp[ 2]=(rp[ 2]+rp[ 3]+MV_MEAN_CONST)>>1;
      dp[ 3]=(rp[ 3]+rp[ 4]+MV_MEAN_CONST)>>1;
      dp[ 4]=(rp[ 4]+rp[ 5]+MV_MEAN_CONST)>>1;
      dp[ 5]=(rp[ 5]+rp[ 6]+MV_MEAN_CONST)>>1;
      dp[ 6]=(rp[ 6]+rp[ 7]+MV_MEAN_CONST)>>1;
      dp[ 7]=(rp[ 7]+rp[ 8]+MV_MEAN_CONST)>>1;
    }
}


static void MC16xXX_420_SGL_FH_Luma(MotionCompensation_Common::PredData* p)
{
  const int yend  = p->y0 + p->blkheight;
  const int x1    = p->x0 + (p->sglpred->h>>1);
  const int dy    = (p->sglpred->v>>1);

  for (int y=p->y0;y<yend;y++)
    {
      int y1 = y+dy;
      const Pixel* rp = &p->sglpred->y[y1  ][x1];
      const Pixel* rpb= &p->sglpred->y[y1+1][x1];
      Pixel* dp = &p->current.y [y ][p->x0];

      dp[ 0]=(rp[ 0]+rpb[ 0]+MV_MEAN_CONST)>>1;
      dp[ 1]=(rp[ 1]+rpb[ 1]+MV_MEAN_CONST)>>1;
      dp[ 2]=(rp[ 2]+rpb[ 2]+MV_MEAN_CONST)>>1;
      dp[ 3]=(rp[ 3]+rpb[ 3]+MV_MEAN_CONST)>>1;
      dp[ 4]=(rp[ 4]+rpb[ 4]+MV_MEAN_CONST)>>1;
      dp[ 5]=(rp[ 5]+rpb[ 5]+MV_MEAN_CONST)>>1;
      dp[ 6]=(rp[ 6]+rpb[ 6]+MV_MEAN_CONST)>>1;
      dp[ 7]=(rp[ 7]+rpb[ 7]+MV_MEAN_CONST)>>1;
      dp[ 8]=(rp[ 8]+rpb[ 8]+MV_MEAN_CONST)>>1;
      dp[ 9]=(rp[ 9]+rpb[ 9]+MV_MEAN_CONST)>>1;
      dp[10]=(rp[10]+rpb[10]+MV_MEAN_CONST)>>1;
      dp[11]=(rp[11]+rpb[11]+MV_MEAN_CONST)>>1;
      dp[12]=(rp[12]+rpb[12]+MV_MEAN_CONST)>>1;
      dp[13]=(rp[13]+rpb[13]+MV_MEAN_CONST)>>1;
      dp[14]=(rp[14]+rpb[14]+MV_MEAN_CONST)>>1;
      dp[15]=(rp[15]+rpb[15]+MV_MEAN_CONST)>>1;
    }
}


static void MC16xXX_420_SGL_FH_Chroma(MotionCompensation_Common::PredData* p)
{
  const int y0    = p->y0 >>1;
  const int x0    = p->x0 >>1;
  const int yend  = y0 + p->blkheight_chr;
  const int x1    = x0 + ((p->sglpred->h/2)>>1);
  const int yoffs = ((p->sglpred->v/2)>>1);

  for (int y=y0;y<yend;y++)
    {
      const Pixel* rp = &p->sglpred->cb[y+yoffs  ][x1];
      const Pixel* rpb= &p->sglpred->cb[y+yoffs+1][x1];
            Pixel* dp = &p->current.cb[y][x0];

      dp[ 0]=(rp[ 0]+rpb[ 0]+MV_MEAN_CONST)>>1;
      dp[ 1]=(rp[ 1]+rpb[ 1]+MV_MEAN_CONST)>>1;
      dp[ 2]=(rp[ 2]+rpb[ 2]+MV_MEAN_CONST)>>1;
      dp[ 3]=(rp[ 3]+rpb[ 3]+MV_MEAN_CONST)>>1;
      dp[ 4]=(rp[ 4]+rpb[ 4]+MV_MEAN_CONST)>>1;
      dp[ 5]=(rp[ 5]+rpb[ 5]+MV_MEAN_CONST)>>1;
      dp[ 6]=(rp[ 6]+rpb[ 6]+MV_MEAN_CONST)>>1;
      dp[ 7]=(rp[ 7]+rpb[ 7]+MV_MEAN_CONST)>>1;

      rp = &p->sglpred->cr[y+yoffs][x1];
      rpb= &p->sglpred->cr[y+yoffs+1][x1];
      dp = &p->current.cr[y][x0];

      dp[ 0]=(rp[ 0]+rpb[ 0]+MV_MEAN_CONST)>>1;
      dp[ 1]=(rp[ 1]+rpb[ 1]+MV_MEAN_CONST)>>1;
      dp[ 2]=(rp[ 2]+rpb[ 2]+MV_MEAN_CONST)>>1;
      dp[ 3]=(rp[ 3]+rpb[ 3]+MV_MEAN_CONST)>>1;
      dp[ 4]=(rp[ 4]+rpb[ 4]+MV_MEAN_CONST)>>1;
      dp[ 5]=(rp[ 5]+rpb[ 5]+MV_MEAN_CONST)>>1;
      dp[ 6]=(rp[ 6]+rpb[ 6]+MV_MEAN_CONST)>>1;
      dp[ 7]=(rp[ 7]+rpb[ 7]+MV_MEAN_CONST)>>1;
    }
}


static void MC16xXX_420_SGL_HH_Luma(MotionCompensation_Common::PredData* p)
{
  const int yend  = p->y0 + p->blkheight;
  const int x1    = p->x0 + (p->sglpred->h>>1);
  const int dy    = (p->sglpred->v>>1);

  for (int y=p->y0;y<yend;y++)
    {
      int y1 = y+dy;
      const Pixel* rp = &p->sglpred->y[y1  ][x1];
      const Pixel* rpb= &p->sglpred->y[y1+1][x1];
            Pixel* dp = &p->current.y [y ][p->x0];

      dp[ 0]=(rp[ 0]+rp[ 1]+rpb[ 0]+rpb[ 1]+2)>>2;
      dp[ 1]=(rp[ 1]+rp[ 2]+rpb[ 1]+rpb[ 2]+2)>>2;
      dp[ 2]=(rp[ 2]+rp[ 3]+rpb[ 2]+rpb[ 3]+2)>>2;
      dp[ 3]=(rp[ 3]+rp[ 4]+rpb[ 3]+rpb[ 4]+2)>>2;
      dp[ 4]=(rp[ 4]+rp[ 5]+rpb[ 4]+rpb[ 5]+2)>>2;
      dp[ 5]=(rp[ 5]+rp[ 6]+rpb[ 5]+rpb[ 6]+2)>>2;
      dp[ 6]=(rp[ 6]+rp[ 7]+rpb[ 6]+rpb[ 7]+2)>>2;
      dp[ 7]=(rp[ 7]+rp[ 8]+rpb[ 7]+rpb[ 8]+2)>>2;
      dp[ 8]=(rp[ 8]+rp[ 9]+rpb[ 8]+rpb[ 9]+2)>>2;
      dp[ 9]=(rp[ 9]+rp[10]+rpb[ 9]+rpb[10]+2)>>2;
      dp[10]=(rp[10]+rp[11]+rpb[10]+rpb[11]+2)>>2;
      dp[11]=(rp[11]+rp[12]+rpb[11]+rpb[12]+2)>>2;
      dp[12]=(rp[12]+rp[13]+rpb[12]+rpb[13]+2)>>2;
      dp[13]=(rp[13]+rp[14]+rpb[13]+rpb[14]+2)>>2;
      dp[14]=(rp[14]+rp[15]+rpb[14]+rpb[15]+2)>>2;
      dp[15]=(rp[15]+rp[16]+rpb[15]+rpb[16]+2)>>2;
    }
}

static void MC16xXX_420_SGL_HH_Chroma(MotionCompensation_Common::PredData* p)
{
  const int y0    = p->y0 >>1;
  const int x0    = p->x0 >>1;
  const int yend  = y0 + p->blkheight_chr;
  const int x1    = x0 + ((p->sglpred->h/2)>>1);
  const int yoffs = ((p->sglpred->v/2)>>1);

  for (int y=y0;y<yend;y++)
    {
      const Pixel* rp = &p->sglpred->cb[y+yoffs  ][x1];
      const Pixel* rpb= &p->sglpred->cb[y+yoffs+1][x1];
            Pixel* dp = &p->current.cb[y][x0];

      dp[ 0]=(rp[ 0]+rp[ 1]+rpb[ 0]+rpb[ 1]+2)>>2;
      dp[ 1]=(rp[ 1]+rp[ 2]+rpb[ 1]+rpb[ 2]+2)>>2;
      dp[ 2]=(rp[ 2]+rp[ 3]+rpb[ 2]+rpb[ 3]+2)>>2;
      dp[ 3]=(rp[ 3]+rp[ 4]+rpb[ 3]+rpb[ 4]+2)>>2;
      dp[ 4]=(rp[ 4]+rp[ 5]+rpb[ 4]+rpb[ 5]+2)>>2;
      dp[ 5]=(rp[ 5]+rp[ 6]+rpb[ 5]+rpb[ 6]+2)>>2;
      dp[ 6]=(rp[ 6]+rp[ 7]+rpb[ 6]+rpb[ 7]+2)>>2;
      dp[ 7]=(rp[ 7]+rp[ 8]+rpb[ 7]+rpb[ 8]+2)>>2;

      rp = &p->sglpred->cr[y+yoffs][x1];
      rpb= &p->sglpred->cr[y+yoffs+1][x1];
      dp = &p->current.cr[y][x0];

      dp[ 0]=(rp[ 0]+rp[ 1]+rpb[ 0]+rpb[ 1]+2)>>2;
      dp[ 1]=(rp[ 1]+rp[ 2]+rpb[ 1]+rpb[ 2]+2)>>2;
      dp[ 2]=(rp[ 2]+rp[ 3]+rpb[ 2]+rpb[ 3]+2)>>2;
      dp[ 3]=(rp[ 3]+rp[ 4]+rpb[ 3]+rpb[ 4]+2)>>2;
      dp[ 4]=(rp[ 4]+rp[ 5]+rpb[ 4]+rpb[ 5]+2)>>2;
      dp[ 5]=(rp[ 5]+rp[ 6]+rpb[ 5]+rpb[ 6]+2)>>2;
      dp[ 6]=(rp[ 6]+rp[ 7]+rpb[ 6]+rpb[ 7]+2)>>2;
      dp[ 7]=(rp[ 7]+rp[ 8]+rpb[ 7]+rpb[ 8]+2)>>2;
    }
}



// -------------------------- SINGLE PREDICTION / Add----------------------------------

static void MC16xXXadd_420_SGL_FF_Luma(MotionCompensation_Common::PredData* p)
{
  const int yend  = p->y0 + p->blkheight;
  const int x1    = p->x0 + (p->sglpred->h>>1);
  const int dy    = (p->sglpred->v>>1);

  for (int y=p->y0;y<yend;y++)
    {
      int y1 = y+dy;
      const Pixel* rp = &p->sglpred->y[y1][x1];
            Pixel* dp = &p->current.y [y ][p->x0];

      dp[ 0]= (dp[ 0]+rp[ 0]+MV_MEAN_CONST)>>1;
      dp[ 1]= (dp[ 1]+rp[ 1]+MV_MEAN_CONST)>>1;
      dp[ 2]= (dp[ 2]+rp[ 2]+MV_MEAN_CONST)>>1;
      dp[ 3]= (dp[ 3]+rp[ 3]+MV_MEAN_CONST)>>1;
      dp[ 4]= (dp[ 4]+rp[ 4]+MV_MEAN_CONST)>>1;
      dp[ 5]= (dp[ 5]+rp[ 5]+MV_MEAN_CONST)>>1;
      dp[ 6]= (dp[ 6]+rp[ 6]+MV_MEAN_CONST)>>1;
      dp[ 7]= (dp[ 7]+rp[ 7]+MV_MEAN_CONST)>>1;
      dp[ 8]= (dp[ 8]+rp[ 8]+MV_MEAN_CONST)>>1;
      dp[ 9]= (dp[ 9]+rp[ 9]+MV_MEAN_CONST)>>1;
      dp[10]= (dp[10]+rp[10]+MV_MEAN_CONST)>>1;
      dp[11]= (dp[11]+rp[11]+MV_MEAN_CONST)>>1;
      dp[12]= (dp[12]+rp[12]+MV_MEAN_CONST)>>1;
      dp[13]= (dp[13]+rp[13]+MV_MEAN_CONST)>>1;
      dp[14]= (dp[14]+rp[14]+MV_MEAN_CONST)>>1;
      dp[15]= (dp[15]+rp[15]+MV_MEAN_CONST)>>1;
    }
}

static void MC16xXXadd_420_SGL_FF_Chroma(MotionCompensation_Common::PredData* p)
{
  const int y0    = p->y0 >>1;
  const int x0    = p->x0 >>1;
  const int yend  = y0 + p->blkheight_chr;
  const int x1    = x0 + ((p->sglpred->h/2)>>1);
  const int yoffs = ((p->sglpred->v/2)>>1);

  for (int y=y0;y<yend;y++)
    {
      const Pixel* rp = &p->sglpred->cb[y+yoffs][x1];
            Pixel* dp = &p->current.cb[y][x0];

      dp[ 0]= (dp[ 0]+rp[ 0]+MV_MEAN_CONST)>>1;
      dp[ 1]= (dp[ 1]+rp[ 1]+MV_MEAN_CONST)>>1;
      dp[ 2]= (dp[ 2]+rp[ 2]+MV_MEAN_CONST)>>1;
      dp[ 3]= (dp[ 3]+rp[ 3]+MV_MEAN_CONST)>>1;
      dp[ 4]= (dp[ 4]+rp[ 4]+MV_MEAN_CONST)>>1;
      dp[ 5]= (dp[ 5]+rp[ 5]+MV_MEAN_CONST)>>1;
      dp[ 6]= (dp[ 6]+rp[ 6]+MV_MEAN_CONST)>>1;
      dp[ 7]= (dp[ 7]+rp[ 7]+MV_MEAN_CONST)>>1;

      rp = &p->sglpred->cr[y+yoffs][x1];
      dp = &p->current.cr[y][x0];

      dp[ 0]= (dp[ 0]+rp[ 0]+MV_MEAN_CONST)>>1;
      dp[ 1]= (dp[ 1]+rp[ 1]+MV_MEAN_CONST)>>1;
      dp[ 2]= (dp[ 2]+rp[ 2]+MV_MEAN_CONST)>>1;
      dp[ 3]= (dp[ 3]+rp[ 3]+MV_MEAN_CONST)>>1;
      dp[ 4]= (dp[ 4]+rp[ 4]+MV_MEAN_CONST)>>1;
      dp[ 5]= (dp[ 5]+rp[ 5]+MV_MEAN_CONST)>>1;
      dp[ 6]= (dp[ 6]+rp[ 6]+MV_MEAN_CONST)>>1;
      dp[ 7]= (dp[ 7]+rp[ 7]+MV_MEAN_CONST)>>1;
    }
}


static void MC16xXXadd_420_SGL_HF_Luma(MotionCompensation_Common::PredData* p)
{
  const int yend  = p->y0 + p->blkheight;
  const int x1    = p->x0 + (p->sglpred->h>>1);
  const int dy    = (p->sglpred->v>>1);

  for (int y=p->y0;y<yend;y++)
    {
      int y1 = y+dy;
      const Pixel* rp = &p->sglpred->y[y1][x1];
            Pixel* dp = &p->current.y [y ][p->x0];

      dp[ 0]=(dp[ 0]+((rp[ 0]+rp[ 1]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 1]=(dp[ 1]+((rp[ 1]+rp[ 2]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 2]=(dp[ 2]+((rp[ 2]+rp[ 3]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 3]=(dp[ 3]+((rp[ 3]+rp[ 4]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 4]=(dp[ 4]+((rp[ 4]+rp[ 5]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 5]=(dp[ 5]+((rp[ 5]+rp[ 6]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 6]=(dp[ 6]+((rp[ 6]+rp[ 7]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 7]=(dp[ 7]+((rp[ 7]+rp[ 8]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 8]=(dp[ 8]+((rp[ 8]+rp[ 9]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 9]=(dp[ 9]+((rp[ 9]+rp[10]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[10]=(dp[10]+((rp[10]+rp[11]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[11]=(dp[11]+((rp[11]+rp[12]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[12]=(dp[12]+((rp[12]+rp[13]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[13]=(dp[13]+((rp[13]+rp[14]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[14]=(dp[14]+((rp[14]+rp[15]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[15]=(dp[15]+((rp[15]+rp[16]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
    }
}

static void MC16xXXadd_420_SGL_HF_Chroma(MotionCompensation_Common::PredData* p)
{
  const int y0    = p->y0 >>1;
  const int x0    = p->x0 >>1;
  const int yend  = y0 + p->blkheight_chr;
  const int x1    = x0 + ((p->sglpred->h/2)>>1);
  const int yoffs = ((p->sglpred->v/2)>>1);

  for (int y=y0;y<yend;y++)
    {
      const Pixel* rp = &p->sglpred->cb[y+yoffs][x1];
            Pixel* dp = &p->current.cb[y][x0];

      dp[ 0]=(dp[ 0]+((rp[ 0]+rp[ 1]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 1]=(dp[ 1]+((rp[ 1]+rp[ 2]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 2]=(dp[ 2]+((rp[ 2]+rp[ 3]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 3]=(dp[ 3]+((rp[ 3]+rp[ 4]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 4]=(dp[ 4]+((rp[ 4]+rp[ 5]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 5]=(dp[ 5]+((rp[ 5]+rp[ 6]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 6]=(dp[ 6]+((rp[ 6]+rp[ 7]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 7]=(dp[ 7]+((rp[ 7]+rp[ 8]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;

      rp = &p->sglpred->cr[y+yoffs][x1];
      dp = &p->current.cr[y][x0];

      dp[ 0]=(dp[ 0]+((rp[ 0]+rp[ 1]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 1]=(dp[ 1]+((rp[ 1]+rp[ 2]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 2]=(dp[ 2]+((rp[ 2]+rp[ 3]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 3]=(dp[ 3]+((rp[ 3]+rp[ 4]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 4]=(dp[ 4]+((rp[ 4]+rp[ 5]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 5]=(dp[ 5]+((rp[ 5]+rp[ 6]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 6]=(dp[ 6]+((rp[ 6]+rp[ 7]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 7]=(dp[ 7]+((rp[ 7]+rp[ 8]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
    }
}


static void MC16xXXadd_420_SGL_FH_Luma(MotionCompensation_Common::PredData* p)
{
  const int yend  = p->y0 + p->blkheight;
  const int x1    = p->x0 + (p->sglpred->h>>1);
  const int dy    = (p->sglpred->v>>1);

  for (int y=p->y0;y<yend;y++)
    {
      int y1 = y+dy;
      const Pixel* rp = &p->sglpred->y[y1  ][x1];
      const Pixel* rpb= &p->sglpred->y[y1+1][x1];
            Pixel* dp = &p->current.y [y ][p->x0];

      dp[ 0]=(dp[ 0]+((rp[ 0]+rpb[ 0]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 1]=(dp[ 1]+((rp[ 1]+rpb[ 1]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 2]=(dp[ 2]+((rp[ 2]+rpb[ 2]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 3]=(dp[ 3]+((rp[ 3]+rpb[ 3]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 4]=(dp[ 4]+((rp[ 4]+rpb[ 4]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 5]=(dp[ 5]+((rp[ 5]+rpb[ 5]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 6]=(dp[ 6]+((rp[ 6]+rpb[ 6]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 7]=(dp[ 7]+((rp[ 7]+rpb[ 7]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 8]=(dp[ 8]+((rp[ 8]+rpb[ 8]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 9]=(dp[ 9]+((rp[ 9]+rpb[ 9]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[10]=(dp[10]+((rp[10]+rpb[10]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[11]=(dp[11]+((rp[11]+rpb[11]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[12]=(dp[12]+((rp[12]+rpb[12]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[13]=(dp[13]+((rp[13]+rpb[13]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[14]=(dp[14]+((rp[14]+rpb[14]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[15]=(dp[15]+((rp[15]+rpb[15]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
    }
}


static void MC16xXXadd_420_SGL_FH_Chroma(MotionCompensation_Common::PredData* p)
{
  const int y0    = p->y0 >>1;
  const int x0    = p->x0 >>1;
  const int yend  = y0 + p->blkheight_chr;
  const int x1    = x0 + ((p->sglpred->h/2)>>1);
  const int yoffs = ((p->sglpred->v/2)>>1);

  for (int y=y0;y<yend;y++)
    {
      const Pixel* rp = &p->sglpred->cb[y+yoffs  ][x1];
      const Pixel* rpb= &p->sglpred->cb[y+yoffs+1][x1];
            Pixel* dp = &p->current.cb[y][x0];

      dp[ 0]=(dp[ 0]+((rp[ 0]+rpb[ 0]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 1]=(dp[ 1]+((rp[ 1]+rpb[ 1]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 2]=(dp[ 2]+((rp[ 2]+rpb[ 2]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 3]=(dp[ 3]+((rp[ 3]+rpb[ 3]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 4]=(dp[ 4]+((rp[ 4]+rpb[ 4]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 5]=(dp[ 5]+((rp[ 5]+rpb[ 5]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 6]=(dp[ 6]+((rp[ 6]+rpb[ 6]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 7]=(dp[ 7]+((rp[ 7]+rpb[ 7]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;

      rp = &p->sglpred->cr[y+yoffs][x1];
      rpb= &p->sglpred->cr[y+yoffs+1][x1];
      dp = &p->current.cr[y][x0];

      dp[ 0]=(dp[ 0]+((rp[ 0]+rpb[ 0]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 1]=(dp[ 1]+((rp[ 1]+rpb[ 1]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 2]=(dp[ 2]+((rp[ 2]+rpb[ 2]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 3]=(dp[ 3]+((rp[ 3]+rpb[ 3]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 4]=(dp[ 4]+((rp[ 4]+rpb[ 4]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 5]=(dp[ 5]+((rp[ 5]+rpb[ 5]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 6]=(dp[ 6]+((rp[ 6]+rpb[ 6]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
      dp[ 7]=(dp[ 7]+((rp[ 7]+rpb[ 7]+MV_MEAN_CONST)>>1)+MV_MEAN_CONST)>>1;
    }
}


static void MC16xXXadd_420_SGL_HH_Luma(MotionCompensation_Common::PredData* p)
{
  const int yend  = p->y0 + p->blkheight;
  const int x1    = p->x0 + (p->sglpred->h>>1);
  const int dy    = (p->sglpred->v>>1);

  for (int y=p->y0;y<yend;y++)
    {
      int y1 = y+dy;
      const Pixel* rp = &p->sglpred->y[y1  ][x1];
      const Pixel* rpb= &p->sglpred->y[y1+1][x1];
            Pixel* dp = &p->current.y [y ][p->x0];

      dp[ 0]=(dp[ 0]+((rp[ 0]+rp[ 1]+rpb[ 0]+rpb[ 1]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 1]=(dp[ 1]+((rp[ 1]+rp[ 2]+rpb[ 1]+rpb[ 2]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 2]=(dp[ 2]+((rp[ 2]+rp[ 3]+rpb[ 2]+rpb[ 3]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 3]=(dp[ 3]+((rp[ 3]+rp[ 4]+rpb[ 3]+rpb[ 4]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 4]=(dp[ 4]+((rp[ 4]+rp[ 5]+rpb[ 4]+rpb[ 5]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 5]=(dp[ 5]+((rp[ 5]+rp[ 6]+rpb[ 5]+rpb[ 6]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 6]=(dp[ 6]+((rp[ 6]+rp[ 7]+rpb[ 6]+rpb[ 7]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 7]=(dp[ 7]+((rp[ 7]+rp[ 8]+rpb[ 7]+rpb[ 8]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 8]=(dp[ 8]+((rp[ 8]+rp[ 9]+rpb[ 8]+rpb[ 9]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 9]=(dp[ 9]+((rp[ 9]+rp[10]+rpb[ 9]+rpb[10]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[10]=(dp[10]+((rp[10]+rp[11]+rpb[10]+rpb[11]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[11]=(dp[11]+((rp[11]+rp[12]+rpb[11]+rpb[12]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[12]=(dp[12]+((rp[12]+rp[13]+rpb[12]+rpb[13]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[13]=(dp[13]+((rp[13]+rp[14]+rpb[13]+rpb[14]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[14]=(dp[14]+((rp[14]+rp[15]+rpb[14]+rpb[15]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[15]=(dp[15]+((rp[15]+rp[16]+rpb[15]+rpb[16]+2)>>2)+MV_MEAN_CONST)>>1;
    }
}

static void MC16xXXadd_420_SGL_HH_Chroma(MotionCompensation_Common::PredData* p)
{
  const int y0    = p->y0 >>1;
  const int x0    = p->x0 >>1;
  const int yend  = y0 + p->blkheight_chr;
  const int x1    = x0 + ((p->sglpred->h/2)>>1);
  const int yoffs = ((p->sglpred->v/2)>>1);

  for (int y=y0;y<yend;y++)
    {
      const Pixel* rp = &p->sglpred->cb[y+yoffs  ][x1];
      const Pixel* rpb= &p->sglpred->cb[y+yoffs+1][x1];
            Pixel* dp = &p->current.cb[y][x0];

      dp[ 0]=(dp[ 0]+((rp[ 0]+rp[ 1]+rpb[ 0]+rpb[ 1]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 1]=(dp[ 1]+((rp[ 1]+rp[ 2]+rpb[ 1]+rpb[ 2]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 2]=(dp[ 2]+((rp[ 2]+rp[ 3]+rpb[ 2]+rpb[ 3]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 3]=(dp[ 3]+((rp[ 3]+rp[ 4]+rpb[ 3]+rpb[ 4]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 4]=(dp[ 4]+((rp[ 4]+rp[ 5]+rpb[ 4]+rpb[ 5]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 5]=(dp[ 5]+((rp[ 5]+rp[ 6]+rpb[ 5]+rpb[ 6]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 6]=(dp[ 6]+((rp[ 6]+rp[ 7]+rpb[ 6]+rpb[ 7]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 7]=(dp[ 7]+((rp[ 7]+rp[ 8]+rpb[ 7]+rpb[ 8]+2)>>2)+MV_MEAN_CONST)>>1;

      rp = &p->sglpred->cr[y+yoffs][x1];
      rpb= &p->sglpred->cr[y+yoffs+1][x1];
      dp = &p->current.cr[y][x0];

      dp[ 0]=(dp[ 0]+((rp[ 0]+rp[ 1]+rpb[ 0]+rpb[ 1]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 1]=(dp[ 1]+((rp[ 1]+rp[ 2]+rpb[ 1]+rpb[ 2]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 2]=(dp[ 2]+((rp[ 2]+rp[ 3]+rpb[ 2]+rpb[ 3]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 3]=(dp[ 3]+((rp[ 3]+rp[ 4]+rpb[ 3]+rpb[ 4]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 4]=(dp[ 4]+((rp[ 4]+rp[ 5]+rpb[ 4]+rpb[ 5]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 5]=(dp[ 5]+((rp[ 5]+rp[ 6]+rpb[ 5]+rpb[ 6]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 6]=(dp[ 6]+((rp[ 6]+rp[ 7]+rpb[ 6]+rpb[ 7]+2)>>2)+MV_MEAN_CONST)>>1;
      dp[ 7]=(dp[ 7]+((rp[ 7]+rp[ 8]+rpb[ 7]+rpb[ 8]+2)>>2)+MV_MEAN_CONST)>>1;
    }
}




// ------------------------- BIDIRECTIONAL PREDICTION --------------------------------------

static void MC16xXX_420_DBL_FFFF_Luma(MotionCompensation_Common::PredData* p)
{
  const int yend  = p->y0 + p->blkheight;
  const int x1    = p->x0 + (p->pred1.h>>1);
  const int x2    = p->x0 + (p->pred2.h>>1);
  const int dy1   = (p->pred1.v>>1);
  const int dy2   = (p->pred2.v>>1);

  for (int y=p->y0;y<yend;y++)
    {
      int y1 = y+dy1;
      int y2 = y+dy2;
      const Pixel* rp1 = &p->pred1.y[y1][x1];
      const Pixel* rp2 = &p->pred2.y[y2][x2];
            Pixel* dp  = &p->current.y [y ][p->x0];

      dp[ 0]=(rp1[ 0]+rp2[ 0]+MV_MEAN_CONST)>>1;
      dp[ 1]=(rp1[ 1]+rp2[ 1]+MV_MEAN_CONST)>>1;
      dp[ 2]=(rp1[ 2]+rp2[ 2]+MV_MEAN_CONST)>>1;
      dp[ 3]=(rp1[ 3]+rp2[ 3]+MV_MEAN_CONST)>>1;
      dp[ 4]=(rp1[ 4]+rp2[ 4]+MV_MEAN_CONST)>>1;
      dp[ 5]=(rp1[ 5]+rp2[ 5]+MV_MEAN_CONST)>>1;
      dp[ 6]=(rp1[ 6]+rp2[ 6]+MV_MEAN_CONST)>>1;
      dp[ 7]=(rp1[ 7]+rp2[ 7]+MV_MEAN_CONST)>>1;
      dp[ 8]=(rp1[ 8]+rp2[ 8]+MV_MEAN_CONST)>>1;
      dp[ 9]=(rp1[ 9]+rp2[ 9]+MV_MEAN_CONST)>>1;
      dp[10]=(rp1[10]+rp2[10]+MV_MEAN_CONST)>>1;
      dp[11]=(rp1[11]+rp2[11]+MV_MEAN_CONST)>>1;
      dp[12]=(rp1[12]+rp2[12]+MV_MEAN_CONST)>>1;
      dp[13]=(rp1[13]+rp2[13]+MV_MEAN_CONST)>>1;
      dp[14]=(rp1[14]+rp2[14]+MV_MEAN_CONST)>>1;
      dp[15]=(rp1[15]+rp2[15]+MV_MEAN_CONST)>>1;
    }
}


static void MC16xXX_420_DBL_FFFF_Chroma(MotionCompensation_Common::PredData* p)
{
  const int y0     = p->y0 >>1;
  const int x0     = p->x0 >>1;
  const int yend   = y0 + p->blkheight_chr;
  const int x1     = x0 + ((p->pred1.h/2)>>1);
  const int x2     = x0 + ((p->pred2.h/2)>>1);
  const int yoffs1 = ((p->pred1.v/2)>>1);
  const int yoffs2 = ((p->pred2.v/2)>>1);

  for (int y=y0;y<yend;y++)
    {
      const Pixel* rp1 = &p->pred1.cb[y+yoffs1][x1];
      const Pixel* rp2 = &p->pred2.cb[y+yoffs2][x2];
            Pixel* dp  = &p->current.cb[y][x0];

      dp[0]=(rp1[0]+rp2[0]+MV_MEAN_CONST)>>1;
      dp[1]=(rp1[1]+rp2[1]+MV_MEAN_CONST)>>1;
      dp[2]=(rp1[2]+rp2[2]+MV_MEAN_CONST)>>1;
      dp[3]=(rp1[3]+rp2[3]+MV_MEAN_CONST)>>1;
      dp[4]=(rp1[4]+rp2[4]+MV_MEAN_CONST)>>1;
      dp[5]=(rp1[5]+rp2[5]+MV_MEAN_CONST)>>1;
      dp[6]=(rp1[6]+rp2[6]+MV_MEAN_CONST)>>1;
      dp[7]=(rp1[7]+rp2[7]+MV_MEAN_CONST)>>1;

      rp1 = &p->pred1.cr[y+yoffs1][x1];
      rp2 = &p->pred2.cr[y+yoffs2][x2];
      dp  = &p->current.cr[y][x0];

      dp[0]=(rp1[0]+rp2[0]+MV_MEAN_CONST)>>1;
      dp[1]=(rp1[1]+rp2[1]+MV_MEAN_CONST)>>1;
      dp[2]=(rp1[2]+rp2[2]+MV_MEAN_CONST)>>1;
      dp[3]=(rp1[3]+rp2[3]+MV_MEAN_CONST)>>1;
      dp[4]=(rp1[4]+rp2[4]+MV_MEAN_CONST)>>1;
      dp[5]=(rp1[5]+rp2[5]+MV_MEAN_CONST)>>1;
      dp[6]=(rp1[6]+rp2[6]+MV_MEAN_CONST)>>1;
      dp[7]=(rp1[7]+rp2[7]+MV_MEAN_CONST)>>1;
    }
}


static void MC16xXX_420_DBL_xxxx_Luma(MotionCompensation_Common::PredData* p,
                                      bool h1,bool v1,bool h2,bool v2)
{
  const int yend  = p->y0 + p->blkheight;
  const int x1    = p->x0 + (p->pred1.h>>1);
  const int x2    = p->x0 + (p->pred2.h>>1);
  const int dy1   = (p->pred1.v>>1);
  const int dy2   = (p->pred2.v>>1);

  for (int y=p->y0;y<yend;y++)
    {
      int y1 = y+dy1;
      int y2 = y+dy2;
      const Pixel* rp1  = &p->pred1.y[y1  ][x1];
      const Pixel* rp1b = (v1 ? &p->pred1.y[y1+1][x1] : 0);
      const Pixel* rp2  = &p->pred2.y[y2  ][x2];
      const Pixel* rp2b = (v2 ? &p->pred2.y[y2+1][x2] : 0);
            Pixel* dp  = &p->current.y [y ][p->x0];

      for (int i=0;i<16;i++)
        {
          int val1;
          if (v1 && h1) val1=(rp1[i]+rp1[i+1]+rp1b[i]+rp1b[i+1]+2)>>2;
          else if (h1)  val1=(rp1[i]+rp1[i+1]+1)>>1;
          else if (v1)  val1=(rp1[i]+rp1b[i]+1)>>1;
          else          val1= rp1[i];

          int val2;
          if (v2 && h2) val2=(rp2[i]+rp2[i+1]+rp2b[i]+rp2b[i+1]+2)>>2;
          else if (h2)  val2=(rp2[i]+rp2[i+1]+1)>>1;
          else if (v2)  val2=(rp2[i]+rp2b[i]+1)>>1;
          else          val2= rp2[i];

          dp[ i]=(val1+val2+2)>>1;
        }
    }
}

static void MC16xXX_420_DBL_xxxx_Chroma(MotionCompensation_Common::PredData* p,
                                        bool h1,bool v1,bool h2,bool v2)
{
  const int y0     = p->y0 >>1;
  const int x0     = p->x0 >>1;
  const int yend   = y0 + p->blkheight_chr;
  const int x1     = x0 + ((p->pred1.h/2)>>1);
  const int x2     = x0 + ((p->pred2.h/2)>>1);
  const int yoffs1 = ((p->pred1.v/2)>>1);
  const int yoffs2 = ((p->pred2.v/2)>>1);

  for (int y=y0;y<yend;y++)
    {
      const Pixel* rp1 = &p->pred1.cb[y+yoffs1  ][x1];
      const Pixel* rp1b= &p->pred1.cb[y+yoffs1+1][x1];
      const Pixel* rp2 = &p->pred2.cb[y+yoffs2  ][x2];
      const Pixel* rp2b= &p->pred2.cb[y+yoffs2+1][x2];
            Pixel* dp  = &p->current.cb[y][x0];

      for (int i=0;i<8;i++)
        {
          int val1;
          if (v1 && h1) val1=(rp1[i]+rp1[i+1]+rp1b[i]+rp1b[i+1]+2)>>2;
          else if (h1)  val1=(rp1[i]+rp1[i+1]+1)>>1;
          else if (v1)  val1=(rp1[i]+rp1b[i]+1)>>1;
          else          val1= rp1[i];
      
          int val2;
          if (v2 && h2) val2=(rp2[i]+rp2[i+1]+rp2b[i]+rp2b[i+1]+2)>>2;
          else if (h2)  val2=(rp2[i]+rp2[i+1]+1)>>1;
          else if (v2)  val2=(rp2[i]+rp2b[i]+1)>>1;
          else          val2= rp2[i];

          dp[ i]=(val1+val2+2)>>1;
        }


      rp1 = &p->pred1.cr[y+yoffs1  ][x1];
      rp1b= &p->pred1.cr[y+yoffs1+1][x1];
      rp2 = &p->pred2.cr[y+yoffs2  ][x2];
      rp2b= &p->pred2.cr[y+yoffs2+1][x2];
      dp  = &p->current.cr[y][x0];

      for (int i=0;i<8;i++)
        {
          int val1;
          if (v1 && h1) val1=(rp1[i]+rp1[i+1]+rp1b[i]+rp1b[i+1]+2)>>2;
          else if (h1)  val1=(rp1[i]+rp1[i+1]+1)>>1;
          else if (v1)  val1=(rp1[i]+rp1b[i]+1)>>1;
          else          val1= rp1[i];
      
          int val2;
          if (v2 && h2) val2=(rp2[i]+rp2[i+1]+rp2b[i]+rp2b[i+1]+2)>>2;
          else if (h2)  val2=(rp2[i]+rp2[i+1]+1)>>1;
          else if (v2)  val2=(rp2[i]+rp2b[i]+1)>>1;
          else          val2= rp2[i];

          dp[ i]=(val1+val2+2)>>1;
        }
    }
}
#endif


/// #endif


#if 0
static void LumaFF(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp;
        Pixel* dp;

  sp = mc->lastimg.y;
  dp = mc->currimg.y;

  for (int y=0;y<mc->blkheight;y++)
    {
      for (int x=0;x<16;x++)
        {
          *dp++ = *sp++;
        }
      sp += mc->bytesperline_lum-16;
      dp += mc->bytesperline_lum-16;
    }
}
#endif

static void LumaHF(MotionCompensation_SglMB::MCData* mc)
{
  const Pixel* sp;
        Pixel* dp;

  sp = mc->lastimg.y;
  dp = mc->currimg.y;

  for (int y=0;y<mc->blkheight;y++)
    {
      int v1 = *sp;

      for (int x=0;x<8;x++)
        {
          sp++;
          int v2 = *sp;
          *dp++ = (v1 + v2 + 1)>>1;

          sp++;
          v1 = *sp;
          *dp++ = (v1 + v2 + 1)>>1;
        }
      sp += mc->bytesperline_lum-16;
      dp += mc->bytesperline_lum-16;
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
  const Pixel* sp;
        Pixel* dp;

  sp = mc->nextimg.y;
  dp = mc->currimg.y;

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






#if 0
static void MC_420_DBL_FFFF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFF(mc); LumaFFadd(mc); }
static void MC_420_DBL_FFFH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFF(mc); LumaFHadd(mc); }
static void MC_420_DBL_FFHF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFF(mc); LumaHFadd(mc); }
static void MC_420_DBL_FFHH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFF(mc); LumaHHadd(mc); }
static void MC_420_DBL_FHFF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFH(mc); LumaFFadd(mc); }
static void MC_420_DBL_FHFH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFH(mc); LumaFHadd(mc); }
static void MC_420_DBL_FHHF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFH(mc); LumaHFadd(mc); }
static void MC_420_DBL_FHHH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaFH(mc); LumaHHadd(mc); }
static void MC_420_DBL_HFFF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHF(mc); LumaFFadd(mc); }
static void MC_420_DBL_HFFH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHF(mc); LumaFHadd(mc); }
static void MC_420_DBL_HFHF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHF(mc); LumaHFadd(mc); }
static void MC_420_DBL_HFHH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHF(mc); LumaHHadd(mc); }
static void MC_420_DBL_HHFF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHH(mc); LumaFFadd(mc); }
static void MC_420_DBL_HHFH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHH(mc); LumaFHadd(mc); }
static void MC_420_DBL_HHHF_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHH(mc); LumaHFadd(mc); }
static void MC_420_DBL_HHHH_Luma(MotionCompensation_SglMB::MCData* mc) { LumaHH(mc); LumaHHadd(mc); }
              
static void MC_420_DBL_FFFF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFF(mc); ChromaFFadd(mc); }
static void MC_420_DBL_FFFH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFF(mc); ChromaFHadd(mc); }
static void MC_420_DBL_FFHF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFF(mc); ChromaHFadd(mc); }
static void MC_420_DBL_FFHH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFF(mc); ChromaHHadd(mc); }
static void MC_420_DBL_FHFF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFH(mc); ChromaFFadd(mc); }
static void MC_420_DBL_FHFH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFH(mc); ChromaFHadd(mc); }
static void MC_420_DBL_FHHF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFH(mc); ChromaHFadd(mc); }
static void MC_420_DBL_FHHH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaFH(mc); ChromaHHadd(mc); }
static void MC_420_DBL_HFFF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHF(mc); ChromaFFadd(mc); }
static void MC_420_DBL_HFFH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHF(mc); ChromaFHadd(mc); }
static void MC_420_DBL_HFHF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHF(mc); ChromaHFadd(mc); }
static void MC_420_DBL_HFHH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHF(mc); ChromaHHadd(mc); }
static void MC_420_DBL_HHFF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHH(mc); ChromaFFadd(mc); }
static void MC_420_DBL_HHFH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHH(mc); ChromaFHadd(mc); }
static void MC_420_DBL_HHHF_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHH(mc); ChromaHFadd(mc); }
static void MC_420_DBL_HHHH_Chroma(MotionCompensation_SglMB::MCData* mc) { ChromaHH(mc); ChromaHHadd(mc); }
#else
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
#endif




MotionCompensation_SglMB_Simple::MotionCompensation_SglMB_Simple()
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

#if 0
  for (int i=0;i<16;i++)
    dblluma[i] = MC_420_DBL_HHFF_Luma;
#endif

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

#if 0
  for (int i=0;i<16;i++)
    dblchroma420[i] = MC_420_DBL_FFFF_Chroma;
#endif
}

MotionCompensation_SglMB::MCompFunc*const*
MotionCompensation_SglMB_Simple::AskMCompFunc_Sgl_Luma() const
{
  return sglluma;
}

MotionCompensation_SglMB::MCompFunc*const*
MotionCompensation_SglMB_Simple::AskMCompFunc_Dbl_Luma() const
{
  return dblluma;
}

MotionCompensation_SglMB::MCompFunc*const*
MotionCompensation_SglMB_Simple::AskMCompFunc_Sgl_Chroma(uint2 chroma) const
{
  return sglchroma420;
}

MotionCompensation_SglMB::MCompFunc*const*
MotionCompensation_SglMB_Simple::AskMCompFunc_Dbl_Chroma(uint2 chroma) const
{
  return dblchroma420;
}

