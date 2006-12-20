/*
 * $Id$
 *
 *  cdialog - Display simple dialog boxes from shell scripts
 *
 *  Copyright 2000-2004,2005	Thomas E. Dickey
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif

#define PASSARGS             t,       av,        offset_add
#define CALLARGS const char *t, char *av[], int *offset_add
typedef int (callerFn) (CALLARGS);

typedef enum {
    o_unknown = 0
    ,o_allow_close
    ,o_and_widget
    ,o_aspect
    ,o_auto_placement
    ,o_backtitle
    ,o_beep
    ,o_beep_after
    ,o_begin
    ,o_calendar
    ,o_cancel_label
    ,o_checklist
    ,o_clear
    ,o_colors
    ,o_cr_wrap
    ,o_create_rc
    ,o_default_item
    ,o_defaultno
    ,o_exit_label
    ,o_extra_button
    ,o_extra_label
    ,o_fixed_font
    ,o_form
    ,o_fselect
    ,o_fullbutton
    ,o_gauge
    ,o_help
    ,o_help_button
    ,o_help_label
    ,o_help_status
    ,o_icon
    ,o_ignore
    ,o_infobox
    ,o_input_fd
    ,o_inputbox
    ,o_inputmenu
    ,o_insecure
    ,o_item_help
    ,o_keep_colors
    ,o_keep_window
    ,o_max_input
    ,o_menu
    ,o_msgbox
    ,o_no_close
    ,o_no_collapse
    ,o_no_cr_wrap
    ,o_no_kill
    ,o_no_label
    ,o_no_shadow
    ,o_nocancel
    ,o_noitem
    ,o_ok_label
    ,o_output_fd
    ,o_passwordbox
    ,o_passwordform
    ,o_pause
    ,o_print_maxsize
    ,o_print_size
    ,o_print_version
    ,o_progressbox
    ,o_radiolist
    ,o_screen_center
    ,o_separate_output
    ,o_separate_widget
    ,o_separator
    ,o_shadow
    ,o_single_quoted
    ,o_size_err
    ,o_sleep
    ,o_smooth
    ,o_stderr
    ,o_stdout
    ,o_tab_correct
    ,o_tab_len
    ,o_tailbox
    ,o_tailboxbg
    ,o_textbox
    ,o_timebox
    ,o_timeout
    ,o_title
    ,o_trim
    ,o_visit_items
    ,o_under_mouse
    ,o_wmclass
    ,o_yes_label
    ,o_yesno
} eOptions;

/*
 * The bits in 'pass' are used to decide which options are applicable at
 * different stages in the program:
 *	1 flags before widgets
 *	2 widgets
 *	4 non-widget options
 */
typedef struct {
    const char *name;
    eOptions code;
    int pass;			/* 1,2,4 or combination */
    const char *help;		/* NULL to suppress, non-empty to display params */
} Options;

typedef struct {
    eOptions code;
    int argmin, argmax;
    callerFn *jumper;
} Mode;

static bool *dialog_opts;
static char **dialog_argv;

static bool ignore_unknown = FALSE;

static const char *program = "dialog";

/*
 * The options[] table is organized this way to make it simple to maintain
 * a sorted list of options for the help-message.
 */
