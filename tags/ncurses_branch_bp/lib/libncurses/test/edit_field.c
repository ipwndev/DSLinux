/*
 * $Id$
 *
 * A wrapper for form_driver() which keeps track of the user's editing changes
 * for each field, and makes the result available as a null-terminated string
 * in field_buffer(field,1).
 *
 * Thomas Dickey - 2003/4/26.
 */

#include <test.priv.h>

#if USE_LIBFORM

#include <edit_field.h>

#define MY_QUIT		EDIT_FIELD('q')
#define MY_INS_MODE	EDIT_FIELD('t')

static struct {
    int code;
    int result;
    const char *help;
} commands[] = {

    {
	CTRL('A'), REQ_NEXT_CHOICE, ""
    },
    {
	CTRL('B'), REQ_PREV_WORD, "go to previous word"
    },
    {
	CTRL('C'), REQ_CLR_EOL, "clear to end of line"
    },
    {
	CTRL('D'), REQ_DOWN_FIELD, "move downward to field"
    },
    {
	CTRL('E'), REQ_END_FIELD, "go to end of field"
    },
    {
	CTRL('F'), REQ_NEXT_PAGE, "go to next page"
    },
    {
	CTRL('G'), REQ_DEL_WORD, "delete current word"
    },
    {
	CTRL('H'), REQ_DEL_PREV, "delete previous character"
    },
    {
	CTRL('I'), REQ_INS_CHAR, "insert character"
    },
    {
	CTRL('K'), REQ_CLR_EOF, "clear to end of field"
    },
    {
	CTRL('L'), REQ_LEFT_FIELD, "go to field to left"
    },
    {
	CTRL('M'), REQ_NEW_LINE, "insert/overlay new line"
    },
    {
	CTRL('N'), REQ_NEXT_FIELD, "go to next field"
    },
    {
	CTRL('O'), REQ_INS_LINE, "insert blank line at cursor"
    },
    {
	CTRL('P'), REQ_PREV_FIELD, "go to previous field"
    },
    {
	CTRL('Q'), MY_QUIT, "exit form"
    },
    {
	CTRL('R'), REQ_RIGHT_FIELD, "go to field to right"
    },
    {
	CTRL('S'), REQ_BEG_FIELD, "go to beginning of field"
    },
    {
	CTRL('U'), REQ_UP_FIELD, "move upward to field"
    },
    {
	CTRL('V'), REQ_DEL_CHAR, "delete character"
    },
    {
	CTRL('W'), REQ_NEXT_WORD, "go to next word"
    },
    {
	CTRL('X'), REQ_CLR_FIELD, "clear field"
    },
    {
	CTRL('Y'), REQ_DEL_LINE, "delete line"
    },
    {
	CTRL('Z'), REQ_PREV_CHOICE, ""
    },
    {
	CTRL('['), MY_QUIT, "exit form"
    },
    {
	CTRL(']'), MY_INS_MODE, "toggle REQ_INS_MODE/REQ_OVL_MODE",
    },
    {
	KEY_F(1), EDIT_FIELD('h'), "show this screen",
    },
    {
	KEY_BACKSPACE, REQ_DEL_PREV, "delete previous character"
    },
    {
	KEY_DOWN, REQ_DOWN_CHAR, "move down 1 character"
    },
    {
	KEY_END, REQ_LAST_FIELD, "go to last field"
    },
    {
	KEY_HOME, REQ_FIRST_FIELD, "go to first field"
    },
    {
	KEY_LEFT, REQ_LEFT_CHAR, "move left 1 character"
    },
    {
	KEY_LL, REQ_LAST_FIELD, "go to last field"
    },
    {
	KEY_NEXT, REQ_NEXT_FIELD, "go to next field"
    },
    {
	KEY_NPAGE, REQ_NEXT_PAGE, "go to next page"
    },
    {
	KEY_PPAGE, REQ_PREV_PAGE, "go to previous page"
    },
    {
	KEY_PREVIOUS, REQ_PREV_FIELD, "go to previous field"
    },
    {
	KEY_RIGHT, REQ_RIGHT_CHAR, "move right 1 character"
    },
    {
	KEY_UP, REQ_UP_CHAR, "move up 1 character"
    }
};

