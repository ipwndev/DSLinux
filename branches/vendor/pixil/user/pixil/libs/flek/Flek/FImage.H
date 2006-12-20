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

#ifndef _FIMAGE_H_
#define _FIMAGE_H_

typedef unsigned char uchar;

class FImage;

class FImageIterator {

 private:
  
  friend class FImage;

  FImageIterator (uchar *p, int c=4) : ptr (p), channels (c) { }

 public:

  FImageIterator () : ptr (0), channels (4) { }

  bool operator == (FImageIterator &p) const { return (ptr == p.ptr); }

  bool operator != (FImageIterator &p) const { return (ptr != p.ptr); }

  FImageIterator operator++ () 
    { 
      ptr += channels;
      return *this;
    }

  FImageIterator operator++ (int)
    {
      FImageIterator tmp = *this;
      ptr += channels;
      return tmp;
    }

  FImageIterator operator-- ()
    {
      ptr -= channels;
      return *this;
    }

  FImageIterator operator-- (int)
    {
      FImageIterator tmp = *this;
      ptr -= channels;
      return tmp;
    }

  FImageIterator& operator = (const FImageIterator &p)
    {
      ptr = p.ptr;
      return *this;
    }

  uchar* operator* () { return ptr; }

 protected:
  
  uchar *ptr;
  int channels;
};


/** @package libflek_core
 * FImage provides four 8 bit channels (Red, Green, Blue and Alpha).
 * The image is stored internally as an array of unsigned chars in
 * RGBA order.  The internal representation can be used directly by
 * OpenGL with the GL_RGBA format and the GL_UNSIGNED_BYTE type.
 */
class FImage
{
  
 public:

  typedef FImageIterator iterator;

  /**
   * The default constructor does not allocate any space for the image.
   */
  FImage ();

  /**
   * This constructor allocated W*H pixels for storing the image.
   */
  FImage (int w, int h, int channels=4);

  /**
   * The copy constructor.
   */
  FImage (FImage *src);
  
  /** 
   * The destructor deallocated any allocated memory.
   */
  ~FImage ();

  //void width (int i) { W = i; }
  //void height (int i) { H = i; }

  /**
   * Gets the width of the image.
   */
  int width () { return W; }

  /**
   * Gets the height of the image.
   */
  int height () { return H; }

  /**
   * Gets the number of channels.
   */
  int channels () { return Channels; }

  /** 
   * A really bad scale function.
   */
  FImage* scale (int w, int h);

  void flip_vertical ();
  
  /** 
   * Clear the image to a certain color.
   */
  void clear (uchar r=0, uchar g=0, uchar b=0, uchar a=0);
  
  /**
   * Sets the number of channels.  Crops or expands the number of channels.
   */
  void channels (int c);
  
  /**
   * Return an iterator connected to the first pixel on row r of the image.
   */
  iterator begin (int r) { return iterator (Data+Channels*W*r, Channels); }

  /**
   * Return an iterator connected to the first pixel in the image.
   */
  iterator begin () { return iterator (Data, Channels); }

  /**
   * Return an iterator connected to the last+1 pixel on row r of the image.
   */
  iterator end (int r) { return iterator (Data+(Channels*W*(r+1)), Channels); }

  /**
   * Return an iterator connected to the last+1 pixel in the image.
   */
  iterator end () { return iterator (Data+(W*H*Channels), Channels); }

  /**
   * Return an iterator connected to position x, y.
   */
  iterator operator() (int x, int y) { return iterator (Data+((W*y)+x)*Channels, Channels); }

  FImage& operator = (const FImage &p) {
    delete [] Data;
    W = p.W;
    H = p.H;
    Channels = p.Channels;
    Data = new uchar [W*H*Channels];
    for (int i=0; i<W*H*Channels; i++) {
      Data[i] = p.Data[i];
    }
    return *this;
  }
  
 protected:

  uchar *Data;
  int W;
  int H;
  int Channels;

};

/**
 * Composite image B "over" image A in place.  Image B can be offset by xo, yo. 
 * And the operation may have an opacity o.
 */
FImage* composite (FImage *A, FImage *B, int xo=0, int yo=0, float o=1.0);

/**
 * Composite src "over" dest, where src and dest are an array of unsigned
 * chars in RGBA order.  opacity is an integer value from 0-255 that 
 * gives the src frame an overall opacity/transparency.  length is the
 * number of RGBA pixels to perform this operation on.
 */
void composite (unsigned char* dest, const unsigned char* src, int opacity, int length, int dest_channels, int src_channels);

/**
 * Add image B to image A in place.  Image B can be offset by xo, yo. 
 * And the operation may have an opacity o.
 */
FImage* add (FImage *A, FImage *B, int xo=0, int yo=0, float o=1.0);

/**
 * Add src to dest, where src and dest are an array of unsigned
 * chars in RGBA order.  opacity is an integer value from 0-255 that 
 * gives the src frame an overall opacity/transparency.  length is the
 * number of RGBA pixels to perform this operation on.
 */
