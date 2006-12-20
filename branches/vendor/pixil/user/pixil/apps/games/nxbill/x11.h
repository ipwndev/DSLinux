
#ifndef X11_WIDGETS_H
#define X11_WIDGETS_H

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xresource.h>
#include <X11/xpm.h>

#ifdef VMS
#include <signal.h>
#include <X11VMS/vmsutil.h>
#endif

#include "UI.h"

#define OK	1
#define CANCEL	2

void popup(Widget w, Widget * box, XtPointer client_data);
void popdown(Widget w, XtPointer call_data, XtPointer client_data);

void new_game_cb(Widget w, XtPointer client_data, XtPointer call_data);
void quit_game_cb(Widget w, XtPointer call_data, XtPointer client_data);

void leave_window_eh(Widget w, XtPointer client_data, XEvent * event);
void enter_window_eh(Widget w, XtPointer client_data, XEvent * event);
void redraw_window_eh(Widget w, XtPointer client_data, XEvent * event);
void button_press_eh(Widget w, XtPointer data, XButtonEvent * event);
void button_release_eh(Widget w, XtPointer data, XButtonEvent * event);
void timer_eh(XtPointer client_data, XtIntervalId * timer_id);

Widget CreateMenuBar(const char *name, Widget parent);
Widget CreatePixmapBox(const char *name, Widget parent, Pixmap pixmap,
		       const char *text);
Widget CreateEnterText(const char *name, Widget parent, const char *text,
		       XtCallbackProc callback);
Widget CreateDialog(const char *name, Widget parent, int buttonmask,
		    Pixmap icon, const char *text, const char *buttonlabel,
		    XtCallbackProc callback);
Widget CreateDrawingArea(const char *name, Widget parent, int width,
			 int height);

void print_to_widget(Widget w, const char *str);
Widget CreateRowCol(const char *name, Widget parent);

void setup_main_widgets();
void setup_other_widgets(Pixmap about, Pixmap rules, Pixmap story);

void warp_apply(Widget w, Widget text, XtPointer client_data);
void enter_name(Widget w, Widget text, XtPointer client_data);

#endif
