
/* Copyright 2003, Century Software */

#ifndef NXWIDGETS_H
#define NXWIDGETS_H

#define MWINCLUDECOLORS

extern "C"
{
#include <nano-X.h>
#include <nxdraw.h>
}

GR_WINDOW_ID createNXApp(char *title, int xsize, int ysize);
void killNXApp(GR_WINDOW_ID app);
GR_WINDOW_ID createNXWidget(GR_WINDOW_ID parent, int x, int y, int width,
			    int height);
void registerNXWidgetCallback(GR_WINDOW_ID wid, int event,
			      GR_FNCALLBACKEVENT callback);
int fireNXWidgetCallback(GR_WINDOW_ID wid, GR_EVENT * event);
void realizeNXWidget(GR_WINDOW_ID widget);
GR_WINDOW_ID createNXDialog(GR_WINDOW_ID parent, char *str);
void popupNXDialog(GR_WINDOW_ID dialog, char *str);

#endif
