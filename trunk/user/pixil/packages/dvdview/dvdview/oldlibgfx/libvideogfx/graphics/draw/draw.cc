/*
 *  draw.cc
 */

#include <string.h>

#include "draw.hh"


template <class T> void DrawPoint(Bitmap<T>& bm,int x,int y,T color)
{
  if (x<0 || y<0) return;
  if (x>=bm.AskWidth() || y>=bm.AskHeight()) return;

  bm.AskFrame()[y][x]=color;
}


template <class T> void DrawLineSlow(Bitmap<T>& bm,int x1,int y1,int x2,int y2,T color)
{
  int vx = x2-x1;
  int vy = y2-y1;

  DrawPoint(bm,x1,y1,color);

  if (abs(vx)>abs(vy))
    {
      for (int x=0;x!=vx;x+=sign(vx))
	DrawPoint(bm , x1+x , vy*x/vx+y1 , color);
    }
  else
    {
      for (int y=0;y!=vy;y+=sign(vy))
	DrawPoint(bm , x1+vx*y/vy , y1+y , color);
    }
}


template <class T> void DrawDottedLine(Bitmap<T>& bm,int x1,int y1,int x2,int y2,T color)
{
  int vx = x2-x1;
  int vy = y2-y1;
  DrawPoint(bm,x1,y1,color);

  if (abs(vx)>abs(vy))
    {
      for (int x=0;x!=vx;x+=4*sign(vx))
	DrawPoint(bm , x1+x , vy*x/vx+y1 , color);
    }
  else
    {
      for (int y=0;y!=vy;y+=4*sign(vy))
	DrawPoint(bm , x1+vx*y/vy , y1+y , color);
    }
}


template <class T> void DrawFilledRectangleHV(Bitmap<T>& bm,int x0,int y0,int x1,int y1,T color)
{
  T*const* p = bm.AskFrame();

  for (int y=y0;y<=y1;y++)
    for (int x=x0;x<=x1;x++)
      p[y][x] = color;
}


template <class T> void Clear(Bitmap<T>& bm,T color)
{
  bm.Hint_ContentsIsNotUsedAnymore();

  T*const* p = bm.AskFrame();
  
  if (sizeof(T)==1)
    {
      for (int y=0;y<bm.AskHeight();y++)
	memset(p[y],color,bm.AskWidth());
    }
  else
    {
      DrawFilledRectangleHV(bm,0,0,bm.AskWidth()-1,bm.AskHeight()-1,color);
    }
}


#if 0
void CopyInnerBorder(const Image<Pixel>& src,Image<Pixel>& dst,int hwidth,int vwidth)
{
  for (int i=0;i<4;i++)
    {
            Bitmap<Pixel>& dstbm = dst.AskBitmap      ((Image<Pixel>::BitmapChannel)i);
      const Bitmap<Pixel>& srcbm = src.AskBitmap_const((Image<Pixel>::BitmapChannel)i);


      // Skip empty bitmaps.

      assert(srcbm.IsEmpty() == dstbm.IsEmpty());

      if (srcbm.IsEmpty())
	continue;


      // Copy border.

      int w = srcbm.AskWidth();
      int h = srcbm.AskHeight();

      assert(dstbm.AskWidth()  == w);
      assert(dstbm.AskHeight() == h);

            Pixel*const* dp = dstbm.AskFrame();
      const Pixel*const* sp = srcbm.AskFrame_const();

      // Horizontal stripes.

      for (int x=0;x<w;x++)
	for (int y=0;x<vwidth;y++)
	  {
	    dp[    y][x]=sp[    y][x];
	    dp[h-1-y][x]=sp[h-1-y][x];
	  }

      // Vertical stripes.
      
      for (int y=vwidth;y<h-vwidth;y++)
	for (int x=0;x<hwidth;x++)
	  {
	    dp[y][    x]=sp[y][    x];
	    dp[y][w-1-x]=sp[y][w-1-x];
	  }
    }
}
#endif

