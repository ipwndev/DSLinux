/********************************************************************************
  utility/heap.hh

  purpose:
    Heap container class template.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   05/Jan/2000 - Dirk Farin
     - first revision
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

#ifndef LIBVIDEOGFX_UTILITY_HEAP_HH
#define LIBVIDEOGFX_UTILITY_HEAP_HH

#include <assert.h>

#include "libvideogfx/types.hh"


class HeapElement
{
public:
  virtual ~HeapElement() { }

  virtual int AskValue() const = 0;
};

template <class T> class Heap
{
public:
   Heap();
  ~Heap();

  void Insert(const T&);
  T&   AskTop() const { assert(d_entries>0); return d_heap[1]; }
  void RemoveTop(); // Remove minimum element.

  bool   IsEmpty() const { return d_entries==0; }
  uint32 Length()  const { return d_entries;    }

private:
  T*  d_heap;
  int d_size; // Size of heap excluding the first (dummy) element

  int d_entries;

  void UpHeap(int i);
  void DownHeap(int i);
};

#endif
