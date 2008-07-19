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

#include "system/sysdemux.hh"
#include "error.hh"


SystemDemux::SystemDemux(class SysPacketSource& src)
  : d_src(src)
{
  for (int i=c_FirstStream;i<=c_LastStream;i++)
    {
      d_enabled[i-c_FirstStream]=false;
    }
}


SystemDemux::~SystemDemux()
{
  Reset();
}


void SystemDemux::Reset()
{
  // Delete all packets that remained in the output queues.

  for (int i=c_FirstStream;i<=c_LastStream;i++)
    {
      while (!d_outputqueue[i-c_FirstStream].IsEmpty())
        {
          delete d_outputqueue[i-c_FirstStream].AskAndRemoveHead();
        }
    }
}


void SystemDemux::EnableChannel(int StreamID,bool enable)
{
  d_enabled[StreamID-c_FirstStream] = enable;

  if (!enable)
    {
      // Disabling channel. Delete all packet in its queue.

      while (!d_outputqueue[StreamID-c_FirstStream].IsEmpty())
        {
          delete d_outputqueue[StreamID-c_FirstStream].AskAndRemoveHead();
        }
    }
}


bool SystemDemux::ReadAPacket()
{
  SysPacket* syspck = d_src.GetNextPacket();
  if (syspck==NULL)
    return false;

  SysPacket_Packet* packet;
  if ((packet=dynamic_cast<SysPacket_Packet*>(syspck)) != NULL)
    {
      int queue = packet->Channel;

      if (queue<c_FirstStream || queue>c_LastStream)
        {
          //Error(ErrSev_Warning,"Packet found whose packet id is out of bounds. Ignoring it.");
          delete packet;
        }
      else
        {
	  if (d_enabled[queue-c_FirstStream])
	    { d_outputqueue[queue-c_FirstStream].Append(packet); }
	  else
	    { delete packet; }
        }
    }
  else
    {
      // Delete system packets.
      delete syspck;
    }

  return true;
}


SysPacket_Packet* SystemDemux::AskNextPacket(int streamid)
{
  int queueidx = streamid - c_FirstStream;

  Assert(d_enabled[queueidx]);


  // Read more packets until a packet of the requested type is received.

  while (d_outputqueue[queueidx].IsEmpty())
    {
      if (!ReadAPacket())
        return NULL;
    }

  return d_outputqueue[queueidx].AskHead();
}


SysPacket_Packet* SystemDemux::GetNextPacket(int streamid)
{
  SysPacket_Packet* pck = AskNextPacket(streamid);
  if (pck) { d_outputqueue[streamid - c_FirstStream].RemoveHead(); }
  return pck;
}

bool SystemDemux::PacketPending(int streamid) const
{
  int queueidx = streamid - c_FirstStream;

  Assert(d_enabled[queueidx]);

  return !(d_outputqueue[queueidx].IsEmpty());
}





SystemDemuxExtractor::SystemDemuxExtractor()
  : d_sysdemux(NULL),
    d_channel(-1)
{
}


SysPacket_Packet* SystemDemuxExtractor::GetNextPacket()
{
  Assert(d_sysdemux != NULL);
  Assert(d_channel>=0);

  return d_sysdemux->GetNextPacket(d_channel);
}

bool SystemDemuxExtractor::PacketPending() const
{
  Assert(d_sysdemux != NULL);
  Assert(d_channel>=0);

  return d_sysdemux->PacketPending(d_channel);
}



template class Queue<SysPacket_Packet*>;

#include "libvideogfx/containers/queue.cc"
