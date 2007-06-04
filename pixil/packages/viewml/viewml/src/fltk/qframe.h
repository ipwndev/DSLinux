#ifndef __QFRAME_H
#define __QFRAME_H

#include "qwidget.h"

class QFrame : public QWidget
{
 protected:
  int m_nLineWidth;
  int m_nFrameStyle;
 public:
  enum Shape { NoFrame = 0, Box = 0x0001, Panel = 0x0002, WinPanel = 0x0003, 
	       HLine = 0x0004, VLine = 0x0005, StyledPanel = 0x0006, 
	       PopupPanel = 0x0007, MShape = 0x000f };
  
  enum Shadow { Plain = 0x0010, Raised = 0x0020, Sunken = 0x0030, 
		MShadow = 0x00f0 };
  QFrame(QWidget * parent =0, const char * name=0, WFlags f=0, bool s = true) :
    QWidget(parent,name)
    { m_nLineWidth = 1; m_nFrameStyle = NoFrame; }
  virtual void setLineWidth(int w) { m_nLineWidth = w; }
  int lineWidth() const { return m_nLineWidth; }
  void setFrameStyle(int style) { m_nFrameStyle = style; }
  int frameStyle() const { return m_nFrameStyle; }
};


#endif
