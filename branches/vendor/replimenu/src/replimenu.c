/*
 * 'replimenu'
 * Copyright (C) 2003 Michel Blomgren, michel AT zebra DOT ath DOT cx.
 *
 * $Id: replimenu.c,v 1.5 2004/01/05 03:45:06 shadow Exp $
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details (refer to the COPYING-file).
 *
 */

/*
 * CVS log
 *
 * $Log: replimenu.c,v $
 * Revision 1.5  2004/01/05 03:45:06  shadow
 * fixed the flickery menu with a dirty hack, generated a bug that I had to fix and added a new feature: multi-line {input,msg,yesno}-boxes.
 *
 * Revision 1.4  2004/01/04 03:11:15  shadow
 * making version 0.9, currently improved the flickery browsing
 *
 * Revision 1.3  2003/12/21 09:56:20  shadow
 * added CVS keywords to most source files
 *
 * Revision 1.2  2003/12/21 09:28:26  shadow
 * added CVS tags
 *
 */

#include "replimenu.h"
#include "functions.h"

/*****************************************************************************/

void drawborders(void) {

	unsigned int i;
	char *bordercolor = NULL;

	if (colorscheme == 1) {
		bordercolor = cs01_bordercolor;
	}
	else if (colorscheme == 2) {
		bordercolor = cs02_bordercolor;
	}
	else if (colorscheme == 3) {
		bordercolor = cs03_bordercolor;
	}
	else if (colorscheme == 4) {
		bordercolor = cs04_bordercolor;
	}
	else if (colorscheme == 5) {
		bordercolor = cs05_bordercolor;
	}
	else if (colorscheme == 6) {
		bordercolor = cs06_bordercolor;
	}
	else if (colorscheme == 7) {
		bordercolor = cs07_bordercolor;
	}
	else if (colorscheme == 8) {
		bordercolor = cs08_bordercolor;
	}
	else if (colorscheme == 9) {
		bordercolor = cs09_bordercolor;
	}
	else {
		bordercolor = cs00_bordercolor;
	}


	maxmenuitemstodisplay = wsize.ws_row;

	if (!text) {
		text = getenv("REPLIMENU_TEXT");
	}
	if (!aftertext) {
		aftertext = getenv("REPLIMENU_AFTERTEXT");
	}

	replacechar(text, 0x09, 0x20);
	replacechar(aftertext, 0x09, 0x20);

	nrwrappedlines = 0;
	indent = textindent;
	foreachline(text, countwrappedlines);
	if (nrwrappedlines < maxmenuitemstodisplay)
		maxmenuitemstodisplay -= nrwrappedlines;
	else
		maxmenuitemstodisplay = 0;
	nroflines_text = nrwrappedlines;

	nrwrappedlines = 0;
	indent = aftertextindent;
	foreachline(aftertext, countwrappedlines);
	if (nrwrappedlines < maxmenuitemstodisplay)
		maxmenuitemstodisplay -= nrwrappedlines;
	else
		maxmenuitemstodisplay = 0;
	nroflines_aftertext = nrwrappedlines;


	if (aftertext) {
		if (maxmenuitemstodisplay)
			maxmenuitemstodisplay--;
	}

	if (text) {
		if (maxmenuitemstodisplay)
			maxmenuitemstodisplay--;
	}


	printf("\033[1;1H%s", bordercolor);
	fflush(stdout);

	for (i = 0; wsize.ws_col > i; i++) {
		putchar(0x20);
	}

	if (((wsize.ws_col / 2) - (strlen(caption) / 2)) > 0)
		i = (wsize.ws_col / 2) - (strlen(caption) / 2);
	else
        	i = 0;

	printf("\033[1;%uH%s\033[%u;1H", i, caption, wsize.ws_row);
	fflush(stdout);
	if (maxmenuitemstodisplay)
		maxmenuitemstodisplay--;

	for (i = 0; wsize.ws_col > i; i++) {
		putchar(0x20);
	}

	if (((wsize.ws_col / 2) - (strlen(bottomrowmsg)) / 2) > 0)
		i = (wsize.ws_col / 2) - (strlen(bottomrowmsg) / 2);
	else
        	i = 0;

	printf("\033[%u;%uH%s\033[0m", wsize.ws_row, i, bottomrowmsg);
	fflush(stdout);
	if (maxmenuitemstodisplay)
		maxmenuitemstodisplay--;

	if (maxmenuitemstodisplay)
		maxmenuitemstodisplay--;
	if (maxmenuitemstodisplay)
		maxmenuitemstodisplay--;

	return;
}

/*****************************************************************************/

