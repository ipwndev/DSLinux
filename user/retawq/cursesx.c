/* retawq/cursesx.c - a small xcurses implementation
   This file is part of retawq (<http://retawq.sourceforge.net/>), a network
   client created by Arne Thomassen; retawq is basically released under certain
   versions of the GNU General Public License and WITHOUT ANY WARRANTY.
   Read the file COPYING for license details, README for program information.
   Copyright (C) 2004-2005 Arne Thomassen <arne@arne-thomassen.de>
*/

/* For the rest of the program, xcurses mode shall look like any normal curses
   mode as far as possible. */

#include "stuff.h"
#include "resource.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

static Display* xws_display;
static const_after_init Window xws_window;
static const_after_init GC xws_gc;
static const_after_init Colormap colormap;
static const_after_init unsigned long pixel_fg, pixel_bg, pixel_black,
  pixel_white, colors[COLOR_PAIRS];
static const_after_init Atom WmProt, WmDelete;
static const_after_init Cursor cursor_arrow, cursor_textmark;
static const_after_init tBoolean use_cursor = falsE;

typedef struct
{ const XFontStruct* font;
  int width, height, ascent;
} tXwsFont;

static const tXwsFont *font_normal, *font_bold;

static attr_t attr = 0;

static WINDOW __stdscr;
WINDOW* stdscr = &__stdscr;
int COLS = 80, LINES = 25;

#define SAI(x, y) ( ((y) * COLS) + (x) ) /* screen array index */
#define A_CHARTEXT (255)

static mmask_t __mousemask = 0;

typedef struct tGetmouse
{ MEVENT data;
  struct tGetmouse* next;
} tGetmouse;

static tGetmouse* getmouse_list = NULL;

static struct
{ const char* text;
  size_t size;
  int x1, y1, x2, y2; /* selection region */
  int gx, gy; /* coordinates of button-press event */
#if MIGHT_USE_SCROLL_BARS
  int sbty, sbkd;
#endif
  unsigned char state;
    /* "&1": user is currently selecting; "&2": use x1/y1; "&4": use x2/y2;
       "&8": scroll bar thumb tracking */
} sel0;

#if MIGHT_USE_SCROLL_BARS
#define SCROLL_BAR_WIDTH (15)
#define SCROLL_BAR_DIST (2)
#define MIN_THUMB_HEIGHT (5)
static const_after_init unsigned long pixel_grey75;
static int sbc[8]; /* scroll bar coordinates */
#endif


/* Helper functions */

static void xcurses_io_handler(void* data,
  __sunused tFdObservationFlags flags __cunused)
{ int fd = MY_POINTER_TO_INT(data);
#if CONFIG_DEBUG
  char b[200]; sprint_safe(b, "xcurses_io_handler(): %d\n", fd); debugmsg(b);
#endif
#if NEED_FD_REGISTER
  (void) fd_register_lookup(&fd);
#endif
  XProcessInternalConnection(xws_display, fd);
}

static void connection_do_watch(int fd)
{
#if NEED_FD_REGISTER
#define fdkXlib (fdkOther)
  /* ("probably" a socket, but handled by Xlib, never by our lwIP code) */
  fd_register(&fd, fdkXlib);
#endif
  if (!fd_is_observable(fd)) fatal_tmofd(fd);
  fd_observe(fd, xcurses_io_handler, MY_INT_TO_POINTER(fd), fdofRead);
}

static void connection_watch(Display* display,
  __sunused XPointer client_data __cunused, int fd, Bool opening,
  __sunused XPointer* watch_data __cunused)
{
  if (display != xws_display) return; /* "can't happen"? */
  if (fd < 0) return; /* "can't happen"? */
  if (opening) connection_do_watch(fd);
  else
  {
#if NEED_FD_REGISTER
    if (!fd_register_rlookup(&fd, fdkXlib)) fatal_error(0, "BUG: frr()");
#endif
    fd_unobserve(fd);
  }
}

static unsigned char __must_redraw = 1 | 2 | 4;
static __my_inline void must_redraw(unsigned char what)
/* <what>: "&1": cursor; "&2": contents; "&4": resized */
{ __must_redraw |= what;
}

