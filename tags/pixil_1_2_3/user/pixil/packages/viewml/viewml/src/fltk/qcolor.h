#ifndef __QCOLOR_H
#define __QCOLOR_H

#include "qstring.h"


class QColor
{
 protected:

  int m_nRed;
  int m_nGreen;
  int m_nBlue;
  bool m_bSet;
 public:
  QColor() { m_nRed = 255; m_nGreen = 64; m_nBlue = 192; m_bSet = false; }
  QColor(const QColor & c)
    {
      m_nRed = c.red(); m_nGreen = c.green(); m_nBlue = c.blue();
      m_bSet = c.isValid();
    }
  QColor(int red, int green, int blue)
    {
      m_nRed = red; m_nGreen = green; m_nBlue = blue;
      m_bSet = true;
    }

  const int red() const { return m_nRed; }
  const int green() const { return m_nGreen; }
  const int blue() const { return m_nBlue; }

  void setNamedColor(const QString & name);

  bool isValid() const { return m_bSet; }
};

extern  const QColor color0;
extern  const QColor color1;
extern  const QColor black;
extern  const QColor white;
extern  const QColor darkGray;
extern  const QColor gray;
extern  const QColor lightGray;
extern  const QColor red;
extern  const QColor green;
extern  const QColor blue;
extern  const QColor cyan;
extern  const QColor magenta;
extern  const QColor yellow;
extern  const QColor darkRed;
extern  const QColor darkGreen;
extern  const QColor darkBlue;
extern  const QColor darkCyan;
extern  const QColor darkMagenta;
extern  const QColor darkYellow;

class QColorGroup
{
 protected:
  QColor m_Light;
  QColor m_Dark;
  QColor m_Mid;
  QColor m_Text;
  QColor m_Foreground;

 public:

  QColorGroup() { 
      m_Text = black;
      m_Foreground = black;
      m_Dark = darkGray;
      m_Light = lightGray;
      m_Mid = gray;
  }

  QColorGroup ( const QColor & foreground, const QColor & button, 
		const QColor & light, const QColor & dark, 
		const QColor & mid, const QColor & text, 
		const QColor & base )
    {
      m_Text = text;
      m_Foreground = foreground;
      m_Dark = dark;
      m_Light = light;
      m_Mid = mid;
    }
    
  const QColor & foreground() const { return m_Foreground; }
  const QColor & light() const { return m_Light; }
  const QColor & dark() const { return m_Dark; }
  const QColor & mid() const { return m_Mid; }
  const QColor & text() const { return m_Text; }
};

#endif
