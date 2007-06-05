
#include "libvideogfx/utility/bitstream/bitbuffer.hh"
#include <assert.h>

const int InitialBufferSize = 1000;  // has to be an integer multiple of d_tmpbuf size.
const int BitsPerEntry = 32;
const int BytesPerEntry = BitsPerEntry/8;

BitBuffer::BitBuffer()
{
  d_buffer = new uint8[InitialBufferSize];
  d_buffersize = InitialBufferSize;
  d_bufferidx  = 0;
  d_freebits   = BitsPerEntry;
  d_tmpbuf     = 0;

  assert(BitsPerEntry%8 == 0);
  assert(sizeof(d_tmpbuf)*8==BitsPerEntry);
  assert(InitialBufferSize%sizeof(d_tmpbuf)==0);
}


BitBuffer::~BitBuffer()
{
  delete[] d_buffer;
}


void BitBuffer::WriteBitsMasked(uint32 bits,int nBits)
{
  uint32 mask=1;
  mask<<=nBits;
  mask--;

  WriteBits(bits & mask,nBits);
}


void BitBuffer::WriteBits(uint32 bits,int nBits)
{
  /*
    +------+----------------------+
    |XXXXXX|0000000000000000000000|
    +------+----------------------+
                d_freebits

    +----+------------------------+
    |0000|XXXXXXXXXXXXXXXXXXXXXXXX|
    +----+------------------------+
                   nBits         
   */

  if (d_freebits>=nBits)
    {
      // Neue Bits passen komplett in das aktuelle Feld hinein.

      uint32 aligned = bits;

      int nShiftLeft = d_freebits - nBits;

      if (nShiftLeft>0)
	{
	  aligned <<= nShiftLeft;
	  d_tmpbuf |= aligned;
	  d_freebits = nShiftLeft;
	}
      else
	{
	  d_tmpbuf |= aligned;

          TmpToBuffer();

          d_tmpbuf   = 0;
	  d_freebits = BitsPerEntry;
	}
    }
  else
    {
      // Neue Bits passen nicht mehr komplett in das aktuelle Feld.
      // Sie werden folglich in zwei Teile geteilt.

      uint32 rightpart = bits;   // Der Teil, der noch in das aktuelle Feld passt.
      uint32 leftpart  = bits;   // Der Rest, der in ein neues Feld kommt.

      int nShiftRight = nBits - d_freebits;
      if (nShiftRight != 0) rightpart >>= nShiftRight;

      d_tmpbuf |= rightpart;

      TmpToBuffer();

      int nShiftLeft = BitsPerEntry-nShiftRight;
      leftpart <<= nShiftLeft;
      d_tmpbuf = leftpart;

      d_freebits = nShiftLeft;
    }
}


void BitBuffer::AlignToByte0()
{
  d_freebits -= d_freebits%8;

  if (d_freebits==0)
    {
      TmpToBuffer();
      d_tmpbuf = 0;
      d_freebits = BitsPerEntry;
    }
}


void BitBuffer::Flush()
{
  AlignToByte0();

  EnlargeIfFull();

  assert(d_bufferidx+4 <= d_buffersize);

  while (d_freebits<BitsPerEntry)
    {
      d_buffer[d_bufferidx++] = (d_tmpbuf>>(BitsPerEntry-8));
      d_tmpbuf<<=8;
      d_freebits+=8;
    }

  assert(d_freebits == BitsPerEntry);
}


void BitBuffer::TmpToBuffer()
{
  EnlargeIfFull();

  assert(d_bufferidx+4 <= d_buffersize);
  
#ifdef WORDS_BIGENDIAN
  unsigned char* p = (unsigned char*)(&d_tmpbuf);
  for (int j=0;j<BytesPerEntry;j++)
    d_buffer[d_bufferidx++] = p[j];
#else
  unsigned char* p = (unsigned char*)(&d_tmpbuf);
  for (int j=BytesPerEntry-1;j>=0;j--)
    d_buffer[d_bufferidx++] = p[j];
#endif
}


void BitBuffer::EnlargeIfFull()
{
  if (d_bufferidx == d_buffersize)
    {
      uint8* newbuf = new uint8[d_buffersize*2];
      for (int i=0;i<d_buffersize;i++)
        newbuf[i]=d_buffer[i];
      
      d_buffersize *= 2;

      delete[] d_buffer;
      d_buffer = newbuf;
    }
}
