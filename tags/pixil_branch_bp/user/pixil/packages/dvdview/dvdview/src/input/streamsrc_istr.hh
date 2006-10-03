/********************************************************************************
  input/streamsrc_istr.hh

  purpose:
    A stream source that reads data out of a C++ "istream".

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   ??/???/1998 - Dirk Farin - first implementation

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

#ifndef INPUT_STREAMSRC_ISTR_HH
#define INPUT_STREAMSRC_ISTR_HH

#include "input/streamsrc.hh"


class StreamSource_IStream : public StreamSource
{
public:
   StreamSource_IStream();
  ~StreamSource_IStream();

  void SetIStream(class istream& istr) { d_istr = &istr; }

  uint32 FillBuffer(uint8* mem,uint32 maxlen);
  bool   MoreDataPending() const;
  
  bool   IsFiniteStream() const { return true; }
  uint64 AskStreamLength() const;
  
  bool   MaySeek() const { return true; }
  uint64 AskCurrentPosition() const;
  uint64 SetCurrentPosition(uint64);

private:
  class istream* d_istr;
};

#endif
