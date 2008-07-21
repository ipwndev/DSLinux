/*
 * psfstriptable.c
 *
 * Strip Unicode character table from a PSF font
 *
 * Copyright (C) 1994 H. Peter Anvin
 *
 * This program may be freely copied under the terms of the GNU
 * General Public License (GPL), version 2, or at your option
 * any later version.
 *
 * fix, aeb, 970316
 * 1997/08, ydi: moved reusable parts into libconsole
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
  printf(_("Usage: %s [options] psffont [outfile]\n"
	   "Strip Unicode character table from a PSF font\n"), progname);
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
  size_t fontlen;
  char buffer[65536];		/* Font data, is scratch only */

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

  /* open output file */
  if (!strcmp (outname, "stdout"))
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
      fprintf(stderr, _("%s: Font already had no character table\n"), inname);
    }

  if (PSF_MODE_HAS512(psfhdr.mode))
    psfhdr.mode = PSF_MODE512NOSFM;
  else
    psfhdr.mode = PSF_MODE256NOSFM;
  
  /* Read font data */
  if ( fread(buffer, psfhdr.charheight, fontlen, in) < fontlen )
    {
      perror(inname);
      exit(EX_DATAERR);
    }

  fclose(in);

  /* Write new font file */
  fwrite(&psfhdr, sizeof(struct psf_header), 1, out);
  fwrite(buffer, psfhdr.charheight, fontlen, out);
  fclose(out);

  exit(EX_OK);
}
