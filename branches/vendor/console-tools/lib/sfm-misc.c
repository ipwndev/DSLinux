/*
 * loadunimap.c - aeb
 *
 * Version 0.92
 */

#include <config.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <linux/kd.h>
#include <sys/ioctl.h>

#include <lct/local.h>
#include <lct/console.h>


int set_kernel_unimap(int fd, struct unimapdesc *descr)
{
  struct unimapinit advice;

  /* Note: after PIO_UNIMAPCLR and before PIO_UNIMAP
   printf does not work. */

  advice.advised_hashsize = 0;
  advice.advised_hashstep = 0;
  advice.advised_hashlevel = 1;	       /* FIXME: 0 or 1 ? */
  
again:
  if (ioctl(fd, PIO_UNIMAPCLR, &advice)) 
    {
#ifdef ENOIOCTLCMD
      if (errno == ENOIOCTLCMD) 
	{
	  fprintf(stderr, _("It seems this kernel is older than 1.1.92\n"));
	  fprintf(stderr, _("No Unicode mapping table loaded.\n"));
	} 
      else
#endif
	  return -1;
    }
  
  if (ioctl(fd, PIO_UNIMAP, descr))
    {
      if (errno == ENOMEM && advice.advised_hashlevel < 100) 
	{
	  advice.advised_hashlevel++;
	  goto again;
	}
      return -1;
    }
  
  return 0;
}


int get_kernel_unimap(int fd, struct unimapdesc *descr)
{
  int ct;
  
  descr->entry_ct = 0;
  descr->entries = NULL;
  
  /* get entry_ct */
  if(ioctl(fd, GIO_UNIMAP, (unsigned long) descr)) /* failed */
    {
      if(errno != ENOMEM || descr->entry_ct == 0)
	{
	  perror(_("GIO_UNIMAP (get count)"));
	  return -1;
	}
      
      /* memorize ct for check below */
      ct = descr->entry_ct;
      
      /* alloc place for entry_ct unipairs */
      descr->entries = (struct unipair *)
	malloc(descr->entry_ct * sizeof(struct unipair));
      
      /* actually get unipairs */
      if(ioctl(fd, GIO_UNIMAP, (unsigned long) descr))
	{
	  perror(_("GIO_UNIMAP (get map)"));
	  return -1;
	}

      /* someone could change the unimap between our
       * first and second ioctl, so we can have descrepancies
       */
      if (ct != descr->entry_ct)
	{
	  fprintf(stderr, _("strange... ct changed from %d to %d\n"),
		  ct, descr->entry_ct);
	  if (ct < descr->entry_ct)
	    {
	      descr->entry_ct = ct;
	      fprintf(stderr, _("... assuming %d\n"), ct);
	    }
	}

      return 0;
    }
  else
    {
      /* no valid SFM currently loaded */
      errno = ENXIO;
      return -1;
    }
}
