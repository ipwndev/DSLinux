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

#include "utility/bytebuffer.hh"
#include "error.hh"

#include <string.h>


const unsigned int ByteBufferParams::c_InitialBufferSize = 10*1024-64;


ByteBufferParams::ByteBufferParams(int InitialSize_Hint,int poolsize)
  : initialsize_hint(InitialSize_Hint),
    memalloc(InitialSize_Hint,poolsize),
    EstimatedSize(c_InitialBufferSize),
    LargestSize(0)
{
}


#define OLDVERSION 0


// This factor defines what fraction of the new estimation will be taken from the
// old estimation.
#define AgingFactor 15:16


ByteBuffer::ByteBuffer(ByteBufferParams& p)
  : d_param(p)
{
  if (d_param.initialsize_hint==0)
    {
      d_buf  = NULL;
      d_len  = 0;
      d_size = 0;
    }
  else
    {
#if OLDVERSION
      d_buf  = new unsigned char[d_param.initialsize_hint];
      Assert(d_buf);
      d_len  = 0;
      d_size = d_param.initialsize_hint;
#else
      int s;
      d_buf  = (unsigned char*)(d_param.memalloc.Alloc(d_param.initialsize_hint,&d_size));
      Assert(d_buf);
      d_len  = 0;
#endif
    }
}

ByteBuffer::~ByteBuffer()
{
  if (d_buf)
    {
#if OLDVERSION
      delete[] d_buf;
#else
      d_param.memalloc.Free(d_buf);
#endif

      // Update statistics.

      d_param.EstimatedSize = (d_param.EstimatedSize * (1?AgingFactor) +          // old fraction
			       d_len * ((0?AgingFactor)-(1?AgingFactor)))   // new fraction
        / (0?AgingFactor);

      if (d_len > d_param.LargestSize) d_param.LargestSize=d_len;
    }
}


unsigned char* ByteBuffer::GetPtrToAppendToBuffer(unsigned int len)
{
  Assert(len>0);

  // If contents will not fit into the buffer, enlarge buffer.
  if (d_len+len > d_size)
    {
      unsigned int newsize=d_size;

      // No buffer allocated so far, use estimated size.
      if (newsize==0)           newsize = d_param.EstimatedSize;

      // If this is not enough, use larger buffer.
      if (newsize<d_len+len && d_param.LargestSize>0)  newsize = d_param.LargestSize;

      // If still not enough, double size until big enough.
      // This will allocate buffers that are far too big in some cases.
      // But allocating just the size that is needed will result in poor
      // performance when appending many small portions of data.
      while (newsize<d_len+len) newsize *= 2;


      // Allocate new buffer and copy old buffer contents to new buffer.

#if OLDVERSION
      unsigned char* newbuf = new unsigned char[newsize];
#else
      int newlen;
      unsigned char* newbuf = (unsigned char*)(d_param.memalloc.Alloc(newsize,&newlen));
#endif

      Assert(newbuf); // Memory error handler should have caught error.
      if (d_len>0) { memcpy(newbuf,d_buf,d_len); }

#if OLDVERSION
      // The old buffer can now be replaced with the new one and be deleted.
      if (d_buf) { delete[] d_buf; }
      d_buf=newbuf;
      d_size=newsize;
#else
      // The old buffer can now be replaced with the new one and be deleted.
      if (d_buf) { d_param.memalloc.Free(d_buf); }
      d_buf=newbuf;
      d_size=newlen;
#endif
    }

  // There has to be enough memory left now.
  Assert(d_len+len <= d_size);

  // Return pointer, pointing behind the last already filled byte in the buffer and
  // enlarge the buffer contents size variable.
  unsigned char* newdatastart = &d_buf[d_len];
  d_len += len;

  return newdatastart;
}


void ByteBuffer::TruncateBuffer(unsigned int nBytes)
{
  Assert(nBytes <= d_len);
  d_len -= nBytes;
}


void ByteBuffer::AppendBytes(unsigned char* mem,unsigned int len)
{
  if (len>0)
    {
      unsigned char* buffermem = GetPtrToAppendToBuffer(len);

#if 0
      if (len<200)
        {
          while (len)
            {
              *buffermem++  = *mem++;
              len--;
            }
        }
      else
#endif
        {
          // Append new contents behind current buffer data.

          memcpy(buffermem,mem,len);
        }
    }
}
