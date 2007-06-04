#include "qpushbutton.h"


static void Button_Callback(Fl_Widget* o, void * pThis) {

  ((QPushButton*)pThis)->_callback();

}


void QPushButton::_callback()
{
  emit clicked();
}

QPushButton::QPushButton(QWidget * parent, const char * name ) : QWidget(parent,name)
{ 
  QWidget::setMinimumSize(40,QPUSHBUTTON_HEIGHT);
  
  Fl_Button * b = new Fl_Button(0,0,40,QPUSHBUTTON_HEIGHT,"Text");
  Fl_Group::add(b);
  setWidget(b);
  b->callback((Fl_Callback*)Button_Callback,this);
}

// CRH
void QPushButton::setText(const QString & text) 
{ 
  m_Text = text; 
  m_pWidget->label(text); 
  m_pWidget->labelfont(m_Font.getFont());
  m_pWidget->labelsize(m_Font.size());
//int c = fl_color();
//  fl_color(m_Color.red(), m_Color.green(), m_Color.blue());
//  fl_color(255, 0, 0);
//  m_pWidget->color(fl_color());
//cerr << c << " " << (int)fl_color() << endl;
  QFontMetrics fm(m_Font);
  QWidget::setMinimumSize(fm.width(text) + QPUSHBUTTON_PADDING,
							fm.height() + QPUSHBUTTON_VPADDING);
  QWidget::resize(sizeHint());
}

#include "qpushbutton.moc"
