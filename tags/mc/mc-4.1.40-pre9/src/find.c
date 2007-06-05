/* Find file command for the Midnight Commander
   Copyright (C) The Free Software Foundation
   Written 1995 by Miguel de Icaza

   Complete rewrote.
   
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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <config.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#include "mad.h"

#include "tree.h"
#include "main.h"
#include "global.h"
#include "tty.h"
#include "win.h"
#include "color.h"
#include "setup.h"
#include "find.h"

/* Dialog manager and widgets */
#include "dlg.h"
#include "widget.h"

#include "dialog.h"     /* For do_refresh() */
#define  DIR_H_INCLUDE_HANDLE_DIRENT
#include "dir.h"
#include "panel.h"		/* current_panel */
#include "main.h"		/* do_cd, try_to_select */
#include "wtools.h"
#include "cmd.h"		/* view_file_at_line */
#include "boxes.h"
#include "../vfs/vfs.h"
#include "regex.h"

extern int verbose;

/* Size of the find parameters window */
#define FIND_Y 16
static int FIND_X = 50;

/* Size of the find window */
#define FIND2_Y LINES-4

static int FIND2_X = 64;
#define FIND2_X_USE FIND2_X-20

/* A couple of extra messages we need */
enum {
    B_STOP = B_USER + 1,
    B_AGAIN,
    B_PANELIZE,
    B_TREE,
    B_VIEW
};

/* List of directories to be ignored, separated by ':' */
char *find_ignore_dirs = 0;

static WInput *in_start;	/* Start path */
static WInput *in_name;		/* Pattern to search */
static WInput *in_with;		/* Text inside filename */
static WCheck *case_sense;	/* "case sensitive" checkbox */
static WCheck *recurse;		/* "Recursive" checkbox */
static WCheck *first_entry;	/* "First Entry Only" checkbox */

static int running = 0;		/* nice flag */
static char *find_pattern;	/* Pattern to search */
static char *content_pattern;	/* pattern to search inside files */
static int count;		/* Number of files displayed */
static int matches;		/* Number of matches */
static int is_start;		/* Status of the start/stop toggle button */
static char *old_dir;

static Dlg_head *find_dlg;	/* The dialog */

static WButton *stop_button;	/* pointer to the stop button */
static WLabel *status_label;	/* Finished, Searching etc. */
static WListbox *find_list;	/* Listbox with the file list */

/* This keeps track of the directory stack */
typedef struct dir_stack {
    char *name;
    struct dir_stack *prev;
} dir_stack ;

static dir_stack *dir_stack_base = 0;

static struct {
	char* text;
	int len;	/* length including space and brackets */
	int x;
} fbuts [] = {
	{ N_("&Suspend"),   11, 29 },
	{ N_("Con&tinue"),  12, 29 },
	{ N_("&Chdir"),     11, 3  },
	{ N_("&Again"),     9,  17 },
	{ N_("&Quit"),      8,  43 },
	{ N_("Pane&lize"),  12, 3  },
	{ N_("&View - F3"), 13, 20 },
	{ N_("&Edit - F4"), 13, 38 }
};

static char *add_to_list (char *text, void *closure);
static void stop_idle (void *data);
static void status_update (char *text);
static void get_list_info (char **file, char **dir);

/* FIXME: r should be local variables */
static regex_t *r; /* Pointer to compiled content_pattern */
 
static int case_sensitive = 1;
static int find_recursively = 1;
static int find_first_entry = 0;

#define TREE_Y 20
#define TREE_X 60

static int
tree_callback (struct Dlg_head *h, int id, int msg)
{
    switch (msg){

    case DLG_POST_KEY:
	/* The enter key will be processed by the tree widget */
	if (id == '\n') {
	    h->ret_value = B_ENTER;
	    dlg_stop (h);
	}
	return MSG_HANDLED;
	
    case DLG_DRAW:
	common_dialog_repaint (h);
	break;
    }
    return MSG_NOT_HANDLED;
}

