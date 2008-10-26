/*
 * $Id: functions.h,v 1.6 2004/01/06 03:51:57 shadow Exp $
 *
 * Functions for replimenu
 *
 */

/*
 * $Log: functions.h,v $
 * Revision 1.6  2004/01/06 03:51:57  shadow
 * version 0.9, hopefully
 *
 * Revision 1.5  2004/01/05 03:45:06  shadow
 * fixed the flickery menu with a dirty hack, generated a bug that I had to fix and added a new feature: multi-line {input,msg,yesno}-boxes.
 *
 * Revision 1.4  2004/01/04 03:11:15  shadow
 * making version 0.9, currently improved the flickery browsing
 *
 * Revision 1.3  2003/12/27 00:10:41  shadow
 * fixed multiline string to gcc33 compliant string
 *
 * Revision 1.2  2003/12/21 09:56:20  shadow
 * added CVS keywords to most source files
 *
 */

/*****************************************************************************/

/* Maximum number of bytes a line can hold (before truncated) in the
 * foreachline()-function.
 */
#ifndef foreachline_max_linebuf_bytes
	#define foreachline_max_linebuf_bytes 1024*4
#endif

void foreachline(char *msg, int(*function)(char *line)) {
/*
 *	Executes a user-specified function for each line in a string.
 *	If the "function" returns (int) 0 it will continue to call it.
 *	If the "function" return != 0, then it will stop.
 *
 *	foreachline() handles UNIX, DOS and/or Macintosh text files.
 *
 *	e.g.:
 *
 *		int myfunction(char *line) {
 *			printf("%s\n", line);	// echo the line to stdout.
 *			return 0;
 *		}
 *
 *		...
 *
 *		foreachline(stdin, myfunction);
 *
 */
	char linebuf[foreachline_max_linebuf_bytes];
	unsigned int lastchar = 0;
	int i = 0;
	int fel_truncate = 0;
	unsigned int x = 0;

	if (msg == NULL)
		return;

	linebuf[0] = 0;

	while (1) {
		if (msg[x] == 0) {
			if (strlen(linebuf)) {
				function(linebuf);
			}
			break;
		}

		if (msg[x] == 0x0a || msg[x] == 0x0d) {
			if ((msg[x] == 0x0a && lastchar == 0x0d) || (msg[x] == 0x0d && lastchar == 0x0a)) {
				/* nop */
			}
			else {
				if (!fel_truncate) {
					if (function(linebuf))
						break;
					linebuf[0] = 0;
				}
			}

			fel_truncate = 0;
			i = 0;
		}
		else {
			if (!fel_truncate) {
				linebuf[i] = msg[x];
				linebuf[i+1] = 0;
				i++;

				if (i >= (sizeof(linebuf)-1)) {
					if (function(linebuf))
						break;
					linebuf[0] = 0;
					fel_truncate = 1;
				}
			}
		}

		lastchar = msg[x];
		x++;
	}
}

/*****************************************************************************/

void file_foreachline(FILE *strm, int(*function)(char *line, FILE *strm)) {
/*
 *	Executes a user-specified function for each line in a stream.
 *	If the "function" returns (int) 0 it will continue to call it.
 *	If the "function" return != 0, then it will stop.
 *
 *	foreachline() handles UNIX, DOS and/or Macintosh text files.
 *
 *	e.g.:
 *
 *		int myfunction(char *line, FILE *strm)
 *		{
 *			printf("%s\n", line);	// echo the line to stdout.
 *			return 0;
 *		}
 *
 *		...
 *
 *		foreachline(stdin, myfunction);
 *
 */
	char charbuf[1];
	char linebuf[foreachline_max_linebuf_bytes];
	unsigned int lastchar = 0;
	int i = 0;
	int fel_truncate = 0;

	linebuf[0] = 0;

	while (1) {
		fread(charbuf, 1, 1, strm);

		if (feof(strm)) {
			if (strlen(linebuf)) {
				function(linebuf, strm);
			}
			break;
		}

		if (ferror(strm)) {
			break;
		}

		if (charbuf[0] == 0x0a || charbuf[0] == 0x0d) {
			if ((charbuf[0] == 0x0a && lastchar == 0x0d) || (charbuf[0] == 0x0d && lastchar == 0x0a)) {
				/* nop */
			}
			else {
				if (!fel_truncate) {
					if (function(linebuf, strm))
						break;
					linebuf[0] = 0;
				}
			}

			fel_truncate = 0;
			i = 0;
		}
		else {
			if (!fel_truncate) {
				linebuf[i] = charbuf[0];
				linebuf[i+1] = 0;
				i++;

				if (i >= (sizeof(linebuf)-1)) {
					if (function(linebuf, strm))
						break;
					linebuf[0] = 0;
					fel_truncate = 1;
				}
			}
		}

		lastchar = charbuf[0];
	}
}

/*****************************************************************************/

void replacechar(char *string, unsigned int from, unsigned int to) {
/*
 * strips a null-terminated string from char 'from' and replaces it with 'to'.
 *
 * example usage:
 *
 *	char string[] = "TEST\nSTRÄNG";
 *
 *	replacechar(string, 0x0a, 0x20);
 *
 */
	char *dest = string;

	unsigned int x = 0;
	unsigned int i;

	if (string == NULL)
		return;

	if (string[0] == 0)
		return;

	for (i = 0; string[i] != 0; i++) {
		if (string[i] == from) {
			dest[x] = to;
			x++;
		}
		else {
			dest[x] = string[i];
			x++;
		}
	}

	dest[x] = 0;
}

/*****************************************************************************/

void replacestringwithchar(char *string, char *from, unsigned int to) {
/*
 * strips a null-terminated string from char 'from' and replaces it with 'to'.
 *
 * example usage:
 *
 *	char string[] = "TEST\nSTRÄNG";
 *
 *	replacechar(string, 0x0a, 0x20);
 *
 */
	char *dest = string;

	unsigned int x = 0;
	unsigned int i;

	if (string == NULL)
		return;
	if (from == NULL)
		return;

	if ((string[0] == 0) || (from[0] == 0))
		return;

	for (i = 0; string[i] != 0; i++) {
		if (!strncmp(&string[i], from, strlen(from))) {
			dest[x] = to;
			x++;
			i += strlen(from) - 1;
		}
		else {
			dest[x] = string[i];
			x++;
		}
	}

	dest[x] = '\0';
}

