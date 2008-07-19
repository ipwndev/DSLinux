/* Base64 encode/decode strings or files.
   Copyright (C) 2004, 2005, 2006 Free Software Foundation, Inc.

   This file is part of Base64.

   Base64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Base64 is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Base64; see the file COPYING.  If not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA. */

/* Written by Simon Josefsson <simon@josefsson.org>.  */

#include <config.h>

#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>

#include "system.h"
#include "error.h"
#include "xstrtol.h"
#include "quote.h"
#include "quotearg.h"

#include "base64.h"

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "base64"

#define AUTHOR "Simon Josefsson"

/* The invocation name of this program.  */
char *program_name;

static const struct option long_options[] = {
  {"decode", no_argument, 0, 'd'},
  {"wrap", required_argument, 0, 'w'},
  {"ignore-garbage", no_argument, 0, 'i'},
  {"help", no_argument, 0, GETOPT_HELP_CHAR},
  {"version", no_argument, 0, GETOPT_VERSION_CHAR},

  {GETOPT_HELP_OPTION_DECL},
  {GETOPT_VERSION_OPTION_DECL},
  {NULL, 0, NULL, 0}
};

static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try `%s --help' for more information.\n"),
	     program_name);
  else
    {
      printf (_("\
Usage: %s [OPTION] [FILE]\n\
Base64 encode or decode FILE, or standard input, to standard output.\n\
\n"), program_name);
      fputs (_("\
  -w, --wrap=COLS       Wrap encoded lines after COLS character (default 76).\n\
                        Use 0 to disable line wrapping.\n\
\n\
  -d, --decode          Decode data.\n\
  -i, --ignore-garbage  When decoding, ignore non-alphabet characters.\n\
\n\
"), stdout);
      fputs (_("\
      --help            Display this help and exit.\n\
      --version         Output version information and exit.\n"), stdout);
      fputs (_("\
\n\
With no FILE, or when FILE is -, read standard input.\n"), stdout);
      fputs (_("\
\n\
The data are encoded as described for the base64 alphabet in RFC 3548.\n\
Decoding require compliant input by default, use --ignore-garbage to\n\
attempt to recover from non-alphabet characters (such as newlines) in\n\
the encoded stream.\n"), stdout);
      printf (_("\nReport bugs to <%s>.\n"), PACKAGE_BUGREPORT);
    }

  exit (status);
}

/* Note that increasing this may decrease performance if --ignore-garbage
   is used, because of the memmove operation below. */
#define BLOCKSIZE 3072
#define B64BLOCKSIZE BASE64_LENGTH (BLOCKSIZE)

/* Ensure that BLOCKSIZE is a multiple of 3 and 4.  */
#if BLOCKSIZE % 12 != 0
# error "invalid BLOCKSIZE"
#endif

static void
wrap_write (const char *buffer, size_t len,
	    uintmax_t wrap_column, size_t *current_column, FILE *out)
{
  size_t written;

  if (wrap_column == 0)
    {
      /* Simple write. */
      if (fwrite (buffer, 1, len, stdout) < len)
	error (EXIT_FAILURE, errno, _("write error"));
    }
  else
    for (written = 0; written < len;)
      {
	uintmax_t cols_remaining = wrap_column - *current_column;
	size_t to_write = MIN (cols_remaining, SIZE_MAX);
	to_write = MIN (to_write, len - written);

	if (to_write == 0)
	  {
	    if (fputs ("\n", out) < 0)
	      error (EXIT_FAILURE, errno, _("write error"));
	    *current_column = 0;
	  }
	else
	  {
	    if (fwrite (buffer + written, 1, to_write, stdout) < to_write)
	      error (EXIT_FAILURE, errno, _("write error"));
	    *current_column += to_write;
	    written += to_write;
	  }
      }
}

static void
do_encode (FILE *in, FILE *out, uintmax_t wrap_column)
{
  size_t current_column = 0;
  char inbuf[BLOCKSIZE];
  char outbuf[B64BLOCKSIZE];
  size_t sum;

  do
    {
      size_t n;

      sum = 0;
      do
	{
	  n = fread (inbuf + sum, 1, BLOCKSIZE - sum, in);
	  sum += n;
	}
      while (!feof (in) && !ferror (in) && sum < BLOCKSIZE);

      if (sum > 0)
	{
	  /* Process input one block at a time.  Note that BLOCKSIZE %
	     3 == 0, so that no base64 pads will appear in output. */
	  base64_encode (inbuf, sum, outbuf, BASE64_LENGTH (sum));

	  wrap_write (outbuf, BASE64_LENGTH (sum), wrap_column,
		      &current_column, out);
	}
    }
  while (!feof (in) && !ferror (in) && sum == BLOCKSIZE);

  /* When wrapping, terminate last line. */
  if (wrap_column && current_column > 0 && fputs ("\n", out) < 0)
    error (EXIT_FAILURE, errno, _("write error"));

  if (ferror (in))
    error (EXIT_FAILURE, errno, _("read error"));
}

static void
do_decode (FILE *in, FILE *out, bool ignore_garbage)
{
  char inbuf[B64BLOCKSIZE];
  char outbuf[BLOCKSIZE];
  size_t sum;

  do
    {
      bool ok;
      size_t n;

      sum = 0;
      do
	{
	  n = fread (inbuf + sum, 1, B64BLOCKSIZE - sum, in);

	  if (ignore_garbage)
	    {
	      size_t i;
	      for (i = 0; n > 0 && i < n;)
		if (isbase64 (inbuf[sum + i]) || inbuf[sum + i] == '=')
		  i++;
		else
		  memmove (inbuf + sum + i, inbuf + sum + i + 1, --n - i);
	    }

	  sum += n;

	  if (ferror (in))
	    error (EXIT_FAILURE, errno, _("read error"));
	}
      while (sum < B64BLOCKSIZE && !feof (in));

      n = BLOCKSIZE;
      ok = base64_decode (inbuf, sum, outbuf, &n);

      if (fwrite (outbuf, 1, n, out) < n)
	error (EXIT_FAILURE, errno, _("write error"));

      if (!ok)
	error (EXIT_FAILURE, 0, _("invalid input"));
    }
  while (!feof (in));
}

int
main (int argc, char **argv)
{
  int opt;
  FILE *input_fh;
  const char *infile;

  /* True if --decode has bene given and we should decode data. */
  bool decode = false;
  /* True if we should ignore non-alphabetic characters. */
  bool ignore_garbage = false;
  /* Wrap encoded base64 data around the 76:th column, by default. */
  uintmax_t wrap_column = 76;

  initialize_main (&argc, &argv);
  program_name = argv[0];
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  atexit (close_stdout);

  while ((opt = getopt_long (argc, argv, "dqiw:", long_options, NULL)) != -1)
    switch (opt)
      {
      case 'd':
	decode = true;
	break;

      case 'w':
	if (xstrtoumax (optarg, NULL, 0, &wrap_column, NULL) != LONGINT_OK)
	  error (EXIT_FAILURE, 0, _("invalid wrap size: %s"),
		 quotearg (optarg));
	break;

      case 'i':
	ignore_garbage = true;
	break;

	case_GETOPT_HELP_CHAR;

	case_GETOPT_VERSION_CHAR (PROGRAM_NAME, AUTHOR);

      default:
	usage (EXIT_FAILURE);
	break;
      }

  if (argc - optind > 1)
    {
      error (0, 0, _("extra operand %s"), quote (argv[optind]));
      usage (EXIT_FAILURE);
    }

  if (optind < argc)
    infile = argv[optind];
  else
    infile = "-";

  if (strcmp (infile, "-") == 0)
    input_fh = stdin;
  else
    {
      input_fh = fopen (infile, "r");
      if (input_fh == NULL)
	error (EXIT_FAILURE, errno, "%s", infile);
    }

  if (decode)
    do_decode (input_fh, stdout, ignore_garbage);
  else
    do_encode (input_fh, stdout, wrap_column);

  if (fclose (input_fh) == EOF)
    {
      if (strcmp (infile, "-") == 0)
	error (EXIT_FAILURE, errno, _("closing standard input"));
      else
	error (EXIT_FAILURE, errno, "%s", infile);
    }

  exit (EXIT_SUCCESS);
}
