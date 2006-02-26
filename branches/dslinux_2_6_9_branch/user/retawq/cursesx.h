/* retawq/cursesx.h - a small xcurses implementation
   This file is part of retawq (<http://retawq.sourceforge.net/>), a network
   client created by Arne Thomassen; retawq is basically released under certain
   versions of the GNU General Public License and WITHOUT ANY WARRANTY.
   Read the file COPYING for license details, README for program information.
   Copyright (C) 2004-2005 Arne Thomassen <arne@arne-thomassen.de>
*/

#ifndef __retawq_cursesx_h__
#define __retawq_cursesx_h__

#include <X11/keysym.h>

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (1)
#endif

#if 1 /* MIGHT_USE_COLORS */
#define COLOR_PAIRS (8) /* (enough for us; must be a power of 2 here) */
#endif

typedef unsigned long chtype;
typedef chtype attr_t;

typedef struct
{ chtype* text;
  attr_t* attr;
  int x, y; /* classical clarification: current cursor coordinates :-) */
} WINDOW;

#define _curx x
#define _cury y

#define OK (0)
#define ERR (-1)

#define KEY_TAB (XK_Tab)
#define KEY_ESCAPE (XK_Escape)
#define KEY_CANCEL (XK_Cancel)
#define KEY_LEFT (XK_Left)
#define KEY_RIGHT (XK_Right)
#define KEY_END (XK_End)
#define KEY_NPAGE (XK_Page_Down)
#define KEY_PPAGE (XK_Page_Up)
#define KEY_DOWN (XK_Down)
#define KEY_UP (XK_Up)
#define KEY_HOME (XK_Home)
#define KEY_ENTER (XK_Return)
#define KEY_BACKSPACE (XK_BackSpace)
#define KEY_DC (XK_Delete)
#define KEY_IC (XK_Insert)
#define KEY_F(x) (XK_F1 + (x) - 1)
#define KEY_MOUSE 0631 /* CHECKME! */
#define KEY_RESIZE 0632 /* CHECKME! */

#define A_REVERSE (1 << 8)
#define A_BOLD (1 << 9)
#define A_UNDERLINE (1 << 10)
#define A_ALTCHARSET (1 << 11)
#if MIGHT_USE_COLORS
#define __A_COLORMARK (1 << 12)
#define __A_COLORPAIRSHIFT (13)
#define __A_COLORPAIRMASK ( (COLOR_PAIRS - 1) << __A_COLORPAIRSHIFT )
#endif

#define ACS_HLINE    ((chtype) (A_ALTCHARSET | 'A'))
#define ACS_VLINE    ((chtype) (A_ALTCHARSET | 'B'))
#define ACS_ULCORNER ((chtype) (A_ALTCHARSET | 'C'))
#define ACS_URCORNER ((chtype) (A_ALTCHARSET | 'D'))
#define ACS_LLCORNER ((chtype) (A_ALTCHARSET | 'E'))
#define ACS_LRCORNER ((chtype) (A_ALTCHARSET | 'F'))
#if 0 /* not (yet?) needed */
#define ACS_LTEE     ((chtype) (A_ALTCHARSET | 'G'))
#define ACS_RTEE     ((chtype) (A_ALTCHARSET | 'H'))
#endif

#define BUTTON1_CLICKED (0x01)
#define BUTTON2_CLICKED (0x02)
#define BUTTON3_CLICKED (0x04)
typedef unsigned char mmask_t;

typedef struct
{ int x, y;
  mmask_t bstate;
} MEVENT;

extern WINDOW* stdscr;
extern int COLS, LINES;

extern void xcurses_confuser(unsigned char, void*, void*); /* see main.c */

extern WINDOW* initscr(void);

extern int addch(chtype);
extern int addstr(const char*);
extern int addnstr(const char*, int);
extern int attron(attr_t);
extern int attroff(attr_t);
extern int clrtoeol(void);
extern int getch(void);
extern int getmouse(MEVENT*);
#if MIGHT_USE_COLORS
extern int has_colors(void);
#endif
extern chtype inch(void);
#if MIGHT_USE_COLORS
extern int init_pair(short, short, short);
#endif
extern mmask_t mousemask(mmask_t, mmask_t*);
extern int move(int, int);
extern int mvaddch(int, int, chtype);
extern int mvaddnstr(int, int, const char*, int);
extern int refresh(void);

#if MIGHT_USE_COLORS
#define COLOR_PAIR(cpn) \
  ( (__A_COLORMARK) | (((attr_t) (cpn)) << __A_COLORPAIRSHIFT) )
#endif

/* lots-o-stubs */

static __my_inline int cbreak(void) { return(OK); }
static __my_inline int clear(void) { return(OK); } /* IMPLEMENTME? */
static __my_inline int endwin(void) { return(OK); } /* IMPLEMENTME? */
static __my_inline int noecho(void) { return(OK); }
static __my_inline int nonl(void) { return(OK); }

#if MIGHT_USE_COLORS
static __my_inline int start_color(void) { return(OK); }
#define bkgdset(ch) do { } while (0) /* IMPLEMENTME? */
#endif

#define intrflush(a, b) (0)
#define keypad(a, b) (0)
#define nodelay(a, b) (0)
#define resizeterm(a, b) (0)

#endif /* #ifndef __retawq_cursesx_h__ */