#if 1
void EnhanceImageWithBorder(Image<Pixel>& img,int borderwidth,bool exactsize)
{
  for (int i=0;i<4;i++)
    {
      Bitmap<Pixel>& srcbm = img.AskBitmap((Image<Pixel>::BitmapChannel)i);
      Bitmap<Pixel>* dstbm;
      Bitmap<Pixel>  newbm;

      if (srcbm.IsEmpty())
	continue;

      int w = srcbm.AskWidth(), h = srcbm.AskHeight();

      bool replacebm;

      if ((!exactsize && srcbm.AskBorderWidth() >= borderwidth) ||
	  ( exactsize && srcbm.AskBorderWidth() == borderwidth))
	{
	  dstbm = &srcbm;
	  replacebm=false;
	}
      else
	{
	  newbm.Create(w,h,1,1,borderwidth);
	  dstbm = &newbm;
	  replacebm = true;
	}

      const Pixel*const* sp = srcbm.AskFrame_const();
            Pixel*const* dp = dstbm->AskFrame();

      for (int y=0;y<h;y++)
	for (int x=0;x<w;x++)
	  {
	    dp[y][x] = sp[y][x];
	  }

      for (int y=0;y<h;y++)
	for (int x=0;x<borderwidth;x++)
	  {
	    dp[y][-1-x]=dp[y][0];
	    dp[y][w+x] =dp[y][w-1];
	  }

      for (int x=-borderwidth;x<w+borderwidth;x++)
	for (int y=0;y<borderwidth;y++)
	  {
	    dp[-1-y][x]=dp[0][x];
	    dp[h+y][x] = dp[h-1][x];
	  }

      if (replacebm)
	img.ReplaceBitmap((Image<Pixel>::BitmapChannel)i,*dstbm);
    }
}
#endif



// This function draws a rectangle. To do so it calls four times the DrawLine(...) function 
template <class T> void DrawRectangle(Bitmap<T>& bm,int x1,int y1,int w, int h,T color)
{
T*const* p = bm.AskFrame();

 DrawLine(bm,x1,y1,x1+w,y1,color);
 DrawLine(bm,x1+w,y1,x1+w,y1+h,color);
 DrawLine(bm,x1+w,y1+h,x1,y1+h,color);
 DrawLine(bm,x1,y1+h,x1,y1,color);
}

// This function is a part of the DrawLine algorithm. Do not call this function directly
static bool Clipt(float denom,float num, float& tE, float& tL)
{
  float t;
  bool accept = true;

  if (denom > 0)
    {
      t = num/denom;
      if (t > tL)
	accept = false;
      else
	if (t > tE)
	  tE = t;
    }
  else if (denom < 0)
    {
      t = num/denom;
      if (t < tE)
	accept = false;
      else
	if (t < tL)
	  tL = t;
    }
  else
    if (num > 0)
      accept = false;

  return accept;
}

// This function is a part of the DrawLine algorithm. Do not call it dirctly
void ClipLine(int& x0,int& y0,int& x1, int& y1, int xMax, int yMax, bool& visible)
{

  float dx = (float)(x1 - x0);
  float dy = (float)(y1 - y0);
  float tE = 0.0;
  float tL = 1.0;
  visible = false;
    
  if (Clipt(dx,0-(x0),tE,tL))
    if (Clipt(-dx,x0-xMax+1,tE,tL))
      if (Clipt(dy,0-(y0),tE,tL))
	if (Clipt(-dy,y0-yMax+1,tE,tL))
	  {
	    visible = true;
	    if (tL < 1.0)
	      {
		x1 = (int)(x0 + tL * dx+.5);
		y1 = (int)(y0 + tL * dy+.5);
	      }
	    if (tE > 0.0)
	      {
		x0 =(int)( x0 + tE * dx+.5);
		y0 =(int)( y0 + tE * dy+.5);
	      }
	  }

  if (visible)
    {
      assert(x0>=0);
      assert(y0>=0);
      assert(x0<xMax);
      assert(y0<yMax);
    }
}


template <class Pel> ArrowPainter<Pel>::ArrowPainter()
{
  SetAlpha(30);
  len  =10;
  bothheads=false;
  color=255;
}

