/* editor menu definitions and initialisation

   Copyright (C) 1996 the Free Software Foundation

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

#include "edit.h"
#include "src/tty.h"		/* KEY_F */
#include "src/menu.h"		/* menu_entry */
#include "src/key.h"		/* XCTRL */
#include "src/main.h"		/* drop_menus */
#include "src/learn.h"		/* learn_keys */
#include "src/wtools.h"		/* query_dialog() */
#include "src/dialog.h"

#include "edit-widget.h"
#include "editcmddef.h"

extern int edit_key_emulation;
extern WEdit *wedit;
extern WButtonBar *edit_bar;
extern Dlg_head *edit_dlg;
extern WMenu *edit_menubar;

#undef edit_message_dialog
#define edit_message_dialog(w,x,y,h,s) query_dialog (h, s, 0, 1, _("&OK"))

#define CFocus(x) 

static void menu_cmd (int i)
{
printf ("MSG:EditManu, Command\r\n");
    send_message (wedit->widget.parent, (Widget *) wedit, WIDGET_COMMAND, i);
}

static void menu_key (int i)
{
printf ("MSG:EditManu, Key\r\n");
    send_message (wedit->widget.parent, (Widget *) wedit, WIDGET_KEY, i);
}

static void
edit_about_cmd (void)
{
    edit_message_dialog (wedit->mainid, 20, 20, _(" About "),
		      _("\n"
		      "                Cooledit  v3.11.5\n"
		      "\n"
		      " Copyright (C) 1996-200 the Free Software Foundation\n"
		      "\n"
		      "       A user friendly text editor written\n"
		      "           for the Midnight Commander.\n")
	);
}

static void menu_mail_cmd (void)         { menu_cmd (CK_Mail); }
static void menu_load_cmd (void)         { menu_cmd (CK_Load); }
static void menu_new_cmd (void)          { menu_cmd (CK_New); }
static void menu_save_cmd (void)         { menu_cmd (CK_Save); }
static void menu_save_as_cmd (void)      { menu_cmd (CK_Save_As); }
static void menu_insert_file_cmd (void)  { menu_cmd (CK_Insert_File); }
static void menu_quit_cmd (void)         { menu_cmd (CK_Exit); }
static void menu_mark_cmd (void)         { menu_cmd (CK_Mark); }
static void menu_markcol_cmd (void)      { menu_cmd (CK_Column_Mark); }
static void menu_ins_cmd (void)          { menu_cmd (CK_Toggle_Insert); }
static void menu_copy_cmd (void)         { menu_cmd (CK_Copy); }
static void menu_move_cmd (void)         { menu_cmd (CK_Move); }
static void menu_delete_cmd (void)       { menu_cmd (CK_Remove); }
static void menu_cut_cmd (void)          { menu_cmd (CK_Save_Block); }
static void menu_search_cmd (void)       { menu_cmd (CK_Find); }
static void menu_search_again_cmd (void) { menu_cmd (CK_Find_Again); }
static void menu_replace_cmd (void)      { menu_cmd (CK_Replace); }
static void menu_begin_record_cmd (void) { menu_cmd (CK_Begin_Record_Macro); }
static void menu_end_record_cmd (void)   { menu_cmd (CK_End_Record_Macro); }
static void menu_exec_macro_cmd (void)   { menu_key (XCTRL ('a')); }
static void menu_exec_macro_delete_cmd (void) { menu_cmd (CK_Delete_Macro); }
static void menu_c_form_cmd (void)       { menu_key (KEY_F (19)); }
static void menu_ispell_cmd (void)       { menu_cmd (CK_Pipe_Block (1)); }
static void menu_sort_cmd (void)         { menu_cmd (CK_Sort); }
static void menu_date_cmd (void)         { menu_cmd (CK_Date); }
static void menu_undo_cmd (void)         { menu_cmd (CK_Undo); }
static void menu_beginning_cmd (void)    { menu_cmd (CK_Beginning_Of_Text); }
static void menu_end_cmd (void)          { menu_cmd (CK_End_Of_Text); }
static void menu_refresh_cmd (void)      { menu_cmd (CK_Refresh); }
static void menu_goto_line (void)        { menu_cmd (CK_Goto); }
static void menu_goto_bracket (void)     { menu_cmd (CK_Match_Bracket); }
static void menu_lit_cmd (void)          { menu_key (XCTRL ('q')); }
static void menu_format_paragraph (void) { menu_cmd (CK_Paragraph_Format);}
static void menu_options (void)          { edit_options_dialog (); }
static void menu_user_menu_cmd (void)    { menu_key (KEY_F (11));}
static void menu_toggle_bookmark_cmd (void)  { menu_cmd (CK_Toggle_Bookmark); }
static void menu_prev_bookmark_cmd (void)    { menu_cmd (CK_Prev_Bookmark); }
static void menu_next_bookmark_cmd (void)    { menu_cmd (CK_Next_Bookmark); }
static void menu_flush_bookmarks_cmd (void)  { menu_cmd (CK_Flush_Bookmarks); }
static void menu_copy_to_X_buf_cmd (void)    { menu_cmd (CK_XStore); }
static void menu_cut_to_X_buf_cmd (void)     { menu_cmd (CK_XCut); }
static void menu_paste_from_X_buf_cmd (void) { menu_cmd (CK_XPaste); }