static tXwsFont* __init load_font(const char* which)
{ tXwsFont* retval;
  XFontStruct* font;
  char buf[1024], buf2[1024];
  int ascent;
  sprint_safe(buf, "-misc-fixed-%s-r-normal--13-120-*-*-c-70-iso8859-1",which);
  font = XLoadQueryFont(xws_display, buf);
  if (font == NULL)
  { sprint_safe(buf2, _("can't load font %s"), buf); fatal_error(0, buf2); }
  retval = memory_allocate(sizeof(tXwsFont), mapPermanent);
  retval->font = font; retval->ascent = ascent = font->ascent;
  retval->height = ascent + font->descent;
  retval->width = XTextWidth(font, "M", 1);
  return(retval);
}

static void use_font(const tXwsFont* font)
{ (void) XSetFont(xws_display, xws_gc, font->font->fid);
}

static tBoolean alloc_color(const char* name, /*@out@*/ unsigned long* pix)
{ XColor color, dummy;
  if (XAllocNamedColor(xws_display, colormap, name, &color, &dummy) != 0)
  { *pix = color.pixel; return(truE); }
  else return(falsE);
}

#define set_fg(fg) XSetForeground(xws_display, xws_gc, fg)

static void set_fgbg(unsigned long fg, unsigned long bg)
{ set_fg(fg); XSetBackground(xws_display, xws_gc, bg);
}

#define draw_line(x1, y1, x2, y2) \
  (void) XDrawLine(xws_display, xws_window, xws_gc, x1, y1, x2, y2)

#define draw_rectangle(x, y, w, h) \
  (void) XFillRectangle(xws_display, xws_window, xws_gc, x, y, w, h)

#define draw_char(x, y, ch) \
  do \
  { unsigned char _c = (unsigned char) (ch); \
    (void) XDrawImageString(xws_display, xws_window, xws_gc, (x), (y), \
      (char*) (&_c), 1); \
  } while (0)

static void set_cursor(Cursor c)
{ if (use_cursor) (void) XDefineCursor(xws_display, xws_window, c);
}

static void coord_limit(int* _x, int* _y)
{ int x = *_x, y = *_y;
  if (x < 0) x = 0;
  else if (x > COLS - 1) x = COLS - 1;
  if (y < 0) y = 0;
  else if (y > LINES - 1) y = LINES - 1;
  *_x = x; *_y = y;
}

static tBoolean coord_pixel2char(int gx, int gy, /*@out@*/ int* _x,
  /*@out@*/ int* _y, tBoolean fail_if_bad)
{ int x = gx / font_normal->width, y = gy / font_normal->height;
  if (fail_if_bad)
  { if ( (x < 0) || (x >= COLS) || (y < 0) || (y >= LINES) ) return(falsE); }
  coord_limit(&x, &y);
  *_x = x; *_y = y;
  return(truE);
}

static void coord_reorder(int* x1, int* y1, int* x2, int* y2)
{ int a;
  if (*y1 > *y2)
  { a = *x1; *x1 = *x2; *x2 = a;
    a = *y1; *y1 = *y2; *y2 = a;
  }
  else if ( (*y1 == *y2) && (*x1 > *x2) ) { a = *x1; *x1 = *x2; *x2 = a; }
}

static void reinit_stdscr_data(void)
/* Call this whenever COLS or LINES changed. */
{ chtype* text;
  int prod = COLS * LINES, count;
  stdscr->x = stdscr->y = 0; __dealloc(stdscr->text);
  text = stdscr->text = __memory_allocate(prod * sizeof(chtype), mapOther);
  for (count = 0; count < prod; count++) text[count] = ' ';
  __dealloc(stdscr->attr);
  stdscr->attr = memory_allocate(prod * sizeof(attr_t), mapOther);
  must_redraw(1 | 2 | 4); sel0.state &= ~(1 | 2 | 4);
}


/* Initialization */

