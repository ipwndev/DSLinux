/*
 * $Id: replimenu.h,v 1.5 2004/01/06 03:51:57 shadow Exp $
 *
 * replimenu header-file
 */

/*
 * $Log: replimenu.h,v $
 * Revision 1.5  2004/01/06 03:51:57  shadow
 * version 0.9, hopefully
 *
 * Revision 1.4  2004/01/05 03:45:06  shadow
 * fixed the flickery menu with a dirty hack, generated a bug that I had to fix and added a new feature: multi-line {input,msg,yesno}-boxes.
 *
 * Revision 1.3  2004/01/04 03:11:15  shadow
 * making version 0.9, currently improved the flickery browsing
 *
 * Revision 1.2  2003/12/21 09:56:20  shadow
 * added CVS keywords to most source files
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <errno.h>
extern int errno;
extern char *optarg;
extern int optind, opterr, optopt;

extern void nullify_surrounding_spaces(char *);
extern char **separate_string(char *);

extern unsigned int readkey(void);


#define box_YES 0
#define box_NO	1
#define box_CANCEL 2


/*
 * ECMA-48 Control Sequence Introducer maxlength.
 */
#define NPAR 16

/*
 * Return codes for readkey().
 */
#define key_UP			0x10000000
#define key_DOWN		0x10000001
#define key_LEFT		0x10000002
#define key_RIGHT		0x10000003
#define key_PGUP		0x10000004
#define key_PGDOWN		0x10000005
#define key_HOME		0x10000006
#define key_END			0x10000007
#define key_INSERT		0x10000008
#define key_DELETE		0x10000009

#define key_SHIFTUP		0x1000000A
#define key_SHIFTDOWN		0x1000000B
#define key_SHIFTLEFT		0x1000000C
#define key_SHIFTRIGHT		0x1000000D
#define key_CTRLUP		0x1000000E
#define key_CTRLDOWN		0x1000000F
#define key_CTRLLEFT		0x10000010
#define key_CTRLRIGHT		0x10000011

#define key_F1			0x10000012
#define key_F2			0x10000013
#define key_F3			0x10000014
#define key_F4			0x10000015
#define key_F5			0x10000016
#define key_F6			0x10000017
#define key_F7			0x10000018
#define key_F8			0x10000019
#define key_F9			0x1000001A
#define key_F10			0x1000001B
#define key_F11			0x1000001C
#define key_F12			0x1000001D

#define key_DBLESC		0x1000001E
#define key_NUMPAD5		0x1000001F

#define RETURN 10
#define SPACE 32
#define BACKSPACE1 8
#define BACKSPACE2 0x7f


/*
 * ECMA-48 SGR (Set Graphics Rendition) sequenses
 *
 * Matte fg colors.
 */
#define RESETGR "\033[0m"
#define RED "\033[0;31m"  
#define GREEN "\033[0;32m"
#define BROWN "\033[0;33m"
#define BLUE "\033[0;34m"
#define DEEPPURPLE "\033[0;35m"
#define CYAN "\033[0;36m"
#define GREY "\033[0;37m"
/*
 * Bright fg colors.
 */
#define DARKGREY "\033[1;30m"
#define BRIGHTRED "\033[1;31m"
#define BRIGHTGREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BRIGHTBLUE "\033[1;34m"
#define PINK "\033[1;35m"
#define BRIGHTCYAN "\033[1;36m"
#define WHITE "\033[1;37m"
/*
 * Bg colors.
 */
#define BLACK_BG "\033[40m"
#define RED_BG "\033[41m"  
#define GREEN_BG "\033[42m"
#define BROWN_BG "\033[43m"
#define BLUE_BG "\033[44m"
#define DEEPPURPLE_BG "\033[45m"
#define CYAN_BG "\033[46m"
#define GREY_BG "\033[47m"
/*
 * Text attributes.
 */
#define UNDERSCORE "\033[4m"
#define BLINK "\033[5m"
#define INVERSE "\033[7m"
#define CONCEALED "\033[8m"

/*
 * Cursor and display manipulation.
 */

