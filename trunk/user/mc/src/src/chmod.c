/* Chmod command -- for the Midnight Commander
   Copyright (C) 1994 Radek Doulik

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  
 */

#include <config.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>	/* For errno on SunOS systems	      */
/* Needed for the extern declarations of integer parameters */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif
#include "tty.h"
#include "mad.h"
#include "util.h"
#include "win.h"
#include "color.h"
#include "dlg.h"
#include "widget.h"
#include "dialog.h"	/* For do_refresh() */

#include "dir.h"
#include "panel.h"		/* Needed for the externs */
#include "file.h"
#include "main.h"
#include "chmod.h"
#include "achown.h"
#include "chown.h"
#include "wtools.h"		/* For common_dialog_repaint */
#include "../vfs/vfs.h"

static int single_set;
struct Dlg_head *ch_dlg;

#define PX		5
#define PY		2

#define FX		40
#define FY		2

#define BX		6
#define BY		17

#define TX		40
#define TY		12

#define PERMISSIONS	12
#define BUTTONS		6

#define B_MARKED	B_USER
#define B_ALL		B_USER+1
#define B_SETMRK        B_USER+2
#define B_CLRMRK        B_USER+3

int mode_change, need_update;
int c_file, end_chmod;

umode_t and_mask, or_mask, c_stat;
char *c_fname, *c_fown, *c_fgrp, *c_fperm;
int c_fsize;

static WLabel *statl;

struct {
    mode_t mode;
    char *text;
    int selected;
    WCheck *check;
} check_perm[PERMISSIONS] = 
{
    { S_IXOTH, N_("execute/search by others"), 0, 0, },
    { S_IWOTH, N_("write by others"), 0, 0, },
    { S_IROTH, N_("read by others"), 0, 0, },
    { S_IXGRP, N_("execute/search by group"), 0, 0, },
    { S_IWGRP, N_("write by group"), 0, 0, },
    { S_IRGRP, N_("read by group"), 0, 0, },
    { S_IXUSR, N_("execute/search by owner"), 0, 0, },
    { S_IWUSR, N_("write by owner"), 0, 0, },
    { S_IRUSR, N_("read by owner"), 0, 0, },
    { S_ISVTX, N_("sticky bit"), 0, 0, },
    { S_ISGID, N_("set group ID on execution"), 0, 0, },
    { S_ISUID, N_("set user ID on execution"), 0, 0, },
};

struct {
    int ret_cmd, flags, y, x;
    char *text;
} chmod_but[BUTTONS] = 
{
    { B_CANCEL, NORMAL_BUTTON,  2, 33, N_("&Cancel") },
    { B_ENTER,  DEFPUSH_BUTTON, 2, 17, N_("&Set") },
    { B_CLRMRK, NORMAL_BUTTON,  0, 42, N_("C&lear marked") },
    { B_SETMRK, NORMAL_BUTTON,  0, 27, N_("S&et marked") },
    { B_MARKED, NORMAL_BUTTON,  0, 12, N_("&Marked all") },
    { B_ALL,    NORMAL_BUTTON,  0, 0,  N_("Set &all") },
};

static void chmod_toggle_select (Dlg_head *h)
{
    int Id = h->current->dlg_id - BUTTONS + single_set * 2;

    attrset (COLOR_NORMAL);
    check_perm[Id].selected ^= 1;

    dlg_move (h, PY + PERMISSIONS - Id, PX + 1);
    addch ((check_perm[Id].selected) ? '*' : ' ');
    dlg_move (h, PY + PERMISSIONS - Id, PX + 3);

}