/*****************************************************************************/

void removechar(char *string, unsigned int c) {
/*
 * strips a null-terminated string from char c.
 *
 * example usage:
 *
 *	char string[] = "TEST\nSTRÄNG";
 *
 *	removechar(string, 0x0a);
 *
 */
	char *dest = string;

	unsigned int x = 0;
	unsigned int i;

	for (i = 0; string[i] != 0; i++) {
		if (string[i] != c) {
			dest[x] = string[i]; x++;
		}
	}
	dest[x] = '\0';
}

/*****************************************************************************/

void printhelp() {

	printf("%s\n"
"A small menu-system.\n"
"\n"
"  -f menufile   The only obligatory option. menufile is the name of a\n"
"                menu configuration file. See the man page on how to\n"
"                write one.\n"
"\n"
"  -c n          n is a number from 0 to 9 (currently available colour-\n"
"                schemes), this will override the menufile's colour-\n"
"                scheme (if any).\n"
"\n"
"  -q            This option will prevent the user from quitting\n"
"                replimenu, not even a SIGTERM/SIGINT will quit.\n"
"                However, a menu item named QUIT will still quit.\n"
"\n"
"  -g WxH        Force a geometry. This will prevent the ioctl() TIOCGWINSZ\n"
"                request to figure out the current terminal size. It's useful\n"
"                for, e.g. telnet sessions that don't DO NAWS. E.g.: -g 80x25\n"
"\n"
"  -a item_names...\n"
"                A comma (,) separated list of menu item names to automatically\n"
"                execute on start-up, e.g.: replimenu -f fu.bar -a fubar,snafu\n"
"\n"
"  -e i          Set the `exitafterauto' flag. Can be used with the -a option.\n"
"                This option overrides the `exitafterauto' variable in the\n"
"                menufile. E.g.: replimenu -f fu.bar -a fubar,snafu -e1\n"
"\n"
"  -V            Print version and exit immediately.\n"
"\n", copyright);

}

/*****************************************************************************/

void die(void) {
	printf("\033[0m\033[2J\033[1;1H");
	fflush(stdout);

	fprintf(stderr, "%s\n", uem);

	/* enable echoing */
	tcsetattr(STDIN_FILENO, TCSANOW, &oldtty);

	exit(1);
}

/*****************************************************************************/

char *getarg(char *unixstr, unsigned int delim, unsigned int arg)
/*
 * getarg() return an argument in a string separated with char 'delim'. The
 * returned string should be free()'d.
 *
 * Returns NULL if argument number 'arg' was not found or malloc() didn't
 * behave, else it returns a free()-able ascii-zero string with the argument.
 *
 * examples:
 *
 *	char teststring[] = "hello:world:wassup";
 *	char *pointer;
 *
 *	pointer = getarg(teststring, ':', 2);
 *
 *	printf("%s\n", pointer);	// will return "world"
 *	if (pointer != NULL)		// just in case
 *		free(pointer);
 *	...
 */
{
	unsigned int i = 0;

	char *start;
	char *end;
	unsigned int length;

	char *memp;

	if (unixstr == NULL)
		return NULL;

	if (arg)
		arg--;

	/*
	 * Find the argument we want...
	 */
	while (arg)
	{
		while (unixstr[i] != delim)
		{
			if (unixstr[i] == 0)
				return NULL;
			i++;
		}
		i++;
		arg--;
	}

	/*
	 * Locate the end, then malloc some memory for the string...
	 */
	start = &unixstr[i];

	i = 0;

	while (start[i] != delim && start[i] != 0)
		i++;

	end = &start[i];
	length = end - start;

	if ((memp = malloc(length+1)) == NULL)
		return NULL;

	strncpy(memp, start, length);
	memp[length] = 0;

	return memp;
}

/*****************************************************************************/

int addtomenu(struct menustruct *yourMenustruct) {
	struct menustruct *tempstruct;

	if ((structs = realloc(structs, sizeof(structs)*(menuitems+2))) == NULL)
            return 1;

	if ((structs[menuitems] = malloc(sizeof(struct menustruct))) == NULL)
            return 1;
        structs[menuitems+1] = NULL;

	tempstruct = structs[menuitems];

	if ((tempstruct->name = strdup(yourMenustruct->name)) == NULL) {
		return 1;
	}
	if ((tempstruct->bullet = strdup(yourMenustruct->bullet)) == NULL) {
		return 1;
	}
	if ((tempstruct->label = strdup(yourMenustruct->label)) == NULL) {
		return 1;
	}
	if ((tempstruct->icaption = strdup(yourMenustruct->icaption)) == NULL) {
		return 1;
	}
	if ((tempstruct->def = strdup(yourMenustruct->def)) == NULL) {
		return 1;
	}

	if (yourMenustruct->command != NULL) {
		if ((tempstruct->command = strdup(yourMenustruct->command)) == NULL) {
			return 1;
		}
	}
	else {
		tempstruct->command = NULL;
	}

	tempstruct->type = yourMenustruct->type;

        if (!(tempstruct->type & RMtype_hidden)) {
            visiblemenuitems++;
        }
        menuitems++;
	return 0;
}

/*****************************************************************************/

int wraptext(char *line) {

    unsigned int i, x;
    unsigned int y = indent;

    /* TABs have already been replaced with spaces */
    /* replacechar(line, 0x09, 0x20); */

    if ((strlen(line)+(y+1)) > wsize.ws_col) {
	i = wsize.ws_col-(y+1);
	while (line[i] != 0x20) {
		i--;
		if (!i)
			return 1;
	}
	if (y)
		printf("\033[%uC", y);
	for (x = 0; i; x++) {
		putchar(line[x]);
		i--;
	}
	printf("\n");
	wraptext(&line[x+1]); /* be recursive */
    }
    else {
	if (!y)
	    printf("%s\n", line);
	else
	    printf("\033[%uC%s\n", y, line);
    }
    return 0;
}

