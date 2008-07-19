/********************************************************************************
  system/ptsvideo.hh
    Adds PTS values to picture headers without one.

  purpose:
    Make smooth synchronization possible.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   03/Jul/2000 - Dirk Farin
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

#ifndef DVDVIEW_SYSTEM_PTSVIDEO_HH
#define DVDVIEW_SYSTEM_PTSVIDEO_HH

#include "types.hh"
#include "system/system1.hh"


class PTSVideoCompleter : public PacketSource
{
public:
  PTSVideoCompleter(PacketSource&);
  ~PTSVideoCompleter();

  void Reset() { d_synchronized=false; }

  // Get next packet or return NULL if the end of the stream is reached.
  SysPacket_Packet* GetNextPacket();

private:
  PacketSource& d_src;

  bool   d_synchronized;
  PTS    d_lastPTS;
  int    d_lastIdx;
  int    d_fps;           // number of PTS-ticks per frame

  int    d_GOPlen;
};

#endif