WINDOW* __init initscr(void)
{ int fd, screen, width, height, count, x, y;
  XSizeHints* size_hints = XAllocSizeHints();
  XWMHints* wm_hints = XAllocWMHints();
  XClassHint* class_hint = XAllocClassHint();
  Pixmap logo = None;

  xws_display = XOpenDisplay(NULL);
  if (xws_display == NULL) { cant_open: fatal_error(0, _(strCantOpenXws)); }
  fd = ConnectionNumber(xws_display);
  if (fd < 0) goto cant_open; /* "can't happen"? */
  connection_do_watch(fd);
  if (!XAddConnectionWatch(xws_display, connection_watch, NULL))
    fatal_error(0, _("can't watch X display connections"));
  screen = DefaultScreen(xws_display);
  colormap = XDefaultColormap(xws_display, screen);
  pixel_black = BlackPixel(xws_display, screen);
  pixel_white = WhitePixel(xws_display, screen);
#if OFWAX
  pixel_fg = pixel_black;
  if (!alloc_color("linen", &pixel_bg)) pixel_bg = pixel_white;
#else
  if (config.flags & (cfColorsOff | cfColorsReverse))
  { pixel_fg = pixel_black; pixel_bg = pixel_white; }
  else { pixel_fg = pixel_white; pixel_bg = pixel_black; }
#endif
#if MIGHT_USE_SCROLL_BARS
  if (!alloc_color("grey75", &pixel_grey75)) pixel_grey75 = pixel_bg;
  my_memclr_arr(sbc);
#endif

  font_normal = load_font("medium"); font_bold = load_font("bold");

  width = 80 * font_normal->width; height = 25 * font_normal->height;
#if MIGHT_USE_SCROLL_BARS
  width += SCROLL_BAR_WIDTH;
#endif
  xws_window = XCreateSimpleWindow(xws_display, RootWindow(xws_display,
    screen), 0, 0, width, height, 2, pixel_fg, pixel_bg);
  if (xws_window == None)
  { fail_window: fatal_error(0, _("can't create window")); }

  size_hints->flags = PSize | PResizeInc;
  size_hints->width = width; size_hints->height = height;
  size_hints->width_inc = font_normal->width;
  size_hints->height_inc = font_normal->height;
  wm_hints->flags = InputHint | StateHint | WindowGroupHint;
  wm_hints->input = True; wm_hints->initial_state = NormalState;
  wm_hints->window_group = xws_window;
  if (logo != None)
  { wm_hints->flags |= IconPixmapHint; wm_hints->icon_pixmap = logo; }
  class_hint->res_name = unconstify(strRetawq);
  class_hint->res_class = unconstify("Xedit");
  XmbSetWMProperties(xws_display, xws_window, strProgramVersion,
    strProgramVersion, NULL, 0, size_hints, wm_hints, class_hint);
  (void) XStoreName(xws_display, xws_window, strProgramVersion);
    /* (jwm reportedly needs this, XmbSetWMProperties() isn't enough) */
  WmProt = XInternAtom(xws_display, "WM_PROTOCOLS", False);
  WmDelete = XInternAtom(xws_display, "WM_DELETE_WINDOW", False);
  if (WmDelete != None) XSetWMProtocols(xws_display, xws_window, &WmDelete, 1);
  (void) XSelectInput(xws_display, xws_window, KeyPressMask | ButtonPressMask |
    ButtonReleaseMask | Button1MotionMask | ExposureMask |
    VisibilityChangeMask | StructureNotifyMask); /* CHECKME! */

  xws_gc = XCreateGC(xws_display, xws_window, 0, NULL);
  if (xws_gc == None) goto fail_window;
  for (count = 0; count < COLOR_PAIRS; count++) colors[count] = pixel_fg;
  set_fgbg(pixel_fg, pixel_bg); use_font(font_normal);

  (void) XMapWindow(xws_display, xws_window);

  cursor_arrow = XCreateFontCursor(xws_display, XC_left_ptr);
  cursor_textmark = XCreateFontCursor(xws_display, XC_xterm);
  if ( (cursor_arrow != None) && (cursor_textmark != None) ) use_cursor = truE;

  if (env_termsize(&x, &y)) { COLS = x; LINES = y; }
  my_memclr_var(__stdscr); reinit_stdscr_data();
  set_cursor(cursor_textmark); (void) XFlush(xws_display); my_memclr_var(sel0);
  return(stdscr);
}


/* Redrawing, event handling */

static const chtype* rdr_t;
static const attr_t* rdr_a;
static int rdr_curx, rdr_cury, rdr_hlx1, rdr_hly1, rdr_hlx2, rdr_hly2;
static tBoolean rdr_use_hl;

