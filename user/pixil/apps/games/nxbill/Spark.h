
#ifndef SPARK_H
#define SPARK_H

#include "NXPicture.h"

class Spark
{
  public:
    NXPicture pictures[2];
    int width, height;
    void draw();
    void load_pix();
    static const int speed = 5;
    int delay(unsigned int lev);
};

#endif
