/* editor options dialog box

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
#include "edit.h"
#include "src/dlg.h"
#include "src/widget.h"
#include "src/dialog.h"
#include "src/wtools.h"		/* for QuickWidgets */

#define OPT_DLG_H 17
#define OPT_DLG_W 72

#ifndef USE_INTERNAL_EDIT
#define USE_INTERNAL_EDIT 1
#endif

extern WEdit *wedit;


static char *key_emu_str[] =
{N_("Intuitive"), N_("Emacs"), NULL};

static char *wrap_str[] =
{N_("None"), N_("Dynamic paragraphing"), N_("Type writer wrap"), NULL};

static void
i18n_translate_array (char *array[])
{
    while (*array!=NULL) {
	*array = _(*array);
        array++;
    }
}

void
edit_options_dialog (void)
{
    char wrap_length[32], tab_spacing[32], *p, *q;
    int wrap_mode = 0;
    int old_syntax_hl;
    int tedit_key_emulation = edit_key_emulation;
    int toption_fill_tabs_with_spaces = option_fill_tabs_with_spaces;
    int toption_edit_save_pos = option_edit_save_pos;
    int tedit_confirm_save = edit_confirm_save;
    int tedit_syntax_highlighting = option_syntax_highlighting;
    int toption_return_does_auto_indent = option_return_does_auto_indent;
    int toption_backspace_through_tabs = option_backspace_through_tabs;
    int toption_fake_half_tabs = option_fake_half_tabs;
    static int i18n_flag = 0;

    QuickWidget quick_widgets[] = {
	/* 0 */
	{quick_button, 6, 10, OPT_DLG_H - 3, OPT_DLG_H, N_("&Cancel"), 0, B_CANCEL, 0,
	 0, XV_WLAY_DONTCARE, NULL},
	/* 1 */
	{quick_button, 2, 10, OPT_DLG_H - 3, OPT_DLG_H, N_("&OK"), 0,  B_ENTER, 0, 
	 0, XV_WLAY_DONTCARE, NULL},
	/* 2 */
	{quick_label, OPT_DLG_W / 2, OPT_DLG_W, OPT_DLG_H - 5, OPT_DLG_H, N_("Word wrap line length: "), 0, 0, 
	 0, 0, XV_WLAY_DONTCARE, NULL},
	/* 3 */
	{quick_input, OPT_DLG_W / 2 + 24, OPT_DLG_W, OPT_DLG_H - 5, OPT_DLG_H, "", OPT_DLG_W / 2 - 4 - 24, 0, 
	 0, 0, XV_WLAY_DONTCARE, "edit-word-wrap"},
	/* 4 */
	{quick_label, OPT_DLG_W / 2, OPT_DLG_W, OPT_DLG_H - 6, OPT_DLG_H, N_("Tab spacing: "), 0, 0, 
	 0, 0, XV_WLAY_DONTCARE, NULL},
	/* 5 */
	{quick_input, OPT_DLG_W / 2 + 24, OPT_DLG_W, OPT_DLG_H - 6, OPT_DLG_H, "", OPT_DLG_W / 2 - 4 - 24, 0, 
	 0, 0, XV_WLAY_DONTCARE, "edit-tab-spacing"},
	/* 6 */
	{quick_checkbox, OPT_DLG_W / 2 + 1, OPT_DLG_W, OPT_DLG_H - 8, OPT_DLG_H, N_("Synta&x highlighting"), 8, 0, 
	 0, 0, XV_WLAY_DONTCARE, NULL},
//	0, 0, NULL},
	/* 7 */
	{quick_checkbox, OPT_DLG_W / 2 + 1, OPT_DLG_W, OPT_DLG_H - 9, OPT_DLG_H, N_("Save file &position"), 0, 0, 
        0, 0, XV_WLAY_DONTCARE, NULL},
	/* 8 */
	{quick_checkbox, OPT_DLG_W / 2 + 1, OPT_DLG_W, OPT_DLG_H - 10, OPT_DLG_H, N_("Confir&m before saving"), 6, 0, 
	 0, 0, XV_WLAY_DONTCARE, NULL},
//	0, 0, NULL},
	/* 9 */
	{quick_checkbox, OPT_DLG_W / 2 + 1, OPT_DLG_W, OPT_DLG_H - 11, OPT_DLG_H, N_("Fill tabs with &spaces"), 0, 0, 
	 0, 0, XV_WLAY_DONTCARE, NULL},
//	0, 0, NULL},
	/* 10 */
	{quick_checkbox, OPT_DLG_W / 2 + 1, OPT_DLG_W, OPT_DLG_H - 12, OPT_DLG_H, N_("&Return does autoindent"), 0, 0, 
	 0, 0, XV_WLAY_DONTCARE, NULL},
//	0, 0, NULL},
	/* 11 */
	{quick_checkbox, OPT_DLG_W / 2 + 1, OPT_DLG_W, OPT_DLG_H - 13, OPT_DLG_H, N_("&Backspace through tabs"), 0, 0, 
	 0, 0, XV_WLAY_DONTCARE, NULL},
//	0, 0, NULL},
	/* 12 */
	{quick_checkbox, OPT_DLG_W / 2 + 1, OPT_DLG_W, OPT_DLG_H - 14, OPT_DLG_H, N_("&Fake half tabs"), 0, 0, 
	 0, 0, XV_WLAY_DONTCARE, NULL},
//	0, 0, NULL},
	/* 13 */
	{quick_radio, 5, OPT_DLG_W, OPT_DLG_H - 7, OPT_DLG_H, "", 3, 0, 
	 0, wrap_str, XV_WLAY_DONTCARE, "wrapm"},
//	0, wrap_str, "wrapm"},
	/* 14 */
	{quick_label, 4, OPT_DLG_W, OPT_DLG_H - 8, OPT_DLG_H, N_("Wrap mode"), 0, 0,
	 0, 0, XV_WLAY_DONTCARE, NULL},
//	 0, 0, NULL},
	/* 15 */
	{quick_radio, 5, OPT_DLG_W, OPT_DLG_H - 13, OPT_DLG_H, "", 2, 0, 
	0, key_emu_str, XV_WLAY_DONTCARE, "keyemu"},
//	0, key_emu_str, "keyemu"},
	/* 16 */
	{quick_label, 4, OPT_DLG_W, OPT_DLG_H - 14, OPT_DLG_H, N_("Key emulation"), 0, 0, 
	 0, 0, XV_WLAY_DONTCARE, NULL},
//	0, 0, NULL},
	{0}
    };

    QuickDialog Quick_options =
	{ OPT_DLG_W, OPT_DLG_H, -1, 0, N_(" Editor options "), "", 0 };

    if (!i18n_flag) {
	i18n_translate_array (key_emu_str);
	i18n_translate_array (wrap_str);
	i18n_flag = 1;
    }

    sprintf (wrap_length, "%d",	option_word_wrap_line_length);
    sprintf (tab_spacing, "%d",	option_tab_spacing);

    quick_widgets[3].text = wrap_length;
    quick_widgets[3].str_result = &p;
    quick_widgets[5].text = tab_spacing;
    quick_widgets[5].str_result = &q;
    quick_widgets[6].result = &tedit_syntax_highlighting;
    quick_widgets[7].result = &toption_edit_save_pos;
    quick_widgets[8].result = &tedit_confirm_save;
    quick_widgets[9].result = &toption_fill_tabs_with_spaces;
    quick_widgets[10].result = &toption_return_does_auto_indent;
    quick_widgets[11].result = &toption_backspace_through_tabs;
    quick_widgets[12].result = &toption_fake_half_tabs;

    if (option_auto_para_formatting)
	wrap_mode = 1;
    else if (option_typewriter_wrap)
	wrap_mode = 2;
    else
	wrap_mode = 0;

    quick_widgets[13].result = &wrap_mode;
    quick_widgets[13].value = wrap_mode;

    quick_widgets[15].result = &tedit_key_emulation;
    quick_widgets[15].value = tedit_key_emulation;

    Quick_options.widgets = quick_widgets;

    if (quick_dialog (&Quick_options) == B_CANCEL)
	return;

    old_syntax_hl = option_syntax_highlighting;

    if (p) {
	option_word_wrap_line_length = atoi (p);
	free (p);
    }
    if (q) {
	option_tab_spacing = atoi (q);
	if (option_tab_spacing <= 0)
	    option_tab_spacing = 8;
	free (q);
    }

    option_syntax_highlighting = tedit_syntax_highlighting;
    edit_confirm_save = tedit_confirm_save;
    option_edit_save_pos = toption_edit_save_pos;
    option_fill_tabs_with_spaces = toption_fill_tabs_with_spaces;
    option_return_does_auto_indent = toption_return_does_auto_indent;
    option_backspace_through_tabs = toption_backspace_through_tabs;
    option_fake_half_tabs = toption_fake_half_tabs;

    if (wrap_mode == 1) {
	option_auto_para_formatting = 1;
	option_typewriter_wrap = 0;
    } else if (wrap_mode == 2) {
	option_auto_para_formatting = 0;
	option_typewriter_wrap = 1;
    } else {
	option_auto_para_formatting = 0;
	option_typewriter_wrap = 0;
    }

    edit_key_emulation = tedit_key_emulation;

    /* Load or unload syntax rules if the option has changed */
    if (option_syntax_highlighting != old_syntax_hl)
	edit_load_syntax (wedit, 0, 0);
}