/* Show tree in a box, not on a panel */
static char *
tree_search_box (char *current_dir)
{
    WTree    *mytree;
    Dlg_head *dlg;
    char     *val;
    WButtonBar *bar;
    int tree_colors[4];

    tree_colors [0] = dialog_colors [0];
    tree_colors [1] = dialog_colors [3];
    tree_colors [2] = dialog_colors [2];
    tree_colors [3] = dialog_colors [0];

    /* Create the components */
    dlg = create_dlg (0, 0, TREE_Y, TREE_X, tree_colors,
		      tree_callback, "[Directory Tree]", NULL, DLG_CENTER);
    x_set_dialog_title (dlg, _("Directory tree"));
    mytree = tree_new (0, 2, 2, TREE_Y - 6, TREE_X - 5);
    add_widget (dlg, mytree);
    bar = buttonbar_new(1);
    add_widget (dlg, bar);
    bar->widget.x = 0;
    bar->widget.y = LINES - 1;
    
    run_dlg (dlg);
    if (dlg->ret_value == B_ENTER)
	val = (char *) strdup (mytree->selected_ptr->name);
    else
	val = 0;
    
    destroy_dlg (dlg);
    return val;
}


/*
 * Callback for the parameter dialog.
 * Validate regex, prevent closing the dialog if it's invalid.
 */
static int
find_parm_callback (struct Dlg_head *h, int id, int Msg)
{
    int flags;

    switch (Msg) {
    case DLG_VALIDATE:
	if ((h->ret_value != B_ENTER) || !in_with->buffer[0])
	    return MSG_HANDLED;

	flags = REG_EXTENDED | REG_NOSUB;

	if (!(case_sense->state & C_BOOL))
	    flags |= REG_ICASE;

	if (regcomp (r, in_with->buffer, flags)) {
	    message (1, MSG_ERROR, _("  Malformed regular expression  "));
	    dlg_select_widget (h, in_with);
	    h->running = 1;	/* Don't stop the dialog */
	}
	return MSG_HANDLED;
    }
    return default_dlg_callback (h, id, Msg);
}

/*
 * find_parameters: gets information from the user
 *
 * If the return value is true, then the following holds:
 *
 * START_DIR and PATTERN are pointers to char * and upon return they
 * contain the information provided by the user.
 *
 * CONTENT holds a strdup of the contents specified by the user if he
 * asked for them or 0 if not (note, this is different from the
 * behavior for the other two parameters.
 *
 */
