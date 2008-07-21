/*
 * call: getkeycodes
 *
 * aeb, 941108
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

#include <lct/local.h>
#include <lct/utils.h>
#include <lct/console.h>


static void usage(char *progname)
{
  printf(_("Usage: %s\n"
	   "Print kernel scancode-to-keycode mapping table\n"), progname);
  OPTIONS_ARE();
  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}

void main(int argc, char **argv) 
{
  int fd, sc;
  struct kbkeycode a;

  setuplocale();
  
  simple_options(argc, argv, usage, strip_path(argv[0]));
  
  if (-1 == (fd = get_console_fd(NULL)))
      exit (1);
  
  printf(_("Plain scancodes xx (hex) versus keycodes (dec)\n"));
  printf(_("0 is an error; for 1-88 (0x01-0x58) scancode equals keycode\n"));
  
  for(sc=88; sc<256; sc++) 
    {
      if (sc == 128)
	  printf(_("\n\nEscaped scancodes e0 xx (hex)\n"));
      if (sc % 8 == 0) 
	{
	  if (sc < 128)
	      printf("\n 0x%02x: ", sc);
	  else
	      printf("\ne0 %02x: ", sc-128);
	}
      
      if (sc <= 88) 
	{
	  printf(" %3d", sc);
	  continue;
	}

      a.scancode = sc;
      a.keycode = 0;
      if (ioctl(fd,KDGETKEYCODE,&a)) 
	{
	  if (errno == EINVAL)
	      printf("   -");
	  else 
	    {
	      perror("KDGETKEYCODE");
	      fprintf(stderr, _("failed to get keycode for scancode 0x%x\n"),
		      sc);
	      exit(1);
	    }
	} 
      else
	  printf(" %3d", a.keycode);
    }
  printf("\n");
  
  exit (EX_OK);
}
