#include "qobject.h"
#include "qevent.h"
#include <Fl.H>
#include <math.h>
#include <unistd.h>

QObject::__timerobj * QObject::m_TimerList[MAXTIMERS];
bool QObject::m_bInit = false;

QObject::__timerobj::__timerobj(QObject * p, int interval)
{
  m_pThis = p;
  m_nInterval = interval;
}

void QObject::InitializeTimers()
{
  for(int i=0; i<MAXTIMERS; i++)
    m_TimerList[i] = NULL;
  m_bInit = true;
}

QObject::~QObject()
{
  for(int i=0; i<MAXTIMERS; i++)
  {
    if(m_TimerList[i])
    {
      if(m_TimerList[i]->getThis() == this)
      {
	delete m_TimerList[i];
	m_TimerList[i] = NULL;
      }
    }
  }

  //  removeThis();
}

void QObject::addSender(QObject * p)
{
  m_Senders.append(p);
}

void QObject::removeThis()
{
  QDictIterator<QConnectionList> i(m_ConnectionDict);
  QConnectionList * list;
  QConnection * c;

  QObject * p;
  while( (p = m_Senders.take(0))) {
    p->removeMe(this);
  }

  // remove me from all the sender lists
  while((list = i.current())) {
    for(c = list->first(); c; c = list->next()) {
      if(c->object()) {
	((QObject*)c->object())->removeSender(this);
      }
    }
    ++i;
  }
}

void QObject::removeSender(QObject * o)
{
  while(m_Senders.removeRef(o));
}

void QObject::removeMe(QObject * p)
{
  QDictIterator<QConnectionList> i(m_ConnectionDict);
  QConnectionList * list;
  QConnection * c;

  while((list = i.current())) {
    for(c = list->first(); c; c = list->next()) {
      if(c->object() == p) {
	list->removeRef(c);
	//	delete c;
      }
    }
    ++i;
  }
}

void QObject::initMetaObject()
{

}

void QObject::killTimer(int id)
{
  DestroyTimer(id);
}

void QObject::TimerFire()
{ 
  QTimerEvent e;
  timerEvent(&e);
}

void QObject::timerEvent(QTimerEvent * e)
{
  cerr << "QObject::timerEvent base called\n";
}

int QObject::startTimer(int interval)
{
  __timerobj * t = CreateTimer(interval); 

  Fl::add_timeout(interval * .001, runthread, t);
  return t->getId();
}

QObject::__timerobj * QObject::CreateTimer(int interval)
{ 
  __timerobj * t = new __timerobj(this,interval);

  if(!m_bInit)
    InitializeTimers();

  for(int i=1; i<MAXTIMERS; i++)
  {
    if(m_TimerList[i] == NULL)
    {
      m_TimerList[i] = t;
      t->setId(i);

      return t;
      break;
    }
  } 
  return NULL;
}	

void QObject::DestroyTimer(int timer)
{
  if(m_TimerList[timer])
  {
    Fl::remove_timeout(runthread,m_TimerList[timer]);
    delete m_TimerList[timer];
  }
  m_TimerList[timer] = NULL;
}

void QObject::runthread(void * p)
{
  for(int i=0; i<MAXTIMERS; i++)
  {
    if(m_TimerList[i] == p)
    {
      __timerobj * t = (__timerobj*)p;

      if(t->getThis())
	t->getThis()->TimerFire();

      // make sure that this timer hasn't been removed
      // in the callback
      if(m_TimerList[i] == t) {

	int id = t->getId();
	
	if(m_TimerList[id])
	  Fl::add_timeout(t->getInterval() * .001 ,runthread,p);
      }
      break;
    }
  }
}

bool QObject::connect ( const QObject * sender, 
			const char * signal, const QObject * receiver,
			const char * member )
{ 
  if(!sender->metaObject())
    ((QObject*)sender)->initMetaObject();
  if(!receiver->metaObject())
    ((QObject*)receiver)->initMetaObject();

  QString _signal,_member,tmp;
  tmp = signal+1;
  _signal = tmp.stripWhiteSpace();
  tmp = member;
  _member = tmp.stripWhiteSpace();
  if(!((QObject*)sender)->m_ConnectionDict[_signal])
    ((QObject*)sender)->m_ConnectionDict.insert(_signal,new QConnectionList());
  
  QConnectionList * cl = ((QObject*)sender)->m_ConnectionDict[_signal];

  QMember p = receiver->findslot(_member);

  //  cerr << "Connecting " << _signal << " with " << _member << "(" << p << ")\n";
  
  cl->append(new QConnection(receiver,p, _member));
  //  cl->append(new QConnection(receiver,
  //			     receiver->findslot(_member), _member));

  ((QObject*)receiver)->addSender((QObject*)sender);

  return true;
}

