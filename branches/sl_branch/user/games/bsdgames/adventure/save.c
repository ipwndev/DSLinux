/*	$NetBSD: save.c,v 1.8 2003/08/07 09:36:51 agc Exp $	*/

/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * The game adventure was originally written in Fortran by Will Crowther
 * and Don Woods.  It was later translated to C and enhanced by Jim
 * Gillogly.  This code is derived from software contributed to Berkeley
 * by Jim Gillogly at The Rand Corporation.
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

#include <sys/cdefs.h>
#ifndef lint
#if 0
static char sccsid[] = "@(#)save.c	8.1 (Berkeley) 5/31/93";
#else
__RCSID("$NetBSD: save.c,v 1.8 2003/08/07 09:36:51 agc Exp $");
#endif
#endif				/* not lint */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include "hdr.h"
#include "extern.h"

struct savestruct {
	void   *address;
	int     width;
};

struct savestruct save_array[] =
{
	{&abbnum, sizeof(abbnum)},
	{&attack, sizeof(attack)},
	{&blklin, sizeof(blklin)},
	{&bonus, sizeof(bonus)},
	{&chloc, sizeof(chloc)},
	{&chloc2, sizeof(chloc2)},
	{&clock1, sizeof(clock1)},
	{&clock2, sizeof(clock2)},
	{&closed, sizeof(closed)},
	{&closng, sizeof(closng)},
	{&daltlc, sizeof(daltlc)},
	{&demo, sizeof(demo)},
	{&detail, sizeof(detail)},
	{&dflag, sizeof(dflag)},
	{&dkill, sizeof(dkill)},
	{&dtotal, sizeof(dtotal)},
	{&foobar, sizeof(foobar)},
	{&gaveup, sizeof(gaveup)},
	{&holdng, sizeof(holdng)},
	{&iwest, sizeof(iwest)},
	{&k, sizeof(k)},
	{&k2, sizeof(k2)},
	{&knfloc, sizeof(knfloc)},
	{&kq, sizeof(kq)},
	{&latncy, sizeof(latncy)},
	{&limit, sizeof(limit)},
	{&lmwarn, sizeof(lmwarn)},
	{&loc, sizeof(loc)},
	{&maxdie, sizeof(maxdie)},
	{&mxscor, sizeof(mxscor)},
	{&newloc, sizeof(newloc)},
	{&numdie, sizeof(numdie)},
	{&obj, sizeof(obj)},
	{&oldlc2, sizeof(oldlc2)},
	{&oldloc, sizeof(oldloc)},
	{&panic, sizeof(panic)},
	{&saveday, sizeof(saveday)},
	{&savet, sizeof(savet)},
	{&scorng, sizeof(scorng)},
	{&spk, sizeof(spk)},
	{&stick, sizeof(stick)},
	{&tally, sizeof(tally)},
	{&tally2, sizeof(tally2)},
	{&tkk, sizeof(tkk)},
	{&turns, sizeof(turns)},
	{&verb, sizeof(verb)},
	{&wd1, sizeof(wd1)},
	{&wd2, sizeof(wd2)},
	{&wzdark, sizeof(wzdark)},
	{&yea, sizeof(yea)},
	{atloc, sizeof(atloc)},
	{dloc, sizeof(dloc)},
	{dseen, sizeof(dseen)},
	{fixed, sizeof(fixed)},
	{hinted, sizeof(hinted)},
	{links, sizeof(links)},
	{odloc, sizeof(odloc)},
	{place, sizeof(place)},
	{prop, sizeof(prop)},
	{tk, sizeof(tk)},

	{NULL, 0}
};

int
save(outfile)			/* Two passes on data: first to get checksum,
				 * second */
	const char   *outfile;	/* to output the data using checksum to start
				 * random #s */
{
	FILE   *out;
	struct savestruct *p;
	char   *s;
	long    sum;
	int     i;

	crc_start();
	for (p = save_array; p->address != NULL; p++)
		sum = crc(p->address, p->width);
	srandom((int) sum);

	if ((out = fopen(outfile, "wb")) == NULL) {
		fprintf(stderr,
		    "Hmm.  The name \"%s\" appears to be magically blocked.\n",
		    outfile);
		return 1;
	}
	fwrite(&sum, sizeof(sum), 1, out);	/* Here's the random() key */
	for (p = save_array; p->address != NULL; p++) {
		for (s = p->address, i = 0; i < p->width; i++, s++)
			*s = (*s ^ random()) & 0xFF;	/* Lightly encrypt */
		fwrite(p->address, p->width, 1, out);
	}
	if (fclose(out) != 0) {
		warn("writing %s", outfile);
		return 1;
	}
	return 0;
}

int
restore(infile)
	const char   *infile;
{
	FILE   *in;
	struct savestruct *p;
	char   *s;
	long    sum, cksum = 0;
	int     i;

	if ((in = fopen(infile, "rb")) == NULL) {
		fprintf(stderr,
		    "Hmm.  The file \"%s\" appears to be magically blocked.\n",
		    infile);
		return 1;
	}
	fread(&sum, sizeof(sum), 1, in);	/* Get the seed */
	srandom((int) sum);
	for (p = save_array; p->address != NULL; p++) {
		fread(p->address, p->width, 1, in);
		for (s = p->address, i = 0; i < p->width; i++, s++)
			*s = (*s ^ random()) & 0xFF;	/* Lightly decrypt */
	}
	fclose(in);

	crc_start();		/* See if she cheated */
	for (p = save_array; p->address != NULL; p++)
		cksum = crc(p->address, p->width);
	if (sum != cksum)	/* Tsk tsk */
		return 2;	/* Altered the file */
	/* We successfully restored, so this really was a save file */
	/* Get rid of the file, but don't bother checking that we did */
	return 0;
}
