/* editor initialisation and callback handler.

   Copyright (C) 1996, 1997 the Free Software Foundation

   Authors: 1996, 1997 Paul Sheer

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.
 */

#include <config.h>

#include "src/tty.h"		/* LINES */

#include "edit.h"
#include "edit-widget.h"

#include "src/main.h"		/* clock */
#include "src/widget.h"		/* redraw_labels() */
#include "src/wtools.h"		/* redraw_labels() */
#include "src/dialog.h"		/* redraw_labels() */
#include "src/menu.h"		/* menubar_new() */
#include "src/key.h"		/* is_idle() */
#include "src/main.h"		/* is_idle() */

#ifdef HAVE_DLGSWITCH
#define NEED_EDIT
#include "src/dlglist.h"
#include "vfs/vfs.h"
#include "src/panel.h"
#endif

WEdit *wedit;
WButtonBar *edit_bar;
Dlg_head *edit_dlg;
WMenu *edit_menubar;

int column_highlighting = 0;
int option_edit_save_pos = 1;

static int edit_callback (Dlg_head *h, WEdit *edit, int msg, int par);

static int
edit_event (WEdit * edit, Gpm_Event * event, int *result)
{
    *result = MOU_NORMAL;
    edit_update_curs_row (edit);
    edit_update_curs_col (edit);

    // Unknown event type
    if (!(event->type & (GPM_DOWN | GPM_DRAG | GPM_UP)))
	return 0;

    // Wheel events 
    if ((event->buttons & GPM_B_UP) && (event->type & GPM_DOWN)) {
	edit_move_up (edit, 2, 1);
	goto update;
    }
    if ((event->buttons & GPM_B_DOWN) && (event->type & GPM_DOWN)) {
	edit_move_down (edit, 2, 1);
	goto update;
    }

    // Outside editor window 
    if (event->y <= 1 || event->x <= 0
	|| event->x > edit->num_widget_columns
	|| event->y > edit->num_widget_lines + 1)
	return 0;

    //A lone up mustn't do anything 
    if (edit->mark2 != -1 && event->type & (GPM_UP | GPM_DRAG))
	return 1;

    if (event->type & (GPM_DOWN | GPM_UP))
	edit_push_key_press (edit);

    edit_cursor_move (edit, edit_bol (edit, edit->curs1) - edit->curs1);

    if (--event->y > (edit->curs_row + 1))
	edit_cursor_move (edit,
			  edit_move_forward (edit, edit->curs1,
					     event->y - (edit->curs_row +
							 1), 0)
			  - edit->curs1);

    if (event->y < (edit->curs_row + 1))
	edit_cursor_move (edit,
			  edit_move_backward (edit, edit->curs1,
					      (edit->curs_row + 1) -
					      event->y) - edit->curs1);

    edit_cursor_move (edit,
		      (int) edit_move_forward3 (edit, edit->curs1,
						event->x -
						edit->start_col - 1,
						0) - edit->curs1);

    edit->prev_col = edit_get_col (edit);

    if (event->type & GPM_DOWN) {
	edit_mark_cmd (edit, 1);	//reset 
	edit->highlight = 0;
    }

    if (!(event->type & GPM_DRAG))
	edit_mark_cmd (edit, 0);

  update:
    edit->force |= REDRAW_COMPLETELY;
    edit_update_curs_row (edit);
    edit_update_curs_col (edit);
    edit_update_screen (edit);

    return 1;
}

int menubar_event (Gpm_Event * event, WMenu * menubar);		/* menu.c */

static int
edit_mouse_event (Gpm_Event *event, void *x)
{
    int result;
    if (edit_event ((WEdit *) x, event, &result))
	return result;
    else
	return menubar_event (event, edit_menubar);

}

void
edit_adjust_size (Dlg_head *h)
{
    WEdit *edit;
    WButtonBar *edit_bar;

    edit = (WEdit *) find_widget_type (h, (callback_fn) edit_callback);
    edit_bar = (WButtonBar *) edit->widget.parent->current->next->widget;
    widget_set_size (&edit->widget, 0, 0, LINES - 1, COLS);
    widget_set_size (&edit_bar->widget, LINES - 1, 0, 1, COLS);
    widget_set_size (&edit_menubar->widget, 0, 0, 1, COLS);

#ifdef RESIZABLE_MENUBAR
    menubar_arrange (edit_menubar);
#endif


}