static void chmod_refresh (Dlg_head *h)
{
    common_dialog_repaint (h);

    attrset (COLOR_NORMAL);

    draw_box (h, PY, PX, PERMISSIONS + 2, 33);
    draw_box (h, FY, FX, 10, 25);

    dlg_move (h, FY + 1, FX + 2);
    addstr (_("Name"));
    dlg_move (h, FY + 3, FX + 2);
    addstr (_("Permissions (Octal)"));
    dlg_move (h, FY + 5, FX + 2);
    addstr (_("Owner name"));
    dlg_move (h, FY + 7, FX + 2);
    addstr (_("Group name"));

    dlg_move (h, TY, TX);
    addstr (_("Use SPACE to change"));
    dlg_move (h, TY + 1, TX);
    addstr (_("an option, ARROW KEYS"));
    dlg_move (h, TY + 2, TX);
    addstr (_("to move between options"));
    dlg_move (h, TY + 3, TX);
    addstr (_("and T or INS to mark"));

    attrset (COLOR_HOT_NORMAL);

    dlg_move (h, PY, PX + 1);
    addstr (_(" Permission "));
    dlg_move (h, FY, FX + 1);
    addstr (_(" File "));
}

static int chmod_callback (Dlg_head *h, int Par, int Msg)
{
    char buffer [10];
    
    switch (Msg) {
    case DLG_ACTION:
	if (Par >= BUTTONS - single_set * 2) {
	    c_stat ^= check_perm[Par - BUTTONS + single_set * 2].mode;
	    sprintf (buffer, "%o", c_stat);
	    label_set_text (statl, buffer);
	    chmod_toggle_select (h);
	    mode_change = 1;
	}
	break;

    case DLG_KEY:
	if ((Par == 'T' || Par == 't' || Par == KEY_IC) &&
	    h->current->dlg_id >= BUTTONS - single_set * 2) {
	    chmod_toggle_select (h);
	    if (Par == KEY_IC)
		dlg_one_down (h);
	    return 1;
	}
	break;
    case DLG_DRAW:
	chmod_refresh (h);
	break;
    }
    return 0;
}

static Dlg_head *
init_chmod (void)
{
    int i;
    Dlg_head *ch_dlg;

    do_refresh ();
    end_chmod = c_file = need_update = 0;
    single_set = (cpanel->marked < 2) ? 2 : 0;

    ch_dlg = create_dlg (0, 0, 22 - single_set, 70, dialog_colors,
		    chmod_callback, "[Chmod]", _("Chmod command"), DLG_CENTER);
			 
    x_set_dialog_title (ch_dlg, _("Chmod command"));

    for (i = 0; i < BUTTONS; i++) {
	if (i == 2 && single_set)
	    break;
	else

	    add_widget (ch_dlg,
			button_new (BY + chmod_but[i].y - single_set,
				    BX + chmod_but[i].x,
				    chmod_but[i].ret_cmd,
				    chmod_but[i].flags,
				    _(chmod_but[i].text), 0, 0, NULL));
    }

    for (i = 0; i < PERMISSIONS; i++) {
	check_perm[i].check =
	    check_new (PY + (PERMISSIONS - i), PX + 2, 0,
		       _(check_perm[i].text), NULL);
	add_widget (ch_dlg, check_perm[i].check);
    }
    return ch_dlg;
}

int stat_file (char *filename, struct stat *st)
{
    if (mc_stat (filename, st))
	return 0;
/*
    if (!(S_ISREG(st->st_mode) || S_ISDIR(st->st_mode) ||
	  S_ISLNK(st->st_mode)))
	return 0;
*/
    return 1;
}

static void chmod_done (void)
{
    if (need_update)
	update_panels (UP_OPTIMIZE, UP_KEEPSEL);
    repaint_screen ();
}

static char *next_file (void)
{
    while (!cpanel->dir.list[c_file].f.marked)
	c_file++;

    return cpanel->dir.list[c_file].fname;
}

static void do_chmod (struct stat *sf)
{
    sf->st_mode &= and_mask;
    sf->st_mode |= or_mask;
    if (mc_chmod (cpanel->dir.list [c_file].fname, sf->st_mode) == -1)
	message (1, MSG_ERROR, _(" Couldn't chmod \"%s\" \n %s "),
	     cpanel->dir.list [c_file].fname, unix_error_string (errno));

    do_file_mark (cpanel, c_file, 0);
}

static void apply_mask (struct stat *sf)
{
    char *fname;

    need_update = end_chmod = 1;
    do_chmod (sf);

    do {
	fname = next_file ();
	if (!stat_file (fname, sf))
	    return;
	c_stat = sf->st_mode;

	do_chmod (sf);
    } while (cpanel->marked);
}

