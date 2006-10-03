/* Copyright 2003, Century Software */

#ifndef NX_H
#define NX_H

#define OK	1
#define CANCEL	2


void popup(GR_WINDOW_ID w);
void popdown(GR_WINDOW_ID w);

void new_game_cb(GR_WINDOW_ID w);
void quit_game_cb(GR_WINDOW_ID w);

void leave_window_eh(GR_EVENT * event);
void enter_window_eh(GR_EVENT * event);
void redraw_window_eh(GR_EVENT * event);
void button_press_eh(GR_EVENT * event);
void button_release_eh(GR_EVENT * event);
void timer_eh();

void redraw_menubar_eh(GR_EVENT * event);
void menubar_buttonup_eh(GR_EVENT * event);
void menubar_buttondown_eh(GR_EVENT * event);

#ifdef NOTUSED
GR_WINDOW_ID CreateMenuBar(const char *name, GR_WINDOW_ID parent);
GR_WINDOW_ID CreatePixmapBox(const char *name, GR_WINDOW_ID parent,
			     Pixmap pixmap, const char *text);
GR_WINDOW_ID CreateEnterText(const char *name, GR_WINDOW_ID parent,
			     const char *text, XtCallbackProc callback);
GR_WINDOW_ID CreateDialog(const char *name, GR_WINDOW_ID parent,
			  int buttonmask, Pixmap icon, const char *text,
			  const char *buttonlabel, XtCallbackProc callback);
GR_WINDOW_ID CreateDrawingArea(const char *name, GR_WINDOW_ID parent,
			       int width, int height);

void print_to_widget(GR_WINDOW_ID w, const char *str);
GR_WINDOW_ID CreateRowCol(char *name, GR_WINDOW_ID parent);

void setup_main_widgets();
void setup_other_widgets(Pixmap about, Pixmap rules, Pixmap story);

void warp_apply(GR_WINDOW_ID w, GR_WINDOW_ID text, XtPointer client_data);
void enter_name(GR_WINDOW_ID w, GR_WINDOW_ID text, XtPointer client_data);
#endif

#endif
