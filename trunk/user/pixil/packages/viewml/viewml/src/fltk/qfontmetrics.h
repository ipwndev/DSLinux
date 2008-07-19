#ifndef __QFONTMETRICS_H
#define __QFONTMETRICS_H

#include "qfont.h"
#include "fl_draw.H"

class QFontMetrics
{
 protected:
  QFont m_Font;
 public:
  QFontMetrics() {}
  QFontMetrics(const QFont & font) { m_Font = font; }

  // int ascent() const { m_Font.SelectFont(); return height(); }
  int ascent() const { m_Font.SelectFont(); return (fl_height() - fl_descent() - 1); } //davet
  int descent() const 
    { m_Font.SelectFont(); return fl_descent(); }
  int width(const QString & str, int len=-1) 
    { 
      m_Font.SelectFont(); 
      if(len == -1)
	return (int)fl_width((const char*)str);
      else
	return (int)fl_width((const char*)str,len);
    }
  int width(char c) const 
    { m_Font.SelectFont(); return (int)fl_width(c); }
  int height() const 
    { m_Font.SelectFont(); return fl_height(); }

};

#endif
