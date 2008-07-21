/*
 * resizecons.c - change console video mode
 *
 * Version 0.90
 *
 * How to use this:
 *
 * 1. Get svgalib, make restoretextmode, put it somewhere in your path.
 * 2. Put vga=ask in /etc/lilo/config, boot your machine a number of times,
 *    each time with a different vga mode, and run the command
 *        "restoretextmode -w COLSxROWS".
 *    For me this resulted in the files 80x25, 80x28, 80x50, 80x60, 100x40,
 *    132x25, 132x28, 132x44. Put these files in /usr/lib/kbd/videomodes
 *    (or in your current dir).
 * 3. Now "resizecons COLSxROWS" will change your video mode. (Assuming you have
 *    an appropriate kernel, and svgalib works for your video card.)
 *
 * Note: this is experimental, but it works for me. Comments are welcome.
 * You may have to be root to get the appropriate ioperm permissions.
 * It is not safe to make this program suid root.
 *
 * aeb@cwi.nl - 940924
 *
 * Harm Hanemaaijer added the -lines option, which reprograms the
 * number of scanlines. He writes:
 *
 * Added -lines option, which reprograms the number of scanlines and
 * the font height of the VGA hardware with register I/O, so that
 * switching is possible between textmodes with different numbers
 * of lines, in a VGA compatible way. It should work for 132 column
 * modes also, except that number of columns cannot be changed.
 *
 * Standard VGA textmode uses a 400 scanline screen which is refreshed
 * at 70 Hz. The following modes are supported that use this vertical
 * resolution (C is the number of columns, usually 80 or 132).
 *
 *	mode		font height
 *	C x 25		16
 *	C x 28		14
 *	C x 36		11	(non-standard height)
 *	C x 44		9	(8-line fonts are a good match)
 *	C x 50		8
 *
 * The following modes are supported with a 480 scanline resolution,
 * refresh at 60 Hz. Some not quite VGA compatible displays may not
 * support this (it uses the same vertical timing as standard VGA
 * 640x480x16 graphics mode).
 *
 *	mode		font height
 *	C x 30		16
 *	C x 34		14
 *	C x 40		12	(non-standard height)
 *	C x 60		8
 *
 * Two 12-line fonts are already in the consolefonts directory,
 * namely lat1-12.psf and lat2-12.psf.
 * For the 36 lines mode (11 line font), lat1-10.psf and lat2-10.psf
 * can be used.
 * 
 * hhanemaa@cs.ruu.nl - 941028
 * 
 * Notes:
 *
 * In the consolefonts directory there is 'default8x9' font file but
 * no 'default8x8'. Why is this? The standard VGA BIOS has an 8-line
 * font, and they are much more common in SVGA modes (e.g. 50 and 60
 * row modes). It is true that standard VGA textmode uses effectively
 * 9 pixel wide characters, but that has nothing to do with the font
 * data.
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#if (__GNU_LIBRARY__ >= 6)
# include <sys/perm.h>
#else
# include <linux/types.h>
# include <linux/termios.h>
#endif
#include <linux/vt.h>

#include <getopt.h>
#include <lct/local.h>
#include <lct/utils.h>
#include <lct/console.h>
#include <lct/font.h>				  /* findvideomode() */

#define MODE_RESTORETEXTMODE	0
#define MODE_VGALINES		1

/* VGA textmode register tweaking. */
static void vga_init_io();
static void vga_400_scanlines();
static void vga_480_scanlines();
static void vga_set_fontheight( int );
static int vga_get_fontheight();
static void vga_set_cursor( int, int );
static void vga_set_verticaldisplayend_lowbyte( int );

static char *progname;

static void usage(char *progname) 
{
  printf(_("Usage:  %s COLSxROWS\n"
	   "        %s COLS ROWS\n"
	   "        %s -lines ROWS, with ROWS one of 25, 28, 30, 34, 36, 40, 44, 50, 60\n"), 
	  progname, progname, progname);
}

