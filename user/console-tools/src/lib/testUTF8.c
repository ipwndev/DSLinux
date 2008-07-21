/* (c) Ricardas Cepas <rch@pub.osf.lt>. Copying policy: GNU GPL V2. */
/* slightly modified for inclusion in console-tools by YDI */
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sysexits.h>

/*
 * return:
 *  0: 8-bit mode
 *  1: UTF-8 mode
 * 
 * -1: error - see errno for details 
 *  ENOTTY: not a tty
 *  EIO:    could not read/write from/to console
 */
int is_in_UTF8_mode (int fd)
{
  unsigned int y, x;
  struct termios termios_orig, termios;
  FILE* f;
  
  if (isatty (fd))
    {
      f = fdopen (fd, "a+");
      if (!f)
	{
	  perror ("is_in_UTF8_mode");
	  exit (EX_SOFTWARE);
	}
      
      tcgetattr (fd, &termios_orig);
      termios = termios_orig;
      tcflush (fd, TCIOFLUSH);
      cfmakeraw (&termios);
      tcsetattr (fd, TCSANOW, &termios);

      /* - ^X^Z cancel any ESC sequence
       * - "\357\200\240" = U+F020 =
       *  `space' in Linux's straight-to-font zone
       * - ask cursor position
       */
      fprintf (f, "\030\032" "\r\357\200\240" "\033[6n\033D");
      /* get cursor position; set error if no answer */
      if (2 != fscanf (f, "\033[%u;%u", &y, &x))
	{
	  errno = EIO;
	  return -1;
	}
      tcsetattr (fd, TCSANOW, &termios_orig);
      /* go back; erase 1 or 3 char */
      fprintf(f, "\033[1F" "\033[%uX", (x-1));
      fflush (f);
  
      /*Get a single byte in UTF-8 and 3 bytes othewise */
      switch (x)
	{
	case 2: /* UTF-8 */
	  x=1;
	  break;
	  
	case 4: /* single-byte mode */
	  x=0;
	  break;
	  
	default: /* error */
	  x=-1;
	}
    }
  else
    {
      errno = ENOTTY;
      x = -1;
    }
  
  return (x);
}

