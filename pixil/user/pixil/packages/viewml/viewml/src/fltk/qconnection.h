#ifndef __QCONNECTION_H
#define __QCONNECTION_H


#include "qstring.h"
#include "qlist.h"

class QObject;

class QConnection
{
 protected:
  const QObject * m_Destination;
  QMember m_Member;
  QString m_strName;
  int m_nArgs;

 private:
  // someboday please tell me that under Qt there is a better way to 
  // count the args .. I feel like such a hack with the below.
  int countargs(const char * name)
    {
      int len = strlen(name);
      int nargs = 0;
      if(!strstr(name,"()")) {
	nargs++;
	for(int i=0; i<len; i++) {
	  if(name[i]==',')
	    nargs++;
	}
      }
      
      return nargs;
    }

 public:
  QConnection(const QObject * p, QMember mem, const char * memberName) 
    { 
      m_Destination = p;
      m_Member = mem;
      m_strName = memberName;
      m_nArgs = countargs(memberName);
    }

  const QObject * object () const { return m_Destination; }
  const QString & name() const { return m_strName; }
  QMember * member() { return &m_Member; }
  int numArgs() const { return m_nArgs; }

};

class QConnectionList : public QList<QConnection>
{
 public:
  QConnectionList() { };
};

class QConnectionListIt
{
 protected:
  QConnectionList * m_List;
 public:
  QConnectionListIt(QListM_QConnection & list) 
    { m_List = &list; m_List->first(); }
  void operator++() { m_List->next(); }
  int count() const { return m_List->count(); }
  QConnection * current() 
    { return m_List->current(); }
};

#define QConnectionDict QDict<QConnectionList>

#endif
