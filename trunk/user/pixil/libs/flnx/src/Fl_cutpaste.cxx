//
// "$Id$"
//
// Cut/paste code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

// Implementation of cut and paste.

// This is seperated from Fl.C mostly to test Fl::add_handler().
// But this will save a small amount of code size in a program that
// has no text editing fields or other things that call cut or paste.

#ifdef WIN32
#include "Fl_cutpaste_win32.cxx"
#else

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <string.h>

static char *selection_buffer;
static int selection_length;
static int selection_buffer_length;
static char beenhere;

extern Fl_Widget *fl_selection_requestor; // widget doing request_paste()

static int selection_xevent_handler(int) {

  switch (fl_xevent->type) {
#ifndef NANO_X  //tanghao
  case SelectionNotify: {
    if (!fl_selection_requestor) return 0;
    static char *pastebuffer;
    if (pastebuffer) {XFree(pastebuffer); pastebuffer = 0;}
    if (fl_xevent->xselection.property != 0) {
      Atom a; int f; unsigned long n,b;
      if (!XGetWindowProperty(fl_display,
			      fl_xevent->xselection.requestor,
			      fl_xevent->xselection.property,
			      0,100000,1,0,&a,&f,&n,&b,
			      (unsigned char**)&pastebuffer)) {
	Fl::e_text = pastebuffer;
	Fl::e_length = int(n);
	fl_selection_requestor->handle(FL_PASTE);
      }
    }}
    return 1;

  case SelectionClear:
    Fl::selection_owner(0);
    return 1;

  case SelectionRequest: {
    XSelectionEvent e;
    e.type = SelectionNotify;
    e.display = fl_display;
    e.requestor = fl_xevent->xselectionrequest.requestor;
    e.selection = fl_xevent->xselectionrequest.selection;
    e.target = fl_xevent->xselectionrequest.target;
    e.time = fl_xevent->xselectionrequest.time;
    if (fl_xevent->xselectionrequest.target != XA_STRING || !selection_length) {
      e.property = 0;
    } else {
      e.property = fl_xevent->xselectionrequest.property;
    }
    if (e.property) {
      XChangeProperty(fl_display, e.requestor, e.property,
		      XA_STRING, 8, 0, (unsigned char *)selection_buffer,
		      selection_length);
    }
    XSendEvent(fl_display, e.requestor, 0, 0, (XEvent *)&e);}
    return 1;

#endif //tanghao
  default:
    return 0;
  }
}

////////////////////////////////////////////////////////////////

// Call this when a "paste" operation happens:
void Fl::paste(Fl_Widget &receiver) {
  if (selection_owner()) {
    // We already have it, do it quickly without window server.
    // Notice that the text is clobbered if set_selection is
    // called in response to FL_PASTE!
    Fl::e_text = selection_buffer;
    Fl::e_length = selection_length;
    receiver.handle(FL_PASTE);
    return;
  }
  // otherwise get the window server to return it:
  fl_selection_requestor = &receiver;
#ifndef NANO_X //tanghao
  XConvertSelection(fl_display, XA_PRIMARY, XA_STRING, XA_PRIMARY,
		    fl_xid(Fl::first_window()), fl_event_time);
#endif
  if (!beenhere) {
    Fl::add_handler(selection_xevent_handler);
    beenhere = 1;
  }
}

////////////////////////////////////////////////////////////////

// call this when you create a selection:
void Fl::selection(Fl_Widget &owner, const char *stuff, int len) {
  if (!stuff || len<0) return;
  if (len+1 > selection_buffer_length) {
    delete[] selection_buffer;
    selection_buffer = new char[len+100];
    selection_buffer_length = len+100;
  }
  memcpy(selection_buffer, stuff, len);
  selection_buffer[len] = 0; // needed for direct paste
  selection_length = len;
  selection_owner(&owner);
  static Window selxid; // window X thinks selection belongs to
#ifdef NANO_X
  if (!selxid) selxid =
		GrNewWindow(GR_ROOT_WINDOW_ID,0,0,1,1,0,0,0);
			//(GR_WINDOW_ID parent, GR_COORD x, GR_COORD y,
			//GR_SIZE width, GR_SIZE height, GR_SIZE bordersize,
			//GR_COLOR background, GR_COLOR bordercolor);
#else
  if (!selxid) selxid =
		 XCreateSimpleWindow(fl_display, 
				     RootWindow(fl_display, fl_screen),
				     0,0,1,1,0,0,0);
  XSetSelectionOwner(fl_display, XA_PRIMARY, selxid, fl_event_time);
#endif
  if (!beenhere) {
    Fl::add_handler(selection_xevent_handler);
    beenhere = 1;
  }
}

#endif

//
// End of "$Id$".
//
