#ifndef __NXSCROLLBAR_H
#define __NXSCROLLBAR_H

#include "nxslider.h"

class NxScrollbar : public NxSlider
{
  int linesize_;
  int pushed_;
  FL_EXPORT void draw();
  FL_EXPORT int handle(int);
  static FL_EXPORT void timeout_cb(void*);
  FL_EXPORT void increment_cb();

 public:
  NxScrollbar(int x, int y, int w, int h, const char * l=0);

  int value() {return int(NxSlider::value());}
  int value(int position, int size, int top, int total) {
    return scrollvalue(position, size, top, total);
  }
  int linesize() const {return linesize_;}
  void linesize(int i) {linesize_ = i;}



};

#endif
