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

#include <iostream.h>
#include <string.h>

#include "bitmap.hh"


/* Return 'val', rounded upward to the next multiple of 'alignment'.
 */
template <class Pel> inline int Bitmap<Pel>::BitmapData::AlignUp(int val,int alignment)
{
  assert(alignment>0);

  int alignedval = val;
  if ((alignedval%alignment)!=0) alignedval += alignment-(val%alignment);

  assert((alignedval%alignment)==0);  // alignment is correct
  assert(alignedval < val+alignment); // aligned value is only as large as needed
  assert(alignedval >= alignment);    // aligned value is not smaller than before

  return alignedval;
}


static bool AlignmentMatches(int align,int val)
{
  return (val%align)==0;
}


template <class Pel> Bitmap<Pel>::Bitmap()
  : indirect(NULL),
    d_hint_contents_not_important(true) // contents cannot be important as it is empty/undefined
{
}


template <class Pel> Bitmap<Pel>::Bitmap(const Bitmap& bm)
  : indirect(bm.indirect),
    d_hint_contents_not_important(bm.d_hint_contents_not_important)
{
  if (indirect)
    indirect->d_ref_cntr++;

  if (!indirect)
    {
      assert(d_hint_contents_not_important);
    }
}


template <class Pel> Bitmap<Pel>::~Bitmap()
{
  Destroy();

  assert(indirect==NULL);
}


/* From the Bitmap class point of view, the bitmap is destroyed. From a global point of view
   this is only strictly correct of it was the last reference to this bitmap.
*/
template <class Pel> void Bitmap<Pel>::Destroy()
{
  if (indirect)
    {
      indirect->d_ref_cntr--;
      if (indirect->d_ref_cntr==0)
	{
	  if (!indirect->d_directmemory) delete[] indirect->d_bitmapptr;
	  delete[] indirect->d_frameptr;
	  delete[] indirect->d_top_field;
	  delete[] indirect->d_bottom_field;
	  delete indirect;
	}

      indirect=NULL;
    }
}


template <class Pel> void Bitmap<Pel>::Decouple()
{
  if (indirect==NULL)
    return;

  if (indirect->d_ref_cntr>1)
    {
      // Cannot decouple direct memory bitmaps.
      assert(!indirect->d_directmemory);

      // create new exclusive BitmapData with the same contents
      BitmapData* bmdata = new BitmapData;
      bmdata->Create(indirect->d_width,indirect->d_height,
		     indirect->d_halign,indirect->d_valign,
		     indirect->d_border);
	  
      if (!d_hint_contents_not_important)
	{
	  cout << "MEMCPY 1 --------------\n";
	  memcpy(bmdata->d_bitmapptr,
		 indirect->d_bitmapptr,
		 indirect->d_internal_width*indirect->d_internal_height*sizeof(Pel));
	}
	  
      bmdata->d_ref_cntr=1;
      indirect->d_ref_cntr--;
      indirect=bmdata;
    }
  else
    {
      // We are the only user of the data.
    }
}


template <class Pel> void Bitmap<Pel>::Create(int width,int height,int halign,int valign,int border,bool exactsize)
{
  /* If this bitmap contains already a bitmap and the sizes are
     the same as those requested. Don't create a new one but reuse the old one instead. */
  if (indirect && // has access to a bitmap
      indirect->d_ref_cntr==1 && /* We are the single user of the old bitmap.
				    This is not really necessary as the bitmap could be shared until
				    we need write-access. But then in Decouple() the bitmap (with random
				    contents) would be copied. To prevent this, we would have to keep track
				    if the contents is random. As most of the time a bitmap is created it
				    will also be written to, we allocate a new one here already.

				    NOTE: Deferred creation could be useful in this case:
				    Bitmap a;
				    [do magic things to 'a']
				    {
				    Bitmap b;
				    b=a;
				    a.Create(...);
				    }
				    [do magic things to 'a']
				 */
      indirect->d_width  ==  width &&    // same width
      indirect->d_height == height &&    // same height
      (( exactsize &&                    // exactly the same alignment of exactsize is wanted
	 indirect->d_halign == halign &&
	 indirect->d_valign == valign &&
	 indirect->d_border == border) ||
       (!exactsize &&
  	 AlignmentMatches(halign,indirect->d_halign) && // old alignment is stronger than new one
	 AlignmentMatches(valign,indirect->d_valign) &&
	 indirect->d_border >= border)))
    {
      // Can reuse old bitmap.

      return;
    }

  // Destroy old bitmap (or at least the reference to the letter-object).
  Destroy();

  // Create new bitmap of correct size.
  indirect = new Bitmap<Pel>::BitmapData;
  indirect->Create(width,height,halign,valign,border);
  indirect->d_ref_cntr=1;
  d_hint_contents_not_important=true;
}