/* *INDENT-OFF* */
static const Options options[] = {
    { "allow-close",	o_allow_close,		1, NULL },
    { "and-widget",	o_and_widget,		4, NULL },
    { "aspect",		o_aspect,		1, "<ratio>" },
    { "auto-placement", o_auto_placement,	1, NULL },
    { "backtitle",	o_backtitle,		1, "<backtitle>" },
    { "beep",		o_beep,			1, NULL },
    { "beep-after",	o_beep_after,		1, NULL },
    { "begin",		o_begin,		1, "<y> <x>" },
    { "calendar",	o_calendar,		2, "<text> <height> <width> <day> <month> <year>" },
    { "cancel-label",	o_cancel_label,		1, "<str>" },
    { "checklist",	o_checklist,		2, "<text> <height> <width> <list height> <tag1> <item1> <status1>..." },
    { "clear",		o_clear,		1, "" },
    { "colors",		o_colors,		1, "" },
    { "cr-wrap",	o_cr_wrap,		1, "" },
    { "create-rc",	o_create_rc,		1, NULL },
    { "default-item",	o_default_item,		1, "<str>" },
    { "defaultno",	o_defaultno,		1, "" },
    { "exit-label",	o_exit_label,		1, "<str>" },
    { "extra-button",	o_extra_button,		1, "" },
    { "extra-label",	o_extra_label,		1, "<str>" },
    { "fb",		o_fullbutton,		1, NULL },
    { "fixed-font",	o_fixed_font,		1, NULL },
    { "form",		o_form,     	 	2, "<text> <height> <width> <form height> <label1> <l_y1> <l_x1> <item1> <i_y1> <i_x1> <flen1> <ilen1>..." },
    { "fselect",	o_fselect,		2, "<filepath> <height> <width>" },
    { "fullbutton",	o_fullbutton,		1, NULL },
    { "gauge",		o_gauge,		2, "<text> <height> <width> [<percent>]" },
    { "guage",		o_gauge,		2, NULL },
    { "help",		o_help,			4, "" },
    { "help-button",	o_help_button,		1, "" },
    { "help-label",	o_help_label,		1, "<str>" },
    { "help-status",	o_help_status,		1, "" },
    { "icon",		o_icon,			1, NULL },
    { "ignore",		o_ignore,		1, "" },
    { "infobox",	o_infobox,		2, "<text> <height> <width>" },
    { "input-fd",	o_input_fd,		1, "<fd>" },
    { "inputbox",	o_inputbox,		2, "<text> <height> <width> [<init>]" },
    { "inputmenu",	o_inputmenu,		2, "<text> <height> <width> <menu height> <tag1> <item1>..." },
    { "insecure",	o_insecure,		1, "" },
    { "item-help",	o_item_help,		1, "" },
    { "keep-colors",	o_keep_colors,		1, NULL },
    { "keep-window",	o_keep_window,		1, "" },
    { "max-input",	o_max_input,		1, "<n>" },
    { "menu",		o_menu,			2, "<text> <height> <width> <menu height> <tag1> <item1>..." },
    { "msgbox",		o_msgbox,		2, "<text> <height> <width>" },
    { "no-cancel",	o_nocancel,		1, "" },
    { "no-close",	o_no_close,		1, NULL },
    { "no-collapse",	o_no_collapse,		1, "" },
    { "no-cr-wrap",	o_no_cr_wrap,		1, NULL },
    { "no-kill",	o_no_kill,		1, "" },
    { "no-label",	o_no_label,		1, "<str>" },
    { "no-shadow",	o_no_shadow,		1, "" },
    { "nocancel",	o_nocancel,		1, NULL }, /* see --no-cancel */
    { "noitem",		o_noitem,		1, NULL },
    { "ok-label",	o_ok_label,		1, "<str>" },
    { "output-fd",	o_output_fd,		1, "<fd>" },
    { "passwordbox",	o_passwordbox,		2, "<text> <height> <width> [<init>]" },
    { "passwordform",	o_passwordform,     	2, "<text> <height> <width> <form height> <label1> <l_y1> <l_x1> <item1> <i_y1> <i_x1> <flen1> <ilen1>..." },
    { "pause",		o_pause,		2, "<text> <height> <width> <seconds>" },
    { "print-maxsize",	o_print_maxsize,	1, "" },
    { "print-size",	o_print_size,		1, "" },
    { "print-version",	o_print_version,	5, "" },
    { "radiolist",	o_radiolist,		2, "<text> <height> <width> <list height> <tag1> <item1> <status1>..." },
    { "screen-center",	o_screen_center,	1, NULL },
    { "separate-output",o_separate_output,	1, "" },
    { "separate-widget",o_separate_widget,	1, "<str>" },
    { "separator",	o_separator,		1, NULL },
    { "shadow",		o_shadow,		1, "" },
    { "single-quoted",	o_single_quoted,	1, "" },
    { "size-err",	o_size_err,		1, "" },
    { "sleep",		o_sleep,		1, "<secs>" },
    { "smooth",		o_smooth,		1, NULL },
    { "stderr",		o_stderr,		1, "" },
    { "stdout",		o_stdout,		1, "" },
    { "tab-correct",	o_tab_correct,		1, "" },
    { "tab-len",	o_tab_len,		1, "<n>" },
    { "tailbox",	o_tailbox,		2, "<file> <height> <width>" },
    { "tailboxbg",	o_tailboxbg,		2, "<file> <height> <width>" },
    { "textbox",	o_textbox,		2, "<file> <height> <width>" },
    { "timebox",	o_timebox,		2, "<text> <height> <width> <hour> <minute> <second>" },
    { "timeout",	o_timeout,		1, "<secs>" },
    { "title",		o_title,		1, "<title>" },
    { "trim",		o_trim,			1, "" },
    { "progressbox",	o_progressbox,		2, "<height> <width>" },
    { "visit-items", 	o_visit_items,		1, "" },
    { "under-mouse", 	o_under_mouse,		1, NULL },
    { "version",	o_print_version,	5, "" },
    { "wmclass",	o_wmclass,		1, NULL },
    { "yes-label",	o_yes_label,		1, "<str>" },
    { "yesno",		o_yesno,		2, "<text> <height> <width>" },
};
/* *INDENT-ON* */

/*
 * Convert a string to an argv[], returning a char** index (which must be
 * freed by the caller).  The string is modified (replacing gaps between
 * tokens with nulls).
 */
static char **
string_to_argv(char *blob)
{
    int n;
    int pass;
    int length = strlen(blob);
    char **result = 0;

    for (pass = 0; pass < 2; ++pass) {
	bool inparm = FALSE;
	bool quoted = FALSE;
	char *param = blob;
	int count = 0;

	for (n = 0; n < length; ++n) {
	    if (quoted && blob[n] == '"') {
		quoted = FALSE;
	    } else if (blob[n] == '"') {
		quoted = TRUE;
		if (!inparm) {
		    if (pass)
			result[count] = param;
		    ++count;
		    inparm = TRUE;
		}
	    } else if (blob[n] == '\\') {
		if (quoted && !isspace(CharOf(blob[n + 1]))) {
		    if (!inparm) {
			if (pass)
			    result[count] = param;
			++count;
			inparm = TRUE;
		    }
		    if (pass) {
			*param++ = blob[n];
			*param++ = blob[n + 1];
		    }
		}
		++n;
	    } else if (!quoted && isspace(CharOf(blob[n]))) {
		inparm = FALSE;
		if (pass) {
		    *param++ = '\0';
		}
	    } else {
		if (!inparm) {
		    if (pass)
			result[count] = param;
		    ++count;
		    inparm = TRUE;
		}
		if (pass) {
		    *param++ = blob[n];
		}
	    }
	}

	if (!pass) {
	    if (count) {
		result = calloc(count + 1, sizeof(char *));
		assert_ptr(result, "string_to_argv");
	    } else {
		break;		/* no tokens found */
	    }
	} else {
	    *param = '\0';
	}
    }
    return result;
}

