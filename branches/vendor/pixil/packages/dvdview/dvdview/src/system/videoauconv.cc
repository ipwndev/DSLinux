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


/* TODO:
   - Warum werden vom d_next-Buffer 3 Bytes an d_curr angehaengt? Das dritte Byte
     wird doch niemals gelesen?
   - Was ist, wenn in d_next keine 3 Bytes drin sind, die an d_curr angehaengt werden
     koennten?
   - Einige Asserts() koennen bei fehlerhaften Eingabestreams ausloesen. In diesem
     Fall sollten Exceptions ausgeloest werden.
*/


#include "system/videoauconv.hh"
#include "error.hh"


VideoAUConverter::VideoAUConverter(PacketSource& src)
  : d_src(src),
    d_current(NULL),
    d_next(NULL),
    d_bytebufparams(3*1024,20)
{
  Reset();
}


VideoAUConverter::~VideoAUConverter()
{
  if (d_current) delete d_current;
  if (d_next)    delete d_next;
}


void VideoAUConverter::Reset()
{
  if (d_current) delete d_current;
  if (d_next)    delete d_next;
  d_current=NULL;
  d_next   =NULL;
  d_curr_pos=0;
}


#if 1
#include <iostream.h>
#include <iomanip.h>
void PrintDataHex(uint8* data,uint32 len)
{
  //cout << "POS: $" << hex << pck->absolute_filepos << dec << endl;

  for (uint32 i=0;i<len;i++)
    {
      if (i%16 == 0)
        cout << endl << hex << setw(8) << setfill('0') << i << ": ";
      
      cout << hex << setw(2) << setfill('0') << ((int)data[i]) << " ";
    }
  cout << dec << endl;
}

void PrintDataBin(uint8* data,uint32 len)
{
  //cout << "POS: $" << hex << pck->absolute_filepos << dec << endl;

  for (uint32 i=0;i<len;i++)
    {
      if (i%8 == 0)
        cout << endl << hex << setw(8) << setfill('0') << i << ": ";

      uint8 mask=0x80;
      for (int k=0;k<8;k++)
	cout << ((data[i]&(mask>>k)) ? '1' : '0');
      cout << ' ';
    }
  cout << dec << endl;
}
#endif


