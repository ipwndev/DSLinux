/*
 *  colorspace.cc
 */

#include "colorspace.hh"


void InterpolateH_Tap4(const Pixel*const* src,Pixel*const* dst,int dst_width,int height)
{
  int src_width = (dst_width+1)/2;

  // Create a chroma-line buffer in the range [-1 .. cw]
  Pixel* line = new Pixel[src_width+2];
  Pixel* linep = &line[1];

  // Process interpolation filter
  for (int y=0;y<height;y++)
    {
      // copy line and duplicate pixels at the end
      for (int x=0;x<src_width;x++)
	linep[x] = src[y][x];
      linep[-1]        = linep[0];
      linep[src_width] = linep[src_width-1];

      // Apply filter (1 3 3 1)//4
      for (int x=0;x<src_width;x++)
	{
	  dst[y][2*x  ] = (linep[x]*3 + linep[x-1]+2)/4;
	  dst[y][2*x+1] = (linep[x]*3 + linep[x+1]+2)/4;
	}
    }

  delete[] line;
}


void InterpolateV_Tap4(const Pixel*const* src,Pixel*const* dst,int width,int dst_height)
{
  int src_height = (dst_height+1)/2;

  // Create a chroma-line buffer in the range [-1 .. cw]
  Pixel* line = new Pixel[src_height+2];
  Pixel* linep = &line[1];

  // Process interpolation filter
  for (int x=0;x<width;x++)
    {
      // copy line and duplicate pixels at the end
      for (int y=0;y<src_height;y++)
	linep[y] = src[y][x];
      linep[-1]         = linep[0];
      linep[src_height] = linep[src_height-1];

      // Apply filter (1 3 3 1)//4
      for (int y=0;y<src_height;y++)
	{
	  dst[2*y  ][x] = (linep[y]*3 + linep[y-1]+2)/4;
	  dst[2*y+1][x] = (linep[y]*3 + linep[y+1]+2)/4;
	}
    }

  delete[] line;
}




void Tr420To444_Duplicate(Image_YUV<Pixel>& img)
{
  ImageParam_YUV param;
  img.GetParam(param);

  assert(param.chroma==Chroma420);
  assert(!param.reduced_chroma_size);

  Pixel*const* up = img.AskFrameU();
  Pixel*const* vp = img.AskFrameV();

  for (int y=(param.height+1)/2-1;y>=0;y--)
    for (int x=(param.width+1)/2-1;x>=0;x--)
      {
	up[2*y  ][2*x  ] =
	up[2*y  ][2*x+1] =
	up[2*y+1][2*x  ] =
	up[2*y+1][2*x+1] = up[y][x];
	vp[2*y  ][2*x  ] =
	vp[2*y  ][2*x+1] =
	vp[2*y+1][2*x  ] =
	vp[2*y+1][2*x+1] = vp[y][x];
      }

  img.SetChromaFormat(Chroma444);
}


void Tr420To422_Duplicate(Image_YUV<Pixel>& img)
{
  ImageParam_YUV param;
  img.GetParam(param);

  assert(param.chroma==Chroma420);
  assert(!param.reduced_chroma_size);

  Pixel*const* up = img.AskFrameU();
  Pixel*const* vp = img.AskFrameV();

  for (int y=(param.height+1)/2-1;y>=0;y--)
    for (int x=0;x<param.width;x++)
      {
	up[2*y  ][x] =
	up[2*y+1][x] = up[y][x];
	vp[2*y  ][x] =
	vp[2*y+1][x] = vp[y][x];
      }

  img.SetChromaFormat(Chroma422);
}


void Tr422To444_Duplicate(Image_YUV<Pixel>& img)
{
  ImageParam_YUV param;
  img.GetParam(param);

  assert(param.chroma==Chroma422);
  assert(!param.reduced_chroma_size);

  Pixel*const* up = img.AskFrameU();
  Pixel*const* vp = img.AskFrameV();

  for (int y=0;y<param.height;y++)
    for (int x=(param.width+1)/2-1;x>=0;x--)
      {
	up[y][2*x  ] =
	up[y][2*x+1] = up[y][x];
	vp[y][2*x  ] =
	vp[y][2*x+1] = vp[y][x];
      }

  img.SetChromaFormat(Chroma444);
}