/*
 * Count the entries in an argv list.
 */
static int
count_argv(char **argv)
{
    int result = 0;

    if (argv != 0) {
	while (argv[result] != 0)
	    ++result;
    }
    return result;
}

/*
 * Make an array showing which argv[] entries are options.  Use "--" as a
 * special token to escape the next argument, allowing it to begin with "--".
 * When we find a "--" argument, also remove it from argv[] and adjust argc.
 * That appears to be an undocumented feature of the popt library.
 *
 * Also, if we see a "--file", expand it into the parameter list by reading the
 * text from the given file and stripping quotes, treating whitespace outside
 * quotes as a parameter delimiter.
 *
 * Finally, if we see a "--args", dump the current list of arguments to the
 * standard error.  This is used for debugging complex --file combinations.
 */
static void
unescape_argv(int *argcp, char ***argvp)
{
    int j, k;
    int limit_includes = 20 + *argcp;
    int count_includes = 0;
    bool changed = FALSE;
    bool doalloc = FALSE;
    char *filename;

    dialog_opts = calloc(*argcp + 1, sizeof(bool));
    assert_ptr(dialog_opts, "unescape_argv");

    for (j = 1; j < *argcp; j++) {
	bool escaped = FALSE;
	if (!strcmp((*argvp)[j], "--")) {
	    changed = TRUE;
	    escaped = TRUE;
	    *argcp -= 1;
	    for (k = j; k <= *argcp; k++)
		(*argvp)[k] = (*argvp)[k + 1];
	} else if (!strcmp((*argvp)[j], "--args")) {
	    fprintf(stderr, "Showing arguments at arg%d\n", j);
	    for (k = 0; k < *argcp; ++k) {
		fprintf(stderr, " arg%d:%s\n", k, (*argvp)[k]);
	    }
	    changed = TRUE;
	    *argcp -= 1;
	    for (k = j; k <= *argcp; k++)
		(*argvp)[k] = (*argvp)[k + 1];
	} else if (!strcmp((*argvp)[j], "--file")) {
	    if (++count_includes > limit_includes)
		dlg_exiterr("Too many --file options");

	    if ((filename = (*argvp)[j + 1]) != 0) {
		FILE *fp;
		char **list;
		char *blob;
		int added;
		int length;
		int n;
		struct stat sb;

		if (stat(filename, &sb) == 0
		    && (sb.st_mode & S_IFMT) == S_IFREG
		    && sb.st_size != 0) {

		    blob = malloc(sb.st_size + 1);
		    assert_ptr(blob, "unescape_argv");

		    fp = fopen(filename, "r");
		    if (fp != 0) {
			length = fread(blob, sizeof(char), sb.st_size, fp);
			fclose(fp);
		    } else {
			length = 0;
		    }
		    blob[length] = '\0';

		    list = string_to_argv(blob);
		    if ((added = count_argv(list)) != 0) {
			if (added > 2) {
			    size_t need = sizeof(char *) * (*argcp + added + 1);
			    if (doalloc) {
				*argvp = realloc(*argvp, need);
				assert_ptr(*argvp, "unescape_argv");
			    } else {
				char **newp = malloc(need);
				assert_ptr(newp, "unescape_argv");
				for (n = 0; n < *argcp; ++n) {
				    newp[n] = (*argvp)[n];
				}
				*argvp = newp;
				doalloc = TRUE;
			    }
			    dialog_opts = realloc(dialog_opts, *argcp + added);
			    assert_ptr(dialog_opts, "unescape_argv");
			}
			for (n = *argcp; n >= j + 2; --n) {
			    (*argvp)[n + added - 2] = (*argvp)[n];
			    dialog_opts[n + added - 2] = dialog_opts[n];
			}
			for (n = 0; n < added; ++n) {
			    (*argvp)[n + j] = list[n];
			    dialog_opts[n + j] = FALSE;
			}
			*argcp += added - 2;
			free(list);
		    }
		}
		(*argvp)[*argcp] = 0;
		++j;
		continue;
	    }
	}
	if (!escaped
	    && (*argvp)[j] != 0
	    && !strncmp((*argvp)[j], "--", 2)
	    && isalpha(UCH((*argvp)[j][2]))) {
	    dialog_opts[j] = TRUE;
	}
    }

    /* if we didn't find any "--" tokens, there's no reason to do the table
     * lookup in isOption()
     */
    if (!changed) {
	free(dialog_opts);
	dialog_opts = 0;
    }
    dialog_argv = (*argvp);
}

/*
 * Check if the given string from main's argv is an option.
 */
static bool
isOption(const char *arg)
{
    bool result = FALSE;

    if (arg != 0) {
	if (dialog_opts != 0) {
	    int n;
	    for (n = 1; dialog_argv[n] != 0; ++n) {
		if (dialog_argv[n] == arg) {
		    result = dialog_opts[n];
		    break;
		}
	    }
	} else if (!strncmp(arg, "--", 2) && isalpha(UCH(arg[2]))) {
	    result = TRUE;
	}
    }
    return result;
}

static eOptions
lookupOption(const char *name, int pass)
{
    unsigned n;

    if (isOption(name)) {
	name += 2;
	for (n = 0; n < sizeof(options) / sizeof(options[0]); n++) {
	    if ((pass & options[n].pass) != 0
		&& !strcmp(name, options[n].name)) {
		return options[n].code;
	    }
	}
    }
    return o_unknown;
}

