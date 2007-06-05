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

#include "tests/memalloc.hh"
#include "utility/fastalloc.hh"
#include "error.hh"

#include <iostream.h>

#include <stdlib.h>
#include <time.h>


void CheckMemAlloc(bool log)
{
  srand(time(0));

  MemoryAllocator al(100,25);

#define ArraySize 20

  void* a[ArraySize];

  for (int i=0;i<ArraySize;i++)
    a[i]=NULL;

  for (int i=0;i<100000;i++)
    {
      int idx = rand()%ArraySize;
      if (a[idx])
	{
	  if (log) cout << "freeing " << a[idx] << endl;

	  al.Free(a[idx]);
	  a[idx]=NULL;
	}
      else
	{
	  int size = (rand()%1000)+100;
	  a[idx]=al.Alloc(size);
	  Assert(a[idx] != NULL);

	  if (log) cout << "allocating " << size << " bytes -> " << a[idx] << endl;

	  for (int x=0;x<size;x++)
	    ((uint8*)(a[idx]))[x]=0xF0;
	}
    }

  for (int i=0;i<ArraySize;i++)
    if (a[i])
      al.Free(a[i]);
}
