
#ifndef LIBRARY_H
#define LIBRARY_H

#include "NXPicture.h"
#include "NXMCursor.h"

class Library
{				/* global structure of all OSes */
  public:
    static const int WINGDOWS = 0;	/* OS 0 is wingdows */
    static const int OFF = -1;	/* OS -1 means the computer is off */
    static const int PC = 5;	/* OS >= PC means the OS is a PC OS */
    static const int NUM_OS = 9;	/* The number of OSes (not Wingdows) */
    int width, height;		/* size of OS picture */
    NXPicture os[NUM_OS + 1];	/* array of OS pictures */
    NXMCursor cursor[NUM_OS + 1];	/* array of OS cursors (drag/drop) */
    void load_pix();
};

#endif
