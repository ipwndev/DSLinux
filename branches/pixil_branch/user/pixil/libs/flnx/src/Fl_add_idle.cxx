//
// "$Id$"
//
// Idle routine support for the Fast Light Tool Kit (FLTK).
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

// Allows you to manage an arbitrary set of idle() callbacks.
// Replaces the older set_idle() call (which is used to implement this)

#include <FL/Fl.H>

struct idle_cb {
  void (*cb)(void*);
  void* data;
  idle_cb *next;
};

// the callbacks are stored linked in a ring.  last points at the one
// just called, first at the next to call.  last->next == first.

static idle_cb* first;
static idle_cb* last;
static idle_cb* freelist;

static void call_idle() {
  idle_cb* p = first;
  last = p; first = p->next;
  p->cb(p->data); // this may call add_idle() or remove_idle()!
}

void Fl::add_idle(void (*cb)(void*), void* data) {
  idle_cb* p = freelist;
  if (p) freelist = p->next;
  else p = new idle_cb;
  p->cb = cb;
  p->data = data;
  if (first) {
    last->next = p;
    p->next = first;
    first = p;
  } else {
    first = last = p;
    p->next = p;
    set_idle(call_idle);
  }
}

void Fl::remove_idle(void (*cb)(void*), void* data) {
  idle_cb* p = first;
  if (!p) return;
  idle_cb* l = last;
  for (;; p = p->next) {
    if (p->cb == cb && p->data == data) break;
    if (p==last) return; // not found
    l = p;
  }
  if (l == p) { // only one
    first = last = 0;
    set_idle(0);
  } else {
    last = l;
    first = l->next = p->next;
  }
  p->next = freelist;
  freelist = p;
}

//
// End of "$Id$".
//