bool QObject::connect ( const QObject * sender, const char * signal, 
			const char * member ) const 
{ 
  return connect(sender,signal,this,member);
}

QMember QObject::findsignal(const char * signal) const
{
  QString _signal,tmp;
  tmp = signal;
  _signal = tmp.stripWhiteSpace();
  //  cerr << "QObject::findsignal " << className() << "::" << signal << "\n";
  if(metaObject()) {
    QMember mbr = metaObject()->findsignal(_signal);
    return mbr;
  } else {
    return 0;
  }
}

QMember QObject::findslot(const char * slot) const
{
  QString _slot,tmp;
  tmp = slot;
  _slot = tmp.stripWhiteSpace();
  //  cerr << "QObject::findslot " << className() << "::" << slot << "\n";
  if(metaObject()) {
    QMember mbr = metaObject()->findslot(_slot);
    return mbr;
  } else {
    return 0;
  }
}

void QObject::activate_signal(const char * signal, const char * p0) 
{
  QString _signal,tmp;
  tmp = signal;
  _signal = tmp.stripWhiteSpace();

  QConnectionList * cl = m_ConnectionDict[_signal];
  
  //  cerr << " QObject::activate_signal " << _signal << " cl = " << cl  << "\n";
  //  cerr << " QObject::activate_signal this =  " << this << "\n";

  if(!cl)
    return;
  //   cerr << "  count = " << cl->count() << "\n";

  QConnection * c;
  for(unsigned int i=0; i<cl->count(); i++) {
    c = (*cl)[i];
    if(c) {
      typedef void (QObject::*RT1)(const char *);
      typedef RT1 * PRT1;

      //      cerr << "   Going to call " << c << "\n";

      RT1  qm = *((PRT1)(c->member()));

      //      cerr << "   c->object = " << c->object() << " from " << this << "\n";

      if(qm)
	(((QObject*)c->object())->*qm)(p0);

      //      cerr << "   returned from call\n";
    }
  }
}

void QObject::activate_signal(const char * signal) 
{ 
  QString _signal,tmp;
  tmp = signal;
  _signal = tmp.stripWhiteSpace();

  QConnectionList * cl = m_ConnectionDict[_signal];

  //  cerr << " QObject::activate_signal " << _signal << " cl = " << cl << "(" << cl->count() << ")\n";
  //cerr << " QObject::activate_signal this =  " << this << "\n";

  if(!cl)
    return;

  QConnection * c;

  for(unsigned int i=0; i<cl->count(); i++) {
    c = (*cl)[i];
    if(c) {
      QMember qm = *c->member();
      if(qm)
	(((QObject*)c->object())->*qm)();
    }
  }
}

void QObject::activate_signal(const char * signal, int p0) 
{
  QString _signal,tmp;
  tmp = signal;
  _signal = tmp.stripWhiteSpace();
  QConnectionList * cl = m_ConnectionDict[_signal];
  
  //  cerr << " QObject::activate_signal " << _signal << " cl = " << cl << "\n";
  //  cerr << " QObject::activate_signal this =  " << this << "\n";

  if(!cl)
    return;

  QConnection * c;
  for(unsigned int i=0; i<cl->count(); i++) {
    c = (*cl)[i];
    if(c) {
      typedef void (QObject::*RT1)(int);
      typedef RT1 * PRT1;

      RT1  qm = *((PRT1)(c->member()));
      if(qm)
	(((QObject*)c->object())->*qm)(p0);
    }
  }
}

QConnectionList * QObject::receivers(const char * signal) const 
{
  QString _signal,tmp;
  tmp = signal;
  _signal = tmp.stripWhiteSpace();
  QConnectionList * cl = ((QObject*)this)->m_ConnectionDict[_signal];
  return cl;
}



















