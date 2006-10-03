#include "qscrollbar.h"

static void Scroll_Callback(Fl_Widget* o, void * pThis) {

  ((QScrollBar*)pThis)->_callback();

}


void QScrollBar::_callback()
{
  valueChanged(m_pScroll->value());
}


QScrollBar:: QScrollBar(int min, int max, int lineStep, int pageStep, int value,
			Orientation o, QWidget * parent, const char * name=0) : QWidget(parent,name)
{ 
  //  m_pScroll = new Fl_Scrollbar(0,0,1,1,"Text");
  m_pScroll = new NxScrollbar(0,0,1,1,"");
  setWidget(m_pScroll);
  if(o == Horizontal)
    m_pScroll->type(FL_HORIZONTAL);
  else
    m_pScroll->type(FL_VERTICAL);
  
  setRange(min,max);
  setSteps(lineStep,pageStep);
  setValue(value);
  m_pScroll->callback((Fl_Callback*)Scroll_Callback,this);
}

void QScrollBar::setValue(int value) 
{ 

  if(m_pScroll->type() == FL_HORIZONTAL) {
  
    m_pScroll->value(value,
		     (int)((m_nMax - m_nMin) * ((float)w() / ((float)w() + 
							     (float)(m_nMax - m_nMin)))),
		     m_nMin,m_nMax); 
  } else {

    m_pScroll->value(value,

		     (int)((m_nMax - m_nMin) * ((float)h() / ((float)h() + 
							     (float)(m_nMax - m_nMin)))),
		     m_nMin,m_nMax); 
  }
  m_pScroll->range(m_nMin,m_nMax);
}


void QScrollBar::resize(int _x, int _y, int _w, int _h)
{
  QWidget::resize(_x,_y,_w,_h);
  setValue(value());
}

#include "qscrollbar.moc"
