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

#ifndef _FL_CALENDAR_H_
#define _FL_CALENDAR_H_

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Input.H>

#ifdef PIXIL
#include "nxbutton.h"
#include "nxbox.h"
#endif

#include <Flek/FDate.H>

#define	DOE_MIN		1		/* January 1, 1 */
#define	DOE_MAX		3652059		/* December 31, 9999 */
#define	YEAR_MIN	1
#define	YEAR_MAX	9999

enum Cal_Type {
        DAY = 0,
        WEEK = 1,
        MONTH = 2
};

/** @package libflek_ui
 * Fl_Calendar_Base is a widget that allows a user to select a date
 * (day, month, and year) with a wall style calendar. 
 */

#ifdef PIXIL
class Fl_Calendar_Base : public Fl_Group {
#else
class Fl_Calendar_Base : public Fl_Group, public FDate {
#endif

public:
  /**
   * The constructor for an empty Fl_Calendar_Base.
   */
#ifdef PIXIL
  Fl_Calendar_Base (int x, int y, int w = (7*20), int h = (6*20),   
		    const char *l = 0, Cal_Type type = DAY);
#else
  Fl_Calendar_Base (int x, int y, int w = (7*20), int h = (6*20),   
		    const char *l = 0);
#endif

  void update ();

  /**
   * Resize the buttons contained in Fl_Calendar_Base to
   * fit in the passed dimensions.
   *
   * @param cx The calendar x position.
   * @param cy The calendar y position.
   * @param cw The calendar width.
   * @param ch The calendar height.
   */
  void csize (int cx, int cy, int cw, int ch);

  /**
   * Returns a pointer to the button for the day D in the Fl_Calendar. 
   */
  Fl_Button * day_button (int i);
   
#ifdef PIXIL
  int year() {return date.year();}
  void year(int y) {date.year(y);}
  int month() {return date.month();}
  void month(int m) {date.month(m);}
  int day() {return date.day();}
  void day(int d) {date.day(d);}
  void set_date(int y, int m, int d) {date.set_date(y, m, d);}
  int days_in_month() {return date.days_in_month();}
  void previous_month() {date.previous_month();}
  void previous_year() {date.previous_year();}
  void next_year() {date.next_year();}
  char *to_string(int fmt) const {return date.to_string(fmt);}
  char *to_string() const {return date.to_string();}
  static const char* month_name[];
  
  int type() {return _type;}
#endif

 protected:
  
  int cal_x;
  int cal_y;
  int cal_w;
  int cal_h;

#ifdef PIXIL
  int m_nRows;
  FDate date;
  int _type;
#endif

  Fl_Button * days[6*7];
};

/**
 * Fl_Calendar is a subclass of Fl_Calendar.  This widget adds 
 * day labels and controllers to the base calendar class.
 * <p><img src="Fl_Calendar.png">
 */
class Fl_Calendar : public Fl_Calendar_Base 
{

 public:
  
  /**
   * The constructor for an empty Fl_Calendar.
   */
#ifdef PIXIL

  Fl_Calendar (int x, int y, int w = (7*20), int h = (8*20), 
  		    const char *l = 0, bool bCaption = true, Cal_Type type = DAY);
#else
  Fl_Calendar (int x, int y, int w = (7*20), int h = (8*20), 
  		    const char *l = 0);
#endif

  /**
   * Sets the value of the widget back by one month. 
   */
  void previous_month ();

  /**
   * Sets the value of the widget forward by one month. 
   */
  void next_month ();

  /**
   * Sets the value of the widget back by one year. 
   */
  void previous_year ();

  /**
   * Sets the value of the widget forward by one year. 
   */
  void next_year ();

#ifdef PIXIL
  void update_buttons(void);
#endif

  /**
   * Updates the widget after values have been changed.
   */  
  void update ();

  /**
   * Resize the buttons contained in Fl_Calendar_Base to
   * fit in the passed dimensions.
   *
   * @param cx The calendar x position.
   * @param cy The calendar y position.
   * @param cw The calendar width.
   * @param ch The calendar height.
   */
  void csize (int cx, int cy, int cw, int ch);
  int  handle (int);

  /**
   * Sets the Calendar's selection color.
   *
   * @param color The calendar's selection color.
   */  
  void selection_color(Fl_Color c) {
	Fl_Widget::selection_color(c);
	for(int i = 0; i < 6*7; i++) {
		if(days[i]->color() == days[i]->selection_color()) {
			days[i]->color(c);
		}
		days[i]->selection_color(c);
	}
  }

  /**
   * Gets the Calendar's selection color.
   *
   * @return The calendar's selection color.
   */  
  Fl_Color selection_color() const {
    return Fl_Group::selection_color();
  }

  /**
   * Gets the user selected day.  This method differs
   * from the day() method in that if a user has not
   * selected a day the return value is zero.
   *
   * @return The selected day if the user has selected a day, 0 otherwise.
   */
  int selected_day() {return selected_day_;}

  /**
   * Sets the user selected day.  If d is non zero, this
   * method also sets the day.
   *
   * @return The selected day if the user has selected a day, 0 otherwise.
   */
  int selected_day(int d) {selected_day_ = d; if (d) day(d); return selected_day_; }

 protected:
  
  Fl_Box * weekdays[7];
#ifdef PIXIL
  NxBox * caption;
  NxButton * nxt_month;
  NxButton * prv_month;
  NxButton * nxt_year;
  NxButton * prv_year;

  bool m_bCaption;
#else
  Fl_Box * caption;
  Fl_Repeat_Button * nxt_month;
  Fl_Repeat_Button * prv_month;
  Fl_Repeat_Button * nxt_year;
  Fl_Repeat_Button * prv_year;
#endif

  int selected_day_;
};


// Is Fl_Agenda_Calendar really necessary?  I don't see how title_height
// makes the widget look any better.  - James

/**
 * Fl_Agenda_Calendar is a subclass of Fl_Calendar.  This widget has a
 * slightly different user interface used for Agenda PDA.
 */
class Fl_Agenda_Calendar : public Fl_Calendar
{

 public:
  
  /**
   * The constructor for an empty Fl_Agenda_Calendar.
   */
  Fl_Agenda_Calendar (int x, int y, int w = (7*20), int h = (8*20), 
		      const char *l = 0, int title_height = -1);
};

/**
 * An input for getting and setting a date.
 */
class Fl_Date_Input : public Fl_Group  {

 protected:
  
  Fl_Input   Input;
  Fl_Button  Btn;
  
  int xpos;
  int ypos;
  int width;
  int height;
  int popcalfmt;
  
 public:

  /**
   * Default constructor.
   * 
   * @param x The x coordinate.
   * @param y The y coordinate.
   * @param w The width of this widget.
   * @param h The height of this widget.
   * @param l The label.
   */
  Fl_Date_Input (int x, int y, int w, int h, char * = 0);
  
  void format (int);
  
  /**
   * Gets the date value.
   * 
   * @return The date string.
   */
  const char *value ();

  /**
   * Sets the date value.
   * 
   * @param date The date string.
   */
  void value (const char* date);

#ifndef FLEK_FLTK_2
  void text_font (int);
  void textfont (int t) { text_font(t); }
  void textsize (int t) { text_size(t); }
#else
  void text_font (Fl_Font);
#endif
  void text_size (int);
  
  void btnDate_Input_cb_i ();
  
};


#endif
