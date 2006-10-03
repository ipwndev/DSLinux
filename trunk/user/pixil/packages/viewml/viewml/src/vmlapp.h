#ifndef __VMLAPP_H
#define __VMLAPP_H

#include <Fl_Input.H>
#ifdef USING_FLEK_LIB
#include <Flve_Combo.H>
#include <Fl_Animator.H>
#endif
#include <Fl_Button.H>
#include "http.h"
#include "html.h"

class VMLAppWindow;

extern VMLAppWindow * g_AppWindow;

extern int g_AppWidth;
extern int g_AppHeight;
#ifdef USING_FLEK_LIB
extern Fl_Animator *g_Logo;
extern Flve_Combo	*g_URL;
#else
extern Fl_Button * g_Logo;
extern Fl_Input * g_URL;
#endif
extern HTML_WIDGET * g_HTML;

// some defines for width/height calculations

#define APPWIDTH (g_AppWidth)
#define APPHEIGHT (g_AppHeight)

#define TITLEHEIGHT 30

#define SCROLLWIDTH 0
#define SCROLLHEIGHT 0
#define SCROLLPAD 0

#define WIDTH (APPWIDTH - SCROLLWIDTH - SCROLLPAD - SCROLLPAD - SCROLLPAD)
#define HEIGHT (APPHEIGHT - TITLEHEIGHT - SCROLLHEIGHT - SCROLLPAD - SCROLLPAD)

// The following define is necessary because we are missing the current copy
// of FLNX from CVS. The Fl_Window_Resize has special callbacks from flnx
// to handle external window resizes and then provide those to the application.
// Undefine the following to use just the standard Fl_Window


#define HAVE_RESIZE_WINDOW 0

//#ifdef HAVE_RESIZE_WINDOW
#if 0
  #include <Fl_Window_Resize.H>
  #define PARENT_WINDOW Fl_Window_Resize
#else
  #include <Fl_Window.H>
  #define PARENT_WINDOW Fl_Window
#endif

class VMLAppWindow : public PARENT_WINDOW
{
public:
  VMLAppWindow(int x, int y, int w, int h) : PARENT_WINDOW(x,y,w,h,"ViewML Browser") { }
#ifdef HAVE_RESIZE_WINDOW
  virtual void resize_notify(int x, int y, int w, int h);
#endif
  virtual int handle(int event);
};

#endif

