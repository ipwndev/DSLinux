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

#include "image.hh"


template <class Pel> void Image<Pel>::_Create(const ImageSpec& spec,bool bitmaps12,bool subh,bool subv)
{
  // Create first bitmap (red channel or Y-channel)

  d_bm[0].Create(spec.width,spec.height,spec.halign,spec.valign,spec.border,spec.exact_size);


  // Create second and third bitmap (green, blue or U,V-channel)
  
  if (bitmaps12)
    {
      ImageSpec chrspec = spec;

      if (subh)
	{
	  chrspec.width =(chrspec.width +1)/2;
	  //chrspec.halign/=2;
	}
      if (subv)
	{
	  chrspec.height=(chrspec.height+1)/2;
	  //chrspec.valign/=2;
	}

      d_bm[1].Create(chrspec.width,chrspec.height,chrspec.halign,chrspec.valign,
		     chrspec.border,chrspec.exact_size);
      d_bm[2].Create(chrspec.width,chrspec.height,chrspec.halign,chrspec.valign,
		     chrspec.border,chrspec.exact_size);
    }


  // Create fourth bitmap (alpha mask)

  if (spec.has_alphamask) 
    d_bm[Bitmap_Alpha].Create(spec.width,spec.height,spec.halign,spec.valign,spec.border,spec.exact_size);

  d_param = spec;
}


template <class Pel> void Image<Pel>::_Destroy()
{
  for (int i=0;i<4;i++)
    d_bm[i].Destroy();
}


template <class Pel> void Image<Pel>::ReplaceBitmap(BitmapChannel id,Bitmap<Pel>& bm)
{
  d_bm[id] = bm;

  if (id==Bitmap_Alpha)
    {
      if (bm.IsEmpty())
	d_param.has_alphamask = false;
      else
	d_param.has_alphamask = true;
    }
}


template <class Pel> Image<Pel>::Image(const Image<Pel>& img)
{
  for (int i=0;i<4;i++)
    d_bm[i] = img.d_bm[i];
  d_param = img.d_param;
}


template <class Pel> const Image<Pel>& Image<Pel>::operator=(const Image<Pel>& img)
{
  for (int i=0;i<4;i++)
    d_bm[i] = img.d_bm[i];
  d_param = img.d_param;

  return *this;
}



// ----------------------- RGB images -----------------------------------



template <class Pel> void Image_RGB<Pel>::Create(const ImageSpec& spec)
{
  _Create(spec,
	  true,       /* 3 bitmaps */
	  false,false /* no subsampling */);
}


// ------------------------ YUV images ---------------------------------

template <class Pel> void Image_YUV<Pel>::Create(const ImageSpec_YUV& spec)
{
  ImageSpec s;
  ((ImageParam&)s)=spec;
  ((ImageInfo_Alignment&)s)=spec;

  _Create(s,!spec.nocolor,
	  spec.reduced_chroma_size && IsSubH(spec.chroma),
	  spec.reduced_chroma_size && IsSubV(spec.chroma));

  d_info_yuvextra = spec;
}


template <class Pel> void Image_YUV<Pel>::SetChromaFormat(ChromaFormat cf)
{
#ifndef NDEBUG
  assert(!d_bm[Bitmap_Y].IsEmpty());
  assert(d_bm[Bitmap_Y].AskWidth()  == d_param.width);
  assert(d_bm[Bitmap_Y].AskHeight() == d_param.height);

  if (d_info_yuvextra.nocolor)
    {
      assert(d_bm[Bitmap_U].IsEmpty());
      assert(d_bm[Bitmap_V].IsEmpty());
    }
  else
    {
      assert(!d_bm[Bitmap_U].IsEmpty());
      assert(!d_bm[Bitmap_V].IsEmpty());

      int w = d_param.width;
      int h = d_param.height;

      if (IsSubH(cf)) w=(w+1)/2;
      if (IsSubV(cf)) h=(h+1)/2;

      assert(d_bm[Bitmap_U].AskWidth()  == w);
      assert(d_bm[Bitmap_U].AskHeight() == h);
      assert(d_bm[Bitmap_V].AskWidth()  == w);
      assert(d_bm[Bitmap_V].AskHeight() == h);
    }
#endif

  d_info_yuvextra.chroma=cf;
}
