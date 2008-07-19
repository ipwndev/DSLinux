#ifndef __QTIMER_H
#define __QTIMER_H

#include "fltk-qbase.h"
#include "qevent.h"

class QTimer : public QObject
{
  Q_OBJECT;
 protected:
  bool m_bActive;
  bool m_bSingleShot;
  int m_nTimerID;

 public:
  QTimer(QObject * parent=0, const char * name = 0);
  ~QTimer() 
    { 
    }

  virtual void timerEvent(QTimerEvent * e);
  bool isActive() const;
  int start( int msec, bool sshot = FALSE);
  void stop();

 signals:
  void timeout();
};

#endif
