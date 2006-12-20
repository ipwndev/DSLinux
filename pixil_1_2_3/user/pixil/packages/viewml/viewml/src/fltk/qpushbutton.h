#ifndef __QPUSHBUTTON_H
#define __QPUSHBUTTON_H

#include "qwidget.h"
#include "qstring.h"
#include "Fl_Button.H"

#ifdef _NANOX
#include "n_x.h"
#else
#include "x.H"
#endif

#define QPUSHBUTTON_HEIGHT 20
#define QPUSHBUTTON_PADDING 10
#define QPUSHBUTTON_VPADDING 6
class QPushButton : public QWidget
{
  Q_OBJECT;
 protected:
  QString m_Text;
  Fl_Button * b;

 public:
  QPushButton(QWidget * parent, const char * name =0);
  ~QPushButton() {}


  QString text() const { return m_Text; }
  void _callback();
// CRH moved to qpushbutton.cpp
  void setText(const QString & text);
//  virtual void setText(const QString & text)
//    { 
//      m_Text = text; 
//      m_pWidget->label(text); 
//      fl_font(m_pWidget->labelfont(),m_pWidget->labelsize());
//      QWidget::setMinimumSize(fl_width((const char *) text) + QPUSHBUTTON_PADDING, 
//			      QPUSHBUTTON_HEIGHT);
//      QWidget::resize(sizeHint());
//    }

 signals:
    void clicked();
};

#endif
