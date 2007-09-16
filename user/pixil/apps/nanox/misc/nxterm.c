/*                                                                       
  * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.     
  *                                                                       
  * This file is part of the PIXIL Operating Environment                 
  *                                                                       
  * The use, copying and distribution of this file is governed by one    
  * of two licenses, the PIXIL Commercial License, or the GNU General    
  * Public License, version 2.                                           
  *                                                                       
  * Licensees holding a valid PIXIL Commercial License may use this file 
  * in accordance with the PIXIL Commercial License Agreement provided   
  * with the Software. Others are governed under the terms of the GNU   
  * General Public License version 2.                                    
  *                                                                       
  * This file may be distributed and/or modified under the terms of the  
  * GNU General Public License version 2 as published by the Free        
  * Software Foundation and appearing in the file LICENSE.GPL included   
  * in the packaging of this file.                                      
  *                                                                       
  * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING  
  * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A            
  * PARTICULAR PURPOSE.                                                  
  *                                                                       
  * RESTRICTED RIGHTS LEGEND                                             
  *                                                                     
  * Use, duplication, or disclosure by the government is subject to      
  * restriction as set forth in paragraph (b)(3)(b) of the Rights in     
  * Technical Data and Computer Software clause in DAR 7-104.9(a).       
  *                                                                      
  * See http://www.pixil.org/gpl/ for GPL licensing       
  * information.                                                         
  *                                                                      
  * See http://www.pixil.org/license.html or              
  * email cetsales@centurysoftware.com for information about the PIXIL   
  * Commercial License Agreement, or if any conditions of this licensing 
  * are not clear to you.                                                
  */

 /*
  * Portions Copyright (C) 1994,95,96 by Torsten Scherer (TeSche)
  */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <fcntl.h>
#ifdef __FreeBSD__
#include <libutil.h>
#endif
#define MWINCLUDECOLORS
#include "nano-X.h"
#include "nxterm.h"

#define DEF_TITLE	"Linux Terminal"
#define DEF_STYLE	(GR_WM_PROPS_APPWINDOW)
#define DEF_COLOR	GR_COLOR_WINDOWTEXT
#define DEF_GEOMETRY	"80x25+0-0"

#ifdef linux
#define NSIG _NSIG
#endif

#define	SMALLBUFFER 80

GR_WINDOW_ID w1;		/* id for window */
GR_GC_ID gc1;			/* text gc */
GR_GC_ID gc2;			/* fillrect gc */
GR_FONT_ID regFont;
GR_BOOL havefocus = GR_FALSE;
int pid, console;
int pipeh;
int wChar;			/* width/height of character */
int hChar;
int wScreen;			/* width/height of terminal window */
int hScreen;
int col, row;
char *begscr = NULL;
char **vscreen;
int inresize = 0;
int escstate, curx, cury, curon, curvis, reverse;
int savx, savy, wrap;
int sbufcnt = 0;
int sbufx, sbufy;
char sbuf[SMALLBUFFER + 1];

int g_colors[] = { BLACK, RED, GREEN, BROWN, BLUE, MAGENTA, CYAN, GRAY,
    LTGRAY, LTRED, LTGREEN, YELLOW, LTBLUE, LTMAGENTA,
    LTCYAN, WHITE
};

int g_fgcolor = 0;
int g_bgcolor = 15;

#define TEXT_BOLD 0x01

int g_attr = 0;

int term_init();
void hide_cursor(void);
void vscroll(void);

void
redisplay(void)
{
    int y;

    hide_cursor();
    for (y = 0; y < row; ++y)
	GrText(w1, gc1, 0, y * hChar, vscreen[y], col, GR_TFTOP);
}

 /*
  * Resize cols and rows to fit passed window size
  */
