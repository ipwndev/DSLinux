/*
 * psfgettable.c
 *
 * Extract a Unicode character table from a PSF font
 *
 * Copyright (C) 1994 H. Peter Anvin
 *
 * This program may be freely copied under the terms of the GNU
 * General Public License (GPL), version 2, or at your option
 * any later version.
 *
 * 1997/08, ydi: moved reusable parts into psf.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <string.h>
#include <getopt.h>

#include <lct/font.h>
#include <lct/local.h>

static void usage(char *progname)
{
  printf(_("Usage: %s psffont chartable [outfile]\n"
	   "Extract a Unicode character table from a PSF font\n"), progname);
  OPTIONS_ARE();
  OPT("-f --font=f       ", _("psffont filename"));
  OPT("-o --outfile=f    ", _("output filename"));
  
  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}

static void parse_cmdline (int argc, char *argv[],
			   char **infile, char **outfile)
{
  char *progname = strip_path (argv[0]);
  const struct option long_opts[] = {
    { "outfile", 1, NULL, 'o' },
    { "font", 1, NULL, 'f' },
    { "version" , 0 , NULL, 'V'},
    { "help", 0, NULL, 'h' },
    { NULL, 0, NULL, 0 }
  };
  int c;

  *infile = NULL;
  *outfile = NULL;

  while ( (c = getopt_long (argc, argv, "Vhf:o:", long_opts, NULL)) != EOF) {
    switch (c) {
    case 'h':
      usage(progname);
      exit(0);
    case 'V':
      version(progname);
      exit(0);
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
  
  if (!*outfile) {
    if (optind < argc ) 
      *outfile = argv[optind]; 
    else
      *outfile = "stdout";
  }

  /* handle stdin */
  if (!strcmp(*infile,"-"))
    *infile = "stdin";
}
    

int main(int argc, char *argv[])
{
  FILE *in, *out;
  char *inname, *outname;
  struct psf_header psfhdr;
  int glyph;
  unicode unichar;
  int fontlen;

  setuplocale();
  
  parse_cmdline (argc, argv, &inname, &outname);
  
  if (!strcmp(inname,"stdin"))
    in = stdin;  
  else
    {
      in = fopen(inname, "r");
      if ( !in )
        {
          perror(inname);
          exit(EX_NOINPUT);
        }
    }

  if (!strcmp(outname,"stdout"))
    out = stdout;
  else
    {
      out = fopen(outname, "w");
      if ( !out )
        {
          perror(outname);
          exit(EX_CANTCREAT);
        }
    }

  if (-1 == psf_header_read (in, &psfhdr))
      perror("psf_header_read"), exit(EX_DATAERR);
  
  fontlen = ( PSF_MODE_HAS512(psfhdr.mode) ) ? 512 : 256;

  if ( ! (PSF_MODE_HASSFM(psfhdr.mode)) )
    {
      fprintf(stderr, _("%s: Font has no character table\n"), inname);
      exit(EX_DATAERR);
    }

    /* Skip font data */
  if ( fseek(in, psfhdr.charheight * fontlen, SEEK_CUR) == -1)
    {
      perror(inname);
      exit(EX_DATAERR);
    }

  /* Copy table */

  fprintf(out, "#\n# Character table extracted from font %s\n#\n", inname);

  for ( glyph = 0 ; glyph < fontlen ; glyph++ )
    {
      if ( fontlen <= 256 )
	fprintf(out, "0x%02x\t", glyph);
      else
	fprintf(out, "0x%03x\t", glyph);

      while ( fread(&unichar, sizeof(unicode), 1, in),
	      unichar != PSF_SEPARATOR )
	fprintf(out, " U+%04x", unichar);

      putc('\n', out);
    }

  fclose(in);
  fclose(out);

  exit(EX_OK);
}
