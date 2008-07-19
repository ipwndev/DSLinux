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

#include "tests/system1.hh"
#include "input/streamsrc_istr.hh"
#include "input/errors.hh"
#include "system/sysdec1.hh"
#include "error.hh"

#include <iostream.h>
#include <iomanip.h>


void ShowSystemPackets(istream& istr)
{
  StreamSource_IStream strsrc;
  strsrc.SetIStream(istr);

  ShowSystemPackets(strsrc);
}


void ShowSystemPackets(StreamSource& strsrc)
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

  for (;;)
    {
      try
	{
	  SysPacket* syspck = sysdec.GetNextPacket();
	  if (syspck==NULL)
	    break;

	  cout << "pck: " << setw(5) << syspck->pck_nr
	       << " abs.pos: " << hex << setw(6) << syspck->absolute_filepos << dec;

	  SysPacket_Packet* syspck_packet = dynamic_cast<SysPacket_Packet*>(syspck);
	  if (syspck_packet)
	    {
	      cout << " Packet, stream: " << hex << ((int)syspck_packet->Channel) << dec;
	      switch (syspck_packet->Channel & 0xF0)
		{
		case 0xC0:
		case 0xD0: cout << " (audio " << ((int)syspck_packet->Channel&0x1F) << ")"; break;
		case 0xE0: cout << " (video " << ((int)syspck_packet->Channel&0x0F) << ")"; break;
		}
	      if (syspck_packet->Channel == 0xBE)
		cout << " (padding)";

	      cout << " length: " << syspck_packet->data.AskLength();
	    }

	  SysPacket_Pack* syspck_pack = dynamic_cast<SysPacket_Pack*>(syspck);
	  if (syspck_pack)
	    {
	      cout << " Pack";
	    }

	  SysPacket_SystemHeader* syspck_hdr = dynamic_cast<SysPacket_SystemHeader*>(syspck);
	  if (syspck_hdr)
	    {
	      cout << " SystemHeader";
	    }

	  if (dynamic_cast<SysPacket_Iso11172EndCode*>(syspck))
	    {
	      cout << " ISO11172 end code";
	    }

	  cout << endl;

	  delete syspck;
	}
      catch(Excpt_Error_InvalidData& e)
	{
	  MessageDisplay::Show(ErrSev_Warning,e.m_text);
	  sysdec.Resync();
	}
      catch(Excpt_Error_UnexpectedEndOfStream& e)
	{
	  MessageDisplay::Show(ErrSev_Warning,e.m_text);
	  break; // stop decoding
	}
    }
}
