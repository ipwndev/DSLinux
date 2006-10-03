/*
 *  rw_ppm.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rw_ppm.hh"


Image_RGB<Pixel> ReadImage_PPM(istream& stream,Image_RGB<Pixel>* srcimg,ImageSpec* spec)
{
  char buffer[100+1];
  stream.getline(buffer,100);

  assert(strlen(buffer)==2);
  assert(buffer[0]=='P');

  bool greyscale;
  if (buffer[1]=='5')
    greyscale=true;
  else if (buffer[1]=='6')
    greyscale=false;
  else
    { assert(0); }


  int width,height,maxval;

  do
    {
      stream.getline(buffer,100);
    } while(buffer[0] == '#');
  sscanf(buffer,"%d %d",&width,&height);
  do
    {
      stream.getline(buffer,100);
    } while(buffer[0] == '#');
  maxval=atoi(buffer);
  assert(maxval==255);


  Image_RGB<Pixel> img;

  if (spec)
    {
      spec->width  = width;
      spec->height = height;
      img.Create(*spec);
    }
  else
    if (srcimg)
      {
	ImageParam param;
	srcimg->GetParam(param);
	assert(param.width  == width);
	assert(param.height == height);

	img = *srcimg;
      }
    else
      {
	ImageSpec spec;
	spec.width  = width;
	spec.height = height;
	img.Create(spec);
      }


  Pixel*const* r = img.AskFrameR();
  Pixel*const* g = img.AskFrameG();
  Pixel*const* b = img.AskFrameB();

  if (greyscale)
    {
      for (int y=0;y<height;y++)
	{
	  stream.read(r[y],width);
	  memcpy(g[y],r[y],width);
	  memcpy(b[y],r[y],width);
	}
    }
  else
    {
      uint8* linebuf = new uint8[width * 3];

      for (int y=0;y<height;y++)
	{
	  stream.read(linebuf,width*3);

	  uint8* p = linebuf;
	  uint8* rp = r[y];
	  uint8* gp = g[y];
	  uint8* bp = b[y];
	  for (int x=0;x<width;x++)
	    {
	      *rp++ = *p++;
	      *gp++ = *p++;
	      *bp++ = *p++;
	    }
	}

      delete[] linebuf;
    }

  return img;
}


Image_YUV<Pixel> ReadImage_PPM5(istream& stream,Image_YUV<Pixel>* srcimg,ImageSpec_YUV* spec)
{
  char buffer[100+1];
  stream.getline(buffer,100);

  assert(strlen(buffer)==2);
  assert(buffer[0]=='P');

  assert(buffer[1]=='5');

  int width,height,maxval;

  do
    {
      stream.getline(buffer,100);
    } while(buffer[0] == '#');
  sscanf(buffer,"%d %d",&width,&height);
  do
    {
      stream.getline(buffer,100);
    } while(buffer[0] == '#');
  maxval=atoi(buffer);
  assert(maxval==255);

  Image_YUV<Pixel> img;

  if (spec)
    {
      spec->width   = width;
      spec->height  = height;
      spec->nocolor = true;
      img.Create(*spec);
    }
  else
    if (srcimg)
      {
	ImageParam_YUV param;
	srcimg->GetParam(param);
	assert(param.width   == width);
	assert(param.height  == height);
	assert(param.nocolor == true);
	img = *srcimg;
      }
    else
      {
	ImageSpec_YUV spec;
	spec.width   = width;
	spec.height  = height;
	spec.nocolor = true;
	img.Create(spec);
      }


  Pixel*const* yy = img.AskFrameY();

  for (int y=0;y<height;y++)
    {
      stream.read(yy[y],width);
    }

  return img;
}


void WriteImage_PPM6(const Image_RGB<Pixel>& img,ostream& stream)
{
  ImageParam param;
  img.GetParam(param);

  const Pixel*const* R = img.AskFrameR_const();
  const Pixel*const* G = img.AskFrameG_const();
  const Pixel*const* B = img.AskFrameB_const();


  // Write file

  stream << "P6\n" << param.width << ' ' << param.height << "\n255\n";

  uint8* linebuf = new uint8[param.width*3];

  for (int y=0;y<param.height;y++)
    {
      uint8* p = linebuf;
      for (int x=0;x<param.width;x++)
	{
	  // This ugly code allows the compiler to schedule the
	  // commands to parallel pipelines.

	  uint8 a,b,c;
	  a = R[y][x];
	  b = G[y][x];
	  c = B[y][x];
	  *p++ = R[y][x];
	  *p++ = G[y][x];
	  *p++ = B[y][x];
	}

      stream.write(linebuf,param.width*3);
    }

  delete[] linebuf;
}


void WriteImage_PPM5(const Image_YUV<Pixel>& img,ostream& stream)
{
  ImageParam param;
  img.Image<Pixel>::GetParam(param);

  const Pixel*const* Y = img.AskFrameY_const();


  // Write file

  stream << "P5\n" << param.width << ' ' << param.height << "\n255\n";

  for (int y=0;y<param.height;y++)
    {
      stream.write(Y[y],param.width);
    }
}
