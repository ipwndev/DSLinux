#ifndef __QPAINTER_H
#define __QPAINTER_H

#include "qpen.h"
#include "qfont.h"
#include "qfontinfo.h"
#include "qbrush.h"
#include "qpixmap.h"
#include "qrect.h"
#include "qwmatrix.h"
#include "qfontmetrics.h"

#include "Fl_Widget.H"
#include <iostream>

//#define ADJUSTX(_x) (m_Widget->x() + _x)
//#define ADJUSTY(_y) (m_Widget->y() + _y)

#define ADJUSTX(_x) (_x)
#define ADJUSTY(_y) (_y)

class QPainter
{
 protected:
  QPen m_Pen;
  QFont m_Font;
  QColor m_BGColor;
  QBrush m_Brush;
  QRect m_ClipRect;
  bool m_bClipping;

  FLTK_PARENT_WIDGET * m_Widget;

public:

  QPainter() { }
  QPainter(const QPaintDevice * pd) { }
  QPaintDevice * device() const { return 0; }
  
  bool begin(const QPaintDevice * pix);
  bool end();

  void drawEllipse(int x, int y, int w, int h);
  void drawEllipse(const QRect & rect);

  void fillRect(int x, int y, int w, int h, const QBrush & br);
  void drawPixmap(int x, int y, const QPixmap & pix);
  void drawPixmap(const QPoint & point, const QPixmap & pixmap, 
		  const QRect & rect);

  void drawText(int x, int y, const QString & str, int len=-1);
  
  const QPen & pen() const { return m_Pen; }
  void setPen(const QColor &  color) { m_Pen = QPen(color); }
  void setPen(const QPen & pen) { m_Pen = pen; }

  QFontMetrics fontMetrics() const { return QFontMetrics(m_Font); }

  const QFont & font() const { return m_Font; }
  void setFont(const QFont & font) { m_Font = font; }

  void setBrush(const QBrush & brush) { m_Brush = brush; }

  void __SetBrushColor()
    {
      __SetColor(m_Brush.color());
    }

  void __SetBGColor()
    {
      __SetColor(m_BGColor);
    }

  void __SetPenColor()
    {
      __SetColor(m_Pen.color());
    }

  void __SetColor(const QColor & c)
    {
      fl_color(c.red(),
	       c.green(),
	       c.blue());
    }

  void setBackgroundColor(const QColor & c);


  void drawRect(int x, int y, int w, int h) 
    { 
      __SetBrushColor(); 
      fl_rect(ADJUSTX(x-1),ADJUSTY(y-1),w,h);
    }

  void qDrawShadePanel( int x, int y, int w, int h,
			const QColorGroup & cg, bool sunken,
			int lineWidth, const QBrush *fill);

  void qDrawShadeLine( int x1, int y1, int x2, int y2,
		       const QColorGroup &g, bool sunken = TRUE,
		       int lineWidth = 1, int midLineWidth = 0 );

  void drawRect(const QRect & rect) 
    { 
      drawRect(rect.x(),rect.y(),rect.width(), rect.height());
    }

  void scale(float sx, float sy) { }

  void eraseRect(int x, int y, int w, int h);
  void eraseRect(const QRect & rect);

  void setClipRect(int x, int y, int w, int h) { setClipRect(QRect(x,y,w,h)); }
  void setClipRect(const QRect & rect) { m_ClipRect = rect; cerr << "setClipRect called\n"; }
  void setClipping(bool enable) { m_bClipping = enable; }
};

#endif
