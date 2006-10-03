#include <FL/Enumerations.H>
#include "nxscroll.h"

#define SLIDER_WIDTH 12

NxScroll::NxScroll(int x, int y, int w, int h, const char *l) :
  Fl_Group(x, y, w, h, l),
  scrollbar(x+w-SLIDER_WIDTH,y,SLIDER_WIDTH,h-SLIDER_WIDTH),
  hscrollbar(x,y+h-SLIDER_WIDTH,w-SLIDER_WIDTH,SLIDER_WIDTH) 
{
  save_h = h;
  move = true;
  resize_ = true;
  type(BOTH);
  xposition_ = 0;
  yposition_ = 0;

  hscrollbar.type(FL_HORIZONTAL);
  hscrollbar.callback(hscrollbar_cb);
  scrollbar.callback(scrollbar_cb);

  scrollbar.align(FL_ALIGN_RIGHT);
  hscrollbar.align(FL_ALIGN_BOTTOM);

  // Provide the "look-and-feel"
  box(FL_BORDER_BOX);
  type(NxScroll::VERTICAL);
  scrollbar.size(12,scrollbar.h());
  
//  color(NxApp::Instance()->getGlobalColor(APP_BG));
 // selection_color(NxApp::Instance()->getGlobalColor(APP_SEL));

  color(FL_WHITE);
  color(FL_BLUE); 

 
} // end of NxScroll::NxScroll()


// Insure the scrollbars are the last children:
void NxScroll::fix_scrollbar_order() 
{
  Fl_Widget*const* a = array();
  if (a[children()-1] != &scrollbar) {
    Fl_Widget** a = (Fl_Widget**)array();
    int i,j; for (i = j = 0; j < children(); j++)
      if (a[j] != &hscrollbar && a[j] != &scrollbar) a[i++] = a[j];
    a[i++] = &hscrollbar;
    a[i++] = &scrollbar;
  }
}

void NxScroll::draw_clip(void* v,int X, int Y, int W, int H) {
  fl_clip(X,Y,W,H);
  NxScroll* s = (NxScroll*)v;
  // erase background if there is a boxtype:
  if (s->box() && !(s->damage()&FL_DAMAGE_ALL)) {
    fl_color(s->color());
    fl_rectf(X,Y,W,H);
  }
  Fl_Widget*const* a = s->array();
  int R = X; int B = Y; // track bottom & right edge of all children
  for (int i=s->children()-2; i--;) {
    Fl_Widget& o = **a++;
    s->draw_child(o);
    s->draw_outside_label(o);
    if (o.x()+o.w() > R) R = o.x()+o.w();
    if (o.y()+o.h() > B) B = o.y()+o.h();
  }
  // fill any area to right & bottom of widgets:
  if (R < X+W && B > Y) {
    fl_color(s->color());
    fl_rectf(R,Y,X+W-R,B-Y);
  }
  if (B < Y+H) {
    fl_color(s->color());
    fl_rectf(X,B,W,Y+H-B);
  }
  fl_pop_clip();
}

void NxScroll::bbox(int& X, int& Y, int& W, int& H) {
  X = x()+Fl::box_dx(box());
  Y = y()+Fl::box_dy(box());
  W = w()-Fl::box_dw(box());
  H = h()-Fl::box_dh(box());
  if (scrollbar.visible()) {
    W -= scrollbar.w();
    if (scrollbar.align() & FL_ALIGN_LEFT) X += scrollbar.w();
  }
  if (hscrollbar.visible()) {
    H -= hscrollbar.h();
    if (scrollbar.align() & FL_ALIGN_TOP) Y += hscrollbar.h();
  }
}

