
#include "libvideogfx/postscript/image2ps.hh"


void Image2Postscript::DrawImage(const Image_YUV<Pixel>& img)
{
  ImageParam param;
  img.Image<Pixel>::GetParam(param);

  const int w = param.width;
  const int h = param.height;

  assert(d_ostr);

  const Pixel*const* yy = img.AskFrameY_const();


  // write postscript commands to display image

  *d_ostr << "/oldstate save def\n"
	  << "/picstr " << w << " string def\n"
	  << d_area.xoffs*cm2pts << " " << (d_area.yoffs-d_area.height)*cm2pts  << " translate\n"
	  << d_area.width*cm2pts << " " << d_area.height*cm2pts << " scale\n"
	  << w << " " << h << " 8\n"
	  << "[" << w << " 0 0 -" << h << " 0 " << h << "]\n"
	  << "{ currentfile ";

       if (d_enc==Image2Postscript::Hex)     *d_ostr << "/ASCIIHexDecode";
  else if (d_enc==Image2Postscript::Ascii85) *d_ostr << "/ASCII85Decode";
  else { assert(0); }

  *d_ostr << " filter picstr readstring pop }\n"
	  << "image\n";


  // write inline image data
      
  for (int y=0;y<h;y++)
    {
           if (d_enc==Image2Postscript::Hex)     { WriteLine_ASCIIHex(yy[y],w); *d_ostr << ">"  << endl; }
      else if (d_enc==Image2Postscript::Ascii85) { WriteLine_ASCII85 (yy[y],w); *d_ostr << "~>" << endl; }
      else { assert(0); }
    }


  // cleanup
  
  *d_ostr << "oldstate restore\n";
}


void Image2Postscript::DrawImage(const Image_RGB<Pixel>& img)
{
  ImageParam param;
  img.Image<Pixel>::GetParam(param);

  const int w = param.width;
  const int h = param.height;

  assert(d_ostr);

  const Pixel*const* rp = img.AskFrameR_const();
  const Pixel*const* gp = img.AskFrameG_const();
  const Pixel*const* bp = img.AskFrameB_const();

  
  // write postscript commands to display image

  *d_ostr << "/oldstate save def\n"
	  << "/picstr " << w*3 << " string def\n"
	  << d_area.xoffs*cm2pts << " " << (d_area.yoffs-d_area.height)*cm2pts  << " translate\n"
	  << d_area.width*cm2pts << " " << d_area.height*cm2pts << " scale\n"
	  << w << " " << h << " 8\n"
	  << "[" << w << " 0 0 -" << h << " 0 " << h << "]\n"
	  << "{ currentfile ";

       if (d_enc==Image2Postscript::Hex)     *d_ostr << "/ASCIIHexDecode";
  else if (d_enc==Image2Postscript::Ascii85) *d_ostr << "/ASCII85Decode";
  else { assert(0); }

  *d_ostr << " filter picstr readstring pop }\n"
	  << "false 3 colorimage\n";

       
  // write inline image data

  Pixel* line = new Pixel[w*3];
  
  for (int y=0;y<h;y++)
    {
      Pixel* p = line;
      const Pixel* r = rp[y];
      const Pixel* g = gp[y];
      const Pixel* b = bp[y];
      for (int x=0;x<w;x++)
	{
	  *p++ = *r++;
	  *p++ = *g++;
	  *p++ = *b++;
	}
      
           if (d_enc==Image2Postscript::Hex)     { WriteLine_ASCIIHex(line,w*3); *d_ostr << ">"  << endl; }
      else if (d_enc==Image2Postscript::Ascii85) { WriteLine_ASCII85 (line,w*3); *d_ostr << "~>" << endl; }
      else { assert(0); }
    }

  delete[] line;

      
  // cleanup

  *d_ostr << "oldstate restore\n";
}




#define MAXLINELEN 80

void Image2Postscript::WriteLine_ASCIIHex(const Pixel* pix,int len)
{
  if (len>MAXLINELEN/2)
    {
      WriteLine_ASCIIHex(pix              ,     MAXLINELEN/2);
      WriteLine_ASCIIHex(pix+MAXLINELEN/2 , len-MAXLINELEN/2);
      return;
    }

  char buf[MAXLINELEN+1];
  char* p=buf;
  static char tohex[16+1] = "0123456789abcdef";

  for (int i=0; i<len; i++,pix++)
    {
      *p++ = tohex[(*pix)>>4];
      *p++ = tohex[(*pix)&0x0F];
    }
  *p = 0;

  *d_ostr << buf << endl;
}



void Image2Postscript::WriteLine_ASCII85 (const Pixel* pix,int len)
{
  assert((len%4)==0);

  const int maxlen = MAXLINELEN/5*4;

  if (len>maxlen)
    {
      WriteLine_ASCII85(pix        ,     maxlen);
      WriteLine_ASCII85(pix+maxlen , len-maxlen);
      return;
    }

  char buf[MAXLINELEN+1];
  char* p=buf;

  for (int x=0;x<len;x+=4)
    {
      unsigned long n;
      n  = (pix[x  ]<<24);
      n |= (pix[x+1]<<16);
      n |= (pix[x+2]<< 8);
      n |=  pix[x+3];
	      
      char c1,c2,c3,c4,c5;
	      
      c5 = n%85; n/=85;
      c4 = n%85; n/=85;
      c3 = n%85; n/=85;
      c2 = n%85; n/=85;
      c1 = n;
      
      p[0] = c1 + 33;
      p[1] = c2 + 33;
      p[2] = c3 + 33;
      p[3] = c4 + 33;
      p[4] = c5 + 33;
      p+=5;
    }

  *p=0;
  *d_ostr << buf << endl;
}
