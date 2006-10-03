#ifndef		NXSCROLL_INCLUDED
#define		NXSCROLL_INCLUDED	1

#include <FL/Fl_Group.H>
#include "nxscrollbar.h"
#include <FL/Fl_Scrollbar.H>

class NxScroll : public Fl_Group
{
  int save_h;
  bool move;
  bool resize_;
  int xposition_, yposition_;
  int width_, height_;
  int oldx, oldy;
  static FL_EXPORT void hscrollbar_cb(Fl_Widget*, void*);
  static FL_EXPORT void scrollbar_cb(Fl_Widget*, void*);
  FL_EXPORT void fix_scrollbar_order();
  static FL_EXPORT void draw_clip(void*,int,int,int,int);
  FL_EXPORT void bbox(int&,int&,int&,int&);

 protected:
  FL_EXPORT void draw();

 public:
  NxScrollbar scrollbar;
  NxScrollbar hscrollbar;

  void movable(bool flag) { move = flag; }
  void resize(bool flag) { resize_ = flag; }
  FL_EXPORT void resize(int,int,int,int);
  FL_EXPORT int handle(int);

  enum { // values for type()
    HORIZONTAL = 1,
    VERTICAL = 2,
    BOTH = 3,
    ALWAYS_ON = 4,
    HORIZONTAL_ALWAYS = 5,
    VERTICAL_ALWAYS = 6,
    BOTH_ALWAYS = 7
  };

  int xposition() const {return xposition_;}
  int yposition() const {return yposition_;}
  FL_EXPORT void position(int, int);

  NxScroll(int x, int y, int w, int h, const char *l=0);
}; // end of NxSlider::NxScroll()

#endif	//	NXSCROLL_INCLUDED