static void
menu_escript_cmd (void)
{
    menu_cmd (CK_Escript);
}

static menu_entry FileMenu[] =
{
    {' ', N_("&Open file..."),         'O', menu_load_cmd},
    {' ', N_("&New              C-n"), 'N', menu_new_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Save              F2"), 'S', menu_save_cmd},
    {' ', N_("Save &as...       F12"), 'A', menu_save_as_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Insert file...   F15"), 'I', menu_insert_file_cmd},
    {' ', N_("Copy to &file...  C-f"), 'F', menu_cut_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&User menu...     F11"), 'U', menu_user_menu_cmd},
    {' ', "", ' ', 0},
    {' ', N_("A&bout...            "), 'B', edit_about_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Quit             F10"), 'Q', menu_quit_cmd}
 };

static menu_entry FileMenuEmacs[] =
{
    {' ', N_("&Open file..."),         'O', menu_load_cmd},
    {' ', N_("&New            C-x k"), 'N', menu_new_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Save              F2"), 'S', menu_save_cmd},
    {' ', N_("Save &as...       F12"), 'A', menu_save_as_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Insert file...   F15"), 'I', menu_insert_file_cmd},
    {' ', N_("Copy to &file...     "), 'F', menu_cut_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&User menu...     F11"), 'U', menu_user_menu_cmd},
    {' ', "", ' ', 0},
    {' ', N_("A&bout..."), 'B', edit_about_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Quit             F10"), 'Q', menu_quit_cmd}
};

static menu_entry EditMenu[] =
{
    {' ', N_("&Toggle Mark           F3"), 'T', menu_mark_cmd},
    {' ', N_("&Mark Columns       Sh-F3"), 'T', menu_markcol_cmd},
    {' ', "", ' ', 0},
    {' ', N_("Toggle &ins/overw     Ins"), 'I', menu_ins_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Copy                  F5"), 'C', menu_copy_cmd},
    {' ', N_("&Move                  F6"), 'M', menu_move_cmd},
    {' ', N_("&Delete                F8"), 'D', menu_delete_cmd},
    {' ', "", ' ', 0},
    {' ', N_("To&ggle Bookmark  C-M-Ins"), 'G', menu_toggle_bookmark_cmd},
    {' ', N_("&Prev. Bookmark    C-M-Up"), 'P', menu_prev_bookmark_cmd},
    {' ', N_("&Next Bookmark   C-M-Down"), 'N', menu_next_bookmark_cmd},
    {' ', N_("&Flush Bookmarks"),       'F', menu_flush_bookmarks_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Copy to clipboard  C-Ins"), 'O', menu_copy_to_X_buf_cmd},
    {' ', N_("Cut to clipboard  Sh-Del"), 'X', menu_cut_to_X_buf_cmd},
    {' ', N_("Paste from clipb. Sh-Ins"), 'V', menu_paste_from_X_buf_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Undo                 C-u"), 'U', menu_undo_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Beginning         C-PgUp"), 'B', menu_beginning_cmd},
    {' ', N_("&End               C-PgDn"), 'E', menu_end_cmd}
};

static menu_entry EditMenuEmacs[] =
{
    {' ', N_("&Toggle Mark           F3"), 'T', menu_mark_cmd},
    {' ', N_("&Mark Columns       Sh-F3"), 'T', menu_markcol_cmd},
    {' ', "", ' ', 0},
    {' ', N_("Toggle &ins/overw     Ins"), 'I', menu_ins_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Copy                  F5"), 'C', menu_copy_cmd},
    {' ', N_("&Move                  F6"), 'M', menu_move_cmd},
    {' ', N_("&Delete                F8"), 'D', menu_delete_cmd},
    {' ', "", ' ', 0},
    {' ', N_("To&ggle Bookmark  C-M-Ins"), 'G', menu_toggle_bookmark_cmd},
    {' ', N_("&Prev. Bookmark    C-M-Up"), 'P', menu_prev_bookmark_cmd},
    {' ', N_("&Next Bookmark   C-M-Down"), 'N', menu_next_bookmark_cmd},
    {' ', N_("&Flush Bookmarks"),       'F', menu_flush_bookmarks_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Copy to clipboard  C-Ins"), 'O', menu_copy_to_X_buf_cmd},
    {' ', N_("Cut to clipboard  Sh-Del"), 'X', menu_cut_to_X_buf_cmd},
    {' ', N_("Paste from clipb. Sh-Ins"), 'V', menu_paste_from_X_buf_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Undo                 C-u"), 'U', menu_undo_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Beginning         C-PgUp"), 'B', menu_beginning_cmd},
    {' ', N_("&End               C-PgDn"), 'E', menu_end_cmd}
};

static menu_entry SearReplMenu[] =
{
    {' ', N_("&Search...         F7"), 'S', menu_search_cmd},
    {' ', N_("Search &again     F17"), 'A', menu_search_again_cmd},
    {' ', N_("&Replace...        F4"), 'R', menu_replace_cmd}
};

static menu_entry SearReplMenuEmacs[] =
{
    {' ', N_("&Search...         F7"), 'S', menu_search_cmd},
    {' ', N_("Search &again     F17"), 'A', menu_search_again_cmd},
    {' ', N_("&Replace...        F4"), 'R', menu_replace_cmd}
};

static menu_entry CmdMenu[] =
{
    {' ', N_("&Go to line...            M-l"), 'G', menu_goto_line},
    {' ', N_("Go to matching &bracket   M-b"), 'B', menu_goto_bracket},
    {' ', "", ' ', 0},
    {' ', N_("Insert &literal...       C-q"), 'L', menu_lit_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Refresh screen          C-l"), 'R', menu_refresh_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Start record macro      C-r"), 'S', menu_begin_record_cmd},
    {' ', N_("&Finish record macro...  C-r"), 'F', menu_end_record_cmd},
    {' ', N_("&Execute macro...   C-a, KEY"), 'E', menu_exec_macro_cmd},
    {' ', N_("Delete macr&o...            "), 'O', menu_exec_macro_delete_cmd},
    {' ', "", ' ', 0},
    {' ', N_("Insert &date/time           "), 'D', menu_date_cmd},
    {' ', "", ' ', 0},
    {' ', N_("Format p&aragraph        M-p"), 'A', menu_format_paragraph},
    {' ', N_("'ispell' s&pell check    C-p"), 'P', menu_ispell_cmd},
    {' ', N_("Sor&t...                 M-t"), 'T', menu_sort_cmd},
    {' ', N_("E&xternal Formatter      F19"), 'C', menu_c_form_cmd},
    {' ', N_("Exte&rnal Scripting      M-y"), 'Y', menu_escript_cmd},
    {' ', N_("&Mail...                    "), 'M', menu_mail_cmd}
};

static menu_entry CmdMenuEmacs[] =
{
    {' ', N_("&Go to line...            M-l"), 'G', menu_goto_line},
    {' ', N_("Go to matching &bracket   M-b"), 'B', menu_goto_bracket},
    {' ', "", ' ', 0},
    {' ', N_("Insert &literal...       C-q"), 'L', menu_lit_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Refresh screen          C-l"), 'R', menu_refresh_cmd},
    {' ', "", ' ', 0},
    {' ', N_("&Start record macro      C-r"), 'S', menu_begin_record_cmd},
    {' ', N_("&Finish record macro...  C-r"), 'F', menu_end_record_cmd},
    {' ', N_("&Execute macro... C-x e, KEY"), 'E', menu_exec_macro_cmd},
    {' ', N_("Delete macr&o...            "), 'o', menu_exec_macro_delete_cmd},
    {' ', "", ' ', 0},
    {' ', N_("Insert &date/time           "), 'D', menu_date_cmd},
    {' ', "", ' ', 0},
    {' ', N_("Format p&aragraph        M-p"), 'a', menu_format_paragraph},
    {' ', N_("'ispell' s&pell check    M-$"), 'P', menu_ispell_cmd},
    {' ', N_("Sor&t...                 M-t"), 'T', menu_sort_cmd},
    {' ', N_("E&xternal Formatter      F19"), 'C', menu_c_form_cmd},
    {' ', N_("Exte&rnal Scripting      M-y"), 'Y', menu_escript_cmd},
    {' ', N_("&Mail...                    "), 'M', menu_mail_cmd}
};

static menu_entry OptMenu[] =
{
    {' ', N_("&General...  "), 'G', menu_options},
    {' ', N_("&Save mode..."), 'S', menu_save_mode_cmd},
#ifndef NATIVE_WIN32
    {' ', N_("learn &Keys..."), 'K', learn_keys}
#endif				/* !NATIVE_WIN32 */
};

static menu_entry OptMenuEmacs[] =
{
    {' ', N_("&General...  "), 'G', menu_options},
    {' ', N_("&Save mode..."), 'S', menu_save_mode_cmd},
#ifndef NATIVE_WIN32
    {' ', N_("learn &Keys..."), 'K', learn_keys}
#endif				/* !NATIVE_WIN32 */
};

#define menu_entries(x) sizeof(x)/sizeof(menu_entry)

//struct Menu *EditMenuBar[N_menus];
Menu EditMenuBar[N_menus];

#ifdef HAVE_DLGSWITCH
/* Hacks */
int menu_ref_count = 0;
#endif

void edit_init_menu_normal (void)
{
#ifdef HAVE_DLGSWITCH
    ++menu_ref_count;
    if (menu_ref_count > 1) return;
#endif
    EditMenuBar[0] = create_menu (_(" File "), FileMenu, menu_entries (FileMenu));
    EditMenuBar[1] = create_menu (_(" Edit "), EditMenu, menu_entries (EditMenu));
    EditMenuBar[2] = create_menu (_(" Sear/Repl "), SearReplMenu, menu_entries (SearReplMenu));
    EditMenuBar[3] = create_menu (_(" Command "), CmdMenu, menu_entries (CmdMenu));
    EditMenuBar[4] = create_menu (_(" Options "), OptMenu, menu_entries (OptMenu));
}

void edit_init_menu_emacs (void)
{
#ifdef HAVE_DLGSWITCH
    ++menu_ref_count;
    if (menu_ref_count > 1) return;
#endif
    EditMenuBar[0] = create_menu (_(" File "), FileMenuEmacs, menu_entries (FileMenuEmacs));
    EditMenuBar[1] = create_menu (_(" Edit "), EditMenuEmacs, menu_entries (EditMenuEmacs));
    EditMenuBar[2] = create_menu (_(" Sear/Repl "), SearReplMenuEmacs, menu_entries (SearReplMenuEmacs));
    EditMenuBar[3] = create_menu (_(" Command "), CmdMenuEmacs, menu_entries (CmdMenuEmacs));
    EditMenuBar[4] = create_menu (_(" Options "), OptMenuEmacs, menu_entries (OptMenuEmacs));
}

void edit_done_menu (void)
{
    int i;
#ifdef HAVE_DLGSWITCH
    --menu_ref_count;
    if (menu_ref_count > 0) return;
#endif
    for (i = 0; i < N_menus; i++)
	destroy_menu (EditMenuBar[i]);
}


static 
void
edit_drop_menu_cmd (WEdit * e, int which)
{
    if (edit_menubar->active)
	return;
    edit_menubar->active = 1;
    edit_menubar->dropped = drop_menus;
    if (which >= 0) {
	edit_menubar->selected = which;
	edit_menubar->previous_selection = which;
    } else {
	edit_menubar->previous_selection =
	    dlg_item_number (e->widget.parent);
    }
    dlg_select_widget (e->widget.parent, edit_menubar);
}


void edit_menu_cmd (WEdit * e)
{
    edit_drop_menu_cmd (e, -1);
}


int edit_drop_hotkey_menu (WEdit * e, int key)
{
    int m = 0;
    switch (key) {
    case ALT ('f'):
	if (edit_key_emulation == EDIT_KEY_EMULATION_EMACS)
	    return 0;
	m = 0;
	break;
    case ALT ('e'):
	m = 1;
	break;
    case ALT ('s'):
	m = 2;
	break;
    case ALT ('c'):
	m = 3;
	break;
    case ALT ('o'):
	m = 4;
	break;
    default:
	return 0;
    }

    edit_drop_menu_cmd (e, m);
    return 1;
}
