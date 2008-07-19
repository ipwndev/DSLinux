#ifndef		NXSLIDER_INCLUDED
#define		NXSLIDER_INCLUDED	1

#include <FL/Fl.H>
#include <FL/Fl_Slider.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Valuator.H>
#include <par.h>

class NxSlider : public Fl_Valuator
{

  float slider_size_;
  float slider_size_min_;
  uchar slider_;
  FL_EXPORT void _NxSlider();
  FL_EXPORT void draw_bg(int, int, int, int);

 protected:
  int handle(int);
  Fl_Color scroll_tray;
  Fl_Color scroll_face;

 public:
  void slider_hor_lines(int x, int y, int w, int h, int W, Fl_Color c);
  void slider_ver_lines(int x, int y, int w, int h, int W, Fl_Color c);
  int scrollvalue(int windowtop,int windowsize,int first,int totalsize);


  NxSlider(int x, int y, int w, int h, const char *l=0);
  void draw(int x, int y, int w, int h);
  void draw();

  FL_EXPORT void bounds(float a, float b);
  float slider_size() const {return slider_size_;}
  FL_EXPORT void slider_size(float v);
  Fl_Boxtype slider() const {return (Fl_Boxtype)slider_;}
  void slider(Fl_Boxtype c) {slider_ = c;}
  FL_EXPORT int handle(int, int, int, int, int);

}; // end of NxSlider::NxSlider()

#endif	//	NXSLIDER_INCLUDED
