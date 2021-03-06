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

static int in_popup = 0;

/**********************/
/* Callback functions */
/**********************/

void
new_game_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    game.start(1);
}

void
quit_game_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    game.quit();
}

void
get_coords(Position * x, Position * y)
{
    XWindowAttributes wattr;
    Window junk;
    int rx, ry;
    XGetWindowAttributes(ui.display, ui.window, &wattr);
    XTranslateCoordinates(ui.display, ui.window, wattr.root,
			  -wattr.border_width, -wattr.border_width, &rx, &ry,
			  &junk);
    *x = rx + 20;
    *y = ry + 40;
}

void
popup(Widget w, Widget * box, XtPointer call_data)
{
    Position x, y;
    in_popup = 1;
#ifdef athena
    get_coords(&x, &y);
    XtMoveWidget(XtParent(*box), x, y);
#endif
    XtManageChild(*box);
    XtAddCallback(XtParent(*box), XtNpopdownCallback,
		  (XtCallbackProc) popdown, NULL);
    XtPopup(XtParent(*box), XtGrabExclusive);
    while (in_popup || XtAppPending(ui.app))
	XtAppProcessEvent(ui.app, XtIMXEvent);
}

void
popdown(Widget w, XtPointer client_data, XtPointer call_data)
{
    in_popup = 0;
}

/******************/
/* Event handlers */
/******************/

void
leave_window_eh(Widget w, XtPointer client_data, XEvent * event)
{
    ui.pause_game();
}

void
enter_window_eh(Widget w, XtPointer client_data, XEvent * event)
{
    ui.resume_game();
}

void
redraw_window_eh(Widget w, XtPointer client_data, XEvent * event)
{
    ui.refresh();
}

void
button_press_eh(Widget w, XtPointer data, XButtonEvent * event)
{
    game.button_press(event->x, event->y);
}

void
button_release_eh(Widget w, XtPointer data, XButtonEvent * event)
{
    game.button_release(event->x, event->y);
}

void
timer_eh(XtPointer client_data, XtIntervalId * timer_id)
{
    ui.restart_timer();
    game.update();
}
