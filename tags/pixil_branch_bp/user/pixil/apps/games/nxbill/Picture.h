

#ifndef X11_PICTURE_H
#define X11_PICTURE_H

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/xpm.h>

class Picture
{
  public:
    Dimension width, height;
    Pixmap pix;
    GC gc;
    void draw(int x, int y);
    void draw_centered();
    void load(const char *name, int index = -1);
};

#endif