void Tr422To420_Subsample(Image_YUV<Pixel>& img)
{
  ImageParam_YUV param;
  img.GetParam(param);

  assert(param.chroma==Chroma422);

  Pixel*const* up = img.AskFrameU();
  Pixel*const* vp = img.AskFrameV();

  for (int y=0;y<(param.height+1)/2-1;y++)
    for (int x=0;x<param.width;x++)
      {
	up[y][x] = up[2*y][x];
	vp[y][x] = vp[2*y][x];
      }

  int ch = (param.height+1)/2;

  img.AskBitmap(Image<Pixel>::Bitmap_U).SetSize(param.width,ch);
  img.AskBitmap(Image<Pixel>::Bitmap_V).SetSize(param.width,ch);

  img.SetChromaFormat(Chroma420);
}


void Tr444To420_Subsample(Image_YUV<Pixel>& img)
{
  ImageParam_YUV param;
  img.GetParam(param);

  assert(param.chroma==Chroma444);

  Pixel*const* up = img.AskFrameU();
  Pixel*const* vp = img.AskFrameV();

  for (int y=0;y<(param.height+1)/2-1;y++)
    for (int x=0;x<(param.width+1)/2-1;x++)
      {
	up[y][x] = up[2*y][2*x];
	vp[y][x] = vp[2*y][2*x];
      }

  int cw = (param.width +1)/2;
  int ch = (param.height+1)/2;

  img.AskBitmap(Image<Pixel>::Bitmap_U).SetSize(cw,ch);
  img.AskBitmap(Image<Pixel>::Bitmap_V).SetSize(cw,ch);

  img.SetChromaFormat(Chroma420);
}

void Tr444To422_Subsample(Image_YUV<Pixel>& img)
{
  ImageParam_YUV param;
  img.GetParam(param);

  assert(param.chroma==Chroma444);

  Pixel*const* up = img.AskFrameU();
  Pixel*const* vp = img.AskFrameV();

  for (int y=0;y<param.height;y++)
    for (int x=0;x<(param.width+1)/2-1;x++)
      {
	up[y][x] = up[y][2*x];
	vp[y][x] = vp[y][2*x];
      }

  int cw = (param.width+1)/2;

  img.AskBitmap(Image<Pixel>::Bitmap_U).SetSize(cw,param.height);
  img.AskBitmap(Image<Pixel>::Bitmap_V).SetSize(cw,param.height);

  img.SetChromaFormat(Chroma422);
}




void Tr422To444_Tap4(Image_YUV<Pixel>& img)
{
  ImageParam_YUV param;
  img.GetParam(param);

  // Check that we have enough memory space available.
  assert(!param.reduced_chroma_size);
  assert(param.chroma==Chroma422);

  // Process interpolation filter on U and V components.
  Pixel*const* up = img.AskFrameU();
  Pixel*const* vp = img.AskFrameV();

  InterpolateH_Tap4(up,up,param.width,param.height);
  InterpolateH_Tap4(vp,vp,param.width,param.height);

  img.SetChromaFormat(Chroma444);
}



inline int Clip(int x) { if (x<0) return 0; if (x>255) return 255; return x; }

