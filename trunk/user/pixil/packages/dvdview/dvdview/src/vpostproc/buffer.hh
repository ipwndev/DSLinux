/********************************************************************************
  vpostproc/buffer.hh

  purpose:
    Converts the PUSH-semantic of the decoder pipeline to a PULL-interface.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
    12/Apr/2000 - Dirk Farin
     - extracted source from video decoder
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

#ifndef DVDVIEW_VPOSTPROC_BUFFER_HH
#define DVDVIEW_VPOSTPROC_BUFFER_HH

#include "libvideogfx/graphics/basic/image.hh"
#include "video12/output.hh"


class VideoImageBuffer : public DecodedPictureSink
{
public:
   VideoImageBuffer(int MaxPoolSize=4);
  ~VideoImageBuffer();

  bool              BufferEmpty() const { return d_queuelength==0; }
  bool              BufferFull() const;
  DecodedImageData* GetNextImage();
  void              FreeImage(DecodedImageData*);


  virtual void ShowMBRows(DecodedImageData*);

private:
  DecodedImageData** d_queue;
  int d_queuelength;
  int d_maxqueuelength;

  DecodedImageData** d_bufferpool;
  uint8* d_flags;
  int d_maxpoolsize;
};

#endif