static void parse_cmdline (int argc, char **argv, 
		    int *mode, int *rows, int *cols)
{
  int c;
  char *p; 
  const struct option long_opts[] =
  {
    { "cols", 1, NULL, 'c' },
    { "lines", 1, NULL, 'l'},
    { "rows", 1, NULL, 'r'},
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'v' },
    { NULL, 0, NULL, 0 }
  };

  while ( (c = getopt_long (argc, argv, "-vhc:r:l:", long_opts, NULL)) != EOF)
    {
      switch (c)
	{
	case 'h':
	  usage(progname);
	  exit(0);
	case 'v':
	  version(progname);
	  exit(0);
	case 'c':
	  *cols = atoi(optarg);
	  if (*cols == 0)
	    {
	      fprintf(stderr, _("%s: Invalid number of columns: %s\n"),
		      progname, optarg);
	      exit(1);
	    }
	  break;
	case 'r':
	  *rows = atoi(optarg);
	  if (*rows == 0)
	    {
	      fprintf(stderr,_("%s: Invalid number of rows: %s\n"),
		      progname, optarg);
	      exit(1);
	    }
	  break;
	case 'l':			/* lines */
	  *rows = atoi(optarg);
	  *mode = MODE_VGALINES;
	  break;
	case '?':
	  usage(progname);
	  exit(1);
	}
  }

   
  if (argc == optind +1) /* COLSxROWS case */
    {
      if (*rows || *cols)
	{
	  fprintf(stderr,_("%s: Invalid arguments"),
		  progname);
	  exit(1);
	}
      if ((p = index (argv[optind],'x')) != 0)
	{
	  *cols = atoi (argv[optind]);
	  *rows = atoi (p+1);
	  return;
	}
      else
	{
	  fprintf(stderr,_("%s: Unrecognized argument"),
		  progname);
	  exit(1);
	}
      return;
     
    }
   
  if (argc == optind +2) /* COLS ROWS case */
    {
      if (*rows || *cols)
	{
	  fprintf(stderr,_("%s: Invalid arguments"),
		  progname);
	  exit(1);
	}
      *cols = atoi(argv[optind]);
      *rows = atoi(argv[optind+1]);
      return;
    }
       
  if (argc == optind && *rows && *cols)
    return;
   
  fprintf (stderr, _("%s: bad number of arguments\n"),
	   progname);
  exit(1);
}
  

