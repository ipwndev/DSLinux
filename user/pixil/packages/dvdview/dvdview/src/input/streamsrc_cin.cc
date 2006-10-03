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

#include "input/streamsrc_cin.hh"
#include "error.hh"

#include <iostream.h>


StreamSource_cin::StreamSource_cin()
{
}


StreamSource_cin::~StreamSource_cin()
{
}


uint32 StreamSource_cin::FillBuffer(uint8* mem,uint32 maxlen)
{
  cin.read(mem,maxlen);
  return cin.gcount();
}


bool StreamSource_cin::MoreDataPending() const
{
  return !cin.eof();
}


uint64 StreamSource_cin::AskStreamLength() const
{
  return (uint64)(-1);
}


uint64 StreamSource_cin::AskCurrentPosition() const
{
  return cin.tellg();
}


uint64 StreamSource_cin::SetCurrentPosition(uint64 pos)
{
  cin.seekg(pos);
  return pos;
}