static void
Usage(char *msg)
{
    dlg_exiterr("Error: %s.\nUse --help to list options.\n\n", msg);
}

/*
 * Count arguments, stopping at the end of the argument list, or on any of our
 * "--" tokens.
 */
static int
arg_rest(char *argv[])
{
    int i = 1;			/* argv[0] points to a "--" token */

    while (argv[i] != 0
	   && (!isOption(argv[i]) || lookupOption(argv[i], 7) == o_unknown))
	i++;
    return i;
}

/*
 * In MultiWidget this function is needed to count how many tags
 * a widget (menu, checklist, radiolist) has
 */
static int
howmany_tags(char *argv[], int group)
{
    int result = 0;
    int have;
    const char *format = "Expected %d arguments, found only %d";
    char temp[80];

    while (argv[0] != 0) {
	if (isOption(argv[0]))
	    break;
	if ((have = arg_rest(argv)) < group) {
	    sprintf(temp, format, group, have);
	    Usage(temp);
	}

	argv += group;
	result++;
    }

    return result;
}

static int
numeric_arg(char **av, int n)
{
    char *last = 0;
    int result = strtol(av[n], &last, 10);
    char msg[80];

    if (last == 0 || *last != 0) {
	sprintf(msg, "Expected a number for token %d of %.20s", n, av[0]);
	Usage(msg);
    }
    return result;
}

static char *
optional_str(char **av, int n, char *dft)
{
    char *ret = dft;
    if (arg_rest(av) > n)
	ret = av[n];
    return ret;
}

static int
optional_num(char **av, int n, int dft)
{
    int ret = dft;
    if (arg_rest(av) > n)
	ret = numeric_arg(av, n);
    return ret;
}

/*
 * On AIX 4.x, we have to flush the output right away since there is a bug in
 * the curses package which discards stdout even when we've used newterm to
 * redirect output to /dev/tty.
 */
static int
show_result(int ret)
{
    bool either = FALSE;

    switch (ret) {
    case DLG_EXIT_OK:
    case DLG_EXIT_EXTRA:
    case DLG_EXIT_HELP:
    case DLG_EXIT_ITEM_HELP:
	if ((dialog_state.output_count > 1) && !dialog_vars.separate_output) {
	    fputs((dialog_state.separate_str
		   ? dialog_state.separate_str
		   : DEFAULT_SEPARATE_STR),
		  dialog_state.output);
	    either = TRUE;
	}
	if (dialog_vars.input_result[0] != '\0') {
	    fputs(dialog_vars.input_result, dialog_state.output);
	    either = TRUE;
	}
	if (either) {
	    fflush(dialog_state.output);
	}
	break;
    }
    return ret;
}

/*
 * These are the widget callers.
 */

static int
call_yesno(CALLARGS)
{
    *offset_add = 4;
    return dialog_yesno(t,
			av[1],
			numeric_arg(av, 2),
			numeric_arg(av, 3));
}

static int
call_msgbox(CALLARGS)
{
    *offset_add = 4;
    return dialog_msgbox(t,
			 av[1],
			 numeric_arg(av, 2),
			 numeric_arg(av, 3), 1);
}

static int
call_infobox(CALLARGS)
{
    *offset_add = 4;
    return dialog_msgbox(t,
			 av[1],
			 numeric_arg(av, 2),
			 numeric_arg(av, 3), 0);
}

static int
call_textbox(CALLARGS)
{
    *offset_add = 4;
    return dialog_textbox(t,
			  av[1],
			  numeric_arg(av, 2),
			  numeric_arg(av, 3));
}

static int
call_menu(CALLARGS)
{
    int tags = howmany_tags(av + 5, MENUBOX_TAGS);
    *offset_add = 5 + tags * MENUBOX_TAGS;

    return dialog_menu(t,
		       av[1],
		       numeric_arg(av, 2),
		       numeric_arg(av, 3),
		       numeric_arg(av, 4),
		       tags, av + 5);
}

static int
call_inputmenu(CALLARGS)
{
    int tags = howmany_tags(av + 5, MENUBOX_TAGS);

    dialog_vars.input_menu = TRUE;

    if (dialog_vars.max_input == 0)
	dialog_vars.max_input = MAX_LEN / 2;

    if (dialog_vars.extra_label == 0)
	dialog_vars.extra_label = _("Rename");

    dialog_vars.extra_button = TRUE;

    *offset_add = 5 + tags * MENUBOX_TAGS;
    return dialog_menu(t,
		       av[1],
		       numeric_arg(av, 2),
		       numeric_arg(av, 3),
		       numeric_arg(av, 4),
		       tags, av + 5);
}

static int
call_checklist(CALLARGS)
{
    int tags = howmany_tags(av + 5, CHECKBOX_TAGS);
    *offset_add = 5 + tags * CHECKBOX_TAGS;
    return dialog_checklist(t,
			    av[1],
			    numeric_arg(av, 2),
			    numeric_arg(av, 3),
			    numeric_arg(av, 4),
			    tags, av + 5, FLAG_CHECK);
}

static int
call_radiolist(CALLARGS)
{
    int tags = howmany_tags(av + 5, CHECKBOX_TAGS);
    *offset_add = 5 + tags * CHECKBOX_TAGS;
    return dialog_checklist(t,
			    av[1],
			    numeric_arg(av, 2),
			    numeric_arg(av, 3),
			    numeric_arg(av, 4),
			    tags, av + 5, FLAG_RADIO);
}

