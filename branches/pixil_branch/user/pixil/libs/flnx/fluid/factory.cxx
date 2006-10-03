//
// "$Id$"
//
// Widget factory code for the Fast Light Tool Kit (FLTK).
//
// Type classes for most of the fltk widgets.  Most of the work
// is done by code in Fl_Widget_Type.C.  Also a factory instance
// of each of these type classes.
//
// This file also contains the "new" menu, which has a pointer
// to a factory instance for every class (both the ones defined
// here and ones in other files)
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

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Item.H>
#include <string.h>
#include <stdio.h>

#if defined(WIN32) || defined(__EMX__)
#define strcasecmp stricmp
#endif

#include "Fl_Widget_Type.h"

////////////////////////////////////////////////////////////////

#include <FL/Fl_Box.H>
class Fl_Box_Type : public Fl_Widget_Type {
public:
  virtual const char *type_name() {return "Fl_Box";}
  Fl_Widget *widget(int x,int y,int w, int h) {
    return new Fl_Box(x,y,w,h,"label");}
  Fl_Widget_Type *_make() {return new Fl_Box_Type();}
};
static Fl_Box_Type Fl_Box_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Button.H>
static Fl_Menu_Item buttontype_menu[] = {
  {"Normal",0,0,(void*)0},
  {"Toggle",0,0,(void*)FL_TOGGLE_BUTTON},
  {"Radio",0,0,(void*)FL_RADIO_BUTTON},
  {0}};
class Fl_Button_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return buttontype_menu;}
public:
  virtual const char *type_name() {return "Fl_Button";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Button(x,y,w,h,"button");}
  Fl_Widget_Type *_make() {return new Fl_Button_Type();}
  int is_button() const {return 1;}
};
static Fl_Button_Type Fl_Button_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Return_Button.H>
class Fl_Return_Button_Type : public Fl_Button_Type {
public:
  virtual const char *type_name() {return "Fl_Return_Button";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Return_Button(x,y,w,h,0);}
  Fl_Widget_Type *_make() {return new Fl_Return_Button_Type();}
};
static Fl_Return_Button_Type Fl_Return_Button_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Repeat_Button.H>
class Fl_Repeat_Button_Type : public Fl_Widget_Type {
public:
  virtual const char *type_name() {return "Fl_Repeat_Button";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Repeat_Button(x,y,w,h,0);}
  Fl_Widget_Type *_make() {return new Fl_Repeat_Button_Type();}
};
static Fl_Repeat_Button_Type Fl_Repeat_Button_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Light_Button.H>
class Fl_Light_Button_Type : public Fl_Button_Type {
public:
  virtual const char *type_name() {return "Fl_Light_Button";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Light_Button(x,y,w,h,"button");}
  Fl_Widget_Type *_make() {return new Fl_Light_Button_Type();}
};
static Fl_Light_Button_Type Fl_Light_Button_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Check_Button.H>
class Fl_Check_Button_Type : public Fl_Button_Type {
public:
  virtual const char *type_name() {return "Fl_Check_Button";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Check_Button(x,y,w,h,"button");}
  Fl_Widget_Type *_make() {return new Fl_Check_Button_Type();}
};
static Fl_Check_Button_Type Fl_Check_Button_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Round_Button.H>
class Fl_Round_Button_Type : public Fl_Button_Type {
public:
  virtual const char *type_name() {return "Fl_Round_Button";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Round_Button(x,y,w,h,"button");}
  Fl_Widget_Type *_make() {return new Fl_Round_Button_Type();}
};
static Fl_Round_Button_Type Fl_Round_Button_type;

////////////////////////////////////////////////////////////////

extern int compile_only;

#include <FL/Fl_Browser.H>
static Fl_Menu_Item browser_type_menu[] = {
  {"No Select",0,0,(void*)FL_NORMAL_BROWSER},
  {"Select",0,0,(void*)FL_SELECT_BROWSER},
  {"Hold",0,0,(void*)FL_HOLD_BROWSER},
  {"Multi",0,0,(void*)FL_MULTI_BROWSER},
  {0}};
