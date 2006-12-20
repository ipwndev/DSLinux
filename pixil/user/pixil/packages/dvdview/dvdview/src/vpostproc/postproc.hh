/********************************************************************************
  vpostproc/postproc.hh
    Video frame post processor.

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, Kapellenweg 15, 72070 Tuebingen, Germany,
     email: farindk@trick.informatik.uni-stuttgart.de

  modifications:
   19/Sep/2000 - Dirk Farin
     - Accumulate-Postprocessor helper class to ease postprocessor writing.
   07/Sep/2000 - Dirk Farin
     - Complete reorganisation because of output architecture change.
   19/Jan/1999 - Dirk Farin
     - interface definition
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

#ifndef DVDVIEW_VPOSTPROC_POSTPROC_HH
#define DVDVIEW_VPOSTPROC_POSTPROC_HH

#include "video12/output.hh"
#include "output/accumulator.hh"
#include "libvideogfx/containers/array.hh"


class VideoPostprocessor : public DecodedPictureSink
{
public:
  VideoPostprocessor();
  virtual ~VideoPostprocessor();

  virtual void SetNext(DecodedPictureSink* next)
    {
      d_next = next;
      Assert(d_next);
    }

  virtual void BeginPicture(const DecodedImageData* dimg) { d_next->BeginPicture(dimg); }
  virtual void ShowMBRows(DecodedImageData*) = 0;
  virtual void FinishedPicture() { d_next->FinishedPicture(); }

  virtual bool  NeedsPictureData(uint3 pictype) const { return d_next->NeedsPictureData(pictype); }
  virtual bool  NeedsMBData     (uint3 pictype) const { return d_next->NeedsMBData(pictype); }

protected:
  class DecodedPictureSink* d_next;
};


class VideoPostprocessor_Accumulate : public VideoPostprocessor,
				      public ImageDataAccumulator
{
public:
  void SetNext(DecodedPictureSink* next)
  {
    VideoPostprocessor::SetNext(next);
    ImageDataAccumulator::SetNext(next);
  }
};


#if 0
  /* Forward all lines that may not be modified and are not in the given range to
     the next postprocessor and exclude these lines from the line range in the
     image data.
     True is returned, if the input image data contains at least one line contained
     in the line range.

     The usage is as follows: If you know that you will operate on the lines in
     the given range, call this method before. Image parts that are not operated
     on will be forwarded to the next postprocessor. The image data range is
     modified to only include those lines that you need to draw to and that are
     present in the image data.
  */
  bool DisplayUnmodifyableLinesOutOfRange(DecodedImageData*,int first,int last);
#endif

#if 0
class VideoPostprocessor_Copy : public VideoPostprocessor
{
public:
  VideoPostprocessor_Copy() : d_width(0), d_height(0) { }

protected:
  /* Prepare image data for overwriting. If the given image contents may not be
     modified, the part that will be written to is copied. All other parts
     are implicitly forwarded to the next postprocessor stage. If false is returned,
     the input image does not contain any lines that will be drawn to. In this
     case, all input lines are implicitly forwarded to the next stage. */
  bool GetLinesForDrawing(DecodedImageData*,int first,int last);

  DecodedImageData d_dimg;

private:
  int d_width,d_height;
};
#endif


#endif
