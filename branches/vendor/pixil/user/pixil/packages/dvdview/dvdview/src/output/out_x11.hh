/********************************************************************************
  output/out_x11.hh

  purpose:
    X11 output.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   20/Sep/2000 - Dirk Farin
    - first implementation, based on old X11 output code.
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

#ifndef DVDVIEW_OUTPUT_OUT_X11_HH
#define DVDVIEW_OUTPUT_OUT_X11_HH

#include "libvideogfx/graphics/basic/image.hh"
#include "video12/output.hh"
#include "output/accumulator.hh"


class VideoSink_X11 : public VideoOutput,
		      public ImageDataAccumulator
{
public:
   VideoSink_X11();
  ~VideoSink_X11();

  void ShowMBRows(DecodedImageData*);
  void FinishedPicture();

  bool PictureAvailable() { return pic_available; }
  PTS  AskPTSOfNextToBeDisplayed() const { return nextpts; }
  void ShowPicture() { pic_available=false; }

  void reset();

private:
  //class ImageWindow_Autorefresh_X11* imgwin;
  class ImageWindow_X11* imgwin;    // the window itself
  class DisplayImage_X11* x11img;   // the image to be displayed
  class Image2Raw* transformation;  // the transformation for image representation convertion
  int bpl;

  bool first;
  int  d_height;

  bool nocreate;

  bool pic_available;
  PTS  nextpts;
};

#endif