int drawmenu(unsigned int pos) {

	static unsigned int previouspos = 0;

	char RMprevious[] = "^(-)";
	char RMnext[] = "v(+)";

	unsigned int i = 0, x, y, start = 0, end = 0, menuitemscount = 0, ivisible = 0, printeditems = 0;
        unsigned int visiblepos = 0;
	unsigned int maxiwrite;

	struct menustruct *tempstruct;

	char buf[256];
	char buf2[16];

	char labelbuf[256];

	char *itemLineHilited = NULL, *itemLineNormal = NULL, *itemLineDummy = NULL, *textcolor = NULL, *morecolor = NULL;
	char *bgcolor = NULL;

	char *value;


	if ((wsize.ws_col-longestBullet-2) >= 0)
		maxiwrite = wsize.ws_col-longestBullet-2;
        else
		return 1;

	if (maxiwrite > sizeof(labelbuf))	/* just in case it's really really (too) long... */
        	maxiwrite = sizeof(labelbuf);


	/* set correct color scheme */
	if (colorscheme == 1) {
		itemLineHilited = cs01_itemLineHilited;
		itemLineNormal = cs01_itemLineNormal;
		itemLineDummy = cs01_itemLineDummy;
		textcolor = cs01_textcolor;
		morecolor = cs01_morecolor;
		bgcolor = cs01_bgcolor;
	}
	else if (colorscheme == 2) {
		itemLineHilited = cs02_itemLineHilited;
		itemLineNormal = cs02_itemLineNormal;
		itemLineDummy = cs02_itemLineDummy;
		textcolor = cs02_textcolor;
		morecolor = cs02_morecolor;
		bgcolor = cs02_bgcolor;
	}
	else if (colorscheme == 3) {
		itemLineHilited = cs03_itemLineHilited;
		itemLineNormal = cs03_itemLineNormal;
		itemLineDummy = cs03_itemLineDummy;
		textcolor = cs03_textcolor;
		morecolor = cs03_morecolor;
		bgcolor = cs03_bgcolor;
	}
	else if (colorscheme == 4) {
		itemLineHilited = cs04_itemLineHilited;
		itemLineNormal = cs04_itemLineNormal;
		itemLineDummy = cs04_itemLineDummy;
		textcolor = cs04_textcolor;
		morecolor = cs04_morecolor;
		bgcolor = cs04_bgcolor;
	}
	else if (colorscheme == 5) {
		itemLineHilited = cs05_itemLineHilited;
		itemLineNormal = cs05_itemLineNormal;
		itemLineDummy = cs05_itemLineDummy;
		textcolor = cs05_textcolor;
		morecolor = cs05_morecolor;
		bgcolor = cs05_bgcolor;
	}
	else if (colorscheme == 6) {
		itemLineHilited = cs06_itemLineHilited;
		itemLineNormal = cs06_itemLineNormal;
		itemLineDummy = cs06_itemLineDummy;
		textcolor = cs06_textcolor;
		morecolor = cs06_morecolor;
		bgcolor = cs06_bgcolor;
	}
	else if (colorscheme == 7) {
		itemLineHilited = cs07_itemLineHilited;
		itemLineNormal = cs07_itemLineNormal;
		itemLineDummy = cs07_itemLineDummy;
		textcolor = cs07_textcolor;
		morecolor = cs07_morecolor;
		bgcolor = cs07_bgcolor;
	}
	else if (colorscheme == 8) {
		itemLineHilited = cs08_itemLineHilited;
		itemLineNormal = cs08_itemLineNormal;
		itemLineDummy = cs08_itemLineDummy;
		textcolor = cs08_textcolor;
		morecolor = cs08_morecolor;
		bgcolor = cs08_bgcolor;
	}
	else if (colorscheme == 9) {
		itemLineHilited = cs09_itemLineHilited;
		itemLineNormal = cs09_itemLineNormal;
		itemLineDummy = cs09_itemLineDummy;
		textcolor = cs09_textcolor;
		morecolor = cs09_morecolor;
		bgcolor = cs09_bgcolor;
	}
	else {
		itemLineHilited = cs00_itemLineHilited;
		itemLineNormal = cs00_itemLineNormal;
		itemLineDummy = cs00_itemLineDummy;
		textcolor = cs00_textcolor;
		morecolor = cs00_morecolor;
		bgcolor = cs00_bgcolor;
	}


	if (!maxmenuitemstodisplay) {
		snprintf(uem, sizeof(uem), "Menu doesn't fit on screen!");
		die();
	}



	/* check if this position is a "dummy" or "hidden" */
	while (1) {
	    if ((tempstruct = structs[pos]) == NULL) {
                /* if we run out of menu items, we allow an empty menu */
                /* we continue with the position = pos as we want it to point to the NULL struct member */
                break;
            }

	    if ((tempstruct->type & RMtype_dummy) || (tempstruct->type & RMtype_hidden)) {
		if (previouspos <= pos) {
		    pos++;
		}
		else {
		    if (pos)
			pos--;
		}
	    }
	    else {
		break;
	    }
	}
	position = pos;     /* save the new position */


        /* get the visiblepos for this position */
	for (i = 0; i < menuitems; i++) {
	    if ((tempstruct = structs[i]) == NULL)
		    break;
            if (i >= pos)
                    break;
            if (!(tempstruct->type & RMtype_hidden))
                visiblepos++;
	}

	if (text) {
		/* we should print text on top, set the cursor position, set up colors and wrap text nicely */
		printf("\033[3;1H%s", textcolor);
		fflush(stdout);
		indent = textindent;
		foreachline(text, wraptext);
		printf("\033[0m");
		fflush(stdout);
	}
	else {
		/* no text to print, but we should place the cursor at the right place */
		printf("\033[2;1H\033[0m");
		fflush(stdout);
	}


	/* figure out where to start */
	if (visiblemenuitems > maxmenuitemstodisplay) {
		for (i = 1; i <= ((visiblemenuitems/maxmenuitemstodisplay)+1); i++) {
			end = i * maxmenuitemstodisplay;
			start = end - maxmenuitemstodisplay;

			if ((visiblepos >= start) && (visiblepos < end))
				break;
		}
	}
	else {
                i = 0;
		start = 0;
		end = visiblemenuitems;
	}

	/* check if we want to draw paging indicators (arrow down to symbol more below) */
	if (i) {
		if (i > 1) {
			if ((wsize.ws_col - strlen(RMprevious) - 2) > 0)
				x = wsize.ws_col - strlen(RMprevious) - 2;
                        else
                        	x = 0;
	                printf("%s\033[2K\033[%uC%s%s\033[0m\n", bgcolor, x, morecolor, RMprevious);
		}
		else {
			printf("%s\033[2K\n", bgcolor);
		}
	}
	else {
		printf("%s\033[2K\n", bgcolor);
	}


	for (i = 0; i < menuitems; i++) {
	    if ((tempstruct = structs[i]) == NULL)
		    return 1;

	    if ((!tempstruct->name) || (!tempstruct->label) || (!tempstruct->bullet))
		    return 1;

	    snprintf(labelbuf, maxiwrite, "%s", tempstruct->label);

                if (!(tempstruct->type & RMtype_hidden)) {
		    if ((ivisible >= start) && (ivisible < end)) {
		        if (i == pos) {
			    /* print a highlighted menu item */

			    if (tempstruct->type & RMtype_checkbox) {
				    snprintf(scratch, sizeof(scratch), "RM_%s", tempstruct->name);
				    if (getenv(scratch))
					    snprintf(buf2, sizeof(buf2), "[X]");
				    else
					    snprintf(buf2, sizeof(buf2), "[ ]");

				    if ((longestItem+longestBullet+1) > wsize.ws_col)
                                    	x = 0;
                                    else
                                    	x = (wsize.ws_col / 2) - ((longestItem+longestBullet+2) / 2);
				    snprintf(buf, sizeof(buf), itemLineHilited, x, longestBullet);
				    printf("%s", bgcolor);
				    printf(buf, buf2, labelbuf);
                                    /* here we do padding after the item label has been printed */
                                    if ((y = wsize.ws_col - longestBullet - strlen(labelbuf) - x - 2) > 0) {
                                        while (y) { putchar(0x20); y--; }
                                    }
                                    printf("\033[0m\n");
                                    fflush(stdout);
			    }
			    else if (tempstruct->type & RMtype_radiobutton) {
				    snprintf(scratch, sizeof(scratch), "RM_%s", tempstruct->name);
				    if ((value = getenv(scratch)) == NULL)
					    value = "";
				    if (!strcmp(value, tempstruct->def))
					    snprintf(buf2, sizeof(buf2), "(*)");
				    else
					    snprintf(buf2, sizeof(buf2), "( )");

				    if ((longestItem+longestBullet+1) > wsize.ws_col)
                                    	x = 0;
                                    else
                                    	x = (wsize.ws_col / 2) - ((longestItem+longestBullet+2) / 2);
				    snprintf(buf, sizeof(buf), itemLineHilited, x, longestBullet);
				    printf("%s", bgcolor);
				    printf(buf, buf2, labelbuf);

                                    /* here we do padding after the item label has been printed */
                                    if ((y = wsize.ws_col - longestBullet - strlen(labelbuf) - x - 2) > 0) {
                                        while (y) { putchar(0x20); y--; }
                                    }
                                    printf("\033[0m\n");
                                    fflush(stdout);
			    }
			    else {
				    if ((longestItem+longestBullet+1) > wsize.ws_col)
                                    	x = 0;
                                    else
                                    	x = (wsize.ws_col / 2) - ((longestItem+longestBullet+2) / 2);
				    snprintf(buf, sizeof(buf), itemLineHilited, x, longestBullet);
				    printf("%s", bgcolor);
				    printf(buf, tempstruct->bullet, labelbuf);
                                    /* here we do padding after the item label has been printed */
                                    if ((y = wsize.ws_col - longestBullet - strlen(labelbuf) - x - 2) > 0) {
                                        while (y) { putchar(0x20); y--; }
                                    }
                                    printf("\033[0m\n");
                                    fflush(stdout);
			    }
		        } else {
			    /* print a normal menu item */

			    if (tempstruct->type & RMtype_dummy) {
				    if ((longestItem+longestBullet+1) > wsize.ws_col)
                                    	x = 0;
                                    else
                                    	x = (wsize.ws_col / 2) - ((longestItem+longestBullet+2) / 2);
                                    snprintf(buf, sizeof(buf), itemLineDummy, x, longestItem);
				    printf("%s", bgcolor);
                                    if ((wsize.ws_col-1) > sizeof(labelbuf))
                                    	snprintf(labelbuf, sizeof(labelbuf), "%s", tempstruct->label);
				    else
                                    	snprintf(labelbuf, (wsize.ws_col-1), "%s", tempstruct->label);
				    printf(buf, labelbuf);
                                    /* here we do padding after the item label has been printed */
                                    if ((y = wsize.ws_col - strlen(labelbuf) - x - 2) > 0) {
                                        while (y) { putchar(0x20); y--; }
                                    }
                                    printf("\033[0m\n");
                                    fflush(stdout);
			    }
			    else if (tempstruct->type & RMtype_checkbox) {
				    snprintf(scratch, sizeof(scratch), "RM_%s", tempstruct->name);
				    if (getenv(scratch))
					    snprintf(buf2, sizeof(buf2), "[X]");
				    else
					    snprintf(buf2, sizeof(buf2), "[ ]");

				    if ((longestItem+longestBullet+1) > wsize.ws_col)
                                    	x = 0;
                                    else
                                    	x = (wsize.ws_col / 2) - ((longestItem+longestBullet+2) / 2);
				    snprintf(buf, sizeof(buf), itemLineNormal, x, longestBullet);
				    printf("%s", bgcolor);
				    printf(buf, buf2, labelbuf);
                                    /* here we do padding after the item label has been printed */
                                    if ((y = wsize.ws_col - longestBullet - strlen(labelbuf) - x - 2) > 0) {
                                        while (y) { putchar(0x20); y--; }
                                    }
                                    printf("\033[0m\n");
                                    fflush(stdout);
			    }
			    else if (tempstruct->type & RMtype_radiobutton) {
				    snprintf(scratch, sizeof(scratch), "RM_%s", tempstruct->name);
				    if ((value = getenv(scratch)) == NULL)
					    value = "";
				    if (!strcmp(value, tempstruct->def))
					    snprintf(buf2, sizeof(buf2), "(*)");
				    else
					    snprintf(buf2, sizeof(buf2), "( )");

				    if ((longestItem+longestBullet+1) > wsize.ws_col)
                                    	x = 0;
                                    else
                                    	x = (wsize.ws_col / 2) - ((longestItem+longestBullet+2) / 2);
				    snprintf(buf, sizeof(buf), itemLineNormal, x, longestBullet);
				    printf("%s", bgcolor);
				    printf(buf, buf2, labelbuf);
                                    /* here we do padding after the item label has been printed */
                                    if ((y = wsize.ws_col - longestBullet - strlen(labelbuf) - x - 2) > 0) {
                                        while (y) { putchar(0x20); y--; }
                                    }
                                    printf("\033[0m\n");
                                    fflush(stdout);
			    }
			    else {
				    if ((longestItem+longestBullet+1) > wsize.ws_col)
                                    	x = 0;
                                    else
                                    	x = (wsize.ws_col / 2) - ((longestItem+longestBullet+2) / 2);
				    snprintf(buf, sizeof(buf), itemLineNormal, x, longestBullet);
				    printf("%s", bgcolor);
				    printf(buf, tempstruct->bullet, labelbuf);
                                    /* here we do padding after the item label has been printed */
                                    if ((y = wsize.ws_col - longestBullet - strlen(labelbuf) - x - 2) > 0) {
                                        while (y) { putchar(0x20); y--; }
                                    }
                                    printf("\033[0m\n");
                                    fflush(stdout);
			    }
		        }
                        printeditems++;
		    }
                    ivisible++;
                }
                menuitemscount++;   /* we keep this count for historical reasons */
	}


	if (printeditems != visiblemenuitems) {
		for (i = maxmenuitemstodisplay - printeditems; i; i--) {
			printf("%s\033[2K\n", bgcolor);
		}
	}


	if (printeditems == visiblemenuitems) {
		printf("\n");
	}
	else {
		if (end < visiblemenuitems) {
                	if ((wsize.ws_col - strlen(RMnext) - 2) > 0)
				x = wsize.ws_col - strlen(RMnext) - 2;
                        else
                        	x = 0;
			printf("%s\033[2K\033[%uC%s%s\033[0m\n", bgcolor, x, morecolor, RMnext);
		}
		else {
			printf("%s\033[2K\n", bgcolor);
		}
	}

	printf("\033[%u;1H\033[%uA%s", wsize.ws_row, nroflines_aftertext+1, textcolor);

	indent = aftertextindent;
	foreachline(aftertext, wraptext);

	printf("\033[%u;%uH", wsize.ws_row, wsize.ws_col);
	fflush(stdout);

	previouspos = pos;

	return 0;
}