static int
call_inputbox(CALLARGS)
{
    *offset_add = arg_rest(av);
    return dialog_inputbox(t,
			   av[1],
			   numeric_arg(av, 2),
			   numeric_arg(av, 3),
			   optional_str(av, 4, 0), 0);
}

static int
call_passwordbox(CALLARGS)
{
    *offset_add = arg_rest(av);
    return dialog_inputbox(t,
			   av[1],
			   numeric_arg(av, 2),
			   numeric_arg(av, 3),
			   optional_str(av, 4, 0), 1);
}

#ifdef HAVE_XDIALOG
static int
call_calendar(CALLARGS)
{
    *offset_add = arg_rest(av);
    return dialog_calendar(t,
			   av[1],
			   numeric_arg(av, 2),
			   numeric_arg(av, 3),
			   optional_num(av, 4, -1),
			   optional_num(av, 5, -1),
			   optional_num(av, 6, -1));
}

static int
call_fselect(CALLARGS)
{
    *offset_add = arg_rest(av);
    return dialog_fselect(t,
			  av[1],
			  numeric_arg(av, 2),
			  numeric_arg(av, 3));
}

static int
call_timebox(CALLARGS)
{
    *offset_add = arg_rest(av);
    return dialog_timebox(t,
			  av[1],
			  numeric_arg(av, 2),
			  numeric_arg(av, 3),
			  optional_num(av, 4, -1),
			  optional_num(av, 5, -1),
			  optional_num(av, 6, -1));
}
#endif

#ifdef HAVE_FORMBOX
static int
call_form(CALLARGS)
{
    int tags = howmany_tags(av + 5, FORMBOX_TAGS);
    *offset_add = 5 + tags * FORMBOX_TAGS;

    return dialog_form(t,
		       av[1],
		       numeric_arg(av, 2),
		       numeric_arg(av, 3),
		       numeric_arg(av, 4),
		       tags, av + 5);
}

static int
call_password_form(CALLARGS)
{
    int save = dialog_vars.formitem_type;
    int result;

    dialog_vars.formitem_type = 1;
    result = call_form(PASSARGS);
    dialog_vars.formitem_type = save;

    return result;
}
#endif

#ifdef HAVE_GAUGE
static int
call_gauge(CALLARGS)
{
    *offset_add = arg_rest(av);
    return dialog_gauge(t,
			av[1],
			numeric_arg(av, 2),
			numeric_arg(av, 3),
			optional_num(av, 4, 0));
}
#endif

#ifdef HAVE_GAUGE
static int
call_pause(CALLARGS)
{
    *offset_add = arg_rest(av);
    return dialog_pause(t,
			av[1],
			numeric_arg(av, 2),
			numeric_arg(av, 3),
			numeric_arg(av, 4));
}
#endif

#ifdef HAVE_GAUGE
static int
call_progressbox(CALLARGS)
{
    *offset_add = arg_rest(av);
    /* the original version does not accept a prompt string, but for
     * consistency we allow it.
     */
    return ((*offset_add == 4)
	    ? dialog_progressbox(t,
				 av[1],
				 numeric_arg(av, 2),
				 numeric_arg(av, 3))
	    : dialog_progressbox(t,
				 "",
				 numeric_arg(av, 1),
				 numeric_arg(av, 2)));
}
#endif

#ifdef HAVE_TAILBOX
static int
call_tailbox(CALLARGS)
{
    *offset_add = 4;
    return dialog_tailbox(t,
			  av[1],
			  numeric_arg(av, 2),
			  numeric_arg(av, 3),
			  FALSE);
}

static int
call_tailboxbg(CALLARGS)
{
    *offset_add = 4;
    return dialog_tailbox(t,
			  av[1],
			  numeric_arg(av, 2),
			  numeric_arg(av, 3),
			  TRUE);
}
#endif
/* *INDENT-OFF* */
static const Mode modes[] =
{
    {o_yesno,           4, 4, call_yesno},
    {o_msgbox,          4, 4, call_msgbox},
    {o_infobox,         4, 4, call_infobox},
    {o_textbox,         4, 4, call_textbox},
    {o_menu,            7, 0, call_menu},
    {o_inputmenu,       7, 0, call_inputmenu},
    {o_checklist,       8, 0, call_checklist},
    {o_radiolist,       8, 0, call_radiolist},
    {o_inputbox,        4, 5, call_inputbox},
    {o_passwordbox,     4, 5, call_passwordbox},
#ifdef HAVE_XDIALOG
    {o_calendar,        4, 7, call_calendar},
    {o_fselect,         4, 5, call_fselect},
    {o_timebox,         4, 7, call_timebox},
#endif
#ifdef HAVE_FORMBOX
    {o_passwordform,   13, 0, call_password_form},
    {o_form,           13, 0, call_form},
#endif
#ifdef HAVE_GAUGE
    {o_gauge,           4, 5, call_gauge},
    {o_pause,           5, 5, call_pause},
    {o_progressbox,     3, 4, call_progressbox},
#endif
#ifdef HAVE_TAILBOX
    {o_tailbox,         4, 4, call_tailbox},
    {o_tailboxbg,       4, 4, call_tailboxbg},
#endif
};
/* *INDENT-ON* */

static char *
optionString(char **argv, int *num)
{
    int next = *num + 1;
    char *result = argv[next];
    if (result == 0) {
	char temp[80];
	sprintf(temp, "Expected a string-parameter for %.20s", argv[*num]);
	Usage(temp);
    }
    *num = next;
    return result;
}