/*****************************************************************************/

int countwrappedlines(char *line) {

	unsigned int i, x;
	unsigned int y = indent;

	if ((strlen(line)+(y+1)) > wsize.ws_col) {
		/* below code resolves a bug present in all previous versions */
		if ((wsize.ws_col-(y+1)) > 0)
			i = wsize.ws_col-(y+1);
                else
                	i = 0;

		while (line[i] != 0x20) {
			if (i)	/* this was a bug before, logical but hard to find :) */
                        	i--;
			if (!i)
				return 1;
		}
		for (x = 0; i; x++) {
			i--;
		}
		nrwrappedlines++;
		countwrappedlines(&line[x+1]); /* be recursive */
	}
	else {
		nrwrappedlines++;
	}
	return 0;
}

/*****************************************************************************/

int menufilelinehandler(char *line, FILE *strm) {

	unsigned int i;

	char starter01[] = "begin";
	char starter02[] = "start";
	char starter03[] = "define";

	char definer_menuitem01[] = "menuitem";
	char definer_menuitem02[] = "item";

	char end01[] = "end";
	char end02[] = "stop";

	char **divided;
	char *tempp;

	static struct menustruct tempstruct;


	menufile_currentline++;

	if ((line[0] == '#') || (line[0] == 0))
		return 0;

	if ((divided = separate_string(line)) == NULL)
		return 0;

	if (divided[0][0] == 0)
		return 0;

	if (menuitem_started) {
		/* somone has previously started to define a menuitem-structure */
		if (!strcasecmp(divided[0], "name")) {
			free(tempstruct.name);
			if ((tempstruct.name = strdup(divided[1])) == NULL) {
				fprintf(stderr, "line %u: strdup() puked - %s!\n", menufile_currentline, strerror(errno));
				exit(1);
			}
		}
		else if (!strcasecmp(divided[0], "bullet")) {
			free(tempstruct.bullet);
			if ((tempstruct.bullet = strdup(divided[1])) == NULL) {
				fprintf(stderr, "line %u: strdup() puked - %s!\n", menufile_currentline, strerror(errno));
				exit(1);
			}
		}
		else if (!strcasecmp(divided[0], "label")) {
			free(tempstruct.label);
			if ((tempstruct.label = strdup(divided[1])) == NULL) {
				fprintf(stderr, "line %u: strdup() puked - %s!\n", menufile_currentline, strerror(errno));
				exit(1);
			}
		}
		else if (!strcasecmp(divided[0], "icaption")) {
			free(tempstruct.icaption);
			if ((tempstruct.icaption = strdup(divided[1])) == NULL) {
				fprintf(stderr, "line %u: strdup() puked - %s!\n", menufile_currentline, strerror(errno));
				exit(1);
			}
		}
		else if (!strcasecmp(divided[0], "default")) {
			free(tempstruct.def);
			if ((tempstruct.def = strdup(divided[1])) == NULL) {
				fprintf(stderr, "line %u: strdup() puked - %s!\n", menufile_currentline, strerror(errno));
				exit(1);
			}
		}
		else if (!strcasecmp(divided[0], "command")) {
			free(tempstruct.command);
			if ((tempstruct.command = strdup(divided[1])) == NULL) {
				fprintf(stderr, "line %u: strdup() puked - %s!\n", menufile_currentline, strerror(errno));
				exit(1);
			}
		}
		else if (!strcasecmp(divided[0], "type")) {
			for (i = 1; (tempp = getarg(divided[1], '|', i)); i++) {
				nullify_surrounding_spaces(tempp);
				if ((!strcasecmp(tempp, "regular")) || (!strcasecmp(tempp, "normal"))) {
					tempstruct.type |= RMtype_regular;
				}
				else if ((!strcasecmp(tempp, "yesno")) || (!strcasecmp(tempp, "ask"))) {
					tempstruct.type |= RMtype_yesno;
				}
				else if ((!strcasecmp(tempp, "input")) || (!strcasecmp(tempp, "inputbox"))) {
					tempstruct.type |= RMtype_input;
				}
				else if (!strcasecmp(tempp, "pause")) {
					tempstruct.type |= RMtype_pause;
				}
				else if ((!strcasecmp(tempp, "variable")) || (!strcasecmp(tempp, "var"))) {
					tempstruct.type |= RMtype_variable;
				}
				else if (!strcasecmp(tempp, "dummy")) {
					tempstruct.type |= RMtype_dummy;
				}
				else if ((!strcasecmp(tempp, "setenvrmitem")) || (!strcasecmp(tempp, "setenvitem"))) {
					tempstruct.type |= RMtype_setenvrmitem;
				}
				else if ((!strcasecmp(tempp, "notempty")) || (!strcasecmp(tempp, "noempty"))) {
					tempstruct.type |= RMtype_notempty;
				}
				else if ((!strcasecmp(tempp, "checkbox")) || (!strcasecmp(tempp, "option"))) {
					tempstruct.type |= RMtype_checkbox;
				}
				else if ((!strcasecmp(tempp, "radiobutton")) || (!strcasecmp(tempp, "radio"))) {
					tempstruct.type |= RMtype_radiobutton;
				}
				else if ((!strcasecmp(tempp, "selected"))) {
					tempstruct.type |= RMtype_selected;
				}
				else if ((!strcasecmp(tempp, "runonexit")) || (!strcasecmp(tempp, "runonexitfirst"))) {
					tempstruct.type |= RMtype_runonexitfirst;
				}
				else if (!strcasecmp(tempp, "runonexitlast")) {
					tempstruct.type |= RMtype_runonexitlast;
				}
				else if (!strcasecmp(tempp, "usecommandretval")) {
					tempstruct.type |= RMtype_usecommandretval;
				}
				else if (!strcasecmp(tempp, "nocls")) {
					tempstruct.type |= RMtype_nocls;
				}
				else if (!strcasecmp(tempp, "hidden")) {
					tempstruct.type |= RMtype_hidden;
				}
				else if (!strcasecmp(tempp, "chain")) {
					tempstruct.type |= RMtype_chain;
				}
				else if ((!strcasecmp(tempp, "auto")) || (!strcasecmp(tempp, "autoexec"))) {
					tempstruct.type |= RMtype_auto;
				}
				else if ((!strcasecmp(tempp, "password")) || (!strcasecmp(tempp, "passwd"))) {
					tempstruct.type |= RMtype_password;
				}
				else if ((!strcasecmp(tempp, "msgbox")) || (!strcasecmp(tempp, "messagebox"))) {
					tempstruct.type |= RMtype_msgbox;
				}
                                else if ((!strcasecmp(tempp, "defaultfromenv")) || (!strcasecmp(tempp, "defaultfromenvironment")) || (!strcasecmp(tempp, "dfenv"))) {
                                        tempstruct.type |= RMtype_dfenv;
                                }
				else {
					fprintf(stderr, "line %u: can't understand \"%s\"!\n", menufile_currentline, tempp);
					exit(1);
				}
				free(tempp);
			}
		}
		else if ((!strcasecmp(divided[0], end01)) || \
			(!strcasecmp(divided[0], end02))) {

                    /* was `default' defined? */

                    if (!strlen(tempstruct.def)) {
                        /* no, it doesn't seem to have been */

                        /* if `defaultfromenv' was set, see if there's an environ value to put in `default' instead... */
                        if (tempstruct.type & RMtype_dfenv) {
                            snprintf(scratch, sizeof(scratch), "RM_%s", tempstruct.name);
                            if ((tempp = getenv(scratch))) {
                                free(tempstruct.def);
                                if ((tempstruct.def = strdup(tempp)) == NULL) {
                                    fprintf(stderr, "strdup() failed on line %d in %s - %s!\n", __LINE__, __FILE__, strerror(errno));
                                    exit(1);
                                }
                            }
                        }
                    }

		    /* fix potentially bad struct members */

		    replacechar(tempstruct.name, 0x0a, 0x20);
		    replacechar(tempstruct.name, 0x0d, 0x20);
		    replacechar(tempstruct.name, 0x09, 0x20);
		    replacechar(tempstruct.name, 0xff, 0x20);

		    replacechar(tempstruct.label, 0x0a, 0x20);
		    replacechar(tempstruct.label, 0x0d, 0x20);
		    replacechar(tempstruct.label, 0x09, 0x20);
		    replacechar(tempstruct.label, 0xff, 0x20);

		    replacechar(tempstruct.icaption, 0x0a, 0x20);
		    replacechar(tempstruct.icaption, 0x0d, 0x20);
		    replacechar(tempstruct.icaption, 0x09, 0x20);
		    replacechar(tempstruct.icaption, 0xff, 0x20);

		    replacechar(tempstruct.def, 0x0a, 0x20);
		    replacechar(tempstruct.def, 0x0d, 0x20);
		    replacechar(tempstruct.def, 0x09, 0x20);
		    replacechar(tempstruct.def, 0xff, 0x20);

		    /* add this structure to the big heap of structs */

		    if (addtomenu(&tempstruct)) {
			    fprintf(stderr, "addtomenu() failed - %s!\n", strerror(errno));
			    exit(1);
		    }


		    /* under certain conditions, set up environment variables */

		    if (tempstruct.type & RMtype_checkbox) {
			    /* if this item is a "checkbox", set up an environment variable */
			    if (tempstruct.type & RMtype_selected) {
				    snprintf(scratch, sizeof(scratch), "RM_%s", tempstruct.name);
				    if (setenv(scratch, tempstruct.def, 1)) {
					    fprintf(stderr, "setenv() puked - %s!\n", strerror(errno));
					    exit(1);
				    }
				    setenvcounter++;
				    if (setenvcounter > MAX_SETENV) {
					    fprintf(stderr, "Security alert: too many calls to setenv(), aborting!\n");
					    exit(1);
				    }
			    }
			    else {
				    snprintf(scratch, sizeof(scratch), "RM_%s", tempstruct.name);
				    unsetenv(scratch);
			    }
		    }
		    else if (tempstruct.type & RMtype_radiobutton) {
			    /* if this item is a "radiobutton", set up an environment variable */
			    if (tempstruct.type & RMtype_selected) {
				    snprintf(scratch, sizeof(scratch), "RM_%s", tempstruct.name);
				    if (setenv(scratch, tempstruct.def, 1)) {
					    fprintf(stderr, "setenv() puked - %s!\n", strerror(errno));
					    exit(1);
				    }
				    setenvcounter++;
				    if (setenvcounter > MAX_SETENV) {
					    fprintf(stderr, "Security alert: too many calls to setenv(), aborting!\n");
					    exit(1);
				    }
			    }
			    else {
				    snprintf(scratch, sizeof(scratch), "RM_%s", tempstruct.name);
				    if (!getenv(scratch)) {
					    if (setenv(scratch, tempstruct.def, 1)) {
						    fprintf(stderr, "setenv() puked - %s!\n", strerror(errno));
						    exit(1);
					    }
					    setenvcounter++;
					    if (setenvcounter > MAX_SETENV) {
						    fprintf(stderr, "Security alert: too many calls to setenv(), aborting!\n");
						    exit(1);
					    }
				    }
			    }
		    }
		    else if (tempstruct.type & RMtype_variable) {

			    /* if this item is defined as a "var", set up an environment variable */

			    snprintf(scratch, sizeof(scratch), "RM_%s", tempstruct.name);
			    if ((tempstruct.type & RMtype_yesno) && (tempstruct.def[0] == 0)) {
				    if (setenv(scratch, "no", 1)) {
					    fprintf(stderr, "setenv() puked - %s!\n", strerror(errno));
					    exit(1);
				    }
				    setenvcounter++;
				    if (setenvcounter > MAX_SETENV) {
					    fprintf(stderr, "Security alert: too many calls to setenv(), aborting!\n");
					    exit(1);
				    }
			    }
			    else {
				    if (setenv(scratch, tempstruct.def, 1)) {
					    fprintf(stderr, "setenv() puked - %s!\n", strerror(errno));
					    exit(1);
				    }
				    setenvcounter++;
				    if (setenvcounter > MAX_SETENV) {
					    fprintf(stderr, "Security alert: too many calls to setenv(), aborting!\n");
					    exit(1);
				    }
			    }
		    }


		    got_end = 1;
		    menuitem_started = 0;
		}
		else if ((!strcasecmp(divided[0], starter01)) || \
			(!strcasecmp(divided[0], starter02)) || \
			(!strcasecmp(divided[0], starter03))) {

			fprintf(stderr, "line %u: \"end\" before \"begin\" not possible!\n", menufile_currentline);
			exit(1);
		}
		else {
			fprintf(stderr, "line %u: \"%s\" not recognized!\n", menufile_currentline, line);
			exit(1);
		}

		return 0;
	}


	if ((!strcasecmp(divided[0], starter01)) || \
		(!strcasecmp(divided[0], starter02)) || \
		(!strcasecmp(divided[0], starter03))) {

		if ((!strcasecmp(divided[1], definer_menuitem01)) || (!strcasecmp(divided[1], definer_menuitem02))) {
			/* someone wants to define a menuitem... */
			if (!got_end) {
				/* starting to define a new menuitem without ending a previous */
				fprintf(stderr, "line %u: starting to define a new menuitem without ending a previous!\n", menufile_currentline);
				exit(1);
			}

			/* apply default values for menuitem */

			tempstruct.name = strdup("GENERIC");
			tempstruct.bullet = strdup("NUMBERED");
			tempstruct.label = strdup("");
                        tempstruct.icaption = strdup("");
			tempstruct.def = strdup("");
			tempstruct.command = strdup("");
			tempstruct.type = 0;

			got_end = 0;
			menuitem_started = 1;

			return 0;
		}
		else {
			fprintf(stderr, "line %u: no such definer \"%s\"!\n", menufile_currentline, divided[1]);
			exit(1);
		}
	}
	else if ((!strcasecmp(divided[0], end01)) || \
		(!strcasecmp(divided[0], end02))) {
		fprintf(stderr, "line %u: \"end\" before \"begin\"? not possible!\n", menufile_currentline);
		exit(1);
	}



	/* else, assume it's a variable. */

	if (!strcasecmp(divided[0], "text")) {
		replacestringwithchar(divided[1], "\\n", 0x0a);
		if ((text = strdup(divided[1])) == NULL) {
			fprintf(stderr, "strdup() failed - %s!\n", strerror(errno));
			exit(1);
		}
	}
	else if (!strcasecmp(divided[0], "aftertext")) {
		replacestringwithchar(divided[1], "\\n", 0x0a);
		if ((aftertext = strdup(divided[1])) == NULL) {
			fprintf(stderr, "strdup() failed - %s!\n", strerror(errno));
			exit(1);
		}
	}
	else if (!strcasecmp(divided[0], "textindent")) {
		textindent = atoi(divided[1]);
	}
	else if (!strcasecmp(divided[0], "aftertextindent")) {
		aftertextindent = atoi(divided[1]);
	}
	else if (!strcasecmp(divided[0], "colorscheme")) {
		if (!dontsetcolorscheme)
			colorscheme = atoi(divided[1]);
	}
	else if (!strcasecmp(divided[0], "caption")) {
		if ((caption = strdup(divided[1])) == NULL) {
			fprintf(stderr, "strdup() failed - %s!\n", strerror(errno));
			exit(1);
		}
	}
	else if (!strcasecmp(divided[0], "runonexit")) {
		if ((runonexit = strdup(divided[1])) == NULL) {
			fprintf(stderr, "strdup() failed - %s!\n", strerror(errno));
			exit(1);
		}
	}
	else if (!strcasecmp(divided[0], "br") || !strcasecmp(divided[0], "dummy")) {
		if ((tempstruct.label = strdup(divided[1])) == NULL) {
			fprintf(stderr, "strdup() failed - %s!\n", strerror(errno));
			exit(1);
		}

		tempstruct.name = strdup("GENERIC");
		tempstruct.bullet = strdup("NUMBERED");
                tempstruct.icaption = strdup("");
		tempstruct.def = strdup("");
		tempstruct.command = strdup("");
		tempstruct.type = RMtype_dummy;

		replacechar(tempstruct.label, 0x0a, 0x20);
		replacechar(tempstruct.label, 0x0d, 0x20);
		replacechar(tempstruct.label, 0x09, 0x20);
		replacechar(tempstruct.label, 0xff, 0x20);

		/* add this structure to the big heap of structs */
		if (addtomenu(&tempstruct)) {
			fprintf(stderr, "addtomenu() failed - %s!\n", strerror(errno));
			exit(1);
		}
	}
	else if (!strcasecmp(divided[0], "nocls")) {
		docls = 0;
	}
	else if (!strcasecmp(divided[0], "exitafterauto")) {
            if (!exitafterauto_overrider)
                exitafterauto = 1;
	}
	else {
		fprintf(stderr, "line %u: \"%s\" not recognized!\n", menufile_currentline, line);
		exit(1);
	}
	return 0;
}