static int
find_parameters (char **start_dir, char **pattern, char **content)
{
    int return_value;
    char *temp_dir;
    static char *case_label = N_("case &Sensitive");
    static char *recurse_label = N_("find &Recursively");
    static char *find_first_entry_label = N_("find &First entry only");

    static char *in_contents = NULL;
    static char *in_start_dir = NULL;
    static char *in_start_name = NULL;

    static char *labs[] =
	{ N_("Start at:"), N_("Filename:"), N_("Content: ") };
    static char *buts[] = { N_("&OK"), N_("&Tree"), N_("&Cancel") };
    static int ilen = 30, istart = 14;
    static int b0 = 3, b1 = 16, b2 = 36;

#ifdef ENABLE_NLS
    static int i18n_flag = 0;

    if (!i18n_flag) {
	register int i = sizeof (labs) / sizeof (labs[0]);
	int l1, maxlen = 0;

	while (i--) {
	    l1 = strlen (labs[i] = _(labs[i]));
	    if (l1 > maxlen)
		maxlen = l1;
	}
	i = maxlen + ilen + 7;
	if (i > FIND_X)
	    FIND_X = i;

	for (i = sizeof (buts) / sizeof (buts[0]), l1 = 0; i--;) {
	    l1 += strlen (buts[i] = _(buts[i]));
	}
	l1 += 21;
	if (l1 > FIND_X)
	    FIND_X = l1;

	ilen = FIND_X - 7 - maxlen;	/* for the case of very long buttons :) */
	istart = FIND_X - 3 - ilen;

	b1 = b0 + strlen (buts[0]) + 7;
	b2 = FIND_X - (strlen (buts[2]) + 6);

	i18n_flag = 1;
	case_label = _(case_label);
    }
#endif				/* ENABLE_NLS */

  find_par_start:
    if (!in_start_dir)
	in_start_dir = (char *) strdup (".");
    if (!in_start_name)
	in_start_name = (char *) strdup (easy_patterns ? "*" : ".");
    if (!in_contents)
	in_contents = (char *) strdup ("");

    find_dlg =
	create_dlg (0, 0, FIND_Y, FIND_X, dialog_colors,
		    find_parm_callback, "[Find File]", _("Find File"),
		    DLG_CENTER);
	x_set_dialog_title (find_dlg, _("Find File"));

    add_widget (find_dlg,
		button_new (13, b2, B_CANCEL, NORMAL_BUTTON, buts[2], 0, 0,
			    "cancel"));
    add_widget (find_dlg,
		button_new (13, b1, B_TREE, NORMAL_BUTTON, buts[1], 0, 0,
			    "tree"));
    add_widget (find_dlg,
		button_new (13, b0, B_ENTER, DEFPUSH_BUTTON, buts[0], 0, 0,
			    "ok"));

    first_entry =
	check_new (11, 3, find_first_entry, find_first_entry_label, "find-first-entry-check");
    add_widget (find_dlg, first_entry);

    case_sense =
	check_new (10, 3, case_sensitive, case_label, "find-case-check");
    add_widget (find_dlg, case_sense);

    recurse =
	check_new (9, 3, find_recursively, recurse_label, "find-recurse-check");
    add_widget (find_dlg, recurse);


    in_with =
	input_new (7, istart, INPUT_COLOR_DEF, ilen, in_contents, "search-string");
    add_widget (find_dlg, in_with);

    in_name =
	input_new (5, istart, INPUT_COLOR_DEF, ilen, in_start_name, "name");
    add_widget (find_dlg, in_name);

    in_start =
	input_new (3, istart, INPUT_COLOR_DEF, ilen, in_start_dir, "start");
    add_widget (find_dlg, in_start);

    add_widget (find_dlg, label_new (7, 3, labs[2], "label-cont"));
    add_widget (find_dlg, label_new (5, 3, labs[1], "label-file"));
    add_widget (find_dlg, label_new (3, 3, labs[0], "label-start"));

    run_dlg (find_dlg);

    switch (find_dlg->ret_value) {
    case B_CANCEL:
	return_value = 0;
	break;

    case B_TREE:
	temp_dir = (char *) strdup (in_start->buffer);
	case_sensitive = case_sense->state & C_BOOL;
	find_recursively = recurse->state & C_BOOL;
	find_first_entry = first_entry->state & C_BOOL;
	destroy_dlg (find_dlg);
	free (in_start_dir);
	if (strcmp (temp_dir, ".") == 0) {
	    free (temp_dir);
	    temp_dir = (char *) strdup (cpanel->cwd);
	}
	in_start_dir = (char *) tree_search_box (temp_dir);
	if (in_start_dir)
	    free (temp_dir);
	else
	    in_start_dir = temp_dir;
	/* Warning: Dreadful goto */
	goto find_par_start;
	break;

    default:
	free (in_contents);
	if (in_with->buffer[0]) {
	    *content = (char *) strdup (in_with->buffer);
	    in_contents = (char *) strdup (*content);
	} else {
	    *content = in_contents = NULL;
	    r = 0;
	}

	case_sensitive = case_sense->state & C_BOOL;
	find_recursively = recurse->state & C_BOOL;
	find_first_entry = first_entry->state & C_BOOL;
	return_value = 1;
	*start_dir = (char *) strdup (in_start->buffer);
	*pattern = (char *) strdup (in_name->buffer);

	free (in_start_dir);
	in_start_dir = (char *) strdup (*start_dir);
	free (in_start_name);
	in_start_name = (char *) strdup (*pattern);
    }

    destroy_dlg (find_dlg);

    return return_value;
}

