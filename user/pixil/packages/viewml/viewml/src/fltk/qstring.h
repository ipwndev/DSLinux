#ifndef __QSTRING_H
#define __QSTRING_H

#include <algorithm>
#include <string>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <cctype>
#include "fltk-qdefs.h"
//#include "qregexp.h"
using namespace std;

class QRegExp;

class QString : public string{
 protected:
  bool m_bNull;
 public:
  QString() { m_bNull = true; }
  QString(int x) { m_bNull = true; }
  QString(const string & s) { assign(s.c_str()); m_bNull = false; }
  QString(const QString & s) { assign(s.c_str()); m_bNull = false;}
  QString(const char * c) { if(c) assign(c); m_bNull = false;}

  QString copy() const { return *this; }
  operator char *() const { return (char*)c_str(); }
  operator const char *() const { return c_str(); }

  int find(const QRegExp & reg, int index=0) const;
  int find(char c, int index=0, bool cs=true) const ;
  int find(const QString & str, int index=0, bool cs=true) const;

  bool isEmpty() const { return empty(); }
  QString left(unsigned int len) { return substr(0,len); }
  QString right(unsigned int len) { return substr(length()-len,len); }
  QString mid(unsigned int index, unsigned int len=0xffffffff) const
    {
      QString tmp;
      if(len==0xffffffff)
	len = length() - index;
      
      tmp = substr(index,len);
      return tmp;

    }
  void detach() { }
  void truncate(unsigned int len) { assign(left(len)); }

  const char * data() const { return c_str(); }
  QString & replace(const QString & reg, const QString & str) { return *this; }
  int toInt(bool * ok=0, int base=10) 
    { 
      int result;
      if(ok)
	*ok = true;
      result = strtol(c_str(),0,base);      if(errno && ok)
	*ok = false;
      return result;
    }
  
  const QString & operator+(char c) { string::operator+=(c); m_bNull = false; return (*this); }
  const QString & operator=(char c) 
    { assign(""); if(c) (*this) + c; else m_bNull=true; return (*this); }
  void setStr(const char * c) { assign(c); m_bNull = false; }

  QString lower() const
    { 
      QString s;
      s.assign(c_str());
      transform(s.begin(), s.end(), s.begin(), tolower); 
      return s;
    }

  QString upper() const
    { 
      QString s;
      s.assign(c_str());
      transform(s.begin(), s.end(), s.begin(), toupper); 
      return s;
    }


  // isNull broken .. see QString ref
  bool isNull() const { return m_bNull; }

  operator bool() const { return !isNull(); }

  bool operator!() const { return isNull(); }

  int findRev(const QString & str, int index=-1, bool cs=true) const;
  int findRev( char c, int index=-1, bool cs=true ) const;

  QString & sprintf(const char * format, ...)
    {
      m_bNull = false;
      // gag .. help me
      char buf[4096];
      va_list vl;
      va_start(vl,format);
      vsprintf(buf,format,vl);
      va_end(vl);
      
      assign(buf);
      return (*this);
    }

  QString & setNum(int n, int base=10) 
    {
      m_bNull = false;
      (*this) = n;
      return *this;
    }

  QString stripWhiteSpace() const 
    {
      QString tmp;
      char c;
      for(unsigned int i=0; i<length(); i++) {
	c = (*this)[i];
	if((c>=9 && c<=13) || c == 32)
	  continue;
	else
	  tmp += c;
      }
      return tmp;
    }
};

#endif