/*****************************************************************************/

int docommand(unsigned int pos) {

	char foochar[1];
	unsigned int rm_type;
	int foo;
	char *rm_itemname;
	char *rm_label;
        char *rm_icaption;
	char *rm_command;
	struct menustruct *tempstruct;

	char *value;

	disable_sigwinch = 1;

	if ((tempstruct = structs[pos]) == NULL) {
            /* if it's NULL drawmenu() might want to tell us it's got an empty menu, so we simply exit nicely... */
            return 0;
        }

        rm_type = tempstruct->type;
        rm_itemname = tempstruct->name;
        rm_label = tempstruct->label;
        rm_icaption = tempstruct->icaption;
        rm_command = tempstruct->command;

	if ((!rm_itemname) || (!rm_label))
		return -1;

	/* just in case... */
	if (rm_type & RMtype_dummy) {
		/* dumb, dumber... */
		flicker = 0;
		goto label_skipcommand;
	}

	if (rm_type & RMtype_checkbox) {
		/* checkbox has precedence over "radiobutton" (and "variable", and the rest) */
		snprintf(scratch, sizeof(scratch), "RM_%s", rm_itemname);
		if ((value = getenv(scratch)) == NULL) {
			/* make this item selected */
			if (setenv(scratch, tempstruct->def, 1)) {
				snprintf(uem, sizeof(uem), "strdup() puked - %s!\n", strerror(errno));
				die();
			}
			setenvcounter++;
			if (setenvcounter > MAX_SETENV) {
				snprintf(uem, sizeof(uem), "Security alert: too many calls to setenv(), aborting!\n");
				die();
			}
		}
		else {
			/* un-select this item */
			unsetenv(scratch);
		}
		flicker = 0;
		goto label_skipcommand;
	}
	else if (rm_type & RMtype_radiobutton) {
		/* radiobutton has precedence over "variable" */
		snprintf(scratch, sizeof(scratch), "RM_%s", rm_itemname);
		unsetenv(scratch);
		if (setenv(scratch, tempstruct->def, 1)) {
			snprintf(uem, sizeof(uem), "strdup() puked - %s!\n", strerror(errno));
			die();
		}
		setenvcounter++;
		if (setenvcounter > MAX_SETENV) {
			snprintf(uem, sizeof(uem), "Security alert: too many calls to setenv(), aborting!\n");
			die();
		}
		flicker = 0;
		goto label_skipcommand;
	}
        else if (rm_type & RMtype_msgbox) {
            /* msgbox has precedence over "variable" */
            makeMsgBox(rm_label, rm_icaption);
	    flicker = 1;
	    goto label_skipcommand;
        }
	else if (rm_type & RMtype_variable) {
	    /* variable has precedence over "inputbox" */

	    if (rm_type & RMtype_yesno) {
		/* if this is a yes/no variable */
		/* RM_{itemname} will be set to either "yes" or "no" */

		foo = makeYesNoBox(rm_label, rm_icaption, tempstruct->def);

		if (foo == box_NO) {
		    /* if "no" */
		    snprintf(inputfieldbuf, sizeof(inputfieldbuf), "no");
		}
		else if (foo == box_YES) {
		    /* it's a "yes" */
		    snprintf(inputfieldbuf, sizeof(inputfieldbuf), "yes");
		}
		else {
		    /* else it's a cancel */
		    flicker = 1;
		    goto label_skipcommand;
		}

		/* free old default in structure and allocate new */
		if (tempstruct->def)
		    free(tempstruct->def);
		if ((tempstruct->def = strdup(inputfieldbuf)) == NULL) {
		    snprintf(uem, sizeof(uem), "strdup() puked - %s!\n", strerror(errno));
		    die();
		}

		/* set new environment variable */
		snprintf(scratch, sizeof(scratch), "RM_%s", rm_itemname);
		if (setenv(scratch, tempstruct->def, 1)) {
		    snprintf(uem, sizeof(uem), "setenv() puked - %s!\n", strerror(errno));
		    die();
		}
		setenvcounter++;
		if (setenvcounter > MAX_SETENV) {
		    snprintf(uem, sizeof(uem), "Security alert: too many calls to setenv(), aborting!\n");
		    die();
		}
	    }
	    else {
                redo_inputbox1:
		if (makeInputBox(rm_label, rm_icaption, tempstruct->def, (tempstruct->type & RMtype_password) ? 1 : 0)) {
		    flicker = 1;
		    goto label_skipcommand;
		}

		/* free old default in structure and allocate new */
		if (tempstruct->def)
			free(tempstruct->def);
		if ((tempstruct->def = strdup(inputfieldbuf)) == NULL) {
			snprintf(uem, sizeof(uem), "strdup() puked - %s!\n", strerror(errno));
			die();
		}

		/* set new environment variable */
		snprintf(scratch, sizeof(scratch), "RM_%s", rm_itemname);
		if (setenv(scratch, tempstruct->def, 1)) {
			snprintf(uem, sizeof(uem), "setenv() puked - %s!\n", strerror(errno));
			die();
		}
		setenvcounter++;
		if (setenvcounter > MAX_SETENV) {
			snprintf(uem, sizeof(uem), "Security alert: too many calls to setenv(), aborting!\n");
			die();
		}

                /* if notempty is enabled and it is empty, show the inputbox again */

		if (rm_type & RMtype_notempty) {
		    if (inputfieldbuf[0] == 0)
                        goto redo_inputbox1;
		}

	    }
	    flicker = 1;
	    goto label_skipcommand;
	}
	else {
	    if (rm_type & RMtype_input) {
                redo_inputbox2:
		/* if it's an input field */
		if (makeInputBox(rm_label, rm_icaption, tempstruct->def, (tempstruct->type & RMtype_password) ? 1 : 0)) {
		    flicker = 1;
		    goto label_skipcommand;
		}

		/* free old default in structure and allocate new */
		if (tempstruct->def)
		    free(tempstruct->def);
		if ((tempstruct->def = strdup(inputfieldbuf)) == NULL) {
		    snprintf(uem, sizeof(uem), "strdup() puked - %s!\n", strerror(errno));
		    die();
		}

		/* set RM_INPUT environment variable */
		if (setenv("RM_INPUT", inputfieldbuf, 1)) {
		    snprintf(uem, sizeof(uem), "setenv() puked - %s!\n", strerror(errno));
		    die();
		}
		setenvcounter++;
		if (setenvcounter > MAX_SETENV) {
		    snprintf(uem, sizeof(uem), "Security alert: too many calls to setenv(), aborting!\n");
		    die();
		}

                /* if notempty is enabled and it is empty, show the inputbox again */

		if (rm_type & RMtype_notempty) {
		    if (inputfieldbuf[0] == 0)
                        goto redo_inputbox2;
		}

		/* go and execute command... */
	    }
	}

	if (rm_type & RMtype_yesno) {
		if (makeYesNoBox(rm_label, rm_icaption, tempstruct->def)) {
			flicker = 1;
			goto label_skipcommand;
		}
	}

/* label_docommand: */

	if (rm_type & RMtype_setenvrmitem) {
	    unsetenv("RM_ITEM");
	    if (setenv("RM_ITEM", rm_itemname, 0)) {
		snprintf(uem, sizeof(uem), "setenv() puked - %s!\n", strerror(errno));
		die();
	    }
	    setenvcounter++;
	    if (setenvcounter > MAX_SETENV) {
		snprintf(uem, sizeof(uem), "Security alert: too many calls to setenv(), aborting!\n");
		die();
	    }
	}

	/* is this item named QUIT? if so, doquit(). */
	if (!strcmp(rm_itemname, "QUIT")) {
		dospecialquit(rm_command, rm_type);
	}

	printf("\033[0m" CLS);
	fflush(stdout);

	tcsetattr(STDIN_FILENO, TCSANOW, &oldtty);	/* enable echoing */
	system(rm_command);
	tcsetattr(STDIN_FILENO, TCSANOW, &newtty);	/* disable echoing */

	if (rm_type & RMtype_pause) {
		printf("\nPress any key to continue...\n");
		read(STDIN_FILENO, foochar, 1);
	}

	if (rm_type & RMtype_input)
		unsetenv("RM_INPUT");

	flicker = 1;

label_skipcommand:
	disable_sigwinch = 0;

        if (rm_type & RMtype_chain) {
	    if (flicker) {
		    printf("%s\033[2J", backgroundcolor);
		    fflush(stdout);
		    drawborders();
	    }
	    if (drawmenu(position)) {
		    snprintf(uem, sizeof(uem), "drawmenu() failed!");
		    die();
	    }
            return(docommand(pos+1));
        }

	return 0;
}