// This function draws a line and places a head on one (arrows == false) or both (arrows==true) sides of the line.
template <class T> void DrawArrow(Bitmap<T>& bm,int x0,int y0,int x1, int y1,float alpha,int l,T color,bool arrows = false)
{
  DrawLine(bm,x0,y0,x1,y1,color);     

  // zeichnen der Pfeilspitze
  int xa,ya,dxp,dyp;
  float norm;
      
  dxp = x1-x0;
  dyp = y1-y0;

  norm = sqrt(dxp*dxp+dyp*dyp);
      
  xa = (int)((cos(alpha)*dxp-sin(alpha)*dyp)*l/norm);
  ya = (int)((sin(alpha)*dxp+cos(alpha)*dyp)*l/norm);

  DrawLine(bm,x1,y1,x1-xa,y1-ya,color);     

  if (arrows == true)
    {
      DrawLine(bm,x0,y0,x0+xa,y0+ya,color);
    }

  xa = (int)(( cos(alpha)*dxp+sin(alpha)*dyp)*l/norm);
  ya = (int)((-sin(alpha)*dxp+cos(alpha)*dyp)*l/norm);

  DrawLine(bm,x1,y1,x1-xa,y1-ya,color);  

  if (arrows == true)
    {
      DrawLine(bm,x0,y0,x0+xa,y0+ya,color);
    }


}

// main function to draw a line very fast. Clipping is included, so don't think about it
template <class T> void DrawLine(Bitmap<T>& bm,int x0,int y0,int x1, int y1,T color)
{
  T*const* p = bm.AskFrame();
  bool visible = true;

  int xMax= bm.AskWidth();
  int yMax= bm.AskHeight();

  if ((x0<0) || (y0<0) || (x1<0) || (y1<0) || (x0>=xMax) || (y0>=yMax) || (x1>=xMax) || (y1>=yMax))
    {
      ClipLine(x0,y0,x1,y1,xMax,yMax,visible);
#if 0
      cout << "x0 = " << x0 << endl;
      cout << "y0 = " << y0 << endl;
      cout << "x1 = " << x1 << endl;
      cout << "y1 = " << y1 << endl;
#endif
    }

  if (visible)
    {
      if (abs(y1-y0)>abs(x1-x0))
	{
	  if (y1<y0) {  swap(x0,x1); swap(y0,y1); }

	  int xinc;
	  int dy = y1 - y0;
	  int dx = x1 - x0;

	  if (dx < 0)
	    {
	      xinc = -1;
	      dx = -dx;
	    }
	  else
	    xinc = 1;

	  int d = 2* dx - dy;
	  int incrE = 2 * dx;  // Increment used for move to E
	  int incrNE = 2 * (dx - dy);   // increment used for move to NE
	  int y = y0;
	  int x = x0;

	  while (y <= y1)
	    {
	  
	      p[y][x] = color;
     
	      if (d <= 0)   // Choose E
		{
		  d = d + incrE;
		  y++;
		}
	      else         // Choose NE
		{
		  d = d + incrNE;
		  y++;
		  x = x + xinc;
		}
	    }
	}
      else{
	if (x1<x0) {  swap(x0,x1); swap(y0,y1); }

	int yinc;
	int dx = x1 - x0;
	int dy = y1 - y0;

	if (dy < 0)
	  {
	    yinc = -1;
	    dy = -dy;
	  }
	else
	  yinc = 1;

	int d = 2* dy - dx;
	int incrE = 2 * dy;  // Increment used for move to E
	int incrNE = 2 * (dy - dx);   // increment used for move to NE
	int x = x0;
	int y = y0;

	while (x <= x1)
	  {
	  
	    p[y][x] = color;
     
	    if (d <= 0)   // Choose E
	      {
		d = d + incrE;
		x++;
	      }
	    else         // Choose NE
	      {
		d = d + incrNE;
		x++;
		y = y + yinc;
	      }
	  }
      }
    }
}

