#ifndef __QPALETTE_H
#define __QPALETTE_H

#include "qcolor.h"

class QPalette 
{
 protected:
  QColorGroup m_Dummy;
 public:
  QPalette() { }
  QPalette copy() const { return *this; }
  const QColorGroup & normal() const { return m_Dummy; }

  void setNormal(const QColorGroup & cg) { m_Dummy = cg; }
};

#endif