template <class Pel> void Bitmap<Pel>::CreateFromExistingMemory(Pel* data,int width,int height,int border,
								int internalw,int internalh)
{
  Destroy();

  indirect = new Bitmap<Pel>::BitmapData;
  indirect->CreateDirectBitmap(data,width,height,internalw,internalh,border);
  indirect->d_ref_cntr=1;
  d_hint_contents_not_important=true;
}


template <class Pel> void Bitmap<Pel>::ShowParam() const
{
  if (!indirect)
    {
      cout << "No actual data in bitmap.\n";
    }
  else
    {
      cout << " w, h: " << indirect->d_width << " x " << indirect->d_height << endl;
      cout << "iw,ih: " << indirect->d_internal_width << " x " << indirect->d_internal_height << endl;
      cout << "ha,va: " << indirect->d_halign << " x " << indirect->d_valign << endl;
      cout << "border:" << indirect->d_border << endl;
      cout << "direct memory: " << (indirect->d_directmemory ? "yes" : "no") << endl;
    }
}


template <class Pel> void Bitmap<Pel>::SetAlignment(int halign,int valign,int border,bool exactsize)
{
  assert(indirect);

  int newibmw = BitmapData::AlignUp(indirect->d_width ,halign);
  int newibmh = BitmapData::AlignUp(indirect->d_height,valign);
  int newinternw = newibmw + 2*border;
  int newinternh = newibmh + 2*border;

  bool modify=false;
  if (!exactsize)
    {
      // Passt das Alignment nicht mehr oder wird der Border groesser, muss neue BM erstellt werden.

      if (newibmw > indirect->d_internal_width -2*indirect->d_border ||
	  newibmh > indirect->d_internal_height-2*indirect->d_border ||
	  border > indirect->d_border)
	modify = true;
    }
  else
    {
      if (newinternw != indirect->d_internal_width ||
	  newinternh != indirect->d_internal_height ||
	  border != indirect->d_border)
	modify = true;
    }

  if (modify)
    {
      assert(!indirect->d_directmemory);

      Bitmap<Pel> newbm;
      newbm.Create(indirect->d_width,indirect->d_height,halign,valign,border,exactsize);
      const Pel*const* sp = AskFrame_const();
            Pel*const* dp = newbm.AskFrame();

      int b = AskBorderWidth();
      if (border<b) b=border;

      cout << "MEMCPY 2 -------\n";

      for (int y=-b;y<indirect->d_height+b;y++)
	memcpy(&dp[y][-b],&sp[y][-b],(indirect->d_width+2*b)*sizeof(Pel));

      *this = newbm;
    }
}