/*****************************************************************************/

void doquit(void) {
	int retval = 0;

	/* enable echoing */
	tcsetattr(STDIN_FILENO, TCSANOW, &oldtty);

	if (docls) {
		printf("\033[0m\033[2J\033[%u;1H", wsize.ws_row);
		fflush(stdout);
	}

	if (runonexit) {
		retval = system(runonexit);
	}

	if (retval >= 256)
		retval /= 256;

	exit(retval);
}

/***/

void dospecialquit(char *command, unsigned int rmtype) {
	int retval = 0;

	/* enable echoing */
	tcsetattr(STDIN_FILENO, TCSANOW, &oldtty);

	if (!(rmtype & RMtype_nocls)) {
		printf("\033[0m\033[2J\033[%u;1H", wsize.ws_row);
		fflush(stdout);
	}

	if (!command) {
		if (runonexit) {
			retval = system(runonexit);
		}
	}
	else if (command[0] == 0) {
		if (runonexit) {
			retval = system(runonexit);
		}
	}
	else {
		if (rmtype & RMtype_runonexitfirst) {
			if (rmtype & RMtype_usecommandretval) {
				system(runonexit);
				retval = system(command);
			}
			else {
				retval = system(runonexit);
				system(command);
			}
		}
		else if (rmtype & RMtype_runonexitlast) {
			if (rmtype & RMtype_usecommandretval) {
				retval = system(command);
				system(runonexit);
			}
			else {
				system(command);
				retval = system(runonexit);
			}
		}
		else {
			retval = system(command);
		}
	}

	if (retval >= 256)
		retval /= 256;

	exit(retval);
}

