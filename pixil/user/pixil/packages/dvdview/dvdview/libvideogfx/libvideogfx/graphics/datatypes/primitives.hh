/********************************************************************************
  libvideogfx/graphics/datatypes/primitives.hh

  purpose:
    Very basic data types.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
    17/Nov/1999 - Dirk Farin - first revision
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

#ifndef LIBVIDEOGFX_GRAPHICS_DATATYPES_PRIMITIVES_HH
#define LIBVIDEOGFX_GRAPHICS_DATATYPES_PRIMITIVES_HH

template <class T> struct Point2D
{
  T x,y;
};

template <class T> struct Rect2D
{
  Point2D<T> upperleft;
  Point2D<T> lowerright;
};

#endif