static void
push_directory (char *dir)
{
    dir_stack *new_;

    new_ = xmalloc (sizeof (dir_stack), "find: push_directory");
    new_->name = (char *) strdup (dir);
    new_->prev = dir_stack_base;
    dir_stack_base = new_;
}

static char*
pop_directory (void)
{
    char *name; 
    dir_stack *next;

    if (dir_stack_base){
	name = dir_stack_base->name;
	next = dir_stack_base->prev;
	free (dir_stack_base);
	dir_stack_base = next;
	return name;
    } else
	return 0;
}

static void
insert_file (char *dir, char *file)
{
    char *tmp_name;
    static char *dirname;
    int i;

    if (dir [0] == PATH_SEP && dir [1] == PATH_SEP)
	dir++;
    i = strlen (dir);
    if (i){
	if (dir [i - 1] != PATH_SEP){
	    dir [i] = PATH_SEP;
	    dir [i + 1] = 0;
	}
    }

    if (old_dir){
	if (strcmp (old_dir, dir)){
	   free (old_dir);
	    old_dir = (char *) strdup (dir);
	    dirname = add_to_list (dir, NULL);
	}
    } else {
	old_dir = (char *) strdup (dir);
	dirname = add_to_list (dir, NULL);
    }
    
    tmp_name = copy_strings ("    ", file, NULL);
    add_to_list (tmp_name, dirname);
    free (tmp_name);
}

static void
find_add_match (Dlg_head *h, char *dir, char *file)
{
    int p = ++matches & 7;

    insert_file (dir, file);

    /* Scroll nicely */
    if (!p)
	listbox_select_last (find_list, 1);
    else
	listbox_select_last (find_list, 0);
	/* Updates the current listing */
	send_message (h, &find_list->widget, WIDGET_DRAW, 0);
	if (p == 7)
	    mc_refresh ();
}

/*
 * get_line_at:
 *
 * Returns malloced null-terminated line from file file_fd.
 * Input is buffered in buf_size long buffer.
 * Current pos in buf is stored in pos.
 * n_read - number of read chars.
 * has_newline - is there newline ?
 */
static char *
get_line_at (int file_fd, char *buf, int *pos, int *n_read, int buf_size,
	     int *has_newline)
{
    char *buffer = 0;
    int buffer_size = 0;
    char ch = 0;
    int i = 0;

    for (;;) {
	if (*pos >= *n_read) {
	    *pos = 0;
	    if ((*n_read = mc_read (file_fd, buf, buf_size)) <= 0)
		break;
	}

	ch = buf[(*pos)++];
	if (ch == 0) {
	    /* skip possible leading zero(s) */
	    if (i == 0)
		continue;
	    else
		break;
	}

	if (i >= buffer_size - 1) {
	    buffer = (char *) realloc (buffer, buffer_size += 80);
	}
	/* Strip newline */
	if (ch == '\n')
	    break;

	buffer[i++] = ch;
    }

    *has_newline = ch ? 1 : 0;

    if (buffer) {
	buffer[i] = 0;
    }

    return buffer;
}

/* 
 * search_content:
 *
 * Search the global (FIXME) regexp compiled content_pattern string in the
 * DIRECTORY/FILE.  It will add the found entries to the find listbox.
 */
static void
search_content (Dlg_head *h, char *directory, char *filename)
{
    struct stat s;
    char buffer [128];
    char str_buf[128];
    char *fname;
    int file_fd;

    fname = concat_dir_and_file (directory, filename);

    if (mc_stat (fname, &s) != 0 || !S_ISREG (s.st_mode)){
	free (fname);
	return;
    }

    file_fd = mc_open (fname, O_RDONLY);
    free (fname);

    if (file_fd == -1)
	return;

    sprintf (buffer, _("Grepping in %s"), name_trunc (filename, FIND2_X_USE));

    status_update (buffer);
    mc_refresh ();

    enable_interrupt_key ();
    got_interrupt ();

    {
	int line = 1;
	int pos = 0;
	int n_read = 0;
	int has_newline;
	char *p;
	int found = 0;

   while (!found && (p = get_line_at (file_fd, buffer, &pos, &n_read, sizeof (buffer), &has_newline))){

		if (regexec (r, p, 1, 0, 0) == 0){
		    free (p);
		    sprintf (str_buf, "%d:%s", line, filename);
		    p = (char *) strdup (str_buf);
		    find_add_match (h, directory, p);
		    found = 1;
		}

	    if (has_newline){
		line++;
		if (!find_first_entry)
		    found = 0;
	    }

	    free (p);
	}
    }
    disable_interrupt_key ();
    mc_close (file_fd);
}