/*****************************************************************************/

void sighandler(int sig) {
	if (sig == SIGINT) {
		/* ignore all sigint's */
	}
	else if (sig == SIGQUIT) {
		if (allowquit)
			doquit();
	}
	else if (sig == SIGTERM) {
		if (allowquit)
			doquit();
	}
	else if (sig == SIGCONT) {
		tcsetattr(STDIN_FILENO, TCSANOW, &newtty);	/* re-set tty */
		if (!disable_sigwinch) {
			printf("\033[0;0r%s\033[2J", backgroundcolor);
			fflush(stdout);
			drawborders();
			if (drawmenu(position)) {
                        	snprintf(uem, sizeof(uem), "drawmenu() failed!");
				die();
                        }
		}
	}
	else if (sig == SIGWINCH) {
		if (ioctl(STDIN_FILENO, TIOCGWINSZ, &wsize) < 0) {
			snprintf(uem, sizeof(uem), "ioctl(): %s!", strerror(errno));
			die();
		}
		if (!disable_sigwinch) {
			printf("\033[0;0r%s\033[2J", backgroundcolor);
			fflush(stdout);
			drawborders();
			if (drawmenu(position)) {
                        	snprintf(uem, sizeof(uem), "drawmenu() failed!");
				die();
                        }

		}
	}
	else {
		/* no action for signal */
	}
	signal(sig, sighandler);
}

