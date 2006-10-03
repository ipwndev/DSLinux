/********************************************************************************
  libvideogfx/containers/array2.hh

  purpose:
    Simple 2dim array class template.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   30/Sep/1999 - Dirk Farin
     - integrated from old DVDview into ULib
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

#ifndef LIBVIDEOGFX_CONTAINERS_ARRAY2_HH
#define LIBVIDEOGFX_CONTAINERS_ARRAY2_HH

#include "libvideogfx/types.hh"


template <class T> class Array2
{
public:
  Array2();
  Array2(int width,int height);
  ~Array2();

  void Create(int width,int height);

  bool IsInitialized() const { return d_array != NULL; }

        T& Ask      (int y,int x)       { return d_array[y][x]; }
  const T& Ask_const(int y,int x) const { return d_array[y][x]; }

  int AskWidth()  const { return d_width;  }
  int AskHeight() const { return d_height; }

private:
  int   d_width,d_height;
  T**   d_array;
};

#endif