void YUV2RGB_444(const Image_YUV<Pixel>& src,Image_RGB<Pixel>& dst)
{
  ImageSpec spec;
  src.Image<Pixel>::GetParam(spec);

  dst.Create(spec);

  ImageParam_YUV param;
  src.GetParam(param);

  assert(param.chroma == Chroma444);

  Pixel*const* rd = dst.AskFrameR();
  Pixel*const* gd = dst.AskFrameG();
  Pixel*const* bd = dst.AskFrameB();

  const Pixel*const* ys = src.AskFrameY_const();
  const Pixel*const* us = src.AskFrameU_const();
  const Pixel*const* vs = src.AskFrameV_const();

  for (int y=0;y<param.height;y++)
    for (int x=0;x<param.width;x++)
      {
	int yy = ((int)ys[y][x]) -16;
	int u  = ((int)us[y][x]) -128;
	int v  = ((int)vs[y][x]) -128;

	rd[y][x] = Clip((         409*v + 298*yy)>>8);
	gd[y][x] = Clip((-100*u - 208*v + 298*yy)>>8);
	bd[y][x] = Clip(( 516*u         + 298*yy)>>8);
      }
}

void YUV2RGB_422(const Image_YUV<Pixel>& src,Image_RGB<Pixel>& dst)
{
  ImageSpec spec;
  src.Image<Pixel>::GetParam(spec);

  dst.Create(spec);

  ImageParam_YUV param;
  src.GetParam(param);

  assert(param.chroma == Chroma422);

  Pixel*const* rd = dst.AskFrameR();
  Pixel*const* gd = dst.AskFrameG();
  Pixel*const* bd = dst.AskFrameB();

  const Pixel*const* ys = src.AskFrameY_const();
  const Pixel*const* us = src.AskFrameU_const();
  const Pixel*const* vs = src.AskFrameV_const();

  for (int y=0;y<param.height;y++)
    for (int x=0;x<param.width;x++)
      {
	int yy = ((int)ys[y][x]) -16;
	int u  = ((int)us[y][x/2]) -128;
	int v  = ((int)vs[y][x/2]) -128;

	rd[y][x] = Clip((         409*v + 298*yy)>>8);
	gd[y][x] = Clip((-100*u - 208*v + 298*yy)>>8);
	bd[y][x] = Clip(( 516*u         + 298*yy)>>8);
      }
}


void YUV2RGB_420(const Image_YUV<Pixel>& src,Image_RGB<Pixel>& dst)
{
  ImageSpec spec;
  src.Image<Pixel>::GetParam(spec);

  dst.Create(spec);

  ImageParam_YUV param;
  src.GetParam(param);

  assert(param.chroma == Chroma420);

  Pixel*const* rd = dst.AskFrameR();
  Pixel*const* gd = dst.AskFrameG();
  Pixel*const* bd = dst.AskFrameB();

  const Pixel*const* ys = src.AskFrameY_const();
  const Pixel*const* us = src.AskFrameU_const();
  const Pixel*const* vs = src.AskFrameV_const();

  for (int y=0;y<param.height;y++)
    for (int x=0;x<param.width;x++)
      {
	int yy = ((int)ys[y][x]) -16;
	int u  = ((int)us[y/2][x/2]) -128;
	int v  = ((int)vs[y/2][x/2]) -128;

	rd[y][x] = Clip((         409*v + 298*yy)>>8);
	gd[y][x] = Clip((-100*u - 208*v + 298*yy)>>8);
	bd[y][x] = Clip(( 516*u         + 298*yy)>>8);
      }
}


void RGB2YUV_444(const Image_RGB<Pixel>& src,Image_YUV<Pixel>& dst)
{
  ImageSpec_YUV spec;
  src.Image<Pixel>::GetParam(spec);
  spec.chroma=Chroma444;

  dst.Create(spec);

  ImageParam param;
  src.GetParam(param);

  const Pixel*const* rs = src.AskFrameR_const();
  const Pixel*const* gs = src.AskFrameG_const();
  const Pixel*const* bs = src.AskFrameB_const();

  Pixel*const* yd = dst.AskFrameY();
  Pixel*const* ud = dst.AskFrameU();
  Pixel*const* vd = dst.AskFrameV();

  for (int y=0;y<param.height;y++)
    for (int x=0;x<param.width;x++)
      {
	int r = ((int)rs[y][x]);
	int g = ((int)gs[y][x]);
	int b = ((int)bs[y][x]);

	yd[y][x] = Clip((( 65*r +  129*g +  24*b)>>8)+16);
	ud[y][x] = Clip(((-37*r +  -74*g + 112*b)>>8)+128);
	vd[y][x] = Clip(((112*r +  -93*g + -18*b)>>8)+128);
      }
}


