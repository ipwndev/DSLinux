/********************************************************************************
  video12/modules/mcomp_sgl_mmx.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   22/Nov/1999 - Dirk Farin
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

#ifndef DVDVIEW_VIDEO12_MODULES_MCOMP_SGL_MMX_HH
#define DVDVIEW_VIDEO12_MODULES_MCOMP_SGL_MMX_HH

#include "error.hh"
#include "video12/viddec_mods.hh"


class MotionCompensation_SglMB_MMX : public MotionCompensation_SglMB
{
public:
  MotionCompensation_SglMB_MMX();

  void Init();

  void DoMotionCompensationSglMB_SGL_H8 (MotionCompensation_SglMB::MCData*) { Assert(0); }
  void DoMotionCompensationSglMB_SGL_H16(MotionCompensation_SglMB::MCData*);
  void DoMotionCompensationSglMB_DBL_H8 (MotionCompensation_SglMB::MCData*) { Assert(0); }
  void DoMotionCompensationSglMB_DBL_H16(MotionCompensation_SglMB::MCData*) { Assert(0); }

  MCompFunc*const* AskMCompFunc_Sgl_Luma() const;  // Returns array of 16 func.pointers.
  MCompFunc*const* AskMCompFunc_Dbl_Luma() const;  // Returns array of 4 func.pointers.
  MCompFunc*const* AskMCompFunc_Sgl_Chroma(uint2 chroma) const;
  MCompFunc*const* AskMCompFunc_Dbl_Chroma(uint2 chroma) const;

private:
  MCompFunc* sglluma[4];
  MCompFunc* dblluma[16];
  MCompFunc* sglchroma420[4];
  MCompFunc* dblchroma420[16];
  MCompFunc* sglchroma422[4];
  MCompFunc* dblchroma422[16];
  MCompFunc* sglchroma444[4];
  MCompFunc* dblchroma444[16];
};

#endif
