/* Copyright 2003, Century Software */
#ifndef NX_PICTURE_H
#define NX_PICTURE_H

#define MWINCLUDECOLORS
extern "C"
{
#include "nano-X.h"
}
class NXPicture
{
  public:
    GR_IMAGE_ID image;
    GR_WINDOW_ID pixmap;
    GR_GC_ID gc;

    int width, height;

    void draw(int x, int y);
    void draw_centered();
    void load(char *name, int index = -1);
      NXPicture::~NXPicture();
};

#endif
