/* User Menu implementation
   Copyright (C) 1994 Miguel de Icaza, Janne Kukonlehto
   
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <errno.h>

#include "global.h"
#include "tty.h"
#include "dialog.h"
#include "color.h"
#include "dir.h"
#include "panel.h"
#include "main.h"
#include "subshell.h" 		/* for subshell_pty */
#include "user.h"
#include "layout.h"
#include "setup.h"

#include "../vfs/vfs.h"

/* For the simple listbox manager */
#include "dlg.h"
#include "widget.h"
#include "wtools.h"

#include "view.h" /* for default_* externs */

#define MAX_ENTRIES 16
#define MAX_ENTRY_LEN 45

static int debug_flag = 0;
static int debug_error = 0;
static char *menu = NULL;

/* Formats defined:
   %%  The % character
   %f  The current file (if non-local vfs, file will be copied locally and
       %f will be full path to it).
   %p  The current file
   %d  The current working directory
   %s  "Selected files"; the tagged files if any, otherwise the current file
   %t  Tagged files
   %u  Tagged files (and they are untagged on return from expand_format)
   %view Runs the commands and pipes standard output to the view command.
       If %view is immediately followed by '{', recognize keywords
       ascii, hex, nroff and unform

   If the format letter is in uppercase, it refers to the other panel.
   
   With a number followed the % character you can turn quoting on (default)
   and off. For example:
       %f    quote expanded macro
       %1f   ditto
       %0f   don't quote expanded macro

   expand_format returns a memory block that must be free()d.
*/

/* Returns how many characters we should advance if %view was found */
int check_format_view (char *p)
{
    const char *q = p;
    if (!strncmp (p, "view", 4)) {
    	q += 4;
    	if (*q == '{') {
    	    for (q++;*q && *q != '}';q++) {
    	    	if (!strncmp (q, "ascii", 5)) {
    	    	    default_hex_mode = 0;
    	    	    q += 4;
    	    	} else if (!strncmp (q, "hex", 3)) {
    	    	    default_hex_mode = 1;
    	    	    q += 2;
    	    	} else if (!strncmp (q, "nroff", 5)) {
    	    	    default_nroff_flag = 1;
    	    	    q += 4;
    	    	} else if (!strncmp (q, "unform", 6)) {
    	    	    default_nroff_flag = 0;
    	    	    q += 5;
    	    	} 
    	    }
    	    if (*q == '}')
    	    	q++;
    	}
    	return q - p;
    }
    return 0;
}

int check_format_cd (char *p)
{
    return (strncmp (p, "cd", 2)) ? 0 : 3;
}

/* Check if p has a "^var\{var-name\}" */
/* Returns the number of skipped characters (zero on not found) */
/* V will be set to the expanded variable name */
int check_format_var (char *p, char **v)
{
    const char *q = p;
    char *var_name;
    char *value;
    const char *dots = 0;
    
    *v = 0;
    if (!strncmp (p, "var{", 4)) {
	for (q += 4; *q && *q != '}'; q++) {
	    if (*q == ':')
		dots = q+1;
	}
	if (!*q)
	    return 0;

	if (!dots || dots == q+5) {
	    message (1,
		     _(" Format error on file Extensions File "),
		     !dots ? _(" The %%var macro has no default ")
		     :       _(" The %%var macro has no variable "));
	    return 0;
	}
	
	/* Copy the variable name */
	var_name = (char *) malloc (dots - p);
	strncpy (var_name, p+4, dots-2 - (p+3));
	var_name [dots-2 - (p+3)] = 0;

	value = (char *) getenv (var_name);
	free (var_name);
	if (value) {
	    *v = strdup (value);
	    return q-p;
	}
	var_name = (char *) malloc (q - dots + 1);
	strncpy (var_name, dots, q - dots + 1);
	var_name [q-dots] = 0;
	*v = var_name;
	return q-p;
    }
    return 0;
}

/* strip file's extension */
static char *
strip_ext(char *ss)
{
    register char *s = ss;
    char *e = NULL;
    while(*s) {
        if(*s == '.') e = s;
        if(*s == PATH_SEP && e) e=NULL; /* '.' in *directory* name */
        s++;
    }
    if(e) *e = 0;
    return ss;
}

