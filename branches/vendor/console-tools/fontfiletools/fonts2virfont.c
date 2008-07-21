/*
 * Usage:
 * 
 *  fonts2virfont { font [ + unimap ] }* > virfontfile
 * 
 */

#include <stdio.h>
#include <stdlib.h>

#include <lct/local.h>
#include "virfont.h"

char* progname;
struct unifont* fonts;
int fontscount;


void usage()
{
  version();
  
  fprintf (stderr, 
	   "Usage: "
	   " %s font [ + unimap ] [ font [ + ... ]] ... \n\n",
	   progname);
}


int parse_command_line(int argc, char* argv[], int do_fill)
{
  int i, my_fontscount;
  int seq_read;   /* how much was read in a font [ + unimap ] sequence */

  /* scan args for fonts count */
  for (i = 1, my_fontscount = 0, seq_read = 0; i < argc; i++)
    {
      if (strcmp(argv[i], "+"))
	{
	  switch (seq_read)
	    {
	    case 0:  /* font name */
	    case 1: /* is another font, instead of "+" */
	      my_fontscount++;
	      seq_read = 1;
	      if (do_fill)
		{
		  fonts[my_fontscount-1].fontfilename = argv[i];
		}
	      break;
	    case 2:  /* unimap name */
	      seq_read = 0;
	      if (do_fill)
		{
		  fonts[my_fontscount-1].mapfilename = argv[i];
		}
	      break;
	    default:
	      fprintf (stderr, "Wow, compiler bug !\nExiting...\n");
	      exit (EX_SOFTWARE);
	    }
	}
      else /* "+" */
	{
	  switch (seq_read)
	    {
	    case 1:  /* just read a font name, next will be unimap */
	      seq_read ++;
	      break;
	    default: /* "+" as unimap name, or as font name */
	      badusage("misplaced `+' character on command line");
	    }
	}
    }
  
  return my_fontscount;
}


void dumpfonts()
{
  int i;
  
  for (i=0; i < fontscount; i++)
      fprintf (stderr, "%s + %s\n", fonts[i].fontfilename, fonts[i].mapfilename);
}

  
void main (int argc, char* argv[])
{
  int i;
  struct virchar* virfont;
  
  progname = strip_path(argv[0]);
  

  /* get input fonts count */
  fontscount = parse_command_line (argc, argv, 0);
  
  if ((argc==1) || (fontscount < 2))
      badusage("wrong number of args");

  
  /* base storage structure */
  fonts = (struct unifont*)calloc(fontscount, sizeof (struct unifont));
  
  /* fill it */
  parse_command_line (argc, argv, 1);

  dumpfonts();
  
  exit (EX_OK);
}
