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

#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/Text.h>

extern Widget scorebox, highscorebox;

Widget
CreateMenuBar(const char *name, Widget parent)
{
    int i, j;
    Widget menubar, temp;
    menubar = XmCreateMenuBar(parent, name, NULL, 0);
    XtManageChild(menubar);
    for (j = 0; j < menu.size; j++) {
	menu.submenu[j].pulldown = XmCreatePulldownMenu(menubar,
							"menu", NULL, 0);
	temp = XtVaCreateManagedWidget(menu.submenu[j].name,
				       xmCascadeButtonWidgetClass, menubar,
				       XmNsubMenuId, menu.submenu[j].pulldown,
				       NULL);
	for (i = 0; i < menu.submenu[j].size; i++) {
	    temp = XtCreateManagedWidget(menu.submenu[j].button[i].name,
					 xmPushButtonWidgetClass,
					 menu.submenu[j].pulldown, NULL, 0);
	    XtAddCallback(temp, XmNactivateCallback,
			  (XtCallbackProc) popup,
			  menu.submenu[j].button[i].dialog);
	}
    }
    return menubar;
}

Widget
CreatePixmapBox(const char *name, Widget parent, Pixmap pixmap,
		const char *text)
{
    Arg wargs[2];
    XmString mstr;
    Widget dialog, base;
    mstr = XmStringCreateLtoR("", XmSTRING_DEFAULT_CHARSET);
    XtSetArg(wargs[0], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);
    XtSetArg(wargs[1], XmNmessageString, mstr);
    dialog = XmCreateMessageDialog(parent, name, wargs, 2);

    base = CreateRowCol("", dialog);

    XtVaCreateManagedWidget("", xmLabelWidgetClass, base, XmNlabelType,
			    XmPIXMAP, XmNlabelPixmap, game.logo.pix, NULL);

    if (pixmap)
	XtVaCreateManagedWidget("", xmLabelWidgetClass, base,
				XmNlabelType, XmPIXMAP, XmNlabelPixmap,
				pixmap, NULL);

    if (text) {
	mstr = XmStringCreateLtoR(text, XmSTRING_DEFAULT_CHARSET);
	XtVaCreateManagedWidget(text, xmLabelWidgetClass, base,
				XmNlabelString, mstr, NULL);
	XmStringFree(mstr);
    }

    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
    XtAddCallback(XtParent(dialog), XtNpopdownCallback,
		  (XtCallbackProc) popdown, NULL);
    return dialog;
}

void
warp_apply(Widget w, Widget text, XtPointer client_data)
{
    char *str;
    int i;
    str = XmTextGetString(text);
    i = atoi(str);
    XtFree(str);
    game.warp_to_level(i);
}

void
enter_name(Widget w, Widget text, XtPointer client_data)
{
    char *str, *nl;
    str = XmTextGetString(text);
    if (!str[0])
	strcpy(str, "Anonymous");
    else if ((nl = strchr(str, '\n')))
	*nl = 0;
    if (strlen(str) > 20)
	str[20] = 0;		/* truncate string if too long */
    scores.recalc(str);
    XtFree(str);
}

Widget
CreateEnterText(const char *name, Widget parent, const char *text,
		XtCallbackProc callback)
{
    Widget dialog;
    Arg wargs[2];
    XmString mstr = XmStringCreateLtoR(text, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(wargs[0], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);
    XtSetArg(wargs[1], XmNselectionLabelString, mstr);
    dialog = XmCreatePromptDialog(parent, name, wargs, 2);
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_APPLY_BUTTON));
    XtAddCallback(dialog, XmNokCallback, callback,
		  XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT));
    XtAddCallback(XtParent(dialog), XtNpopdownCallback, popdown, NULL);
    return dialog;
}

Widget
CreateDialog(const char *name, Widget parent, int buttonmask,
	     Pixmap icon, const char *text, const char *buttonlabel,
	     XtCallbackProc callback)
{
    Widget dialog;
    Arg wargs[2];
    XmString mstr = XmStringCreateLtoR(text, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(wargs[0], XmNdialogStyle, XmDIALOG_APPLICATION_MODAL);
    XtSetArg(wargs[1], XmNmessageString, mstr);
    dialog = XmCreateMessageDialog(parent, name, wargs, 2);
    if (icon)
	XtVaSetValues(dialog, XmNsymbolPixmap, icon, NULL);
    XmStringFree(mstr);
    if (!(buttonmask & CANCEL))
	XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
    if (buttonlabel) {
	mstr = XmStringCreateLtoR(buttonlabel, XmSTRING_DEFAULT_CHARSET);
	XtVaSetValues(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON),
		      XmNlabelString, mstr, NULL);
	XmStringFree(mstr);
    }
    if (callback)
	XtAddCallback(dialog, XmNokCallback, callback, NULL);
    XtAddCallback(XtParent(dialog), XtNpopdownCallback, popdown, NULL);
    return dialog;
}

Widget
CreateDrawingArea(const char *name, Widget parent, int width, int height)
{
    return XtVaCreateManagedWidget(name, xmDrawingAreaWidgetClass, parent,
				   XtNwidth, width, XtNheight, height,
				   XmNresizePolicy, FALSE, NULL);
}

void
UI::update_hsbox(char *str)
{
    XmString mstr = XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET);
    XtVaSetValues(XmMessageBoxGetChild(highscorebox, XmDIALOG_MESSAGE_LABEL),
		  XmNlabelString, mstr, NULL);
    XmStringFree(mstr);
}

void
UI::update_scorebox(int level, int score)
{
    char str[40];
    XmString mstr;
    sprintf(str, "After Level %d:\nYour score: %d", level, score);
    mstr = XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET);
    XtVaSetValues(XmMessageBoxGetChild(scorebox, XmDIALOG_MESSAGE_LABEL),
		  XmNlabelString, mstr, NULL);
    XmStringFree(mstr);
}

Widget
CreateRowCol(const char *name, Widget parent)
{
    return XtCreateManagedWidget(name, xmRowColumnWidgetClass, parent,
				 NULL, 0);
}
