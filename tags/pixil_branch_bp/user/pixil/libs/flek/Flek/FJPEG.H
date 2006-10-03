/* -*-C++-*- 

   "$Id$"
   
   Copyright 1999-2000 by the Flek development team.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.
   
   Please report all bugs and problems to "flek-devel@sourceforge.net".

*/

#ifndef _FJPEG_H_
#define _FJPEG_H_

#include <Flek/FImage.H>

/** @package libflek_jpeg
 * The FJPEG class provides static methods that read, write and test JPEG images.
 */
class FJPEG {
 public:

  /*
   * Check the PBM/PGM/PPM magic number to see if this image matches any of
   * the recognized formats.
   */
  //static bool valid (char *filename);

  /**
   * Read a JPEG from disk and return a new fImage object.
   */
  static FImage * read (char *filename);

  /**
   * Write a JPEG to disk from an fImage object.
   */  
  static int write (char *filename, FImage* img, int qaulity);

};

#endif
