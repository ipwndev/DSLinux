#include "qpixmap.h"
#include "Fl_Image.H"
#include "fl_draw.H"

#ifndef _NANOX

#include <Imlib.h>
static ImlibData * m_id;
extern Display * fl_display;
extern Window fl_window;

#else

#include "unistd.h"
#include <nano-X.h>

extern int fl_display;
extern int fl_window;
extern GR_GC_ID fl_gc;

#endif

bool QPixmap::m_bInitialized;

QPixmap::QPixmap_::QPixmap_(int w, int h)
{
  m_nRefCount = 1;
  m_nWidth = w; m_nHeight = h; m_pBitmap = 0; m_IM=0; 
}

void QPixmap::QPixmap_::destroy()
{
  if(--m_nRefCount == 0)
    delete this;
}

void QPixmap::QPixmap_::create()
{
  m_nRefCount++;
}

QPixmap::QPixmap_::~QPixmap_()
{
#ifndef _NANOX
  if(m_id && m_IM)
    Imlib_destroy_image(m_id,m_IM);
#else
  if(m_IM)
    GrFreeImage(m_IM);

  //    Delete the file
  if(m_IM && m_Filename.length()) 
    unlink(m_Filename);
#endif

}

QPixmap::QPixmap(int w, int h, int depth=-1, Optimization = DefaultOptim) 
{ 
  if(parent()) {
    ((Fl_Group*)parent())->remove(this);
  }
  m_pPixmap = new QPixmap_(w,h);
}

QPixmap::QPixmap()
{ 
  if(parent()) {
    ((Fl_Group*)parent())->remove(this);
  }

  m_pPixmap = new QPixmap_();
}

QPixmap::QPixmap(const QPixmap & qp)
{
  if(qp.m_pPixmap) {
    m_pPixmap = qp.m_pPixmap;
    m_pPixmap->create();
  }
}


bool QPixmap::loadFromData( const QByteArray & data, const char * format,
			    int conversion_flags)
{
#ifdef _NANOX

  GR_IMAGE_INFO info;

  m_pPixmap->m_IM = GrLoadImageFromBuffer((void *) data.buffer(), (int) data.length(), 0);

  if (!m_pPixmap->m_IM) return(false);

  GrGetImageInfo(m_pPixmap->m_IM,&info);
  
  m_pPixmap->m_nPix_W = m_pPixmap->m_nWidth = info.width;
  m_pPixmap->m_nPix_H = m_pPixmap->m_nHeight = info.height;

  return(true);

#else

   // This is a bit of a hack
  char namebuf[1024];
  int fd;
  bool ret;

  strcpy(namebuf,"/tmp/viewmlXXXXXX");
  fd = mkstemp(namebuf);

  if(fd == -1) {
    cerr << "Couldn't make temporary file for QPixmap::loadFromData!\n";
    return false;
  }

  unsigned int loopc;

  char * buf = (char*)data.buffer();

  for(loopc=0;loopc<data.length();loopc++) {
    write(fd,buf + loopc,1);
  }

  close(fd);

  return(load(namebuf));
#endif
}

bool QPixmap::load ( const QString & fileName, 
		     const char * format, ColorMode mode )
{
  if(m_pPixmap->m_IM) {
    m_pPixmap->destroy();

    m_pPixmap = new QPixmap_();
  }
    
#ifndef _NANOX
  if(!m_bInitialized) {
    m_id = Imlib_init(fl_display);
    if(!m_id)
      return false;
    m_bInitialized = 1;
  }

  ImlibImage * im;
  im=Imlib_load_image(m_id,(char *)fileName);
  
  m_pPixmap->m_IM = im;

  if(im==0) {
    cerr << "Errk! Failed to load QPixmap " << fileName << "\n";
    return false;
  }
  m_pPixmap->m_nPix_W = m_pPixmap->m_nWidth = im->rgb_width;
  m_pPixmap->m_nPix_H = m_pPixmap->m_nHeight = im->rgb_height;

  // Remove old image if there is one
  if(m_pPixmap->m_Pixmap) {
    Imlib_free_pixmap(m_id,(Pixmap)m_pPixmap->m_Pixmap);
  }
  Imlib_render(m_id,im,m_pPixmap->m_nPix_W,m_pPixmap->m_nPix_H);
  
  m_pPixmap->m_Pixmap = (unsigned long)Imlib_move_image(m_id,im);
  m_pPixmap->m_Mask = (unsigned long)Imlib_move_mask(m_id,im);

#else

#ifdef NOTUSED

  /* Legacy code, not required for the latest version of microwindows */
  GR_IMAGE_INFO info;


  FILE * ftmp = fopen(fileName,"r");

  if(!ftmp) {
    return false;
  }

  fclose(ftmp);

  m_pPixmap->m_IM = GrLoadImageFromFile(fileName,0);
  
  GrGetImageInfo(m_pPixmap->m_IM,&info);
  
  m_pPixmap->m_nPix_W = m_pPixmap->m_nWidth = info.width;
  m_pPixmap->m_nPix_H = m_pPixmap->m_nHeight = info.height;

#endif
#endif

  return true;
}

QPixmap::~QPixmap()
{
  m_pPixmap->destroy();
}

void QPixmap::drawPixmap(const QPoint & point, const QRect & rect) const
{
  int x = point.x()-1;
  int y = point.y()-1;

#ifndef _NANOX
  Imlib_paste_image(m_id, m_pPixmap->m_IM, fl_window,
		    x,y,rect.width(),rect.height());
#else
  
  if(m_pPixmap->m_IM) {
    GrDrawImageToFit(fl_window, fl_gc, x, y, rect.width(), rect.height(), m_pPixmap->m_IM);
  }
#endif


}
