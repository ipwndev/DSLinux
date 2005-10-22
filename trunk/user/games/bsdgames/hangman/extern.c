/*	$NetBSD: extern.c,v 1.8 2003/08/07 09:37:20 agc Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
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
static char sccsid[] = "@(#)extern.c	8.1 (Berkeley) 5/31/93";
#else
__RCSID("$NetBSD: extern.c,v 1.8 2003/08/07 09:37:20 agc Exp $");
#endif
#endif /* not lint */

#include	"hangman.h"

bool    Guessed[26];

char    Word[BUFSIZ], Known[BUFSIZ];
const char *const Noose_pict[] = {
	"     ______",
	"     |    |",
	"     |",
	"     |",
	"     |",
	"     |",
	"   __|_____",
	"   |      |___",
	"   |_________|",
	NULL
};

int     Errors, Wordnum = 0;
unsigned int Minlen = MINLEN;

double  Average = 0.0;

const ERR_POS Err_pos[MAXERRS] = {
	{2, 10, 'O'},
	{3, 10, '|'},
	{4, 10, '|'},
	{5, 9, '/'},
	{3, 9, '/'},
	{3, 11, '\\'},
	{5, 11, '\\'}
};

const char *Dict_name = _PATH_DICT;

FILE   *Dict = NULL;

off_t   Dict_size;