char *
expand_format (char c, int quote)
{
    WPanel *panel;
    char *(*quote_func) (const char *, int);
    char *fname;

    if (c == '%')
	return strdup ("%");

    if (islower ((unsigned) c))
	panel = cpanel;
    else {
	if (get_other_type () != view_listing)
	    return strdup ("");
	panel = other_panel;
    }
    if (!panel)
	panel = cpanel;

    if (quote)
	quote_func = name_quote;
    else
	quote_func = fake_name_quote;

    c = tolower (c);
    fname = panel->dir.list[panel->selected].fname;

    switch (c) {
    case 'f':
    case 'p':
	return (*quote_func) (fname, 0);
    case 'x':
	return (*quote_func) (extension (fname), 0);
    case 'd':
	return (*quote_func) (panel->cwd, 0);
    case 'k':			/* block file name */
    case 'b':			/* block file name / strip extension */  {
	    if (c == 'b') {
		return strip_ext ((*quote_func) (fname, 0));
	    }
	    break;
	}
    case 'm':			/* menu file name */
	if (menu)
	    return (*quote_func) (menu, 0);
	break;
    case 's':
	if (!panel->marked)
	    return (*quote_func) (fname, 0);

	/* Fall through */

    case 't':
    case 'u':
	{
	    int length = 2, i;
	    char *block, *tmp;

	    for (i = 0; i < panel->count; i++)
		if (panel->dir.list[i].f.marked)
		    length += strlen (panel->dir.list[i].fname) + 1;	/* for space */

	    block = (char *) malloc (length * 2 + 1);
	    *block = 0;
	    for (i = 0; i < panel->count; i++)
		if (panel->dir.list[i].f.marked) {
		    strcat (block, tmp =
			    (*quote_func) (panel->dir.list[i].fname, 0));
		    free (tmp);
		    strcat (block, " ");
		    if (c == 'u')
			do_file_mark (panel, i, 0);
		}
	    return block;
	}			/* sub case block */
    }				/* switch */
    return strdup ("");
}

/*
 * Check for the "shell_patterns" directive.  If it's found and valid,
 * interpret it and move the pointer past the directive.  Return the
 * current pointer.
 */
char *
check_patterns (char *p)
{
    static const char def_name[] = "shell_patterns=";
    char *p0 = p;

    if (strncmp (p, def_name, sizeof (def_name) - 1) != 0)
	return p0;

    p += sizeof (def_name) - 1;
    if (*p == '1')
	easy_patterns = 1;
    else if (*p == '0')
	easy_patterns = 0;
    else
	return p0;

    /* Skip spaces */
    p++;
    while (*p == '\n' || *p == '\t' || *p == ' ')
	p++;
    return p;
}

/* Copies a whitespace separated argument from p to arg. Returns the
   point after argument. */
static char *extract_arg (char *p, char *arg)
{
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n'))
	p++;
                /* support quote space .mnu */
    while (*p && (*p != ' ' || *(p-1) == '\\') && *p != '\t' && *p != '\n')
	*arg++ = *p++;
    *arg = 0;
    if (!*p || *p == '\n')
	p --;
    return p;
}

/* Tests whether the selected file in the panel is of any of the types
   specified in argument. */
static int test_type (WPanel *panel, char *arg)
{
    int result = 0; /* False by default */
    int st_mode = panel->dir.list [panel->selected].buf.st_mode;

    for (;*arg != 0; arg++) {
	switch (*arg) {
	case 'n':	/* Not a directory */
	    result |= !S_ISDIR (st_mode);
	    break;
	case 'r':	/* Regular file */
	    result |= S_ISREG (st_mode);
	    break;
	case 'd':	/* Directory */
	    result |= S_ISDIR (st_mode);
	    break;
	case 'l':	/* Link */
	    result |= S_ISLNK (st_mode);
	    break;
	case 'c':	/* Character special */
	    result |= S_ISCHR (st_mode);
	    break;
	case 'b':	/* Block special */
	    result |= S_ISBLK (st_mode);
	    break;
	case 'f':	/* Fifo (named pipe) */
	    result |= S_ISFIFO (st_mode);
	    break;
	case 's':	/* Socket */
	    result |= S_ISSOCK (st_mode);
	    break;
	case 'x':	/* Executable */
	    result |= (st_mode & 0111) ? 1 : 0;
	    break;
	case 't':
	    result |= panel->marked ? 1 : 0;
	    break;
	default:
	    debug_error = 1;
	    break;
	}
    }
    return result;
}

