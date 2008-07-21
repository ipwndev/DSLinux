#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <endian.h>
#include <linux/kd.h>

#include <lct/font.h>

int sfm_read_binary(FILE* fpi, struct unimapdesc *ud, int fontsize)
{
  struct unipair *up;
  int ct = 0, maxct;
  int glyph, rd_count;
  unicode uc;

  maxct = fontsize << 1 /* *2 */;      /* enough for most purposes */
  if (!(up = (struct unipair *) malloc(maxct * sizeof(struct unipair))))
      return -1;
  
  for (glyph = 0; glyph < fontsize; glyph++) 
    {
      while ((rd_count = fread(&uc, sizeof(uc), 1, fpi)) == 1 &&
	     uc != PSF_SEPARATOR) 
	{
#ifndef BYTE_ORDER
# error <endian.h> was not included
#elif BYTE_ORDER == BIG_ENDIAN
	  uc = ((uc << 8) | (uc >> 8)) & 0xffff;
#endif
	  if (ct >= maxct)	       /* allocated table-size exceeded */
	    {
	      maxct <<= 1 /* *= 2 */;
	      up = (struct unipair*) realloc(up, maxct * sizeof(struct unipair));
	    }
	  up[ct].unicode = uc;
	  up[ct].fontpos = glyph;
	  ct++;
	}
      if (rd_count != 1) 
	{
	  errno = EBFONT;
	  return -1;
	}
    }

  ud->entry_ct = ct;
  ud->entries = up;
  
  return 0;
}