void add (unsigned char* dest, const unsigned char* src, int opacity, int length, int dest_channels, int src_channels);

/**
 * Subtract image B from image A in place.  Image B can be offset by xo, yo. 
 * And the operation may have an opacity o.
 */
FImage* subtract (FImage *A, FImage *B, int xo=0, int yo=0, float o=1.0);

/**
 * Subtract src from dest, where src and dest are an array of unsigned
 * chars in RGBA order.  opacity is an integer value from 0-255 that 
 * gives the src frame an overall opacity/transparency.  length is the
 * number of RGBA pixels to perform this operation on.
 */
void subtract (unsigned char* dest, const unsigned char* src, int opacity, int length, int dest_channels, int src_channels);

/**
 * Create a difference from images A and B in place.  Image B can be offset by xo, yo. 
 * And the operation may have an opacity o.
 */
FImage* difference (FImage *A, FImage *B, int xo=0, int yo=0, float o=1.0);

/**
 * The difference of src and dest, where src and dest are an array of unsigned
 * chars in RGBA order.  opacity is an integer value from 0-255 that 
 * gives the src frame an overall opacity/transparency.  length is the
 * number of RGBA pixels to perform this operation on.
 */
void difference (unsigned char* dest, const unsigned char* src, int opacity, int length, int dest_channels, int src_channels);

/**
 * Lighten image A from Image B in place.  Image B can be offset by xo, yo. 
 * And the operation may have an opacity o.
 */
FImage* lighten_only (FImage *A, FImage *B, int xo=0, int yo=0, float o=1.0);

/**
 * Use the maximum (lightest) value of src and dest, where src and dest are an array of unsigned
 * chars in RGBA order.  opacity is an integer value from 0-255 that 
 * gives the src frame an overall opacity/transparency.  length is the
 * number of RGBA pixels to perform this operation on.
 */
void lighten_only (unsigned char* dest, const unsigned char* src, int opacity, int length, int dest_channels, int src_channels);

/**
 * Darken image A from Image B in place.  Image B can be offset by xo, yo. 
 * And the operation may have an opacity o.
 */
FImage* darken_only (FImage *A, FImage *B, int xo=0, int yo=0, float o=1.0);

/**
 * Use the minimum (darkest) value of src and dest, where src and dest are an array of unsigned
 * chars in RGBA order.  opacity is an integer value from 0-255 that 
 * gives the src frame an overall opacity/transparency.  length is the
 * number of RGBA pixels to perform this operation on.
 */
void darken_only (unsigned char* dest, const unsigned char* src, int opacity, int length, int dest_channels, int src_channels);

/**
 * Divide image A by Image B in place.  Image B can be offset by xo, yo. 
 * And the operation may have an opacity o.
 */
FImage* divide (FImage *A, FImage *B, int xo=0, int yo=0, float o=1.0);

/**
 * Divide dest by src, where src and dest are an array of unsigned
 * chars in RGBA order.  opacity is an integer value from 0-255 that 
 * gives the src frame an overall opacity/transparency.  length is the
 * number of RGBA pixels to perform this operation on.
 */
void divide (unsigned char* dest, const unsigned char* src, int opacity, int length, int dest_channels, int src_channels);

/**
 * Multiply image A by Image B in place.  Image B can be offset by xo, yo. 
 * And the operation may have an opacity o.
 */
FImage* multiply (FImage *A, FImage *B, int xo=0, int yo=0, float o=1.0);

/**
 * Multiply dest by src, where src and dest are an array of unsigned
 * chars in RGBA order.  opacity is an integer value from 0-255 that 
 * gives the src frame an overall opacity/transparency.  length is the
 * number of RGBA pixels to perform this operation on.
 */
void multiply (unsigned char* dest, const unsigned char* src, int opacity, int length, int dest_channels, int src_channels);

/**
 * Screen image A by Image B in place.  Image B can be offset by xo, yo. 
 * And the operation may have an opacity o.
 */
FImage* screen (FImage *A, FImage *B, int xo=0, int yo=0, float o=1.0);

/**
 * Screen dest by src, where src and dest are an array of unsigned
 * chars in RGBA order.  opacity is an integer value from 0-255 that 
 * gives the src frame an overall opacity/transparency.  length is the
 * number of RGBA pixels to perform this operation on.
 */
void screen (unsigned char* dest, const unsigned char* src, int opacity, int length, int dest_channels, int src_channels);

/**
 * Overlay image A by Image B in place.  Image B can be offset by xo, yo. 
 * And the operation may have an opacity o.
 */
FImage* overlay (FImage *A, FImage *B, int xo=0, int yo=0, float o=1.0);

/**
 * Overlay dest by src, where src and dest are an array of unsigned
 * chars in RGBA order.  opacity is an integer value from 0-255 that 
 * gives the src frame an overall opacity/transparency.  length is the
 * number of RGBA pixels to perform this operation on.
 */
void overlay (unsigned char* dest, const unsigned char* src, int opacity, int length, int dest_channels, int src_channels);

#endif
