#ifndef __QMOVIE_H
#define __QMOVIE_H

#include "qbuffer.h"
#include "qrect.h"
#include "qpixmap.h"

class QMovie
{
 protected:
  QRect m_Rect;
  QPixmap m_Pixmap;
 public:
  QMovie(QByteArray data, int bufsize=1024) { }
  void connectUpdate(QObject * receiver, const char * member) { }
  void connectStatus(QObject * receiver, const char * member) { }
  const QRect & getValidRect() const { return m_Rect; }
  const QPixmap & framePixmap() const { return m_Pixmap; }
  void pause() { }
  void disconnectUpdate(QObject * receiver, const char * member=0) { }
};

#endif