class Fl_Browser_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return browser_type_menu;}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
public:
  virtual const char *type_name() {return "Fl_Browser";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Browser* b = new Fl_Browser(x,y,w,h);
    // Fl_Browser::add calls fl_height(), which requires the X display open.
    // Avoid this when compiling so it works w/o a display:
    if (!compile_only) {
      char buffer[20];
      for (int i = 1; i <= 20; i++) {
	sprintf(buffer,"Browser Line %d",i);
	b->add(buffer);
      }
    }
    return b;
  }
  Fl_Widget_Type *_make() {return new Fl_Browser_Type();}
};
static Fl_Browser_Type Fl_Browser_type;

int Fl_Browser_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
  Fl_Browser *o = (Fl_Browser*)(w==4 ? ((Fl_Widget_Type*)this->factory)->o : this->o);
  switch (w) {
    case 4:
    case 0: f = o->textfont(); s = o->textsize(); c = o->textcolor(); break;
    case 1: o->textfont(f); break;
    case 2: o->textsize(s); break;
    case 3: o->textcolor(c); break;
  }
  return 1;
}

////////////////////////////////////////////////////////////////

#include <FL/Fl_Counter.H>
static Fl_Menu_Item counter_type_menu[] = {
  {"Normal",0,0,(void*)FL_NORMAL_COUNTER},
  {"Simple",0,0,(void*)FL_SIMPLE_COUNTER},
  {0}};
class Fl_Counter_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return counter_type_menu;}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
  int is_valuator() const {return 1;}
public:
  virtual const char *type_name() {return "Fl_Counter";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Counter(x,y,w,h,"counter:");}
  Fl_Widget_Type *_make() {return new Fl_Counter_Type();}
};
static Fl_Counter_Type Fl_Counter_type;

int Fl_Counter_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
  Fl_Counter *o = (Fl_Counter*)(w==4 ? ((Fl_Widget_Type*)this->factory)->o : this->o);
  switch (w) {
    case 4:
    case 0: f = o->textfont(); s = o->textsize(); c = o->textcolor(); break;
    case 1: o->textfont(f); break;
    case 2: o->textsize(s); break;
    case 3: o->textcolor(c); break;
  }
  return 1;
}

////////////////////////////////////////////////////////////////

#include <FL/Fl_Input.H>
static Fl_Menu_Item input_type_menu[] = {
  {"Normal",0,0,(void*)FL_NORMAL_INPUT},
  {"Multiline",0,0,(void*)FL_MULTILINE_INPUT},
  {"Secret",0,0,(void*)FL_SECRET_INPUT},
  {"Int",0,0,(void*)FL_INT_INPUT},
  {"Float",0,0,(void*)FL_FLOAT_INPUT},
  {0}};
class Fl_Input_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return input_type_menu;}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
public:
  virtual const char *type_name() {return "Fl_Input";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Input *o = new Fl_Input(x,y,w,h,"input:");
    o->value("Text Input");
    return o;
  }
  Fl_Widget_Type *_make() {return new Fl_Input_Type();}
};
static Fl_Input_Type Fl_Input_type;

int Fl_Input_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
  Fl_Input_ *o = (Fl_Input_*)(w==4 ? ((Fl_Widget_Type*)this->factory)->o : this->o);
  switch (w) {
    case 4:
    case 0: f = o->textfont(); s = o->textsize(); c = o->textcolor(); break;
    case 1: o->textfont(f); break;
    case 2: o->textsize(s); break;
    case 3: o->textcolor(c); break;
  }
  return 1;
}

////////////////////////////////////////////////////////////////

#include <FL/Fl_Clock.H>
class Fl_Clock_Type : public Fl_Widget_Type {
public:
  virtual const char *type_name() {return "Fl_Clock";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Clock(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_Clock_Type();}
};
static Fl_Clock_Type Fl_Clock_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Adjuster.H>
class Fl_Adjuster_Type : public Fl_Widget_Type {
  int is_valuator() const {return 1;}
public:
  virtual const char *type_name() {return "Fl_Adjuster";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Adjuster(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_Adjuster_Type();}
};
static Fl_Adjuster_Type Fl_Adjuster_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Dial.H>
static Fl_Menu_Item dial_type_menu[] = {
  {"Dot",0,0,(void*)0},
  {"Line",0,0,(void*)FL_LINE_DIAL},
  {"Fill",0,0,(void*)FL_FILL_DIAL},
  {0}};
class Fl_Dial_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return dial_type_menu;}
  int is_valuator() const {return 1;}
public:
  virtual const char *type_name() {return "Fl_Dial";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Dial(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_Dial_Type();}
};
static Fl_Dial_Type Fl_Dial_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Roller.H>
static Fl_Menu_Item roller_type_menu[] = {
  {"Vertical",0,0,(void*)0},
  {"Horizontal",0,0,(void*)FL_HORIZONTAL},
  {0}};
class Fl_Roller_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return roller_type_menu;}
  int is_valuator() const {return 1;}