void
resize(GR_SIZE width, GR_SIZE height)
{
    int y, x;
    int newcol, newrow;
    char *newbegscr;
    char **newvscreen;
    char buf[32];

    newcol = width / wChar;
    newrow = height / hChar;
    newbegscr = malloc(newrow * newcol * sizeof(char));
    newvscreen = malloc(newrow * sizeof(char *));
    if (!newbegscr || !newvscreen) {
	fprintf(stderr, "Can't allocate screen\n");
	exit(1);
    }
    for (y = 0; y < newrow; ++y)
	newvscreen[y] = &newbegscr[y * newcol];
    memset(newbegscr, ' ', newrow * newcol);

    /* copy old vscreen if present */
    if (begscr) {
	for (y = 0; y < row && y < newrow; ++y)
	    for (x = 0; x < col && x < newcol; ++x)
		newvscreen[y][x] = vscreen[y][x];
	free(vscreen);
	free(begscr);
    }
    /* set new global values */
    begscr = newbegscr;
    vscreen = newvscreen;
    col = newcol;
    row = newrow;
    wScreen = width;
    hScreen = height;

    /* set TERMCAP= string */
    if (termcap_string) {
	sprintf(termcap_string + strlen(termcap_string),
		"li#%d:co#%d:", row, col);
	putenv(termcap_string);
    }
    /* set COLUMN= and LINES= variables */
    sprintf(buf, "%d", col);
    setenv("COLUMNS", buf, 1);
    sprintf(buf, "%d", row);
    setenv("LINES", buf, 1);

    /* JHC - this is not right - we shouldn't restart the 
       shell, but just accept the resize as it happens 
     */

#ifdef NOTUSED
    /*
     * Start new shell after resetting environment variables...
     */
    if (pipeh) {
	++inresize;

	hide_cursor();

	if (++cury >= row) {
	    vscroll();
	    cury = row - 1;
	}

	curx = 0;

	close(pipeh);
	pause();
	pipeh = term_init();
	--inresize;
    }
#endif
}

void
sflush(void)
{
    if (sbufcnt) {
	GrText(w1, gc1, sbufx * wChar, sbufy * hChar, sbuf, sbufcnt,
	       GR_TFTOP);
	sbufcnt = 0;
    }
}

void
sadd(int c)
{
    if (sbufcnt == SMALLBUFFER)
	sflush();

    if (!sbufcnt) {
	sbufx = curx;
	sbufy = cury;
    }
    sbuf[sbufcnt++] = c;
    vscreen[cury][curx] = c;
}

void
show_cursor(void)
{
    GrSetGCMode(gc1, GR_MODE_XOR);
    GrSetGCForeground(gc1, WHITE);
    GrFillRect(w1, gc1, curx * wChar, cury * hChar + 1, wChar, hChar - 1);
    GrSetGCForeground(gc1, g_colors[g_fgcolor]);
    GrSetGCMode(gc1, GR_MODE_SET);
}

void
draw_cursor(void)
{
    if (!curvis) {
	curvis = 1;
	show_cursor();
    }
}

void
hide_cursor(void)
{
    if (curvis) {
	curvis = 0;
	show_cursor();
    }
}

void
vscroll(void)
{
    int y;
    char *p;

    hide_cursor();
    GrCopyArea(w1, gc1, 0, 0, col * wChar, (row - 1) * hChar, w1, 0, hChar,
	       MWROP_SRCCOPY);
    p = vscreen[0];
    for (y = 0; y < row - 1; ++y)
	vscreen[y] = vscreen[y + 1];
    vscreen[row - 1] = p;
    memset(vscreen[row - 1], ' ', col);

    GrFillRect(w1, gc2, 0, (row - 1) * hChar, col * wChar, hChar);
}

struct
{
    unsigned long key;
    char *keycode;
}
g_keys[] =
{
    {
    MWKEY_UP, "\033[A"}
    , {
    MWKEY_DOWN, "\033[B"}
    , {
    MWKEY_RIGHT, "\033[C"}
    , {
    MWKEY_LEFT, "\033[D"}
    , {
    0, ""}
};

/* generate keystrokes for the VT100 */
/* FIXME:  Make this table driven    */

void
handle_key(int key)
{
    int i;

    for (i = 0; g_keys[i].key; i++)
	if (g_keys[i].key == key) {
	    write(pipeh, g_keys[i].keycode, strlen(g_keys[i].keycode));
	    return;
	}
}

