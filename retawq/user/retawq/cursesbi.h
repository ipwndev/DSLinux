/* retawq/cursesbi.h - a small built-in curses library implementation
   This file is part of retawq (<http://retawq.sourceforge.net/>), a network
   client created by Arne Thomassen; retawq is basically released under certain
   versions of the GNU General Public License and WITHOUT ANY WARRANTY.
   Read the file COPYING for license details, README for program information.
   Copyright (C) 2004-2005 Arne Thomassen <arne@arne-thomassen.de>
*/

#ifndef __retawq_cursesbi_h__
#define __retawq_cursesbi_h__

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (1)
#endif

#define OK (0)
#define ERR (-1)

typedef unsigned long chtype;
typedef chtype attr_t;

#define A_REVERSE (1 << 8)
#define A_BOLD (1 << 9)
#define A_UNDERLINE (1 << 10)
#if MIGHT_USE_COLORS
#define __A_COLORMARK (1 << 11)
#define __A_COLORPAIRSHIFT (12)
#define __A_COLORPAIRMASK ( (COLOR_PAIRS - 1) << __A_COLORPAIRSHIFT )
#endif

typedef struct
{ chtype* text;
  attr_t* attr;
  unsigned char* dirty_lines; /* bitfield */
  size_t dirty_lines_size;
  int x, y;
  tBoolean update_cursor;
} WINDOW;

#define _curx x
#define _cury y

typedef unsigned char mmask_t;

typedef struct
{ int x, y;
  mmask_t bstate;
} MEVENT;

extern WINDOW* stdscr;
extern int COLS, LINES;

#define KEY_DOWN 0402
#define KEY_UP 0403
#define KEY_LEFT 0404
#define KEY_RIGHT 0405
#define KEY_HOME 0406
#define KEY_BACKSPACE 0407
/* #define KEY_F0 0410 */
/* #define KEY_F(x) (KEY_F0 + (x)) */
#define KEY_DC 0512
#define KEY_IC 0513
#define KEY_NPAGE 0522
#define KEY_PPAGE 0523
#define KEY_ENTER 0527
#define KEY_CANCEL 0543
#define KEY_END 0550
#define KEY_MOUSE 0631 /* CHECKME! */
#define KEY_RESIZE 0632 /* CHECKME! */

extern WINDOW* initscr(void);

extern int addch(chtype);
extern int addstr(const char*);
extern int addnstr(const char*, int);
extern int attron(attr_t);
extern int attroff(attr_t);
extern int clear(void);
extern int clrtoeol(void);
extern int endwin(void);
extern int getmouse(MEVENT*);
extern chtype inch(void);
extern mmask_t mousemask(mmask_t, mmask_t*);
extern int move(int, int);
extern int mvaddch(int, int, chtype);
extern int mvaddnstr(int, int, const char*, int);
extern int resizeterm(int, int);
#if MIGHT_USE_COLORS
extern int start_color(void);
#endif

extern int my_builtin_getch(tBoolean);
extern int getch(void);

extern int refresh(void);

/* colors stubs - IMPLEMENTME! */

#if MIGHT_USE_COLORS
#define COLOR_PAIRS (0)
#define COLOR_PAIR(cpn) (0)
static __my_inline int has_colors(void) { return(FALSE); }
#define bkgdset(ch) do { } while (0)
#define init_pair(a, b, c) (ERR)
#endif

/* lots-o-stubs */

static __my_inline int cbreak(void) { return(OK); }
static __my_inline int noecho(void) { return(OK); }
static __my_inline int nonl(void) { return(OK); }

#define intrflush(a, b) (0)
#define keypad(a, b) (0)
#define nodelay(a, b) (0)

#endif /* #ifndef __retawq_cursesbi_h__ */
