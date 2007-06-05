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

#include "libvideogfx/containers/array2.hh"


template <class T> Array2<T>::Array2()
{
  d_width = d_height = 0;
  d_array = 0;
}


template <class T> Array2<T>::Array2(int width,int height)
{
  Create(width,height);
}


template <class T> void Array2<T>::Create(int width,int height)
{
  if (d_array)
    {
      if (d_width == width && d_height == height)
	return;

      delete[] d_array[0];
      delete[] d_array;
    }

  d_width  = width;
  d_height = height; 

  T* bitmap = new T[d_width * d_height];
  d_array   = new T* [d_height];

  {for (int y=0;y<d_height;y++)
    d_array[y]= &bitmap[y*d_width];}
}


template <class T> Array2<T>::~Array2()
{
  if (d_array)
    {
      delete[] d_array[0];
      delete[] d_array;
    }
}

