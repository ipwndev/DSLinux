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


#include "system/sysdec1.hh"
#include "input/streamsrc.hh"
#include "input/errors.hh"
#include "error.hh"

#include <stdio.h>

#include <iostream.h>


const int c_ElementaryPacketSize = 63*1024;

int SystemDecoder_MPEG1::s_next_pck_nr = 0;


SystemDecoder_MPEG1::SystemDecoder_MPEG1()
  : d_strsrc(NULL),
    d_streamtype(Unknown),
    d_readahead_packet(NULL),
    d_synced(false),
    d_bytebufparams(3*1024,20)
{
}


SystemDecoder_MPEG1::~SystemDecoder_MPEG1()
{
  if (d_readahead_packet)
    {
      delete d_readahead_packet;
    }
}


void SystemDecoder_MPEG1::SetSource(StreamSource& strsrc)
{
  d_strsrc=&strsrc;
  HardReset();
}


void SystemDecoder_MPEG1::HardReset()
{
  SoftReset();


  // If streamtype is now known, guess it from the next startcode.

  if (d_streamtype == Unknown)
    {
      // Read first four bytes of stream to determine type of stream.
      // As we have read some bytes we have to read a complete startcode unit then.

      uint8 buf[4];
      int BytesRead = d_strsrc->FillBuffer(buf,4);

      /* New 17/May/99: Skip extra leading 0-bytes. */
      while (buf[0]==0 && buf[1]==0 && buf[2]==0)
	{
	  buf[0]=buf[1];
	  buf[1]=buf[2];
	  buf[2]=buf[3];
	  BytesRead--;
	  BytesRead += d_strsrc->FillBuffer(&buf[3],1);

	  // Leave loop if no complete startcode is possible any more.
	  if (BytesRead != 4)
	    break;
	}

      if (BytesRead!=4 || buf[0]!=0x00 || buf[1]!=0x00 || buf[2]!=0x01)
	{
	  d_streamtype = Illegal;
	}
      else if (buf[3]==0xBA)
	{
	  d_streamtype = System;
	  try
	    {
	      d_readahead_packet = ReadPack();
	    }
	  catch (Excpt_Error_UnexpectedEndOfStream&)
	    {
	      // Stream is far too short to be useable.
	      d_streamtype = Illegal;
	    }
	}
      else
	{
	  d_streamtype = Video;
	  d_readahead_packet = ReadElemPacket(true,buf[3]);
	}
    }
}


void SystemDecoder_MPEG1::SoftReset()
{
  if (d_readahead_packet)
    {
      delete d_readahead_packet;
      d_readahead_packet=NULL;
    }

  d_synced=false;
}


SysPacket* SystemDecoder_MPEG1::GetNextPacket()
{
  // While there are previously read packets pending in the queue, return these.

  if (d_readahead_packet)
    {
      SysPacket* pck = d_readahead_packet;
      d_readahead_packet = NULL;
      return pck;
    }

  // If end of input reached return nothing (as no more packets are in the queue).

  if (!d_strsrc->MoreDataPending() &&
      !d_synced) // This test because we have read 3 bytes already.
    return NULL;

  // Remember 'd_synced' flag and clear it.

  bool sync=d_synced;
  d_synced=false;

  // There is more data to be read. Generate more SysPackets if possible.

  if (d_streamtype==System)
    {
      // Find next startcode.

      uint8 buf[4];
      int BytesRead;
      if (!sync)
	{
	  BytesRead = d_strsrc->FillBuffer(buf,4);
	}
      else
	{
	  buf[0]=buf[1]=0; buf[2]=1;
	  BytesRead = d_strsrc->FillBuffer(&buf[3],1) + 3;
	}

      // Incomplete startcode.
      if (BytesRead!=4)
        {
	  if (BytesRead==0)
	    return NULL;	// End of stream reached, no more startcodes.
	  else
	    throw Excpt_Error_UnexpectedEndOfStream();
	}

      while (!(buf[0]==0 && buf[1]==0 && buf[2]==1)) // While startcode sync not found.
        {
	  // Shift buffer forward one byte.
	  buf[0]=buf[1]; buf[1]=buf[2]; buf[2]=buf[3];
	  if (d_strsrc->FillBuffer(&buf[3],1) != 1)
	    {
	      throw Excpt_Error_UnexpectedEndOfStream();
	    }
        }


      // According to the startcode read, get complete startcode-unit.

      if (buf[3]>=0xBD && buf[3]<=0xEF)
        {
	  //printf("%p\n",buf[3]);
          return ReadPacket(buf[3]);
        }
      else
        {
          switch (buf[3])
            {
            case 0xB9:
              {
                SysPacket* syspck = new SysPacket_Iso11172EndCode;
                syspck->pck_nr = s_next_pck_nr++;
                syspck->absolute_filepos = d_strsrc->AskCurrentPosition()-4;
                return syspck;
              }
              break;
            case 0xBA: return ReadPack(); break;
            case 0xBB: return ReadSystemHeader(); break;
            default:
	      {
		char errtxt[100];
		sprintf(errtxt,"Undefined system start code read (%p).",0x100|buf[3]);
		throw Excpt_Error_InvalidData(errtxt);
		break;
	      }
            }
        }
    }
  else
    {
      return ReadElemPacket(false);
    }

  Assert(0);
}