static int
do_search (struct Dlg_head *h)
{
    static struct dirent *dp   = 0;
    static DIR  *dirp = 0;
    static char directory [MC_MAXPATHLEN+2];
    struct stat tmp_stat;
    static int pos;
    static int subdirs_left = 0;
    char *tmp_name;		/* For building file names */

    if (!h) { /* someone forces me to close dirp */
	if (dirp) {
	    mc_closedir (dirp);
	    dirp = 0;
	}
        dp = 0;
	return 1;
    }
 do_search_begin:
    while (!dp){
	
	if (dirp){
	    mc_closedir (dirp);
	    dirp = 0;
	}
	
	while (!dirp){
	    char *tmp;

	    attrset (REVERSE_COLOR);
	    while (1) {
		tmp = pop_directory ();
		if (!tmp){
		    running = 0;
		    status_update (_("Finished"));
		    stop_idle (h);
		    return 0;
		}
		if (find_ignore_dirs){
                    int found;
		    char *temp_dir = copy_strings (":", tmp, ":", NULL);

                    found = strstr (find_ignore_dirs, temp_dir) != 0;
                    free (temp_dir);
		    if (found)
			free (tmp);
		    else
			break;
		} else
		    break;
	    } 
	    
	    strcpy (directory, tmp);
	    free (tmp);

	    if (verbose){
			char buffer [128];

		    sprintf (buffer, _("Searching %s"), name_trunc (directory, FIND2_X_USE));
		    status_update (buffer);
	    }
	    /* mc_stat should not be called after mc_opendir
	       because vfs_s_opendir modifies the st_nlink
	    */
	    mc_stat (directory, &tmp_stat);
	    subdirs_left = tmp_stat.st_nlink - 2;
	    /* Commented out as unnecessary
	       if (subdirs_left < 0)
	       subdirs_left = MAXINT;
	    */
	    dirp = mc_opendir (directory);
	}
	dp = mc_readdir (dirp);
    }

    if (strcmp (dp->d_name, ".") == 0 ||
	strcmp (dp->d_name, "..") == 0){
	dp = mc_readdir (dirp);
	return 1;
    }
    
    tmp_name = concat_dir_and_file (directory, dp->d_name);

    if (subdirs_left){
	mc_lstat (tmp_name, &tmp_stat);
	if (S_ISDIR (tmp_stat.st_mode)){
	    if (find_recursively)
		push_directory (tmp_name);
	    subdirs_left--;
	}
    }

    if (regexp_match (find_pattern, dp->d_name, match_file)){
	if (content_pattern)
	    search_content (h, directory, dp->d_name);
	else 
	    find_add_match (h, directory, dp->d_name);
    }
    
    free (tmp_name);
    dp = mc_readdir (dirp);

    /* Displays the nice dot */
    count++;
    if (!(count & 31)){
	/* For nice updating */
	char *rotating_dash = "|/-\\";

	if (verbose){
	    pos = (pos + 1) % 4;
	    attrset (NORMALC);
	    dlg_move (h, FIND2_Y-6, FIND2_X - 4);
	    addch (rotating_dash [pos]);
	    mc_refresh ();
	}
    } else
	goto do_search_begin;
    return 1;
}

static void
init_find_vars (void)
{
    char *dir;
    
    if (old_dir){
	free (old_dir);
	old_dir = 0;
    }
    count = 0;
    matches = 0;

    /* Remove all the items in the stack */
    while ((dir = pop_directory ()) != NULL)
	free (dir);
}

