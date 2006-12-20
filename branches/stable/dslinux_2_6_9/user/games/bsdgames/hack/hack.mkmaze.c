/*	$NetBSD: hack.mkmaze.c,v 1.6 2003/04/02 18:36:38 jsm Exp $	*/

/*
 * Copyright (c) 1985, Stichting Centrum voor Wiskunde en Informatica,
 * Amsterdam
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Stichting Centrum voor Wiskunde en
 * Informatica, nor the names of its contributors may be used to endorse or
 * promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1982 Jay Fenlason <hack@gnu.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
__RCSID("$NetBSD: hack.mkmaze.c,v 1.6 2003/04/02 18:36:38 jsm Exp $");
#endif				/* not lint */

#include "hack.h"
#include "extern.h"
#include "def.mkroom.h"		/* not really used */
const struct permonst hell_hound =
{"hell hound", 'd', 12, 14, 2, 3, 6, 0};

void
makemaz()
{
	int             x, y;
	int		zx, zy;
	coord           mm;
	boolean         al = (dlevel >= 30 && !flags.made_amulet);

	for (x = 2; x < COLNO - 1; x++)
		for (y = 2; y < ROWNO - 1; y++)
			levl[x][y].typ = (x % 2 && y % 2) ? 0 : HWALL;
	if (al) {
		struct monst   *mtmp;

		zx = 2 * (COLNO / 4) - 1;
		zy = 2 * (ROWNO / 4) - 1;
		for (x = zx - 2; x < zx + 4; x++)
			for (y = zy - 2; y <= zy + 2; y++) {
				levl[x][y].typ =
					(y == zy - 2 || y == zy + 2 || x == zx - 2 || x == zx + 3) ? POOL :
					(y == zy - 1 || y == zy + 1 || x == zx - 1 || x == zx + 2) ? HWALL :
					ROOM;
			}
		(void) mkobj_at(AMULET_SYM, zx, zy);
		flags.made_amulet = 1;
		walkfrom(zx + 4, zy);
		if ((mtmp = makemon(&hell_hound, zx, zy)) != NULL)
			mtmp->msleep = 1;
		if ((mtmp = makemon(PM_WIZARD, zx + 1, zy)) != NULL) {
			mtmp->msleep = 1;
			flags.no_of_wizards = 1;
		}
	} else {
		mm = mazexy();
		zx = mm.x;
		zy = mm.y;
		walkfrom(zx, zy);
		(void) mksobj_at(WAN_WISHING, zx, zy);
		(void) mkobj_at(ROCK_SYM, zx, zy);	/* put a rock on top of
							 * it */
	}

	for (x = 2; x < COLNO - 1; x++)
		for (y = 2; y < ROWNO - 1; y++) {
			switch (levl[x][y].typ) {
			case HWALL:
				levl[x][y].scrsym = '-';
				break;
			case ROOM:
				levl[x][y].scrsym = '.';
				break;
			}
		}
	for (x = rn1(8, 11); x; x--) {
		mm = mazexy();
		(void) mkobj_at(rn2(2) ? GEM_SYM : 0, mm.x, mm.y);
	}
	for (x = rn1(10, 2); x; x--) {
		mm = mazexy();
		(void) mkobj_at(ROCK_SYM, mm.x, mm.y);
	}
	mm = mazexy();
	(void) makemon(PM_MINOTAUR, mm.x, mm.y);
	for (x = rn1(5, 7); x; x--) {
		mm = mazexy();
		(void) makemon((struct permonst *) 0, mm.x, mm.y);
	}
	for (x = rn1(6, 7); x; x--) {
		mm = mazexy();
		mkgold(0L, mm.x, mm.y);
	}
	for (x = rn1(6, 7); x; x--)
		mktrap(0, 1, (struct mkroom *) 0);
	mm = mazexy();
	levl[(xupstair = mm.x)][(yupstair = mm.y)].scrsym = '<';
	levl[xupstair][yupstair].typ = STAIRS;
	xdnstair = ydnstair = 0;
}

void
walkfrom(x, y)
	int             x, y;
{
	int             q, a, dir;
	int             dirs[4];
	levl[x][y].typ = ROOM;
	while (1) {
		q = 0;
		for (a = 0; a < 4; a++)
			if (okay(x, y, a))
				dirs[q++] = a;
		if (!q)
			return;
		dir = dirs[rn2(q)];
		move(&x, &y, dir);
		levl[x][y].typ = ROOM;
		move(&x, &y, dir);
		walkfrom(x, y);
	}
}

void
move(x, y, dir)
	int            *x, *y;
	int             dir;
{
	switch (dir) {
	case 0:
		--(*y);
		break;
	case 1:
		(*x)++;
		break;
	case 2:
		(*y)++;
		break;
	case 3:
		--(*x);
		break;
	}
}

int
okay(x, y, dir)
	int             x, y;
	int             dir;
{
	move(&x, &y, dir);
	move(&x, &y, dir);
	if (x < 3 || y < 3 || x > COLNO - 3 || y > ROWNO - 3 || levl[x][y].typ != 0)
		return (0);
	else
		return (1);
}

coord
mazexy()
{
	coord           mm;
	mm.x = 3 + 2 * rn2(COLNO / 2 - 2);
	mm.y = 3 + 2 * rn2(ROWNO / 2 - 2);
	return mm;
}
