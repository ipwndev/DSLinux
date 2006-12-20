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

#include "objects.h"
#include "NX.h"

//static int in_popup = 0;

/**********************/
/* Callback functions */
/**********************/

void
new_game_cb(GR_WINDOW_ID w)
{
    game.start(1);
}

void
quit_game_cb(GR_WINDOW_ID w)
{
    game.quit();
}

void
get_coords(GR_EVENT * event, int *x, int *y)
{


    *x = event->mouse.x;
    *y = event->mouse.y;
}

#ifdef NOTUSED
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
#endif

/******************/
/* Event handlers */
/******************/

void
redraw_menubar_eh(GR_EVENT * event)
{
    ui.redraw_menubar();
}

void
menubar_buttonup_eh(GR_EVENT * event)
{
    ui.menubar_buttonup(event->button.x, event->button.y);
}

void
menubar_buttondown_eh(GR_EVENT * event)
{
    /* NOP */
}

void
leave_window_eh(GR_EVENT * event)
{
    ui.pause_game();
}

void
enter_window_eh(GR_EVENT * event)
{
    ui.resume_game();
}

void
redraw_window_eh(GR_EVENT * event)
{
    ui.refresh();
}

void
button_press_eh(GR_EVENT * event)
{
    game.button_press(event->button.x, event->button.y);
}

void
button_release_eh(GR_EVENT * event)
{
    game.button_release(event->button.x, event->button.y);
}

void
timer_eh()
{
    ui.restart_timer();
    game.update();
}