//Pointer is set to this function if the circle is full visible 
template <class T> static void CirclePoints_Direct(Bitmap<T>& bm,int x0,int y0,int dx,int dy,T color,bool fill=false)
{
  T*const* p = bm.AskFrame();

  static int xMax= bm.AskWidth();
  static int yMax= bm.AskHeight();

  int y0dy1=y0-dy;
  int y0dy2=y0+dy;
  int x0dx1=x0-dx;
  int x0dx2=x0+dx;

  int y0dx1=y0-dx;
  int y0dx2=y0+dx;
  int x0dy1=x0-dy;
  int x0dy2=x0+dy;

  
  p[y0dy1][x0dx1]=color;
  p[y0dy1][x0dx2]=color;
  p[y0+dy][x0-dx]=color;
  p[y0+dy][x0+dx]=color;

  p[y0-dx][x0-dy]=color;
  p[y0-dx][x0+dy]=color;
  p[y0+dx][x0-dy]=color;
  p[y0+dx][x0+dy]=color;
  if (fill == true)   // if fill is true, fill up the circle from the top to the bottom
    {
      DrawLine(bm,x0dx1,y0dy1,x0dx2,y0dy1,color);
      DrawLine(bm,x0dy1,y0dx1,x0dy2,y0dx1,color);
      DrawLine(bm,x0dy2,y0dx2,x0dy1,y0dx2,color);
      DrawLine(bm,x0dx2,y0dy2,x0dx1,y0dy2,color);      
    }

}

//Pointer is set to this function if the circle is not fully visible 
template <class T> static void CirclePoints_Save(Bitmap<T>& bm,int x0,int y0,int dx,int dy,T color,bool fill=false)
{
  T*const* p = bm.AskFrame();

  static int xMax= bm.AskWidth();
  static int yMax= bm.AskHeight();

  int y0dy1=y0-dy;
  int y0dy2=y0+dy;
  int x0dx1=x0-dx;
  int x0dx2=x0+dx;

  int y0dx1=y0-dx;
  int y0dx2=y0+dx;
  int x0dy1=x0-dy;
  int x0dy2=x0+dy;

  
  if ((y0dy1)>=0 && (x0dx1)>=0 && (y0dy1)<yMax && (x0dx1)<xMax)
    p[y0dy1][x0dx1]=color;
  if ((y0dy1)>=0 && (x0dx2)>=0 && (y0dy1)<yMax && (x0dx2)<xMax)
    p[y0dy1][x0dx2]=color;
  if ((y0dy2)>=0 && (x0dx1)>=0 && (y0dy2)<yMax && (x0dx1)<xMax)
    p[y0+dy][x0-dx]=color;
  if ((y0dy2)>=0 && (x0dx2)>=0 && (y0dy2)<yMax && (x0dx2)<xMax)
    p[y0+dy][x0+dx]=color;


  if ((y0dx1)>=0 && (x0dy1)>=0 && (y0dx1)<yMax && (x0dy1)<xMax)
    p[y0-dx][x0-dy]=color;
  if ((y0dx1)>=0 && (x0dy2)>=0 && (y0dx1)<yMax && (x0dy2)<xMax)
    p[y0-dx][x0+dy]=color;
  if ((y0dx2)>=0 && (x0dy1)>=0 && (y0dx2)<yMax && (x0dy1)<xMax)
    p[y0+dx][x0-dy]=color;
  if ((y0dx2)>=0 && (x0dy2)>=0 && (y0dx2)<yMax && (x0dy2)<xMax)
    p[y0+dx][x0+dy]=color;

  if (fill == true) // if fill is true, fill up the circle from the top to the bottom
    {
      DrawLine(bm,x0dx1,y0dy1,x0dx2,y0dy1,color);
      DrawLine(bm,x0dy1,y0dx1,x0dy2,y0dx1,color);
      DrawLine(bm,x0dy2,y0dx2,x0dy1,y0dx2,color);
      DrawLine(bm,x0dx2,y0dy2,x0dx1,y0dy2,color);      
    }
}

// main function to draw a circle
template <class T> void DrawCircle(Bitmap<T>& bm,int x0,int y0, int radius,T color,bool fill = false)
{
  int x,y,d;

  void (*drawpoints)(Bitmap<T>& bm,int x0,int y0,int dx,int dy,T color,bool fill =false);

  /* int octant[8];*/ /* -1: draussen
		     0: drin
		     1: halb drin/draussen */

  x=0;
  y=radius;
  d=1-radius;

  if (x0-radius>=0 && x0+radius<bm.AskWidth() &&
      y0-radius>=0 && y0+radius<bm.AskHeight())
    { drawpoints = CirclePoints_Direct; }
  else
    { drawpoints = CirclePoints_Save; }

  drawpoints(bm,x0,y0,x,y,color,fill);

  while (y>x)
    {
      if (d<0)
	{
	  d=d+2*x+3;
	  x++;
	}
      else
	{
	  d=d+2*(x-y)+5;
	  x++;
	  y--;
	}
      drawpoints(bm,x0,y0,x,y,color,fill);
    }
}

