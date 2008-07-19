 /*
 * Copyright (C) 200-2002 Century Embedded Techonlogies
 *
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

/* Nano-X UI for XBill rewritten by Jordan Crouse */
/* Embed this, baby! */


#include "objects.h"
#include "Strings.h"
#include "NXwidgets.h"
#include "NXUI.h"
#include "NX.h"

/**************************/
/* Timer control routines */
/**************************/

void
UI::restart_timer()
{
    playing = 0;
    timeout = 250;
}

void
UI::kill_timer()
{
    timeout = 0;
}

void
UI::pause_game()
{
    if (timeout)
	playing = 1;
    kill_timer();
}

void
UI::resume_game()
{
    if (playing && !timeout)
	restart_timer();
    playing = 0;
}

/*******************/
/* Window routines */
/*******************/

void
UI::initialize(int *argc, char **argv)
{
    /* Open the graphics */

    if (GrOpen() < 0)
	exit(-1);

    /* Allocate a font for use by everybody */
    global_font = GrCreateFont((GR_CHAR *) GR_FONT_GUI_VAR, 0, 0);

    toplevel = createNXApp("nxBill", game.scrwidth, game.scrheight);
}

void
UI::make_mainwin()
{
    /* Give the menubar its own GC */

    menubargc = GrNewGC();
    GrSetGCFont(menubargc, global_font);

    menubar = createNXWidget(toplevel, 0, 0, game.scrwidth, 30);

    registerNXWidgetCallback(menubar, GR_EVENT_TYPE_EXPOSURE,
			     redraw_menubar_eh);
    registerNXWidgetCallback(menubar, GR_EVENT_TYPE_BUTTON_UP,
			     menubar_buttonup_eh);
    registerNXWidgetCallback(menubar, GR_EVENT_TYPE_BUTTON_DOWN,
			     menubar_buttondown_eh);

    /* Set some text for it */

    field = createNXWidget(toplevel, 0, 30, game.playwidth, game.playheight);

    /* And register the callbacks for later use */
    registerNXWidgetCallback(field, GR_EVENT_TYPE_BUTTON_DOWN,
			     button_press_eh);
    registerNXWidgetCallback(field, GR_EVENT_TYPE_BUTTON_UP,
			     button_release_eh);

#ifndef PDA			/* These are only valid for the non pda version */
    registerNXWidgetCallback(field, GR_EVENT_TYPE_MOUSE_ENTER,
			     enter_window_eh);
    registerNXWidgetCallback(field, GR_EVENT_TYPE_MOUSE_EXIT,
			     leave_window_eh);
#endif

    registerNXWidgetCallback(field, GR_EVENT_TYPE_EXPOSURE, redraw_window_eh);

    /* Map the various widgets */

    realizeNXWidget(menubar);
    realizeNXWidget(field);

#ifdef NOTUSED
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
#endif

}

void
UI::make_windows()
{

    /* Dialog boxes? */
    endgamebox = createNXDialog(toplevel, endgamestr);
    scorebox = createNXDialog(toplevel, NULL);

#ifdef NOTYET
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

    highscorebox = CreateDialog("HighScore", base, OK, (Pixmap) NULL,
				"", (char *) NULL, NULL);
#endif

}

void
UI::popup_dialog(int dialog)
{
    GR_WINDOW_ID w = 0;

    switch (dialog) {
    case ENDGAME:
	w = endgamebox;
	break;
    }

    if (w)
	popupNXDialog(w, NULL);
}

/*********************/
/* Graphics routines */
/*********************/

/* If we are on a PDA, don't bother with */
/* mouse cursors */

#ifndef PDA

void
UI::set_cursor(int cursor)
{
    switch (cursor) {
    case BUCKETC:
	bucket.cursor.setCursor(toplevel);
	break;
    case DOWNC:
	bucket.cursor.setCursor(toplevel);
	break;
    case DEFAULTC:
	bucket.cursor.setCursor(toplevel);
	break;
    default:
	bucket.cursor.setCursor(toplevel);
	break;
    }
}

void
UI::load_cursors()
{

    defaultcursor.load("hand_up", defaultcursor.SEP_MASK);

    defaultcursor.setCursor(toplevel);

    downcursor.load("hand_down", downcursor.SEP_MASK);
}
#endif

