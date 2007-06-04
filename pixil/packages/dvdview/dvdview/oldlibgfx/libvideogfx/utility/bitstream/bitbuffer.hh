/********************************************************************************
  $Header$

    Memory buffer that can be filled with bit-strings.
 +-------------------------------------------------------------------------------
 | $Log$
 | Revision 1.1  2006/10/03 11:25:54  dslinux_amadeus
 | adding pristine copy of pixil to HEAD so I can branch from it
 |
 | Revision 1.1.1.1  2003/09/10 19:53:45  jasonk
 |
 |
 | Revision 1.1.1.1  2003/09/10 19:36:16  jasonk
 | Initial import of dvdview 1.1.0d
 |
 | Revision 1.2  2001/02/01 17:19:09  farin
 | added WriteBool() method for convenience
 |
 | Revision 1.1  2000/12/14 18:13:46  farin
 | new BitBuffer class for bit-oriented output
 |
 ********************************************************************************
    Copyright (C) 2000 Dirk Farin (see the README for complete list of authors)

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

#ifndef LIBVIDEOGFX_UTILITY_BITSTREAM_BITBUFFER_HH
#define LIBVIDEOGFX_UTILITY_BITSTREAM_BITBUFFER_HH

#include "libvideogfx/types.hh"


class BitBuffer
{
public:
   BitBuffer();
  ~BitBuffer();

  void WriteBits(uint32 bits,int nBits);       // input has to be right aligned
  void WriteBool(bool b) { WriteBits(b ? 1 : 0 , 1); }
  void WriteBitsMasked(uint32 bits,int nBits);
  void AlignToByte0(); // Fill 0-bits until a byte boundary is reached. 0-7 bits are inserted.

  void Flush(); // Fill 0-bits to next byte boundary and make all data available at the output buffer.
  uint8* AskBuffer() const { return d_buffer; }
  int    AskBufferSize() const { return d_bufferidx; }

private:
  uint8* d_buffer;
  int    d_bufferidx;

  int    d_buffersize;

  uint32 d_tmpbuf;
  int    d_freebits;

  void TmpToBuffer();
  void EnlargeIfFull();
};

#endif
