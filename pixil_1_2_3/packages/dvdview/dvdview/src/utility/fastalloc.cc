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

#include "utility/fastalloc.hh"
#include "error.hh"

#include <stdlib.h>


MemoryAllocator::MemoryAllocator(int minmemsize,int poolsize)
  : d_nAreasInPool(0),
    d_PoolSize(poolsize),
    d_MinMemSize(minmemsize)
{
  d_Pool = new int*[d_PoolSize];
}


MemoryAllocator::~MemoryAllocator()
{
  for (int i=0;i<d_nAreasInPool;i++)
    {
      Assert(d_Pool[i]);
      free(d_Pool[i]);
    }

  delete[] d_Pool;
}


void* MemoryAllocator::Alloc(int size,int* realsize)
{
  // Find memory block that is at least as large as the requested size.

  int minidx = -1;
  int minsize;
  int i;
  for (i=0;i<d_nAreasInPool;i++)
    if (d_Pool[i][0] >= size)
      {
	minidx=i;
	minsize=d_Pool[minidx][0];
	break;
      }


  // Now look if there is a memory block that is smaller but still large enought.

  for (;i<d_nAreasInPool;i++)
    if (d_Pool[i][0] >= size && d_Pool[i][0]<minsize)
      {
	minidx=i;
	minsize=d_Pool[i][0];
      }


  // If an appropriate memory block is found, remove this block from the pool
  // and return it.

  if (minidx>=0)
    {
      int* mem = d_Pool[minidx];
      d_Pool[minidx] = d_Pool[--d_nAreasInPool];

      if (realsize) *realsize=mem[0];

      return &mem[1];
    }
  else
    {
      // Otherwise we have to allocate a new memory block.

      int* mem = (int*)malloc(size + sizeof(int));
      mem[0] = size;

      if (realsize) *realsize=size;

      return &mem[1];
    }
}


void MemoryAllocator::Free(void* memptr)
{
  int* mem = (int*)memptr;
  mem = &mem[-1];

  if (d_nAreasInPool<d_PoolSize)
    {
      d_Pool[d_nAreasInPool++]=mem;
    }
  else
    {
      // If returned memory block does not fit into the pool anymore,
      // delete it. This is perhaps better than deleting the smallest
      // memory area as otherwise the total allocated memory for
      // grow forever. This way every memory block will eventually
      // be deleted independent of its size.

      free(mem);
    }
}

