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

#ifndef __FSGI__
#define __FSGI__

#include <Flek/FImage.H>

/** @package libflek_core
 * The fSGI class provides static methods that read, write and test SGI images,
 * which are also refered to as "RGB" or "Iris" images.
 */
class FSGI {
 public:

  enum {
    NONE = 0,
    RLE = 1,
    ARLE = 2,
    MAGIC = 474
  };

  /**
   * Check to see if this seems to be a valid SGI format image.
   */
  static bool valid (char *filename);

  /**
   * Read an SGI Image from disk and return a new FImage object.
   */
  static FImage * read (char *filename);

  /**
   * Write an SGI Image to disk from an FImage object.
   */  
  static int write (char *filename, FImage* img, int compression = RLE, int channels=3);

};

#endif
