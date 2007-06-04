/* -*-C++-*- 

   "$Id$"
   
   Copyright 1999-2000 by the Flek development team.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.
   
   Please report all bugs and problems to "flek-devel@sourceforge.net".

*/

#ifndef _FL_APP_WINDOW_H_
#define _FL_APP_WINDOW_H_

#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>

#include <Flek/Fl_Dockable_Window.H>
#include <Flek/export.h>

/** @package libflek_ui
 * Fl_App_Window is a window that can have Fl_Dockable_Windows
 * "docked" either vertically or horizontally to the contents of the window.
 * An Fl_App_Window starts with a top-level "application" window.
 * Flek creates an Fl_Pack group that fills the application window.
 * The pack group contains the child "contents" window and any
 * "docked" groups.
 * You might think of the contents window as the "client" area of the application.
 */
class FLEK_UI_API Fl_App_Window : public Fl_Window {
private:
  /**
   * Holds the contents window and any docked windows.
   */
  Fl_Pack* _pack;

  /**
   * Provides the "client" area of the application window.
   */
  Fl_Window* _contents;

  /**
   * Holds a list of pointers to dockable windows that have permission to
   * dock to this window.
   * This is also used to provide automatic showing and hiding of undocked
   * windows when the application windows is shown/hid.
   */
  Fl_Dockable_Window** dockable_windows;

  /**
   * Specifies the capacity of the dockable_windows list.
   */
  int dockable_windows_capacity;

  /**
   * Tracks the number of elements used in the dockable_windows list.
   */
  int dockable_windows_size;

  /**
   * Helps the constructors do window creation.
   */
  void create_app_window(int w, int h, const char* l);

protected:
  virtual void flush();

public:
  /**
   * Constructs an Fl_App_Window.
   */
  Fl_App_Window(int w, int h, const char *l = 0);

  /**
   * Constructs an Fl_App_Window, forcing a specific position.
   * This probably isn't what you want.  Try the other constructor instead.
   */
  Fl_App_Window(int x, int y, int w, int h, const char *l = 0);

  ~Fl_App_Window();
  
  /**
   * Returns a pointer to the Fl_Pack group, which contains the contents window
   * and any docked groups.
   * From what I can tell, this is only used by the docking sample.
   */
  Fl_Pack* tpack() { return _pack; }

  /**
   * Returns a pointer to the contents window.
   */
  Fl_Window* contents() { return _contents; }

  /**
   * Sets the box type and color for the contents window.
   * Includes uchar signatures to keep fluid happy.
   */
  void box(Fl_Boxtype b) { _contents->box(b); }
  void color(Fl_Color a) { _contents->color(a); }
// CET - FIXME  void color(Fl_Color a, Fl_Color b) { _contents->color(a); , b); }
//  void color(uchar a) { _contents->color(a); }
//  void color(uchar a, uchar b) { _contents->color(a, b); }

  /**
   * Adds the widget to the contents window.
   */
  void add(Fl_Widget *w);

  /**
   * Repacks the window.  Used after resizing or adding dockable windows.
   */
  void repack();
  
  /**
   * Resizes the contents window.
   */
  void resize(int x, int y, int w, int h);
  
#ifdef FLEK_FLTK_2
  void layout();
#endif  
  
  /**
   * Grants permission for the dockable window to dock to this window.
   * This does not cause the dockable window to be docked to this window.
   */  
  void accept_dockable(Fl_Dockable_Window* W);

  /**
   * Determines if a dockable window has permission to dock to this window.
   */  
  bool may_dock(Fl_Dockable_Window* W);
  
  /**
   * Docks the dockable window to this window, adding it to the specified
   * position in the pack group.
   * This also grants permission for the dockable window to dock
   * to this window.
   * Docking an Fl_Dockable_Window to an Fl_App_Window does not change
   * the size of the Fl_App_Window.  Rather, the size of the contents
   * window is adjusted to accomodate the Fl_Dockable_Window.
   */
  void add_dockable(Fl_Dockable_Window* W, int position = 0);

  /**
   * Begin adding widgets to the contents window.
   */
  void begin() { Fl_Group::current(_contents); }

  /**
   * Shows or hides the application window.
   */
  void show();
  void hide();

  int handle(int);
};

#endif
