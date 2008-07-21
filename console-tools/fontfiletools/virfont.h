#ifndef _LCT_VIRFONT_H
#define _LCT_VIRFONT_H

#include <linux/kd.h>

#include <lct/font.h>

struct virchar
{
  struct unifont *src_font;
  unsigned long src_index;
};

#endif /* _LCT_VIRFONT_H */