static one_caller void redraw_char(int x, int y)
{ const int idx = SAI(x, y);
  chtype c = rdr_t[idx];
  attr_t a = rdr_a[idx];
  const tXwsFont* font = ( (a & A_BOLD) ? font_bold : font_normal );
  const int width = font->width, height = font->height, gx = x * width,
    ay = y * height, gy = ay + font->ascent;
#if MIGHT_USE_COLORS
  const tColorPairNumber cpn = ( (a & __A_COLORMARK) ?
    ((a & __A_COLORPAIRMASK) >> __A_COLORPAIRSHIFT) : cpnDefault );
  unsigned long fg = colors[cpn], bg = pixel_bg;
#else
  unsigned long fg = pixel_fg, bg = pixel_bg;
#endif
  if ( (x == rdr_curx) && (y == rdr_cury) ) a ^= A_REVERSE; /* cursor */
  if (rdr_use_hl)
  { if ( (y < rdr_hly1) || ( (y == rdr_hly1) && (x < rdr_hlx1) ) ) { }
    else if ( (y > rdr_hly2) || ( (y == rdr_hly2) && (x > rdr_hlx2) ) ) { }
    else a ^= A_REVERSE; /* selection */
  }
  if (a & A_REVERSE)
  {
#if OFWAX
    bg = pixel_grey75;
#else
    { unsigned long xg = fg; fg = bg; bg = xg; }
#endif
  }
  set_fgbg(fg, bg);
  if (!(a & A_ALTCHARSET)) { draw_ch: use_font(font); draw_char(gx, gy, c); }
  else
  { const int xmid = gx + width / 2, ymid = ay + height / 2;
    draw_char(gx, gy, ' ');
    switch (c)
    { case 'A': draw_line(gx, ymid, gx + width - 1, ymid); break;
      case 'B': draw_line(xmid, ay, xmid, ay + height - 1); break;
      case 'C': draw_line(xmid, ymid, gx + width - 1, ymid);
        draw_line(xmid, ymid, xmid, ay + height - 1); break;
      case 'D': draw_line(gx, ymid, xmid, ymid);
        draw_line(xmid, ymid, xmid, ay + height - 1); break;
      case 'E': draw_line(xmid, ymid, xmid, ay);
        draw_line(xmid, ymid, gx + width - 1, ymid); break;
      case 'F': draw_line(gx, ymid, xmid, ymid);
        draw_line(xmid, ymid, xmid, ay); break;
      default: c = '?'; goto draw_ch; /*@notreached@*/ break;
    }
  }
  if (a & A_UNDERLINE)
  { (void) XDrawLine(xws_display, xws_window, xws_gc, gx, gy + 1,
      gx + width - 1, gy + 1);
  }
}

#if MIGHT_USE_SCROLL_BARS

static one_caller void redraw_scrollbar(void)
{ int width = SCROLL_BAR_WIDTH, gh = LINES * font_normal->height,
    x1 = COLS * font_normal->width, y1 = 0, x2 = x1 + width - 1, y2 = gh - 1;
  my_memclr_arr(sbc); sbc[0] = x1;
  set_fg(pixel_grey75); draw_rectangle(x1, y1, width, gh);
  if (config.flags & cfUseScrollBars)
  { int heights[3];
    int ty1, ty2, th, ty3, ty4, xmiddle, arrow_size;
    xcurses_confuser(3, heights, NULL);
    if (heights[0] >= heights[2]) return; /* everything is visible */
    x1 += SCROLL_BAR_DIST; x2 -= SCROLL_BAR_DIST;
    xmiddle = x1 + ((x2 - x1) / 2); arrow_size = width - 5;
    /* top arrow */
    set_fg(pixel_white);
    draw_line(xmiddle, SCROLL_BAR_DIST, x1, arrow_size + SCROLL_BAR_DIST);
    set_fg(pixel_black);
    draw_line(xmiddle, SCROLL_BAR_DIST, x2, arrow_size + SCROLL_BAR_DIST);
    draw_line(x1, arrow_size + SCROLL_BAR_DIST, x2,
      arrow_size + SCROLL_BAR_DIST);
    /* bottom arrow */
    set_fg(pixel_white);
    draw_line(xmiddle, y2 - SCROLL_BAR_DIST, x1,
      y2 - arrow_size - SCROLL_BAR_DIST);
    draw_line(x1, y2 - arrow_size - SCROLL_BAR_DIST, x2,
      y2 - arrow_size - SCROLL_BAR_DIST);
    set_fg(pixel_black);
    draw_line(xmiddle, y2 - SCROLL_BAR_DIST, x2,
      y2 - arrow_size - SCROLL_BAR_DIST);
    /* thumb */
    ty3 = arrow_size + SCROLL_BAR_DIST + 1;
    ty4 = y2 - arrow_size - SCROLL_BAR_DIST - 1;
    if (ty4 - ty3 < 5) return; /* not enough space (window too small) */
    th = ((ty4 - ty3) * heights[0]) / heights[2];
    if (th < MIN_THUMB_HEIGHT) th = MIN_THUMB_HEIGHT; /* make thumb "usable" */
    ty1 = ty3 + ((ty4 - ty3) * heights[1]) / heights[2];
    ty2 = ty1 + th - 1;
    if (ty2 > ty4) { ty2 = ty4; ty1 = ty2 - th + 1; if (ty1 < ty3) ty1 = ty3; }
    set_fg(pixel_white); draw_line(x1, ty1, x2, ty1);
    draw_line(x1, ty1, x1, ty2);
    set_fg(pixel_black); draw_line(x2, ty1, x2, ty2);
    draw_line(x1, ty2, x2, ty2);
    sbc[1] = ty3; sbc[2] = ty1; sbc[3] = ty2; sbc[4] = ty4;
    sbc[5] = gh; sbc[6] = heights[1]; sbc[7] = heights[2];
  }
}

