#ifndef QFILE_H
#define QFILE_H

#include "qiodevice.h"

class QFile : public QIODevice
{
 public:
  QFile(const QString & name) { }
  virtual bool open(int) { return true; }
  virtual void close() { }
};

#endif
