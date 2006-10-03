/********************************************************************************
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

#include "output/out_yuv.hh"


VideoSink_YUVFile::VideoSink_YUVFile()
  : first(true), pic_available(false)
{
}


VideoSink_YUVFile::~VideoSink_YUVFile()
{
  if (!first) ostr.close();
}


void VideoSink_YUVFile::ShowMBRows(DecodedImageData* dimg)
{
  nextpts = dimg->m_timing.pts;

  if (first)
    {
      ostr.open("sequence.yuv");
      writer.SetYUVStream(ostr);
      first=false;
    }

  DecodedImageData* decimg = Accumulate(dimg);
  if (!decimg)
    return;

  writer.WriteImage(decimg->m_image);
}


void VideoSink_YUVFile::BeginPicture(const DecodedImageData* dimg)
{
  StartAccumulation(0,dimg->m_height-1,false);
}

void VideoSink_YUVFile::FinishedPicture()
{
  pic_available=true;
}
