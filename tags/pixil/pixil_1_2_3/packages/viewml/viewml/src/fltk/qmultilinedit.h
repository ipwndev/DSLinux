#ifndef __QMULTILINEDIT_H
#define __QMULTILINEDIT_H

#include "qwidget.h"
#include <Fl_Multiline_Input.H>

class QMultiLineEdit : public QWidget
{
 protected:
  QString m_Text;
  Fl_Multiline_Input * m_pInput;
 public:
  QMultiLineEdit(QWidget * parent=0, const char * name=0) : QWidget(parent,name)
    { 
      QWidget::setMinimumSize(40,20);
      m_pInput = new Fl_Multiline_Input(0,0,40,20,"Text"); 
      setWidget(m_pInput);
    }
  QString text() const { return m_pInput->value(); }
  void setText(const QString & text) { m_pInput->value(text); }

};

#endif