/* Callback for the edit dialog */
static int
edit_dialog_callback (Dlg_head * h, int id, int msg)
{
    return default_dlg_callback (h, id, msg);
}

extern Menu EditMenuBar[5];

#ifdef HAVE_DLGSWITCH
int finish_edit(
    Dlg_head *_edit_dlg,
    WEdit *_wedit, WButtonBar *_edit_bar, WMenu *_edit_menubar )
{
    int framed = 0; /* ??? */

    edit_dlg	= _edit_dlg;
    wedit	= _wedit;
    edit_bar	= _edit_bar;
    edit_menubar = _edit_menubar;

    if (!framed)
	edit_done_menu ();	/* editmenu.c */

    destroy_dlg (edit_dlg);

    dlglist_remove_current_dialog();
}

void run_editor(
    Dlg_head *_edit_dlg,
    WEdit *_wedit, WButtonBar *_edit_bar, WMenu *_edit_menubar )
{
    edit_dlg	= _edit_dlg;
    wedit	= _wedit;
    edit_bar	= _edit_bar;
    edit_menubar = _edit_menubar;

    run_dlg( edit_dlg );

    if (edit_dlg->soft_exit)
	return;

    finish_edit( _edit_dlg, _wedit, _edit_bar, _edit_menubar );
}
#endif

int
edit (const char *_file, int line)
{
    static int made_directory = 0;
    int framed = 0;

#ifdef HAVE_DLGSWITCH
    int absolute = 0, construct_file = 0;
    char *editfile = NULL;
#endif
    
    Dlg_head *edit_dlg;
    WButtonBar *edit_bar;

    if (option_backup_ext_int != -1) {
	option_backup_ext = malloc (sizeof (int) + 1);
	option_backup_ext[sizeof (int)] = '\0';
	memcpy (option_backup_ext, (char *) &option_backup_ext_int,
		sizeof (int));
    }
    if (!made_directory) {
	mkdir (catstrs (home_dir, EDIT_DIR, 0), 0700);
	made_directory = 1;
    }

#ifndef HAVE_DLGSWITCH
    if (!(wedit = edit_init (NULL, LINES - 2, COLS, _file, line))) {
	return 0;
    }
#else /* HAVE_DLGSWITCH */
    /* Directory should be used only on local filesystems,
       because we can switch to another directory before saving the file */
    construct_file = 0;
    if (_file && *(_file)) {
	absolute = _file[0] == PATH_SEP; /* Very simple check */
	construct_file = !absolute;
    }
    if (construct_file) {
	editfile = concat_dir_and_file( cpanel->cwd, _file );
    } else {
	editfile = (char*) _file;
    }

    wedit = edit_init (NULL, LINES - 2, COLS, editfile, line);

    if (construct_file) free( editfile );

    if (!(wedit)) { 
 	return 0;
    }
#endif

    /* Create a new dialog and add it widgets to it */
    edit_dlg = create_dlg (0, 0, LINES, COLS, NULL, edit_dialog_callback,
		    "[Internal File Editor]", NULL, DLG_WANT_TAB);
    init_widget (&(wedit->widget), 0, 0, LINES - 1, COLS,
		 (callback_fn) edit_callback, (destroy_fn) edit_clean,
		 (mouse_h) edit_mouse_event, 0);

    widget_want_cursor (wedit->widget, 1);
    edit_dlg->raw = 1;		/*so that tab = '\t' key works */

    edit_bar = buttonbar_new (1);

    if (!framed) {
	switch (edit_key_emulation) {
	case EDIT_KEY_EMULATION_NORMAL:
	    edit_init_menu_normal ();	/* editmenu.c */
	    break;
	case EDIT_KEY_EMULATION_EMACS:
	    edit_init_menu_emacs ();	/* editmenu.c */
	    break;
	}
    if (!clock_in_editor) clock_cancel();
	edit_menubar = menubar_new (0, 0, COLS, EditMenuBar, N_menus);
    }
    add_widget (edit_dlg, wedit);

    if (!framed)
	add_widget (edit_dlg, edit_menubar);

    add_widget (edit_dlg, edit_bar);

#ifndef HAVE_DLGSWITCH
    run_dlg (edit_dlg);

    if (!framed)
	edit_done_menu ();	/* editmenu.c */

    destroy_dlg (edit_dlg);
#else
    dlglist_add_editor( edit_dlg, _file, wedit, edit_bar, edit_menubar );
    run_editor( edit_dlg, wedit, edit_bar, edit_menubar );
#endif

    return 1;
}

