/********************************************************************************
  vpostproc/pp_qscale.hh

  purpose:
    Show QScale of macroblocks.

  notes:

  to do:

  author(s):
   - Dirk Farin, Kapellenweg 15, 72070 Tuebingen, Germany,
     email: farindk@trick.informatik.uni-stuttgart.de

  modifications:
   15/Apr/00 - Dirk Farin
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

#ifndef DVDVIEW_VPOSTPROC_PP_QSCALE_HH
#define DVDVIEW_VPOSTPROC_PP_QSCALE_HH

#include "vpostproc/postproc.hh"


class VideoPostprocessor_QScale : public VideoPostprocessor_Accumulate
{
public:
  VideoPostprocessor_QScale() { }

  void BeginPicture(const DecodedImageData*);
  void ShowMBRows(DecodedImageData*);
  void FinishedPicture();

  bool NeedsPictureData(uint3 pictype) const { return true; }
  bool NeedsMBData(uint3 pictype) const { return true; }
private:
};

#endif
