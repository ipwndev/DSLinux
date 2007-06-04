/********************************************************************************
  vpostproc/pp_filteredresize.hh
    Resizes output image with additional filtering to enhance quality.

  purpose:

  notes:

  to do:

  author(s):
   - Carlo Daffara, cdaffara@mail.conecta.it
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   16/Oct/2000 - Carlo Daffara
     - first implementation
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

#ifndef DVDVIEW_VPOSTPROC_PP_FILTEREDRESIZE_HH
#define DVDVIEW_VPOSTPROC_PP_FILTEREDRESIZE_HH

#include "vpostproc/postproc.hh"
#include "vpostproc/pp_resize.hh"


class VideoPostprocessor_FilteredResize : public VideoPostprocessor_Accumulate,
					  public Resize_Interface
{
public:
  VideoPostprocessor_FilteredResize() : d_do_resize(false),
					d_ltab_h(NULL), d_ltab_v(NULL),
					d_ctab_h(NULL), d_ctab_v(NULL), tab_initialized(false) { }
  ~VideoPostprocessor_FilteredResize();

  void SetScaledSize(int w,int h);
  void ResetScaledSize() { d_do_resize=false; }

  void BeginPicture(const DecodedImageData*);
  void ShowMBRows(DecodedImageData*);
  void FinishedPicture();

private:
  bool  d_do_resize;
  int   width,height;

  DecodedImageData newdimg;

  int *d_ltab_h,*d_ltab_v;
  int *d_ctab_h,*d_ctab_v;
  bool tab_initialized;
};

#endif