void
handle_attribute(int attr)
{
    switch (attr) {
    case 0:
	g_fgcolor = 0;
	g_bgcolor = 15;
	g_attr = 0;
	break;

    case 1:
	if (g_fgcolor < 8)
	    g_fgcolor += 8;

	g_attr |= TEXT_BOLD;
	break;

    case 39:
	g_fgcolor = 0;
	break;

    case 49:
	g_bgcolor = 15;
	break;

    case 30 ... 37:
	g_fgcolor = attr - 30;
	if (g_attr & TEXT_BOLD)
	    g_fgcolor += 8;
	break;

    case 40 ... 47:
	g_bgcolor = attr - 40;
	break;

    default:
	printf("We don't handle attribute %d yet\n", attr);
	return;
    }

    GrSetGCForeground(gc1, g_colors[g_fgcolor]);
    GrSetGCBackground(gc1, g_colors[g_bgcolor]);
}

/* A handler for the CSI (Code Sequence Introducer) sequences */

void
handle_csi(int c)
{

    /* We can handle up to two integer arguments on the sequence */

    static int args[2] = { 0, 0 };
    static int argc = 0;

    int arg = 0;

    switch (c) {
    case 'm':
	for (arg = 0; arg <= argc; arg++)
	    handle_attribute(args[arg]);
	break;

    case ';':
	argc++;
	return;

    default:
	if (c >= '0' && c <= '9')
	    args[argc] = (args[argc] * 10) + (c - '0');
	else
	    printf("Warning - unknown char '%c' in the sequence\n", c);

	return;
    }

    args[0] = args[1] = 0;

    escstate = 0;
    argc = 0;
}

void
esc5(int c)
{				/* setting background color */
    //bgcolor = colors[c];
    GrSetGCBackground(gc1, g_colors[g_bgcolor]);
    GrSetGCForeground(gc2, g_colors[g_bgcolor]);
    escstate = 0;
}

void
esc4(int c)
{				/* setting foreground color */
    //fgcolor = colors[c];
    GrSetGCForeground(gc1, g_colors[g_fgcolor]);
    escstate = 0;
}

void
esc3(int c)
{				/* cursor position x axis */
    curx = (c - 32) & 0xff;
    if (curx >= col)
	curx = col - 1;
    else if (curx < 0)
	curx = 0;
    escstate = 0;
}


void
esc2(int c)
{				/* cursor position y axis */
    cury = (c - 32) & 0xff;
    if (cury >= row)
	cury = row - 1;
    else if (cury < 0)
	cury = 0;
    escstate = 3;
}

