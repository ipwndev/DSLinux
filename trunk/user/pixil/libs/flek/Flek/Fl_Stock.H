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

#ifndef _FL_STOCK_H_
#define _FL_STOCK_H_

#include <FL/Fl_Button.H>
#include <Flek/export.h>

// Values used to align the text in the widget.

#ifndef FLEK_FLTK_2
#include <FL/Fl_Pixmap.H>
#define Fl_Flags int
#define Fl_Image Fl_Pixmap
#define FL_INACTIVE 4096
#endif

#define FL_TEXT_ALIGN_LEFT   0x00100000
#define FL_TEXT_ALIGN_RIGHT  0x00200000
#define FL_TEXT_ALIGN_TOP    0x00400000
#define FL_TEXT_ALIGN_BOTTOM 0x00800000

/** @package libflek_ui
 * The base class from which all stock buttons are derived.
 * These include:
 * 
 * Fl_Stock_Button_Ok, Fl_Stock_Button_Cancel,<br>
 * Fl_Stock_Button_Yes, Fl_Stock_Button_No,<br>
 * Fl_Stock_Button_Apply, Fl_Stock_Button_Close,<br>
 * Fl_Stock_Button_Add, Fl_Stock_Button_Remove,<br>
 * Fl_Stock_Button_New, Fl_Stock_Button_Previous,<br>
 * Fl_Stock_Button_Left, Fl_Stock_Button_Next,<br>
 * Fl_Stock_Button_Right, Fl_Stock_Button_Up,<br>
 * Fl_Stock_Button_Down, Fl_Stock_Button_Top,<br>
 * Fl_Stock_Button_Bottom, Fl_Stock_Button_Clear,<br>
 * Fl_Stock_Button_Open, Fl_Stock_Button_Copy,<br>
 * Fl_Stock_Button_Cut, Fl_Stock_Button_Exec,<br>
 * Fl_Stock_Button_First, Fl_Stock_Button_Help,<br>
 * Fl_Stock_Button_Last, Fl_Stock_Button_Save,<br>
 * Fl_Stock_Button_Search and Fl_Stock_Button_Search_Replace.
 *<p><img src="Fl_Stock_Button.png">
 */
class Fl_Stock_Button : public Fl_Button
{
public:
  /**
   * The constructor.  Passing a label to a stock button is usually
   * ignored.
   */
  Fl_Stock_Button (int,int,int,int,const char * = 0);
  void draw_label(int X, int Y, int W, int H, Fl_Color c, Fl_Flags f);
  void draw ();
#ifndef FLEK_FLTK_2
protected:
  Fl_Pixmap * image() { return image_; }
  void image (Fl_Pixmap*p) {image_ = p; }
  Fl_Pixmap *image_;
  void set_flag (int f) {flags_ |= f; Fl_Widget::set_flag(f);}
  void clear_flag (int f) {flags_ &= ~f; Fl_Widget::clear_flag(f);}
  int flags () {return (Fl_Widget::flags() | flags_); }
  int flags_;
#endif
};

class FLEK_UI_API Fl_Stock_Button_Ok : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Ok (int,int,int,int,const char*l="Ok");
};

class FLEK_UI_API Fl_Stock_Button_Cancel : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Cancel (int,int,int,int,const char*l="Cancel");
};

class FLEK_UI_API Fl_Stock_Button_Yes : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Yes (int,int,int,int,const char*l="Yes");
};

class FLEK_UI_API Fl_Stock_Button_No : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_No (int,int,int,int,const char*l="No");
};

class FLEK_UI_API Fl_Stock_Button_Apply : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Apply (int,int,int,int,const char*l="Apply");
};

class FLEK_UI_API Fl_Stock_Button_Close : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Close (int,int,int,int,const char*l="Close");
};

class FLEK_UI_API Fl_Stock_Button_Add : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Add (int,int,int,int,const char*l="Add");
};

class FLEK_UI_API Fl_Stock_Button_Remove : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Remove (int,int,int,int,const char*l="Remove");
};

class FLEK_UI_API Fl_Stock_Button_New : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_New (int,int,int,int,const char*l="New");
};

class FLEK_UI_API Fl_Stock_Button_Previous : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Previous (int,int,int,int,const char*l="Previous");
};

class FLEK_UI_API Fl_Stock_Button_Next : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Next (int,int,int,int,const char*l="Next");
};

class FLEK_UI_API Fl_Stock_Button_Left : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Left (int,int,int,int,const char*l="Left");
};

class FLEK_UI_API Fl_Stock_Button_Right : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Right (int,int,int,int,const char*l="Right");
};

class FLEK_UI_API Fl_Stock_Button_Up : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Up (int,int,int,int,const char*l="Up");
};

class FLEK_UI_API Fl_Stock_Button_Down : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Down (int,int,int,int,const char*l="Down");
};

class FLEK_UI_API Fl_Stock_Button_Top : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Top (int,int,int,int,const char*l="Top");
};

class FLEK_UI_API Fl_Stock_Button_Bottom : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Bottom (int,int,int,int,const char*l="Bottom");
};

class FLEK_UI_API Fl_Stock_Button_Clear : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Clear (int,int,int,int,const char*l="Clear");
};

class FLEK_UI_API Fl_Stock_Button_Open : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Open (int,int,int,int,const char*l="Open");
};

class FLEK_UI_API Fl_Stock_Button_Copy : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Copy (int,int,int,int,const char*l="Copy");
};

class FLEK_UI_API Fl_Stock_Button_Cut : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Cut (int,int,int,int,const char*l="Cut");
};

class FLEK_UI_API Fl_Stock_Button_Exec : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Exec (int,int,int,int,const char*l="Exec");
};

class FLEK_UI_API Fl_Stock_Button_First : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_First (int,int,int,int,const char*l="First");
};

class FLEK_UI_API Fl_Stock_Button_Help : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Help (int,int,int,int,const char*l="Help");
};

class FLEK_UI_API Fl_Stock_Button_Last : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Last (int,int,int,int,const char*l="Last");
};

class FLEK_UI_API Fl_Stock_Button_Save : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Save (int,int,int,int,const char*l="Save");
};

class FLEK_UI_API Fl_Stock_Button_Save_As : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Save_As (int,int,int,int,const char*l="Save As");
};

class FLEK_UI_API Fl_Stock_Button_Search : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Search (int,int,int,int,const char*l="Search");
};

class FLEK_UI_API Fl_Stock_Button_Search_Replace : public Fl_Stock_Button
{
  public:
  Fl_Stock_Button_Search_Replace (int,int,int,int,const char*l="Search and Replace");
};

#endif