static WINDOW *old_window;

static void
begin_popup(void)
{
    doupdate();
    old_window = dupwin(curscr);
}

static void
end_popup(void)
{
    touchwin(old_window);
    wnoutrefresh(old_window);
    doupdate();
    delwin(old_window);
}

/*
 * Display a temporary window listing the keystroke-commands we recognize.
 */
void
help_edit_field(void)
{
    int x0 = 4;
    int y0 = 2;
    int y1 = 0;
    int y2 = 0;
    int wide = COLS - ((x0 + 1) * 2);
    int high = LINES - ((y0 + 1) * 2);
    WINDOW *help = newwin(high, wide, y0, x0);
    WINDOW *data = newpad(2 + SIZEOF(commands), wide - 4);
    unsigned n;
    int ch = ERR;

    begin_popup();

    keypad(help, TRUE);
    keypad(data, TRUE);
    waddstr(data, "Defined form edit/traversal keys:\n");
    for (n = 0; n < SIZEOF(commands); ++n) {
	const char *name;
#ifdef NCURSES_VERSION
	if ((name = form_request_name(commands[n].result)) == 0)
#endif
	    name = commands[n].help;
	wprintw(data, "%s -- %s\n",
		keyname(commands[n].code),
		name != 0 ? name : commands[n].help);
    }
    waddstr(data, "Arrow keys move within a field as you would expect.");
    y2 = getcury(data);

    do {
	switch (ch) {
	case CTRL('P'):
	case KEY_UP:
	    if (y1 > 0)
		--y1;
	    else
		beep();
	    break;
	case CTRL('N'):
	case KEY_DOWN:
	    if (y1 < y2)
		++y1;
	    else
		beep();
	    break;
	default:
	    beep();
	    break;
	case ERR:
	    break;
	}
	werase(help);
	box(help, 0, 0);
	wnoutrefresh(help);
	pnoutrefresh(data, y1, 0, y0 + 1, x0 + 1, high, wide);
	doupdate();
    } while ((ch = wgetch(data)) != ERR && ch != QUIT && ch != ESCAPE);
    werase(help);
    wrefresh(help);
    delwin(help);
    delwin(data);

    end_popup();
}

static int
offset_in_field(FORM * form)
{
    FIELD *field = current_field(form);
    return form->curcol + form->currow * field->dcols;
}

