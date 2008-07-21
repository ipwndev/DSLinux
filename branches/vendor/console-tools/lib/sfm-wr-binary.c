#include <stdio.h>
#include <stdlib.h>
#include <linux/kd.h>
#include <endian.h>

#include <lct/font.h>

int sfm_write_binary (FILE* out, struct unimapdesc *map, int fontlen)
{
  int i;
  struct unipair *pair;
  struct unicode_list *unilist[512];
  unicode psf_sep, uc;
  struct unicode_list *newmbr, tmpmbr;
  
  /* 
   * first build an array on linked lists, mapping fontpos to unicodes
   */
  
  /* FIXME: `512' should probably read `fontlen' instead */

  for ( i = 0 ; i < 512 ; i++ )	/* Initialize unicode list */
    unilist[i] = NULL;

  /* read list in reverse order */
  for ( pair = map->entries + map->entry_ct - 1; pair >= map->entries; pair-- )
    {
      if ( pair->unicode != PSF_SEPARATOR && pair->unicode <= 0xFFFF )
	{
	  /* Add to linked list */
	  
	  newmbr = malloc(sizeof(struct unicode_list));
	  newmbr->uc = (unicode) pair->unicode;
	  /* in reverse order, so that we get original order back inside entries */
	  newmbr->next = unilist[pair->fontpos];
	  unilist[pair->fontpos] = newmbr;
	}
      /* otherwise: ignore */
    }

  
  /*
   * then use it to build the PSF_SEPARATOR-separated table used
   * in PSF files, and output at once.
   */
  
  psf_sep = PSF_SEPARATOR;	/* Separator between unicode strings */

  for ( i = 0 ; i < fontlen ; i++ )
    {
      for ( newmbr = unilist[i] ; newmbr ; newmbr = tmpmbr.next )
	{
	  tmpmbr = *newmbr;
	  uc = tmpmbr.uc;
#ifndef BYTE_ORDER
# error <endian.h> was not included
#elif BYTE_ORDER == BIG_ENDIAN
	  uc = ((uc << 8) | (uc >> 8)) & 0xffff;
#endif
	  fwrite(&uc, sizeof(unicode), 1, out);
	  free(newmbr);
	}

      /* Write string terminator */
      fwrite(&psf_sep, sizeof(unicode), 1, out);
    }
  
  return 0;
}
