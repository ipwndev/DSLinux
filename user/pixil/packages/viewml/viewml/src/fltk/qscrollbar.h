#ifndef __QSCROLLBAR_H
#define __QSCROLLBAR_H

#include "qwidget.h"
//#include "Fl_Scrollbar.H"
#include "nxscrollbar.h"

class QScrollBar : public QWidget
{
  Q_OBJECT;
 protected:
  int m_nLineStep;
  int m_nPageStep;
  int m_nMin;
  int m_nMax;
  //  Fl_Scrollbar * m_pScroll;
  NxScrollbar * m_pScroll;
 public:

  enum Orientation { Horizontal, Vertical };

  QScrollBar() 
    { 
    }
  QScrollBar(int min, int max, int lineStep, int pageStep, int value,
	     Orientation o, QWidget * parent, const char * name=0);

  void _callback();
  virtual void resize(int _x, int _y, int _w, int _h);
  void setSteps(int line, int page) { m_nLineStep=line; m_nPageStep=page; }
  void setValue(int value);
  void setRange(int minValue, int maxValue) 
    {m_nMin = minValue; m_nMax = maxValue; setValue(0); }
  int value() const { return m_pScroll->value(); }
  int pageStep() const { return m_nPageStep; }
  int lineStep() const { return m_nLineStep; }
  void addLine() { setValue(value() + lineStep()); }
  void subtractLine() { setValue(value() - lineStep()); }
  void addPage() { setValue(value() + pageStep()); }
  void subtractPage() { setValue(value() - pageStep()); }

 signals:
  void valueChanged(int);

};

#endif