/* Calculates the truth value of the next condition starting from
   p. Returns the point after condition. */
static char *test_condition (char *p, int *condition)
{
    WPanel *panel;
    char arg [256];

    /* Handle one condition */
    for (;*p != '\n' && *p != '&' && *p != '|'; p++) {
                /* support quote space .mnu */
	if ((*p == ' ' && *(p-1) != '\\') || *p == '\t')
	    continue;
	if (*p >= 'a')
	    panel = cpanel;
	else {
	    if (get_other_type () == view_listing)
		panel = other_panel;
	    else
		panel = NULL;
	}
	*p |= 0x20;

	switch (*p++) {
	case '!':
	    p = test_condition (p, condition);
	    *condition = ! *condition;
	    p--;
	    break;
	case 'f': /* file name pattern */
	    p = extract_arg (p, arg);
	    *condition = panel && regexp_match (arg, panel->dir.list [panel->selected].fname, match_file);
	    break;
	case 'd':
	    p = extract_arg (p, arg);
	    *condition = panel && regexp_match (arg, panel->cwd, match_file);
	    break;
	case 't':
	    p = extract_arg (p, arg);
	    *condition = panel && test_type (panel, arg);
	    break;
	case 'x': /* executable */
	{
	    struct stat status;
	    
	    p = extract_arg (p, arg);
	    if (stat (arg, &status) == 0)
		*condition = is_exe (status.st_mode);
	    else
		*condition = 0; 
	    break;
	}
	default:
	    debug_error = 1;
	    break;
	} /* switch */

    } /* while */
    return p;
}

/* General purpose condition debug output handler */
static void
debug_out (char *start, char *end, int cond)
{
    static char msg [256];
    int len;

    if (start == NULL && end == NULL) {
	if (cond == 0) {
	    /* Init */
	    msg [0] = 0;
	} else {
	    /* Show output */
	    if (!debug_flag)
		return;
	    len = strlen (msg);
	    if (len)
		msg [len - 1] = 0;
	    message (0, _(" Debug "), "%s", msg);
	    debug_flag = 0;
	}
    } else {
	/* Save debug info for later output */
	if (!debug_flag)
	    return;
	/* Save the result of the condition */
	if (debug_error) {
	    strcat (msg, _(" ERROR: "));
	    debug_error = 0;
	}
	else if (cond)
	    strcat (msg, _(" True:  "));
	else
	    strcat (msg, _(" False: "));
	/* Copy condition statement */
	len = strlen (msg);
	if (end == NULL) {
	    /* Copy one character */
	    msg [len] = *start;
	    msg [len + 1] = 0;
	} else {
	    /* Copy many characters */
	    while (start < end) {
		msg [len++] = *start++;
	    }
	    msg [len] = 0;
	}
	strcat (msg, " \n");
    }
}

/* Calculates the truth value of one lineful of conditions. Returns
   the point just before the end of line. */
static char *test_line (char *p, int *result)
{
    int condition;
    char operator;
    char *debug_start, *debug_end;

    /* Init debugger */
    debug_out (NULL, NULL, 0);
    /* Repeat till end of line */
    while (*p && *p != '\n') {
        /* support quote space .mnu */
	while ((*p == ' ' && *(p-1) != '\\' ) || *p == '\t')
	    p++;
	if (!*p || *p == '\n')
	    break;
	operator = *p++;
	if (*p == '?') {
	    debug_flag = 1;
	    p++;
	}
        /* support quote space .mnu */
	while ((*p == ' ' && *(p-1) != '\\' ) || *p == '\t')
	    p++;
	if (!*p || *p == '\n')
	    break;
	condition = 1;	/* True by default */

	debug_start = p;
	p = test_condition (p, &condition);
	debug_end = p;
	/* Add one debug statement */
	debug_out (debug_start, debug_end, condition);

	switch (operator) {
	case '+':
	case '=':
	    /* Assignment */
	    *result = condition;
	    break;
	case '&':	/* Logical and */
	    *result &= condition;
	    break;
	case '|':	/* Logical or */
	    *result |= condition;
	    break;
	default:
	    debug_error = 1;
	    break;
	} /* switch */
	/* Add one debug statement */
	debug_out (&operator, NULL, *result);
	
    } /* while (*p != '\n') */
    /* Report debug message */
    debug_out (NULL, NULL, 1);

    if (!*p || *p == '\n')
	p --;
    return p;
}

