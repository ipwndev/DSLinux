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

#include "tests/vidaccess.hh"
#include "input/streamsrc_istr.hh"
#include "input/errors.hh"
#include "system/sysdec1.hh"
#include "system/resync.hh"
#include "system/sysdemux.hh"
#include "system/videoauconv.hh"
#include "error.hh"

#include <iostream.h>
#include <iomanip.h>


void ShowVideoAccessUnitPackets(istream& istr)
{
  StreamSource_IStream strsrc;
  strsrc.SetIStream(istr);

  ShowVideoAccessUnitPackets(strsrc);
}


extern void PrintDataHex(uint8* data,uint32 len);

void ShowVideoAccessUnitPackets(class StreamSource& strsrc)
{
  SystemDecoder_MPEG1 sysdec;
  sysdec.SetSource(strsrc);

  cout << "Stream is ";
  switch (sysdec.AskStreamType())
    {
    case SystemDecoder_MPEG1::System:  cout << "a system stream\n"; break;
    case SystemDecoder_MPEG1::Audio:   cout << "an audio stream\n"; break;
    case SystemDecoder_MPEG1::Video:   cout << "a video stream\n";  break;
    case SystemDecoder_MPEG1::Illegal: cout << "an illegal MPEG stream\n"; return; break;
    }

  cout << endl;


  SystemResyncer resync;
  resync.SetSource(&sysdec);

  SystemDemux demux(resync);
  demux.EnableChannel(StreamIDBase_Video|0);

  SystemDemuxExtractor extract;
  extract.SetSource(&demux);
  extract.SetChannel(StreamIDBase_Video|0);

  VideoAUConverter vidcon(extract);

  for (;;)
    {
      SysPacket_Packet* pck = vidcon.GetNextPacket();
      if (pck==NULL)
	return;

      cout << "-------------------------------------------------------\n";
      cout << "abs.pos: $" << hex << pck->absolute_filepos << dec << endl;
      cout << "PTS: "; if (pck->timing.HasPTS) cout << pck->timing.pts << endl; else cout << "none\n";
      cout << "DTS: "; if (pck->timing.HasDTS) cout << pck->timing.dts << endl; else cout << "none\n";
      cout << endl;

      PrintDataHex(pck->data.AskContents(),
		   pck->data.AskLength());

      delete pck;
    }
}