#define ERASE_DISPLAY "\033[2J"
#define CLS "\033[1;1H\033[2J"
#define DISABLE_BLANKING "\033[9;0]"
#define MOVE_CURSOR_UP_ONE "\033[1A"
#define MOVE_CURSOR_DOWN_ONE "\033[1B"
#define MOVE_CURSOR_RIGHT_ONE "\033[1C"
#define MOVE_CURSOR_LEFT_ONE "\033[1D"


/*
 * Data definitions.
 */

struct termios oldtty, newtty;

const char copyright[] = "replimenu " VERSIONSTRING " Copyright (C) 2003,2004 Michel Blomgren\nhttp://replimenu.sf.net";
const char bottomrowmsg[] = "replimenu " VERSIONSTRING;

unsigned int allowquit = 1;

unsigned int colorscheme = 0;
int dontsetcolorscheme = 0;
char *backgroundcolor = NULL;

char *text = NULL;
char *aftertext = NULL;

unsigned int indent;
unsigned int textindent = 2;
unsigned int aftertextindent = 2;
unsigned int nroflines_text = 0;
unsigned int nroflines_aftertext = 0;

char *readfromfile = NULL;
FILE *menufp;

char defaultcaption[] = "My Menu";
char *caption = defaultcaption;

char *runonexit = NULL;
int docls = 1;
int exitafterauto = 0;
int exitafterauto_overrider = 0;

unsigned int position = 0;
unsigned int min_position;
unsigned int max_position;

unsigned int longestItem = 0;
unsigned int longestBullet = 0;
unsigned int maxmenuitemstodisplay = 0;
unsigned int nrwrappedlines = 0;
struct winsize wsize;

unsigned int forced_width = 0;
unsigned int forced_height = 0;

#define uem scratch
char uem[256];

char inputfieldbuf[256];

#define MAX_SETENV 1000
unsigned int setenvcounter = 0;

unsigned int flicker;

/*
 * menustruct "type"-types, these can be OR'd together. If e.g.
 * "RMtype_regular | RMtype_yesno" is specified, RMtype_regular
 * has always precedence and will override any other definition.
 */

#define	RMtype_regular		1	/* "regular", "normal" */
#define	RMtype_yesno		2	/* "yesno", "ask" */
#define RMtype_input		4	/* "input", "inputbox" */
#define RMtype_pause		8	/* "pause" */
#define RMtype_variable		16	/* "variable", "var" */
#define RMtype_dummy		32	/* "dummy" */
#define RMtype_setenvrmitem	64	/* "setenvrmitem", "setenvitem" */
#define RMtype_notempty		128	/* "notempty", "noempty" */
#define RMtype_checkbox		256	/* "checkbox", "option" */
#define RMtype_radiobutton	512	/* "radiobutton", "radio" */
#define RMtype_selected		1024	/* "selected" */
#define RMtype_runonexitfirst	2048	/* "runonexitfirst", "runonexit" */
#define RMtype_runonexitlast	4096	/* "runonexitlast" */
#define RMtype_usecommandretval	8192	/* "usecommandretval" */
#define RMtype_nocls		16384	/* "nocls" */
#define RMtype_hidden           32768   /* "hidden" */
#define RMtype_chain            65536   /* "chain" */
#define RMtype_auto             131072  /* "auto* */
#define RMtype_password         262144  /* "password" */
#define RMtype_msgbox           524288  /* "msgbox" */
#define RMtype_dfenv            1048576 /* "defaultfromenv", "defaultfromenvironment", "dfenv" */

/*
 * The highly essential menuitem structure.
 */
struct menustruct {
	char *name;		/* menu entry's name */
	char *bullet;		/* bullet; this entry's prefix */
	char *label;		/* menuitem's visible text */
        char *icaption;         /* caption to use instead of 'label' for input boxes */
	char *def;		/* default value for an input box */
	char *command;		/* command to execute using system() */
	unsigned int type;	/* menuitem type, e.g. "regular", "yesno", "pause", etc. */
};

/*
 * Menuitem variables.
 */
unsigned int menufile_currentline;
unsigned int menuitem_started = 0;
unsigned int got_end = 1;
void **structs = NULL;
unsigned int menuitems = 0;
unsigned int visiblemenuitems = 0;
unsigned int automenuitems = 0;

