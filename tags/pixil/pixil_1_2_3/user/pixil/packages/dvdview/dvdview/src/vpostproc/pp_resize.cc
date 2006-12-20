
#include "vpostproc/pp_resize.hh"


VideoPostprocessor_Resize::~VideoPostprocessor_Resize()
{
  if (d_ltab_h) delete[] d_ltab_h;
  if (d_ltab_v) delete[] d_ltab_v;
  if (d_ctab_h) delete[] d_ctab_h;
  if (d_ctab_v) delete[] d_ctab_v;
}


void VideoPostprocessor_Resize::SetScaledSize(int w,int h)
{
  width=w;
  height=h;
  d_do_resize=true;

  if (d_ltab_h) delete[] d_ltab_h;
  if (d_ltab_v) delete[] d_ltab_v;
  if (d_ctab_h) delete[] d_ctab_h;
  if (d_ctab_v) delete[] d_ctab_v;

  d_ltab_h = NULL;
  d_ltab_v = NULL;
  d_ctab_h = NULL;
  d_ctab_v = NULL;

  tab_initialized=false;
}


void VideoPostprocessor_Resize::ShowMBRows(DecodedImageData* decimg)
{
  /* Only resize image, if resizing is enabled and the target size is different
     from the input size. */

  if (!d_do_resize || (decimg->m_width==width && decimg->m_height==height))
    {
      d_next->ShowMBRows(decimg);
      return;
    }

  // Accumulate image pieces until the complete image is complete.

  DecodedImageData* dimg = Accumulate(decimg);
  if (!dimg)
    { return; }


  // Get input image size

  ImageParam_YUV param;
  dimg->m_image.GetParam(param);


  // Create new image with new size

  ImageSpec_YUV spec;
  dimg->m_image.GetParam(spec);
  spec.width  = width;
  spec.height = height;
  newdimg.m_image.Create(spec);

  newdimg.m_width  = width;
  newdimg.m_height = height;

  newdimg.m_src_y_start=0;
  newdimg.m_src_y_end  =height-1;
  newdimg.m_dst_y_start=0;
  newdimg.m_field_lines=false;
  newdimg.m_may_modify =true;

  newdimg.m_timing = dimg->m_timing;


  int oldcw = param.GetChromaWidth();
  int oldch = param.GetChromaHeight();
  int newcw = spec.GetChromaWidth();
  int newch = spec.GetChromaHeight();


  // Fill resizing tabs if not done yet

  if (!tab_initialized)
    {
      d_ltab_h = new int[width];
      d_ltab_v = new int[height];
      d_ctab_h = new int[newcw];
      d_ctab_v = new int[newch];

      for (int x=0;x<width;x++)	 d_ltab_h[x] = (dimg->m_width-1)*x/(width-1);
      for (int y=0;y<height;y++) d_ltab_v[y] = (dimg->m_height-1)*y/(height-1);
      for (int x=0;x<newcw;x++)	 d_ctab_h[x] = (oldcw-1)*x/(newcw-1);
      for (int y=0;y<newch;y++)  d_ctab_v[y] = (oldch-1)*y/(newch-1);

      tab_initialized=true;
    }


  // Resize image by simple resampling

  Pixel*const* dpy = newdimg.m_image.AskFrameY();
  Pixel*const* dpu = newdimg.m_image.AskFrameU();
  Pixel*const* dpv = newdimg.m_image.AskFrameV();

  const Pixel*const* spy = dimg->m_image.AskFrameY_const();
  const Pixel*const* spu = dimg->m_image.AskFrameU_const();
  const Pixel*const* spv = dimg->m_image.AskFrameV_const();

  for (int y=0;y<height;y++)
    for (int x=0;x<width;x++)
      {
	dpy[y][x] = spy[d_ltab_v[y]][d_ltab_h[x]];
      }

  for (int y=0;y<newch;y++)
    for (int x=0;x<newcw;x++)
      {
	dpu[y][x] = spu[d_ctab_v[y]][d_ctab_h[x]];
	dpv[y][x] = spv[d_ctab_v[y]][d_ctab_h[x]];
      }


  // Forward image data to next postprocessor

  d_next->ShowMBRows(&newdimg);
}


void VideoPostprocessor_Resize::BeginPicture(const DecodedImageData* dimg)
{
  StartAccumulation(0,dimg->m_height-1,true);  // Accumulate complete picture

  d_next->BeginPicture(dimg);
}


void VideoPostprocessor_Resize::FinishedPicture()
{
  d_next->FinishedPicture();
}
