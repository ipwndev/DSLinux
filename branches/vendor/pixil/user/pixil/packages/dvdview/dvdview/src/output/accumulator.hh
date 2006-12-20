/********************************************************************************
  output/accumulator.hh
    Build continuous regions of image data from output stripes.

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, Kapellenweg 15, 72070 Tuebingen, Germany,
     email: farindk@trick.informatik.uni-stuttgart.de

  modifications:
   20/Sep/2000 - Dirk Farin
     - Moved class definition to a file of its own.
   19/Sep/2000 - Dirk Farin
     - Accumulate helper class to ease postprocessor and display writing.
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

#ifndef DVDVIEW_OUTPUT_ACCUMULATOR_HH
#define DVDVIEW_OUTPUT_ACCUMULATOR_HH

#include "video12/output.hh"
#include "libvideogfx/containers/array.hh"

class ImageDataAccumulator
{
public:
  ImageDataAccumulator() : d_width(0), d_height(0), d_sink(NULL) { }
  virtual ~ImageDataAccumulator() { }

  void SetNext(DecodedPictureSink* next) { d_sink = next; } /* The image sink, where unused
							       image strip data is sent to. */
  void              StartAccumulation(int first,int last,         // range to accumulate
				      bool modifyable_copy=true); // if we are going to write to the data
  DecodedImageData* Accumulate(DecodedImageData*);  // !NULL -> image is complete

private:
  DecodedImageData d_dimg;

  Array<bool> d_line_available;
  bool        d_modifyable_copy;

  int         d_width,d_height;

  class DecodedPictureSink* d_sink;
};

#endif