void
esc1(int c)
{				/* various control codes */
    int y, x;
    escstate = 0;

    switch (c) {
    case 'A':			/* cursor up */
	if (--cury < 0)
	    cury = 0;
	break;

    case 'B':			/* cursor down */
	if (++cury >= row)
	    cury = row - 1;
	break;

    case 'C':			/* cursor right */
	if (++curx >= col)
	    curx = col - 1;
	break;

    case 'D':			/* cursor left */
	if (--curx < 0)
	    curx = 0;
	break;

    case 'E':			/* clear screen & home */
	memset(begscr, ' ', row * col);
	GrClearWindow(w1, 0);
	curx = 0;
	cury = 0;
	break;

    case 'H':			/* cursor home */
	curx = 0;
	cury = 0;
	break;

    case 'I':			/* reverse index */
	if (--cury < 0)
	    cury = 0;
	break;

    case 'J':			/* erase to end of page */
	if (cury < row - 1) {
	    for (y = cury; y < row; ++y)
		memset(vscreen[y], ' ', col);
	    GrFillRect(w1, gc2, 0, (cury + 1) * hChar, wScreen,
		       (row - 1 - cury) * hChar);
	}
	for (x = curx; x < col; ++x)
	    vscreen[cury][x] = ' ';
	GrFillRect(w1, gc2, curx * wChar, cury * hChar,
		   (col - curx) * wChar, hChar);
	break;

    case 'K':			/* erase to end of line */
	for (x = curx; x < col; ++x)
	    vscreen[cury][x] = ' ';
	GrFillRect(w1, gc2, curx * wChar, cury * hChar,
		   (col - curx) * wChar, hChar);
	break;

    case 'L':			/* insert line */
	if (cury < row - 1)
	    vscroll();		//FIXME buggy

	curx = 0;
	break;

    case 'M':			/* delete line */
	if (cury < row - 1)
	    vscroll();
	curx = 0;
	break;

    case 'Y':			/* position cursor */
	escstate = 2;
	break;

    case 'b':			/* set foreground color */
	escstate = 4;
	break;

    case 'c':			/* set background color */
	escstate = 5;
	break;

    case 'd':			/* erase beginning of display */
	if (cury > 0) {
	    for (y = 0; y < cury; ++y)
		memset(vscreen[y], ' ', col);
	    GrFillRect(w1, gc2, 0, 0, wScreen, cury * hChar);
	}
	if (curx > 0) {
	    for (x = 0; x < curx; ++x)
		vscreen[cury][x] = ' ';
	    GrFillRect(w1, gc2, 0, cury * hChar, curx * wChar, hChar);
	}
	break;

    case 'e':			/* enable cursor */
	curon = 1;
	break;

    case 'f':			/* disable cursor */
	curon = 0;
	break;

    case 'j':			/* save cursor position */
	savx = curx;
	savy = cury;
	break;

    case 'k':			/* restore cursor position */
	curx = savx;
	cury = savy;
	break;

    case 'l':			/* erase entire line */
	memset(vscreen[cury], ' ', col);
	GrFillRect(w1, gc2, 0, cury * hChar, wScreen, hChar);
	curx = 0;
	break;

    case 'o':			/* erase beginning of line */
	if (curx > 0) {
	    for (x = 0; x < curx; ++x)
		vscreen[cury][x] = ' ';
	    GrFillRect(w1, gc2, 0, cury * hChar, curx * wChar, hChar);
	}
	break;

    case 'p':			/* enter reverse video mode */
	if (!reverse) {
	    //FIXME
	    //GrSetGCForeground(gc1,gi.background);
	    //GrSetGCBackground(gc1,gi.foreground);
	    reverse = 1;
	}
	break;

    case 'q':			/* exit reverse video mode */
	if (reverse) {
	    //GrSetGCForeground(gc1,gi.foreground);
	    //GrSetGCBackground(gc1,gi.background);
	    reverse = 0;
	}
	break;

    case 'v':			/* enable wrap at end of line */
	wrap = 1;
	break;

    case 'w':			/* disable wrap at end of line */
	wrap = 0;
	break;

	/* and these are the extensions not in VT52 */
    case '[':
	escstate = 6;
	break;

    case 'G':			/* clear all attributes */
    case 'g':			/* enter bold mode */
    case 'h':			/* exit bold mode */
    case 'i':			/* enter underline mode */
	/* j, k and l are already used */
    case 'm':			/* exit underline mode */
	/* these ones aren't yet on the termcap entries */
    case 'n':			/* enter italic mode */
	/* o, p and q are already used */
    case 'r':			/* exit italic mode */
    case 's':			/* enter light mode */
    case 't':			/* exit light mode */
    default:			/* unknown escape sequence */
	printf("Escape sequence %c\n", c);
	break;
    }
}

void
esc0(int c)
{
    switch (c) {
    case 0:			/* nul */
	break;

    case 7:			/* bell */
	break;

    case 8:			/* backspace */
	sflush();
	if (--curx < 0)
	    curx = 0;
	esc0(' ');
	sflush();
	if (--curx < 0)
	    curx = 0;
	break;

    case 9:			/* tab */
	do {
	    esc0(' ');
	} while (curx & 7);
	break;

    case 10:			/* line feed */
	sflush();
	if (++cury >= row) {
	    vscroll();
	    cury = row - 1;
	}
	break;

    case 13:			/* carriage return */
	sflush();
	curx = 0;
	break;

    case 27:			/* escape */
	escstate = 1;
	break;

    case 127:			/* delete */
	break;

    default:			/* any printable char */
	if (c < ' ')
	    return;
	sadd(c);
	if (++curx >= col) {
	    sflush();
	    if (!wrap)
		curx = col - 1;
	    else {
		curx = 0;
		if (++cury >= row) {
		    vscroll();
		    cury = row - 1;
		}
	    }
	}
	break;
    }
}

