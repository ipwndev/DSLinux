#ifndef __KLOCALE_H
#define __KLOCALE_H

#include "qstring.h"

class KLocale
{
 protected:
  QString m_CharSet;
 public:
  KLocale() { m_CharSet = "us-ascii"; }
  const QString & charset() const { return m_CharSet; }
};

#endif
