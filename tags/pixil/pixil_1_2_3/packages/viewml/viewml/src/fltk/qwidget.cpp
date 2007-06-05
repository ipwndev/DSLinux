#include "qpoint.h"
#include "qwidget.h"

#include <Fl.H>
#ifdef _NANOX
#include <n_x.h>
#else
#include <x.H>
#endif
QPoint QMouseEvent::g;
//extern int fl_window;

QMetaObject *QWidget::metaObj = 0;

const char * QWidget::className() const
{
  return "QWidget";
}

void QWidget::resize(int w, int h) 
{ 
  resize(x(),y(),w,h);
}

void QWidget::repaint()
{
  redraw();
}

void QWidget::repaint(bool erase)
{
  redraw();
}

void QWidget::draw() 
{ 

  QRect r(0,0,w(),h());

  m_bInPaint = true;

  if(m_pWidget) {
    Fl_Window::draw();
  } else {
    QPaintEvent pe(r);
    paintEvent(&pe);
  }
 
  m_bInPaint = false;      
}

void QWidget::move(int x, int y)
{
  FLTK_PARENT_WIDGET::position(x,y);
}

void QWidget::repaint(int x, int y, int w, int h, bool erase)
{
  damage(0,x,y,w,h);
  redraw();
}

void QWidget::show()
{
  if(!visible()){
    FLTK_PARENT_WIDGET::show();
#ifdef _NANOX
    GrReparentWindow(fl_xid(this),fl_xid((Fl_Window*)parent()),x(),y());
#endif
  } else {
    if(!shown())
      FLTK_PARENT_WIDGET::show();
  }

  if(m_pWidget) {
    m_pWidget->show();
  }
}

void QWidget::hide()
{
  FLTK_PARENT_WIDGET::hide();
  if(m_pWidget) {
    m_pWidget->hide();
  }
}

void QWidget::setWidget(Fl_Widget * w)
{
  m_pWidget = w;
  Fl_Group::add(w);
}

// CRH
Fl_Widget *QWidget::getWidget()
{
	return m_pWidget;
}

void QWidget::resize(int _x, int _y, int _w, int _h)
{
  QSize old_size(width(), height());
  QSize new_size(_w,_h);
  QResizeEvent q(new_size,old_size);
  FLTK_PARENT_WIDGET::resize(_x,_y,_w,_h);  
  resizeEvent(&q);

  if(m_pWidget) {
    m_pWidget->size(_w,_h);
  }
}

void QWidget::setGeometry(int _x, int _y, int _w, int _h)
{
  resize(_x,_y,_w,_h);
}

void QWidget::resizeEvent( QResizeEvent *)
{
  //  cerr << "QWidget::resizeEvent\n";
  
}

void QWidget::FillMouseEvent(QMouseEvent * e)
{
/*
  int x = parent() ? Fl::event_x() - parent()->x() : Fl::event_x();
  int y = parent() ? Fl::event_y() - parent()->y() : Fl::event_y();
*/

  int x = Fl::event_x();
  int y = Fl::event_y();

  int b = Fl::event_state();
  int b1 = 0;
  if(b == FL_BUTTON1)
    b1 = LeftButton;
  else if(b== FL_BUTTON2)
    b1 = RightButton;
  else
    b1 = MidButton;
  
  e->setButton(b1);
  e->setPos(QPoint(x,y));
  e->setGlobalPos(QPoint(Fl::event_x(), Fl::event_y()));
}


void QWidget::initMetaObject()
{

}

static int send(Fl_Widget* o, int event) {
  if (o->type() < FL_WINDOW) return o->handle(event);
  int save_x = Fl::e_x; Fl::e_x -= o->x();
  int save_y = Fl::e_y; Fl::e_y -= o->y();
  int ret = o->handle(event);
  Fl::e_y = save_y;
  Fl::e_x = save_x;
  return ret;
}

int QWidget::handle(int event)
{
  Fl_Widget*const* a = array();
  int i;
  Fl_Widget * o;

  int ret = 0;
  QMouseEvent e;

  //  if(m_pWidget)
  //    return Fl_Window::handle(event);
  
  switch(event) {
  case FL_PUSH:
    FillMouseEvent(&e);
    mousePressEvent(&e);
    ret = 0;
    break;
  case FL_MOVE:
    FillMouseEvent(&e);
    //    mouseMoveEvent(&e);
    ret = 0;
    break;
  case FL_RELEASE:
    FillMouseEvent(&e);
    mouseReleaseEvent(&e);
    for (i = children(); i--;) {
      o = a[i];
      if (o->takesevents() && Fl::event_inside(o)) {
	if (send(o,FL_RELEASE)) {
	  return 1;
	}
      }
    }
    ret = 0;
    break;
  }

  return Fl_Window::handle(event);

  return ret;
}