/*****************************************************************************/

int count_drawBoxlines(char *line) {
/*
 *     This function is called from a foreachline()-funcion and counts the
 *     number of lines fed to it and increments/updates the drawBox_ global
 *     variables.
 */
    unsigned int x;
    x = strlen(line);
    if (x > drawBox_longestLine)
        drawBox_longestLine = x;
    drawBox_numberOfLines++;
    return 0;
}

int print_drawBoxlines(char *line) {
    unsigned int z;
    drawBox_x++;
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }
    printf("\033[%u;%uH\033[0m%s%s", drawBox_x, (wsize.ws_col/2)-(drawBox_longestLine/2)+1, drawBox_textcolor, line);
    return 0;
}

void drawMsgBox(char *boxtext) {
    unsigned int z;
    unsigned int minimum_boxheight = 4, boxheight;

    char okbutton[] = "\033[0;41m< \033[1mO\033[0;41mK >%s";
    unsigned int okbutton_size = 6;

    if (boxtext == NULL)
	    return;

    /* initialize some values first */

    drawBox_longestLine = 0;
    drawBox_numberOfLines = 0;

    /* define the colorscheme... */

    if (colorscheme == 1) {
	    drawBox_bgcolor = cs01_boxbgcolor;
	    drawBox_textcolor = cs01_boxtextcolor;
    }
    else if (colorscheme == 2) {
	    drawBox_bgcolor = cs02_boxbgcolor;
	    drawBox_textcolor = cs02_boxtextcolor;
    }
    else if (colorscheme == 3) {
	    drawBox_bgcolor = cs03_boxbgcolor;
	    drawBox_textcolor = cs03_boxtextcolor;
    }
    else if (colorscheme == 4) {
	    drawBox_bgcolor = cs04_boxbgcolor;
	    drawBox_textcolor = cs04_boxtextcolor;
    }
    else if (colorscheme == 5) {
	    drawBox_bgcolor = cs05_boxbgcolor;
	    drawBox_textcolor = cs05_boxtextcolor;
    }
    else if (colorscheme == 6) {
	    drawBox_bgcolor = cs06_boxbgcolor;
	    drawBox_textcolor = cs06_boxtextcolor;
    }
    else if (colorscheme == 7) {
	    drawBox_bgcolor = cs07_boxbgcolor;
	    drawBox_textcolor = cs07_boxtextcolor;
    }
    else if (colorscheme == 8) {
	    drawBox_bgcolor = cs08_boxbgcolor;
	    drawBox_textcolor = cs08_boxtextcolor;
    }
    else if (colorscheme == 9) {
	    drawBox_bgcolor = cs09_boxbgcolor;
	    drawBox_textcolor = cs09_boxtextcolor;
    }
    else {
	    drawBox_bgcolor = cs00_boxbgcolor;
	    drawBox_textcolor = cs00_boxtextcolor;
    }

    /* fix a multi-line box */

    /* replace all \n strings in box message */
    replacestringwithchar(boxtext, "\\n", 0x0a);

    /* count lines in msgbox */
    foreachline(boxtext, count_drawBoxlines);

    boxheight = minimum_boxheight + drawBox_numberOfLines;

    drawBox_x = wsize.ws_row/2;
    drawBox_y = wsize.ws_col/2;

    drawBox_x -= boxheight / 2;

    if (drawBox_longestLine < (okbutton_size)) {
	    drawBox_y -= (okbutton_size+4) / 2;
	    drawBox_boxwidth = (okbutton_size+4);
    }
    else {
	    drawBox_y -= (drawBox_longestLine+4) / 2;
	    drawBox_boxwidth = drawBox_longestLine+4;
    }



    /* print the msgbox */

    drawBox_x++;
    drawBox_y++;

    /* intro */
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }

    foreachline(boxtext, print_drawBoxlines);

    drawBox_x++;
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }

    /* OK button */
    drawBox_x++;
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }
    printf("\033[%u;%uH", drawBox_x, (wsize.ws_col/2)-(okbutton_size/2)+1);
    printf(okbutton, drawBox_bgcolor, drawBox_bgcolor);

    /* outro */
    drawBox_x++;
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }

    printf("\033[%u;%uH\033[0m", wsize.ws_row, wsize.ws_col);
    fflush(stdout);

    return;
}