static void
find_do_view_edit (int unparsed_view, int edit, char *dir, char *file)
{
    char *fullname, *filename;
    int line;
    
    if (content_pattern){
	filename = strchr (file + 4, ':') + 1;
	line = atoi (file + 4);
    } else {
	 filename = file + 4;
	 line = 0;
    }
    if (dir [0] == '.' && dir [1] == 0)
	 fullname = (char *) strdup (filename);
    else if (dir [0] == '.' && dir [1] == PATH_SEP)
	 fullname = concat_dir_and_file (dir+2, filename);
    else
	 fullname = concat_dir_and_file (dir, filename);

    if (edit)
	do_edit_at_line (fullname, line);
    else
        view_file_at_line (fullname, unparsed_view, use_internal_view, line);
    free (fullname);
}

static void
get_list_info (char **file, char **dir)
{
    listbox_get_current (find_list, file, dir);
}

static char *
add_to_list (char *text, void *data)
{
	return listbox_add_item (find_list, 0, 0, text, data);
}

static void
stop_idle (void *data)
{
	set_idle_proc (data, 0);
}

static int
view_edit_currently_selected_file (int unparsed_view, int edit)
{
    WLEntry *entry = find_list->current;
    char *dir;

    if (!entry)
        return MSG_NOT_HANDLED;

    dir = entry->data;

    if (!entry->text || !dir)
	return MSG_NOT_HANDLED;

    find_do_view_edit (unparsed_view, edit, dir, entry->text);
    return MSG_HANDLED;
}

static int
find_callback (struct Dlg_head *h, int id, int Msg)
{
    switch (Msg){
    case DLG_DRAW:
        common_dialog_repaint (h);
	break;

    case DLG_KEY:
	if (id == KEY_F(3) || id == KEY_F(13)){
	    int unparsed_view = (id == KEY_F(13));
	    return view_edit_currently_selected_file (unparsed_view, 0);
	}
	if (id == KEY_F(4)){
	    return view_edit_currently_selected_file (0, 1);
	 }
	 return MSG_NOT_HANDLED;

     case DLG_IDLE:
	 do_search (h);
	 break;
     }
     return 0;
}

/* Handles the Stop/Start button in the find window */
static int
start_stop (int button, void *extra)
{
    running = is_start;
    set_idle_proc (find_dlg, running);
    is_start = !is_start;

    status_update (is_start ? _("Stopped") : _("Searching"));
    button_set_text (stop_button, fbuts [is_start].text);

    return 0;
}

/* Handle view command, when invoked as a button */
static int
find_do_view_file (int button, void *extra)
{
    view_edit_currently_selected_file (0, 0);
    return 0;
}

/* Handle edit command, when invoked as a button */
static int
find_do_edit_file (int button, void *extra)
{
    view_edit_currently_selected_file (0, 1);
    return 0;
}

