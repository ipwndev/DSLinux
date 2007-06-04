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

#include "config.h"

#include <assert.h>

#include "heap.hh"


#define INITIAL_SIZE 50

template <class T> Heap<T>::Heap()
  : d_heap(NULL), d_size(0), d_entries(0)
{
}

template <class T> Heap<T>::~Heap()
{
  if (d_heap) delete[] d_heap;
}

template <class T> void Heap<T>::Insert(const T& e)
{
  // If heap is full, allocate a bigger one and copy the old one into it.

  if (d_entries==d_size)
    {
      int newsize = max(max(d_size*2,d_size+10),INITIAL_SIZE);
      assert(newsize>d_size);

      T* newheap = new T[newsize+1]; // Create array for new heap including an extra dummy element [0].
      for (uint32 i=1;i<=d_entries;i++)
	newheap[i] = d_heap[i];

      if (d_heap) delete[] d_heap;
      d_heap = newheap;
      d_size = newsize;
    }

  d_entries++;
  d_heap[d_entries] = e;

  UpHeap(d_entries);
}

template <class T> void Heap<T>::UpHeap(int i)
{
  while (i>1 && d_heap[i/2].AskValue() > d_heap[i].AskValue())
    {
      swap(d_heap[i],d_heap[i/2]);
      i /= 2;
    }
}

template <class T> void Heap<T>::RemoveTop()
{
  d_heap[1] = d_heap[d_entries];
  d_entries--;
  DownHeap(1);
}

template <class T> void Heap<T>::DownHeap(int i)
{
again1:

  // Element ist am unteren Rand.
  if (i*2 > d_entries)
    return;

  if (i*2==d_entries ||
      d_heap[i*2].AskValue() < d_heap[i*2+1].AskValue())
    {
      if (d_heap[i].AskValue() > d_heap[i*2].AskValue())
        {
          swap(d_heap[i],d_heap[i*2]);
          i=i*2;
          goto again1;
        }
    }
  else
    {
      if (d_heap[i].AskValue() > d_heap[i*2+1].AskValue())
        {
          swap(d_heap[i],d_heap[i*2+1]);
          i=i*2+1;
          goto again1;
        }
    }
}

