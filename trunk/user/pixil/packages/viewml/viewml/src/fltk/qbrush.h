#ifndef __QBRUSH_H
#define __QBRUSH_H

#include "qcolor.h"

enum BrushStyle                                 // brush style
      { NoBrush, SolidPattern,
        Dense1Pattern, Dense2Pattern, Dense3Pattern, Dense4Pattern,
        Dense5Pattern, Dense6Pattern, Dense7Pattern,
        HorPattern, VerPattern, CrossPattern,
        BDiagPattern, FDiagPattern, DiagCrossPattern, CustomPattern=24 };

class QBrush
{
 protected:
  QColor m_Color;
  BrushStyle m_BS;
 public:
  QBrush() { }
  QBrush(const QColor & color, BrushStyle bs=SolidPattern) 
    { 
      m_Color = color;
      m_BS = bs;
    }
  const QColor & color() const { return m_Color; }
};

#endif
