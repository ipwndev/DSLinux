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


#include "system/ptsvideo.hh"
#include "error.hh"

// #include <iostream.h>


static int framerate2pts[] =
{
  0,
  3754,  // 23.976 fps
  3750,  // 24     fps
  3600,  // 25     fps
  3003,  // 29.97  fps
  3000,  // 30     fps
  1800,  // 50     fps
  1502,  // 59.94  fps
  1500   // 60     fps
};


PTSVideoCompleter::PTSVideoCompleter(PacketSource& src)
  : d_src(src),
    d_lastPTS(50000),
    d_fps(0),
    d_lastIdx(-9999),  // invalid val
    d_GOPlen(-9999)    // invalid val
{
  Reset();
}


PTSVideoCompleter::~PTSVideoCompleter()
{
}

SysPacket_Packet* PTSVideoCompleter::GetNextPacket()
{
  SysPacket_Packet* pck = d_src.GetNextPacket();

  if (pck==NULL)
    return NULL;

  uint8* currentbuf = pck->data.AskContents();

  Assert(currentbuf[0]==0x00);
  Assert(currentbuf[1]==0x00);
  Assert(currentbuf[2]==0x01);

  if (pck->timing.HasPTS)
    {
      d_lastPTS = pck->timing.pts;
      d_synchronized=true;
    }

  if (currentbuf[3]!=0x00)
    {
      // Extract fps-value out of sequence header.

      if (currentbuf[3]==0xB3)
	{
	  d_fps = framerate2pts[currentbuf[7] & 0xF];
	}

      // If GOP-start, move temporal references

      if (currentbuf[3]==0xB8)
	{
	  d_lastIdx = (d_lastIdx - d_GOPlen) -1;
	  d_GOPlen=0;
	}
  
      // Don't modify anything. PTS is only relevant in Picture-Header packets
    }
  else
    {
      int thisIdx = (((int)currentbuf[4])<<2) | (currentbuf[5]>>6);

      // If picture header already has a PTS, keep it.

      if (!pck->timing.HasPTS)
	{
	  pck->timing.HasPTS=true;
	  pck->timing.pts = d_lastPTS + (thisIdx-d_lastIdx)*d_fps;
	}
      else
	{
	  d_lastIdx = thisIdx;
	}

      // Watch GOP length.

      if (thisIdx > d_GOPlen)
	{
	  d_GOPlen = thisIdx;
	}
    }

  return pck;
}

