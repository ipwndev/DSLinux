/*
 * loadunimap.c - aeb
 *
 * Version 0.92
 */

#include <config.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <lct/console.h>
#include <lct/font.h>
#include <lct/local.h>

void saveunicodemap(int fd, char *oufil, int verbose, int no_act)
{
  FILE *fpo;
  struct unimapdesc descr;

  get_kernel_unimap(fd, &descr);
  
  if (no_act)
      printf(_("Would save %d SFM entries in `%s'.\n"), descr.entry_ct, oufil);
  else
    {
      if (verbose)
	  printf("Saving %d SFM entries in `%s'.\n", descr.entry_ct, oufil);

      if ((fpo = fopen(oufil, "w")) == NULL) 
	{
	  perror(oufil);
	  exit(1);
	}
      
      sfm_write_ascii(fpo, &descr);
      
      fclose(fpo);
    }
}
