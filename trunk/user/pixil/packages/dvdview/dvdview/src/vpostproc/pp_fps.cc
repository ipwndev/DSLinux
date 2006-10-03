
#include "vpostproc/pp_fps.hh"
#include "libvideogfx/graphics/draw/draw.hh"

#include <sys/time.h>
#include <unistd.h>

#include <iostream.h>

int ac_s = 0*2;
int ac_e = ac_s+15;

static char* fps_text = 
"*****  ****    ***\n"
"*      *   *  *   *\n"
"*      *   *  *      * \n"
"***    ****    ***\n"
"*      *          *  *\n"
"*      *      *   *\n"
"*      *       ***";

static char* num_text[11] = {
  " *** \n"
  "*   *\n"
  "*   *\n"
  "*   *\n"
  "*   *\n"
  "*   *\n"
  " *** \n"
  ,
  "  ** \n"
  " * * \n"
  "*  * \n"
  "   * \n"
  "   * \n"
  "   * \n"
  "   * \n"
  ,
  " *** \n"
  "*   *\n"
  "*   *\n"
  "   * \n"
  "  *  \n"
  " *   \n"
  "*****\n"
  ,
  " *** \n"
  "*   *\n"
  "    *\n"
  "  ** \n"
  "    *\n"
  "*   *\n"
  " *** \n"
  ,
  "  *  \n"
  " *   \n"
  "*  * \n"
  "*****\n"
  "   * \n"
  "   * \n"
  "   * \n"
  ,
  "*****\n"
  "*    \n"
  "*    \n"
  "**** \n"
  "    *\n"
  "*   *\n"
  " *** \n"
  ,
  " *** \n"
  "*   *\n"
  "*    \n"
  "**** \n"
  "*   *\n"
  "*   *\n"
  " *** \n"
  ,
  "*****\n"
  "    *\n"
  "   * \n"
  " ****\n"
  "  *  \n"
  "  *  \n"
  "  *  \n"
  ,
  " *** \n"
  "*   *\n"
  "*   *\n"
  " *** \n"
  "*   *\n"
  "*   *\n"
  " *** \n"
  ,
  " *** \n"
  "*   *\n"
  "*   *\n"
  " ****\n"
  "    *\n"
  "*   *\n"
  " *** \n"
  ,
  "     \n"
  "     \n"
  "     \n"
  "     \n"
  "     \n"
  " **  \n"
  " **  \n" };

VideoPostprocessor_FramesPerSec::VideoPostprocessor_FramesPerSec(int len,int period)
{
  dataval = new timeval[len];
  nDataval=len;
  idx_in=0;

  for (int i=0;i<len;i++)
    gettimeofday(&dataval[i],NULL);

  assert(period>=1);
  update_cnt=0;
  update_period=period;
}

VideoPostprocessor_FramesPerSec::~VideoPostprocessor_FramesPerSec()
{
  delete[] dataval;
}

inline bool VideoPostprocessor_FramesPerSec::NeedsPictureData(uint3 pictype) const
{
  assert(d_next);
  return d_next->NeedsPictureData(pictype);
}

bool VideoPostprocessor_FramesPerSec::NeedsMBData(uint3 pictype) const
{
  assert(d_next);
  return d_next->NeedsMBData(pictype);
}

extern void ShowDIMG(DecodedImageData* dimg);