void
UI::graph_init()
{
    /* Create a gc */

    stdgc = GrNewGC();

    /* Set the font that we want to use */
    GrSetGCFont(stdgc, global_font);

    /* Not used, yet */
    /* XSetLineAttributes (display, stdgc, 3, LineSolid, CapRound, JoinMiter); */

    GrSetGCBackground(stdgc, WHITE);
    GrSetGCForeground(stdgc, BLACK);

    whitegc = GrNewGC();
    GrSetGCBackground(stdgc, WHITE);
    GrSetGCForeground(stdgc, WHITE);

    /* Create an offscreen pixmap for good times */


    //offscreen = GrNewWindow(GR_ROOT_WINDOW_ID, 0, 0, game.playwidth, game.playheight, 0, BLACK, WHITE);
    //GrMapWindow(offscreen);

    offscreen = GrNewPixmap(game.playwidth, game.playheight, NULL);
}

void
UI::clear()
{
    GrFillRect(offscreen, whitegc, 0, 0, game.playwidth, game.playheight);
}

void
UI::refresh()
{
    GrCopyArea(field, stdgc, 0, 0, game.playwidth, game.playheight,
	       offscreen, 0, 0, MWROP_SRCCOPY);
}

void
UI::draw(NXPicture * pict, int x, int y)
{
    GR_GC_ID tgc;

#ifdef NOTUSED
    GR_REGION_ID clipregion;
    GR_RECT rect;
    clipregion = GrNewRegion();

    rect.x = x;
    rect.y = y;
    rect.width = pict->width;
    rect.height = pict->height;

    GrUnionRectWithRegion(clipregion, &rect);
#endif

    tgc = GrNewGC();

#ifdef NOTUSED
    GrSetGCRegion(tgc, clipregion);
#endif

#ifdef NOTUSED
    GrCopyArea(offscreen, tgc, x, y, pict->width, pict->height,
	       pict->pixmap, 0, 0, MWROP_SRCCOPY);
#endif

    GrDrawImageToFit(offscreen, tgc, x, y,
		     pict->width, pict->height, pict->image);

    GrDestroyGC(tgc);

#ifdef NOTUSED
    GrDestroyRegion(clipregion);
#endif
}

void
UI::draw_centered(NXPicture * pict)
{
    draw(pict, (game.playwidth - pict->width) / 2,
	 (game.playheight - pict->height) / 2);
}

void
UI::draw_line(int x1, int y1, int x2, int y2)
{
    GrSetGCForeground(stdgc, BLACK);
    GrLine(offscreen, stdgc, x1, y1, x2, y2);
}

void
UI::draw_str(char *str, int x, int y)
{
    GrSetGCForeground(stdgc, WHITE);
    GrFillRect(offscreen, stdgc, 0, y, game.playwidth, game.playheight);
    GrPoint(offscreen, stdgc, x, y);

    GrSetGCForeground(stdgc, BLACK);
    GrText(offscreen, stdgc, x, y, str, strlen(str), 0);
}


/******************/
/* Other routines */
/******************/

void
UI::set_pausebutton(int action)
{
}


void
UI::MainLoop()
{
    GR_EVENT event;

    /* Ok, so we want to handle the main loop here */

    while (1) {
	if (timeout)
	    GrGetNextEventTimeout(&event, timeout);
	else
	    GrGetNextEvent(&event);

	switch (event.type) {
	case GR_EVENT_TYPE_BUTTON_DOWN:
	    fireNXWidgetCallback(event.button.wid, &event);
	    break;

	case GR_EVENT_TYPE_BUTTON_UP:
	    fireNXWidgetCallback(event.button.wid, &event);
	    break;

	case GR_EVENT_TYPE_MOUSE_ENTER:
	    break;

	case GR_EVENT_TYPE_MOUSE_EXIT:
	    break;

	case GR_EVENT_TYPE_EXPOSURE:
	    fireNXWidgetCallback(event.exposure.wid, &event);
	    break;

	case GR_EVENT_TYPE_TIMEOUT:
	    timer_eh();
	    break;

	case GR_EVENT_TYPE_CLOSE_REQ:
	    close_program();
	}
    }
}

/******* MENUBAR CALLBACKS ******** */


#define BUTTON_SIZE 70
#define BUTTON_SPACING 3

static char *button_labels[] = { "New Game", "Pause Game", "Exit Game" };

static GR_BOOL
PtInRect(GR_RECT * prc, GR_SIZE x, GR_SIZE y)
{
    return (x >= prc->x && x < (prc->x + prc->width) &&
	    y >= prc->y && y < (prc->y + prc->height));
}

