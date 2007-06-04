/********************************************************************************
  system/system1.hh

  purpose:
    MPEG-1 system headers.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   27/Sep/1999 - Dirk Farin
     - Extracted structure declarations into this file.
     - Moved 'absolute_filepos' member variable to SysPacket base class.
   19/Dec/1998 - Dirk Farin
     - Added member 'absolute_filepos' to SysPacket_Packet.
   17/Dec/1998 - Dirk Farin
     - Each SysPacket now has a pck_nr member.
   20/Nov/1998 - Dirk Farin
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

#ifndef DVDVIEW_SYSTEM_SYSTEM1_HH
#define DVDVIEW_SYSTEM_SYSTEM1_HH

#include "types.hh"
#include "utility/bytebuffer.hh"

typedef unsigned long long PTS;
typedef unsigned long long DTS;


struct SysPacket
{
  uint64 absolute_filepos;	// File position where this system packet started.

  int pck_nr; /* NOTE: This member is considered obsolet but is not
                 removed because is may help debugging. Think twice
                 before writing code that relies on this member.

                 Consecutive packet number. More packet with the same
                 packet number may be generated in later processing
                 stages but the SystemDecoder always generated a
                 strictly increasing sequence. */

  virtual ~SysPacket() { }
};


struct SystemTimingInformation
{
  bool   HasPTS;
  PTS    pts;
  
  bool   HasDTS;
  DTS    dts;
};


struct SysPacket_Packet : public SysPacket
{
  SysPacket_Packet(ByteBufferParams& p)
    : data(p) { }

  uint8  Channel;

  SystemTimingInformation timing;

  bool   HasSTDBufferSpecs;
  uint8  STDBufferScale;
  uint16 STDBufferSize;

  ByteBuffer data; /* The actual packet data. Note that this can be larger
                      than it is allowed by the standard, as the system
                      decoder is free to create pseudo system packets that
                      do not underlie any limitations. */
};


struct SysPacket_Pack : public SysPacket
{
  long long SCR;
  uint32    MuxRate;
};


struct SysPacket_SystemHeader : public SysPacket
{
  int   RateBound;
  int   AudioBound;
  int   VideoBound;
  bool  FixedBitRate;
  bool  ConstrainedBitstream;
  bool  AudioLock;
  bool  VideoLock;

  uint32 STDBufferSizeAudio[32];
  uint32 STDBufferSizeVideo[16];
};


struct SysPacket_Iso11172EndCode : public SysPacket
{
};



class SysPacketSource
{
public:
  virtual ~SysPacketSource() { }

  virtual SysPacket* GetNextPacket() = 0;	// Get next packet or return NULL if the end of the stream is reached.
};


class PacketSource
{
public:
  virtual ~PacketSource() { }

  virtual SysPacket_Packet* GetNextPacket() = 0; // Get next packet or return NULL if the end of the stream is reached.
};

#endif