static int
optionValue(char **argv, int *num)
{
    int next = *num + 1;
    char *src = argv[next];
    char *tmp = 0;
    int result = 0;

    if (src != 0) {
	result = strtol(src, &tmp, 0);
	if (tmp == 0 || *tmp != 0)
	    src = 0;
    }

    if (src == 0) {
	char temp[80];
	sprintf(temp, "Expected a numeric-parameter for %.20s", argv[*num]);
	Usage(temp);
    }
    *num = next;
    return result;
}

/*
 * Print parts of a message
 */
static void
PrintList(const char *const *list)
{
    const char *leaf = strrchr(program, '/');
    unsigned n = 0;

    if (leaf != 0)
	leaf++;
    else
	leaf = program;

    while (*list != 0) {
	fprintf(dialog_state.output, *list, n ? leaf : dialog_version());
	(void) fputc('\n', dialog_state.output);
	n = 1;
	list++;
    }
}

static const Mode *
lookupMode(eOptions code)
{
    const Mode *modePtr = 0;
    unsigned n;

    for (n = 0; n < sizeof(modes) / sizeof(modes[0]); n++) {
	if (modes[n].code == code) {
	    modePtr = &modes[n];
	    break;
	}
    }
    return modePtr;
}

/*
 * Print program help-message
 */
static void
Help(void)
{
    static const char *const tbl_1[] =
    {
	"cdialog (ComeOn Dialog!) version %s",
	"Copyright (C) 2005 Thomas E. Dickey",
	"This is free software; see the source for copying conditions.  There is NO",
	"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.",
	"",
	"* Display dialog boxes from shell scripts *",
	"",
	"Usage: %s <options> { --and-widget <options> }",
	"where options are \"common\" options, followed by \"box\" options",
	"",
#ifdef HAVE_RC_FILE
	"Special options:",
	"  [--create-rc \"file\"]",
#endif
	0
    }, *const tbl_3[] =
    {
	"",
	"Auto-size with height and width = 0. Maximize with height and width = -1.",
	"Global-auto-size if also menu_height/list_height = 0.",
	0
    };
    unsigned j, k;

    PrintList(tbl_1);
    fprintf(dialog_state.output, "Common options:\n ");
    for (j = k = 0; j < sizeof(options) / sizeof(options[0]); j++) {
	if ((options[j].pass & 1)
	    && options[j].help != 0) {
	    unsigned len = 6 + strlen(options[j].name) + strlen(options[j].help);
	    k += len;
	    if (k > 75) {
		fprintf(dialog_state.output, "\n ");
		k = len;
	    }
	    fprintf(dialog_state.output, " [--%s%s%s]", options[j].name,
		    *(options[j].help) ? " " : "", options[j].help);
	}
    }
    fprintf(dialog_state.output, "\nBox options:\n");
    for (j = 0; j < sizeof(options) / sizeof(options[0]); j++) {
	if ((options[j].pass & 2) != 0
	    && options[j].help != 0
	    && lookupMode(options[j].code))
	    fprintf(dialog_state.output, "  --%-12s %s\n", options[j].name,
		    options[j].help);
    }
    PrintList(tbl_3);

    dlg_exit(DLG_EXIT_OK);
}

/*
 * "Common" options apply to all widgets more/less.  Most of the common options
 * set values in dialog_vars, a few set dialog_state and a couple write to the
 * output stream.
 */
