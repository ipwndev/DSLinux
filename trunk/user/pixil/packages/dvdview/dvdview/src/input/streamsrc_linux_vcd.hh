/********************************************************************************
  input/streamsrc_linux_vcd.hh

  purpose:
    A stream source that reads data directly from a VideoCD.

  notes:
    This code is Linux specific!

  to do:
   - This is alpha stage code!

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   ??/???/1999 - Dirk Farin - first implementation
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


#ifndef INPUT_STREAMSRC_LINUX_VCD_HH
#define INPUT_STREAMSRC_LINUX_VCD_HH

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/cdrom.h>
}

#include "input/streamsrc.hh"
#include <iostream.h>



class StreamSource_VCD : public StreamSource
{
public:
  StreamSource_VCD();
  ~StreamSource_VCD();


  // VCD specific methods

  void Init(const char* device="/dev/cdrom");
  int  AskNTracks() const;
  int  AskCurrentTrack() const;
  void SkipToTrack(int track);


  // overloaded methods

  uint32 FillBuffer(uint8* mem,uint32 maxlen);
  bool   MoreDataPending() const;
  
  bool   IsFiniteStream() const { return true; }
  uint64 AskStreamLength() const;
  
  bool   MaySeek() const { return false; } // TODO: of course seeking should be possible.
  uint64 AskCurrentPosition() const;
  uint64 SetCurrentPosition(uint64);

private:
  int d_fd;
  int d_nTracks;
  struct cdrom_tocentry d_entry;

  uint8  d_buf[2352];
  uint32 d_bufidx;
  uint32 d_buflen;
};

#endif