static void
setup_gui (void)
{
#ifdef ENABLE_NLS
    static int i18n_flag = 0;
    if (!i18n_flag) {
	register int i = sizeof (fbuts) / sizeof (fbuts[0]);
	while (i--)
	    fbuts[i].len = strlen (fbuts[i].text = _(fbuts[i].text)) + 3;
	fbuts[2].len += 2;	/* DEFPUSH_BUTTON */
	i18n_flag = 1;
    }
#endif				/* ENABLE_NLS */

    /*
     * Dynamically place buttons centered within current window size
     */
    {
	int l0 = max (fbuts[0].len, fbuts[1].len);
	int l1 = fbuts[2].len + fbuts[3].len + l0 + fbuts[4].len;
	int l2 = fbuts[5].len + fbuts[6].len + fbuts[7].len;
	int r1, r2;

	FIND2_X = COLS - 16;

	/* Check, if both button rows fit within FIND2_X */
	if (l1 + 9 > FIND2_X)
	    FIND2_X = l1 + 9;
	if (l2 + 8 > FIND2_X)
	    FIND2_X = l2 + 8;

	/* compute amount of space between buttons for each row */
	r1 = (FIND2_X - 4 - l1) % 5;
	l1 = (FIND2_X - 4 - l1) / 5;
	r2 = (FIND2_X - 4 - l2) % 4;
	l2 = (FIND2_X - 4 - l2) / 4;

	/* ...and finally, place buttons */
	fbuts[2].x = 2 + r1 / 2 + l1;
	fbuts[3].x = fbuts[2].x + fbuts[2].len + l1;
	fbuts[0].x = fbuts[3].x + fbuts[3].len + l1;
	fbuts[4].x = fbuts[0].x + l0 + l1;
	fbuts[5].x = 2 + r2 / 2 + l2;
	fbuts[6].x = fbuts[5].x + fbuts[5].len + l2;
	fbuts[7].x = fbuts[6].x + fbuts[6].len + l2;
    }

    find_dlg = create_dlg (0, 0, FIND2_Y, FIND2_X, dialog_colors,
			   find_callback, "[Find File]", _("Find File"),
			   DLG_CENTER);

    add_widget (find_dlg,
		button_new (FIND2_Y - 3, fbuts[7].x, B_VIEW, NORMAL_BUTTON,
			    fbuts[7].text, find_do_edit_file, find_dlg,
			    "button-edit"));
    add_widget (find_dlg,
		button_new (FIND2_Y - 3, fbuts[6].x, B_VIEW, NORMAL_BUTTON,
			    fbuts[6].text, find_do_view_file, find_dlg,
			    "button-view"));
    add_widget (find_dlg,
		button_new (FIND2_Y - 3, fbuts[5].x, B_PANELIZE,
			    NORMAL_BUTTON, fbuts[5].text, 0, 0,
			    "button-panelize"));

    add_widget (find_dlg,
		button_new (FIND2_Y - 4, fbuts[4].x, B_CANCEL,
			    NORMAL_BUTTON, fbuts[4].text, 0, 0,
			    "button-quit"));
    stop_button =
	button_new (FIND2_Y - 4, fbuts[0].x, B_STOP, NORMAL_BUTTON,
		    fbuts[0].text, start_stop, find_dlg, "start-stop");
    add_widget (find_dlg, stop_button);
    add_widget (find_dlg,
		button_new (FIND2_Y - 4, fbuts[3].x, B_AGAIN,
			    NORMAL_BUTTON, fbuts[3].text, 0, 0,
			    "button-again"));
    add_widget (find_dlg,
		button_new (FIND2_Y - 4, fbuts[2].x, B_ENTER,
			    DEFPUSH_BUTTON, fbuts[2].text, 0, 0,
			    "button-chdir"));

    status_label =
	label_new (FIND2_Y - 6, 4, _("Searching"), "label-search");
    add_widget (find_dlg, status_label);

    find_list =
	listbox_new (2, 2, FIND2_X - 4, FIND2_Y - 9, listbox_finish, 0,
		     "listbox");
    add_widget (find_dlg, find_list);
}

static int
run_process (void)
{
    set_idle_proc (find_dlg, 1);
    run_dlg (find_dlg);
    return find_dlg->ret_value;
}

static void
status_update (char *text)
{
	label_set_text (status_label, text);
}

static void
kill_gui (void)
{
    set_idle_proc (find_dlg, 0);
    destroy_dlg (find_dlg);
}