template <class Pel> void Bitmap<Pel>::BitmapData::Create(int width,int height,
							  int halign,int valign,int border)
{
  assert(halign>0);
  assert(valign>0);
  assert(border>=0);
  assert(width >0);
  assert(height>0);

  d_width  = width;
  d_height = height; 
  d_halign = halign;
  d_valign = valign; 
  d_border = border;

  d_internal_width  = AlignUp(width ,halign) +2*border;
  d_internal_height = AlignUp(height,valign) +2*border;

  d_bitmapptr    = new Pel[d_internal_width * d_internal_height];
  d_frameptr     = new Pel* [d_internal_height];
  d_top_field    = new Pel* [(d_internal_height+1)/2];
  d_bottom_field = new Pel* [(d_internal_height+1)/2];

  d_directmemory = false;

  // Frame-Pointer ausfuellen
  {for (int y=0;y<d_internal_height;y++)
    d_frameptr[y]= &d_bitmapptr[y*d_internal_width+border];}

  // Field-Pointer ausfuellen
  {for (int y=0,y2=0;y2<d_internal_height;y++)
    {
      d_top_field   [y]= &d_bitmapptr[y2*d_internal_width+border]; y2++;
      d_bottom_field[y]= &d_bitmapptr[y2*d_internal_width+border]; y2++;
    }}
}


template <class Pel> void Bitmap<Pel>::BitmapData::CreateDirectBitmap(Pel* mem,int width,int height,
								      int internal_w,int internal_h,
								      int border)
{
  if (internal_w==0) internal_w=width;
  if (internal_h==0) internal_h=height;

  assert(internal_w>=width);
  assert(internal_h>=height);
  assert(width >0);
  assert(height>0);
  assert(border>=0);

  d_width  = width;
  d_height = height; 
  d_halign = 1;
  d_valign = 1; 
  d_border = border;

  d_internal_width  = internal_w;
  d_internal_height = internal_h;

  d_bitmapptr    = mem;
  d_frameptr     = new Pel* [d_internal_height];
  d_top_field    = new Pel* [(d_internal_height+1)/2];
  d_bottom_field = new Pel* [ d_internal_height   /2];

  d_directmemory = true;

  // Frame-Pointer ausfuellen
  {for (int y=0;y<d_internal_height;y++)
    d_frameptr[y]= &d_bitmapptr[y*d_internal_width+border];}

  // Field-Pointer ausfuellen
  {for (int y=0,y2=0;y2<d_internal_height;y++)
    {
      d_top_field   [y]= &d_bitmapptr[y2*d_internal_width+border]; y2++;
      d_bottom_field[y]= &d_bitmapptr[y2*d_internal_width+border]; y2++;
    }}
}


template <class Pel> Pel*const* Bitmap<Pel>::AskFrame()
{
  assert(indirect);
  Decouple();

  d_hint_contents_not_important=false;
  return &indirect->d_frameptr[indirect->d_border];
}


template <class Pel> const Pel*const * Bitmap<Pel>::AskFrame_const() const
{
  assert(indirect);
  return &indirect->d_frameptr[indirect->d_border];
}


template <class Pel> Pel*const* Bitmap<Pel>::AskField(bool top)
{
  assert(indirect);
  Decouple();

  d_hint_contents_not_important=false;
  return top ? &indirect->d_top_field   [indirect->d_border/2]   :
               &indirect->d_bottom_field[indirect->d_border/2];
}

template <class Pel> const Pel*const* Bitmap<Pel>::AskField_const(bool top) const
{
  assert(indirect);
  return top ? &indirect->d_top_field   [indirect->d_border/2] :
               &indirect->d_bottom_field[indirect->d_border/2];
}


template <class Pel> void Bitmap<Pel>::SetSize(int w,int h)
{
  assert(indirect);

  assert(w<=indirect->d_internal_width  -2*indirect->d_border &&
	 h<=indirect->d_internal_height -2*indirect->d_border);

  indirect->d_width  = w;
  indirect->d_height = h;
}


template <class Pel> const Bitmap<Pel>& Bitmap<Pel>::operator=(const Bitmap<Pel>& bm)
{
  if (bm.indirect==NULL)
    {
      // Copying an empty bitmap destroys the current bitmap.

      Destroy();
    }
  else
    {
      bm.indirect->d_ref_cntr++;
      Destroy();
      indirect=bm.indirect;
      d_hint_contents_not_important = bm.d_hint_contents_not_important;
    }

  return *this;
}


