/********************************************************************************
  input/errors.hh

  purpose:
    Stream input errors.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   27/Sep/1999 - Dirk Farin - first implementation

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

#ifndef DVDVIEW_INPUT_ERRORS_HH
#define DVDVIEW_INPUT_ERRORS_HH

#include "error.hh"

struct Excpt_Error_InvalidData : public Excpt_Base
{ Excpt_Error_InvalidData(const char* txt="Invalid data read.") : Excpt_Base(ErrSev_Error,txt) { } };

struct Excpt_Error_UnexpectedEndOfStream : public Excpt_Base
{ Excpt_Error_UnexpectedEndOfStream()     : Excpt_Base(ErrSev_Error,"Unexpected end of stream reached.") { } };

#endif
