/********************************************************************************
  vpostproc/pp_mv.hh

  purpose:
    Show motionvectors.

  notes:

  to do:

  author(s):
   - Dirk Farin, Kapellenweg 15, 72070 Tuebingen, Germany,
     email: farindk@trick.informatik.uni-stuttgart.de

  modifications:
   20/Sep/2000 - Dirk Farin
     - Adaptation to new output architecture
   15/Apr/2000 - Dirk Farin
     - reimplementation
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

#ifndef DVDVIEW_VPOSTPROC_PP_MV_HH
#define DVDVIEW_VPOSTPROC_PP_MV_HH

#include "vpostproc/postproc.hh"


class VideoPostprocessor_MotionVector : public VideoPostprocessor_Accumulate
{
public:
  VideoPostprocessor_MotionVector() : d_show_forward(true), d_show_backward(true),
  d_colored(false), d_show_p(true), d_show_b(true) { }

  void SelectMVs(bool showforw,bool showback)
  {
    d_show_forward = showforw;
    d_show_backward = showback;
  }

  void SelectFrametypes(bool p,bool b)
  {
    d_show_p=p; d_show_b=b;
  }

  void ColoredVectors(bool flag=true) { d_colored=flag; }

  bool NeedsPictureData(uint3 pictype) const;
  bool NeedsMBData(uint3 pictype) const;

  void BeginPicture(const DecodedImageData*);
  void ShowMBRows(DecodedImageData*);
  void FinishedPicture();

private:
  bool d_show_forward;
  bool d_show_backward;
  bool d_show_p,d_show_b;
  bool d_colored;
};

#endif
