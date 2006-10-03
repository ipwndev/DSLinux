 /*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "objects.h"
#include "x11.h"
#include "menu.h"

#include <X11/Xaw/XawInit.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/AsciiText.h>

extern Widget scorebox, highscorebox;

Widget
CreateMenuBar(const char *name, Widget parent)
{
    int i, j;
    Widget menubar, temp;
    menubar = XtVaCreateManagedWidget(name, boxWidgetClass, parent,
				      XtNborderWidth, 0, XtNorientation,
				      XtEhorizontal, NULL);
    for (j = 0; j < menu.size; j++) {
	temp = XtCreateManagedWidget(menu.submenu[j].name,
				     menuButtonWidgetClass, menubar, NULL, 0);
	menu.submenu[j].pshell = XtCreatePopupShell("menu",
						    simpleMenuWidgetClass,
						    temp, NULL, 0);
	for (i = 0; i < menu.submenu[j].size; i++) {
	    if (strlen(menu.submenu[j].button[i].name)) {
		temp = XtCreateManagedWidget(menu.submenu[j].button[i].name,
					     smeBSBObjectClass,
					     menu.submenu[j].pshell, NULL, 0);
		XtAddCallback(temp, XtNcallback,
			      (XtCallbackProc) popup,
			      menu.submenu[j].button[i].dialog);
	    } else
		XtCreateManagedWidget("", smeLineObjectClass,
				      menu.submenu[j].pshell, NULL, 0);
	}
    }
    return menubar;
}

void
close_window(Widget w, XtPointer client_data, XtPointer call_data)
{
    XtPopdown(XtParent(XtParent(w)));
}

Widget
CreatePixmapBox(const char *name, Widget parent, Pixmap pixmap,
		const char *text)
{
    Widget base, pshell, button;

    pshell = XtCreatePopupShell(name, transientShellWidgetClass,
				parent, NULL, 0);

    base = CreateRowCol("", pshell);

    XtVaCreateManagedWidget("", labelWidgetClass, base,
			    XtNbitmap, game.logo.pix, XtNborderWidth, 0,
			    NULL);

    if (pixmap)
	XtVaCreateManagedWidget("", labelWidgetClass, base,
				XtNbitmap, pixmap, XtNborderWidth, 0, NULL);
    if (text)
	XtVaCreateManagedWidget("", labelWidgetClass, base,
				XtNlabel, text, XtNborderWidth, 0, NULL);

    button = XtVaCreateManagedWidget("OK", commandWidgetClass, base, NULL);
    XtAddCallback(button, XtNcallback, (XtCallbackProc) close_window, NULL);
    return base;
}

void
warp_apply(Widget w, Widget text, XtPointer client_data)
{
    char *str;
    XtVaGetValues(text, XtNstring, &str, NULL);
    game.warp_to_level(atoi(str));
}

void
enter_name(Widget w, Widget text, XtPointer client_data)
{
    char *str, *nl;
    XtVaGetValues(text, XtNstring, &str, NULL);
    if (!str[0])
	strcpy(str, "Anonymous");
    else if ((nl = strchr(str, '\n')))
	*nl = 0;
    if (strlen(str) > 20)
	str[20] = 0;		/* truncate string if too long */
    scores.recalc(str);
}

Widget
CreateEnterText(const char *name, Widget parent, const char *text,
		XtCallbackProc callback)
{
    Widget base, pshell, button, textfield;
    pshell = XtCreatePopupShell(name, transientShellWidgetClass,
				parent, NULL, 0);
    base = CreateRowCol("", pshell);
    XtVaCreateManagedWidget("", labelWidgetClass, base,
			    XtNlabel, text, XtNborderWidth, 0, NULL);

    textfield = XtVaCreateManagedWidget("", asciiTextWidgetClass, base,
					XtNeditType, XawtextEdit, XtNstring,
					"", XtNwidth, 200, NULL);

    button = XtVaCreateManagedWidget("OK", commandWidgetClass, base, NULL);
    XtAddCallback(button, XtNcallback, (XtCallbackProc) callback, textfield);
    XtAddCallback(button, XtNcallback, (XtCallbackProc) close_window, NULL);
    button = XtVaCreateManagedWidget("Cancel", commandWidgetClass, base,
				     NULL);
    XtAddCallback(button, XtNcallback, (XtCallbackProc) close_window, NULL);
    return base;
}

Widget
CreateDialog(const char *name, Widget parent, int buttonmask,
	     Pixmap icon, const char *text, const char *buttonlabel,
	     XtCallbackProc callback)
{
    Widget base, pshell, button;
    char *ttext = (char *) malloc(strlen(text) + 5);
    pshell = XtCreatePopupShell(name, transientShellWidgetClass,
				parent, NULL, 0);
    base = CreateRowCol("base", pshell);
    strcpy(ttext, text);
    if (strlen(ttext) < 12)
	strcat(ttext, "     ");
    XtVaCreateManagedWidget("label", labelWidgetClass, base,
			    XtNlabel, ttext, XtNborderWidth, 0, NULL);
    if (icon)
	XtVaCreateManagedWidget("", labelWidgetClass, base,
				XtNbitmap, icon, XtNborderWidth, 0, NULL);
    if (buttonmask & OK) {
	if (!buttonlabel)
	    buttonlabel = "OK";
	button = XtVaCreateManagedWidget(buttonlabel,
					 commandWidgetClass, base, NULL);
	if (callback)
	    XtAddCallback(button, XtNcallback,
			  (XtCallbackProc) callback, NULL);
	XtAddCallback(button, XtNcallback,
		      (XtCallbackProc) close_window, NULL);
    }
    if (buttonmask & CANCEL) {
	button = XtVaCreateManagedWidget("Cancel", commandWidgetClass,
					 base, NULL);
	XtAddCallback(button, XtNcallback,
		      (XtCallbackProc) close_window, NULL);
    }
    return base;
}

Widget
CreateDrawingArea(const char *name, Widget parent, int width, int height)
{
    return XtVaCreateManagedWidget(name, coreWidgetClass, parent, XtNwidth,
				   width, XtNheight, height, NULL);
}

void
UI::update_hsbox(char *str)
{
    WidgetList t;
    XtVaGetValues(highscorebox, XtNchildren, &t, NULL);
    XtVaSetValues(t[0], XtNlabel, str, NULL);
}

void
UI::update_scorebox(int level, int score)
{
    WidgetList t;
    char str[40];
    sprintf(str, "After Level %d:     \nYour score: %d\n", level, score);
    XtVaGetValues(scorebox, XtNchildren, &t, NULL);
    XtVaSetValues(t[0], XtNlabel, str, NULL);
}

Widget
CreateRowCol(const char *name, Widget parent)
{
    return XtCreateManagedWidget(name, boxWidgetClass, parent, NULL, 0);
}