/*****************************************************************************/

int makeMsgBox(char *alt_bt, char *bt) {
        char *boxtext;
	unsigned int key = 0;

        if (bt != NULL) {
            if (strlen(bt))
                boxtext = bt;
            else
                boxtext = alt_bt;
        } else {
            boxtext = alt_bt;
        }

	drawMsgBox(boxtext);

	while (1) {
		key = readkey();
                if (key == key_DBLESC || key == RETURN || key == 'O' || key == 'o')
                    break;
	}

	return 0;
}

/*****************************************************************************/

void drawYesNoBox(char *boxtext, int yesNoBox_chosen) {
    unsigned int z;
    unsigned int minimum_boxheight = 4, boxheight;

    char yesnoline_yeshi[] = "\033[0;41m< \033[1mY\033[0;41mes >%s  \033[0m  \033[1mN\033[0mo  %s";
    char yesnoline_nohi[] = "\033[0m  \033[1mY\033[0mes  %s  \033[0;41m< \033[1mN\033[0;41mo >%s";
    unsigned int yesnoline_size = 7+2+6;

    if (boxtext == NULL)
	    return;

    /* initialize some values first */

    drawBox_longestLine = 0;
    drawBox_numberOfLines = 0;

    /* define the colorscheme... */

    if (colorscheme == 1) {
	drawBox_bgcolor = cs01_boxbgcolor;
	drawBox_textcolor = cs01_boxtextcolor;
    }
    else if (colorscheme == 2) {
	drawBox_bgcolor = cs02_boxbgcolor;
	drawBox_textcolor = cs02_boxtextcolor;
    }
    else if (colorscheme == 3) {
	drawBox_bgcolor = cs03_boxbgcolor;
	drawBox_textcolor = cs03_boxtextcolor;
    }
    else if (colorscheme == 4) {
	drawBox_bgcolor = cs04_boxbgcolor;
	drawBox_textcolor = cs04_boxtextcolor;
    }
    else if (colorscheme == 5) {
	drawBox_bgcolor = cs05_boxbgcolor;
	drawBox_textcolor = cs05_boxtextcolor;
    }
    else if (colorscheme == 6) {
	drawBox_bgcolor = cs06_boxbgcolor;
	drawBox_textcolor = cs06_boxtextcolor;
    }
    else if (colorscheme == 7) {
	drawBox_bgcolor = cs07_boxbgcolor;
	drawBox_textcolor = cs07_boxtextcolor;
    }
    else if (colorscheme == 8) {
	drawBox_bgcolor = cs08_boxbgcolor;
	drawBox_textcolor = cs08_boxtextcolor;
    }
    else if (colorscheme == 9) {
	drawBox_bgcolor = cs09_boxbgcolor;
	drawBox_textcolor = cs09_boxtextcolor;
    }
    else {
	drawBox_bgcolor = cs00_boxbgcolor;
	drawBox_textcolor = cs00_boxtextcolor;
    }

    /* fix a multi-line box */

    /* replace all \n strings in box message */
    replacestringwithchar(boxtext, "\\n", 0x0a);

    /* count lines in msgbox */
    foreachline(boxtext, count_drawBoxlines);

    boxheight = minimum_boxheight + drawBox_numberOfLines;

    drawBox_x = wsize.ws_row/2;
    drawBox_y = wsize.ws_col/2;

    drawBox_x -= boxheight / 2;

    if (drawBox_longestLine < (yesnoline_size)) {
        drawBox_y -= (yesnoline_size+4) / 2;
        drawBox_boxwidth = (yesnoline_size+4);
    }
    else {
        drawBox_y -= (drawBox_longestLine+4) / 2;
        drawBox_boxwidth = drawBox_longestLine+4;
    }

    /* print the yesnobox */

    drawBox_x++;
    drawBox_y++;

    /* intro */
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }

    foreachline(boxtext, print_drawBoxlines);

    drawBox_x++;
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }

    /* OK button */
    drawBox_x++;
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }
    if (yesNoBox_chosen == box_YES) {
	printf("\033[%u;%uH", drawBox_x, (wsize.ws_col/2)-(yesnoline_size/2)+1);
	printf(yesnoline_yeshi, drawBox_bgcolor, drawBox_bgcolor);
    }
    else {
	printf("\033[%u;%uH", drawBox_x, (wsize.ws_col/2)-(yesnoline_size/2)+1);
	printf(yesnoline_nohi, drawBox_bgcolor, drawBox_bgcolor);
    }

    /* outro */
    drawBox_x++;
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }

    printf("\033[%u;%uH\033[0m", wsize.ws_row, wsize.ws_col);
    fflush(stdout);

    return;
}

