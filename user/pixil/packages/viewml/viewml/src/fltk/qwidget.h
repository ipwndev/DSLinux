#ifndef __QWIDGET_H
#define __QWIDGET_H

#include "fltk-qbase.h"
#include "qpaintdevice.h"
#include "qcursor.h"
#include "qpalette.h"
#include "qsize.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "Fl_Widget.H"
#include "fl_draw.H"
#include <iostream>
#include "qevent.h"

class QWidget : public QObject, public QPaintDevice
{
  Q_OBJECT
 protected:

  QCursor m_Cursor;
  QColor m_Color;
  QPalette m_Palette;
  
  QSize m_MinSize;
  QFont m_Font;
  QFontMetrics m_FontMetrics;
  bool m_bInPaint;
  bool m_bToss;


  Fl_Widget * m_pWidget;
  
 protected:
  virtual void keyPressEvent(QKeyEvent * e) { m_bToss = true; }

 public:
  QWidget() : QObject() { m_bInPaint = false; begin(); m_pWidget = 0; }
  QWidget(QWidget * _parent, const char * _name=0, WFlags f=0) :
    QObject()
    { 
      m_pWidget = 0;
      if(Fl_Group::parent())
	((Fl_Group*)parent())->remove(this);
      if(_parent) {
	_parent->add(this);
	setGeometry(0,0,_parent->width(),_parent->height());
	end();
      } else {
	begin();
      }
      m_bInPaint = false;
    }

  ~QWidget() 
    { 
      if(parent()) { 
	((Fl_Group*)parent())->remove(this); 
      } 

      if(m_pWidget) 
	delete m_pWidget; 
    }

  int height() const { return h(); }
  int width() const { return w(); }
  int x() const 
    {
/*
      if(parent())
	return parent()->x() - Fl_Widget::x();
      else
*/
	return FLTK_PARENT_WIDGET::x();
    }

  int y() const 
    { 
/*
      if(parent())
	return parent()->y() - Fl_Widget::y();
      else
*/
	return FLTK_PARENT_WIDGET::y();
    }

  virtual void paintEvent(QPaintEvent * _pe) { }

  virtual FL_EXPORT void draw();

  bool inPaint() const { return m_bInPaint; }

  virtual int handle(int event);

  bool isVisible() const { return Fl_Window::visible(); }
  virtual void hide();
  void raise() {  show(); }
  virtual void show();


  void setMinimumSize(int w, int h) { setMinimumSize(QSize(w,h)); } 
  void setMinimumSize(const QSize & size) { m_MinSize = size; }
  virtual void setCursor(const QCursor & cursor) { m_Cursor = cursor; }
  virtual void setFocusProxy(QWidget * w) { }
  virtual void setGeometry(int _x, int _y, int _w, int _h);

  virtual void setGeometry(const QRect & rect)
    {
      setGeometry(rect.x(), rect.y(),
		  rect.width(), rect.height());
    }

  QPoint mapToGlobal( const QPoint & point) const { return QPoint(1,1); }
  QPoint mapFromGlobal( const QPoint & point) const { return QPoint(1,1); }

  void repaint();
  void repaint(bool erase);
  void repaint(int x, int y, int w, int h, bool erase = TRUE);

  virtual void setMouseTracking(bool enable) { } 

  virtual void setBackgroundColor(const QColor & color) { m_Color = color; }
  virtual void move(int x, int y);
  void grabMouse() { }
  void releaseMouse() { }
  const QPalette & palette() { return m_Palette; }
  bool isUpdatesEnabled() const { return true; }
  virtual void resizeEvent( QResizeEvent *);

  virtual void resize(const QSize & size)
    { resize(size.width(), size.height()); }
  virtual void resize(int w, int h);
  virtual void resize(int _x, int _y, int _w, int _h);

  virtual QSize sizeHint() const { return m_MinSize; }
  void setFont(const QFont & font) { m_Font = font; }
  QFont font() const { return m_Font; }
  QFontMetrics fontMetrics() const { return m_FontMetrics; }

  virtual void mousePressEvent( QMouseEvent *) { }
  virtual void mouseMoveEvent( QMouseEvent * ) { }
  virtual void mouseReleaseEvent( QMouseEvent * ) { }
  void FillKeyEvent(QKeyEvent * e);
  void FillMouseEvent(QMouseEvent * e);

  void setWidget(Fl_Widget * w);
// CRH
  Fl_Widget *QWidget::getWidget();
};

#endif
