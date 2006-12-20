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

#include "input/streamsrc_istr.hh"
#include "error.hh"

#include <iostream.h>


StreamSource_IStream::StreamSource_IStream()
 : d_istr(NULL)
{
}


StreamSource_IStream::~StreamSource_IStream()
{
}


uint32 StreamSource_IStream::FillBuffer(uint8* mem,uint32 maxlen)
{
  Assert(d_istr);

  d_istr->read(mem,maxlen);
  return d_istr->gcount();
}


bool StreamSource_IStream::MoreDataPending() const
{
  Assert(d_istr);

  return !d_istr->eof();
}


uint64 StreamSource_IStream::AskStreamLength() const
{
  Assert(d_istr);

  int64 currentpos; currentpos = d_istr->tellg();
  d_istr->seekg(0,ios::end);
  int64 length; length = d_istr->tellg();
  d_istr->seekg(currentpos);

  return length;
}


uint64 StreamSource_IStream::AskCurrentPosition() const
{
  return d_istr->tellg();
}


uint64 StreamSource_IStream::SetCurrentPosition(uint64 pos)
{
  d_istr->seekg(pos);
  return pos;
}