/*****************************************************************************/

int makeYesNoBox(char *alt_bt, char *bt, char *yesorno) {

        char *boxtext;
	unsigned int key = 0;
	int yesNoBox_chosen = box_NO;
	int original_yesNoBox_chosen = box_NO;

        if (bt != NULL) {
            if (strlen(bt))
                boxtext = bt;
            else
                boxtext = alt_bt;
        } else {
            boxtext = alt_bt;
        }

	if (yesorno) {
		if (!strcasecmp(yesorno, "yes")) {
			yesNoBox_chosen = box_YES;
			original_yesNoBox_chosen = box_YES;
		}
		else {
			yesNoBox_chosen = box_NO;
			original_yesNoBox_chosen = box_NO;
		}
	}

	drawYesNoBox(boxtext, yesNoBox_chosen);

	while (1) {
		key = readkey();

		if (key == key_DBLESC) {
			return box_CANCEL;
		}
		else if (key == RETURN) {
			return yesNoBox_chosen;
		}
		else if ((key == key_LEFT) || (key == key_RIGHT) || (key == key_UP) || (key == key_DOWN)) {
			if (yesNoBox_chosen == box_YES) {
				yesNoBox_chosen = box_NO;
			}
			else {
				yesNoBox_chosen = box_YES;
			}
			drawYesNoBox(boxtext, yesNoBox_chosen);
		}
		else if ((key == 'y') || (key == 'Y'))
			return box_YES;
		else if ((key == 'n') || (key == 'N'))
			return box_NO;
	}

	return box_NO;
}

/*****************************************************************************/

