/********************************************************************************
  libvideogfx/containers/array.hh

  purpose:
    Simple 1dim array class template.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   19/Sep/2000 - Dirk Farin
   - New method CreateRange()
   30/Jun/2000 - Dirk Farin
   - first implementation
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

#ifndef LIBVIDEOGFX_CONTAINERS_ARRAY_HH
#define LIBVIDEOGFX_CONTAINERS_ARRAY_HH

#include "libvideogfx/types.hh"


template <class T> class Array
{
public:
  Array();
  Array(const Array<T>&);
  Array(int size,int base=0);
  ~Array();

  void Create(int size,int base=0);
  void CreateRange(int first,int last) { Create(last-first+1,first); }
  void Destroy();

  bool IsInitialized() const { return d_array != NULL; }

        T* Data()             { return &d_array[d_baseoffset]; }
  const T* Data_const() const { return &d_array[d_baseoffset]; }

  int AskBase() const { return -d_baseoffset;  }
  int AskSize() const { return d_size; }
  int AskStartIdx() const { return AskBase(); }
  int AskEndIdx() const { return AskSize()+AskBase()-1; }

  const Array& operator=(const Array<T>&);

private:
  int   d_baseoffset,d_size;
  T*    d_array;
};

#endif