public:
  virtual const char *type_name() {return "Fl_Roller";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Roller(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_Roller_Type();}
};
static Fl_Roller_Type Fl_Roller_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Scrollbar.H>
static Fl_Menu_Item slider_type_menu[] = {
  {"Vertical",0,0,(void*)FL_VERT_SLIDER},
  {"Horizontal",0,0,(void*)FL_HOR_SLIDER},
  {"Vert Fill",0,0,(void*)FL_VERT_FILL_SLIDER},
  {"Horz Fill",0,0,(void*)FL_HOR_FILL_SLIDER},
  {"Vert Knob",0,0,(void*)FL_VERT_NICE_SLIDER},
  {"Horz Knob",0,0,(void*)FL_HOR_NICE_SLIDER},
  {0}};
class Fl_Slider_Type : public Fl_Widget_Type {
  Fl_Menu_Item *subtypes() {return slider_type_menu;}
  int is_valuator() const {return 2;}
public:
  virtual const char *type_name() {return "Fl_Slider";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Slider(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_Slider_Type();}
};
static Fl_Slider_Type Fl_Slider_type;

static Fl_Menu_Item scrollbar_type_menu[] = {
  {"Vertical",0,0,(void*)FL_VERT_SLIDER},
  {"Horizontal",0,0,(void*)FL_HOR_SLIDER},
  {0}};
class Fl_Scrollbar_Type : public Fl_Slider_Type {
  Fl_Menu_Item *subtypes() {return scrollbar_type_menu;}
public:
  virtual const char *type_name() {return "Fl_Scrollbar";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Scrollbar(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_Scrollbar_Type();}
};
static Fl_Scrollbar_Type Fl_Scrollbar_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Output.H>
static Fl_Menu_Item output_type_menu[] = {
  {"Normal",0,0,(void*)FL_NORMAL_INPUT},
  {"Multiline",0,0,(void*)FL_MULTILINE_INPUT},
  {0}};
class Fl_Output_Type : public Fl_Input_Type {
  Fl_Menu_Item *subtypes() {return output_type_menu;}
public:
  virtual const char *type_name() {return "Fl_Output";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Output *o = new Fl_Output(x,y,w,h,"output:");
    o->value("Text Output");
    return o;
  }
  Fl_Widget_Type *_make() {return new Fl_Output_Type();}
};
static Fl_Output_Type Fl_Output_type;

////////////////////////////////////////////////////////////////

#include <FL/Fl_Value_Input.H>
class Fl_Value_Input_Type : public Fl_Widget_Type {
public:
  virtual const char *type_name() {return "Fl_Value_Input";}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
  int is_valuator() const {return 1;}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Value_Input *o = new Fl_Value_Input(x,y,w,h,"value:");
    return o;
  }
  Fl_Widget_Type *_make() {return new Fl_Value_Input_Type();}
};
static Fl_Value_Input_Type Fl_Value_Input_type;

int Fl_Value_Input_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
  Fl_Value_Input *o = (Fl_Value_Input*)(w==4 ? ((Fl_Widget_Type*)this->factory)->o : this->o);
  switch (w) {
    case 4:
    case 0: f = o->textfont(); s = o->textsize(); c = o->textcolor(); break;
    case 1: o->textfont(f); break;
    case 2: o->textsize(s); break;
    case 3: o->textcolor(c); break;
  }
  return 1;
}

////////////////////////////////////////////////////////////////