int makeInputBox(char *alt_bt, char *bt, char *defaultinput, int ispasswd) {
    char *boxtext;
    unsigned int key = 0;
    unsigned int z, i;
    unsigned int minimum_boxheight = 4, boxheight;

    unsigned int inputfieldlen = 28, iL_x = 1, iL_y = 1, retflag;
    char inputfield[] = "                            ";
    char *p;

    if (bt != NULL) {
        if (strlen(bt))
            boxtext = bt;
        else
            boxtext = alt_bt;
    } else {
        boxtext = alt_bt;
    }

    if ((boxtext == NULL) || (defaultinput == NULL))
        return 1;

    memset(inputfieldbuf, 0, sizeof(inputfieldbuf));
    strncpy(inputfieldbuf, defaultinput, sizeof(inputfieldbuf)-1);


    /* initialize some values first */

    drawBox_longestLine = 0;
    drawBox_numberOfLines = 0;

    /* define the colorscheme... */

    if (colorscheme == 1) {
	    drawBox_inputcolor = cs01_boxinputcolor;
	    drawBox_bgcolor = cs01_boxbgcolor;
	    drawBox_textcolor = cs01_boxtextcolor;
    }
    else if (colorscheme == 2) {
	    drawBox_inputcolor = cs02_boxinputcolor;
	    drawBox_bgcolor = cs02_boxbgcolor;
	    drawBox_textcolor = cs02_boxtextcolor;
    }
    else if (colorscheme == 3) {
	    drawBox_inputcolor = cs03_boxinputcolor;
	    drawBox_bgcolor = cs03_boxbgcolor;
	    drawBox_textcolor = cs03_boxtextcolor;
    }
    else if (colorscheme == 4) {
	    drawBox_inputcolor = cs04_boxinputcolor;
	    drawBox_bgcolor = cs04_boxbgcolor;
	    drawBox_textcolor = cs04_boxtextcolor;
    }
    else if (colorscheme == 5) {
	    drawBox_inputcolor = cs05_boxinputcolor;
	    drawBox_bgcolor = cs05_boxbgcolor;
	    drawBox_textcolor = cs05_boxtextcolor;
    }
    else if (colorscheme == 6) {
	    drawBox_inputcolor = cs06_boxinputcolor;
	    drawBox_bgcolor = cs06_boxbgcolor;
	    drawBox_textcolor = cs06_boxtextcolor;
    }
    else if (colorscheme == 7) {
	    drawBox_inputcolor = cs07_boxinputcolor;
	    drawBox_bgcolor = cs07_boxbgcolor;
	    drawBox_textcolor = cs07_boxtextcolor;
    }
    else if (colorscheme == 8) {
	    drawBox_inputcolor = cs08_boxinputcolor;
	    drawBox_bgcolor = cs08_boxbgcolor;
	    drawBox_textcolor = cs08_boxtextcolor;
    }
    else if (colorscheme == 9) {
	    drawBox_inputcolor = cs09_boxinputcolor;
	    drawBox_bgcolor = cs09_boxbgcolor;
	    drawBox_textcolor = cs09_boxtextcolor;
    }
    else {
	    drawBox_inputcolor = cs00_boxinputcolor;
	    drawBox_bgcolor = cs00_boxbgcolor;
	    drawBox_textcolor = cs00_boxtextcolor;
    }

    /* fix a multi-line box */

    /* replace all \n strings in box message */
    replacestringwithchar(boxtext, "\\n", 0x0a);

    /* count lines in msgbox */
    foreachline(boxtext, count_drawBoxlines);

    boxheight = minimum_boxheight + drawBox_numberOfLines;

    drawBox_x = wsize.ws_row/2;
    drawBox_y = wsize.ws_col/2;

    drawBox_x -= boxheight / 2;

    if (drawBox_longestLine < inputfieldlen) {
	drawBox_y -= (inputfieldlen+4) / 2;
	drawBox_boxwidth = (inputfieldlen+4);
    }
    else {
	drawBox_y -= (drawBox_longestLine+4) / 2;
	drawBox_boxwidth = drawBox_longestLine+4;
    }

    /* print the inputbox */

    drawBox_x++;
    drawBox_y++;

    /* intro */
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }

    foreachline(boxtext, print_drawBoxlines);

    drawBox_x++;
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }

    /* inputfield preparation */
    drawBox_x++;
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }
    iL_x = drawBox_x;
    iL_y = (wsize.ws_col/2)-(inputfieldlen/2)+1;

    /* midtro */
    drawBox_x++;
    printf("\033[%u;%uH%s", drawBox_x, drawBox_y, drawBox_bgcolor);
    for (z = 0; z < drawBox_boxwidth; z++) {
	putchar(0x20);
    }

    /* draw the inputfield */

    i = strlen(inputfieldbuf);
    if (i >= (inputfieldlen-1)) {
	    p = &inputfieldbuf[i - (inputfieldlen-1)];
    }
    else {
	    p = inputfieldbuf;
    }

    /* draw the input value of the field. if it's a password type, just draw stars */

    printf("\033[%u;%uH\033[0m%s%s", iL_x, iL_y, drawBox_inputcolor, inputfield);
    if (ispasswd) {
        printf("\033[%u;%uH\033[0m%s", iL_x, iL_y, drawBox_inputcolor);
        for (i = 0; i < strlen(p); i++) {
            putchar('*');
        }
    } else {
        printf("\033[%u;%uH\033[0m%s%s", iL_x, iL_y, drawBox_inputcolor, p);
    }
    fflush(stdout);

    /* read from input stuff here */

    while (1) {
	key = readkey();
	i = strlen(inputfieldbuf);

	if (key == key_DBLESC) {
	    retflag = 1;
	    break;
	}
	else if ((key >= 0x10000000) && (key < 0x20000000)) {
	    /* filter out any other keys generated by readkey() */
	}
	else if ((key == BACKSPACE1) || (key == BACKSPACE2)) {
	    /* delete the last char from inputfieldbuf */
	    if (i) {
		    inputfieldbuf[i-1] = 0;
		    i--;
	    }
	}
	else if (key == RETURN) {
	    /* accept what's in inputfieldbuf */
	    retflag = 0;
	    break;
	}
	else {
	    /* everything else gets added to inputfieldbuf */

	    /* convert tabs */
	    if (key == 0x09)
		    key = 0x20;

	    if (i < sizeof(inputfieldbuf)) {
		    inputfieldbuf[i] = key;
		    i++;
	    }
	}

	/* redraw the inputfield */

	if (i >= (inputfieldlen-1)) {
	    p = &inputfieldbuf[i - (inputfieldlen-1)];
	}
	else {
	    p = inputfieldbuf;
	}

        /* draw the input value of the field. if it's a password type, just draw stars */

	printf("\033[%u;%uH\033[0m%s%s", iL_x, iL_y, drawBox_inputcolor, inputfield);
        if (ispasswd) {
            printf("\033[%u;%uH\033[0m%s", iL_x, iL_y, drawBox_inputcolor);
            for (i = 0; i < strlen(p); i++) {
                putchar('*');
            }
        } else {
            printf("\033[%u;%uH\033[0m%s%s", iL_x, iL_y, drawBox_inputcolor, p);
        }
	fflush(stdout);
    }

    printf("\033[%u;1H\033[0m", wsize.ws_row);
    fflush(stdout);

    return retflag;
}

/*****************************************************************************/
