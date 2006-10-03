#ifndef __QREGION_H
#define __QREGION_H

#include "qrect.h"
#include "qpoint.h"

class QRegion
{
 public:
  enum RegionType { Rectangle, Ellipse };
  QRegion() { };
  QRegion(const QPointArray & pa, bool winding=false) { }
  QRegion(const QRect & rect, RegionType=Rectangle) { }
  bool contains(const QPoint & p) const { return false; }
  bool contains(const QRect & r) const {  return false; }
};

#endif