void SystemDecoder_MPEG1::Resync()
{
  if (d_streamtype==System)
    {
      int state=0;
      for (;;)
	{
	  unsigned char c;

	  // End of stream reached -> exit.
	  if (d_strsrc->FillBuffer(&c,1) != 1)
	    return;

	  /*                      -0-
                                  | |
                                  \ /
	    [0] --0--> (1) --0--> (2) --9--> (X)
              ^         |          |
              \        /          /
               ---?---------------
	   */

	  // simple state machine
	  if (state <= 1 && c==0)
	    state++;
	  else if (state==2 && c==0)
	    {
	    }
	  else if (state==2 && c==1)
	    {
	      d_synced=true;
	      return;
	    }
	  else
	    state=0;
	}
    }
}


SysPacket* SystemDecoder_MPEG1::ReadPack()
{
  uint8 buf[10];
  int BytesRead = d_strsrc->FillBuffer(buf,8);

  if (BytesRead!=8)
    {
      throw Excpt_Error_UnexpectedEndOfStream();
    }

  SysPacket_Pack* syspck = new SysPacket_Pack;
  syspck->absolute_filepos = d_strsrc->AskCurrentPosition()-4-8;
  syspck->pck_nr = s_next_pck_nr++;
  syspck->SCR     = (((uint64)(buf[0]&0x0E))<<29) |
                    (((uint64)(buf[1]&0xFF))<<22) |
                    (((uint64)(buf[2]&0xFE))<<14) |
                    (((uint64)(buf[3]&0xFF))<< 7) |
                    (((uint64)(buf[4]&0xFE))>> 1);
  syspck->MuxRate = (((uint32)(buf[5]&0x7F))<<15) |
                    (((uint32)(buf[6]&0xFF))<< 7) |
                    (((uint32)(buf[7]&0xFE))>> 1);

  return syspck;
}


