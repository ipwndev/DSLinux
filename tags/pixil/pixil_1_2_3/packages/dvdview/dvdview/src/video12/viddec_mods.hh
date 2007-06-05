/********************************************************************************
  video12/viddec_mods.hh
    Main video decoder. Abstract classes for decoding modules.

  purpose:
    Abstract base classes declarations that encapsulate computation intensive
    parts of the MPEG decoding process. So you can subclass these classes and
    create implementations that are optimized for specific architectures.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   05/Sep/2000 - Dirk Farin
     - removed Init() method
   04/Oct/1999 - Dirk Farin
     - first revision
 ********************************************************************************
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

#ifndef DVDVIEW_VIDEO12_VIDDEC_MODS_HH
#define DVDVIEW_VIDEO12_VIDDEC_MODS_HH

#include "types.hh"
#include "libvideogfx/graphics/basic/image.hh"

struct PixPtrs
{
  Pixel* y;
  Pixel* cr;
  Pixel* cb;
};
struct PixPtrs_const
{
  const Pixel* y;
  const Pixel* cr;
  const Pixel* cb;
};

// Do not change these bit positions, as they are hard-coded in parts of the video decoder.
#define MC_Last_HalfV 1
#define MC_Last_HalfH 2
#define MC_Next_HalfV 4
#define MC_Next_HalfH 8

class MotionCompensation_SglMB
{
public:
  virtual ~MotionCompensation_SglMB() { }

  struct MCData
  {
    PixPtrs_const  lastimg;
    PixPtrs        currimg;
    PixPtrs_const  nextimg;

    int     bytesperline_lum;
    int     bytesperline_chr;

    int     blkheight;
    int     blkheight_chr;

    int     LumaHalfFlags;
    int     ChromaHalfFlags;
  };

  static MotionCompensation_SglMB* Create();

  typedef void MCompFunc(MCData*);

  virtual MCompFunc*const* AskMCompFunc_Sgl_Luma()   const = 0;  // Returns array of 16 func.pointers.
  virtual MCompFunc*const* AskMCompFunc_Dbl_Luma()   const = 0;  // Returns array of 4 func.pointers.
  virtual MCompFunc*const* AskMCompFunc_Sgl_Chroma(uint2 chroma) const = 0;
  virtual MCompFunc*const* AskMCompFunc_Dbl_Chroma(uint2 chroma) const = 0;

  void CallSglLuma(MCData* d) const { (AskMCompFunc_Sgl_Luma())[d->LumaHalfFlags](d); }
  void CallDblLuma(MCData* d) const { (AskMCompFunc_Dbl_Luma())[d->LumaHalfFlags](d); }
  void CallSglChroma(uint2 chroma,MCData* d)const{(AskMCompFunc_Sgl_Chroma(chroma))[d->ChromaHalfFlags](d);}
  void CallDblChroma(uint2 chroma,MCData* d)const{(AskMCompFunc_Dbl_Chroma(chroma))[d->ChromaHalfFlags](d);}
};

#endif
