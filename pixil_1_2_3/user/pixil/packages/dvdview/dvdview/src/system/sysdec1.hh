/********************************************************************************
  system/sysdec1.hh

  purpose:
    Breaks a continuous input stream (delivered by a StreamSource
    object) into system packet. In case the stream consists of an
    elementary stream only, pseudo system packets are built with
    no PTS or DTS value set.

  notes:
   - Handling of corrupted input streams:
     Whenever invalid data is read in GetNextPacket(), an
     exception is thrown. If you want to continue decoding, you
     can catch these exceptions and call Resync(). This will
     probably find a position in the stream where decoding can
     be continued. Be prepared that although you called Resync(),
     the next GetNextPacket() call may fail again. It is guaranteed
     that continuing this way will not result in an endless loop.

  to do:
   - recognize audio stream type

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   22/Oct/1999 - Dirk Farin
     - MPEG-2 PES decoding (no header fields extracted yet)
   30/Sep/1999 - Dirk Farin
     - Added ForceStreamType().
   28/Sep/1999 - Dirk Farin
     - Resynchronization can be done after a stream error.
   17/May/1999 - Dirk Farin
     - System streams that start with extra 0-bytes are interpreted
       correctly now.
   31/Jan/1999 - Dirk Farin
     - Bugfix: Padding stream packets were not read correctly.
   19/Dec/1998 - Dirk Farin
     - New method introduced but not implemented:
       void RestartDecoderAtFileposition(uint64 filepos)
   18/Dec/1998 - Dirk Farin
     - Special Stream-IDs 0xB8 and 0xB9 in SequenceHeader are now
       supported.
     - Bugfix: SysPacket_Packet field HasDTS/HasPTS/HasSTDBufferSpecs
       were not set to false when there is no corresponding information.
   17/Dec/1998 - Dirk Farin
     - SysPacket_Iso11172EndCode packets are returned when a
       corresponding start code has been read.
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

#ifndef DVDVIEW_SYSTEM_SYSDEC1_HH
#define DVDVIEW_SYSTEM_SYSDEC1_HH

#include "types.hh"
#include "utility/bytebuffer.hh"
#include "system/system1.hh"


class SystemDecoder_MPEG1 : public SysPacketSource
{
public:
   SystemDecoder_MPEG1();
  ~SystemDecoder_MPEG1();

  void SetSource(class StreamSource&); // Setting the source implies an implicit hard reset.

  void HardReset();		// Make full reset. Every internal variables are recalculated.
				// The hard reset implies an implicit soft reset.

  void SoftReset();		// Output buffers are cleared but stream specific variables are
				// not reset. A soft reset is needed if the current position in
				// the input stream is modified.

  enum StreamType { Illegal, Unknown, System, Audio, Video }; // The overall stream type.

  StreamType AskStreamType() const { return d_streamtype; }
  void ForceStreamType(StreamType t) { d_streamtype=t; } // You should call SoftReset() afterwards. Do not call
							 // HardReset() as this will try to guess the stream type again.

  SysPacket* GetNextPacket();	// Get next packet or return NULL if the end of the stream is reached.
				// May throw Excpt_Error_InvalidData or Excpt_Error_UnexpectedEndOfFile.
				// You may want to handle this by calling Resync() and trying again.

  void Resync();		// Try to resync to data stream after some parse error.

private:
  class StreamSource* d_strsrc;
  enum StreamType     d_streamtype;
  SysPacket*          d_readahead_packet;

  bool  d_synced;

  ByteBufferParams d_bytebufparams;

  SysPacket* ReadPack();
  SysPacket* ReadPacket(uint8 Channel);
  SysPacket* ReadSystemHeader();

  SysPacket* ReadElemPacket(bool AppendStartcode=false,uint8 startcode=0x00);

  static int s_next_pck_nr;
};

#endif
