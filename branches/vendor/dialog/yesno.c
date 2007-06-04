/*
 *  $Id$
 *
 *  yesno.c -- implements the yes/no box
 *
 *  Copyright 1999-2004,2005	Thomas E. Dickey
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
 *  An earlier version of this program lists as authors
 *	Savio Lam (lam836@cs.cuhk.hk)
 */

#include <dialog.h>
#include <dlg_keys.h>

/*
 * Display a dialog box with two buttons - Yes and No.
 */
int
dialog_yesno(const char *title, const char *cprompt, int height, int width)
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

    int x, y;
    int key = 0, fkey;
    int code;
    int button = dlg_defaultno_button();
    WINDOW *dialog = 0;
    int result = DLG_EXIT_UNKNOWN;
    char *prompt = dlg_strclone(cprompt);
    const char **buttons = dlg_yes_labels();

#ifdef KEY_RESIZE
    int req_high = height;
    int req_wide = width;
  restart:
#endif

    dlg_tab_correct_str(prompt);
    dlg_auto_size(title, prompt, &height, &width, 2, 25);
    dlg_print_size(height, width);
    dlg_ctl_size(height, width);

    x = dlg_box_x_ordinate(width);
    y = dlg_box_y_ordinate(height);

#ifdef KEY_RESIZE
    if (dialog != 0) {
	(void) wresize(dialog, height, width);
	(void) mvwin(dialog, y, x);
	(void) refresh();
    } else
#endif
    {
	dialog = dlg_new_window(height, width, y, x);
	dlg_register_window(dialog, "yesno", binding);
	dlg_register_buttons(dialog, "yesno", buttons);
    }

    dlg_draw_box(dialog, 0, 0, height, width, dialog_attr, border_attr);
    dlg_draw_bottom_box(dialog);
    dlg_draw_title(dialog, title);

    wattrset(dialog, dialog_attr);
    dlg_print_autowrap(dialog, prompt, height, width);

    dlg_draw_buttons(dialog, height - 2, 0, buttons, button, FALSE, width);

    while (result == DLG_EXIT_UNKNOWN) {
	key = dlg_mouse_wgetch(dialog, &fkey);
	if (dlg_result_key(key, fkey, &result))
	    break;
	if ((code = dlg_char_to_button(key, buttons)) >= 0) {
	    result = dlg_ok_buttoncode(code);
	    break;
	}
	/* handle function keys */
	if (fkey) {
	    switch (key) {
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
		result = dlg_yes_buttoncode(button);
		break;
#ifdef KEY_RESIZE
	    case KEY_RESIZE:
		dlg_clear();
		height = req_high;
		width = req_wide;
		goto restart;
#endif
	    default:
		if (is_DLGK_MOUSE(key)) {
		    result = dlg_yes_buttoncode(key - M_EVENT);
		    if (result < 0)
			result = DLG_EXIT_OK;
		} else {
		    beep();
		}
		break;
	    }
	} else {
	    beep();
	}
    }

    dlg_del_window(dialog);
    dlg_mouse_free_regions();
    free(prompt);
    return result;
}
