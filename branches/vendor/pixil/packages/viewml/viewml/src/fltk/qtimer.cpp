#include "qtimer.h"
#include "unistd.h"


QTimer::QTimer(QObject * parent, const char * name ) :
  QObject() 
{ 
  m_bActive = false;
}	

void QTimer::stop() 
{
  killTimer(m_nTimerID);
  m_bActive = false;
}

bool QTimer::isActive() const
{
  return m_bActive;
}

int QTimer::start( int msec, bool sshot)
{
  m_bSingleShot = sshot;
  m_nTimerID = startTimer(msec);
  m_bActive = true;
  return m_nTimerID;
}

void QTimer::timerEvent(QTimerEvent * e)
{
  timeout();
  if(m_bSingleShot)
     stop();
}

#include "qtimer.moc" 
