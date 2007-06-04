#ifndef __QPIXMAP_H
#define __QPIXMAP_H

#include "qstring.h"
#include "qpaintdevice.h"
#include "qpoint.h"
#include "qrect.h"
#include "qsize.h"
#include "qbitmap.h"
#include "qwmatrix.h"


class QPixmap : public QPaintDevice
{

 public:
  
  class QPixmap_ {
  public:
    QPixmap_(int w=0, int h=0);
    ~QPixmap_();
    void destroy();
    void create();
    
    int m_nWidth;
    int m_nHeight;
    int m_nDepth;
    QBitmap * m_pBitmap;
    
    int m_nPix_W;
    int m_nPix_H;
    
    unsigned long m_Pixmap;
    unsigned long m_Mask;
    unsigned long m_IM;
    
#ifdef _NANOX
    QString m_Filename;
#endif

    int m_nRefCount;

  };

 protected:
  static bool m_bInitialized;
  
  QPixmap_ * m_pPixmap;

 public:

  enum Optimization { DefaultOptim, NoOptim, MemoryOptim=NoOptim, 
		      NormalOptim, BestOptim };
  enum ColorMode { Auto,Color,Mono };
  
  QPixmap();
  QPixmap(const QPixmap & qp);
  QPixmap(int w, int h, int depth=-1, Optimization = DefaultOptim);

  ~QPixmap();

  bool isNull() const 
    { 
      return (m_pPixmap->m_IM==0); 
    }
  
  int depth() const { return m_pPixmap->m_nDepth; }

  void drawPixmap(const QPoint & point, const QRect & rect) const;

  int width() const { return m_pPixmap->m_nWidth; }
  int height() const { return m_pPixmap->m_nHeight; }

  bool loadFromData( const QByteArray & data, const char * format=0,
		     int conversion_flags=0);
  bool load ( const QString & fileName, 
	      const char * format=0, ColorMode mode=Auto );

  void resize(const QSize & qs) { }
  void resize(int width, int height) { }
  const QBitmap * mask() const { return m_pPixmap->m_pBitmap;}
  QPixmap xForm(const QWMatrix & matrix) const { return *this; }
};

#endif
