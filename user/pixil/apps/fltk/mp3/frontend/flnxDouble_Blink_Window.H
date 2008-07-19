
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

class double_blink_window : public Fl_Double_Window {
  void draw() { 
    cout << "parent: " << (void*)this << endl;
    for ( int i=0; i < children(); i++ ) {
      draw_child( *child(i) );
      cout << "child: " << (void*)child(i) << endl;
    }
  }
        
 public:
  double_blink_window(int x, int y, int w, int h, const char *l=0)
    : Fl_Double_Window(x,y,w,h,l) { resizable(this); }
  double_blink_window(int w, int h, const char *l=0)
    : Fl_Double_Window(w,h,l) { resizable(this); }
};
