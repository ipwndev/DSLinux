//
// "$Id$"
//
// Menu utilities for the Fast Light Tool Kit (FLTK).
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

// Methods to alter the menu in an Fl_Menu_ widget.

// These are for Forms emulation and for dynamically changing the
// menus.  They are in this source file so they are not linked in if
// not used, which is what will happen if the the program only uses
// constant menu tables.

// Not at all guaranteed to be Forms compatable, especially with any
// string with a % sign in it!

#include <FL/Fl_Menu_.H>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// If the array is this, we will double-reallocate as necessary:
static Fl_Menu_Item* local_array = 0;
static int local_array_alloc = 0; // number allocated
static int local_array_size = 0; // == size(local_array)
extern Fl_Menu_* fl_menu_array_owner; // in Fl_Menu_.cxx

// For historical reasons there are matching methods that work on a
// user-allocated array of Fl_Menu_Item.  These methods are quite
// depreciated and should not be used.  These old methods use the
// above pointers to detect if the array belongs to an Fl_Menu_
// widget, and if so it reallocates as necessary.

// Insert a single Fl_Menu_Item into an array of size at offset n,
// if this is local_array it will be reallocated if needed.
static Fl_Menu_Item* insert(
  Fl_Menu_Item* array, int size,
  int n,
  const char *text,
  int flags
) {
  if (array == local_array && size >= local_array_alloc) {
    local_array_alloc = 2*size;
    Fl_Menu_Item* newarray = new Fl_Menu_Item[local_array_alloc];
    memmove(newarray, array, size*sizeof(Fl_Menu_Item));
    delete[] local_array;
    local_array = array = newarray;
  }
  // move all the later items:
  memmove(array+n+1, array+n, sizeof(Fl_Menu_Item)*(size-n));
  // create the new item:
  Fl_Menu_Item* m = array+n;
  m->text = text ? strdup(text) : 0;
  m->shortcut_ = 0;
  m->callback_ = 0;
  m->user_data_ = 0;
  m->flags = flags;
  m->labeltype_ = m->labelfont_ = m->labelsize_ = m->labelcolor_ = 0;
  return array;
}

// Add an item.  The text is split at '|' characters to automatically
// produce submenus (actually a totally unnecessary feature as you can
// now add submenu titles directly by setting SUBMENU in the flags):
int Fl_Menu_Item::add(
  const char *text,
  int shortcut,
  Fl_Callback *cb,	
  void *data,
  int flags
) {
  Fl_Menu_Item *array = this;
  Fl_Menu_Item *m = this;
  const char *p;
  char *q;
  char buf[1024];

  int size = array==local_array ? local_array_size : array->size();
  int flags1 = 0;
  char* item;
  for (;;) {    /* do all the supermenus: */

    /* fill in the buf with name, changing \x to x: */
    q = buf;
    for (p=text; *p && *p != '/'; *q++ = *p++) if (*p=='\\') p++;
    *q = 0;

    item = buf;
    if (*item == '_') {item++; flags1 = FL_MENU_DIVIDER;}
    if (*p != '/') break; /* not a menu title */
    text = p+1;	/* point at item title */

    /* find a matching menu title: */
    for (; m->text; m = m->next())
      if (m->flags&FL_SUBMENU && !strcmp(item, m->text)) break;

    if (!m->text) { /* create a new menu */
      int n = m-array;
      array = insert(array, size, n, item, FL_SUBMENU|flags1);
      size++;
      array = insert(array, size, n+1, 0, 0);
      size++;
      m = array+n;
    }
    m++;	/* go into the submenu */
    flags1 = 0;
  }

  /* find a matching menu item: */
  for (; m->text; m = m->next())
    if (!strcmp(m->text,item)) break;

  if (!m->text) {	/* add a new menu item */
    int n = m-array;
    array = insert(array, size, n, item, flags|flags1);
    size++;
    if (flags & FL_SUBMENU) { // add submenu delimiter
      array = insert(array, size, n+1, 0, 0);
      size++;
    }
    m = array+n;
  }

  /* fill it in */
  m->shortcut_ = shortcut;
  m->callback_ = cb;
  m->user_data_ = data;
  m->flags = flags|flags1;

  if (array == local_array) local_array_size = size;
  return m-array;
}

int Fl_Menu_::add(const char *t, int s, Fl_Callback *c,void *v,int f) {
  // make this widget own the local array:
  if (this != fl_menu_array_owner) {
    if (fl_menu_array_owner) {
      Fl_Menu_* o = fl_menu_array_owner;
      // the previous owner get's its own correctly-sized array:
      int value_offset = o->value_-local_array;
      int n = local_array_size;
      Fl_Menu_Item* newMenu = o->menu_ = new Fl_Menu_Item[n];
      memcpy(newMenu, local_array, n*sizeof(Fl_Menu_Item));
      if (o->value_) o->value_ = newMenu+value_offset;
    }
    if (menu_) {
      // this already has a menu array, use it as the local one:
      delete[] local_array;
      if (!alloc) copy(menu_); // duplicate a user-provided static array
      // add to the menu's current array:
      local_array_alloc = local_array_size = size();
      local_array = menu_;
    } else {
      // start with a blank array:
      alloc = 2; // indicates that the strings can be freed
      if (local_array) {
	menu_ = local_array;
      } else {
	local_array_alloc = 15;
	local_array = menu_ = new Fl_Menu_Item[local_array_alloc];
      }
      menu_[0].text = 0;
      local_array_size = 1;
    }
    fl_menu_array_owner = this;
  }
  int r = menu_->add(t,s,c,v,f);
  // if it rellocated array we must fix the pointer:
  int value_offset = value_-menu_;
  menu_ = local_array; // in case it reallocated it
  if (value_) value_ = menu_+value_offset;
  return r;
}

// This is a Forms (and SGI GL library) compatable add function, it
// adds many menu items, with '|' seperating the menu items, and tab
// seperating the menu item names from an optional shortcut string.
int Fl_Menu_::add(const char *str) {
  char buf[128];
  int r = 0;
  while (*str) {
    int shortcut = 0;
    char *c;
    for (c = buf; *str && *str != '|'; str++) {
      if (*str == '\t') {*c++ = 0; shortcut = fl_old_shortcut(str);}
      else *c++ = *str;
    }
    *c = 0;
    r = add(buf, shortcut, 0, 0, 0);
    if (*str) str++;
  }
  return r;
}

void Fl_Menu_::replace(int i, const char *str) {
  if (i<0 || i>=size()) return;
  if (!alloc) copy(menu_);
  if (alloc > 1) {
    free((void *)menu_[i].text);
    str = strdup(str);
  }
  menu_[i].text = str;
}

void Fl_Menu_::remove(int i) {
  int n = size();
  if (i<0 || i>=n) return;
  if (!alloc) copy(menu_);
  if (alloc > 1) free((void *)menu_[i].text);
  memmove(&menu_[i],&menu_[i+1],(n-i)*sizeof(Fl_Menu_Item));
}

//
// End of "$Id$".
//