// this function draws an ellipse. Clipping is also included.
template <class T> void DrawEllipse(Bitmap<T>& bm,int xm,int ym, int a,int b,float angle,T color)
{
  T*const* p = bm.AskFrame();
  static int flag = 0;

  const float beta = angle*M_PI/180;

  const float cosb = cos(beta);
  const float sinb = sin(beta);
  static float alpha_step = M_PI/180;
  int x[3],y[3];

  for (float alpha=0;alpha<2*M_PI;alpha+=alpha_step)
    {

      float sina,cosa;

      cosa = cos(alpha)*a;
      sina = sin(alpha)*b;

      x[0] = (int)((cosa*cosb - sina*sinb)+0.5);
      y[0] = (int)((sinb*cosa + sina*cosb)+0.5);
      
      //      cout << x[0] << "," << y[0] << " = ";
      

      if (flag < 1 || (abs(x[0]-x[1])==1 && abs(y[0]-y[1])==1) || 
	  (abs(x[0]-x[1])==0 && abs(y[0]-y[1])==1) || 
	  (abs(x[0]-x[1])==1 && abs(y[0]-y[1])==0))
	{
	  if (flag == 0)
	    {
	      x[1]=x[0];
	      y[1]=y[0];
	      flag = 1;
	    }
	  if (flag == 1)
	    {
 	      x[2]=x[1];
	      y[2]=y[1];
	      x[1]=x[0];
	      y[1]=y[0];

	      flag = 2;
	    }
	  
	  if (!(abs(x[0]-x[2])==1 && abs(y[0]-y[2])==1))
	  {
	    
	    if (!(x[1]+xm<0 || y[1]+ym<0 || x[1]+xm>=bm.AskWidth() || y[1]+ym>=bm.AskHeight()))
	      p[x[1]+xm][y[1]+ym]=color;
    
	    x[2]=x[1];
	    y[2]=y[1];
	    
	    x[1]=x[0];
	    y[1]=y[0];

	    //	    cout << "draw!" << endl;
	  }
	  else
	    {
	    x[1]=x[0];
	    y[1]=y[0];
	    //	    cout << "cut!" << endl;
	    }
	}
      else if (abs(x[0]-x[1])==0 && abs(y[0]-y[1])==0)
	{
	  alpha -= alpha_step;
	  alpha_step += alpha_step;
	  //	  cout << "alpha_step up =" << alpha_step << endl;
	}
      else
	{
	  alpha -= alpha_step;
	  alpha_step *= 0.9;
	  //	  cout << "alpha_step down =" << alpha_step << endl;
	}
 

    }
  if (!(x[0]+xm<0 || y[0]+ym<0 || x[0]+xm>=bm.AskWidth() || y[0]+ym>=bm.AskHeight()))
    p[x[0]+xm][y[0]+ym]=color;
}



template void DrawRectangle(Bitmap<Pixel>& bm,int x1,int y1,int w, int h,Pixel color);
template void DrawArrow(Bitmap<Pixel>& bm,int x0,int y0,int x1, int y1,float alpha,int l,
			Pixel color,bool arrows);
template void DrawLine(Bitmap<Pixel>& bm,int x0,int y0,int x1, int y1,Pixel color);
template void DrawCircle(Bitmap<Pixel>& bm,int x0,int y0, int radius,Pixel color,bool);
template void DrawEllipse(Bitmap<Pixel>& bm,int xm,int ym, int a,int b,float angle,Pixel);
template class ArrowPainter<Pixel>;


template void DrawPoint     (Bitmap<Pixel>&,int x,int y,Pixel color);
template void DrawLineSlow  (Bitmap<Pixel>&,int x1,int y1,int x2,int y2,Pixel color);
template void DrawDottedLine(Bitmap<Pixel>&,int x1,int y1,int x2,int y2,Pixel color);
template void DrawFilledRectangleHV(Bitmap<Pixel>&,int x0,int y0,int x1,int y1,Pixel color);
template void Clear         (Bitmap<Pixel>&,Pixel color);

template void Clear         (Bitmap<bool>&,bool color);