#if 0 /* HAVE_GETTIMEOFDAY */

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

static tBoolean scroll_bar_timeout(/*@out@*/ int* _msec)
{
}

static one_caller void scroll_bar_timeout_on(void)
{ timeout_register(scroll_bar_timeout);
}

static one_caller void scroll_bar_timeout_off(void)
{ timeout_unregister(scroll_bar_timeout);
}

#else

static one_caller void scroll_bar_timeout_on(void) { }
static one_caller void scroll_bar_timeout_off(void) { }

#endif /* #if HAVE_GETTIMEOFDAY */

#endif /* #if MIGHT_USE_SCROLL_BARS */

static void redraw_stdscr(void)
{ int x, y;
  /* use some global variables to make all this frequently executed redrawing
     much faster... */
  rdr_t = stdscr->text; rdr_a = stdscr->attr;
  rdr_curx = stdscr->x; rdr_cury = stdscr->y;
  rdr_use_hl = cond2boolean( (sel0.state & (2 | 4)) == (2 | 4) );
  if (rdr_use_hl)
  { rdr_hlx1 = sel0.x1; rdr_hly1 = sel0.y1;
    rdr_hlx2 = sel0.x2; rdr_hly2 = sel0.y2;
    coord_reorder(&rdr_hlx1, &rdr_hly1, &rdr_hlx2, &rdr_hly2);
  }
  for (y = 0; y < LINES; y++) { for (x = 0; x < COLS; x++) redraw_char(x, y); }
#if MIGHT_USE_SCROLL_BARS
  redraw_scrollbar();
#endif
  /* use_font(font_normal); */ set_fgbg(pixel_fg, pixel_bg); __must_redraw = 0;
}

static void write_key(tKey key)
{ (void) my_write_pipe(fd_xcurses2main_write, &key, sizeof(key));
}

static one_caller void selection_highlight(int x2, int y2)
{ /* if ( (sel0.x2 != x2) || (sel0.y2 != y2) || (!(sel0.state & 4)) ) */
  { sel0.x2 = x2; sel0.y2 = y2; sel0.state |= 4; redraw_stdscr(); }
}

static void selection_unhighlight(void)
{ sel0.state = 0; redraw_stdscr(); /* KISS */
}

