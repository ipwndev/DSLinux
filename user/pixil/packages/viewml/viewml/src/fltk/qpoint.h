#ifndef QPOINT_H
#define QPOINT_H

#include "qarray.h"

class QPoint
{
 public:
  
  QPoint() { m_nX = 0; m_nY = 0; }
  QPoint(int x, int y) { m_nX = x; m_nY = y; }

  int x() const { return m_nX; }
  int y() const { return m_nY; }
  
  void setX(int x) { m_nX = x; }
  void setY(int y) { m_nY = y; }

 protected:
  int m_nX;
  int m_nY;
};

class QPointArray : public QArray<QPoint>
{
 public:
  void setPoint(uint i, int x, int y) { }
};

#endif