/* FIXME: recode this routine on version 3.0, it could be cleaner */
static void
execute_menu_command (char *commands)
{
    FILE *cmd_file;
    int  cmd_file_fd;
    int  expand_prefix_found = 0;
    char *parameter = 0;
    int  do_quote = 0;
    char prompt [80];
    int  col;
    char *file_name;
#ifdef NATIVE_WIN32
    char *p;
#endif
    /* Skip menu entry title line */
    commands = strchr (commands, '\n');
    if (!commands) {
	return;
    }

    cmd_file_fd = mc_mkstemp (&file_name, "mcusr", SCRIPT_SUFFIX);

    if (cmd_file_fd == -1) {
	message (1, MSG_ERROR, _(" Cannot create temporary command file \n %s "),
		 unix_error_string (errno));
	return;
    }
    cmd_file = fdopen (cmd_file_fd, "w");
    commands++;
    
    for (col = 0; *commands; commands++) {
	if (col == 0) {
	    if (*commands != ' ' && *commands != '\t')
		break;
	    while (*commands == ' ' || *commands == '\t')
	        commands++;
	}
	col++;
	if (*commands == '\n')
	    col = 0;
	if (parameter) {
	    if (*commands == '}') {
		char *tmp;
		*parameter = 0;
		parameter = input_dialog (_(" Parameter "), prompt, "");
		if (!parameter || !*parameter) {
		    /* User canceled */
		    fclose (cmd_file);
		    unlink (file_name);
                    free (file_name);
		    return;
		}
		if (do_quote) {
    		    fputs (tmp = name_quote (parameter, 0), cmd_file);
		    free (tmp);
		} else
		    fputs (parameter, cmd_file);
		free (parameter);
		parameter = 0;
	    } else {
		if (parameter < &prompt [sizeof (prompt) - 1]) {
		    *parameter++ = *commands;
		} 
	    }
	} else if (expand_prefix_found) {
	    expand_prefix_found = 0;
	    if (isdigit ((unsigned)*commands)) {
		do_quote = atoi (commands);
		while (isdigit ((unsigned)*commands))
		    commands++;
	    }
	    if (*commands == '{')
		parameter = prompt;
	    else{
		char *text = expand_format (*commands, do_quote);
		fputs (text, cmd_file);
		free (text);
	    }
	} else {
	    if (*commands == '%') {
		do_quote = 1; /* Default: Quote expanded macro */
		expand_prefix_found = 1;
	    } else
		fputc (*commands, cmd_file);
	}
    }
    fclose (cmd_file);
    chmod (file_name, S_IRWXU);
    execute (file_name);
    unlink (file_name);
    free (file_name);
}

/* 
**	Check owner of the menu file. Using menu file is allowed, if
**	owner of the menu is root or the actual user. In either case
**	file should not be group and word-writable.
**	
**	Q. Should we apply this routine to system and home menu (and .ext files)?
*/

static int
menu_file_own(char* path)
{
    struct stat st;

    if (stat (path, &st) == 0
	&& (!st.st_uid || (st.st_uid == geteuid ()))
	&& ((st.st_mode & (S_IWGRP | S_IWOTH)) == 0)
    ) {
	return 1;
    }

    return 0;
}

/*
 * If edit_widget is NULL then we are called from the mc menu,
 * otherwise we are called from the mcedit menu.
 */