static int
find_file (char *start_dir, char *pattern, char *content, char **dirname,  char **filename)
{
    int return_value = 0;
    char *dir;
    char *dir_tmp, *file_tmp;

    setup_gui ();
	    
    /* FIXME: Need to cleanup this, this ought to be passed non-globaly */
    find_pattern    = pattern;
    content_pattern = content;
    
    init_find_vars ();
    push_directory (start_dir);

    return_value = run_process ();

    /* Remove all the items in the stack */
    while ((dir = pop_directory ()) != NULL)
	free (dir);

    get_list_info (&file_tmp, &dir_tmp);

    if (dir_tmp)
	*dirname  = (char *) strdup (dir_tmp);
    if (file_tmp)
	*filename = (char *) strdup (file_tmp);

    if (return_value == B_PANELIZE && *filename){
	int status, link_to_dir, stalled_link;
	int next_free = 0;
	int i;
	struct stat buf;
	WLEntry *entry = find_list->list;
	dir_list *list = &cpanel->dir;
	char *dir, *name;

	for (i = 0; entry && i < find_list->count; entry = entry->next, i++){
	    char *filename;

	    if (!entry->text || !entry->data)
		continue;

	    if (content_pattern)
		filename = strchr (entry->text+4, ':')+1;
	    else
		filename = entry->text+4;

	    dir = entry->data;
	    if (dir [0] == '.' && dir [1] == 0)
		name = (char *) strdup (filename);
	    else if (dir [0] == '.' && dir [1] == PATH_SEP)
		name = concat_dir_and_file (dir + 2, filename);
	    else
		name = concat_dir_and_file (dir, filename);
	    status = handle_path (list, name, &buf, next_free, &link_to_dir,
	        &stalled_link);
	    if (status == 0) {
		free (name);
		continue;
	    }
	    if (status == -1) {
	        free (name);
		break;
	    }

	    /* don't add files more than once to the panel */
	    if (content_pattern && next_free > 0){
		if (strcmp (list->list [next_free-1].fname, name) == 0) {
		    free (name);
		    continue;
		}
	    }

	    if (!next_free) /* first turn i.e clean old list */
    		clean_dir (list, cpanel->count);
	    list->list [next_free].fnamelen = strlen (name);
	    list->list [next_free].fname = name;
	    file_mark (cpanel, next_free, 0);
	    list->list [next_free].f.link_to_dir = link_to_dir;
	    list->list [next_free].f.stalled_link = stalled_link;
	    list->list [next_free].f.dir_size_computed = 0; /* Fix by [DA] */
	    list->list [next_free].buf = buf;
		list->list [next_free].dir_size = 0; // WARNING!
	    next_free++;
           if (!(next_free & 15))
	       rotate_dash ();
	}
	if (next_free){
	    cpanel->count = next_free;
	    cpanel->is_panelized = 1;
	    cpanel->has_dir_sizes = 0;
	  /* Done by panel_clean_dir a few lines above 
	    cpanel->dirs_marked = 0;
	    cpanel->marked = 0;
	    cpanel->total = 0;
	    cpanel->top_file = 0;
	    cpanel->selected = 0;*/

	    if (start_dir [0] == PATH_SEP){
		strcpy (cpanel->cwd, PATH_SEP_STR);
		chdir (PATH_SEP_STR);
	    }
	}
    }

    kill_gui ();
    do_search (0); /* force do_search to release resources */
    if (old_dir){
	free (old_dir);
	old_dir = 0;
    }
    return return_value;
}

void
do_find (void)
{
    char *start_dir, *pattern, *content;
    char *filename, *dirname;
    int  v, dir_and_file_set;
    regex_t rx; /* Compiled content_pattern to search inside files */

    for (r = &rx; find_parameters (&start_dir, &pattern, &content); r = &rx){

	dirname = filename = NULL;
	is_start = 0;
	v = find_file (start_dir, pattern, content, &dirname, &filename);
	free (start_dir);
	free (pattern);
	if (r)
	    regfree (r);
	
	if (v == B_ENTER){
	    if (dirname || filename){
		if (dirname){
		    do_cd (dirname, cd_exact);
		    if (filename)
			try_to_select (cpanel, filename + (content ? 
			   (strchr (filename + 4, ':') - filename + 1) : 4) );
		} else if (filename)
		    do_cd (filename, cd_exact);
		paint_panel (cpanel);
		select_item (cpanel);
	    }
	    if (dirname)  
		free (dirname);
	    if (filename) 
		free (filename);
	    break;
	}
	if (content) 
	    free (content);
	dir_and_file_set = dirname && filename;
	if (dirname) free (dirname);
	if (filename) free (filename);
	if (v == B_CANCEL)
	    break;
	
	if (v == B_PANELIZE){
	    if (dir_and_file_set){
	        try_to_select (cpanel, NULL);
		panel_re_sort (cpanel);
	        paint_panel (cpanel);
	    }
	    break;
	}
    }
}