SysPacket* SystemDecoder_MPEG1::ReadPacket(uint8 Channel)
{
  SysPacket_Packet* syspck = new SysPacket_Packet(d_bytebufparams);
  syspck->absolute_filepos = d_strsrc->AskCurrentPosition()-4;
  syspck->Channel=Channel;
  syspck->timing.HasPTS=false;
  syspck->timing.HasDTS=false;
  syspck->HasSTDBufferSpecs=false;

  // Read packet_length.

  uint8 buf[10];
  if (d_strsrc->FillBuffer(buf,2)!=2)
    {
      delete syspck;
      throw Excpt_Error_UnexpectedEndOfStream();
    }

  int packet_length = (((uint32)buf[0])<<8) | (((uint32)buf[1])<<0);

  if (Channel == 0xBF)
    {
      d_strsrc->SetCurrentPosition(d_strsrc->AskCurrentPosition()+packet_length);
      return syspck;
    }

  bool MPEG2_PES=false;

  if (Channel != 0xBE)   // FIX: 99/01/31     was: 0xBF
    {
      if (d_strsrc->FillBuffer(buf,1)!=1)
	{
	  delete syspck;
	  throw Excpt_Error_UnexpectedEndOfStream();
	}

      if ((buf[0]>>6)==0x02)
	{
	  // MPEG-2 PES packet

	  MPEG2_PES=true;

	  d_strsrc->SetCurrentPosition(d_strsrc->AskCurrentPosition()+1);
	  packet_length-=2;

	  d_strsrc->FillBuffer(buf,1);
	  d_strsrc->SetCurrentPosition(d_strsrc->AskCurrentPosition()+buf[0]);
	  packet_length-=buf[0]+1;
	}
      else
	{
	  // Read byte-padding.

	  for(;;)
	    {
	      if (buf[0] != 0xFF)
		break;
	      else
		packet_length--;

	      // read another byte

	      if (d_strsrc->FillBuffer(buf,1)!=1)
		{
		  delete syspck;
		  throw Excpt_Error_UnexpectedEndOfStream();
		}
	    }
	}


      if (!MPEG2_PES)
	{
	  // Read STDBufferSpecs.

	  if ((buf[0]&0xC0) == 0x40)
	    {
	      if (d_strsrc->FillBuffer(&buf[1],1)!=1)
		{
		  delete syspck;
		  throw Excpt_Error_UnexpectedEndOfStream();
		}

	      syspck->HasSTDBufferSpecs=true;
	      syspck->STDBufferScale = ((buf[0]&0x20)!=0);
	      syspck->STDBufferSize  = (((uint32)(buf[0]&0x1F))<<8) | ((uint32)(buf[1]));
	      packet_length -= 2;

	      if (d_strsrc->FillBuffer(buf,1)!=1)
		{
		  delete syspck;
		  throw Excpt_Error_UnexpectedEndOfStream();
		}
	    }
	  else
	    syspck->HasSTDBufferSpecs=false;


	  // Read PTS/DTS

	  if ((buf[0] & 0xE0) == 0x20)
	    {
	      bool ReadDTS = ((buf[0]&0xF0)==0x30);

	      syspck->timing.HasPTS=true;
	      syspck->timing.HasDTS=ReadDTS;

	      uint32 BytesToRead = (ReadDTS ? 10 : 5);
	      if (d_strsrc->FillBuffer(&buf[1],BytesToRead-1) != BytesToRead-1)
		{
		  delete syspck;
		  throw Excpt_Error_UnexpectedEndOfStream();
		}

	      syspck->timing.pts = (((PTS)(buf[0]&0x0E))<<29) |
		(((PTS)(buf[1]&0xFF))<<22) |
		(((PTS)(buf[2]&0xFE))<<14) |
		(((PTS)(buf[3]&0xFF))<< 7) |
		(((PTS)(buf[4]&0xFE))>> 1);

	      if (ReadDTS)
		{
		  syspck->timing.dts = (((DTS)(buf[5]&0x0E))<<29) |
		    (((DTS)(buf[6]&0xFF))<<22) |
		    (((DTS)(buf[7]&0xFE))<<14) |
		    (((DTS)(buf[8]&0xFF))<< 7) |
		    (((DTS)(buf[9]&0xFE))>> 1);
		}

	      packet_length -= BytesToRead;
	    }
	  else
	    {
	      if (buf[0])
		{
		  if (buf[0] != 0x0F)
		    {
		      delete syspck;
		      throw Excpt_Error_InvalidData("Invalid data read: packet header does not contain 00001111 where it should.");
		    }

		  packet_length--;
		}
	    }
	}
    }

  // Read packet data.

  if (packet_length<=0)
    {
      delete syspck;
      throw Excpt_Error_InvalidData("Invalid data read: packet data length <= 0.");
    }

  unsigned char* mem = syspck->data.GetPtrToAppendToBuffer(packet_length);
  uint32 nBytesRead = d_strsrc->FillBuffer(mem,packet_length);
  if (nBytesRead<packet_length)
    {
      delete syspck;
      throw Excpt_Error_UnexpectedEndOfStream();

      //syspck->data.TruncateBuffer(packet_length - nBytesRead);
    }


#if 0
  // throw away padding stream
  if (syspck->Channel == 0xBE)
    {
      return NULL;
    }
#endif

  syspck->pck_nr = s_next_pck_nr++;
  return syspck;
}


