/********************************************************************************
  output/out_mgavid.hh

  purpose:
    MGA_VID (Matrox G200/G400 BES output).

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   27/Jun/2000 - Christophe Labouisse <labouiss@cybercable.fr>
    - G200 support
   13/Jun/2000 - Dirk Farin
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

#ifndef DVDVIEW_OUTPUT_OUT_MGAVID_HH
#define DVDVIEW_OUTPUT_OUT_MGAVID_HH

#include "libvideogfx/graphics/basic/image.hh"
#include "video12/output.hh"
#include "vpostproc/pp_resize.hh"
#include "thirdparty/mga_vid.h"


class VideoSink_MGA : public VideoOutput,
		      public Resize_Interface
{
public:
   VideoSink_MGA();
  ~VideoSink_MGA();

  bool MGA_Available() const { return fileh != -1; }
  
  void SetScaledSize(int w,int h) { width=w; height=h; scaled=true; }
  void ResetScaledSize() { scaled=false; }

  void ShowMBRows(DecodedImageData*);
  void FinishedPicture() { pic_available=true; }

  bool PictureAvailable() { return pic_available; }
  PTS  AskPTSOfNextToBeDisplayed() const { return nextpts; }
  void ShowPicture() { pic_available=false; }

private:
  int fileh;
  bool first;
  mga_vid_config_t config;
  uint_8* base;

  bool scaled;
  int width,height;

  bool pic_available;
  PTS  nextpts;
};

#endif
