/********************************************************************************
  input/streamsrc.hh

  purpose:
    Abstract base class: objects of this class are the source of
    the data. This may be an ordinary file, a VCD/DVD reader or
    a network connection.

  notes:
   - All methods do not throw any exceptions.

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

#ifndef INPUT_STREAMSRC_HH
#define INPUT_STREAMSRC_HH

#include "types.hh"


class StreamSource
{
public:
  virtual ~StreamSource() { }

  virtual uint32 FillBuffer(uint8* mem,uint32 maxlen) = 0;
  virtual bool   MoreDataPending() const = 0;

  /* If the stream is of finite length, you may call AskStreamLength().
     It is however not guaranteed that the size of the stream will not
     grow any more.

     These methods are provided to show a progress bar that displays
     where you are when playing from a file. Displaying a progress
     bar not always makes sense. If you are displaying a live video
     signal, it is quite useless.
  */
  virtual bool   IsFiniteStream() const = 0;
  virtual uint64 AskStreamLength() const = 0;


  /* These methods allow seeking in the input stream if this is possible.
     When the stream is located on a random access medium, seeking
     will normally be possible. On the other hand, when playing a
     stream that is received online via a network connection, seeking
     is not possible.

     Even when seeking is possible, it is not guaranteed that you can
     seek to any arbitrary position. This may be the case when you buffer
     for example the last 5 minutes of a live stream. So you may seek
     back up to 5 minutes but not more. Although this application is
     quite exotic you should check the result of "SetCurrentPosition()",
     which is the position that the class decides to set the current
     position really to.
  */
  virtual bool   MaySeek() const = 0;
  virtual uint64 AskCurrentPosition() const = 0;
  virtual uint64 SetCurrentPosition(uint64) = 0;
};

#endif