void TransferBitmapsRGB2YUV(Image_RGB<Pixel>& src,Image_YUV<Pixel>& dst)
{
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Y     , src.AskBitmap(Image<Pixel>::Bitmap_Red  ) );
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_U     , src.AskBitmap(Image<Pixel>::Bitmap_Green) );
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_V     , src.AskBitmap(Image<Pixel>::Bitmap_Blue ) );
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Alpha , src.AskBitmap(Image<Pixel>::Bitmap_Alpha) );
}

void TransferBitmapsYUV2RGB(Image_YUV<Pixel>& src,Image_RGB<Pixel>& dst)
{
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Red   , src.AskBitmap(Image<Pixel>::Bitmap_Y) );
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Green , src.AskBitmap(Image<Pixel>::Bitmap_U) );
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Blue  , src.AskBitmap(Image<Pixel>::Bitmap_V) );
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Alpha , src.AskBitmap(Image<Pixel>::Bitmap_Alpha) );
}

void YUV2RGB_444_Inplace(Image_YUV<Pixel>& src,Image_RGB<Pixel>& dst)
{
  // --- Transfer all YUV image bitmaps to the RGB image. ---

  // Transfer bitmaps

  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Red   , src.AskBitmap(Image<Pixel>::Bitmap_Y) );
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Green , src.AskBitmap(Image<Pixel>::Bitmap_U) );
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Blue  , src.AskBitmap(Image<Pixel>::Bitmap_V) );
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Alpha , src.AskBitmap(Image<Pixel>::Bitmap_Alpha) );

  // Adjust parameters of RGB image.

  ImageParam_YUV param;
  src.GetParam(param);
  assert(param.chroma == Chroma444);
  dst.SetParam(param);

  // Remove bitmaps from YUV image.

  Bitmap<Pixel> emptybm;
  src.ReplaceBitmap(Image<Pixel>::Bitmap_Y     , emptybm);
  src.ReplaceBitmap(Image<Pixel>::Bitmap_U     , emptybm);
  src.ReplaceBitmap(Image<Pixel>::Bitmap_V     , emptybm);
  src.ReplaceBitmap(Image<Pixel>::Bitmap_Alpha , emptybm);


  // --- Now do the conversion ---

  Pixel*const* rd = dst.AskFrameR();
  Pixel*const* gd = dst.AskFrameG();
  Pixel*const* bd = dst.AskFrameB();

  const Pixel*const* ys = rd;
  const Pixel*const* us = gd;
  const Pixel*const* vs = bd;

  for (int y=0;y<param.height;y++)
    for (int x=0;x<param.width;x++)
      {
	int yy = ((int)ys[y][x]) -16;
	int u  = ((int)us[y][x]) -128;
	int v  = ((int)vs[y][x]) -128;

	rd[y][x] = Clip((         409*v + 298*yy)>>8);
	gd[y][x] = Clip((-100*u - 208*v + 298*yy)>>8);
	bd[y][x] = Clip(( 516*u         + 298*yy)>>8);
      }
}


