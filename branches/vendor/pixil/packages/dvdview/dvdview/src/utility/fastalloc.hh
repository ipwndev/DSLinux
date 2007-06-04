/********************************************************************************
  utility/fastalloc.hh

  purpose:
    Very fast memory allocation.

  notes:
   - It is suggested that you create a separate MemoryAllocator object
     for each purpose of allocation as the allocator may generate statistics
     on how large the allocated memory area usually are. This may be needed
     for optimal performance.

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   28/Sep/1999 - Dirk Farin
     - first implementation based on old DVDView's alloc.cc

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

#ifndef DVDVIEW_UTILITY_FASTALLOC_HH
#define DVDVIEW_UTILITY_FASTALLOC_HH

#include "types.hh"


class MemoryAllocator
{
public:
  MemoryAllocator(int MinimumMemorySize,int PoolSize);
  ~MemoryAllocator();

  void* Alloc(int size,int* realsize=NULL);
  void  Free(void*);

private:
  int** d_Pool;
  int     d_nAreasInPool;
  int     d_PoolSize;
  int     d_MinMemSize;
};

#endif