static one_caller void selection_copy(/*const*/ XButtonEvent* be)
{ int x1 = sel0.x1, y1 = sel0.y1, x2 = sel0.x2, y2 = sel0.y2, x, y;
  size_t size;
  char *text, *ptr;
  tBoolean need_newline = falsE;
  coord_reorder(&x1, &y1, &x2, &y2);
  size = (y2 - y1 + 1) * (COLS + 1) + 1; /* better too much than... */
  __dealloc(sel0.text);
  sel0.text = text = ptr = __memory_allocate(size, mapOther);
  x = x1;
  for (y = y1; y <= y2; y++)
  { int xend = ( (y == y2) ? x2 : COLS - 1 ), spacecount = 0;
    if (need_newline) { need_newline = falsE; *ptr++ = '\n'; }
    while (x <= xend)
    { const char ch = stdscr->text[SAI(x, y)];
      if (ch == ' ') spacecount++; /* (to ignore trailing space characters) */
      else
      { while (spacecount > 0) { *ptr++ = ' '; spacecount--; }
        *ptr++ = ch;
      }
      x++;
    }
    if (x >= COLS - 1) need_newline = truE;
    x = 0;
  }
  *ptr = '\0'; debugmsg("selection_copy(): *"); debugmsg(text);debugmsg("*\n");
  sel0.size = ptr - text; /* exact, in contrast to <size> */
  (void) XSetSelectionOwner(xws_display, XA_PRIMARY, xws_window, be->time);
  if (XGetSelectionOwner(xws_display, XA_PRIMARY) != xws_window)
    selection_unhighlight();
}

static one_caller void selection_provide(const XSelectionRequestEvent* sre)
{ XSelectionEvent e;
  Window requestor;
  if (sel0.text == NULL) return; /* "should not happen" */
  requestor = sre->requestor;
  my_memclr_var(e); e.type = SelectionNotify; e.display = sre->display;
  e.requestor = requestor; e.selection = sre->selection;
  e.target = sre->target; e.property = sre->property; e.time = sre->time;
  (void) XChangeProperty(xws_display, requestor, e.property, XA_STRING, 8,
    PropModeReplace, sel0.text, sel0.size);
  (void) XSendEvent(xws_display, requestor, False, 0, (XEvent*) (&e));
}

static one_caller void selection_paste(Time t)
{ static tBoolean did_init = falsE;
  static Atom dummy;
  if (XGetSelectionOwner(xws_display, XA_PRIMARY) == None) return;
  if (!did_init)
  { dummy = XInternAtom(xws_display, "__RETAWQ_SELECTION", False);
    did_init = truE;
  }
  (void) XConvertSelection(xws_display, XA_PRIMARY, XA_STRING, dummy,
    xws_window, t);
}

static one_caller void selection_do_paste(const XSelectionEvent* se)
{ Atom prop = se->property, type;
  int format;
  unsigned long nitems, remaining;
  unsigned char* value;
  if (prop == None) return;
  if (XGetWindowProperty(xws_display, xws_window, prop, 0, 1024, True,
      AnyPropertyType, &type, &format, &nitems, &remaining, &value) != 0)
    return;
  if (format != 8) { debugmsg("do_paste(): bad format\n"); return; }
  xcurses_confuser(0, value, &nitems);
  XFree(value);
  /* IMPLEMENTME: read the _whole_ value? */
}