void NxScroll::draw() {
  fix_scrollbar_order();
  int X,Y,W,H; bbox(X,Y,W,H);

  uchar d = damage();

  X=X+1;
  W=W-2;
  H=H-1;	

  if (d & FL_DAMAGE_ALL) { // full redraw
    
    draw_box(box(),x(),y()-1,w(),h()+1,color());
    draw_clip(this, X, Y, W, H);
  } else {
    if (d & FL_DAMAGE_SCROLL) { // scroll the contents:
      fl_scroll(X, Y, W, H, oldx-xposition_, oldy-yposition_, draw_clip, this);
    }
    if (d & FL_DAMAGE_CHILD) { // draw damaged children
      fl_clip(X, Y, W, H);
      Fl_Widget*const* a = array();
      for (int i=children()-2; i--;) update_child(**a++);
      fl_pop_clip();
    }
  }

  // accumulate bounding box of children:
  int l = X; int r = X; int t = Y; int b = Y;
  Fl_Widget*const* a = array();
  for (int i=children()-2; i--;) {
    Fl_Object* o = *a++;
    if (o->x() < l) l = o->x();
    if (o->y() < t) t = o->y();
    if (o->x()+o->w() > r) r = o->x()+o->w();
    if (o->y()+o->h() > b) b = o->y()+o->h();
  }

  // turn the scrollbars on and off as necessary:
  for (int z = 0; z<2; z++) {
    if ((type()&VERTICAL) && (type()&ALWAYS_ON || t < Y || b > Y+H)) {
      if (!scrollbar.visible()) {
	scrollbar.set_visible();
	W -= scrollbar.w();
	d = FL_DAMAGE_ALL;
      }
    } else {
      if (scrollbar.visible()) {
	scrollbar.clear_visible();
	draw_clip(this,
		  scrollbar.align()&FL_ALIGN_LEFT ? X-scrollbar.w() : X+W,
		  Y, scrollbar.w(), H);
	W += scrollbar.w();
	d = FL_DAMAGE_ALL;
      }
    }
    if ((type()&HORIZONTAL) && (type()&ALWAYS_ON || l < X || r > X+W)) {
      if (!hscrollbar.visible()) {
	hscrollbar.set_visible();
	H -= hscrollbar.h();
	d = FL_DAMAGE_ALL;
      }
    } else {
      if (hscrollbar.visible()) {
	hscrollbar.clear_visible();
	draw_clip(this, X,
		  scrollbar.align()&FL_ALIGN_TOP ? Y-hscrollbar.h() : Y+H,
		  W, hscrollbar.h());
	H += hscrollbar.h();
	d = FL_DAMAGE_ALL;
      }
    }
  }


#if 1
  
#if 1  
  int __Y = Y - 1;
  int __H = H + 2;
  int __X = X; 
  int __W = W;

  scrollbar.resize(scrollbar.align()&FL_ALIGN_LEFT ? __X-scrollbar.w()  : __X+__W,
		   __Y, scrollbar.w(), __H);

  hscrollbar.resize(__X,
		    scrollbar.align()&FL_ALIGN_TOP ? __Y-hscrollbar.h() : __Y+__H,
		    __W, hscrollbar.h());
  
#else	
	  scrollbar.resize(scrollbar.align()&FL_ALIGN_LEFT ? X-scrollbar.w()  : X+W,
	  Y, scrollbar.w(), H);
	  
	  hscrollbar.resize(X,
	  scrollbar.align()&FL_ALIGN_TOP ? Y-hscrollbar.h() : Y+H,
	  W, hscrollbar.h());
	  #endif
#else
  scrollbar.resize(scrollbar.align()&FL_ALIGN_LEFT ? X-scrollbar.w() : X+W,
		   Y, scrollbar.w(), H);
  
  hscrollbar.resize(X,
		    scrollbar.align()&FL_ALIGN_TOP ? Y-hscrollbar.h() : Y+H,
		    W, hscrollbar.h());
#endif
  scrollbar.value(oldy = yposition_ = (Y-t), H, 0, b-t);

  hscrollbar.value(oldx = xposition_ = (X-l), W, 0, r-l);

  // draw the scrollbars:
  if (d & FL_DAMAGE_ALL) {
    draw_child(scrollbar);
    draw_child(hscrollbar);
    if (scrollbar.visible() && hscrollbar.visible()) {
      // fill in the little box in the corner
      fl_color(color());
      fl_rectf(scrollbar.x(), hscrollbar.y(), scrollbar.w(), hscrollbar.h());
    }
  } else {
    update_child(scrollbar);
    update_child(hscrollbar);
  }

	if(scrollbar.visible()) {
		Fl_Color old_color = color();
		fl_color(FL_BLACK);
		fl_line(scrollbar.x(), scrollbar.y(), scrollbar.x() + scrollbar.w(), scrollbar.y());
		fl_line(scrollbar.x(), scrollbar.y() + scrollbar.h() - 1, 
		scrollbar.x() + scrollbar.w(), scrollbar.y() + scrollbar.h() - 1);
		fl_color(old_color);
	}

	if(hscrollbar.visible()) {
		Fl_Color old_color = color();
		fl_color(FL_BLACK);
		fl_line(hscrollbar.x(), hscrollbar.y(), hscrollbar.x(), hscrollbar.y() + hscrollbar.h() - 1);
		fl_line(hscrollbar.x() + hscrollbar.w(), hscrollbar.y(), 
				hscrollbar.x() + hscrollbar.w(), hscrollbar.y() + hscrollbar.h() - 1);
		fl_color(old_color);
	}
		
}
#include <stdio.h>
void NxScroll::resize(int X, int Y, int W, int H) {

  if (!resize_) return;
  
  fix_scrollbar_order();

  int new_h;
  if ( H == save_h )
    new_h = save_h;
  else
    new_h = H - (this->y() - Y) - 5;

  if (move) {
  
    // move all the children:
    Fl_Widget*const* a = array();
    for (int i=children()-2; i--;) {
      Fl_Object* o = *a++;
      o->position(o->x()+X-x(), o->y()+Y-y());
    }
  
    Fl_Widget::resize(X,Y,W,new_h);
  
  } else {
 
    // move all the children:
    Fl_Widget*const* a = array();
    for (int i=children()-2; i--;) {
      Fl_Object* o = *a++;
      //o->position(o->x()+X-x(), o->y()+Y-y());
      o->position(o->x()+X-x(), o->y());
    }

    Fl_Widget::resize(this->x(), this->y(), W, new_h);

  }

}

void NxScroll::position(int X, int Y) {
  int dx = xposition_-X;
  int dy = yposition_-Y;

  if (!dx && !dy) return;
  xposition_ = X;
  yposition_ = Y;
  Fl_Widget*const* a = array();
  for (int i=children(); i--;) {
    Fl_Widget* o = *a++;
    if (o == &hscrollbar || o == &scrollbar) continue;
    o->position(o->x()+dx, o->y()+dy);
  }
  damage(FL_DAMAGE_SCROLL);
}

void NxScroll::hscrollbar_cb(Fl_Widget* o, void*) {
  NxScroll* s = (NxScroll*)(o->parent());
  s->position(int(((NxScrollbar*)o)->value()), s->yposition());
}

void NxScroll::scrollbar_cb(Fl_Widget* o, void*) {
  NxScroll* s = (NxScroll*)(o->parent());
  s->position(s->xposition(), int(((NxScrollbar*)o)->value()));
}

int NxScroll::handle(int event) {
  fix_scrollbar_order();
  return Fl_Group::handle(event);
}