#include <FL/Fl_Value_Output.H>
class Fl_Value_Output_Type : public Fl_Widget_Type {
public:
  virtual const char *type_name() {return "Fl_Value_Output";}
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
  int is_valuator() const {return 1;}
  Fl_Widget *widget(int x,int y,int w,int h) {
    Fl_Value_Output *o = new Fl_Value_Output(x,y,w,h,"value:");
    return o;
  }
  Fl_Widget_Type *_make() {return new Fl_Value_Output_Type();}
};
static Fl_Value_Output_Type Fl_Value_Output_type;

int Fl_Value_Output_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
  Fl_Value_Output *o = (Fl_Value_Output*)(w==4 ? ((Fl_Widget_Type*)this->factory)->o : this->o);
  switch (w) {
    case 4:
    case 0: f = o->textfont(); s = o->textsize(); c = o->textcolor(); break;
    case 1: o->textfont(f); break;
    case 2: o->textsize(s); break;
    case 3: o->textcolor(c); break;
  }
  return 1;
}

////////////////////////////////////////////////////////////////

#include <FL/Fl_Value_Slider.H>
class Fl_Value_Slider_Type : public Fl_Slider_Type {
  int textstuff(int w, Fl_Font& f, int& s, Fl_Color& c);
public:
  virtual const char *type_name() {return "Fl_Value_Slider";}
  Fl_Widget *widget(int x,int y,int w,int h) {
    return new Fl_Value_Slider(x,y,w,h);}
  Fl_Widget_Type *_make() {return new Fl_Value_Slider_Type();}
};
static Fl_Value_Slider_Type Fl_Value_Slider_type;

int Fl_Value_Slider_Type::textstuff(int w, Fl_Font& f, int& s, Fl_Color& c) {
  Fl_Value_Slider *o = (Fl_Value_Slider*)(w==4 ? ((Fl_Widget_Type*)this->factory)->o : this->o);
  switch (w) {
    case 4:
    case 0: f = o->textfont(); s = o->textsize(); c = o->textcolor(); break;
    case 1: o->textfont(f); break;
    case 2: o->textsize(s); break;
    case 3: o->textcolor(c); break;
  }
  return 1;
}

////////////////////////////////////////////////////////////////

extern class Fl_Function_Type Fl_Function_type;
extern class Fl_Code_Type Fl_Code_type;
extern class Fl_CodeBlock_Type Fl_CodeBlock_type;
extern class Fl_Decl_Type Fl_Decl_type;
extern class Fl_DeclBlock_Type Fl_DeclBlock_type;
extern class Fl_Class_Type Fl_Class_type;
extern class Fl_Window_Type Fl_Window_type;
extern class Fl_Group_Type Fl_Group_type;
extern class Fl_Pack_Type Fl_Pack_type;
extern class Fl_Tabs_Type Fl_Tabs_type;
extern class Fl_Scroll_Type Fl_Scroll_type;
extern class Fl_Tile_Type Fl_Tile_type;
extern class Fl_Choice_Type Fl_Choice_type;
extern class Fl_Menu_Bar_Type Fl_Menu_Bar_type;
extern class Fl_Menu_Button_Type Fl_Menu_Button_type;
extern class Fl_Menu_Item_Type Fl_Menu_Item_type;
extern class Fl_Submenu_Type Fl_Submenu_type;

extern void select(Fl_Type *,int);
extern void select_only(Fl_Type *);

static void cb(Fl_Widget *, void *v) {
  Fl_Type *t = ((Fl_Type*)v)->make();
  if (t) {select_only(t); modflag = 1; t->open();}
}

