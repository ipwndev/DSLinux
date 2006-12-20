#ifndef __QLINEEDIT_H
#define __QLINEEDIT_H

#include "qwidget.h"
#include "qstring.h"

#include <Fl_Input.H>

class QLineEdit : public QWidget
{
  Q_OBJECT
 protected:
  Fl_Input * m_pInput;
 public:
  enum        EchoMode { Normal, NoEcho, Password };
  
  QLineEdit(QWidget * parent, const char * name=0);
  ~QLineEdit() { }
  QString text() const { return m_pInput->value(); }

  void _callback();

  void setText(const QString & text) { m_pInput->value(text); }
  void setMaxLength(int len) { m_pInput->maximum_size(len); }
  int maxLength() const { return m_pInput->maximum_size(); }
  virtual void setEchoMode(bool echo) { if(echo) m_pInput->type(FL_SECRET_INPUT); } 

 signals:

    void textChanged( const char *);
  void returnPressed( );
};

#endif
