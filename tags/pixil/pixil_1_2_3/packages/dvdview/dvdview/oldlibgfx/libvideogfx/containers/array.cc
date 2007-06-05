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

#include "array.hh"


template <class T> Array<T>::Array()
  : d_baseoffset(0),d_size(0), d_array(NULL)
{
}


template <class T> Array<T>::Array(int size,int base)
  : d_array(NULL)
{
  Create(size,base);
}


template <class T> Array<T>::Array(const Array<T>& a)
  : d_array(NULL)
{
  *this = a;
}


template <class T> void Array<T>::Create(int size,int base)
{
  if (d_array)
    {
      if (d_size == size)
	{
	  d_baseoffset = -base;
	  return;
	}

      delete[] d_array;
    }

  d_size       = size;
  d_baseoffset = -base;

  d_array   = new T[d_size];
}


template <class T> Array<T>::~Array()
{
  Destroy();
}


template <class T> void Array<T>::Destroy()
{
  if (d_array)
    {
      delete[] d_array;
    }

  d_array=NULL;
  d_baseoffset=0;
  d_size=0;
}


template <class T> const Array<T>& Array<T>::operator=(const Array<T>& a)
{
  Destroy();

  if (a.d_array)
    {
      Create(a.d_size, -a.d_baseoffset);

      for (int i=0;i<d_size;i++)
	d_array[i] = a.d_array[i];
    }

  return *this;
}