Fl_Menu_Item New_Menu[] = {
{"code",0,0,0,FL_SUBMENU},
  {"function/method",0,cb,(void*)&Fl_Function_type},
  {"code",0,cb,(void*)&Fl_Code_type},
  {"code block",0,cb,(void*)&Fl_CodeBlock_type},
  {"declaration",0,cb,(void*)&Fl_Decl_type},
  {"declaration block",0,cb,(void*)&Fl_DeclBlock_type},
  {"class",0,cb,(void*)&Fl_Class_type},
{0},
{"group",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Window_type},
  {0,0,cb,(void*)&Fl_Group_type},
  {0,0,cb,(void*)&Fl_Pack_type},
  {0,0,cb,(void*)&Fl_Tabs_type},
  {0,0,cb,(void*)&Fl_Scroll_type},
  {0,0,cb,(void*)&Fl_Tile_type},
{0},
{"buttons",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Button_type},
  {0,0,cb,(void*)&Fl_Return_Button_type},
  {0,0,cb,(void*)&Fl_Light_Button_type},
  {0,0,cb,(void*)&Fl_Check_Button_type},
  {0,0,cb,(void*)&Fl_Round_Button_type},
  {0,0,cb,(void*)&Fl_Repeat_Button_type},
{0},
{"valuators",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Slider_type},
  {0,0,cb,(void*)&Fl_Scrollbar_type},
  {0,0,cb,(void*)&Fl_Value_Slider_type},
  {0,0,cb,(void*)&Fl_Adjuster_type},
  {0,0,cb,(void*)&Fl_Counter_type},
  {0,0,cb,(void*)&Fl_Dial_type},
  {0,0,cb,(void*)&Fl_Roller_type},
  {0,0,cb,(void*)&Fl_Value_Input_type},
  {0,0,cb,(void*)&Fl_Value_Output_type},
{0},
{"text",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Input_type},
  {0,0,cb,(void*)&Fl_Output_type},
{0},
{"menus",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Menu_Bar_type},
  {0,0,cb,(void*)&Fl_Menu_Button_type},
  {0,0,cb,(void*)&Fl_Choice_type},
  {0,0,cb, (void*)&Fl_Submenu_type},
  {0,0,cb, (void*)&Fl_Menu_Item_type},
{0},
{"other",0,0,0,FL_SUBMENU},
  {0,0,cb,(void*)&Fl_Browser_type},
  {0,0,cb,(void*)&Fl_Box_type},
  {0,0,cb,(void*)&Fl_Clock_type},
{0},
{0}};

void fill_in_New_Menu() {
  for (unsigned i = 0; i < sizeof(New_Menu)/sizeof(*New_Menu); i++) {
    Fl_Menu_Item *m = New_Menu+i;
    if (m->user_data() && !m->text) {
      const char *n = ((Fl_Type*)(m->user_data()))->type_name();
      if (!strncmp(n,"Fl_",3)) n += 3;
      m->text = n;
    }
  }
}

// use keyword to pick the type, this is used to parse files:
int reading_file;
Fl_Type *Fl_Type_make(const char *tn) {
  reading_file = 1; // makes labels be null
  Fl_Type *r = 0;
  for (unsigned i = 0; i < sizeof(New_Menu)/sizeof(*New_Menu); i++) {
    Fl_Menu_Item *m = New_Menu+i;
    if (!m->user_data()) continue;
    Fl_Type *t = (Fl_Type*)(m->user_data());
    if (!strcasecmp(tn,t->type_name())) {r = t->make(); break;}
  }
  reading_file = 0;
  return r;
}

////////////////////////////////////////////////////////////////

// Since I have included all the .H files, do this table here:
// This table is only used to read fdesign files:

struct symbol {const char *name; int value;};