void Grey2RGB_Inplace(Image_YUV<Pixel>& src,Image_RGB<Pixel>& dst)
{
  // --- Transfer all YUV image bitmaps to the RGB image. ---

  // Transfer bitmaps. The trick is here that all RGB channels are set to the same bitmap.

  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Red   , src.AskBitmap(Image<Pixel>::Bitmap_Y) );
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Green , src.AskBitmap(Image<Pixel>::Bitmap_Y) );
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Blue  , src.AskBitmap(Image<Pixel>::Bitmap_Y) );
  dst.ReplaceBitmap(Image<Pixel>::Bitmap_Alpha , src.AskBitmap(Image<Pixel>::Bitmap_Alpha) );

  // Adjust parameters of RGB image.

  ImageParam_YUV param;
  src.GetParam(param);
  param.nocolor = true;
  dst.SetParam(param);


  // Remove bitmaps from YUV image.

  Bitmap<Pixel> emptybm;
  src.ReplaceBitmap(Image<Pixel>::Bitmap_Y     , emptybm);
  src.ReplaceBitmap(Image<Pixel>::Bitmap_U     , emptybm);
  src.ReplaceBitmap(Image<Pixel>::Bitmap_V     , emptybm);
  src.ReplaceBitmap(Image<Pixel>::Bitmap_Alpha , emptybm);
}


void TrTo420_DupSub(Image_YUV<Pixel>& img)
{
  ImageParam_YUV param;
  img.GetParam(param);

  switch (param.chroma)
    {
    case Chroma420: break;
    case Chroma422: Tr422To420_Subsample(img); break;
    case Chroma444: Tr444To420_Subsample(img); break;
    }
}


void TrTo422_DupSub(Image_YUV<Pixel>& img)
{
  ImageParam_YUV param;
  img.GetParam(param);

  switch (param.chroma)
    {
    case Chroma420: Tr420To422_Duplicate(img); break;
    case Chroma422: break;
    case Chroma444: Tr444To422_Subsample(img); break;
    }
}


void TrTo444_DupSub(Image_YUV<Pixel>& img)
{
  ImageParam_YUV param;
  img.GetParam(param);

  switch (param.chroma)
    {
    case Chroma420: Tr420To444_Duplicate(img); break;
    case Chroma422: Tr422To444_Duplicate(img); break;
    case Chroma444: break;
    }
}


void HSB2RGB(float h,float s,float br, float& red,float& green,float& blue)
{
  float hue = h;
  float saturation = s;
  float brightness = br;

  if (brightness == 0.0) { red   = green = blue  = 0.0;        return; }
  if (saturation == 0.0) { red   = green = blue  = brightness; return; }

  float offs;   // hue mod 1/6
  if (hue < 1.0/6)
    {   // red domain; green ascends
      offs = hue;
      red   = brightness;
      blue  = brightness * (1.0 - saturation);
      green = blue + (brightness - blue) * offs * 6;
    }
  else if (hue < 2.0/6)
    { // yellow domain; red descends
      offs = hue - 1.0/6;
      green = brightness;
      blue  = brightness * (1.0 - saturation);
      red   = green - (brightness - blue) * offs * 6;
    }
  else if (hue < 3.0/6)
    { // green domain; blue ascends
      offs = hue - 2.0/6;
      green = brightness;
      red   = brightness * (1.0 - saturation);
      blue  = red + (brightness - red) * offs * 6;
    }
  else if (hue < 4.0/6)
    { // cyan domain; green descends
      offs = hue - 3.0/6;
      blue  = brightness;
      red   = brightness * (1.0 - saturation);
      green = blue - (brightness - red) * offs * 6;
    }
  else if (hue < 5.0/6)
    { // blue domain; red ascends
      offs = hue - 4.0/6;
      blue  = brightness;
      green = brightness * (1.0 - saturation);
      red   = green + (brightness - green) * offs * 6;
    }
  else
    { // magenta domain; blue descends
      offs = hue - 5.0/6;
      red   = brightness;
      green = brightness * (1.0 - saturation);
      blue  = red - (brightness - green) * offs * 6;
    }
}


void RGB2YUV(uint8 r,uint8 g,uint8 b, uint8& y,uint8& u,uint8& v)
{
  y = Clip((( 65*r +  129*g +  24*b)>>8)+16);
  u = Clip(((-37*r +  -74*g + 112*b)>>8)+128);
  v = Clip(((112*r +  -93*g + -18*b)>>8)+128);
}
