/********************************************************************************
  output/out_yuv.hh

  purpose:
    YUV file-output.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   25/Sep/2000 - Dirk Farin
    - first implementation, based on old YUV file output code.
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

#ifndef DVDVIEW_OUTPUT_OUT_YUV_HH
#define DVDVIEW_OUTPUT_OUT_YUV_HH

#include "libvideogfx/graphics/basic/image.hh"
#include "video12/output.hh"
#include "output/accumulator.hh"
#include "libvideogfx/graphics/fileio/write_yuv.hh"
#include <fstream.h>


class VideoSink_YUVFile : public VideoOutput,
			  public ImageDataAccumulator
{
public:
   VideoSink_YUVFile();
  ~VideoSink_YUVFile();

  void BeginPicture(const DecodedImageData*);
  void ShowMBRows(DecodedImageData*);
  void FinishedPicture();

  bool PictureAvailable() { return pic_available; }
  PTS  AskPTSOfNextToBeDisplayed() const { return nextpts; }
  void ShowPicture() { pic_available=false; }

private:
  bool first;
  ofstream ostr;
  FileWriter_YUV1 writer;

  bool pic_available;
  PTS  nextpts;
};

#endif