/*****************************************************************************/

void menufoo(void) {
	unsigned int key = 0, i, visiblepos = 0;
        struct menustruct *tempstruct;

	drawborders();

	if (drawmenu(position)) {
		snprintf(uem, sizeof(uem), "drawmenu() failed!");
		die();
	}

        /* get the visiblepos for this position */
	for (i = 0; i < menuitems; i++) {
	    if ((tempstruct = structs[i]) == NULL)
		    break;
            if (i >= position)
                    break;
            if (!(tempstruct->type & RMtype_hidden))
                visiblepos++;
	}

	while (1) {
		key = readkey();

		if (key == key_UP) {
			if (position > min_position) {
				position--;
				if (drawmenu(position)) {
					snprintf(uem, sizeof(uem), "drawmenu() failed!");
					die();
				}
			}
		}
		else if (key == key_DOWN) {
			if (position < max_position) {
				position++;
				if (drawmenu(position)) {
					snprintf(uem, sizeof(uem), "drawmenu() failed!");
					die();
				}
			}
		}
		else if (key == key_PGUP) {
		    if (position > min_position) {
			if (position > maxmenuitemstodisplay) {
                            i = maxmenuitemstodisplay;
                            while (i) {
                                if (position > 0) {
                                    position--;
                                    if ((tempstruct = structs[position]) == NULL)
                                        break;
                                    if (!(tempstruct->type & RMtype_hidden)) {
                                        i--;
                                    }
                                } else {
                                    i--;
                                }
                            }
			    if (position < min_position)
				position = min_position;
			}
			else {
			    position = min_position;
			}

			if (drawmenu(position)) {
			    snprintf(uem, sizeof(uem), "drawmenu() failed!");
			    die();
			}
		    }
		}
		else if (key == key_PGDOWN) {
		    if (position != max_position) {
			if ((position+maxmenuitemstodisplay) < max_position) {
                            i = maxmenuitemstodisplay;
                            while (i) {
                                if (position < max_position) {
                                    position++;
                                    if ((tempstruct = structs[position]) == NULL)
                                        break;
                                    if (!(tempstruct->type & RMtype_hidden)) {
                                        i--;
                                    }
                                } else {
                                    i--;
                                }
                            }

			    if (position > max_position)
				position = max_position;
			}
			else {
			    position = max_position;
			}

			if (drawmenu(position)) {
			    snprintf(uem, sizeof(uem), "drawmenu() failed!");
			    die();
			}
		    }
		}
		else if (key == key_HOME) {
			if (position > min_position ) {
				position = min_position;
				if (drawmenu(position)) {
					snprintf(uem, sizeof(uem), "drawmenu() failed!");
					die();
				}
			}
		}
		else if (key == key_END) {
			if (position != max_position) {
				position = max_position;
				if (drawmenu(position)) {
					snprintf(uem, sizeof(uem), "drawmenu() failed!");
					die();
				}
			}
		}
		else if (key == key_F10) {
			break;
		}
		else if ((key >= 0x10000000) && (key < 0x2000000)) {
			/* filter out any other keys generated by readkey() */
		}
		else {
			if ((key == 'q') || (key == 'Q'))
				break;
			else if (key == RETURN || ((key == SPACE) && (((struct menustruct **)structs)[position]->type & (RMtype_checkbox|RMtype_radiobutton)))) {
				if (docommand(position)) {
					snprintf(uem, sizeof(uem), "docommand() failed!");
					die();
				}

				if (flicker) {
					printf("%s\033[2J", backgroundcolor);
					fflush(stdout);
					drawborders();
				}

				if (drawmenu(position)) {
					snprintf(uem, sizeof(uem), "drawmenu() failed!");
					die();
				}
			}
		}
	}

	return;
}

