#ifndef QSIZE_H
#define QSIZE_H

class QSize
{
 protected:
  int m_nHeight;
  int m_nWidth;

 public:
  QSize() { m_nHeight = m_nWidth = 0; }
  QSize(int w, int h) { m_nHeight = h; m_nWidth = w; }
  int width() const { return m_nWidth; }
  int height() const { return m_nHeight; }
  void setHeight(int h) { m_nHeight = h; }
  void setWidth(int w) { m_nWidth = w; }
};


#endif
