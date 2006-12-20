#include "vmlapp.h"
#include <Fl.H>


#ifdef HAVE_RESIZE_WINDOW

void VMLAppWindow::resize_notify(int x, int y, int w, int h) 
{
  cerr << "resize_notify of " << x << " " << y << " " << w << " " << h << "\n";
  if ( g_AppWindow ) {
    g_AppWidth = w;
    g_AppHeight = h;
    
    g_AppWindow->w(w);
    g_AppWindow->h(h);
    if(g_URL)
      g_URL->resize(81, 5, APPWIDTH - 110, 20);
    if(g_Logo)
      g_Logo->resize(APPWIDTH-24, 5, 20, 20);
    g_HTML->setGeometry(SCROLLPAD,TITLEHEIGHT,WIDTH,HEIGHT);
    g_AppWindow->redraw();
  }

}
#endif

static int send(Fl_Widget* o, int event) {
  if (o->type() < FL_WINDOW) return o->handle(event);
  int save_x = Fl::e_x; Fl::e_x -= o->x();
  int save_y = Fl::e_y; Fl::e_y -= o->y();
  int ret = o->handle(event);
  Fl::e_y = save_y;
  Fl::e_x = save_x;
  return ret;
}

int VMLAppWindow::handle(int event)
{
  Fl_Widget*const* a = array();
  int i;
  Fl_Widget * o;
  if(event == FL_RELEASE) {
    for (i = children(); i--;) {
      o = a[i];
      if (o->takesevents() && Fl::event_inside(o)) {
	if (send(o,FL_RELEASE)) {
	  return 1;
	}
      }
    }
  } else {
    return PARENT_WINDOW::handle(event);
  }
}


