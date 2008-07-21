/*
 * setmetamode.c - aeb, 940130
 *
 * Call: setmetamode { metabit | escprefix }
 * and report the setting before and after.
 * Without arguments setmetamode will only report.
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <linux/kd.h>
#include <sys/ioctl.h>

#include <lct/local.h>
#include <lct/utils.h>

char* progname;

void usage()
{
  printf(_("\nUsage:\n"
	   "	%1$s [ metabit | meta | bit | escprefix | esc | prefix ]\n"
	   "Each vt has his own copy of this bit. Use\n"
	   "	%1$s [arg] < /dev/ttyn\n"
	   "to change the settings of another vt.\n"
	   "The setting before and after the change are reported.\n"),
	  progname);
}

void report(int meta)
{
  char *s;
  
  switch(meta) 
    {
    case K_METABIT:
      s = N_("Meta key sets high order bit\n");
      break;
    case K_ESCPREFIX:
      s = N_("Meta key gives Esc prefix\n");
      break;
    default:
      s = N_("Strange mode for Meta key?\n");
    }
  printf(_(s));
}

struct meta 
{
  char *name;
  int val;
} metas[] = {
    { "metabit",   K_METABIT },
    { "meta",      K_METABIT },
    { "bit",       K_METABIT },
    { "escprefix", K_ESCPREFIX },
    { "esc",       K_ESCPREFIX },
    { "prefix",    K_ESCPREFIX }
};

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

void main(int argc, char* argv[])
{
  char ometa, nmeta;
  struct meta *mp;

  setuplocale();
  
  progname = strip_path(argv[0]);
  
  if (ioctl(0, KDGKBMETA, &ometa)) 
    {
      perror("KDGKBMETA");
      fprintf(stderr, _("Error reading current setting. Maybe stdin is not a VT?\n"));
      exit(1);
    }

  if (argc <= 1) 
    {
      report(ometa);
      exit(0);
    }
  
  nmeta = 0;			/* make gcc happy */
  for (mp = metas; mp-metas < SIZE(metas); mp++) 
    {
      if(!strcmp(argv[1], mp->name)) 
	{
	  nmeta = mp->val;
	  goto fnd;
	}
    }
  badusage(_("unrecognized argument"));
  
fnd:
  printf(_("old state:    "));
  report(ometa);
  if (ioctl(0, KDSKBMETA, nmeta)) 
    {
      perror("KDSKBMETA");
      exit(1);
    }
  printf(_("new state:    "));
  report(nmeta);
  
  exit(0);
}
