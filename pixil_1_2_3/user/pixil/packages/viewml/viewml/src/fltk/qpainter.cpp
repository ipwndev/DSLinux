#include "qpainter.h"
#include "qwidget.h"
#include "fl_draw.H"
#ifdef NANOX
#include <FL/n_x.h>
#else
#include <FL/x.H>
#endif

#include <FL/Fl.H>
#include "qfontmetrics.h"

void QPainter::drawText(int x, int y, const QString & str, int len) 
{
  __SetPenColor();
  font().SelectFont();
  if(len == -1) {
    fl_draw(str,ADJUSTX(x),ADJUSTY(y));
  } else {
    QString tmp(str);
    fl_draw(tmp.substr(0,len).c_str(),ADJUSTX(x),ADJUSTY(y));
  }

  // check for underline - davet
  if(font().m_bUnderline == true)
  {
    //    cerr << "Underline len = " << len << " str = " << str << "\n";

    int tlen;
    if(len == -1)
      tlen = (int)fl_width((const char *)str);
    else
      tlen = (int)fl_width((const char *)str,len);

    //    cerr << "Calculated len = " << tlen << "\n";

    fl_line(ADJUSTX(x),ADJUSTY(y+1),ADJUSTX(x+tlen),ADJUSTY(y+1));


  }

  // check for strikeout flag - davet
  if(font().m_bStrikeOut == true)
  {
    int tlen,yoff;
    if(len == -1)
      tlen = (int)fl_width((const char *)str);
    else
      tlen = (int)fl_width((const char *)str,len);
    yoff = (fl_height() - fl_descent()) / 2;
    fl_line(ADJUSTX(x),ADJUSTY(y-yoff),ADJUSTX(x+tlen),ADJUSTY(y-yoff));
  }
}
  
bool QPainter::begin(const QPaintDevice * pix)
{ 
  Fl_X *i = Fl_X::i(pix);

  if(!i) {
    pix->show();
    i = Fl_X::i(pix);
  }

  if(i && fl_window != i->other_xid && fl_window != i->xid)
    pix->make_current();

  m_Widget = (FLTK_PARENT_WIDGET*) pix;
  
#ifndef USE_FL_WIDGET

  if(m_Widget) {
    m_Widget->show();
  }

#endif
  return true; 
}

bool QPainter::end()
{
  return true;
}

void QPainter::qDrawShadeLine( int x1, int y1, int x2, int y2,
			       const QColorGroup &g, bool sunken,
			       int lineWidth, int midLineWidth )
{
  if(!sunken) {
    fl_color(g.light().red(),
	     g.light().green(),
	     g.light().blue());
  } else {
    fl_color(g.dark().red(),
	     g.dark().green(),
	     g.dark().blue());
  }
  
  fl_line(ADJUSTX(x1),ADJUSTY(y1),ADJUSTX(x2),ADJUSTY(y2));

  if(sunken) {
    fl_color(g.light().red(),
	     g.light().green(),
	     g.light().blue());
  } else {
    fl_color(g.dark().red(),
	     g.dark().green(),
	     g.dark().blue());
  }
  
  // for now assume that the line is horizontal .. yes I know this sucks
  fl_line(ADJUSTX(x1),ADJUSTY(y1+1),ADJUSTX(x2),ADJUSTY(y2+1));
  
}

void QPainter::qDrawShadePanel( int x, int y, int w, int h,
				const QColorGroup & cg, bool sunken,
				int lineWidth = 1, const QBrush *fill = 0 ) 
{
  if(!sunken) {
    fl_color(cg.light().red(),
	     cg.light().green(),
	     cg.light().blue());
  } else {
    fl_color(cg.dark().red(),
	     cg.dark().green(),
	     cg.dark().blue());
  }

  for(int i=0; i<lineWidth; i++) {
    fl_line(ADJUSTX(x)+i,ADJUSTY(y)+i,ADJUSTX(x)+w-i,ADJUSTY(y)+i);
    fl_line(ADJUSTX(x)+i,ADJUSTY(y)+i,ADJUSTX(x)+i,ADJUSTY(y)+h-i);
    
  }

  if(sunken) {
    fl_color(cg.light().red(),
	     cg.light().green(),
	     cg.light().blue());
  } else {
    fl_color(cg.dark().red(),
	     cg.dark().green(),
	     cg.dark().blue());
  }

  for(int i=0; i<lineWidth; i++) {
    fl_line(ADJUSTX(x)+i,ADJUSTY(y)-i+h,ADJUSTX(x)+w-i,ADJUSTY(y)-i+h);
    fl_line(ADJUSTX(x)+w-i,ADJUSTY(y)+h-i,ADJUSTX(x)+w-i,ADJUSTY(y)+i);
  }

}

void QPainter::setBackgroundColor(const QColor & c)
{
  m_BGColor = c; 
}

void QPainter::fillRect(int x, int y, int w, int h, const QBrush & br)
{
  QWidget * p = (QWidget *)m_Widget;

  if(!p->inPaint())
    return;

  fl_rectf(ADJUSTX(x),ADJUSTY(y),w,h,
	   br.color().red(),
	   br.color().green(),
	   br.color().blue());

}

void QPainter::drawPixmap(int x, int y, const QPixmap & pix)
{
  QPoint tmp;

  tmp.setX(ADJUSTX(x));
  tmp.setY(ADJUSTY(y));
 
  pix.drawPixmap(tmp,QRect(0,0,pix.width(),pix.height()));

}

void QPainter::drawPixmap(const QPoint & point, const QPixmap & pixmap, 
			  const QRect & rect) 
{ 
  QPoint tmp;

  tmp.setX(ADJUSTX(point.x()));
  tmp.setY(ADJUSTY(point.y()));
 
  pixmap.drawPixmap(tmp,rect);
}

void QPainter::drawEllipse(int x, int y, int w, int h) 
{ 
  __SetPenColor();
  fl_pie(ADJUSTX(x),ADJUSTY(y),w,h,0,359);
}

void QPainter::drawEllipse(const QRect & rect)
{
  drawEllipse(rect.x(), rect.y(), rect.width(), rect.height());
}

void QPainter::eraseRect(int x, int y, int w, int h) 
{ 
  fillRect(x+1,y+1,w,h,m_BGColor);
}

void QPainter::eraseRect(const QRect & rect)
{
  eraseRect(rect.x(), rect.y(), rect.width(), rect.height());
}
