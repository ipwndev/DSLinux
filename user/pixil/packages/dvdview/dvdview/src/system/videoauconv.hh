/********************************************************************************
  system/videoauconv.hh
    Video-AccessUnit-Converter.

  purpose:
    Objects of this class split and combine packets so that the
    returned packets contain exactly one startcode-unit of video.

  notes:

  to do:
  1) PTS insertion when undefined ( step 2 in GetNextPacket() )

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   22/Oct/1999 - Dirk Farin
     - If stream does not start with 000001, data until first startcode is
       skipped.
   27/Dec/1998 - Dirk Farin
     - Bugfix: very first start code in file was not found if it
       had more leading 0-bytes than needed.
   22/Dec/1998 - Dirk Farin
     - Bugfix: the last byte of a input packet was inserted into
       the destination packet twice.
   19/Dec/1998 - Dirk Farin
     - first implementation, PTS insertion when undefined is not
       implemented yet.
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

#ifndef DVDVIEW_SYSTEM_VIDEOAUCONV_HH
#define DVDVIEW_SYSTEM_VIDEOAUCONV_HH

#include "types.hh"
#include "utility/bytebuffer.hh"
#include "system/system1.hh"


class VideoAUConverter : public PacketSource
{
public:
  VideoAUConverter(PacketSource&);
  ~VideoAUConverter();

  // Get next packet or return NULL if the end of the stream is reached.
  SysPacket_Packet* GetNextPacket();

private:
  PacketSource& d_src;

  SysPacket_Packet* d_current;
  SysPacket_Packet* d_next;

  int    d_curr_pos;

  bool   d_HasPTS;
  PTS    d_pts;
  int    d_pts_picoffset;
  float d_framerate;

  ByteBufferParams d_bytebufparams;

  void   Reset();
};

#endif