static void edit_my_define (Dlg_head * h, int idx, char *text,
			    void (*fn) (WEdit *), WEdit * edit)
{
    define_label_data (h, (Widget *) edit, idx, text, (buttonbarfn) fn, edit);
}


static void cmd_F1 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (1));
}

static void cmd_F2 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (2));
}

static void cmd_F3 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (3));
}

static void cmd_F4 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (4));
}

static void cmd_F5 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (5));
}

static void cmd_F6 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (6));
}

static void cmd_F7 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (7));
}

static void cmd_F8 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (8));
}

#if 0
static void cmd_F9 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (9));
}
#endif

static void cmd_F10 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (10));
}

static void
edit_labels (WEdit *edit)
{
    Dlg_head *h = edit->widget.parent;

    edit_my_define (h, 1, _("Help"), cmd_F1, edit);
    edit_my_define (h, 2, _("Save"), cmd_F2, edit);
    edit_my_define (h, 3, _("Mark"), cmd_F3, edit);
    edit_my_define (h, 4, _("Replac"), cmd_F4, edit);
    edit_my_define (h, 5, _("Copy"), cmd_F5, edit);
    edit_my_define (h, 6, _("Move"), cmd_F6, edit);
    edit_my_define (h, 7, _("Search"), cmd_F7, edit);
    edit_my_define (h, 8, _("Delete"), cmd_F8, edit);
    if (!edit->have_frame)
	edit_my_define (h, 9, _("PullDn"), edit_menu_cmd, edit);
    edit_my_define (h, 10, _("Quit"), cmd_F10, edit);

    redraw_labels (h, (Widget *) edit);
//    redraw_labels (h);
}

void edit_update_screen (WEdit * e)
{
    edit_scroll_screen_over_cursor (e);

    edit_update_curs_col (e);
    edit_status (e);

/* pop all events for this window for internal handling */

    if (!is_idle ()) {
	e->force |= REDRAW_PAGE;
	return;
    }
    if (e->force & REDRAW_COMPLETELY)
	e->force |= REDRAW_PAGE;
    edit_render_keypress (e);
}

static int edit_callback (Dlg_head *h, WEdit *e, int msg, int par)
{
    switch (msg) {
    case WIDGET_INIT:
	e->force |= REDRAW_COMPLETELY;
	edit_labels (e);
	break;
    case WIDGET_DRAW:
	e->force |= REDRAW_COMPLETELY;
	e->num_widget_lines = LINES - 2;
	e->num_widget_columns = COLS;
    case WIDGET_FOCUS:
	edit_update_screen (e);
	return 1;
    case WIDGET_KEY:{
	    int cmd, ch;
	    if (edit_drop_hotkey_menu (e, par))		/* first check alt-f, alt-e, alt-s, etc for drop menus */
		return 1;
	    if (!edit_translate_key (e, par, &cmd, &ch))
		return 0;
	    edit_execute_key_command (e, cmd, ch);
	    edit_update_screen (e);
	}
	return 1;
    case WIDGET_COMMAND:
	edit_execute_key_command (e, par, -1);
	edit_update_screen (e);
	return 1;
    case WIDGET_CURSOR:
	widget_move (&e->widget, e->curs_row + EDIT_TEXT_VERTICAL_OFFSET, e->curs_col + e->start_col);
	return 1;
    }
    return default_proc (h, msg, par);
}
