/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Modified 12 April 1990 by Mark Adler for use on MSDOS systems with
 * Microsoft C and Turbo C.  Standard input problem fixed 29 April 1990
 * as per suggestion by Steve Harrold.
 *
 * Modifed 13 February 1991 by Greg Roelofs for use on VMS systems.
 * Compile and link normally (but note that the shared-image link option
 * produces a binary only 6 blocks long, as opposed to the 152-block one
 * produced by an ordinary link).  To set up the VMS symbol to run the
 * program ("run uuencode filename1 filename2 filename3" won't work), do:
 *		uuencode :== "$disk:[directory]uuencode.exe"
 * and don't forget the leading "$" or it still won't work.  The syntax
 * differs slightly from the Unix and MS-DOS versions since VMS has such
 * an awkward approach to redirection; run the program with no arguments
 * for the usage (or see USAGE below).  The output file is in VMS "stream-
 * LF" format but should be readable by MAIL, ftp, or anything else.
 */

#ifndef lint
static char sccsid[] = "@(#)uuencode.c	5.6 (Berkeley) 7/6/88";
#endif /* not lint */

#ifdef __MSDOS__        /* For Turbo C */
#define MSDOS 1
#endif

/*
 * uuencode [input] output
 *
 * Encode a file so it can be mailed to a remote system.
 */
#include <stdio.h>

#ifdef VMS
#  define OUT out	/* force user to specify output file */
#  define NUM_ARGS 3
#  define USAGE "Usage: uuencode [infile] remotefile uufile\n"
#  include <types.h>
#  include <stat.h>
#else
#  define OUT stdout	/* Unix, MS-DOS:  anybody with decent redirection */
#  define NUM_ARGS 2
#  define USAGE "Usage: uuencode [infile] remotefile\n"
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

#if MSDOS
#include <io.h>
#include <fcntl.h>
#endif

/* ENC is the basic 1-character encoding function to make a char printing */
#define ENC(c) ((c) ? ((c) & 077) + ' ': '`')

main(argc, argv)
char **argv;
{
#ifdef VMS
	FILE *out;
#endif
	FILE *in;
	struct stat sbuf;
	int mode;

	/* optional 1st argument */
	if (argc > NUM_ARGS) {
		if ((in = fopen(argv[1], "r")) == NULL) {
			perror(argv[1]);
			exit(1);
		}
		argv++; argc--;
	} else
		in = stdin;

#if MSDOS
	/* set input file mode to binary for MSDOS systems */
	setmode(fileno(in), O_BINARY);
#endif

	if (argc != NUM_ARGS) {
		fprintf(stderr, USAGE);
		exit(2);
	}

#ifdef VMS   /* mandatory 3rd argument is name of uuencoded file */
	if ((out = fopen(argv[2], "w")) == NULL) {
		perror(argv[2]);
		exit(4);
	}
#endif

	/* figure out the input file mode */
	if (fstat(fileno(in), &sbuf) < 0 || !isatty(fileno(in)))
		mode = 0666 & ~umask(0666);
	else
		mode = sbuf.st_mode & 0777;
	fprintf(OUT, "begin %o %s\n", mode, argv[1]);

	encode(in, OUT);

	fprintf(OUT, "end\n");
	exit(0);
}

/*
 * copy from in to out, encoding as you go along.
 */
encode(in, out)
register FILE *in;
register FILE *out;
{
	char buf[80];
	register int i, n;

	for (;;) {
		/* 1 (up to) 45 character line */
		n = fread(buf, 1, 45, in);
		putc(ENC(n), out);

		for (i=0; i<n; i += 3)
			outdec(&buf[i], out);

		putc('\n', out);
		if (n <= 0)
			break;
	}
}

/*
 * output one group of 3 bytes, pointed at by p, on file f.
 */
outdec(p, f)
register char *p;
register FILE *f;
{
	register int c1, c2, c3, c4;

	c1 = *p >> 2;
	c2 = (*p << 4) & 060 | (p[1] >> 4) & 017;
	c3 = (p[1] << 2) & 074 | (p[2] >> 6) & 03;
	c4 = p[2] & 077;
	putc(ENC(c1), f);
	putc(ENC(c2), f);
	putc(ENC(c3), f);
	putc(ENC(c4), f);
}