int
edit_field(FORM * form, int *result)
{
    int ch = wgetch(form_win(form));
    int status;
    FIELD *before;
    FIELD *after;
    unsigned n;
    char lengths[80];
    int length;
    char *buffer;
    int before_row = form->currow;
    int before_col = form->curcol;
    int before_off = offset_in_field(form);

    before = current_field(form);
    set_field_back(before, A_NORMAL);
    if (ch <= KEY_MAX) {
	set_field_back(before, A_REVERSE);
    } else if (ch <= MAX_FORM_COMMAND) {
	set_field_back(before, A_UNDERLINE);
    }

    *result = ch;
    for (n = 0; n < SIZEOF(commands); ++n) {
	if (commands[n].code == ch) {
	    *result = commands[n].result;
	    break;
	}
    }

    status = form_driver(form, *result);

    if (status == E_OK) {
	bool modified = TRUE;

	length = 0;
	if ((buffer = field_buffer(before, 1)) != 0)
	    length = atoi(buffer);
	if (length < before_off)
	    length = before_off;
	switch (*result) {
	case REQ_CLR_EOF:
	    length = before_off;
	    break;
	case REQ_CLR_EOL:
	    if (before_row + 1 == before->rows)
		length = before_off;
	    break;
	case REQ_CLR_FIELD:
	    length = 0;
	    break;
	case REQ_DEL_CHAR:
	    if (length > before_off)
		--length;
	    break;
	case REQ_DEL_PREV:
	    if (length > 0) {
		if (before_col > 0) {
		    --length;
		} else if (before_row > 0) {
		    length -= before->cols + before_col;
		}
	    }
	    break;
	case REQ_NEW_LINE:
	    length += before->cols;
	    break;
#if 0
	    /* FIXME: finish these */
	case REQ_DEL_LINE:	/* delete line */
	case REQ_DEL_WORD:	/* delete word at cursor */
	case REQ_INS_CHAR:	/* insert blank char at cursor */
	case REQ_INS_LINE:	/* insert blank line at cursor */
	case REQ_INS_MODE:	/* begin insert mode */
	case REQ_OVL_MODE:	/* begin overlay mode */
#endif
	    /* ignore all of the motion commands */
	case REQ_SCR_BCHAR:	/* FALLTHRU */
	case REQ_SCR_BHPAGE:	/* FALLTHRU */
	case REQ_SCR_BLINE:	/* FALLTHRU */
	case REQ_SCR_BPAGE:	/* FALLTHRU */
	case REQ_SCR_FCHAR:	/* FALLTHRU */
	case REQ_SCR_FHPAGE:	/* FALLTHRU */
	case REQ_SCR_FLINE:	/* FALLTHRU */
	case REQ_SCR_FPAGE:	/* FALLTHRU */
	case REQ_SCR_HBHALF:	/* FALLTHRU */
	case REQ_SCR_HBLINE:	/* FALLTHRU */
	case REQ_SCR_HFHALF:	/* FALLTHRU */
	case REQ_SCR_HFLINE:	/* FALLTHRU */
	case REQ_BEG_FIELD:	/* FALLTHRU */
	case REQ_BEG_LINE:	/* FALLTHRU */
	case REQ_DOWN_CHAR:	/* FALLTHRU */
	case REQ_DOWN_FIELD:	/* FALLTHRU */
	case REQ_END_FIELD:	/* FALLTHRU */
	case REQ_END_LINE:	/* FALLTHRU */
	case REQ_FIRST_FIELD:	/* FALLTHRU */
	case REQ_FIRST_PAGE:	/* FALLTHRU */
	case REQ_LAST_FIELD:	/* FALLTHRU */
	case REQ_LAST_PAGE:	/* FALLTHRU */
	case REQ_LEFT_CHAR:	/* FALLTHRU */
	case REQ_LEFT_FIELD:	/* FALLTHRU */
	case REQ_NEXT_CHAR:	/* FALLTHRU */
	case REQ_NEXT_CHOICE:	/* FALLTHRU */
	case REQ_NEXT_FIELD:	/* FALLTHRU */
	case REQ_NEXT_LINE:	/* FALLTHRU */
	case REQ_NEXT_PAGE:	/* FALLTHRU */
	case REQ_NEXT_WORD:	/* FALLTHRU */
	case REQ_PREV_CHAR:	/* FALLTHRU */
	case REQ_PREV_CHOICE:	/* FALLTHRU */
	case REQ_PREV_FIELD:	/* FALLTHRU */
	case REQ_PREV_LINE:	/* FALLTHRU */
	case REQ_PREV_PAGE:	/* FALLTHRU */
	case REQ_PREV_WORD:	/* FALLTHRU */
	case REQ_RIGHT_CHAR:	/* FALLTHRU */
	case REQ_RIGHT_FIELD:	/* FALLTHRU */
	case REQ_SFIRST_FIELD:	/* FALLTHRU */
	case REQ_SLAST_FIELD:	/* FALLTHRU */
	case REQ_SNEXT_FIELD:	/* FALLTHRU */
	case REQ_SPREV_FIELD:	/* FALLTHRU */
	case REQ_UP_CHAR:	/* FALLTHRU */
	case REQ_UP_FIELD:	/* FALLTHRU */
	case REQ_VALIDATION:	/* FALLTHRU */
	    modified = FALSE;
	    break;

	default:
	    modified = FALSE;
	    if (ch >= MIN_FORM_COMMAND) {
		beep();
	    } else if (isprint(ch)) {
		modified = TRUE;
	    }
	    break;
	}

	/*
	 * If we do not force a re-validation, then field_buffer 0 will
	 * be lagging by one character.
	 */
	if (modified && form_driver(form, REQ_VALIDATION) == E_OK && *result
	    < MIN_FORM_COMMAND)
	    ++length;

	sprintf(lengths, "%d", length);
	set_field_buffer(before, 1, lengths);
    }

    if ((after = current_field(form)) != before)
	set_field_back(before, A_UNDERLINE);
    return status;
}
#else

extern void no_edit_field(void);

void
no_edit_field(void)
{
}

#endif
