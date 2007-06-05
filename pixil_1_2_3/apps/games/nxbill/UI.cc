 /*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "objects.h"
#include "x11.h"
#include "Strings.h"

Widget toplevel, base, menubar, field;
Widget scorebox, highscorebox;
Widget endgamebox, enternamebox;
Widget warpbox, quitbox, newgamebox, pausebox;
Widget aboutbox, rulesbox, storybox;

/**************************/
/* Timer control routines */
/**************************/

void
UI::restart_timer()
{
    timer = XtAppAddTimeOut(app, 250, timer_eh, NULL);	/* 250 ms */
}

void
UI::kill_timer()
{
    if (timer) {
	XtRemoveTimeOut(timer);
	timer = (XtIntervalId) 0;
    }
}

void
UI::pause_game()
{
    if (timer)
	playing = 1;
    kill_timer();
}

void
UI::resume_game()
{
    if (playing && !timer)
	restart_timer();
    playing = 0;
}

/*******************/
/* Window routines */
/*******************/

void
UI::initialize(int *argc, char **argv)
{
    toplevel = XtAppInitialize(&app, "XBill", NULL, 0, argc, argv,
			       NULL, NULL, 0);
}

void
UI::make_mainwin()
{
    Screen *screen;
    XrmDatabase database;
    XSizeHints h;
    Dimension winwidth, winheight;

/*
	XtAddEventHandler(toplevel, (EventMask)0, TRUE,
		_XEditResCheckMessages, NULL);
*/
    display = XtDisplay(toplevel);

    database = XrmGetDatabase(display);
    XrmPutStringResource(&database, "*background", "#c4c4c4");
    XrmSetDatabase(display, database);

    base = CreateRowCol("base", toplevel);
    menubar = CreateMenuBar("menubar", base);
    field = CreateDrawingArea("field", base, game.scrwidth, game.scrheight);
    XtAddEventHandler(field, ButtonPressMask, FALSE,
		      (XtEventHandler) button_press_eh, NULL);
    XtAddEventHandler(field, ButtonReleaseMask, FALSE,
		      (XtEventHandler) button_release_eh, NULL);
    XtAddEventHandler(field, LeaveWindowMask, FALSE,
		      (XtEventHandler) leave_window_eh, NULL);
    XtAddEventHandler(field, EnterWindowMask, FALSE,
		      (XtEventHandler) enter_window_eh, NULL);
    XtAddEventHandler(field, ExposureMask, FALSE,
		      (XtEventHandler) redraw_window_eh, NULL);

    XtRealizeWidget(toplevel);
    screen = XtScreen(toplevel);
    depth = DefaultDepthOfScreen(screen);
    rootwindow = RootWindowOfScreen(screen);
    window = XtWindow(field);

    colormap = DefaultColormapOfScreen(screen);
    white.pixel = WhitePixelOfScreen(screen);
    XQueryColor(display, colormap, &white);
    black.pixel = BlackPixelOfScreen(screen);
    XQueryColor(display, colormap, &black);

    XtVaGetValues(toplevel, XtNwidth, &winwidth, XtNheight, &winheight, NULL);
    h.width = h.base_width = h.min_width = h.max_width = winwidth;
    h.height = h.base_height = h.min_height = h.max_height = winheight;
    h.flags = USSize | PSize | PMaxSize | PMinSize;
    XSetNormalHints(display, window, &h);
}

void
UI::make_windows()
{
    Picture about;

    icon.load("icon");
    XtVaSetValues(toplevel, XtNiconPixmap, icon.pix, NULL);

    newgamebox = CreateDialog("New Game", base, OK | CANCEL, (Pixmap) NULL,
			      newgamestr, (char *) NULL, new_game_cb);
    pausebox = CreateDialog("Pause Game", base, OK, icon.pix,
			    pausestr, "Continue", NULL);
    quitbox = CreateDialog("Quit", base, OK | CANCEL, (Pixmap) NULL,
			   quitstr, (char *) NULL, quit_game_cb);
    warpbox = CreateEnterText("Warp To Level", base, warpstr,
			      (XtCallbackProc) warp_apply);
    about.load("about");

    aboutbox = CreatePixmapBox("About", base, about.pix, "");
    rulesbox = CreatePixmapBox("Rules", base, (Pixmap) NULL, rulesstr);
    storybox = CreatePixmapBox("Story", base, (Pixmap) NULL, storystr);

    scorebox = CreateDialog("Score", base, OK, (Pixmap) NULL,
			    "", (char *) NULL, NULL);
    endgamebox = CreateDialog("Endgame", base, OK, (Pixmap) NULL,
			      endgamestr, "Nuts!", NULL);
    highscorebox = CreateDialog("HighScore", base, OK, (Pixmap) NULL,
				"", (char *) NULL, NULL);
    enternamebox = CreateEnterText("Enter Name", base,
				   enternamestr, (XtCallbackProc) enter_name);
}

