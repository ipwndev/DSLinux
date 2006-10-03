/********************************************************************************
  libvideogfx/graphics/basic/bitmap.hh

  purpose:
    Abstract data type 'bitmap'.

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de
     University Mannheim, Dept. Circuitry and Simulation
     B 6,26 EG, room 0.10 / D-68131 Mannheim / Germany

  modifications:
    19/May/2000 - Dirk Farin - adapted comments to doxygen
    11/Jan/2000 - Dirk Farin - new method: SetSize()
    05/Nov/1999 - Dirk Farin - adapted comments to DOC++
    08/Jul/1999 - Dirk Farin - introduced ContentsNotImportant-hint
    23/Jun/1999 - Dirk Farin - bitmap border
    22/Jun/1999 - Dirk Farin - bitmap data is now shared
    02/Jun/1999 - Dirk Farin - first implementation based on DVDview code
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

#ifndef LIBVIDEOGFX_GRAPHICS_BASIC_BITMAP_HH
#define LIBVIDEOGFX_GRAPHICS_BASIC_BITMAP_HH

#include "libvideogfx/types.hh"


/**
   A two dimensional bitmap template class for arbitrary types.
   Bitmap can easily be created given its size, optional alignment and surrounding border.
   Access to the data is the same whether you want frame or field access.
   This data type is implemented as a shared data object. That means
   that assignment of bitmaps is very efficient.

   You get access to the actual bitmap data by using a variant of the AskFrame() / AskField()
   methods. These return a pointer array with pointers to the beginning of the lines of data.
   The border area can be accessed with values that are below 0 or greater/equal bitmap width.
   Use the *\_const version whenever possible as this improves performance when using bitmap sharing.

   \image latex bitmapdimensions.eps width=7cm

   Notes:
   <UL>
   <LI> Whenever you call the non-const versions of AskFrame() or AskField(),
      all pointers you got back from preceding calls to these methods may
      become illegal!
      So if it is possible that you access const-pointers and non-const-pointers
      to the same image at the same time (like in a color conversion routine)
      always get the non-const-pointers first!

   <LI> Resize(int width,int height,int halign=1,int valign=1,int border=0,bool exactalign=false)
      is not really needed. It is better to create a new one and copy the envelope object.
   <LI> Hint-methods do not affect the logical behavior of the object but can be
     used to increase performance.
   </UL>
*/
template <class Pel> class Bitmap
{
public:
  Bitmap();   /// Create Bitmap object with no bitmap data attached.
  Bitmap(const Bitmap&);
  ~Bitmap();

  void Create(int width,int height,int halign=1,int valign=1,int border=0,bool exactsize=false);
  void CreateFromExistingMemory(Pel* data,int width,int height,int border=0,
				int internalw=0,int internalh=0);

  /** Change the alignment of the bitmap. If possible the old internal bitmap sizes are not modified
      to be more time efficient. For example removing a border does nothing if \c exactsize is false. */
  void SetAlignment(int halign,int valign,int border,bool exactsize=false);

  /** Free bitmap. */
  void Destroy();

  /// Get write access to full bitmap data.
  Pel*const* AskFrame();
  /// Get read-only access to full bitmap data.
  const Pel*const* AskFrame_const() const;

  /** Get write access to a field of the bitmap. This means that consecutive elements of the pointer array
      point to lines with a distance of 2. */
  Pel*const* AskField(bool top);
  /// Get read-only access to a field of the bitmap.
  const Pel*const* AskField_const(bool top) const;

  const Pel* operator[](int y) const { return AskFrame_const()[y]; }

  /** Ask logical bitmap width.
      Logical sizes do not include aligment and border sizes. */
  int AskWidth()  const { assert(indirect); return indirect->d_width;  }
  /** Ask logical bitmap height.
      Logical sizes do not include aligment and border sizes. */
  int AskHeight() const { assert(indirect); return indirect->d_height; }

  /** Ask internal bitmap width.
      Internal sizes include alignment and borders.
      Internal sizes can be different from (be greater than) the values you expect them to be,
      except you have created the bitmap without setting 'exactsize' to true. */
  int AskInternalWidth()  const { assert(indirect); return indirect->d_internal_width;  }
  /// Ask internal bitmap height. See AskInternalWidth() for more information.
  int AskInternalHeight() const { assert(indirect); return indirect->d_internal_height; }
  /// Ask border size.
  int AskBorderWidth()    const { assert(indirect); return indirect->d_border; }

  /** Resizes bitmap. Contents will not be destroyed. You cannot resize the bitmap to any size larger
      than the current internal size excluding the borders. The new bitmap has the same alignment
      as the old one. */
  void SetSize(int w,int h);

  /** Check if bitmap contains data. This can be useful when defining images that consist of a not fixed
      set of bitmaps. You could test for example if the alpha-channel bitmap exists. */
  bool IsEmpty() const { return indirect==NULL; }

  /// Assignment operator. Not only the bitmap contents but also the alignment information will be copied.
  const Bitmap<Pel>& operator=(const Bitmap<Pel>&);

  /** Call this hint if you know that you do not need the image contents any more.
      This could be the case when:
      <UL>
      <LI> You are about to destroy it soon (but give it as parameter to another function before).
      <LI> The next time you write to the image, the contents will be completely overwritten.
      </UL>
  */
  void Hint_ContentsIsNotUsedAnymore() { d_hint_contents_not_important=true; }

  // DEBUG. Very ugly method only intended for debugging purpose.
  int AskRefCntr() const { if (!indirect) return 0;
  else return (d_hint_contents_not_important ? 100 : 0) + indirect->d_ref_cntr; }
  void ShowParam() const;


private:
  /* IMPLEMENTATION:
     Bitmap really is only an envelope class for BitmapData objects. As much as possible bitmap sharing
     is used.
  */

  /* Make a private copy of the bitmap if it is shared with other Bitmap envelope objects.
   */
  void Decouple();
  
  struct BitmapData
  {
    int   d_ref_cntr;  // envelope->letter access counter
	 
    int   d_width,d_height;
    int   d_internal_width, d_internal_height;
    int   d_halign,d_valign;
    int   d_border;

    bool  d_directmemory; ///< Bitmap class is a wrapper for a fixed memory area (behave like a singleton).

    Pel*  d_bitmapptr; // pointer to linear bitmap data.
    
    /* These pointers point to arrays of pointers that point to the beginnings of bitmap lines
       (behind the border). d_*[0] points to the topmost line including the border.
       So the top left pixel of the bitmap is accessed as d_frameptr[d_border][0] */
    Pel** d_frameptr;
    Pel** d_top_field;
    Pel** d_bottom_field;

    /* Create new bitmap. You will get exactly the size you specified. */
    void Create(int width,int height,int halign,int valign,int border);

    /* Create a wrapper for a fixed memory area. */
    void CreateDirectBitmap(Pel* start,int width,int height,
			    int internal_width,int internal_height,int border);

  private:
    friend class Bitmap<Pel>;

    static int AlignUp(int val,int alignment);
  }* indirect;

  bool  d_hint_contents_not_important; // TRUE if contents is not relevant using this Bitmap interface
};

/*! \fn void Bitmap<Pel>::Create(int width,int height,int halign=1,int valign=1,int border=0,bool exactsize=false)
  \brief Create new bitmap.
  \param width bitmap width
  \param height bitmap height
  \param halign horizontal alignment. \c width will be extended to be a multiple of this value
   (\f$width\ \textrm{mod}\ halign\f$ will be 0).
  \param valign vertical alignment. \c height will be extended to be a multiple of this value.
   (\f$height\ \textrm{mod}\ valign\f$ will be 0).
  \param border additional space that is added to all four sides after alignment has occurred.
  \param exactsize force the bitmap to have the minimum size that fulfills all given parameters

  Notes:
  <UL>
  <LI> Position [0][0] is the top left of the bitmap, left and top border pixels are
       addressed using negative indices.
  <LI> \c border should be an even number if you intend to use field access.
  <LI> Set \c exactsize to \c false if you do not depend on the actual memory layout
       as this allows for better memory reuseage.
  <LI> You may call Create() without destroying the old contents with Destroy().
  </UL>
*/

#endif
