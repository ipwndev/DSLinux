/* Copyright 2003, Century Software */
#ifndef NX_MCURSOR_H
#define NX_MCURSOR_H

#define MWINCLUDECOLORS
extern "C"
{
#include "nano-X.h"
}

class NXMCursor
{
  public:
    static const int SEP_MASK = 0;
    static const int OWN_MASK = 1;

    GR_BITMAP *cursor;
    GR_BITMAP *mask;

    int width, height, hx, hy;

    void setCursor(GR_WINDOW_ID window);
    void load(char *file, int mask);

};

#endif