int VideoPostprocessor_FramesPerSec::Draw(const char* text,int x0,int y0,
					  DecodedImageData* drawimg)
{
  Pixel*const* py = drawimg->m_image.AskFrameY();
  Pixel*const* pu = drawimg->m_image.AskFrameU();
  Pixel*const* pv = drawimg->m_image.AskFrameV();

  ImageParam_YUV param;
  drawimg->m_image.GetParam(param);

  int maxx=0;
  int dx=0,dy=0;
  const char* p = text;
  while (*p)
    {
      if (*p=='\n')
	{
	  dx=0; dy+=2;
	}
      else
	{
	  if (*p=='*')
	    {
	      //cout << y0+dy << "/" << y0+dy+1 << endl;
	      //ShowDIMG(drawimg);

	      Assert(drawimg->ContainsOutputLine(y0+dy));
	      Assert(drawimg->ContainsOutputLine(y0+dy+1));

	      int ypos = drawimg->GetIndexForOutputLine(y0+dy);

	      py[ypos  ][x0+dx]=py[ypos  ][x0+dx+1]=255;
	      py[ypos+1][x0+dx]=py[ypos+1][x0+dx+1]=255;

	      if (param.chroma==Chroma420)
		{
		  pu[(ypos)/2][(x0+dx)/2]=128;
		  pv[(ypos)/2][(x0+dx)/2]=128;
		}
	      else if (param.chroma==Chroma422)
		{
		  pu[ypos][(x0+dx)/2]=pu[ypos+1][(x0+dx)/2]=128;
		  pv[ypos][(x0+dx)/2]=pv[ypos+1][(x0+dx)/2]=128;
		}
	      else
		{
		  Assert(param.chroma==Chroma444);

		  pu[ypos][(x0+dx)  ]=pu[ypos+1][(x0+dx)  ]=128;
		  pu[ypos][(x0+dx)+1]=pu[ypos+1][(x0+dx)+1]=128;
		  pv[ypos][(x0+dx)  ]=pv[ypos+1][(x0+dx)  ]=128;
		  pv[ypos][(x0+dx)+1]=pv[ypos+1][(x0+dx)+1]=128;
		}
	    }
	  dx+=2;

	  if (dx>maxx) maxx=dx;
	}
      p++;
    }

  return x0+maxx;
}

void VideoPostprocessor_FramesPerSec::BeginPicture(const DecodedImageData* dimg)
{
  StartAccumulation(ac_s,ac_e,true);
  d_next->BeginPicture(dimg);
}

void VideoPostprocessor_FramesPerSec::ShowMBRows(DecodedImageData* dimg)
{
  long udiff;

  if (dimg->m_dst_y_start==0) // do it only once per frame
    {
      // calculate fps

      timeval* now = &dataval[idx_in];

      gettimeofday(&dataval[idx_in],NULL);
      idx_in++;
      if (idx_in==nDataval) idx_in=0;

      timeval* past = &dataval[idx_in];

      if (update_cnt==0)
	{
	  long sec_diff = now->tv_sec - past->tv_sec;
	  udiff = sec_diff * 1000000;
	  assert(sec_diff>=0);
	  udiff += now->tv_usec-past->tv_usec;

	  fps = ((nDataval-1)*100)*1000000/udiff;

	  update_cnt=update_period-1;
  //cout << "-> FPS: " << fps << "/100   " << udiff << endl;
	}
      else
	{
	  update_cnt--;
	}
    }

  DecodedImageData* drawimg = Accumulate(dimg);

  if (!drawimg)
    return;

  Assert(drawimg->m_may_modify);

  ImageParam_YUV param;
  drawimg->m_image.GetParam(param);

  int x0 = 4; //param.width-2*(25 /*fps:*/ + 5*7);
  int y0 = ac_s+2; //param.height-2*(10);

  x0 &= ~1;
  y0 &= ~1;

  // draw "FPS:"

  x0 = Draw(fps_text,x0,y0,drawimg);

  if (fps>=10000) x0=Draw(num_text[((int)(fps/10000))%10],x0,y0,drawimg); else x0 += 2*5; x0+=4;
  if (fps>= 1000) x0=Draw(num_text[((int)(fps/ 1000))%10],x0,y0,drawimg); else x0 += 2*5; x0+=4;
  x0=Draw(num_text[((int)(fps/100))%10],x0,y0,drawimg);
  x0=Draw(num_text[10],x0,y0,drawimg);
  x0=Draw(num_text[((int)(fps/10))%10],x0,y0,drawimg); x0+=4;
  x0=Draw(num_text[((int)(fps))%10],x0,y0,drawimg);

  d_next->ShowMBRows(drawimg);
}

void VideoPostprocessor_FramesPerSec::FinishedPicture()
{
  d_next->FinishedPicture();
}