void
printc(int c)
{
    switch (escstate) {
    case 0:
	esc0(c);
	break;

    case 1:
	sflush();
	esc1(c);
	break;

    case 2:
	sflush();
	esc2(c);
	break;

    case 3:
	sflush();
	esc3(c);
	break;

    case 4:
	sflush();
	esc4(c);
	break;

    case 5:
	sflush();
	esc5(c);
	break;

    case 6:
	handle_csi(c);
	break;

    default:
	escstate = 0;
	break;
    }
}

void
init()
{
    curx = savx = 0;
    cury = savy = 0;
    wrap = 1;
    curon = 1;
    curvis = 0;
    reverse = 0;
    escstate = 0;
    g_fgcolor = 0;
    g_bgcolor = 15;
}

void
mainloop(void)
{
    int in, l;
    MWKEY key;
    GR_EVENT wevent;
    unsigned char ch;
    unsigned char buf[1024];

    GrRegisterInput(pipeh);
    while (42) {
	if (havefocus)
	    draw_cursor();
	GrGetNextEvent(&wevent);

	switch (wevent.type) {
	case GR_EVENT_TYPE_CLOSE_REQ:
	    GrClose();
	    exit(0);
	    break;

	case GR_EVENT_TYPE_EXPOSURE:
	    redisplay();
	    break;

	case GR_EVENT_TYPE_KEY_DOWN:
	    key = wevent.keystroke.ch;
	    /* toss all special keys */
	    if (key & MWKEY_NONASCII_MASK)
		handle_key(key);
	    else {
		ch = (unsigned char) key;
		write(pipeh, &ch, 1);
	    }
	    break;

	case GR_EVENT_TYPE_FOCUS_IN:
	    havefocus = GR_TRUE;
	    break;

	case GR_EVENT_TYPE_FOCUS_OUT:
	    havefocus = GR_FALSE;
	    hide_cursor();
	    break;

	case GR_EVENT_TYPE_UPDATE:
	    switch (wevent.update.utype) {
	    case GR_UPDATE_UNMAPTEMP:
		/*
		 * if we get temporarily unmapped (moved),
		 * set cursor state off.
		 */
		curvis = 0;
		break;
	    case GR_UPDATE_SIZE:
		resize(wevent.update.width, wevent.update.height);
		break;
	    }
	    break;

	case GR_EVENT_TYPE_FDINPUT:
	    hide_cursor();
	    while ((in = read(pipeh, buf, sizeof(buf))) > 0) {
		for (l = 0; l < in; l++)
		    printc(buf[l]);
		sflush();
	    }
	    break;
	}
    }
}

void
usage(char *s)
{
    if (s)
	fprintf(stderr, "error: %s\n", s);
    printf("usage: nxterm [-g <geometry>] [-c] [-h] [program {args}]\n");
    exit(0);
}

void
sigchld(int sig)
{
    int status;
    int pid = wait(&status);
    printf("Pid %d died\n", pid);

    /* FIXME:  Should we exit here? */
}

#if 0000
void *
mysignal(int signum, void *handler)
{
    struct sigaction sa, so;

    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(signum, &sa, &so);

    return so.sa_handler;
}

void
sigpipe(int sig)
{
    /* this one musn't close the window */
    kill(-pid, SIGHUP);
    _exit(sig);
}

/* Responsibly handling the child */

void
sigquit(int sig)
{
    signal(sig, SIG_IGN);
    kill(-pid, SIGHUP);
}

#endif