static int
process_common_options(int argc, char **argv, int offset, bool output)
{
    bool done = FALSE;

    while (offset < argc && !done) {	/* Common options */
	switch (lookupOption(argv[offset], 1)) {
	case o_title:
	    dialog_vars.title = optionString(argv, &offset);
	    break;
	case o_backtitle:
	    dialog_vars.backtitle = optionString(argv, &offset);
	    break;
	case o_separator:
	case o_separate_widget:
	    dialog_state.separate_str = optionString(argv, &offset);
	    break;
	case o_separate_output:
	    dialog_vars.separate_output = TRUE;
	    break;
	case o_colors:
	    dialog_vars.colors = TRUE;
	    break;
	case o_cr_wrap:
	    dialog_vars.cr_wrap = TRUE;
	    break;
	case o_no_collapse:
	    dialog_vars.nocollapse = TRUE;
	    break;
	case o_no_kill:
	    dialog_vars.cant_kill = TRUE;
	    break;
	case o_nocancel:
	    dialog_vars.nocancel = TRUE;
	    break;
	case o_single_quoted:
	    dialog_vars.single_quoted = TRUE;
	    break;
	case o_size_err:
	    dialog_vars.size_err = TRUE;
	    break;
	case o_beep:
	    dialog_vars.beep_signal = TRUE;
	    break;
	case o_beep_after:
	    dialog_vars.beep_after_signal = TRUE;
	    break;
	case o_shadow:
	    dialog_state.use_shadow = TRUE;
	    break;
	case o_defaultno:
	    dialog_vars.defaultno = TRUE;
	    break;
	case o_default_item:
	    dialog_vars.default_item = optionString(argv, &offset);
	    break;
	case o_insecure:
	    dialog_vars.insecure = TRUE;
	    break;
	case o_item_help:
	    dialog_vars.item_help = TRUE;
	    break;
	case o_help_button:
	    dialog_vars.help_button = TRUE;
	    break;
	case o_help_status:
	    dialog_vars.help_status = TRUE;
	    break;
	case o_extra_button:
	    dialog_vars.extra_button = TRUE;
	    break;
	case o_ignore:
	    ignore_unknown = TRUE;
	    break;
	case o_keep_window:
	    dialog_vars.keep_window = TRUE;
	    break;
	case o_no_shadow:
	    dialog_state.use_shadow = FALSE;
	    break;
	case o_print_size:
	    dialog_vars.print_siz = TRUE;
	    break;
	case o_print_maxsize:
	    if (output) {
		/*
		 * If this is the last option, we do not want any error
		 * messages - just our output.  Calling end_dialog() cancels
		 * the refresh() at the end of the program as well.
		 */
		if (argv[offset + 1] == 0) {
		    ignore_unknown = TRUE;
		    end_dialog();
		}
		fflush(dialog_state.output);
		fprintf(dialog_state.output, "MaxSize: %d, %d\n", SLINES, SCOLS);
	    }
	    break;
	case o_print_version:
	    if (output) {
		fprintf(stdout, "Version: %s\n", dialog_version());
	    }
	    break;
	case o_tab_correct:
	    dialog_vars.tab_correct = TRUE;
	    break;
	case o_sleep:
	    dialog_vars.sleep_secs = optionValue(argv, &offset);
	    break;
	case o_timeout:
	    dialog_vars.timeout_secs = optionValue(argv, &offset);
	    break;
	case o_max_input:
	    dialog_vars.max_input = optionValue(argv, &offset);
	    break;
	case o_tab_len:
	    dialog_state.tab_len = optionValue(argv, &offset);
	    break;
	case o_trim:
	    dialog_vars.trim_whitespace = TRUE;
	    break;
	case o_visit_items:
	    dialog_state.visit_items = TRUE;
	    break;
	case o_aspect:
	    dialog_state.aspect_ratio = optionValue(argv, &offset);
	    break;
	case o_begin:
	    dialog_vars.begin_set = TRUE;
	    dialog_vars.begin_y = optionValue(argv, &offset);
	    dialog_vars.begin_x = optionValue(argv, &offset);
	    break;
	case o_clear:
	    dialog_vars.dlg_clear_screen = TRUE;
	    break;
	case o_yes_label:
	    dialog_vars.yes_label = optionString(argv, &offset);
	    break;
	case o_no_label:
	    dialog_vars.no_label = optionString(argv, &offset);
	    break;
	case o_ok_label:
	    dialog_vars.ok_label = optionString(argv, &offset);
	    break;
	case o_cancel_label:
	    dialog_vars.cancel_label = optionString(argv, &offset);
	    break;
	case o_extra_label:
	    dialog_vars.extra_label = optionString(argv, &offset);
	    break;
	case o_exit_label:
	    dialog_vars.exit_label = optionString(argv, &offset);
	    break;
	case o_help_label:
	    dialog_vars.help_label = optionString(argv, &offset);
	    break;
	case o_noitem:
	case o_fullbutton:
	    /* ignore */
	    break;
	    /* options of Xdialog which we ignore */
	case o_icon:
	case o_wmclass:
	    (void) optionString(argv, &offset);
	    /* FALLTHRU */
	case o_allow_close:
	case o_auto_placement:
	case o_fixed_font:
	case o_keep_colors:
	case o_no_close:
	case o_no_cr_wrap:
	case o_screen_center:
	case o_smooth:
	case o_under_mouse:
	    break;
	case o_unknown:
	    if (ignore_unknown)
		break;
	    /* FALLTHRU */
	default:		/* no more common options */
	    done = TRUE;
	    break;
	}
	if (!done)
	    offset++;
    }
    return offset;
}

/*
 * Initialize options at the start of a series of common options culminating
 * in a widget.
 */
static void
init_result(char *buffer)
{
    static bool first = TRUE;
    static char **special_argv = 0;
    static int special_argc = 0;

    /* clear everything we do not save for the next widget */
    memset(&dialog_vars, 0, sizeof(dialog_vars));

    dialog_vars.input_result = buffer;
    dialog_vars.input_result[0] = '\0';

    /*
     * The first time this is called, check for common options given by an
     * environment variable.
     */
    if (first) {
	char *env = getenv("DIALOGOPTS");
	if (env != 0)
	    env = dlg_strclone(env);
	if (env != 0) {
	    special_argv = string_to_argv(env);
	    special_argc = count_argv(special_argv);
	}
    }
    if (special_argv != 0) {
	process_common_options(special_argc, special_argv, 0, FALSE);
#ifdef NO_LEAKS
	free(special_argv[0]);
	free(special_argv);
	first = TRUE;
#endif
    }
}

