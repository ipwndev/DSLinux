#ifndef QOBJECT_H
#define QOBJECT_H

#include "qobjectdefs.h"
#include "qmetaobject.h"
#include "qconnection.h"
#include "qdict.h"

#include <iostream>
#include <signal.h>
#include <pthread.h>

#define MAXTIMERS 100


class QTimerEvent;


class QObject
{
  // timer support
 protected:

  class __timerobj
    {
    protected:
      QObject * m_pThis;
      int m_nInterval;
      int m_nID;
    public:
      __timerobj(QObject * p, int interval);
      void setId(int id) { m_nID = id; }
      int getId() const { return m_nID; }
      QObject * getThis() const { return m_pThis; }
      int getInterval() const { return m_nInterval; }
    };

  static bool m_bInit;

  static __timerobj * m_TimerList[MAXTIMERS];
  pthread_t m_Thread;
  
  void TimerFire();

  static void runthread(void*p);

  __timerobj *  CreateTimer(int interval);
  void DestroyTimer(int timerid);

 public:
  void InitializeTimers();
  // end timer support

 protected:

  QList<QObject> m_Senders;

  QConnectionDict m_ConnectionDict;
  QConnectionList * receivers(const char * signal) const;

  void addSender(QObject * p);
  void removeThis();
  void removeSender(QObject * o);
  void removeMe(QObject * p);

  virtual void initMetaObject();

 public:

  QObject() { initMetaObject(); }
  virtual ~QObject();
  virtual void timerEvent(QTimerEvent * e);

  int startTimer(int interval);
  void killTimer(int id);
  bool inherits(const char * c) const { return false; }
  virtual const char * className() const { return "ClassNULL"; }
  static bool connect ( const QObject * sender, 
			const char * signal, const QObject * receiver,
			const char * member );

  bool connect ( const QObject * sender, const char * signal, 
		 const char * member ) const;

  bool disconnect ( const char * signal=0, const QObject * receiver=0, 
		    const char * member=0 ) { return false; }
  bool disconnect ( const QObject * receiver, 
		    const char * member=0 ) { return false; }

  static void badSuperclassWarning(const char * className,
				   const char * superclassName) { }

  bool signalsBlocked() const { return false; }
  void blockSignals(bool b) { }

  void activate_signal(const char * signal, const char *);
  void activate_signal(const char * signal);
  void activate_signal(const char * signal, int i);

  bool isA(const char * name) const { return !strcmp(className(),name); }

  virtual QMetaObject * metaObject() const { return 0; }
  QMember findsignal(const char * signal) const;
  QMember findslot(const char * slot) const;
};


class QSenderObject : public QObject
{
 public:
  void setSender(QObject * s) { } 
};

#endif