int disable_sigwinch = 0;


/*
 * drawBox values for msgboxes, yesnoboxes and inputboxes.
 */
unsigned int drawBox_longestLine, drawBox_numberOfLines, drawBox_boxwidth;
unsigned int drawBox_x, drawBox_y, drawBox_i;
char *drawBox_bgcolor, *drawBox_textcolor, *drawBox_inputcolor;


/*
 * Available colorschemes
 */

char cs00_textcolor[] = "\033[0m";
char cs00_bordercolor[] = INVERSE;
char cs00_itemLineHilited[] = "\033[2K\033[%uC\033[1m%%%us %%s\033[0m";
char cs00_itemLineNormal[] = "\033[2K\033[%uC%%%us %%s\033[0m";
char cs00_itemLineDummy[] = "\033[2K\033[%uC%%s\033[0m";
char cs00_morecolor[] = "\033[1;30m";
char cs00_bgcolor[] = "\033[0m";
char cs00_boxbgcolor[] = "\033[47m";
char cs00_boxtextcolor[] = "\033[30;47m";
char cs00_boxinputcolor[] = "\033[0m";

char cs01_textcolor[] = "\033[37;44m";
char cs01_bordercolor[] = "\033[31;47m";
char cs01_itemLineHilited[] = "\033[2K\033[%uC\033[1;37;41m%%%us %%s\033[37;44m";
char cs01_itemLineNormal[] = "\033[2K\033[%uC\033[1;37m%%%us %%s\033[37;44m";
char cs01_itemLineDummy[] = "\033[2K\033[%uC\033[37m%%s\033[37;44m";
char cs01_morecolor[] = "\033[1;32m";
char cs01_bgcolor[] = "\033[44m";
char cs01_boxbgcolor[] = "\033[43m";
char cs01_boxtextcolor[] = "\033[30;43m";
char cs01_boxinputcolor[] = "\033[30;47m";

char cs02_textcolor[] = "\033[30;47m";
char cs02_bordercolor[] = "\033[1;37;41m";
char cs02_itemLineHilited[] = "\033[%uC\033[34;43m%%%us %%s\033[30;47m";
char cs02_itemLineNormal[] = "\033[%uC\033[34m%%%us %%s\033[30;47m";
char cs02_itemLineDummy[] = "\033[%uC\033[30m%%s\033[30;47m";
char cs02_morecolor[] = "\033[1;37m";
char cs02_bgcolor[] = "\033[47m";
char cs02_boxbgcolor[] = "\033[46m";
char cs02_boxtextcolor[] = "\033[30;46m";
char cs02_boxinputcolor[] = "\033[30;47m";

char cs03_textcolor[] = "\033[30;43m";
char cs03_bordercolor[] = "\033[30;41m";
char cs03_itemLineHilited[] = "\033[2K\033[%uC\033[1;33;40m%%%us %%s\033[30;43m";
char cs03_itemLineNormal[] = "\033[2K\033[%uC\033[1;37m%%%us %%s\033[30;43m";
char cs03_itemLineDummy[] = "\033[2K\033[%uC\033[30m%%s\033[30;43m";
char cs03_morecolor[] = "\033[1;33m";
char cs03_bgcolor[] = "\033[43m";
char cs03_boxbgcolor[] = "\033[42m";
char cs03_boxtextcolor[] = "\033[30;42m";
char cs03_boxinputcolor[] = "\033[30;47m";

char cs04_textcolor[] = "\033[31;47m";
char cs04_bordercolor[] = "\033[1;37;42m";
char cs04_itemLineHilited[] = "\033[2K\033[%uC\033[37;40m%%%us %%s\033[31;47m";
char cs04_itemLineNormal[] = "\033[2K\033[%uC\033[30m%%%us %%s\033[31;47m";
char cs04_itemLineDummy[] = "\033[2K\033[%uC\033[31m%%s\033[31;47m";
char cs04_morecolor[] = "\033[1;37m";
char cs04_bgcolor[] = "\033[47m";
char cs04_boxbgcolor[] = "\033[44m";
char cs04_boxtextcolor[] = "\033[1;37;44m";
char cs04_boxinputcolor[] = "\033[30;47m";

