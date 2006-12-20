#ifndef __QCOLLECTION_H
#define __QCOLLECTION_H

class QCollection
{
 protected:
  bool m_bAutoDelete;
 public:
  QCollection() { m_bAutoDelete = false; }
  void setAutoDelete(bool b) { m_bAutoDelete = b; }
};

#endif