static one_caller void handle_event(/*const*/ XEvent* event)
{ int width, height;
  /*const*/ XKeyEvent* ke;
  /*const*/ XButtonEvent* be;
  unsigned int button;
  KeySym sym;
  char ch;
  switch (event->type)
  { case KeyPress:
      ke = (/*const*/ XKeyEvent*) event;
      (void) XLookupString(ke, &ch, 1, &sym, NULL);
      if (IsModifierKey(sym)) { /* nothing */ }
      else if (sym != NoSymbol)
      { if ( (ke->state & ControlMask) && (my_islower(sym)) )
          sym = (sym - 'a') + 1;
        write_key(sym);
      }
      break;
    case Expose:
      if ( ((XExposeEvent*) event)->count <= 0 ) redraw_stdscr();
      break;
    case ButtonPress:
      be = (/*const*/ XButtonEvent*) event;
      button = be->button; sel0.state &= ~8;
#if MIGHT_USE_SCROLL_BARS
      if ( (sbc[0] > 0) && (be->x >= sbc[0]) ) /* inside the scroll bar */
      { unsigned char count, code;
        int y;
        if ( (button != Button1) || (sbc[1] <= 0) ) return;
        y = be->y; code = 5;
        for (count = 1; count <= 4; count++)
        { if (y <= sbc[count]) { code = count; break; } }
        if (code == 3) { sel0.state = 8; sel0.sbty = y; sel0.sbkd = sbc[6]; }
        else { xcurses_confuser(1, &code, NULL); scroll_bar_timeout_on(); }
      }
      else
#endif
      if (__mousemask != 0)
      { mmask_t m = 0;
        if (button == Button1) m |= BUTTON1_CLICKED;
        else if (button == Button2) m |= BUTTON2_CLICKED;
        else if (button == Button3) m |= BUTTON3_CLICKED;
        m &= __mousemask;
        if (m != 0)
        { tGetmouse *list, *entry;
          MEVENT* data;
          int x, y;
          if (!coord_pixel2char(be->x, be->y, &x, &y, truE)) return;
          list = getmouse_list;
          entry = memory_allocate(sizeof(tGetmouse), mapOther);
          data = &(entry->data); data->x = x; data->y = y; data->bstate = m;
          if (list == NULL) getmouse_list = entry;
          else /* rare */
          { while (list->next != NULL) list = list->next;
            list->next = entry;
          }
          write_key(KEY_MOUSE);
        }
      }
      else if (button == Button1) /* user starts text selection */
      { int gx = be->x, gy = be->y, x, y;
        if (!coord_pixel2char(gx, gy, &x, &y, falsE)) return;
        sel0.gx = gx; sel0.gy = gy;
        sel0.x1 = x; sel0.y1 = y; sel0.state |= 1 | 2;
      }
      else if ( (button == Button2) || (button == Button3) )
        selection_paste(be->time);
      break;
    case ButtonRelease:
      be = (/*const*/ XButtonEvent*) event; button = be->button;
#if MIGHT_USE_SCROLL_BARS
      if (button == Button1) { sel0.state &= ~8; scroll_bar_timeout_off(); }
#endif
      if (__mousemask != 0) goto not_sel;
      if ( (button == Button1) && (sel0.state & 1) )
      { int x, y;
        if ( (be->x == sel0.gx) && (be->y == sel0.gy) )
        { selection_unhighlight(); goto not_sel; }
        if (!coord_pixel2char(be->x, be->y, &x, &y, falsE)) goto not_sel;
        sel0.x2 = x; sel0.y2 = y; sel0.state |= 4; selection_copy(be);
      }
      not_sel: sel0.state &= ~1; break;
    case MotionNotify:
#if MIGHT_USE_SCROLL_BARS
      if (sel0.state & 8)
      { const XMotionEvent* me = (const XMotionEvent*) event;
        int y = me->y, ydiff = y - sel0.sbty, linediff =
          (ydiff * sbc[7]) / (sbc[4] - sbc[1]), l = sel0.sbkd + linediff;
        if (l <= 0) l = 0;
        else if (l >= sbc[7]) l = sbc[7] - 1;
        xcurses_confuser(2, &l, NULL);
      }
      else
#endif
      if (sel0.state & 1) /* highlight the currently selected text */
      { const XMotionEvent* me = (const XMotionEvent*) event;
        int x2, y2;
        if (coord_pixel2char(me->x, me->y, &x2, &y2, falsE))
          selection_highlight(x2, y2);
      }
      break;
    case SelectionClear:
      if (sel0.state & (2 | 4)) selection_unhighlight();
      break;
    case SelectionRequest:
      selection_provide((XSelectionRequestEvent*) event); break;
    case SelectionNotify:
      selection_do_paste((XSelectionEvent*) event); break;
    case DestroyNotify:
      do_finish: /* XCloseDisplay(xws_display); */
      do_quit(); /*@notreached@*/ break;
    case ConfigureNotify:
      width = event->xconfigure.width;
#if MIGHT_USE_SCROLL_BARS
      width -= SCROLL_BAR_WIDTH;
#endif
      width /= font_normal->width;
      if (width < CURSES_MINCOLS) width = CURSES_MINCOLS;
      else if (width > CURSES_MAXCOLS) width = CURSES_MAXCOLS;
      height = event->xconfigure.height / font_normal->height;
      if (height < CURSES_MINLINES) height = CURSES_MINLINES;
      else if (height > CURSES_MAXLINES) height = CURSES_MAXLINES;
      COLS = width; LINES = height; reinit_stdscr_data(); redraw_stdscr();
      write_key(KEY_RESIZE);
      break;
    case ClientMessage:
      if ( (event->xclient.message_type == WmProt) &&
           ( ((Atom)(event->xclient.data.l[0])) == WmDelete) )
        goto do_finish;
      break;
  }
}