char cs05_textcolor[] = "\033[1;32;42m";
char cs05_bordercolor[] = "\033[1;36;46m";
char cs05_itemLineHilited[] = "\033[2K\033[%uC\033[32;40m%%%us %%s\033[1;32;42m";
char cs05_itemLineNormal[] = "\033[2K\033[%uC\033[30m%%%us %%s\033[1;32;42m";
char cs05_itemLineDummy[] = "\033[2K\033[%uC\033[1;32m%%s\033[1;32;42m";
char cs05_morecolor[] = "\033[1;32m";
char cs05_bgcolor[] = "\033[42m";
char cs05_boxbgcolor[] = "\033[46m";
char cs05_boxtextcolor[] = "\033[1;37;46m";
char cs05_boxinputcolor[] = "\033[30;47m";

char cs06_textcolor[] = "\033[1;37;46m";
char cs06_bordercolor[] = "\033[30;47m";
char cs06_itemLineHilited[] = "\033[2K\033[%uC\033[31;47m%%%us %%s\033[1;37;46m";
char cs06_itemLineNormal[] = "\033[2K\033[%uC\033[30;46m%%%us %%s\033[1;37;46m";
char cs06_itemLineDummy[] = "\033[2K\033[%uC\033[1;37;46m%%s\033[1;37;46m";
char cs06_morecolor[] = "\033[1;36m";
char cs06_bgcolor[] = "\033[46m";
char cs06_boxbgcolor[] = "\033[47m";
char cs06_boxtextcolor[] = "\033[30;47m";
char cs06_boxinputcolor[] = "\033[1;37;40m";

char cs07_textcolor[] = "\033[37;44m";
char cs07_bordercolor[] = "\033[30;47m";
char cs07_itemLineHilited[] = "\033[2K\033[%uC\033[31;47m%%%us %%s\033[37;44m";
char cs07_itemLineNormal[] = "\033[2K\033[%uC\033[37;44m%%%us %%s\033[37;44m";
char cs07_itemLineDummy[] = "\033[2K\033[%uC\033[37;44m%%s\033[37;44m";
char cs07_morecolor[] = "\033[1;34m";
char cs07_bgcolor[] = "\033[44m";
char cs07_boxbgcolor[] = "\033[47m";
char cs07_boxtextcolor[] = "\033[30;47m";
char cs07_boxinputcolor[] = "\033[37;40m";

char cs08_textcolor[] = "\033[32;40m";
char cs08_bordercolor[] = "\033[30;47m";
char cs08_itemLineHilited[] = "\033[2K\033[%uC\033[30;42m%%%us %%s\033[32;40m";
char cs08_itemLineNormal[] = "\033[2K\033[%uC\033[1;32;40m%%%us %%s\033[32;40m";
char cs08_itemLineDummy[] = "\033[2K\033[%uC\033[32;40m%%s\033[32;40m";
char cs08_morecolor[] = "\033[1;30m";
char cs08_bgcolor[] = "\033[40m";
char cs08_boxbgcolor[] = "\033[47m";
char cs08_boxtextcolor[] = "\033[30;47m";
char cs08_boxinputcolor[] = "\033[37;40m";

char cs09_textcolor[] = "\033[37;44m";
char cs09_bordercolor[] = "\033[30;47m";
char cs09_itemLineHilited[] = "\033[2K\033[%uC\033[30;47m%%%us %%s\033[37;44m";
char cs09_itemLineNormal[] = "\033[2K\033[%uC\033[1;37;44m%%%us %%s\033[37;44m";
char cs09_itemLineDummy[] = "\033[2K\033[%uC\033[37;44m%%s\033[37;44m";
char cs09_morecolor[] = "\033[1;34m";
char cs09_bgcolor[] = "\033[44m";
char cs09_boxbgcolor[] = "\033[43m";
char cs09_boxtextcolor[] = "\033[30;43m";
char cs09_boxinputcolor[] = "\033[30;47m";
