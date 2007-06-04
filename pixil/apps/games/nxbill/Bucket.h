
#ifndef BUCKET_H
#define BUCKET_H

#include "NXPicture.h"

#ifndef PDA
#include "NXMCursor.h"
#endif

class Bucket
{
  public:
    NXPicture picture;
#ifndef PDA
    NXMCursor cursor;
#endif
    int width, height;
    void draw();
    int clicked(int x, int y);
    void load_pix();
};

#endif