void user_menu_cmd (void)
{
    char *p;
    char *data, **entries;
    int  max_cols, menu_lines, menu_limit;
    int  col, i, accept_entry = 1;
    int  selected, old_patterns;
    Listbox *listbox;
    
    if (!vfs_current_is_local ()) {
	message (1, MSG_ERROR,
		 _(" Cannot execute commands on non-local filesystems"));
	return;
    }
    
    menu = strdup (MC_LOCAL_MENU);
    if (!exist_file (menu) || !menu_file_own (menu)) {
	free (menu);
        menu = concat_dir_and_file \
                            (home_dir, MC_HOME_MENU);
	if (!exist_file (menu)) {
	    free (menu);
	    menu = concat_dir_and_file \
                        (mc_home, MC_GLOBAL_MENU);
	}
    }

    if ((data = load_file (menu)) == NULL) {
	message (1, MSG_ERROR, _(" Cannot open file %s \n %s "),
		 menu, unix_error_string (errno));
	free (menu);
	menu = NULL;
	return;
    }
    
    max_cols = 0;
    selected = 0;
    menu_limit = 0;
    entries = 0;

    /* Parse the menu file */
    old_patterns = easy_patterns;
    p = check_patterns (data);
    for (menu_lines = col = 0; *p; p++) {
	if (menu_lines >= menu_limit) {
	    char ** new_entries;
	    
	    menu_limit += MAX_ENTRIES;
	    new_entries = realloc (entries, sizeof (new_entries[0]) * menu_limit);

	    if (new_entries == 0)
		break;

	    entries = new_entries;
	    new_entries += menu_limit;
	    while (--new_entries >= &entries[menu_lines])
		*new_entries = 0;
	}
	if (col == 0 && !entries [menu_lines]) {
	    if (*p == '#') {
		/* A commented menu entry */
		accept_entry = 1;
	    } else if (*p == '+') {
		if (*(p+1) == '=') {
		    /* Combined adding and default */
		    p = test_line (p, &accept_entry);
		    if (selected == 0 && accept_entry)
			selected = menu_lines;
		} else {
		    /* A condition for adding the entry */
		    p = test_line (p, &accept_entry);
		}
	    } else if (*p == '=') {
		if (*(p+1) == '+') {
		    /* Combined adding and default */
		    p = test_line (p, &accept_entry);
		    if (selected == 0 && accept_entry)
			selected = menu_lines;
		} else {
		    /* A condition for making the entry default */
		    i = 1;
		    p = test_line (p, &i);
		    if (selected == 0 && i)
			selected = menu_lines;
		}
	    }
	    else if (*p != ' ' && *p != '\t' && is_printable (*p)) {
		/* A menu entry title line */
		if (accept_entry)
		    entries [menu_lines] = p;
		else
		    accept_entry = 1;
	    }
	}
	if (*p == '\n') {
	    if (entries [menu_lines]) {
		menu_lines++;
		accept_entry = 1;
	    }
	    max_cols = max (max_cols, col);
	    col = 0;
	} else {
	    if (*p == '\t')
		*p = ' ';
	    col++;
	}
    }

    if (menu_lines == 0) {
	message (1, MSG_ERROR, _(" No suitable entries found in %s "), menu);
    } else {

    max_cols = min (max (max_cols, col), MAX_ENTRY_LEN);
 
    /* Create listbox */
    listbox = create_listbox_window (max_cols+2, menu_lines, _(" User menu "),
				     "[Menu File Edit]");
    /* insert all the items found */
    for (i = 0; i < menu_lines; i++) {
	static unsigned char hk;
	p = entries [i];
    /* extracting hotkey */
	hk = p[0];
    /* skip hotkey char */
	++p;
    /* if here spaces, cut them away */
      while (isspace ((unsigned char) p[0]))
	++p;

    /* and, finally, add normal menu item */
	LISTBOX_APPEND_TEXT (listbox, hk, extract_line (p, p + MAX_ENTRY_LEN), p);
    }
    /* Select the default entry */
    listbox_select_by_number (listbox->list, selected);
    
    selected = run_listbox (listbox);
    if (selected >= 0)
	execute_menu_command (entries [selected]);

    do_refresh ();
    }

    easy_patterns = old_patterns;
    free (menu);
    menu = NULL;
    if (entries)
	free (entries);
    free (data);
}