int main(int argc, char* argv[]) 
{
  int rr = 0, cc = 0, fd, i, mode = MODE_RESTORETEXTMODE;
  struct vt_sizes vtsizes;
  struct vt_stat vtstat;
  struct winsize winsize;
  char tty[12], cmd[80], infile[1024];
  FILE *fin;
  char *defaultfont;
  char pathname[1024];

  setuplocale();
  
  progname = strip_path(argv[0]);
  parse_cmdline (argc, argv, &mode, &rr, &cc);
  
  if (mode == MODE_RESTORETEXTMODE) 
    {
      /* prepare for: restoretextmode -r 80x25 */
      sprintf(infile, "%dx%d", cc, rr);
      fin = findvideomode(infile, pathname, sizeof(pathname), stdin, NULL);
      if (!fin) 
	{
	  fprintf(stderr, _("%s: cannot find videomode file %s\n"), progname, infile);
	  exit(EX_NOINPUT);
	}
      fclose(fin);
    }

  if (-1 == (fd = get_console_fd(NULL))) exit (EX_OSERR);
  
  if(ioctl(fd, TIOCGWINSZ, &winsize)) 
    {
      perror("TIOCGWINSZ");
      exit(EX_OSERR);
    }
  
  if (mode == MODE_VGALINES) 
    {
      /* Get the number of columns. */
      cc = winsize.ws_col;
      if (rr != 25 && rr != 28 && rr !=30 && rr != 34 && rr != 36
	  && rr != 40 && rr != 44 && rr != 50 && rr != 60) 
	{
	  fprintf(stderr, _("Invalid number of lines\n"));
	  exit(EX_DATAERR);
	}
    }
  
  if(ioctl(fd, VT_GETSTATE, &vtstat)) 
    {
      perror("VT_GETSTATE");
      exit(EX_OSERR);
    }

  vtsizes.v_rows = rr;
  vtsizes.v_cols = cc;
  vtsizes.v_scrollsize = 0;
  
  vga_init_io();		/* maybe only if (mode == MODE_VGALINES) */
  
  if(ioctl(fd, VT_RESIZE, &vtsizes)) 
    {
      perror("VT_RESIZE");
      exit(EX_OSERR);
    }

    if (mode == MODE_VGALINES) 
    {
      /* Program the VGA registers. */
      int scanlines_old;
      int scanlines_new;
      int fontheight;
      
      if (winsize.ws_row == 25 || winsize.ws_row == 28 ||
	  winsize.ws_row == 36 || winsize.ws_row == 44 ||
	  winsize.ws_row == 50)
	  scanlines_old = 400;
      else
	  scanlines_old = 480;
      
      if (rr == 25 || rr == 28 || rr == 36 || rr == 44 || rr == 50)
	  scanlines_new = 400;
      else
	  scanlines_new = 480;
      
      /* Switch to 400 or 480 scanline vertical timing if required. */
      if (scanlines_old != 400 && scanlines_new == 400)
	  vga_400_scanlines();
      
      if (scanlines_old != 480 && scanlines_new == 480)
	  vga_480_scanlines();
      
      switch (rr) 
	{
	case 25 : fontheight = 16; break;
	case 28 : fontheight = 14; break;
        case 30 : fontheight = 16; break;
        case 34 : fontheight = 14; break;
        case 36 : fontheight = 12; break;
        case 40 : fontheight = 12; break;
        case 44 : fontheight = 9; break;
        case 50 : fontheight = 8; break;
        case 60 : fontheight = 8; break;
        default : fontheight = 8; break;
	}
      
      /* Set the VGA character height. */
      vga_set_fontheight(fontheight);
      
      /* Set the line offsets within a character cell of the cursor. */
      if (fontheight >= 10)
	  vga_set_cursor(fontheight - 3, fontheight - 2);
      else
	  vga_set_cursor(fontheight - 2, fontheight - 1);
      
      /*
       * If there are a few unused scanlines at the bottom of the
       * screen, make sure they are not displayed, otherwise
       * there is a annoying changing partial line at the bottom.
       */
      vga_set_verticaldisplayend_lowbyte((fontheight * rr - 1) & 0xff);
      printf(_("Old mode: %dx%d  New mode: %dx%d\n"), winsize.ws_col,
	     winsize.ws_row, cc, rr);
      printf(_("Old #scanlines: %d  New #scanlines: %d  Character height: %d\n"),
	     scanlines_old, scanlines_new, fontheight);
    }

  if (mode == MODE_RESTORETEXTMODE) 
    {
      /* do: restoretextmode -r 25x80 */
      sprintf(cmd, "restoretextmode -r %s\n", pathname);
      errno = 0;
      if(system(cmd)) 
	{
	  if(errno)
	      perror("restoretextmode");
	  fprintf(stderr, _("%s: the command `%s' failed\n"), progname, cmd);
	  exit(1);
	}
    }

  /*
   * for i in /dev/tty[0-9] /dev/tty[0-9][0-9]
   * do
   *     stty rows $rr cols $cc < $i
   * done
   */
  winsize.ws_row = rr;
  winsize.ws_col = cc;
  for (i=0; i<16; i++)
      if (vtstat.v_state & (1<<i)) 
         {
	   sprintf(tty, "/dev/tty%d", i);
	   if ((fd = open(tty, 0)) > 0) 
	     {
	       if(ioctl(fd, TIOCSWINSZ, &winsize))
		   perror("TIOCSWINSZ");
	       close(fd);
	     }
	 }

  /* do: setfont default8x16 */
  /* (other people might wish other fonts - this should be settable) */
  
  /* We read the VGA font height register to be sure. */
  /* There isn't much consistency in this. */
  switch (vga_get_fontheight()) 
    {
    case 8 :
    case 9 : defaultfont = "default8x9"; break;
    case 10 : defaultfont = "lat1-10.psf"; break;
    case 11 :
    case 12 : defaultfont = "lat1-12.psf"; break;
    case 13 :
    case 14 : defaultfont = "iso01.f14"; break;
    case 15 :
    case 16 :
    default : defaultfont = "default8x16"; break;
    }

  sprintf(cmd, "setfont %s", defaultfont);
  errno = 0;
  if(system(cmd)) 
    {
      if(errno)
	  perror("setfont");
      fprintf(stderr, _("%s: the command `%s' failed\n"), progname, cmd);
      exit(1);
    }

  fprintf(stderr,
	  _("%s: don't forget to change TERM (maybe to con%dx%d or linux-%dx%d)\n"),
	  progname, cc, rr, cc, rr);
  if (getenv("LINES") || getenv("COLUMNS"))
      fprintf(stderr,
	      _("Also the variables LINES and COLUMNS may need adjusting.\n"));
  
  return 0;
}

/*
 * The code below is used only with the option `-lines ROWS', and is
 * very hardware dependent, and requires root privileges.
 */