void
UI::popup_dialog(int dialog)
{
    Widget w;
    switch (dialog) {
    case game.ENTERNAME:
	w = enternamebox;
	break;
    case game.HIGHSCORE:
	w = highscorebox;
	break;
    case game.SCORE:
	w = scorebox;
	break;
    case game.ENDGAME:
	w = endgamebox;
	break;
    }
    popup(NULL, &w, NULL);
}

/*********************/
/* Graphics routines */
/*********************/

void
UI::set_cursor(int cursor)
{
    switch (cursor) {
    case game.BUCKETC:
	XDefineCursor(display, window, bucket.cursor.cursor);
	break;
    case game.DOWNC:
	XDefineCursor(display, window, downcursor.cursor);
	break;
    case game.DEFAULTC:
	XDefineCursor(display, window, defaultcursor.cursor);
	break;
    default:
	XDefineCursor(display, window, OS.cursor[cursor].cursor);
    }
}

void
UI::load_cursors()
{
    defaultcursor.load("hand_up", defaultcursor.SEP_MASK);
    XDefineCursor(display, window, defaultcursor.cursor);
    downcursor.load("hand_down", downcursor.SEP_MASK);
}

void
UI::graph_init()
{
    XGCValues gcval;
    unsigned long gcmask;
    gcmask = GCGraphicsExposures;
    gcval.graphics_exposures = False;
    stdgc = XCreateGC(display, window, gcmask, &gcval);
    XSetLineAttributes(display, stdgc, 3, LineSolid, CapRound, JoinMiter);
    XSetBackground(display, stdgc, white.pixel);
    XSetForeground(display, stdgc, black.pixel);
    whitegc = XCreateGC(display, window, gcmask, &gcval);
    XSetBackground(display, whitegc, white.pixel);
    XSetForeground(display, whitegc, white.pixel);

    offscreen = XCreatePixmap(display, rootwindow, game.scrwidth,
			      game.scrheight, depth);
}

void
UI::clear()
{
    XFillRectangle(display, offscreen, whitegc, 0, 0,
		   game.scrwidth, game.scrheight);
}

void
UI::refresh()
{
    XCopyArea(display, offscreen, window, stdgc, 0, 0,
	      game.scrwidth, game.scrheight, 0, 0);
}

void
UI::draw(Picture pict, int x, int y)
{
    XSetClipOrigin(display, pict.gc, x, y);
    XCopyArea(display, pict.pix, offscreen, pict.gc, 0, 0,
	      pict.width, pict.height, x, y);
}

void
UI::draw_centered(Picture pict)
{
    draw(pict, (game.scrwidth - pict.width) / 2,
	 (game.scrheight - pict.height) / 2);
}

void
UI::draw_line(int x1, int y1, int x2, int y2)
{
    XDrawLine(display, offscreen, stdgc, x1, y1, x2, y2);
}

void
UI::draw_str(char *str, int x, int y)
{
    XDrawString(display, offscreen, stdgc, x, y, str, strlen(str));
}


/******************/
/* Other routines */
/******************/

void
UI::set_pausebutton(int action)
{
    Widget w = XtNameToWidget(menubar,
#ifdef athena
			      "Game.menu.Pause game");
#else
			      "popup_menu.menu.Pause game");
#endif
    if (w)
	XtSetSensitive(w, action);
}


void
UI::MainLoop()
{
    XtAppMainLoop(app);
}
