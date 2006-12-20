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

#include "system/resync.hh"
#include "system/sysdec1.hh"
#include "input/errors.hh"
#include "error.hh"


SystemResyncer::SystemResyncer()
  : d_src(NULL)
{
}


SysPacket* SystemResyncer::GetNextPacket()
{
  Assert(d_src);

  int errcnt=0;

  for (;;)
    {
      try
	{
	  SysPacket* syspck = d_src->GetNextPacket();
	  return syspck;
	}
      catch(Excpt_Error_InvalidData& e)
	{
	  // Display error message only the first time as consecutive errors are
	  // most probably resynchronization errors.
	  if (errcnt==0)
	    MessageDisplay::Show(ErrSev_Warning,e.m_text);
	  errcnt++;

	  d_src->Resync();
	}
    }
}
