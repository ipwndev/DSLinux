/********************************************************************************
  system/sysdemux.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   16/Apr/2000 - Dirk Farin
     - added method: PacketPending()
   30/Sep/1999 - Dirk Farin
     - new class: SystemDemuxExtractor
     - added Reset() to SystemDemux
   29/Sep/1999 - Dirk Farin
     - cleanup for integration into CVS
     - removed System-packet queue
     - channel queues may be selectively enabled/disabled
   17/Dec/1998 - Dirk Farin
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

#ifndef DVDVIEW_SYSTEM_SYSDEMUX_HH
#define DVDVIEW_SYSTEM_SYSDEMUX_HH

#include "types.hh"
#include "libvideogfx/containers/queue.hh"
#include "system/system1.hh"

#define StreamIDBase_Audio 0xC0
#define StreamIDBase_Video 0xE0


class SystemDemux
{
public:
  SystemDemux(SysPacketSource&);
  ~SystemDemux();

  void Reset(); // Clear all queues.

  // To be able to get packet of a channel, you have to enable this channel
  // before decoding.
  void EnableChannel(int StreamID,bool enable=true);

  // Get next packet or return NULL if the end of the stream is reached.
  SysPacket_Packet* AskNextPacket(int StreamID);
  SysPacket_Packet* GetNextPacket(int StreamID);
  bool PacketPending(int StreamID) const; /* True iff a packet is pending in the output queue.
					     A return value of "false" does NOT mean that
					     there will not be more packets of this channel
					     in the stream. */

private:

#define c_FirstStream 0xBD
#define c_LastStream  0xEF

  SysPacketSource& d_src;
  Queue<SysPacket_Packet*> d_outputqueue[c_LastStream-c_FirstStream+1];
  bool d_enabled[c_LastStream-c_FirstStream+1];

  bool ReadAPacket(); // Return false if no more packets available.
};



class SystemDemuxExtractor : public PacketSource
{
public:
  SystemDemuxExtractor();

  void SetSource(SystemDemux* src) { d_sysdemux=src; }
  void SetChannel(int ch) { d_channel=ch; }

  SysPacket_Packet* GetNextPacket();
  bool PacketPending() const;

private:
  SystemDemux* d_sysdemux;
  int          d_channel;
};

#endif
