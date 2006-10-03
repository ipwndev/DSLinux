/********************************************************************************
  vpostproc/pp_resize.hh
    Resizes output image.

  purpose:

  notes:
    This is a hack, and I'm not sure if the postprocessor chain is the
    right place to do resizing, as the MB-data does not correspond to
    the image afterwards. Moreover, we have all the problems that result
    from resizing interlaced pictures.

  to do:

  author(s):
   - Dirk Farin, Kapellenweg 15, 72070 Tuebingen, Germany,
     email: farindk@trick.informatik.uni-stuttgart.de

  modifications:
   22/Sep/2000 - Dirk Farin
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

#ifndef DVDVIEW_VPOSTPROC_PP_RESIZE_HH
#define DVDVIEW_VPOSTPROC_PP_RESIZE_HH

#include "vpostproc/postproc.hh"


class Resize_Interface
{
public:
  virtual ~Resize_Interface() { }

  virtual void SetScaledSize(int w,int h) = 0;
  virtual void ResetScaledSize() = 0;
};

class VideoPostprocessor_Resize : public VideoPostprocessor_Accumulate,
				  public Resize_Interface
{
public:
  VideoPostprocessor_Resize() : d_do_resize(false),
				d_ltab_h(NULL), d_ltab_v(NULL),
				d_ctab_h(NULL), d_ctab_v(NULL), tab_initialized(false) { }
  ~VideoPostprocessor_Resize();

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
