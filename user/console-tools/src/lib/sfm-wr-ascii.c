#include <stdio.h>
#include <linux/kd.h>

#include <lct/font.h>

int sfm_write_ascii(FILE* fpo, struct unimapdesc *descr)
{
  int i;
  char utf[4]; /* enough for UCS2 */

  for(i=0; i<descr->entry_ct; i++)
    {
      ucs2_to_utf8 (descr->entries[i].unicode, utf);
      fprintf(fpo, "0x%02x\tU+%04x\t# %s \n",
	      descr->entries[i].fontpos,
	      descr->entries[i].unicode,
	      utf);
    }
  
  return 0;
}
