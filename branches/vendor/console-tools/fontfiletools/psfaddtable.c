/*
 * psfaddtable.c
 *
 * Add a Unicode character table to a PSF font
 *
 * Copyright (C) 1994 H. Peter Anvin
 *
 * This program may be freely copied under the terms of the GNU
 * General Public License (GPL), version 2, or at your option
 * any later version.
 *
 * Added input ranges, aeb.
 * 1997/08, ydi: moved reusable parts into psf.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

#include <lct/font.h>
#include <lct/local.h>

int verbose;

static void usage(char *progname)
{
  printf(_("Usage: %s psffont_in sfm_file [psffont_out]\n"
	   "Add a Unicode character table to a PSF font\n"), progname);
  OPTIONS_ARE();
  OPT("-f --font=f       ", _("input psffont filename"));
  OPT("-s --sfm=f        ", _("screen-font-map filename"));
  OPT("-o --outfile=f    ", _("output psffont filename"));

  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}

static void parse_cmdline (int argc, char *argv[],
			   char **infile, char **tblname, char **outfile)
{
  char *progname = strip_path (argv[0]);
  const struct option long_opts[] = {
    { "sfm", 1, NULL, 's' },
    { "outfile", 1, NULL, 'o' },
    { "font", 1, NULL, 'f' },
    { "version" , 0 , NULL, 'V'},
    { "help", 0, NULL, 'h' },
    { NULL, 0, NULL, 0 }
  };
  int c;

  *infile = NULL;
  *outfile = NULL;
  *tblname = NULL;

  while ( (c = getopt_long (argc, argv, "Vhs:f:o:", long_opts, NULL)) != EOF) {
    switch (c) {
    case 'h':
      usage(progname);
      exit(0);
    case 'V':
      version(progname);
      exit(0);
    case 's':
      *tblname = optarg;
      break;
    case 'o':
      *outfile = optarg;
      break;
    case 'f':
      *infile = optarg;
      break;
    case '?':
      usage(progname);
      exit(1);
    }
  }
  
  if (!*infile) {
    if (optind < argc) {
      *infile = argv[optind++];
    } else {
      fprintf (stderr, _("%s: wrong number of arguments\n"), 
	       progname);
      exit (1);
    }
  }
  
  if (!*tblname) {
    if (optind < argc ) {
      *tblname = argv[optind++];
    } else {
      fprintf (stderr, _("%s: wrong number of arguments\n"),
	       progname);
      exit (1);
    }
  }

  if (!*outfile) {
    if (optind < argc ) 
      *outfile = argv[optind]; 
    else
      *outfile = "stdout";
  }

  /* handle stdin */
  if (!strcmp(*infile,"-"))
    *infile = "stdin";
  if (!strcmp(*tblname,"-") && !strcmp(*infile,"stdin")) {
    fprintf (stderr, _("%s: psffont and chartable cannot both be stdin\n"),
	     progname);
    exit(1);
  }
  if (!strcmp (*tblname,"-"))
    *tblname = "stdin";
}
    
    
int main(int argc, char *argv[])
{
  FILE *in, *ctbl, *out, *mapf;
  char pathname[1024];
  char *inname, *tblname, *outfile;
  char buffer[65536];
  struct unimapdesc map;
/*  struct unicode_list *unilist[512];*/
  struct psf_header psfhdr;
  size_t fontlen;

  setuplocale();
  
  parse_cmdline (argc, argv, &inname, &tblname, &outfile);

  /* open font */
  if (!strcmp (inname,"stdin"))
    in = stdin;
  else {
    in = fopen(inname, "r");
    if ( !in )
      {
	perror(inname);
	exit(EX_NOINPUT);
      }
  }

  /* open table file */
  if (!strcmp (tblname,"stdin"))
    ctbl = stdout;
  else {
    ctbl = fopen(tblname, "r");
    if ( !ctbl )
      {
	perror(tblname);
	exit(EX_NOINPUT);
      }
  }
  
  /* open output file */
  if (!strcmp (outfile,"stdout"))
    out = stdout;
  else
    {
      out = fopen(outfile, "w");
      if ( !out )
	{
	  perror(outfile);
	  exit(EX_CANTCREAT);
	}
    }

  if (-1 == psf_header_read (in, &psfhdr))
      perror("psf_header_read"), exit(EX_DATAERR);
  
  fontlen = ( PSF_MODE_HAS512(psfhdr.mode) ) ? 512 : 256;

  /* Copy font data */
  if ( fread(buffer, psfhdr.charheight, fontlen, in) < fontlen )
    {
      perror(inname);
      exit(EX_DATAERR);
    }
  fclose(in);			/* Done with input */

  /* Set has-table bit in mode field, and copy to output */

  if (PSF_MODE_HAS512(psfhdr.mode))
    psfhdr.mode = PSF_MODE512SFM;
  else
    psfhdr.mode = PSF_MODE256SFM;
  fwrite(&psfhdr, sizeof(struct psf_header), 1, out);
  fwrite(buffer, psfhdr.charheight, fontlen, out);

  if (NULL == (mapf = findsfm(tblname, pathname, sizeof(pathname),
			      stdin, NULL)))
      perror("findsfm"), exit (1);
  
  if (verbose) fprintf (stderr, _("Loading unicode map from %s.\n"), pathname);
  if (sfm_read_ascii (mapf, &map, fontlen))
      perror ("sfm_read_ascii"), exit (EX_DATAERR);
  
  /* Okay, now glyph table should be read */

  fclose(ctbl);

  sfm_write_binary (out, &map, fontlen);
      
  fclose(out);

  exit(EX_OK);
}
