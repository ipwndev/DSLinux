/*
 *  $Id$
 *
 *  pause.c -- implements the pause dialog
 *
 *  Copyright 2004-2005,2006	Thomas E. Dickey
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to
 *	Free Software Foundation, Inc.
 *	51 Franklin St., Fifth Floor
 *	Boston, MA 02110, USA.
 *
 *  This is adapted from source contributed by
 *	Yura Kalinichenko
 */

#include <dialog.h>
#include <dlg_keys.h>

#define MY_TIMEOUT 50

/*
 * This is like gauge, but can be interrupted.
 *
 * A pause box displays a meter along the bottom of the box.  The meter
 * indicates how many seconds remain until the end of the pause.  The pause
 * exits when timeout is reached (status OK) or the user presses the Exit
 * button (status CANCEL).
 */
int
dialog_pause(const char *title,
	     const char *cprompt,
	     int height,
	     int width,
	     int seconds)
{
    /* *INDENT-OFF* */
    static DLG_KEYS_BINDING binding[] = {
	ENTERKEY_BINDINGS,
	DLG_KEYS_DATA( DLGK_ENTER,	' ' ),
	DLG_KEYS_DATA( DLGK_FIELD_NEXT,	KEY_DOWN ),
	DLG_KEYS_DATA( DLGK_FIELD_NEXT, KEY_RIGHT ),
	DLG_KEYS_DATA( DLGK_FIELD_NEXT, TAB ),
	DLG_KEYS_DATA( DLGK_FIELD_PREV,	KEY_UP ),
	DLG_KEYS_DATA( DLGK_FIELD_PREV, KEY_BTAB ),
	DLG_KEYS_DATA( DLGK_FIELD_PREV, KEY_LEFT ),
	END_KEYS_BINDING
    };
    /* *INDENT-ON* */

#ifdef KEY_RESIZE
    int old_height = height;
    int old_width = width;
#endif

    int i, x, y, step;
    int button = 0;
    int seconds_orig;
    WINDOW *dialog;
    const char **buttons = dlg_exit_label();
    int key = 0, fkey;
    int result = DLG_EXIT_UNKNOWN;
    char *prompt = dlg_strclone(cprompt);

    curs_set(0);

    dlg_tab_correct_str(prompt);

    seconds_orig = (seconds > 0) ? seconds : 1;

#ifdef KEY_RESIZE
  retry:
    height = old_height;
    width = old_width;
#endif

    dlg_auto_size(title, prompt, &height, &width, 0, 0);
    dlg_print_size(height, width);
    dlg_ctl_size(height, width);

    /* center dialog box on screen */
    x = dlg_box_x_ordinate(width);
    y = dlg_box_y_ordinate(height);

    dialog = dlg_new_window(height, width, y, x);
    dlg_register_window(dialog, "pause", binding);
    dlg_register_buttons(dialog, "pause", buttons);

    dlg_mouse_setbase(x, y);
    nodelay(dialog, TRUE);

    do {
	(void) werase(dialog);
	dlg_draw_box(dialog, 0, 0, height, width, dialog_attr, border_attr);

	dlg_draw_title(dialog, title);

	wattrset(dialog, dialog_attr);
	dlg_print_autowrap(dialog, prompt, height, width);

	dlg_draw_box(dialog,
		     height - 6, 2 + MARGIN,
		     2 + MARGIN, width - 2 * (2 + MARGIN),
		     dialog_attr,
		     border_attr);

	/*
	 * Clear the area for the progress bar by filling it with spaces
	 * in the title-attribute, and write the percentage with that
	 * attribute.
	 */
	(void) wmove(dialog, height - 5, 4);
	wattrset(dialog, title_attr);

	for (i = 0; i < (width - 2 * (3 + MARGIN)); i++)
	    (void) waddch(dialog, ' ');

	(void) wmove(dialog, height - 5, (width / 2) - 2);
	(void) wprintw(dialog, "%3d", seconds);

	/*
	 * Now draw a bar in reverse, relative to the background.
	 * The window attribute was useful for painting the background,
	 * but requires some tweaks to reverse it.
	 */
	x = (seconds * (width - 2 * (3 + MARGIN))) / seconds_orig;
	if ((title_attr & A_REVERSE) != 0) {
	    wattroff(dialog, A_REVERSE);
	} else {
	    wattrset(dialog, A_REVERSE);
	}
	(void) wmove(dialog, height - 5, 4);
	for (i = 0; i < x; i++) {
	    chtype ch = winch(dialog);
	    if (title_attr & A_REVERSE) {
		ch &= ~A_REVERSE;
	    }
	    (void) waddch(dialog, ch);
	}

	dlg_draw_bottom_box(dialog);
	mouse_mkbutton(height - 2, width / 2 - 4, 6, '\n');
	dlg_draw_buttons(dialog, height - 2, 0, buttons, button, FALSE, width);
	(void) wrefresh(dialog);

	for (step = 0;
	     (result == DLG_EXIT_UNKNOWN) && (step < 1000);
	     step += MY_TIMEOUT) {

	    napms(MY_TIMEOUT);
	    key = dlg_mouse_wgetch_nowait(dialog, &fkey);
	    if (dlg_result_key(key, fkey, &result))
		break;

	    switch (key) {
#ifdef KEY_RESIZE
	    case KEY_RESIZE:
		dlg_clear();	/* fill the background */
		dlg_del_window(dialog);		/* delete this window */
		refresh();	/* get it all onto the terminal */
		goto retry;
#endif
	    case DLGK_FIELD_NEXT:
		button = dlg_next_button(buttons, button);
		if (button < 0)
		    button = 0;
		dlg_draw_buttons(dialog,
				 height - 2, 0,
				 buttons, button,
				 FALSE, width);
		break;
	    case DLGK_FIELD_PREV:
		button = dlg_prev_button(buttons, button);
		if (button < 0)
		    button = 0;
		dlg_draw_buttons(dialog,
				 height - 2, 0,
				 buttons, button,
				 FALSE, width);
		break;
	    case DLGK_ENTER:
		/* Do not use dlg_exit_buttoncode() since we want to return
		 * a cancel rather than ok if the timeout has not expired.
		 */
		result = button ? DLG_EXIT_HELP : DLG_EXIT_CANCEL;
		break;
	    case DLGK_MOUSE(0):
		result = DLG_EXIT_CANCEL;
		break;
	    case DLGK_MOUSE(1):
		result = DLG_EXIT_HELP;
		break;
	    case ERR:
		break;
	    default:
		result = DLG_EXIT_CANCEL;
		break;
	    }
	}
    } while ((result == DLG_EXIT_UNKNOWN) && (seconds-- > 0));

    curs_set(1);
    dlg_mouse_free_regions();
    dlg_del_window(dialog);
    free(prompt);
    return (result);
}
