/*
 *  read_yuv.cc
 */

#include "read_yuv.hh"


FileReader_YUV1::FileReader_YUV1()
  : d_yuvstr(NULL), d_alphastr(NULL),
    d_greyscale_input(false),
    d_interleavedUV(false),
    d_initialized(false)
{
}



void FileReader_YUV1::Init()
{
  if (d_initialized)
    return;


  // Get file length to calculate number of frames in file.

  assert(d_yuvstr);

  d_yuvstr->seekg(0,ios::end);
  long length = d_yuvstr->tellg();
  d_yuvstr->seekg(0,ios::beg);


  // Calculate the size of one frame.

  switch (d_spec.chroma)
    {
    case Chroma420:  d_Framesize = d_spec.width * d_spec.height *3/2; break;
    case Chroma422:  d_Framesize = d_spec.width * d_spec.height *2;   break;
    case Chroma444:  d_Framesize = d_spec.width * d_spec.height *3;   break;
    default: assert(0); break;
    }

  d_nFrames = length/d_Framesize;
  if (d_nFrames * d_Framesize != length)
    {
      cerr << "Input file has strange file size, continuing anyway.\n";
      // TOOD: Put exceptionhandling here.
    }

  //cout << d_nFrames << " frames\n";

  d_nextFrame=0;
  d_initialized=true;
}



int FileReader_YUV1::AskNFrames() const
{
  if (d_initialized)
    return d_nFrames;

  (const_cast<FileReader_YUV1*>(this))->Init();
  return d_nFrames;
}



bool FileReader_YUV1::IsEOF() const
{
  (const_cast<FileReader_YUV1*>(this))->Init();
  return d_nextFrame >= d_nFrames;
}



void FileReader_YUV1::SkipToImage(int nr)
{
  if (!d_initialized)
    Init();

  assert(nr>=0);
  assert(nr<d_nFrames);

  d_yuvstr->seekg(nr*d_Framesize,ios::beg);
  if (d_alphastr) d_alphastr->seekg(nr * d_spec.width * d_spec.height , ios::beg);
  d_nextFrame=nr;
}



void FileReader_YUV1::ReadImage(Image_YUV<Pixel>& img)
{
  if (!d_initialized)
    Init();

  img.Create(d_spec);

  Pixel*const* yp = img.AskFrameY();
  Pixel*const* up = (d_greyscale_input ? NULL : img.AskFrameU());
  Pixel*const* vp = (d_greyscale_input ? NULL : img.AskFrameV());

  // Y
  for (int y=0;y<d_spec.height;y++)
    d_yuvstr->read(yp[y],d_spec.width);

  // color

  if (!d_greyscale_input)
    {
      int ch,cw;
      d_spec.GetChromaSizes(cw,ch);

      if (d_interleavedUV)
	{
	  uint8* buf = new uint8[cw*2];
	  
	  for (int y=0;y<ch;y++)
	    {
	      d_yuvstr->read(buf,cw*2);
	      
	      // demangle U,V components
	      
	      for (int x=0;x<cw;x++)
		{
		  up[y][x] = buf[x*2  ];
		  vp[y][x] = buf[x*2+1];
		}
	    }
	
	  delete[] buf;
	}
      else
	{
	  // U
	  for (int y=0;y<ch;y++)
	    d_yuvstr->read(up[y],cw);
	  
	  // V
	  for (int y=0;y<ch;y++)
	    d_yuvstr->read(vp[y],cw);
	}
    }

  // Alpha mask

  if (d_alphastr)
    {
      assert(!img.AskBitmap(Image<Pixel>::Bitmap_Alpha).IsEmpty());
      Pixel*const* aa = img.AskFrameA();

      for (int y=0;y<d_spec.height;y++)
	d_alphastr->read(aa[y],d_spec.width);
    }

  d_nextFrame++;
}