int
main(int argc, char *argv[])
{
    FILE *input = stdin;
    char temp[256];
    bool esc_pressed = FALSE;
    int offset = 1;
    int offset_add;
    int retval = DLG_EXIT_OK;
    int j;
    eOptions code;
    const Mode *modePtr;
    char my_buffer[MAX_LEN + 1];

    memset(&dialog_state, 0, sizeof(dialog_state));
    memset(&dialog_vars, 0, sizeof(dialog_vars));

#if defined(ENABLE_NLS)
    /* initialize locale support */
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#elif defined(HAVE_SETLOCALE)
    (void) setlocale(LC_ALL, "");
#endif

    unescape_argv(&argc, &argv);
    program = argv[0];
    dialog_state.output = stderr;

    /*
     * Look for the last --stdout, --stderr or --output-fd option, and use
     * that.  We can only write to one of them.  If --stdout is used, that
     * can interfere with initializing the curses library, so we want to
     * know explicitly if it is used.
     */
    while (offset < argc) {
	int base = offset;
	switch (lookupOption(argv[offset], 7)) {
	case o_stdout:
	    dialog_state.output = stdout;
	    break;
	case o_stderr:
	    dialog_state.output = stderr;
	    break;
	case o_input_fd:
	    if ((j = optionValue(argv, &offset)) < 0
		|| (input = fdopen(j, "r")) == 0)
		dlg_exiterr("Cannot open input-fd\n");
	    break;
	case o_output_fd:
	    if ((j = optionValue(argv, &offset)) < 0
		|| (dialog_state.output = fdopen(j, "w")) == 0)
		dlg_exiterr("Cannot open output-fd\n");
	    break;
	default:
	    ++offset;
	    continue;
	}
	for (j = base; j < argc; ++j) {
	    dialog_argv[j] = dialog_argv[j + 1 + (offset - base)];
	    if (dialog_opts != 0)
		dialog_opts[j] = dialog_opts[j + 1 + (offset - base)];
	}
	argc -= (1 + offset - base);
	offset = base;
    }
    offset = 1;
    init_result(my_buffer);

    if (argc == 2) {		/* if we don't want clear screen */
	switch (lookupOption(argv[1], 7)) {
	case o_print_maxsize:
	    (void) initscr();
	    endwin();
	    fflush(dialog_state.output);
	    fprintf(dialog_state.output, "MaxSize: %d, %d\n", SLINES, SCOLS);
	    break;
	case o_print_version:
	    fprintf(stdout, "Version: %s\n", dialog_version());
	    break;
	case o_clear:
	    initscr();
	    refresh();
	    endwin();
	    break;
	case o_ignore:
	    break;
	default:
	case o_help:
	    dialog_state.output = stdout;
	    Help();
	    break;
	}
	return DLG_EXIT_OK;
    }

    if (argc < 2) {
	Help();
    }
#ifdef HAVE_RC_FILE
    if (lookupOption(argv[1], 7) == o_create_rc) {
	if (argc != 3) {
	    sprintf(temp, "Expected a filename for %.50s", argv[1]);
	    Usage(temp);
	}
	if (dlg_parse_rc() == -1)	/* Read the configuration file */
	    dlg_exiterr("dialog: dlg_parse_rc");
	dlg_create_rc(argv[2]);
	return DLG_EXIT_OK;
    }
#endif

    init_dialog(input, dialog_state.output);

    while (offset < argc && !esc_pressed) {
	init_result(my_buffer);

	offset = process_common_options(argc, argv, offset, TRUE);

	if (argv[offset] == NULL) {
	    if (ignore_unknown)
		break;
	    Usage("Expected a box option");
	}

	for (j = offset; j < argc; j++) {
	    if (offset > 0) {
		switch (lookupOption(argv[j - 1], 7)) {
		case o_unknown:
		case o_title:
		case o_backtitle:
		    break;
		default:
		    dlg_trim_string(argv[j]);
		    break;
		}
	    }
	}

	if (lookupOption(argv[offset], 2) != o_checklist
	    && dialog_vars.separate_output) {
	    sprintf(temp, "Expected --checklist, not %.20s", argv[offset]);
	    Usage(temp);
	}

	if (dialog_state.aspect_ratio == 0)
	    dialog_state.aspect_ratio = DEFAULT_ASPECT_RATIO;

	dlg_put_backtitle();

	/* use a table to look for the requested mode, to avoid code duplication */

	modePtr = 0;
	if ((code = lookupOption(argv[offset], 2)) != o_unknown)
	    modePtr = lookupMode(code);
	if (modePtr == 0) {
	    sprintf(temp, "%s option %.20s",
		    lookupOption(argv[offset], 7) != o_unknown
		    ? "Unexpected"
		    : "Unknown",
		    argv[offset]);
	    Usage(temp);
	}

	if (arg_rest(&argv[offset]) < modePtr->argmin) {
	    sprintf(temp, "Expected at least %d tokens for %.20s, have %d",
		    modePtr->argmin - 1, argv[offset],
		    arg_rest(&argv[offset]) - 1);
	    Usage(temp);
	}
	if (modePtr->argmax && arg_rest(&argv[offset]) > modePtr->argmax) {
	    sprintf(temp,
		    "Expected no more than %d tokens for %.20s, have %d",
		    modePtr->argmax - 1, argv[offset],
		    arg_rest(&argv[offset]) - 1);
	    Usage(temp);
	}

	retval = show_result((*(modePtr->jumper)) (dialog_vars.title,
						   argv + offset,
						   &offset_add));
	offset += offset_add;

	if (dialog_vars.input_result != my_buffer)
	    free(dialog_vars.input_result);

	if (retval == DLG_EXIT_ESC) {
	    esc_pressed = TRUE;
	} else {

	    if (dialog_vars.beep_after_signal)
		(void) beep();

	    if (dialog_vars.sleep_secs)
		(void) napms(dialog_vars.sleep_secs * 1000);

	    if (offset < argc) {
		switch (lookupOption(argv[offset], 7)) {
		case o_and_widget:
		    offset++;
		    break;
		case o_unknown:
		    sprintf(temp, "Expected --and-widget, not %.20s",
			    argv[offset]);
		    Usage(temp);
		    break;
		default:
		    /* if we got a cancel, etc., stop chaining */
		    if (retval != DLG_EXIT_OK)
			esc_pressed = TRUE;
		    else
			dialog_vars.dlg_clear_screen = TRUE;
		    break;
		}
	    }
	    if (dialog_vars.dlg_clear_screen)
		dlg_clear();
	}
    }

    dlg_killall_bg(&retval);
    if (dialog_state.screen_initialized) {
	(void) refresh();
	end_dialog();
    }
    dlg_exit(retval);
}