void
UI::menubar_buttonup(int x, int y)
{
    GR_WINDOW_INFO winfo;

    int bsize, startx, bspacing;
    GR_RECT rect_start, rect_pause, rect_end;

    GrGetWindowInfo(menubar, &winfo);

    bsize = (winfo.width / 2) - (BUTTON_SIZE / 2);
    startx = (bsize - BUTTON_SIZE) / 2;
    bspacing = (bsize - startx) - BUTTON_SIZE;

    /* Check for a button down */

    rect_start.x = startx;
    rect_start.y = 5;
    rect_start.width = BUTTON_SIZE;
    rect_start.height = 20;

    rect_pause.x = rect_start.x + (BUTTON_SIZE + bspacing);
    rect_pause.y = rect_start.y;
    rect_pause.width = BUTTON_SIZE;
    rect_pause.height = rect_start.height;

    rect_end.x = rect_pause.x + (BUTTON_SIZE + bspacing);
    rect_end.y = rect_pause.y;
    rect_end.width = BUTTON_SIZE;
    rect_end.height = rect_pause.height;

    if (PtInRect(&rect_start, x, y))
	game.warp_to_level(1);
    else if (PtInRect(&rect_pause, x, y)) {
	if (playing)
	    resume_game();
	else
	    pause_game();

	redraw_menubar();
    } else if (PtInRect(&rect_end, x, y))
	close_program();

}

void
UI::redraw_menubar()
{
    GR_WINDOW_INFO winfo;

    int i, state;
    int bsize, startx, bspacing;

    if (!menubargc)
	return;

    /* Get the window information so we know how big to make the screen */
    GrGetWindowInfo(menubar, &winfo);

    GrSetGCForeground(menubargc, GRAY);
    GrFillRect(menubar, menubargc, 0, 0, winfo.width, winfo.height);

    /* If we are playing PDA, then ensure that we go with the 2D look */

#ifdef PDA
    GrSetGCForeground(menubargc, BLACK);
    GrRect(menubar, menubargc, 0, 0, winfo.width, winfo.height);
#else
    nxDraw3dBox(menubar, 0, 0, winfo.width, winfo.height, (GR_COLOR) WHITE,
		(GR_COLOR) BLACK);
#endif

    /* We want the middle button to be centered, so figure how much room that leaves us */
    /* for the other two */

    bsize = (winfo.width / 2) - (BUTTON_SIZE / 2);
    startx = (bsize - BUTTON_SIZE) / 2;
    bspacing = (bsize - startx) - BUTTON_SIZE;

    for (i = 0; i < 3; i++) {
	if (i == 1) {
	    state = playing;
	} else
	    state = 0;

#ifdef PDA
	if (!state)
	    GrSetGCForeground(menubargc, LTGRAY);
	else
	    GrSetGCForeground(menubargc, BLACK);
#else
	GrSetGCForeground(menubargc, LTGRAY);
#endif

	GrFillRect(menubar, menubargc,
		   startx + (i * (BUTTON_SIZE + bspacing)),
		   5, BUTTON_SIZE, 20);

#ifdef PDA
	GrSetGCForeground(menubargc, BLACK);
	GrRect(menubar, menubargc, startx + (i * (BUTTON_SIZE + bspacing)), 5,
	       BUTTON_SIZE, 20);
#else
	nxDraw3dUpDownState(menubar, startx + (i * (BUTTON_SIZE + bspacing)),
			    5, BUTTON_SIZE, 20, state);
#endif


#ifdef PDA
	if (!state) {
	    GrSetGCBackground(menubargc, LTGRAY);
	    GrSetGCForeground(menubargc, BLACK);
	} else {
	    GrSetGCBackground(menubargc, BLACK);
	    GrSetGCForeground(menubargc, LTGRAY);
	}
#else
	GrSetGCBackground(menubargc, LTGRAY);
	GrSetGCForeground(menubargc, BLACK);
#endif

	GrText(menubar, menubargc,
	       (startx + 3) + (i * (BUTTON_SIZE + bspacing)), 20,
	       button_labels[i], strlen(button_labels[i]), 0);
    }
}

void
UI::update_scorebox(int level, int score)
{
    char scorestr[100];

    sprintf(scorestr,
	    "After %d grueling levels\nyour mind bending score is %d!\nKeep going!",
	    level, score);

    popupNXDialog(scorebox, scorestr);
}

/* We really don't have a high score box right now..  sorry */

void
UI::update_hsbox(char *str)
{
    return;
}

void
UI::close_program()
{
    /* Images will kill themselves as the program dies */

    killNXApp(toplevel);
    GrClose();
    exit(0);
}

/* Moved here from the headre file */

UI::UI()
{
    playing = 0;
    timeout = 0;
}
