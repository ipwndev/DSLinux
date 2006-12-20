
#include "output/accumulator.hh"
#include <string.h>

#define DEBUGINFO 0

#if DEBUGINFO
void ShowDIMG(DecodedImageData* dimg)
{
  cout << dimg->m_src_y_start << "-" << dimg->m_src_y_end << " to "
       << dimg->m_dst_y_start << "-" << dimg->m_dst_y_start+dimg->m_src_y_end-dimg->m_src_y_start
       << (dimg->m_may_modify ? " may modify" : " may not modify") << endl;
}
#endif


void ImageDataAccumulator::StartAccumulation(int first,int last,bool modify)
{
  // Round range up multiples of 4, so we don't have any problems with choma even in
  // field pictures.

  //first &= ~3;
  //last = (last+3) & ~3;   DOESN't WORK (watch chroma with "spaceneedle.mpg")

  // cout << "START ACCUMULATION " << first << " -> " << last << endl;

  d_line_available.CreateRange(first,last);
  bool* p = d_line_available.Data();
  for (int i=first;i<=last;i++)
    p[i]=false;

  d_modifyable_copy = modify;
}


DecodedImageData* ImageDataAccumulator::Accumulate(DecodedImageData* dimg)
{
  const int first = d_line_available.AskStartIdx();                      // first line to accumulate into
  const int last  = min(dimg->m_height-1,d_line_available.AskEndIdx());  // last line to accumulate into

  const int ys = dimg->m_dst_y_start;                        // first output line
  const int ye = ys + dimg->m_src_y_end-dimg->m_src_y_start; // last output line

#if DEBUGINFO
  cout << "--- Accumulate into: " << first << " to " << last << endl;
  ShowDIMG(dimg);
#endif


  /* If image area is completely out of the range used, it can simply be sent to
     the next postprocessor.
  */

  if (ye < first) { if (DEBUGINFO) cout << "show all (above)\n"; d_sink->ShowMBRows(dimg); return NULL; }
  if (ys > last)  { if (DEBUGINFO) cout << "show all (below)\n"; d_sink->ShowMBRows(dimg); return NULL; }

  /* If image area completely contains the used range and it may be modified or no
     write-access is required, simply return the area. */

  if (ys<=first && ye>=last && (!d_modifyable_copy || dimg->m_may_modify) && !dimg->m_field_lines)
    {
#if DEBUGINFO
      cout << "return complete region\n";
#endif
      return dimg;
    }


  /* Save values out of dimg as we may leave it unchanged at the end of our function. */

  const int old_src_y_start = dimg->m_src_y_start;
  const int old_src_y_end   = dimg->m_src_y_end;
  const int old_dst_y_start = dimg->m_dst_y_start;
  const int old_may_modify  = dimg->m_may_modify;


  /* Forward the areas that are not used to the next postprocessor. */

  if (ys < first)
    {
      // show lines above drawing region

      int before = first-ys;
      //cout << "lines before: " << before << endl;


      // show unmodified lines

      dimg->m_src_y_end = dimg->m_src_y_start+before-1;

      if (dimg->m_field_lines && (before&1)==0) dimg->m_src_y_end--;  // make displayed line range tight

      Assert(d_sink);
      d_sink->ShowMBRows(dimg);
#if DEBUGINFO
      cout << "show unmodified above: ";  ShowDIMG(dimg);
#endif

      // remove already shown lines

      if (dimg->m_field_lines && (before&1)==1) before++;  // obey parity of accumulated lines

      dimg->m_src_y_end = old_src_y_end;
      dimg->m_src_y_start += before;
      dimg->m_dst_y_start += before;
    }

  if (ye>last)
    {
      // show lines below drawing region

      int after = ye-last;
      //cout << "lines after: " << after << endl;

      dimg->m_src_y_start = dimg->m_src_y_end-after+1;
      dimg->m_dst_y_start = ye-after+1;

      if (dimg->m_field_lines && (after&1)==0)
	{
	  dimg->m_src_y_start++;
	  dimg->m_dst_y_start++;
	}

      Assert(d_sink);
      d_sink->ShowMBRows(dimg);
#if DEBUGINFO
      cout << "show unmodified below: ";  ShowDIMG(dimg);
#endif

      // remove already shown lines

      if (dimg->m_field_lines && (after&1)==1) after++;  // obey parity of accumulated lines

      dimg->m_src_y_start = old_src_y_start;
      dimg->m_dst_y_start = old_dst_y_start;
      dimg->m_src_y_end -= after;
    }



  ImageParam_YUV param;
  dimg->m_image.GetParam(param);

  if (d_width != param.width ||
      d_height < d_line_available.AskSize())
    {
      ImageSpec_YUV spec; ((ImageParam_YUV&)spec)=param;
      spec.height = d_line_available.AskSize();
      d_dimg.m_image.Create(spec);

      d_width  = spec.width;
      d_height = spec.height;

      d_dimg.m_width      =dimg->m_width;
      d_dimg.m_height     =dimg->m_height;
      d_dimg.m_src_y_start=0;
      d_dimg.m_src_y_end  =d_line_available.AskSize()-1;
      d_dimg.m_dst_y_start=d_line_available.AskStartIdx();
      d_dimg.m_field_lines=false;
      d_dimg.m_may_modify =true;
    }

  d_dimg.m_picdata1   =dimg->m_picdata1;
  d_dimg.m_picdata2   =dimg->m_picdata2;
  d_dimg.m_pichdr1    =dimg->m_pichdr1;
  d_dimg.m_pichdr2    =dimg->m_pichdr2;
  d_dimg.m_timing     =dimg->m_timing;

  int lineskip = (dimg->m_field_lines ? 2 : 1);
  int y0 = -dimg->m_src_y_start+dimg->m_dst_y_start-first;

  bool* line = d_line_available.Data();

#if DEBUGINFO
  cout << "Copy from " << dimg->m_src_y_start << " to " << dimg->m_src_y_end << endl;
  cout << "y0: " << y0 << endl;
#endif

  // accumulate luminance data (and 4:2:2 or 4:4:4 chrominance data)

  int chromawidth = param.GetChromaWidth();

  for (int y=0; ;y+=lineskip)
    {
      int src_y = dimg->m_src_y_start+y;
      int dst_y = dimg->m_dst_y_start+y;
      int dat_y = dimg->m_dst_y_start-first+y;

      if (src_y > dimg->m_src_y_end)
	break;

      // cout << "copy line " << y << " to " << y+y0 << endl;
      memcpy(d_dimg.m_image.AskFrameY()[dat_y],
	     dimg->m_image.AskFrameY()[src_y],d_width);
      line[dst_y]=true;

      if (param.chroma==Chroma422 || param.chroma==Chroma444)
	{
	  memcpy(d_dimg.m_image.AskFrameU()[dat_y],
		 dimg->m_image.AskFrameU()[src_y],chromawidth);
	  memcpy(d_dimg.m_image.AskFrameV()[dat_y],
		 dimg->m_image.AskFrameV()[src_y],chromawidth);
	}
    }

  // accumulate 4:2:0 chrominance data

  if (param.chroma==Chroma420)
    {
      int parity = (dimg->m_src_y_start&1);

      for (int y=0; ;y+=lineskip)
	{
	  int src_y,dst_y,dat_y;

	  if (dimg->m_field_lines)
	    {
	      src_y = ((dimg->m_src_y_start/2)&~1) +parity+y;
	      dst_y = ((dimg->m_dst_y_start/2)&~1) +parity+y;
	      dat_y = (((dimg->m_dst_y_start-first)/2)&~1) +parity+y;
	    }
	  else
	    {
	      src_y = dimg->m_src_y_start/2+y;
	      dst_y = dimg->m_dst_y_start/2+y;
	      dat_y = (dimg->m_dst_y_start-first)/2+y;
	    }

	  if (src_y > dimg->m_src_y_end/2)
	    break;

	  memcpy(d_dimg.m_image.AskFrameU()[dat_y],
		 dimg->m_image.AskFrameU()[src_y],d_width/2);
	  memcpy(d_dimg.m_image.AskFrameV()[dat_y],
		 dimg->m_image.AskFrameV()[src_y],d_width/2);
	}
    }


  // restore old values into dimg

  dimg->m_src_y_start = old_src_y_start;
  dimg->m_src_y_end   = old_src_y_end;
  dimg->m_dst_y_start = old_dst_y_start;
  dimg->m_may_modify  = old_may_modify;


  for (int i=last;i>=first;i--)
    if (line[i]==false)
      {
	if (DEBUGINFO) cout << "line " << i << " is still missing...\n";
	return NULL;
      }

  if (DEBUGINFO) cout << "image data range is complete..!\n";

  return &d_dimg;
}


template class Array<bool>;
#include "libvideogfx/containers/array.cc"
