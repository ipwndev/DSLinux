#ifndef QRECT_H
#define QRECT_H

#include "qpoint.h"

class QRect
{
 protected:
  int m_nX;
  int m_nY;
  int m_nWidth;
  int m_nHeight;

 public:

  QRect() { m_nWidth = m_nHeight = m_nX = m_nY = 0; }
  QRect(int left, int top, int width, int height)
    {
      m_nX = left;
      m_nY = top;
      m_nWidth = width;
      m_nHeight = height;
    }

  bool contains(const QRect & r, bool proper=false) const;
  bool contains(const QPoint & p, bool proper=false) const;

  bool intersects(const QRect & r) const;
  QRect intersect(const QRect & r) const;
  void moveBy(int dx, int dy) 
    { 
      m_nX += dx;
      m_nY += dy;
    }
  int x() const { return m_nX; }
  int y() const { return m_nY; }
  int width() const { return m_nWidth; }
  int height() const { return m_nHeight; }
  void setRight(int r) { m_nWidth = r - m_nX; }
  void setBottom(int b) { m_nHeight = b - m_nY; }
  void setWidth(int w) { m_nWidth = w; }
  void setHeight(int h) { m_nHeight = h; }
};

#endif