int
main(int argc, char **argv)
{
    GR_BITMAP bitmap1fg[7];	/* mouse cursor */
    GR_BITMAP bitmap1bg[7];
    GR_FONT_INFO fi;		/* Font Info */
    char *shell = NULL;
    struct passwd *pw;
    char thesh[128];

#ifdef SIGTTOU
    /* just in case we're started in the background */
    signal(SIGTTOU, SIG_IGN);
#endif

    /* who am I? */
    if (!(pw = getpwuid(getuid()))) {
	fprintf(stderr,
		"nxterm: can't determine determine your login name\n");
	exit(-1);
    }
    if (GrOpen() < 0) {
	fprintf(stderr, "cannot open graphics\n");
	exit(1);
    }
    /*
     * scan arguments...
     */
    argv++;
    while (*argv && **argv == '-')
	switch (*(*argv + 1)) {
	case 'c':
	    console = 1;
	    argv++;
	    break;

	case 'h':
	    /* this will never return */
	    usage("");

	default:
	    usage("unknown option");
	}

    /*
     * now *argv either points to a program to start or is zero
     */

    if (*argv) {
	shell = *argv;
    }
    if (!shell) {
	shell = getenv("SHELL=");
    }
    if (!shell) {
	shell = pw->pw_shell;
    }
    if (!shell) {
	shell = "/bin/sh";
    }
    if (!*argv) {
	char *cptr;

	/*
	 * the '-' makes the shell think it is a login shell,
	 * we leave argv[0] alone if it isn`t a shell (ie.
	 * the user specified the program to run as an argument
	 * to wterm.
	 */
	cptr = strrchr(shell, '/');
	sprintf(thesh, "-%s", cptr ? cptr + 1 : shell);
	*--argv = thesh;
    }

    init();
    regFont = GrCreateFont(GR_FONT_SYSTEM_FIXED, 0, NULL);
    GrGetFontInfo(regFont, &fi);
    wChar = fi.maxwidth;
    hChar = fi.height;

    resize(80 * wChar, 25 * hChar);
    w1 = GrNewWindowEx(DEF_STYLE, DEF_TITLE, GR_ROOT_WINDOW_ID, 10, 10,
		       wScreen, hScreen, g_colors[g_bgcolor]);

    GrSelectEvents(w1, GR_EVENT_MASK_BUTTON_DOWN |
		   GR_EVENT_MASK_KEY_DOWN | GR_EVENT_MASK_EXPOSURE |
		   GR_EVENT_MASK_FOCUS_IN | GR_EVENT_MASK_FOCUS_OUT |
		   GR_EVENT_MASK_UPDATE | GR_EVENT_MASK_CLOSE_REQ);

    GrMapWindow(w1);

    gc1 = GrNewGC();
    GrSetGCForeground(gc1, g_colors[g_fgcolor]);
    GrSetGCBackground(gc1, g_colors[g_bgcolor]);
    GrSetGCFont(gc1, regFont);
    gc2 = GrNewGC();
    GrSetGCForeground(gc2, g_colors[g_bgcolor]);

#define	_	((unsigned) 0)	/* off bits */
#define	X	((unsigned) 1)	/* on bits */
#define	MASK(a,b,c,d,e,f,g) \
	(((((((((((((a * 2) + b) * 2) + c) * 2) + d) * 2) \
	+ e) * 2) + f) * 2) + g) << 9)
    bitmap1fg[0] = MASK(_, _, X, _, X, _, _);
    bitmap1fg[1] = MASK(_, _, _, X, _, _, _);
    bitmap1fg[2] = MASK(_, _, _, X, _, _, _);
    bitmap1fg[3] = MASK(_, _, _, X, _, _, _);
    bitmap1fg[4] = MASK(_, _, _, X, _, _, _);
    bitmap1fg[5] = MASK(_, _, _, X, _, _, _);
    bitmap1fg[6] = MASK(_, _, X, _, X, _, _);

    bitmap1bg[0] = MASK(_, X, X, X, X, X, _);
    bitmap1bg[1] = MASK(_, _, X, X, X, _, _);
    bitmap1bg[2] = MASK(_, _, X, X, X, _, _);
    bitmap1bg[3] = MASK(_, _, X, X, X, _, _);
    bitmap1bg[4] = MASK(_, _, X, X, X, _, _);
    bitmap1bg[5] = MASK(_, _, X, X, X, _, _);
    bitmap1bg[6] = MASK(_, X, X, X, X, X, _);
    GrSetCursor(w1, 7, 7, 3, 3, g_colors[g_fgcolor], g_colors[g_bgcolor],
		bitmap1fg, bitmap1bg);

#ifdef __FreeBSD__
    putenv("TERM=wterm");
#else
    putenv("TERM=vt52");
#endif

    /* create a pty */
    pipeh = term_init();

    /* prepare to catch console output */
    if (console)
	ioctl(pipeh, TIOCCONS, 0);

    //signal(SIGCHLD, sigchld);

#if 0
    /* catch some signals */
    mysignal(SIGTERM, sigquit);
    mysignal(SIGHUP, sigquit);
    mysignal(SIGINT, SIG_IGN);
    mysignal(SIGQUIT, sigquit);
    mysignal(SIGPIPE, sigpipe);
    mysignal(SIGCHLD, sigchld);
#endif

    mainloop();
    return 0;
}

