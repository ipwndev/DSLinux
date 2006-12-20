#ifndef _FL_BETTER_WINDOW_H_
#define _FL_BETTER_WINDOW_H_

#include <FL/Fl_Window.H>

class Fl_Better_Window : public Fl_Window
{
 public:
  Fl_Better_Window (int x, int y, int w, int h, const char *l = 0) 
    : Fl_Window (x, y, w, h, l) {}

  static int get_window_borders (Fl_Window *win, 
                                 int &left, int &right, 
                                 int &top, int &bottom);
  
  int get_window_borders (int &left, int &right, 
			  int &top, int &bottom) 
    { 
      return get_window_borders (this, left, right, top, bottom);
    }
  
};

#endif
