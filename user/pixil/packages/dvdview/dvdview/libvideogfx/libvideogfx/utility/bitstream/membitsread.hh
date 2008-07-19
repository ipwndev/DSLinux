/********************************************************************************
  libvideogfx/utility/bitstream/membitsread.hh

  purpose:
   Implements bit-access to an externally provided memory buffer.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   03/Jul/2000 - Dirk Farin
     - new method: SkipBitsFast()
   20/Jun/2000 - Dirk Farin
     - completely new implementation
   30/Sep/1999 - Dirk Farin
     - integrated from old DVDview into ULib
   26/Dec/1998 - Dirk Farin
     - made most methods inline
     - new method: "AskBitsLeft()"
   23/Dec/1998 - Dirk Farin
     - first implementation, not working yet
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

#ifndef LIBVIDEOGFX_UTILITY_BITSTREAM_MEMBITSREAD_HH
#define LIBVIDEOGFX_UTILITY_BITSTREAM_MEMBITSREAD_HH


class MemBitstreamReader
{
public:
  MemBitstreamReader(const uint8* buffer,uint32 len);

  inline uint32 GetBits (int nbits);
  inline uint32 PeekBits(int nbits);
  inline void   SkipBits(int nbits);
  inline void   SkipBitsFast(int nbits); /* Use ONLY when you called PeekBits() with at
					    least as many bits before! */

  inline uint32 AskBitsLeft() const; // Return number of bits that have still not been read.

  inline bool   eof() const;       // True iff current cursor position at or behind file end

private:
  uint64 d_buffer;
  uint32 d_bitsleft;

  const uint8* d_ptr;
  const uint8* d_endptr;

  void Refill();
};

#include "membitsread.icc"

#endif