SysPacket* SystemDecoder_MPEG1::ReadElemPacket(bool AppendStartcode,uint8 startcode)
{
  /* As it's a video or audio stream, cut input into arbitrary pieces and
     return as SysPacket without PTS/DTS-times. */
  
  SysPacket_Packet* syspck = new SysPacket_Packet(d_bytebufparams);
  syspck->absolute_filepos = d_strsrc->AskCurrentPosition();
  syspck->timing.HasPTS = false;
  syspck->timing.HasDTS = false;
  syspck->HasSTDBufferSpecs = false;

       if (d_streamtype==Video) syspck->Channel = 0xE0;
  else if (d_streamtype==Audio) syspck->Channel = 0xC0;
  else { cerr << "Not implemented streamtype: " << d_streamtype << endl; NotImplemented }

  if (AppendStartcode)
    {
      syspck->absolute_filepos -= 4;
      uint8 buf[4]; buf[0]=buf[1]=0; buf[2]=1; buf[3]=startcode;
      syspck->data.AppendBytes(buf,4);
    }

  unsigned char* mem = syspck->data.GetPtrToAppendToBuffer(c_ElementaryPacketSize);
  int nBytesRead = d_strsrc->FillBuffer(mem,c_ElementaryPacketSize);
  if (nBytesRead<c_ElementaryPacketSize)
    { syspck->data.TruncateBuffer(c_ElementaryPacketSize - nBytesRead); }

  syspck->pck_nr = s_next_pck_nr++;
  return syspck;
}


SysPacket* SystemDecoder_MPEG1::ReadSystemHeader()
{
  // Read packet_length.

  uint8 buf[8];
  if (d_strsrc->FillBuffer(buf,8)!=8)
    {
      throw Excpt_Error_UnexpectedEndOfStream();
    }

  SysPacket_SystemHeader* syspck = new SysPacket_SystemHeader;

  uint32 packet_length = (((uint32)buf[0])<<8) | (((uint32)buf[1])<<0);

  syspck->absolute_filepos = d_strsrc->AskCurrentPosition()-4-8;
  syspck->RateBound = (((int)(buf[2]&0x7F))<<15) |
                      (((int)(buf[3]&0xFF))<< 7) |
                      (((int)(buf[4]&0xFE))>> 1);
  syspck->AudioBound = (buf[5]&0xFC)>>2;
  syspck->VideoBound = (buf[6]&0x1F);
  syspck->FixedBitRate         = ((buf[5]&0x02) != 0);
  syspck->ConstrainedBitstream = ((buf[5]&0x01) != 0);
  syspck->AudioLock  = ((buf[6]&0x80) != 0);
  syspck->VideoLock  = ((buf[6]&0x40) != 0);

  packet_length -= 6;

  while (packet_length>0)
    {
      if (packet_length<3)
        {
          delete syspck;
          throw Excpt_Error_InvalidData("Invalid data read: Junk at end of system header detected.");
        }

      if (d_strsrc->FillBuffer(buf,3)!=3)
        {
	  delete syspck;
	  throw Excpt_Error_UnexpectedEndOfStream();
	}

      if ((buf[0]&0x80) == 0)
        break;

      int scale;
      if ((buf[1]&0x20) != 0)
        scale=128;
      else
        scale=1024;

      uint32 size = (((uint32)(buf[1]&0x1F))<<8) | ((uint32)(buf[2]));

      if (buf[0]==0xB8)
        {
          // special StreadID 0xB8: BufferSize is set for all audio channels
          for (int i=0;i<32;i++) syspck->STDBufferSizeAudio[i] = size*scale;
        }
      else if (buf[0]==0xB9)
        {
          // special StreadID 0xB9: BufferSize is set for all video channels
          for (int i=0;i<16;i++) syspck->STDBufferSizeVideo[i] = size*scale;
        }
      else if (buf[0]<0xC0 || buf[0]>0xEF)
        {
	  /*
          delete syspck;
          throw Excpt_Error_InvalidData("Invalid data read: stream ID out of range.");
	  */
        }
      else
        {
          if (buf[0]<0xE0) syspck->STDBufferSizeAudio[buf[0]-0xC0] = size*scale;
          else             syspck->STDBufferSizeVideo[buf[0]-0xE0] = size*scale;
        }

      packet_length-=3;
    }

  syspck->pck_nr = s_next_pck_nr++;
  return syspck;
}