SysPacket_Packet* VideoAUConverter::GetNextPacket()
{
  // ****************** STEP 1: cut packet into new portions ********************/

  if (d_current==NULL)
    {
      d_current = d_src.GetNextPacket();
      if (d_current==NULL)
        return NULL;

      //cout << "VID 1\n";
      //PrintDataHex(d_current->data.AskContents(),d_current->data.AskLength());

      Assert(d_next==NULL);

      d_next = d_src.GetNextPacket();
      if (d_next)
        {
          //cout << "VID 2\n";
          //PrintDataHex(d_next->data.AskContents(),d_next->data.AskLength());

          // append first two bytes of second buffer to the end of the first one
          d_current->data.AppendBytes(d_next->data.AskContents(),3);
        }
    }


  uint8* currentbuf = d_current->data.AskContents();

  // TODO: throw exception instead (corrupted stread)
  //Assert(currentbuf[d_curr_pos]==0 && currentbuf[d_curr_pos+1]==0);

  SysPacket_Packet* newpck = new SysPacket_Packet(d_bytebufparams);
  newpck->pck_nr  = d_current->pck_nr;
  newpck->Channel = d_current->Channel;
  newpck->absolute_filepos = d_current->absolute_filepos;

  /* These fields are copied, but may be changed later. They are copied to
     store the values that were in effect when an access unit began. */

  newpck->timing = d_current->timing;


  // TODO: do we need these three lines?
  newpck->HasSTDBufferSpecs = d_current->HasSTDBufferSpecs;
  newpck->STDBufferScale    = d_current->STDBufferScale;
  newpck->STDBufferSize     = d_current->STDBufferSize;


  // Copy data until next startcode to new buffer.

  int state=0;
  uint32 pos=d_curr_pos+1;
  uint32 lastsearchpos = d_current->data.AskLength();
  if (d_next) lastsearchpos--;

  for (;;)
    {
      if (pos == lastsearchpos)
        {
          /* If end of current packet reached, copy rest of unused data in current-packet
             to new-packet and move next-packet into current-packet. The next-packet will
             be filled with a new packet if there is one. */
          if (d_next)
            {
              /* Copy from current position until end (excluding the last 2 extra bytes)
		 into the new buffer. */
              uint8* copystart = &currentbuf[d_curr_pos];
              uint32 copylen = d_current->data.AskLength()-1-d_curr_pos-2;
              /* Bugfix 22.Dec.98: This was the old (wrong?) version:
                 uint32 copylen = d_current->data.AskLength()-d_curr_pos-2;
              */
              newpck->data.AppendBytes(copystart,copylen);
              
              // Read in new packet.
              
              delete d_current;
              d_current=d_next;
              Assert(d_current);
              currentbuf = d_current->data.AskContents();
              d_next = d_src.GetNextPacket();
              if (d_next)
                { d_current->data.AppendBytes(d_next->data.AskContents(),3); }
              //cout << "VID 3\n";
              //PrintDataHex(d_next->data.AskContents(),d_next->data.AskLength());
              
              // Reset state machine and pointers.
              
              state=0;
              d_curr_pos=0;
              pos = 0;
              lastsearchpos = d_current->data.AskLength();
              if (d_next) lastsearchpos--;
            }
          else
            {
              // We are at the very end.

              Assert(d_next==NULL);

              uint8* copystart = &currentbuf[d_curr_pos];
              uint32 copylen = d_current->data.AskLength()-d_curr_pos;
              newpck->data.AppendBytes(copystart,copylen);

              delete d_current;
              d_current=NULL;
              goto step2;
            }
        }


      // State machine to find next occurrence of a startcode.

      uint8 c = currentbuf[pos];
      switch (state)
        {
        case 0:
        case 1:
          if (c==0) state++;
          else state=0;
          break;
        case 2: // before the 0x01
          if (c==1)
            {
	      goto leave_statemachine;
            }
          else if (c==0) { /*state=2 as it was before*/ }
          else state=0;
          break;
        }

      pos++;
    }

leave_statemachine:
  pos -= 2;

  newpck->data.AppendBytes( &currentbuf[d_curr_pos] , pos-d_curr_pos );
  d_curr_pos = pos;


step2:
  // ****************** STEP 2: adjust PTS values ********************


  // ****************** Throw away completely NULL-packet *****************
  // (This may be the case for the very first packet in the stream).

  /*
  Assert(newpck->data.AskContents()[0]==0);
  Assert(newpck->data.AskContents()[1]==0);
  */
  if (newpck->data.AskContents()[0] !=0 || newpck->data.AskContents()[1] != 0)
    {
      delete newpck;
      return GetNextPacket();
    }


  if (newpck->data.AskContents()[2]==0)
    {
      // check for all NULL-packet

      for (int i=0;i<newpck->data.AskLength();i++)
        { Assert(newpck->data.AskContents()[i]==0); }

      delete newpck;
      return GetNextPacket();
    }

#if 0
  if (newpck->data.AskContents()[0]==0 &&
      newpck->data.AskContents()[1]==0 &&
      newpck->data.AskContents()[2]==1 &&
      newpck->data.AskContents()[3]==0)
    {
      cout << "Picture Header: ";
      cout << "tRef: " << ((newpck->data.AskContents()[4] <<2) |
			   (newpck->data.AskContents()[5]>>6)) << " ";
      cout << "cType: " << ((newpck->data.AskContents()[5]>>3)&0x7) << " ";
      cout << "PTS: " << newpck->timing.pts << endl;
    }
#endif

  return newpck;
}

