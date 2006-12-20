#ifndef __QFONTINFO_H
#define __QFONTINFO_H

#include "qfont.h"
#include "qstring.h"

class QFontInfo
{
 public:
  QFontInfo(QFont & qf);
  QString family();
  bool italic();
  bool fixedPitch();
  bool bold();
  QFont::CharSet charSet();

 private:
  QString m_family;
  bool m_italic;
  int m_weight;
  QFont::CharSet m_charSet;
};

#endif
