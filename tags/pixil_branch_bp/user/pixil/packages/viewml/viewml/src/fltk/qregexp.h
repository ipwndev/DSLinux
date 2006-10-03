#ifndef __QREGEXP_H
#define __QREGEXP_H

#include "qstring.h"

class QString;

class QRegExp
{
 protected:
  QString m_RegExp;
  bool m_bCaseSensitive;
 public:
  QRegExp(const QString & reg, bool cs=true, bool wildcard=false);

  int match(const QString & str, int index=0, int * len=0, 
	    bool indexIsStart=true) const;

};

#endif
