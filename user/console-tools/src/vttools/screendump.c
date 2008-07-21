/*
 * screendump.c - aeb 950214
 *
 * [Note: similar functionality can be found in setterm]
 *
 * Call: "screendump N" when the screen of /dev/ttyN has to be dumped
 *
 * On Linux up to 1.1.91 there is an ioctl that will do the dumping.
 * Because of problems with security this has been scrapped.
 * From 1.1.92 on, make devices "virtual console screen" and
 * "virtual console screen with attributes" by (fill in the ellipses):
 *	cd /dev
 *	for i in 0 1 2 3 ...; do
 *		mknod vcs$i c 7 $i
 *		mknod vcsa$i c 7 `expr 128 + $i`
 *	done
 * and give them your favourite owners and permissions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <getopt.h>

#include <lct/local.h>
#include <lct/utils.h>

static void usage(char *progname)
{
  printf(_("Usage: %s [console]\n"
	   "Dump the contents of the screen to stdout\n"), progname);
  OPTIONS_ARE();
  OPT("-s --screen=s     ", _("dump contents of screen s"));

  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}

int parse_args (int argc,char **argv) 
{
  char * progname = strip_path(argv[0]);
  const struct option long_opts[] = {
    {"screen", 1, NULL, 's' },
    {"help"  , no_argument, NULL, 'h' },
    {"version", no_argument, NULL, 'V' },
    { NULL, 0, NULL, 0}
  };
  int c, cons = 0;
  while ( (c = getopt_long (argc, argv, "Vhs:", long_opts, NULL)) != EOF) {
    switch (c) {
    case 'h':
      usage(progname);
      exit(0);
    case 'V':
      version(progname);
      exit(0);
    case 's':			/* screen */
      cons = atoi(optarg);
      break;
    case '?':
    default:
      usage(progname);
      exit(1);            
    }
  }
  if (optind < argc)
    {
      if (argc == optind+1)
	cons = atoi(argv[optind]);
      else {
	fprintf(stderr,_("%s: Wrong number of args\n"), progname);
	exit(1);
      }
    }
    
  return cons;
}
  

  
  
int main(int argc, char **argv) 
{
  int cons = 0;
  char infile[20];
  unsigned char header[4];
  unsigned int rows, cols, i, j;
  int fd;
  char *inbuf, *outbuf, *p, *q;

  setuplocale();
  
  cons = parse_args (argc, argv);
  
  if (cons)
      sprintf(infile, "/dev/vcsa%d", cons);
  else
      sprintf(infile, "/dev/vcsa");
  
  fd = open(infile, 0);
  if (fd < 0 || read(fd, header, 4) != 4)
      goto try_ioctl;
  rows = header[0];
  cols = header[1];
  if (rows * cols == 0)
      goto try_ioctl;
  inbuf = (char*)malloc(rows*cols*2);
  outbuf = (char*)malloc(rows*(cols+1));
  if(!inbuf || !outbuf) 
    {
      fprintf(stderr, _("Out of memory\n"));
      exit(1);
    }
  if (read(fd, inbuf, rows*cols*2) != (int)(rows*cols*2)) 
    {
      fprintf(stderr, _("Error reading %s\n"), infile);
      exit(1);
    }
  p = inbuf;
  q = outbuf;
  for(i=0; i<rows; i++) 
    {
      for(j=0; j<cols; j++) 
	{
	  *q++ = *p;
	  p += 2;
        }
      while(j-- > 0 && q[-1] == ' ')
          q--;
      *q++ = '\n';
    }
  goto done;
  
  try_ioctl:
    {
      struct winsize win;
      char consnam[20];
      unsigned char *screenbuf;
      
      fprintf(stderr, _("Could not use /dev/vcs*, trying TIOCLINUX\n"));
      sprintf(consnam, "/dev/tty%d", cons);
      if((fd = open(consnam, 0)) < 0) 
	{
	  perror(consnam);
	  fd = 0;
	}

      if (ioctl(fd,TIOCGWINSZ,&win)) 
	{
	  perror("TIOCGWINSZ");
	  exit(1);
	}

      screenbuf = (char*)malloc(2 + win.ws_row * win.ws_col);
      if (!screenbuf) 
	{
	  fprintf(stderr, _("Out of memory.\n"));
	  exit(1);
	}

      screenbuf[0] = 0;
      screenbuf[1] = (unsigned char) cons;
      
      if (ioctl(fd,TIOCLINUX,screenbuf) &&
	  (!fd || ioctl(0,TIOCLINUX,screenbuf))) 
	{
	  perror("TIOCLINUX");
	  fprintf(stderr,_("couldn't read %s, and cannot ioctl dump\n"),
		  infile);
	  exit(1);
	}

      rows = screenbuf[0];
      cols = screenbuf[1];
      if (rows != win.ws_row || cols != win.ws_col) 
	{
	  fprintf(stderr, _("Strange ... screen is both %dx%d and %dx%d ??\n"),
		  win.ws_col, win.ws_row, cols, rows);
	  exit(1);
	}
      
      outbuf = (char*)malloc(rows*(cols+1));
      if(!outbuf) 
	{
	  fprintf(stderr, _("Out of memory?\n"));
	  exit(1);
	}
      p = screenbuf + 2;
      q = outbuf;
      for (i=0; i<rows; i++) 
	{
	  for (j=0; j<cols; j++)
	      *q++ = *p++;
	  while (j-- > 0 && (q[-1] == ' '))
              q--;
	  *q++ = '\n';
        }
    }
  done:
  if (write(1, outbuf, q-outbuf) != q-outbuf) 
    {
      fprintf(stderr, _("Error writing screen dump\n")); /* write failed */
      exit(1);
    }
  exit(0);
}
