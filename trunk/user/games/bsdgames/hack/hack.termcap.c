/*	$NetBSD: hack.termcap.c,v 1.12 2003/04/02 18:36:40 jsm Exp $	*/
/* For Linux: still using old termcap interface from version 1.9.  */

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
__RCSID("$NetBSD: hack.termcap.c,v 1.12 2003/04/02 18:36:40 jsm Exp $");
#endif				/* not lint */

#include <string.h>
#include <termios.h>
#include <termcap.h>
#include <stdlib.h>
#include <unistd.h>
#include "hack.h"
#include "extern.h"
#include "def.flag.h"		/* for flags.nonull */

static char     tbuf[512];
char           *HO, *CL, *CE, *UP, *CM, *ND, *XD, *BC, *SO, *SE, *TI, *TE;
static char    *VS, *VE;
static int      SG;
char            hack_PC = '\0';
char           *CD;		/* tested in pri.c: docorner() */
int             CO, LI;		/* used in pri.c and whatis.c */

void
startup()
{
	char           *term;
	char           *tptr;
	char           *tbufptr, *pc;

	tptr = (char *) alloc(1024);

	tbufptr = tbuf;
	if (!(term = getenv("TERM")))
		error("Can't get TERM.");
	if (!strncmp(term, "5620", 4))
		flags.nonull = 1;	/* this should be a termcap flag */
	if (tgetent(tptr, term) < 1)
		error("Unknown terminal type: %s.", term);
	if ((pc = tgetstr("pc", &tbufptr)) != NULL)
		hack_PC = *pc;
	if (!(BC = tgetstr("bc", &tbufptr))) {
		if (!tgetflag("bs"))
			error("Terminal must backspace.");
		BC = tbufptr;
		tbufptr += 2;
		*BC = '\b';
	}
	HO = tgetstr("ho", &tbufptr);
	CO = tgetnum("co");
	LI = tgetnum("li");
	if (CO < COLNO || LI < ROWNO + 2)
		setclipped();
	if (!(CL = tgetstr("cl", &tbufptr)))
		error("Hack needs CL.");
	ND = tgetstr("nd", &tbufptr);
	if (tgetflag("os"))
		error("Hack can't have OS.");
	CE = tgetstr("ce", &tbufptr);
	UP = tgetstr("up", &tbufptr);
	/*
	 * It seems that xd is no longer supported, and we should use a
	 * linefeed instead; unfortunately this requires resetting CRMOD, and
	 * many output routines will have to be modified slightly. Let's
	 * leave that till the next release.
	 */
	XD = tgetstr("xd", &tbufptr);
	/* not: 		XD = tgetstr("do", &tbufptr); */
	if (!(CM = tgetstr("cm", &tbufptr))) {
		if (!UP && !HO)
			error("Hack needs CM or UP or HO.");
		printf("Playing hack on terminals without cm is suspect...\n");
		getret();
	}
	SO = tgetstr("so", &tbufptr);
	SE = tgetstr("se", &tbufptr);
	SG = tgetnum("sg");	/* -1: not fnd; else # of spaces left by so */
	if (!SO || !SE || (SG > 0))
		SO = SE = 0;
	CD = tgetstr("cd", &tbufptr);
	set_whole_screen();	/* uses LI and CD */
	if (tbufptr - tbuf > (int)sizeof(tbuf))
		error("TERMCAP entry too big...\n");
	free(tptr);
}

void
start_screen()
{
	xputs(TI);
	xputs(VS);
}

void
end_screen()
{
	xputs(VE);
	xputs(TE);
}

/* Cursor movements */
void
curs(x, y)
	int             x, y;	/* not xchar: perhaps xchar is unsigned and
				 * curx-x would be unsigned as well */
{

	if (y == cury && x == curx)
		return;
	if (!ND && (curx != x || x <= 3)) {	/* Extremely primitive */
		cmov(x, y);	/* bunker!wtm */
		return;
	}
	if (abs(cury - y) <= 3 && abs(curx - x) <= 3)
		nocmov(x, y);
	else if ((x <= 3 && abs(cury - y) <= 3) || (!CM && x < abs(curx - x))) {
		(void) putchar('\r');
		curx = 1;
		nocmov(x, y);
	} else if (!CM) {
		nocmov(x, y);
	} else
		cmov(x, y);
}

void
nocmov(x, y)
	int x, y;
{
	if (cury > y) {
		if (UP) {
			while (cury > y) {	/* Go up. */
				xputs(UP);
				cury--;
			}
		} else if (CM) {
			cmov(x, y);
		} else if (HO) {
			home();
			curs(x, y);
		}		/* else impossible("..."); */
	} else if (cury < y) {
		if (XD) {
			while (cury < y) {
				xputs(XD);
				cury++;
			}
		} else if (CM) {
			cmov(x, y);
		} else {
			while (cury < y) {
				xputc('\n');
				curx = 1;
				cury++;
			}
		}
	}
	if (curx < x) {		/* Go to the right. */
		if (!ND)
			cmov(x, y);
		else		/* bah */
			/* should instead print what is there already */
			while (curx < x) {
				xputs(ND);
				curx++;
			}
	} else if (curx > x) {
		while (curx > x) {	/* Go to the left. */
			xputs(BC);
			curx--;
		}
	}
}

void
cmov(x, y)
	int x, y;
{
	xputs(tgoto(CM, x - 1, y - 1));
	cury = y;
	curx = x;
}

int
xputc(c)
	char            c;
{
	return (fputc(c, stdout));
}

void
xputs(s)
	char           *s;
{
	tputs(s, 1, xputc);
}

void
cl_end()
{
	if (CE)
		xputs(CE);
	else {			/* no-CE fix - free after Harold Rynes */
		/*
		 * this looks terrible, especially on a slow terminal but is
		 * better than nothing
		 */
		int cx = curx, cy = cury;

		while (curx < COLNO) {
			xputc(' ');
			curx++;
		}
		curs(cx, cy);
	}
}

void
clear_screen()
{
	xputs(CL);
	curx = cury = 1;
}

void
home()
{
	if (HO)
		xputs(HO);
	else if (CM)
		xputs(tgoto(CM, 0, 0));
	else
		curs(1, 1);	/* using UP ... */
	curx = cury = 1;
}

void
standoutbeg()
{
	if (SO)
		xputs(SO);
}

void
standoutend()
{
	if (SE)
		xputs(SE);
}

void
backsp()
{
	xputs(BC);
	curx--;
}

void
bell()
{
	(void) putchar('\007');	/* curx does not change */
	(void) fflush(stdout);
}

void
hack_delay_output()
{

	/* delay 50 ms - could also use a 'nap'-system call */
	  /* or the usleep call like this :-) */
	usleep(50000);
}

void
cl_eos()
{				/* free after Robert Viduya *//* must only be
				 * called with curx = 1 */

	if (CD)
		xputs(CD);
	else {
		int             cx = curx, cy = cury;
		while (cury <= LI - 2) {
			cl_end();
			xputc('\n');
			curx = 1;
			cury++;
		}
		cl_end();
		curs(cx, cy);
	}
}
