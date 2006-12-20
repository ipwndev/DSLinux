#ifndef __QPEN_H
#define __QPEN_H

#include "qcolor.h"

class QColor;

enum PenStyle { NoPen, SolidLine, DashLine,     // pen style
                DotLine, DashDotLine, DashDotDotLine };
class QPen
{
 protected:
  QColor m_Color;
  PenStyle m_Style;
 public:
  QPen() { }
  QPen(const QColor & color) { m_Color = color; }

  const QColor & color() const { return m_Color; }
  void setColor(const QColor & color) { m_Color = color; }
  void setStyle(PenStyle p) { m_Style = p; }
};

#endif