/* curses interface */

int addch(chtype c)
{ const int x = stdscr->x, idx = SAI(x, stdscr->y);
  stdscr->text[idx] = c & A_CHARTEXT;
  stdscr->attr[idx] = (c & ~A_CHARTEXT) | attr;
  if (x < COLS - 1) stdscr->x++;
  must_redraw(1 | 2);
  return(OK);
}

int addstr(const char* _str)
{ const unsigned char* str = (const unsigned char*) _str; /* avoid sign ext. */
  unsigned char ch;
  while ( (ch = *str++) != '\0' ) (void) addch((chtype) ch);
  return(OK);
}

int addnstr(const char* _str, int len)
{ const unsigned char* str = (const unsigned char*) _str; /* avoid sign ext. */
  while (len-- > 0)
  { const unsigned char ch = *str++;
    if (ch == '\0') break;
    (void) addch((chtype) ch);
  }
  return(OK);
}

int attron(attr_t a)
{
#if MIGHT_USE_COLORS
  if (a & __A_COLORMARK) attr &= ~__A_COLORPAIRMASK; /* caller changes color */
#endif
  attr |= a;
  return(0);
}

int attroff(attr_t a)
{
#if MIGHT_USE_COLORS
  if (a & __A_COLORMARK) a |= __A_COLORPAIRMASK; /* caller turns color off */
#endif
  attr &= ~a;
  return(0);
}

int clrtoeol(void)
{ int x = stdscr->x;
  if (x < COLS)
  { const int y = stdscr->y;
    while (x < COLS)
    { const int idx = SAI(x, y);
      stdscr->text[idx] = ' '; stdscr->attr[idx] = 0; x++;
    }
    must_redraw(2);
  }
  return(OK);
}

int getch(void)
{ tKey key;
  if (my_read_pipe(fd_keyboard_input, &key, sizeof(key)) != sizeof(key))
    fatal_error(0, "xcurses getch() failed");
  return(key);
}

int getmouse(MEVENT* e)
{ tGetmouse* entry = getmouse_list;
  if (entry == NULL) return(ERR); /* "should not happen" */
  my_memcpy(e, &(entry->data), sizeof(MEVENT));
  getmouse_list = entry->next; memory_deallocate(entry);
  return(OK);
}

#if MIGHT_USE_COLORS
int has_colors(void)
{ return( (XDisplayCells(xws_display, DefaultScreen(xws_display)) > 2)
    ? TRUE : FALSE );
}
#endif

chtype inch(void)
{ const int idx = SAI(stdscr->x, stdscr->y);
  return(stdscr->text[idx] | stdscr->attr[idx]);
}

#if MIGHT_USE_COLORS
int init_pair(short p, short fg, short bg)
{ unsigned long pix = pixel_fg, pix0;
  const char* name;
  switch (fg)
  { case ccRed: name = "red"; break;
    case ccBlue: name = "blue"; break;
    default: goto out; /*@notreached@*/ break;
  }
  if ( (alloc_color(name, &pix0)) && (pix0 != pixel_bg) ) pix = pix0;
  out:
  colors[p] = pix;
  return(OK);
}
#endif

mmask_t mousemask(mmask_t new_mask, mmask_t* old_mask)
{ if (old_mask != NULL) *old_mask = __mousemask;
  __mousemask = new_mask;
  set_cursor(__mousemask ? cursor_arrow : cursor_textmark);
  return(__mousemask);
}

int move(int y, int x)
{ coord_limit(&x, &y);
  stdscr->x = x; stdscr->y = y; must_redraw(1);
  return(OK);
}

int mvaddch(int y, int x, chtype ch)
{ (void) move(y, x); (void) addch(ch);
  return(OK);
}

int mvaddnstr(int y, int x, const char* str, int len)
{ (void) move(y, x); (void) addnstr(str, len);
  return(OK);
}

int refresh(void)
{ loop:
  while (XPending(xws_display) > 0)
  { XEvent event;
    (void) XNextEvent(xws_display, &event); /* CHECKME: return value? */
    handle_event(&event);
  }
  if (__must_redraw) { redraw_stdscr(); goto loop; }
  return(OK);
}
