#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <linux/kd.h>

#include <lct/font.h>

/* NOTE: the IOCTLs request 32 scanlines, whatever the fontheight */

int fontdata_write_binary(FILE* out, struct cfontdesc *cfd)
{
  unsigned i;

  if (cfd->charwidth != 8)
    {
      errno = EINVAL;
      return -1;
    }
  
  for (i = 0; i < cfd->charcount; i++)
      if (fwrite(cfd->chardata + (i << 5 /* 32*i */), cfd->charheight, 1, out) != 1)
	  return -1;
  
  return 0;
}

int fontdata_read_binary(FILE* in, struct cfontdesc *cfd)
{
  unsigned i;
  
  /* sanity check */
  if (cfd->charheight < 1 || cfd->charheight > 32) 
    {
      errno = EINVAL;
      return -1;
    }

  if (cfd->charwidth != 8)
    {
      errno = EINVAL;
      return -1;
    }
  
  /* padding zeroes till scanline 32 */
  memset(cfd->chardata, 0, sizeof(cfd->chardata));

  /* chars not yet read */
  for (i = 0; i < cfd->charcount; i++)
    {
      if (fread(cfd->chardata+(32*i), cfd->charheight, 1, in) != 1) 
	{
	  perror("Cannot read font from file");
	  exit(1);
	}
    }

  return 0;
}