#ifdef __FreeBSD__
int
term_init()
{
    struct winsize winsz;
    char *ptr;
    char pty[128];

    winsz.ws_col = col;
    winsz.ws_row = row;
    if ((pid = forkpty(&pipeh, pty, NULL, &winsz)) < 0) {
	fprintf(stderr, "wterm: can't create pty\r\n");
	perror("wterm");
	sleep(2);
	GrKillWindow(w1);
	exit(-1);
    }
    if ((ptr = rindex(pty, '/')))
	strcpy(pty, ptr + 1);

    if (!pid) {
	int i;

	for (i = getdtablesize(); --i >= 3;)
	    close(i);
	/*
	 * SIG_IGN are not reset on exec()
	 */
	for (i = NSIG; --i >= 0;)
	    signal(i, SIG_DFL);

	/* caution: start shell with correct user id! */
	seteuid(getuid());
	setegid(getgid());

	/* this shall not return */
	execvp(shell, argv);

	/* oops? */
	fprintf(stderr, "wterm: can't start shell\r\n");
	GrKillWindow(w1);
	_exit(-1);
    }
    return pipeh;
}

#else /* !FreeBSD */

void
sigchild(int signo)
{
  int status;

  int pid = wait(&status);

  printf("THE CHILD HAS DIED [%d]\n", pid);

    if (!inresize) {
	GrClose();
	exit(0);
    }
}

#if ELKS
char *nargv[2] = { "/bin/sash", NULL };
#else
#if DOS_DJGPP
char *nargv[2] = { "bash", NULL };
#else
char *nargv[2] = { "/bin/sh", NULL };
#endif
#endif

int
term_init()
{
    int tfd;
    int n = 0;
    pid_t pid;
    char pty_name[12];

  again:
    sprintf(pty_name, "/dev/ptyp%d", n);
    if ((tfd = open(pty_name, O_RDWR | O_NONBLOCK)) < 0) {
	if ((errno == EBUSY || errno == EIO) && n < 10) {
	    n++;
	    goto again;
	}
	fprintf(stderr, "Can't create pty %s\n", pty_name);
	return -1;
    }
    signal(SIGCHLD, sigchild);
    signal(SIGINT,  sigchild);
    if ((pid = vfork()) == -1) {
	fprintf(stderr, "No processes\n");
	return -1;
    }
    if (!pid) {
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(tfd);

	setsid();
	pty_name[5] = 't';
	if ((tfd = open(pty_name, O_RDWR)) < 0) {
	    fprintf(stderr, "Child: Can't open pty %s [%s]\n", pty_name,
		    strerror(errno));
	    exit(1);
	}
	close(STDERR_FILENO);
	dup2(tfd, STDIN_FILENO);
	dup2(tfd, STDOUT_FILENO);
	dup2(tfd, STDERR_FILENO);
	execv(nargv[0], nargv);
	exit(1);
    }
    return tfd;
}
#endif /* !FreeBSD */
