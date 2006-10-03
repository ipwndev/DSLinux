/********************************************************************************
  utility/bytebuffer.hh

  purpose:
   Each ByteBuffer encapsulates a dynamically allocated memory area
   to which an arbitrary amount of bytes may be appended. It
   automatically enlarges if more data is appended than the current
   buffer can hold.

  notes:
  1) You must not acquire several pointers using GetPtrToAppendToBuffer() to
     fill the associated memory area later. Strictly speaking you have to get
     the pointer and fill the memory area before you call any non-const method
     with this ByteBuffer-object again.

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   28/Sep/1999 - Dirk Farin
     - Memory is now allocated via a MemoryAllocator object for better performance.
     - ByteBuffer statistics are now held externally in a ByteBufferParams
       structure that can be shared across multiple ByteBuffer objects.
   18/Dec/1998 - Dirk Farin
     - Changed several int-types to unsigned to eliminate compiler warnings.
   20/Nov/1998 - Dirk Farin
     - introduced TruncateBuffer()
   15/Nov/1998 - Dirk Farin
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

#ifndef DVDVIEW_UTILITY_BYTEBUFFER_HH
#define DVDVIEW_UTILITY_BYTEBUFFER_HH

#include "types.hh"
#include "utility/fastalloc.hh"


struct ByteBufferParams
{
  ByteBufferParams(int InitialSize_Hint,
		   int PoolSize);
  ~ByteBufferParams() { }

private:
  uint32          initialsize_hint;
  MemoryAllocator memalloc;

  // Collect information about how large these buffers usually get.
  // This helps to allocate buffers that are most of the time big enough
  // to hold all the data in them.
  unsigned int EstimatedSize;
  unsigned int LargestSize;

  // Initial buffer size if we did not collect a significant amount of
  // statistics yet.
  static const unsigned int c_InitialBufferSize;

  friend class ByteBuffer;
};


class ByteBuffer
{
public:
  ByteBuffer(ByteBufferParams&);
  ~ByteBuffer();

  void AppendBytes(unsigned char* mem,unsigned int len);
  unsigned char* GetPtrToAppendToBuffer(unsigned int len); // see note 1)
  void TruncateBuffer(unsigned int nBytes); // Throw away last 'nBytes' bytes of buffer.
  void Clear() { d_len=0; }  // Clear buffer contents but do not free memory.

  unsigned char* AskContents() const { return d_buf; }
  int            AskLength() const { return d_len; }

private:
  unsigned char* d_buf;
           int d_len;  // Amount of data in the buffer.
           int d_size; // Total size of allocated buffer memory.

  ByteBufferParams& d_param;
};

#endif
