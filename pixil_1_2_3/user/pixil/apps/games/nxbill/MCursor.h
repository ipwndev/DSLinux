
#ifndef X11_MCURSOR_H
#define X11_MCURSOR_H

#include <X11/Xlib.h>
#include <X11/xpm.h>

class MCursor
{
  public:
    static const int SEP_MASK = 0;
    static const int OWN_MASK = 1;
    Pixmap cursor;
    void load(const char *name, int masked);
};

#endif
