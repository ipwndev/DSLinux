#include "qlineedit.h"

static void Edit_Callback(Fl_Widget* o, void * pThis) 
{

  ((QLineEdit*)pThis)->_callback();

}


void QLineEdit::_callback()
{
  emit textChanged(m_pInput->value());
}


QLineEdit::QLineEdit(QWidget * parent, const char * name) : QWidget(parent, name)
{ 
  QWidget::setMinimumSize(40,20);
  m_pInput = new Fl_Input(0,0,40,20,"Text");
  setWidget(m_pInput);
  m_pInput->callback((Fl_Callback*)Edit_Callback,this);
  m_pInput->when(FL_WHEN_ENTER_KEY | FL_WHEN_CHANGED);
}

#include "qlineedit.moc"