void chmod_cmd (void)
{
    char buffer [10];
    char *fname;
    int i;
    struct stat sf_stat;
    Dlg_head *ch_dlg;
/*
    if (!vfs_current_is_local ()) {
	if (vfs_current_is_extfs ()) {
	    message (1, _(" Oops... "),
		     _(" I can't run the Chmod command on an extfs "));
	    return;
	} else if (vfs_current_is_tarfs ()) {
	    message (1, _(" Oops... "),
		     (" I can't run the Chmod command on a tarfs "));
	    return;
	}
    }
*/
    do {			/* do while any files remaining */
	ch_dlg = init_chmod ();
	if (cpanel->marked)
	    fname = next_file ();	/* next marked file */
	else
	    fname = selection (cpanel)->fname;	/* single file */

	if (!stat_file (fname, &sf_stat)) {	/* get status of file */
	    destroy_dlg (ch_dlg);
	    break;
	}
	
	c_stat = sf_stat.st_mode;
	mode_change = 0;	/* clear changes flag */

	/* set check buttons */
	for (i = 0; i < PERMISSIONS; i++) {
	    check_perm[i].check->state = (c_stat & check_perm[i].mode) ? 1 : 0;
	    check_perm[i].selected = 0;
	}

	/* Set the labels */
	c_fname = name_trunc (fname, 21);
	add_widget (ch_dlg, label_new (FY+2, FX+2, c_fname, NULL));
	c_fown = name_trunc (get_owner (sf_stat.st_uid), 21);
	add_widget (ch_dlg, label_new (FY+6, FX+2, c_fown, NULL));
	c_fgrp = name_trunc (get_group (sf_stat.st_gid), 21);
	add_widget (ch_dlg, label_new (FY+8, FX+2, c_fgrp, NULL));
	sprintf (buffer, "%o", c_stat);
	statl = label_new (FY+4, FX+2, buffer, NULL);
	add_widget (ch_dlg, statl);
	
	run_dlg (ch_dlg);	/* retrieve an action */
	
	/* do action */
	switch (ch_dlg->ret_value) {
	case B_ENTER:
	    if (mode_change)
		if (mc_chmod (fname, c_stat) == -1)
		    message (1, MSG_ERROR, _(" Couldn't chmod \"%s\" \n %s "),
	 		 fname, unix_error_string (errno));
	    need_update = 1;
	    break;
	    
	case B_CANCEL:
	    end_chmod = 1;
	    break;
	    
	case B_ALL:
	case B_MARKED:
	    and_mask = or_mask = 0;
	    and_mask = ~and_mask;

	    for (i = 0; i < PERMISSIONS; i++) {
		if (check_perm[i].selected || ch_dlg->ret_value == B_ALL) {
		    if (check_perm[i].check->state & C_BOOL)
			or_mask |= check_perm[i].mode;
		    else
			and_mask &= ~check_perm[i].mode;
		}
	    }

	    apply_mask (&sf_stat);
	    break;
	    
	case B_SETMRK:
	    and_mask = or_mask = 0;
	    and_mask = ~and_mask;

	    for (i = 0; i < PERMISSIONS; i++) {
		if (check_perm[i].selected)
		    or_mask |= check_perm[i].mode;
	    }

	    apply_mask (&sf_stat);
	    break;
	case B_CLRMRK:
	    and_mask = or_mask = 0;
	    and_mask = ~and_mask;

	    for (i = 0; i < PERMISSIONS; i++) {
		if (check_perm[i].selected)
		    and_mask &= ~check_perm[i].mode;
	    }

	    apply_mask (&sf_stat);
	    break;
	}

	if (cpanel->marked && ch_dlg->ret_value!=B_CANCEL) {
	    do_file_mark (cpanel, c_file, 0);
    	    need_update = 1;
	}
	destroy_dlg (ch_dlg);
    } while (cpanel->marked && !end_chmod);
    chmod_done ();
}
