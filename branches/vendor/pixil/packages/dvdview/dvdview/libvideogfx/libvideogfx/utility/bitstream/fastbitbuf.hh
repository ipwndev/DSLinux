/********************************************************************************
  libvideogfx/utility/bitstream/fastbitbuf.hh

  purpose:
   Implements very fast bit-access to an externally provided memory buffer.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   27/Sep/2000 - Dirk Farin
     - first implementation, based on ideas seen in "mpeg2dec"
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

#ifndef LIBVIDEOGFX_UTILITY_BITSTREAM_FASTBITBUF_HH
#define LIBVIDEOGFX_UTILITY_BITSTREAM_FASTBITBUF_HH

#include "libvideogfx/types.hh"

#include <iostream.h>

void ShowBits(uint32 bits,int nbits)
{
  for (int i=nbits-1;i>=0;i--)
    {
      if ((1<<i) & bits) cout << '1'; else cout << '0';
    }
}



#define NEEDBITS(bit_buf,bits) bs.LocalFill16Bits(bit_buf,bits)

// remove num valid bits from bit_buf
#define DUMPBITS(bit_buf,bits,num)      \
do {                                    \
    bit_buf <<= (num);                  \
    bits += (num);                      \
} while (0)

// take num bits from the high part of bit_buf and zero extend them
#define UBITS(bit_buf,num) (((uint32)(bit_buf)) >> (32 - (num)))

// take num bits from the high part of bit_buf and sign extend them
#define SBITS(bit_buf,num) (((int32)(bit_buf)) >> (32 - (num)))





class FastBitBuf
{
public:
  FastBitBuf(const uint8* buffer,uint32 len)
    : d_ptr(buffer), d_endptr(buffer+len),
      d_buffer(0), d_freebits(16)
  {
    Fill16Bits();
  }

  inline void MakeLocalCopy(uint32& buf,int& bits) { buf=d_buffer; bits=d_freebits; }
  inline void RestoreFromLocal(uint32 buf,int bits) { d_buffer=buf; d_freebits=bits; }
  inline void LocalFill16Bits(uint32& buf,int& bits)
  {
    if (bits >= 0)
      {
	buf |= ((((int)d_ptr[0])<<8)|d_ptr[1])<<bits;
	d_ptr+=2;
	bits-=16;
      }
  }

  inline uint32 GetNextWord()
  {
    uint32 val = (((int)d_ptr[0])<<8) | d_ptr[1];
    d_ptr+=2;
    return val;
  }

  inline uint32 Peek16BitsMSB()
  {
    return d_buffer;
  }

  inline uint32 PeekBitsFast(int nbits)
  {
    return UBITS(d_buffer,nbits);
  }

  inline void   SkipBitsFast(int nbits)
  {
    Assert(nbits<=16);
    Assert(16-d_freebits >= nbits);

    DUMPBITS(d_buffer,d_freebits,nbits);
  }

  inline void   Fill16Bits()
  {
    if (d_freebits >= 0)
      {
	d_buffer |= ((((int)d_ptr[0])<<8)|d_ptr[1])<<d_freebits;
	d_ptr+=2;
	d_freebits-=16;
      }
  }


  // ---------------- Slow methods for compatibility and non-time critical parts ---------------

  inline uint32 PeekBits(int nbits)
  {
    if(nbits>16)
      {
	uint32 copy_buffer = d_buffer;
	int copy_freebits = d_freebits;
	const uint8 * copy_ptr = d_ptr;

	uint32 bits = GetBits(16)<<(nbits-16);

	bits |= PeekBits(nbits-16);

	d_buffer=copy_buffer;
	d_freebits = copy_freebits;
	d_ptr = copy_ptr;

	return bits;
      }

    Fill16Bits();
    uint32 bits = d_buffer>>(32-nbits);
    return bits;
  }

  inline uint32 GetBits(int nbits)
  {
    if (nbits>16)
      {
	uint32 bits = GetBits(16)<<(nbits-16);
	bits |= GetBits(nbits-16);
	return bits;
      }

    uint32 bits = PeekBits(nbits);
    SkipBits(nbits);
    return bits;
  }

  inline void   SkipBits(int nbits)
  {
    if (nbits>16)
      {
	SkipBits(16);
	SkipBits(nbits-16);
	return;
      }

    Fill16Bits();

    d_buffer<<=nbits;
    d_freebits += nbits;
  }
  
  inline uint32 AskBitsLeft() const // Return number of bits that have still not been read.
  {
    return ((d_endptr-d_ptr)<<3) + 16-d_freebits;
  }

  inline bool   eof() const       // True iff current cursor position at or behind file end
  {
    return d_ptr >= d_endptr;
  }

public:
  uint32 d_buffer;
  int    d_freebits; // number of invalid bits in top 16 bits of buffer

private:
  const uint8* d_ptr;
  const uint8* d_endptr;
};

#endif
