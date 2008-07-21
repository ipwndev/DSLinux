/* showfont.c - sidplay a screen-font's contents
 * ydi, 1997-10, from work by aeb
 */
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/kd.h>

#include <lct/local.h>
#include <lct/utils.h>
#include <lct/console.h>

static void usage(char *progname)
{
  printf(_("Usage: %s\n"
	   "Displays a screen-font's contents.\n"), progname);
  OPTIONS_ARE();

  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}

void main(int argc, char* argv[])
{
  int i, j, fontsize;
  char s[10];
  unsigned w;
  int fd;
  char format[10];		       /* indeed 8 should be enough */
  int was_utf;

  int nchars_in_row;
  char *std_hspace, *sep_hspace;

  setuplocale();
  
  simple_options(argc, argv, usage, strip_path(argv[0]));

  /* find the console */
  fd = get_console_fd(NULL);
  
  was_utf = is_in_UTF8_mode(fd);

  /*
   * get the size of current font
   */
  /* FIXME: wrap this into libconsole */
#ifdef GIO_FONTX
  {
    struct consolefontdesc cfd;
      
    cfd.charcount = 0;
    cfd.chardata = NULL;
    if (ioctl (fd, GIO_FONTX, &cfd))
      {
	perror("GIO_FONTX");
	fontsize = 256;
      }
    else
      fontsize = cfd.charcount;
  }
#else
  fontsize = 256;  
#endif

  if (fontsize == 512)
    {
      nchars_in_row = 32;
      std_hspace = " ";
      sep_hspace = "  ";
    }
  else
    {
      nchars_in_row = 16;
      std_hspace = "  ";
      sep_hspace = "   ";
    }
  
/*   fprintf (stderr, "fontsize = %d\n", fontsize); */
  
  if (!was_utf)
    printf("\033%%G");				  /* maybe enter Unicode mode */

  /* compute necessary width to write all fontposes in hex */
  for (i = fontsize - 1, w = 0; i; i >>= 4, w++);

  /* use it to create the format */
  sprintf (format, "0x%%0%ux:", w);

  /* display header */
  for (i = -3; i < (signed)w; i++)
    putchar (' ');
  for (i = 0; i < nchars_in_row; i++)
    {
      fputs ((i%8) ? std_hspace : sep_hspace, stdout);
      putchar ("0123456789ABCDEF"[i%16]);
    }
  putchar ('\n');
  
  for (i = -2; i < (signed)w; i++)
    putchar (' ');
  puts ("+");

  /* for each line */
  for (i = 0; i < fontsize; )
    {
      /* start of line "0x00".."0xF0", or "0x000".."0x1F0", or... */
      printf (format, i);
      
      /* the chars */
      for (j = 0; j < nchars_in_row; j++, i++)
	{
	  ucs2_to_utf8 (0xF000 + i, s);
	  fputs ((i%8) ? std_hspace : sep_hspace, stdout);
	  fputs (s, stdout);
	}
      
      /* final CR */
      printf ("\n");

      /* display as 128-chars groups */
      if (!(i % 128)) printf("\n");
    }

  if (!was_utf)
    printf("\033%%@");				  /* maybe leave Unicode mode */

  exit(0);
}
