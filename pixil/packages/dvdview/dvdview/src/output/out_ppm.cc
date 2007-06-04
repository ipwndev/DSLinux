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

#include "output/out_ppm.hh"
#include "libvideogfx/graphics/fileio/rw_ppm.hh"
#include "libvideogfx/graphics/color/colorspace.hh"
#include <fstream.h>
#include <stdio.h>


VideoSink_PPMFile::VideoSink_PPMFile()
  : cnt(0), pic_available(false)
{
}


VideoSink_PPMFile::~VideoSink_PPMFile()
{
}


void VideoSink_PPMFile::ShowMBRows(DecodedImageData* dimg)
{
  nextpts = dimg->m_timing.pts;

  DecodedImageData* decimg = Accumulate(dimg);
  if (!decimg)
    return;

  char buffer[100];
  sprintf(buffer,"img%05d.ppm",cnt);
  ofstream ostr(buffer);

  Image_RGB<Pixel> rgbimg;

  ImageParam_YUV param;
  decimg->m_image.GetParam(param);

  switch (param.chroma)
    {
    case Chroma420: YUV2RGB_420(decimg->m_image,rgbimg); break;
    case Chroma422: YUV2RGB_422(decimg->m_image,rgbimg); break;
    case Chroma444: YUV2RGB_444(decimg->m_image,rgbimg); break;
    }

  WriteImage_PPM6(rgbimg,ostr);

  cnt++;
}


void VideoSink_PPMFile::BeginPicture(const DecodedImageData* dimg)
{
  StartAccumulation(0,dimg->m_height-1,false);
}

void VideoSink_PPMFile::FinishedPicture()
{
  pic_available=true;
}