/*****************************************************************************/

int main(int argc, char **argv) {

	unsigned int i, x;
	struct menustruct *tempstruct;
        char *tempp;
        char *autolist = NULL;

	/* check command line arguments here */

	while ((i = getopt(argc, argv, "hVqf:c:g:a:e:")) != EOF) {
	    switch(i) {
		case 'h':
		    printhelp();
		    return 0;
		case 'V':
		    printf("%s\n", copyright);
		    return 0;
		case 'q':
		    allowquit = 0;
		    break;
		case 'f':
		    /* read menu from file */
		    readfromfile = optarg;
		    break;
		case 'c':
		    /* select colorscheme */
		    colorscheme = atoi(optarg);
		    dontsetcolorscheme = 1;
		    break;
                case 'g':
                    /* force geometry */
                    if (optarg) {
                        tempp = strtok(optarg, "x");
                        if (tempp)
                            forced_width = atoi(tempp);
                        tempp = strtok(NULL, "x");
                        if (tempp)
                            forced_height = atoi(tempp);
                    }
                    break;
                case 'a':
                    /* items to mark as auto (comma separated list) */
                    if (optarg) {
                        autolist = strdup(optarg);
                    }
                    break;
                case 'e':
                    /* exitafterauto */
                    exitafterauto_overrider = 1;
                    if (atoi(optarg)) {
                        exitafterauto = 1;
                    } else {
                        exitafterauto = 0;
                    }
                    break;
		default:
		    return 2;
	    }
	}

	/* bust out if stdin/stdout ain't tty */

	if (!isatty(STDIN_FILENO)) {
		fprintf(stderr, "STDIN_FILENO is not a TTY!\n");
		return 1;
	}
	if (!isatty(STDOUT_FILENO)) {
		fprintf(stderr, "STDOUT_FILENO is not a TTY!\n");
		return 1;
	}

	/*
	 * read menu from file.
	 */
	if (readfromfile) {
		if ((menufp = fopen(readfromfile, "r")) == NULL) {
			fprintf(stderr, "Can't open \"%s\" - %s!\n", readfromfile, strerror(errno));
			return 1;
		}
		menufile_currentline = 0;
		file_foreachline(menufp, menufilelinehandler);
		fclose(menufp);
	}
	else {
		fprintf(stderr, "You must specify a menu configuration file using the \"-f\" option!\n");
		return 3;
	}


	if (!menuitems) {
	    fprintf(stderr, "No menu items defined, you must have at least one item (hidden or visible).\n");
	    return 3;
	}


	/* fix potentially bad caption */

	replacechar(caption, 0x0a, 0x20);
	replacechar(caption, 0x0d, 0x20);
	replacechar(caption, 0x09, 0x20);
	replacechar(caption, 0xff, 0x20);


	/* resolve any "numbered" bullets */
	x = 1;
	for (i = 0; i < menuitems; i++) {
		if ((tempstruct = structs[i]) == NULL)
			break;

		if (tempstruct->type & RMtype_regular)
			tempstruct->type = RMtype_regular;

		if (!(tempstruct->type & RMtype_dummy) && \
		!(tempstruct->type & RMtype_checkbox) && \
		!(tempstruct->type & RMtype_radiobutton) && \
                !(tempstruct->type & RMtype_hidden) && \
		!strcmp(tempstruct->bullet, "NUMBERED")) {

			snprintf(scratch, sizeof(scratch), "%u.", x++);
			free(tempstruct->bullet);
			if ((tempstruct->bullet = strdup(scratch)) == NULL) {
				fprintf(stderr, "strdup() puked - %s!\n", strerror(errno));
				return 1;
			}
		}

		/* we don't want any defaulted "NUMBERED" bullets to hang around */
		if (!strcmp(tempstruct->bullet, "NUMBERED")) {
			free(tempstruct->bullet);
			if ((tempstruct->bullet = strdup("")) == NULL) {
				fprintf(stderr, "strdup() puked - %s!\n", strerror(errno));
				return 1;
			}
		}
	}

	/* find out the longest menu label and the longest bullet */
	for (i = 0; i < menuitems; i++) {
	    if ((tempstruct = structs[i]) == NULL)
		    break;

            if (!(tempstruct->type & RMtype_hidden)) {
                x = strlen(tempstruct->label);

	        if (x > longestItem)
		        longestItem = x;

	        if ((tempstruct->type & RMtype_radiobutton) || (tempstruct->type & RMtype_checkbox))
		        x = 3;
	        else
		        x = strlen(tempstruct->bullet);

	        if (x > longestBullet)
		        longestBullet = x;
            }
	}


	/* count all "auto" types */
	for (i = 0; i < menuitems; i++) {
	    if ((tempstruct = structs[i]) == NULL)
		    break;
            if (tempstruct->type & RMtype_auto)
                automenuitems++;
	}

	/* find out min_position and max_position  */

	min_position = 0;
	max_position = menuitems-1;

	for (i = 0; i < menuitems; i++) {
	    if ((tempstruct = structs[i]) == NULL)
		break;
            if ((tempstruct->type & RMtype_dummy) || (tempstruct->type & RMtype_hidden))
                continue;
            else
                break;
	}
	min_position += i;

	for (i = max_position; i; i--) {
	    if ((tempstruct = structs[i]) == NULL)
		break;
            if ((tempstruct->type & RMtype_dummy) || (tempstruct->type & RMtype_hidden))
                continue;
            else
		break;
	}
	max_position = i;

	/* check consistency */

	if ((tempstruct = structs[menuitems-1]) == NULL) {
		fprintf(stderr, "Oops, menuitems are fucked up!\n");
		return 1;
	}

	/* get window size */

        if (forced_width && forced_height) {
            wsize.ws_col = forced_width;
            wsize.ws_row = forced_height;
        } else {
	    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize) < 0) {
		    fprintf(stderr, "ioctl(): %s!\n", strerror(errno));
		    return 1;
	    }
        }

	if ((wsize.ws_col < strlen(bottomrowmsg)) || (wsize.ws_col < strlen(caption))) {
		fprintf(stderr, "Window width is too small!\n");
		return 1;
	}


	/* save old tty attributes */
	tcgetattr(STDIN_FILENO, &oldtty);
	newtty = oldtty;
	/* Turn off echo and canonical mode */
	newtty.c_lflag &= ~(ECHO|ECHONL|ICANON);
	/* manipulate the tty; disable echoing */
	tcsetattr(STDIN_FILENO, TCSANOW, &newtty);


	if (colorscheme == 1) {
		backgroundcolor = cs01_bgcolor;
	}
	else if (colorscheme == 2) {
		backgroundcolor = cs02_bgcolor;
	}
	else if (colorscheme == 3) {
		backgroundcolor = cs03_bgcolor;
	}
	else if (colorscheme == 4) {
		backgroundcolor = cs04_bgcolor;
	}
	else if (colorscheme == 5) {
		backgroundcolor = cs05_bgcolor;
	}
	else if (colorscheme == 6) {
		backgroundcolor = cs06_bgcolor;
	}
	else if (colorscheme == 7) {
		backgroundcolor = cs07_bgcolor;
	}
	else if (colorscheme == 8) {
		backgroundcolor = cs08_bgcolor;
	}
	else if (colorscheme == 9) {
		backgroundcolor = cs09_bgcolor;
	}
	else {
		backgroundcolor = cs00_bgcolor;
	}

	/* catch a few signals */
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGQUIT, sighandler);
	signal(SIGCONT, sighandler);
	signal(SIGWINCH, sighandler);



        /* execute all items listed in the -a command line option */
        /* we do it this way in order to preserve the order in which the user lists the items */
        if (autolist) {
            tempp = strtok(autolist, ",");
            while (tempp) {
                for (i = 0; i < menuitems; i++) {
                    if ((tempstruct = structs[i]) == NULL)
                        break;
                    if (!strcmp(tempstruct->name, tempp)) {
                        printf("\033[0;0r%s\033[2J", backgroundcolor);
                        fflush(stdout);
	                drawborders();
	                if (drawmenu(position)) {
                            snprintf(uem, sizeof(uem), "drawmenu() failed!");
                            die();
	                }
                        docommand(i);
                        /* prevent this item from being executed twice... */
                        if (tempstruct->type & RMtype_auto)
                            tempstruct->type ^= RMtype_auto;
                    }
                }
                tempp = strtok(NULL, ",");
            }
            if (exitafterauto) {
                doquit();
            }
        }

        /* execute all "auto" types, including those in the -a list (autolist command line option) */
        if (automenuitems) {
            for (i = 0; i < menuitems; i++) {
	        if ((tempstruct = structs[i]) == NULL)
                    break;

                if (tempstruct->type & RMtype_auto) {
                    printf("\033[0;0r%s\033[2J", backgroundcolor);
                    fflush(stdout);
	            drawborders();
	            if (drawmenu(position)) {
                        snprintf(uem, sizeof(uem), "drawmenu() failed!");
                        die();
	            }
                    docommand(i);
                }
            }
            if (exitafterauto) {
                doquit();
            }
        }

	/* in case allowquit == 0, doquit() won't exit. */
	while (1) {
		printf("\033[0;0r%s\033[2J", backgroundcolor);
		fflush(stdout);
		menufoo();
		if (allowquit)
			doquit();
	}

	return 0;
}
