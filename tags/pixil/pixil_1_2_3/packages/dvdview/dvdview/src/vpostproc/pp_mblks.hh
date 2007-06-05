/********************************************************************************
  vpostproc/pp_mblks.hh
    Marks makroblock-boundaries.

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, Kapellenweg 15, 72070 Tuebingen, Germany,
     email: farindk@trick.informatik.uni-stuttgart.de

  modifications:
   20/Sep/2000 - Dirk Farin
     - Adaptation to new output architecture
   14/Apr/2000 - Dirk Farin
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

#ifndef DVDVIEW_VPOSTPROC_PP_MBLKS_HH
#define DVDVIEW_VPOSTPROC_PP_MBLKS_HH

#include "vpostproc/postproc.hh"


class VideoPostprocessor_MBBoundaries : public VideoPostprocessor_Accumulate
{
public:
  VideoPostprocessor_MBBoundaries() : d_bigmarks(false), d_maxmode(true), d_staticmode(false) { }
  ~VideoPostprocessor_MBBoundaries() { }

  void SetBigMarks(bool flag=true) { d_bigmarks=flag; }
  void SetMaximizeMode(bool flag=true) { d_maxmode=flag; d_staticmode=!flag; }
  void SetStaticMode(Pixel p=255,bool flag=true) { d_staticmode=flag; d_maxmode=!flag; d_val=p; }

  bool NeedsPictureData(uint3 pictype) const { return true; }

  void BeginPicture(const DecodedImageData*);
  void ShowMBRows(DecodedImageData*);
  void FinishedPicture();

private:
  bool  d_bigmarks;
  bool  d_maxmode;
  bool  d_staticmode;
  Pixel d_val;
};

#endif
