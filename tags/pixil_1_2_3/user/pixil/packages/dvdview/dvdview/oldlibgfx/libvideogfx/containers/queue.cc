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

#include "libvideogfx/containers/queue.hh"
#include <assert.h>


#define INITIAL_SIZE 50

template <class T> Queue<T>::Queue()
  : d_queue(NULL), d_size(0), d_entries(0)
{
}

template <class T> Queue<T>::~Queue()
{
  if (d_queue) delete[] d_queue;
}

template <class T> void Queue<T>::Append(const T& e)
{
  // If queue is full, allocate a bigger one and copy the old one into it.

  if (d_entries==d_size)
    {
      Enlarge();
    }

  d_queue[ (d_first+d_entries)%d_size ] = e;
  d_entries++;
}

template <class T> void Queue<T>::AppendAtHead(const T& e)
{
  // If queue is full, allocate a bigger one and copy the old one into it.

  if (d_entries==d_size)
    {
      Enlarge();
    }

  d_first = (d_first-1+d_size)%d_size;
  d_queue[d_first] = e;
  d_entries++;
}

template <class T> void Queue<T>::Enlarge()
{
  int newsize = max(max(d_size*2,d_size+10),INITIAL_SIZE);
  assert(newsize>d_size);

  T* newqueue = new T[newsize];
  for (uint32 i=0;i<d_entries;i++)
    newqueue[i] = d_queue[(i+d_first)%d_size];

  if (d_queue) delete[] d_queue;
  d_queue = newqueue;
  d_size  = newsize;
  d_first = 0;
}

template <class T> T& Queue<T>::AskTail() const
{
  assert(!IsEmpty());
  return d_queue[(d_first+d_entries-1)%d_size];
}

template <class T> T& Queue<T>::AskFromHead(uint32 idx) const
{
  assert(!IsEmpty());
  assert(idx<d_entries);
  return d_queue[(d_first+idx)%d_size];
}

template <class T> T& Queue<T>::AskFromTail(uint32 idx) const
{
  assert(!IsEmpty());
  assert(idx<d_entries);
  return d_queue[(d_first+d_entries-1-idx)%d_size];
}

template <class T> void Queue<T>::RemoveHead()
{
  assert(!IsEmpty());
  d_first++; d_first %= d_size;
  d_entries--;
}

template <class T> void Queue<T>::RemoveFromHead(uint32 idx)
{
  assert(!IsEmpty());
  assert(idx<d_entries);

  for (uint32 i=0;i<d_entries-idx-1;i++)
    d_queue[(d_first+idx+i)%d_size] = d_queue[(d_first+idx+i+1)%d_size];

  d_entries--;
}