/* Port I/O macros. Note that these are not compatible with the ones */
/* defined in the kernel header files. */

static inline void outb( int port, int value )
{
  __asm__ volatile ("outb %0,%1"
		    : : "a" ((unsigned char)value), "d" ((unsigned short)port));
}

static inline int inb( int port )
{
  unsigned char value;
  __asm__ volatile ("inb %1,%0"
		    : "=a" (value)
		    : "d" ((unsigned short)port));
  return value;
}


/* VGA textmode register tweaking functions. */

static int crtcport;

static void vga_init_io() 
{
  if (iopl(3) < 0) 
    {
      fprintf(stderr, _("%s: cannot get I/O permissions.\n"), progname);
      exit(1);
    }
  crtcport = 0x3d4;
  if ((inb(0x3cc) & 0x01) == 0)
      crtcport = 0x3b4;
}

static void vga_set_fontheight( int h ) 
{
  outb(crtcport, 0x09);
  outb(crtcport + 1, (inb(crtcport + 1) & 0xe0) | (h - 1));
}

static int vga_get_fontheight() 
{
  outb(crtcport, 0x09);
  return (inb(crtcport + 1) & 0x1f) + 1;
}

static void vga_set_cursor( int top, int bottom ) 
{
  outb(crtcport, 0x0a);
  outb(crtcport + 1, (inb(crtcport + 1) & 0xc0) | top);
  outb(crtcport, 0x0b);
  outb(crtcport + 1, (inb(crtcport + 1) & 0xe0) | bottom);
}

static void vga_set_verticaldisplayend_lowbyte( int byte ) 
{
  /* CRTC register 0x12 */
  /* vertical display end */
  outb(crtcport, 0x12);
  outb(crtcport + 1, byte);
}

static void vga_480_scanlines() 
{
  /* CRTC register 0x11 */
  /* vertical sync end (also unlocks CR0-7) */
  outb(crtcport, 0x11);
  outb(crtcport + 1, 0x0c);
  
  /* CRTC register 0x06 */
  /* vertical total */
  outb(crtcport, 0x06);
  outb(crtcport + 1, 0x0b);
  
  /* CRTC register 0x07 */
  /* (vertical) overflow */
  outb(crtcport, 0x07);
  outb(crtcport + 1, 0x3e);

  /* CRTC register 0x10 */
  /* vertical sync start */
  outb(crtcport, 0x10);
  outb(crtcport + 1, 0xea);

  /* CRTC register 0x12 */
  /* vertical display end */
  outb(crtcport, 0x12);
  outb(crtcport + 1, 0xdf);

  /* CRTC register 0x15 */
  /* vertical blank start */
  outb(crtcport, 0x15);
  outb(crtcport + 1, 0xe7);

  /* CRTC register 0x16 */
  /* vertical blank end */
  outb(crtcport, 0x16);
  outb(crtcport + 1, 0x04);

  /* Misc Output register */
  /* Preserver clock select bits and set correct sync polarity */
  outb(0x3c2, (inb(0x3cc) & 0x0d) | 0xe2);
}

static void vga_400_scanlines() 
{
  /* CRTC register 0x11 */
  /* vertical sync end (also unlocks CR0-7) */
  outb(crtcport, 0x11);
  outb(crtcport + 1, 0x0e);
  
  /* CRTC register 0x06 */
  /* vertical total */
  outb(crtcport, 0x06);
  outb(crtcport + 1, 0xbf);

  /* CRTC register 0x07 */
  /* (vertical) overflow */
  outb(crtcport, 0x07);
  outb(crtcport + 1, 0x1f);
  
  /* CRTC register 0x10 */
  /* vertical sync start */
  outb(crtcport, 0x10);
  outb(crtcport + 1, 0x9c);

  /* CRTC register 0x12 */
  /* vertical display end */
  outb(crtcport, 0x12);
  outb(crtcport + 1, 0x8f);

  /* CRTC register 0x15 */
  /* vertical blank start */
  outb(crtcport, 0x15);
  outb(crtcport + 1, 0x96);
  
  /* CRTC register 0x16 */
  /* vertical blank end */
  outb(crtcport, 0x16);
  outb(crtcport + 1, 0xb9);

  /* Misc Output register */
  /* Preserver clock select bits and set correct sync polarity */
  outb(0x3c2, (inb(0x3cc) & 0x0d) | 0x62);
}