static symbol table[] = {
  {"BLACK",	FL_BLACK},
  {"RED",	FL_RED},
  {"GREEN",	FL_GREEN},
  {"YELLOW",	FL_YELLOW},
  {"BLUE",	FL_BLUE},
  {"MAGENTA",	FL_MAGENTA},
  {"CYAN",	FL_CYAN},
  {"WHITE",	FL_WHITE},

  {"LCOL",		 FL_BLACK},
  {"COL1",		 FL_GRAY},
  {"MCOL",		 FL_LIGHT1},
  {"LEFT_BCOL",		 FL_LIGHT3},
  {"TOP_BCOL",		 FL_LIGHT2},
  {"BOTTOM_BCOL",	 FL_DARK2},
  {"RIGHT_BCOL",		 FL_DARK3},
  {"INACTIVE",		 FL_INACTIVE_COLOR},
  {"INACTIVE_COL",	 FL_INACTIVE_COLOR},
  {"FREE_COL1",		 FL_FREE_COLOR},
  {"FREE_COL2",		 FL_FREE_COLOR+1},
  {"FREE_COL3",		 FL_FREE_COLOR+2},
  {"FREE_COL4",		 FL_FREE_COLOR+3},
  {"FREE_COL5",		 FL_FREE_COLOR+4},
  {"FREE_COL6",		 FL_FREE_COLOR+5},
  {"FREE_COL7",		 FL_FREE_COLOR+6},
  {"FREE_COL8",		 FL_FREE_COLOR+7},
  {"FREE_COL9",		 FL_FREE_COLOR+8},
  {"FREE_COL10",		 FL_FREE_COLOR+9},
  {"FREE_COL11",		 FL_FREE_COLOR+10},
  {"FREE_COL12",		 FL_FREE_COLOR+11},
  {"FREE_COL13",		 FL_FREE_COLOR+12},
  {"FREE_COL14",		 FL_FREE_COLOR+13},
  {"FREE_COL15",		 FL_FREE_COLOR+14},
  {"FREE_COL16",		 FL_FREE_COLOR+15},
  {"TOMATO",		 131},
  {"INDIANRED",		 164},
  {"SLATEBLUE",		 195},
  {"DARKGOLD",		 84},
  {"PALEGREEN",		 157},
  {"ORCHID",		 203},
  {"DARKCYAN",		 189},
  {"DARKTOMATO",		 113},
  {"WHEAT",		 174},
  {"ALIGN_CENTER",	FL_ALIGN_CENTER},
  {"ALIGN_TOP",		FL_ALIGN_TOP},
  {"ALIGN_BOTTOM",	FL_ALIGN_BOTTOM},
  {"ALIGN_LEFT",	FL_ALIGN_LEFT},
  {"ALIGN_RIGHT",	FL_ALIGN_RIGHT},
  {"ALIGN_INSIDE",	FL_ALIGN_INSIDE},
  {"ALIGN_TOP_LEFT",	 FL_ALIGN_TOP | FL_ALIGN_LEFT},
  {"ALIGN_TOP_RIGHT",	 FL_ALIGN_TOP | FL_ALIGN_RIGHT},
  {"ALIGN_BOTTOM_LEFT",	 FL_ALIGN_BOTTOM | FL_ALIGN_LEFT},
  {"ALIGN_BOTTOM_RIGHT", FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT},
  {"ALIGN_CENTER|FL_ALIGN_INSIDE",	FL_ALIGN_CENTER|FL_ALIGN_INSIDE},
  {"ALIGN_TOP|FL_ALIGN_INSIDE",		FL_ALIGN_TOP|FL_ALIGN_INSIDE},
  {"ALIGN_BOTTOM|FL_ALIGN_INSIDE",	FL_ALIGN_BOTTOM|FL_ALIGN_INSIDE},
  {"ALIGN_LEFT|FL_ALIGN_INSIDE",	FL_ALIGN_LEFT|FL_ALIGN_INSIDE},
  {"ALIGN_RIGHT|FL_ALIGN_INSIDE",	FL_ALIGN_RIGHT|FL_ALIGN_INSIDE},
  {"ALIGN_INSIDE|FL_ALIGN_INSIDE",	FL_ALIGN_INSIDE|FL_ALIGN_INSIDE},
  {"ALIGN_TOP_LEFT|FL_ALIGN_INSIDE",	FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE},
  {"ALIGN_TOP_RIGHT|FL_ALIGN_INSIDE",	FL_ALIGN_TOP|FL_ALIGN_RIGHT|FL_ALIGN_INSIDE},
  {"ALIGN_BOTTOM_LEFT|FL_ALIGN_INSIDE",	FL_ALIGN_BOTTOM|FL_ALIGN_LEFT|FL_ALIGN_INSIDE},
  {"ALIGN_BOTTOM_RIGHT|FL_ALIGN_INSIDE",FL_ALIGN_BOTTOM|FL_ALIGN_RIGHT|FL_ALIGN_INSIDE},

  {"ALIGN_LEFT_TOP",	 FL_ALIGN_TOP | FL_ALIGN_LEFT},
  {"ALIGN_RIGHT_TOP",	 FL_ALIGN_TOP | FL_ALIGN_RIGHT},
  {"ALIGN_LEFT_BOTTOM",	 FL_ALIGN_BOTTOM | FL_ALIGN_LEFT},
  {"ALIGN_RIGHT_BOTTOM", FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT},
  {"INVALID_STYLE",	 255},
  {"NORMAL_STYLE",	 FL_HELVETICA},
  {"BOLD_STYLE",		 FL_HELVETICA|FL_BOLD},
  {"ITALIC_STYLE",	 FL_HELVETICA|FL_ITALIC},
  {"BOLDITALIC_STYLE",	 FL_HELVETICA|FL_BOLD|FL_ITALIC},
  {"FIXED_STYLE",	 FL_COURIER},
  {"FIXEDBOLD_STYLE",	 FL_COURIER|FL_BOLD},
  {"FIXEDITALIC_STYLE",	 FL_COURIER|FL_ITALIC},
  {"FIXEDBOLDITALIC_STYLE",  FL_COURIER|FL_BOLD|FL_ITALIC},
  {"TIMES_STYLE",	 FL_TIMES},
  {"TIMESBOLD_STYLE",	 FL_TIMES|FL_BOLD},
  {"TIMESITALIC_STYLE",	 FL_TIMES|FL_ITALIC},
  {"TIMESBOLDITALIC_STYLE",  FL_TIMES|FL_BOLD|FL_ITALIC},
  {"SHADOW_STYLE",	(_FL_SHADOW_LABEL<<8)},
  {"ENGRAVED_STYLE",	(_FL_ENGRAVED_LABEL<<8)},
  {"EMBOSSED_STYLE",	(_FL_EMBOSSED_LABEL<<0)},
  {"TINY_SIZE",		 8},
  {"SMALL_SIZE",		 11},
  {"NORMAL_SIZE",	 FL_NORMAL_SIZE},
  {"MEDIUM_SIZE",	 18},
  {"LARGE_SIZE",		 24},
  {"HUGE_SIZE",		 32},
  {"DEFAULT_SIZE",	 FL_NORMAL_SIZE},
  {"TINY_FONT",		 8},
  {"SMALL_FONT",		 11},
  {"NORMAL_FONT",	 FL_NORMAL_SIZE},
  {"MEDIUM_FONT",	 18},
  {"LARGE_FONT",		 24},
  {"HUGE_FONT",		 32},
  {"NORMAL_FONT1",	 11},
  {"NORMAL_FONT2",	 FL_NORMAL_SIZE},
  {"DEFAULT_FONT",	 11},
  {"RETURN_END_CHANGED",  0},
  {"RETURN_CHANGED",	 1},
  {"RETURN_END",		 2},
  {"RETURN_ALWAYS",	 3},
  {"PUSH_BUTTON",	FL_TOGGLE_BUTTON},
  {"RADIO_BUTTON",	FL_RADIO_BUTTON},
  {"HIDDEN_BUTTON",	FL_HIDDEN_BUTTON},
  {"SELECT_BROWSER",	FL_SELECT_BROWSER},
  {"HOLD_BROWSER",	FL_HOLD_BROWSER},
  {"MULTI_BROWSER",	FL_MULTI_BROWSER},
  {"SIMPLE_COUNTER",	FL_SIMPLE_COUNTER},
  {"LINE_DIAL",		FL_LINE_DIAL},
  {"FILL_DIAL",		FL_FILL_DIAL},
  {"VERT_SLIDER",	FL_VERT_SLIDER},
  {"HOR_SLIDER",	FL_HOR_SLIDER},
  {"VERT_FILL_SLIDER",	FL_VERT_FILL_SLIDER},
  {"HOR_FILL_SLIDER",	FL_HOR_FILL_SLIDER},
  {"VERT_NICE_SLIDER",	FL_VERT_NICE_SLIDER},
  {"HOR_NICE_SLIDER",	FL_HOR_NICE_SLIDER},
};

#include <stdlib.h>

int lookup_symbol(const char *name, int &v, int numberok) {
  if (name[0]=='F' && name[1]=='L' && name[2]=='_') name += 3;
  for (int i=0; i < int(sizeof(table)/sizeof(*table)); i++)
    if (!strcasecmp(name,table[i].name)) {v = table[i].value; return 1;}
  if (numberok && ((v = atoi(name)) || !strcmp(name,"0"))) return 1;
  return 0;
}

//
// End of "$Id$".
//
