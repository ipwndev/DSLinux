/********************************************************************************
  video12/output.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
    16/Apr/2000 - Dirk Farin
     - new method: FreePictureData()
    04/Oct/1999 - Dirk Farin
     - integration into new architecture (code extracted out of
       old vsink.h)
    05/May/1999 - Dirk Farin
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

#ifndef DVDVIEW_VIDEO12_OUTPUT_HH
#define DVDVIEW_VIDEO12_OUTPUT_HH

#include "types.hh"
#include "libvideogfx/graphics/basic/image.hh"
#include "video12/vidsyntax.hh"
#include "system/system1.hh"



struct DecodedImageData
{
  DecodedImageData()
    : // m_IsDirectBuffer(false),
      m_picdata_needed(false),
      m_picdata1(NULL), m_picdata2(NULL)
    { }

  virtual ~DecodedImageData() { }

  Image_YUV<Pixel> m_image;

  int  m_width,m_height;          // Total size of frame picture.

  int  m_src_y_start,m_src_y_end; // Number of lines in m_image, filled with data.
  int  m_dst_y_start;             // Position of decoded lines in destination image.
  bool m_field_lines;             // Skip every second line if in field mode.
  bool m_may_modify;              // If image contents may be modified by postprocessors.

  bool ContainsOutputLine(int y)
  {
    if (y>=m_dst_y_start &&
	y<=m_dst_y_start+m_src_y_end-m_src_y_start)
      {
	if (!m_field_lines)
	  return true;
	if (m_field_lines && (m_dst_y_start&1)==(y&1))
	  return true;
      }

    return false;
  }

  int GetIndexForOutputLine(int y)
  {
    return y-m_dst_y_start+m_src_y_start;
  }

#if 0
  bool m_IsDirectBuffer; /* Data goes directly to the display.
			    The decoder will have to wait for the PTS before decoding.
			    In the other case, the decoder will decode as soon as
			    possible and wait for the PTS before actually sending
			    the output data.
			 */
#endif

  bool m_picdata_needed;
  
  PictureHeader m_pichdr1; //  first field or frame
  PictureHeader m_pichdr2; //  second field
  PictureData* m_picdata1; //  first field or frame
  PictureData* m_picdata2; //  second field

  SystemTimingInformation m_timing; /* When two fields are contained, the times for
				       the first one. */


  void FreePictureData()
  {
    if (m_picdata1) PictureData::FreePictureData(m_picdata1);
    if (m_picdata2) PictureData::FreePictureData(m_picdata2);
    m_picdata1 = m_picdata2 = NULL;
  }
};



class DecodedPictureSink
{
public:
  virtual ~DecodedPictureSink() { }

  void ShowAllMBRows(DecodedImageData* data)
  {
    ImageParam_YUV param;
    data->m_image.GetParam(param);

    data->m_src_y_start = 0;
    data->m_src_y_end   = param.height-1;
    data->m_dst_y_start = 0;
    data->m_field_lines = false;

    ShowMBRows(data);
  }

  virtual void BeginPicture(const DecodedImageData*) { }
  virtual void ShowMBRows(DecodedImageData*) = 0;
  virtual void FinishedPicture() { };

  virtual bool  NeedsPictureData(uint3 pictype) const { return false; }
  virtual bool  NeedsMBData     (uint3 pictype) const { return false; }
};


class VideoOutput : public DecodedPictureSink
{
public:
  virtual bool PictureAvailable() = 0;
  virtual PTS  AskPTSOfNextToBeDisplayed() const = 0;
  virtual void ShowPicture() { }
};

#endif
