#ifndef __QPRINTER_H
#define __QPRINTER_H

#include "fltk-qbase.h"
#include "qpaintdevice.h"

class QWidget;

class QPrinter : public QPaintDevice
{
 public:
  enum PageSize { A4,B5,Letter,Legal,Executive };
  enum Orientation { Portrait, Landscape };
  bool setup(QWidget * parent = 0) { return true; }
  PageSize pageSize() const { return Letter; }
  Orientation orientation() const { return Portrait; }
  bool newPage() { return true; }
};

#endif
