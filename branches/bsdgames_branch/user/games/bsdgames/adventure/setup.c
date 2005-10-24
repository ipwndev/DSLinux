/* $NetBSD: setup.c,v 1.10 2003/08/07 09:36:51 agc Exp $ */

/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jim Gillogly at The Rand Corporation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static const char copyright[] __attribute__((__unused__)) = "@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";

#if 0
static char sccsid[] = "@(#)setup.c	8.1 (Berkeley) 5/31/93";
#else
static const char rcsid[] __attribute__((__unused__)) = "$NetBSD: setup.c,v 1.10 2003/08/07 09:36:51 agc Exp $";
#endif
#endif				/* not lint */

/*
 * Setup: keep the structure of the original Adventure port, but use an
 * internal copy of the data file, serving as a sort of virtual disk.  It's
 * lightly encrypted to prevent casual snooping of the executable.
 *
 * Also do appropriate things to tabs so that bogus editors will do the right
 * thing with the data file.
 *
 */

#define SIG1 " *      Jim Gillogly"
#define SIG2 " *      Sterday, 6 Thrimidge S.R. 1993, 15:24"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "hdr.h"		/* SEED lives in there; keep them coordinated. */

#define USAGE "Usage: setup file > data.c (file is typically glorkz)\n"

#define YES 1
#define NO  0

#define LINE 10			/* How many values do we get on a line? */

int main(int, char *[]);

int
main(argc, argv)
	int     argc;
	char   *argv[];
{
	FILE   *infile;
	int     c, count, linestart;

	if (argc != 2) {
		fprintf(stderr, USAGE);
		exit(1);
	}

	if ((infile = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "Can't read file %s: %s\n", argv[1],
		    strerror(errno));
		exit(1);
	}
	puts("/*\n * data.c: created by setup from the ascii data file.");
	puts(SIG1);
	puts(SIG2);
	puts(" */");
	printf("\n\nchar data_file[] =\n{");
	srandom(SEED);
	count = 0;
	linestart = YES;

	while ((c = getc(infile)) != EOF) {
		if (linestart && c == ' ') {	/* Convert first spaces to tab */
			printf("0x%02x,", (unsigned int)('\t' ^ random()) & 0xFF);
			while ((c = getc(infile)) == ' ' && c != EOF);
			/* Drop the non-whitespace character through */
			linestart = NO;
		}
		switch (c) {
		case '\t':
			linestart = NO;	/* Don't need to convert spaces */
			break;
		case '\n':
			linestart = YES;	/* Ready to convert spaces
						 * again */
			break;
		}
		if (count++ % LINE == 0)	/* Finished a line? */
			printf("\n\t");
		printf("0x%02x,", (unsigned int)(c ^ random()) & 0xFF);
	}
	puts("\n\t0\n};");
	fclose(infile);
	fflush(stdout);
	if (ferror(stdout)) {
		perror("error writing standard output");
		exit(1);
	}
	exit(0);
}
