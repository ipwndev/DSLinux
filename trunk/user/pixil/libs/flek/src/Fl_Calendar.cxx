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

#include <FL/Fl.H>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <Flek/Fl_Calendar.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Repeat_Button.H>

#ifdef PIXIL
#include <nxapp.h>
#include <time.h>

void
set_week_buttons(Fl_Calendar_Base * c, int val)
{
    tm d;
    time_t new_time;
    tm *tt;

    memset(&d, 0, sizeof(d));
    d.tm_year = c->year() - 1900;
    d.tm_mon = c->month() - 1;
    d.tm_mday = c->day();

    new_time = mktime(&d);
    tt = localtime(&new_time);

    int wday = tt->tm_wday;
    int mon = d.tm_mon;
    int jdx = 0;
    int idx = 0;

    d.tm_wday = wday;

    for (jdx = 1; jdx <= wday; jdx++) {
	if (mon == tt->tm_mon) {
	    if (c->day_button(val - jdx)) {
		c->day_button(val - jdx)->color(NxApp::GlobalColor(HILIGHT));
		c->day_button(val -
			      jdx)->labelcolor(NxApp::GlobalColor(APP_BG));
		c->day_button(val -
			      jdx)->
		    selection_color(NxApp::GlobalColor(APP_FG));
	    } else
		break;
	} else
	    break;
	tt->tm_mday--;
	new_time = mktime(tt);
	tt = localtime(&new_time);
    }

    new_time = mktime(&d);
    tt = localtime(&new_time);
    mon = tt->tm_mon;
    wday = tt->tm_wday;
    for (jdx = wday, idx = 0; jdx < 7; jdx++, idx++) {
	if (mon == tt->tm_mon) {
	    if (c->day_button(val + idx)) {
		c->day_button(val + idx)->color(NxApp::GlobalColor(HILIGHT));
		c->day_button(val +
			      idx)->labelcolor(NxApp::GlobalColor(APP_BG));
		c->day_button(val +
			      idx)->
		    selection_color(NxApp::GlobalColor(APP_BG));
	    } else
		break;
	} else
	    break;
	tt->tm_mday++;
	new_time = mktime(tt);
	tt = localtime(&new_time);
    }

}
#endif

static void
fl_calendar_button_cb(Fl_Button * a, void *b)
{
    long j = 0;
    Fl_Calendar *c = (Fl_Calendar *) b;
    Fl_Button *sb;
    int numdays = c->days_in_month() + 1;
    for (int i = 1; i < numdays; i++) {
	sb = c->day_button(i);
#ifdef PIXIL
	sb->color(NxApp::GlobalColor(APP_BG));
	sb->labelcolor(NxApp::GlobalColor(APP_FG));
	sb->selection_color(NxApp::GlobalColor(HILIGHT));
#else
	sb->color(52);
#endif
	if (a == sb) {
	    c->selected_day(i);
	    j = i;
#ifdef PIXIL
	    sb->color(NxApp::GlobalColor(HILIGHT));
	    sb->labelcolor(NxApp::GlobalColor(APP_BG));
	    sb->selection_color(NxApp::GlobalColor(HILIGHT));
#else
	    sb->color(sb->selection_color());
#endif
	}
    }
    c->redraw();
    c->do_callback(c, j);
}

#ifdef PIXIL
Fl_Calendar_Base::Fl_Calendar_Base(int x, int y, int w, int h,
				   const char *l, Cal_Type type):
Fl_Group(x, y, w, h, l)
#else
Fl_Calendar_Base::Fl_Calendar_Base(int x, int y, int w, int h, const char *l):
Fl_Group(x, y, w, h, l),
FDate()
#endif
{
    int i;

#ifdef PIXIL
    _type = type;

    for (i = 0; i < (7 * 6); i++) {
	days[i] = new Fl_Button((w / 7) * (i % 7) + x,
				(h / (m_nRows - 2)) * (i / 7) + y,
				(w / 7), (h / (m_nRows - 2)));
#else
    for (i = 0; i < (7 * 6); i++) {
	days[i] = new Fl_Button((w / 7) * (i % 7) + x,
				(h / 6) * (i / 7) + y, (w / 7), (h / 6));
#endif

#if FL_MAJOR_VERSION == 1
	days[i]->down_box(FL_THIN_DOWN_BOX);
	days[i]->labelsize(10);
#else
	days[i]->label_size(10);
#endif
	days[i]->box(FL_THIN_UP_BOX);

#ifdef PIXIL
	days[i]->color(NxApp::GlobalColor(APP_BG));
	days[i]->selection_color(NxApp::GlobalColor(HILIGHT));
	days[i]->labelcolor(NxApp::GlobalColor(APP_FG));
#else
	days[i]->color(52);
#endif

	days[i]->callback((Fl_Callback *) & fl_calendar_button_cb,
			  (void *) this);
    }
}

#ifdef PIXIL

const char *
    Fl_Calendar_Base::month_name[] = {
    "January",
    "Febuary",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

#endif

void
Fl_Calendar_Base::csize(int cx, int cy, int cw, int ch)
{
    int i;

#ifdef PIXIL
    for (i = 0; i < (7 * (m_nRows - 2)); i++) {
	days[i]->resize((cw / 7) * (i % 7) + cx,
			(ch / (m_nRows - 2)) * (i / 7) + cy,
			(cw / 7), (ch / (m_nRows - 2)));
    }
#else
    int oi = (cw - (7 * (int) (cw / 7))) / 2;
    int of = (cw - (7 * (int) (cw / 7))) - oi;
    int xi, wxi;

    for (i = 0; i < (7 * 6); i++) {
	if ((i % 7) == 0)
	    xi = 0;
	else
	    xi = oi;
	if ((i % 7) == 0)
	    wxi = oi;
	else if ((i % 7) == 6)
	    wxi = of;
	else
	    wxi = 0;

	days[i]->resize((cw / 7) * (i % 7) + cx + xi,
			(ch / 6) * (i / 7) + cy, (cw / 7) + wxi, (ch / 6));
    }
#endif
}


#ifndef PIXIL

void
Fl_Calendar_Base::update()
{
    int dow = day_of_week(year(), month(), 1);
    int dim = days_in_month(month(), leap_year(year()));
    int i;

    for (i = 0; i < dow; i++) {
	days[i]->hide();
    }

    for (i = (dim + dow); i < (6 * 7); i++) {
	days[i]->hide();
    }

    for (i = dow; i < (dim + dow); i++) {
	char t[8];
	sprintf(t, "%d", (i - dow + 1));
	days[i]->label(strdup(t));
	days[i]->color(52);
	if ((i - dow + 1) == day())
	    days[i]->color(selection_color());
	days[i]->show();
    }
}

#endif

Fl_Button *
Fl_Calendar_Base::day_button(int i)
{
#ifdef PIXIL
    if ((i > 0) && (i <= date.days_in_month()))
	return days[i + date.day_of_week(date.year(), date.month(), 1) - 1];
#else
    if ((i > 0) && (i <= days_in_month()))
	return days[i + day_of_week(year(), month(), 1) - 1];
#endif

    return 0;


}



static void
fl_calendar_prv_month_cb(Fl_Button *, void *b)
{
    Fl_Calendar *c = (Fl_Calendar *) b;
    c->previous_month();
    c->do_callback(c, (long) 0);
}

static void
fl_calendar_nxt_month_cb(Fl_Button *, void *b)
{
    Fl_Calendar *c = (Fl_Calendar *) b;
    c->next_month();
    c->do_callback(c, (long) 0);
}

static void
fl_calendar_prv_year_cb(Fl_Button *, void *b)
{
    Fl_Calendar *c = (Fl_Calendar *) b;
    c->previous_year();
    c->do_callback(c, (long) 0);
}

static void
fl_calendar_nxt_year_cb(Fl_Button *, void *b)
{
    Fl_Calendar *c = (Fl_Calendar *) b;
    c->next_year();
    c->do_callback(c, (long) 0);
}

#ifdef PIXIL
Fl_Calendar::Fl_Calendar(int x, int y, int w, int h,
			 const char *l, bool bCaption, Cal_Type type):
Fl_Calendar_Base(x, y, w, h, l)
#else
Fl_Calendar::Fl_Calendar(int x, int y, int w, int h, const char *l):
Fl_Calendar_Base(x, y, w, h, l)
#endif
{

    int title_height = h / 8;
    int i;
    selected_day_ = 0;

#ifdef PIXIL
    m_bCaption = bCaption;

    if (m_bCaption)
	m_nRows = 8;
    else
	m_nRows = 7;

    for (i = 0; i < 7; i++) {
	if (m_bCaption) {
	    weekdays[i] = new Fl_Box((w / 7) * (i % 7) + x,
				     (h / m_nRows) * ((i / 7) + 1) + y,
				     (w / 7), (h / m_nRows));
	} else {
	    weekdays[i] = new Fl_Box((w / 7) * (i % 7) + x,
				     (h / m_nRows) * ((i / 7)) + y,
				     (w / 7), (h / m_nRows));

	}
#else

    // If the Calendar width isn't divisible by 7 there will be a gap
    // on the right or left side.
    // So we will distribute this extra space between Sunday and Saturday
    int oi = (w - (7 * (int) (w / 7))) / 2;
    int of = (w - (7 * (int) (w / 7))) - oi;
    int xi, wxi;

    for (i = 0; i < 7; i++) {
	if (i == 0)
	    xi = 0;
	else
	    xi = oi;
	if (i == 0)
	    wxi = oi;
	else if (i == 6)
	    wxi = of;
	else
	    wxi = 0;
	weekdays[i] = new Fl_Box((w / 7) * (i % 7) + x + xi,
				 ((h - title_height) / 7) * ((i / 7)) + y +
				 title_height, (w / 7) + wxi,
				 ((h - title_height) / 7));
#endif

	weekdays[i]->box(FL_THIN_UP_BOX);
#if FL_MAJOR_VERSION == 1
	weekdays[i]->labelsize(10);
#else
	weekdays[i]->label_size(10);
#endif

#ifdef PIXIL
	weekdays[i]->color(NxApp::GlobalColor(APP_BG));
	weekdays[i]->labelcolor(NxApp::GlobalColor(APP_FG));
	NxApp::DefaultFont((Fl_Widget *) weekdays[i]);
#else
	weekdays[i]->color(52);
#endif
    }

    weekdays[SUNDAY]->label("S");
    weekdays[MONDAY]->label("M");
    weekdays[TUESDAY]->label("T");
    weekdays[WEDNESDAY]->label("W");
    weekdays[THURSDAY]->label("R");
    weekdays[FRIDAY]->label("F");
    weekdays[SATURDAY]->label("S");

#ifdef PIXIL
    for (i = SUNDAY; i <= SATURDAY; i++) {
	weekdays[i]->box(FL_NO_BOX);
    }

    if (m_bCaption) {
	prv_year = new NxButton(x, y, 15, 15, "@<<");
	prv_year->box(FL_FLAT_BOX);
	prv_year->movable(false);
	prv_year->labeltype(FL_SYMBOL_LABEL);
	prv_year->callback((Fl_Callback *) & fl_calendar_prv_year_cb,
			   (void *) this);

	prv_month = new NxButton(x + 15, y, 15, 15, "@<");
	prv_month->box(FL_FLAT_BOX);
	prv_month->movable(false);
	prv_month->labeltype(FL_SYMBOL_LABEL);
	prv_month->callback((Fl_Callback *) & fl_calendar_prv_month_cb,
			    (void *) this);

	nxt_month = new NxButton(x + w - 30, y, 15, 15, "@>");
	nxt_month->box(FL_FLAT_BOX);
	nxt_month->movable(false);
	nxt_month->labeltype(FL_SYMBOL_LABEL);

	nxt_month->callback((Fl_Callback *) & fl_calendar_nxt_month_cb,
			    (void *) this);

	nxt_year = new NxButton(x + w - 15, y, 15, 15, "@>>");
	nxt_year->movable(false);
	nxt_year->box(FL_FLAT_BOX);
	nxt_year->labeltype(FL_SYMBOL_LABEL);
	nxt_year->callback((Fl_Callback *) & fl_calendar_nxt_year_cb,
			   (void *) this);

	caption = new NxBox(x + 29, y, w - 58, 15);
	caption->box(FL_BORDER_BOX);
	caption->movable(false);
	caption->resize(false);
	caption->color(NxApp::GlobalColor(APP_BG));
	caption->default_box(false);
	caption->box_color(NxApp::GlobalColor(BUTTON_FACE));
	caption->labelcolor(NxApp::GlobalColor(APP_FG));
	caption->labeltype(FL_SYMBOL_LABEL);
    }
#else
    prv_year =
	new Fl_Repeat_Button((x - of + w - (int) (w / 7) * 4), y, (w / 7),
			     (h / 8), "Y-");
#ifdef AGENDA
    prv_year->repeat(1000, 500);
#endif
    prv_year->box(FL_THIN_UP_BOX);
#if FL_MAJOR_VERSION == 1
    prv_year->labelsize(10);
    prv_year->down_box(FL_THIN_DOWN_BOX);
#else
    prv_year->label_size(10);
#endif
    prv_year->callback((Fl_Callback *) & fl_calendar_prv_year_cb,
		       (void *) this);

    prv_month =
	new Fl_Repeat_Button(x - of + w - (int) (w / 7) * 3, y, (w / 7),
			     (h / 8), "M-");
#ifdef AGENDA
    prv_month->repeat(1000, 500);
#endif
    prv_month->box(FL_THIN_UP_BOX);
#if FL_MAJOR_VERSION == 1
    prv_month->labelsize(10);
    prv_month->down_box(FL_THIN_DOWN_BOX);
#else
    prv_month->label_size(10);
#endif
    prv_month->callback((Fl_Callback *) & fl_calendar_prv_month_cb,
			(void *) this);

    nxt_month =
	new Fl_Repeat_Button(x - of + w - (int) (w / 7) * 2, y, (w / 7),
			     (h / 8), "M+");
#ifdef AGENDA
    nxt_month->repeat(1000, 500);
#endif
    nxt_month->box(FL_THIN_UP_BOX);
#if FL_MAJOR_VERSION == 1
    nxt_month->labelsize(10);
    nxt_month->down_box(FL_THIN_DOWN_BOX);
#else
    nxt_month->label_size(10);
#endif
    nxt_month->callback((Fl_Callback *) & fl_calendar_nxt_month_cb,
			(void *) this);

    nxt_year =
	new Fl_Repeat_Button(x - of + w - (int) (w / 7) * 1, y, (w / 7) + of,
			     (h / 8), "Y+");
#ifdef AGENDA
    nxt_year->repeat(1000, 500);
#endif
    nxt_year->box(FL_THIN_UP_BOX);
#if FL_MAJOR_VERSION == 1
    nxt_year->labelsize(10);
    nxt_year->down_box(FL_THIN_DOWN_BOX);
#else
    nxt_year->label_size(10);
#endif
    nxt_year->callback((Fl_Callback *) & fl_calendar_nxt_year_cb,
		       (void *) this);

    caption = new Fl_Box(x, y, (w / 7) * 3 + oi, (h / 8));
    caption->box(FL_THIN_UP_BOX);
#if FL_MAJOR_VERSION == 1
    caption->labelfont(FL_HELVETICA_BOLD);
    caption->labelsize(10);
#else
    caption->label_size(10);
#endif
#endif /* PIXIL */

#ifdef PIXIL
    if (m_bCaption)
	Fl_Calendar_Base::csize(x, y + (2 * h / m_nRows), w,
				(6 * h / m_nRows));
    else
	Fl_Calendar_Base::csize(x, y + (h / m_nRows), w, (6 * h / m_nRows));
#else
    Fl_Calendar_Base::csize(x, y + title_height + (h - title_height) / 7, w,
			    h - title_height - (h - title_height) / 7);
#endif

    update();
}

void
Fl_Calendar::csize(int cx, int cy, int cw, int ch)
{
    int i;

#ifdef PIXIL
    for (i = 0; i < 7; i++) {
	weekdays[i] = new Fl_Box((cw / 7) * (i % 7) + cx,
				 (ch / m_nRows) * ((i / 7) + 1) + cy,
				 (cw / 7), (ch / m_nRows));
	NxApp::DefaultFont((Fl_Widget *) weekdays[i]);
    }

    if (m_bCaption) {
	prv_month->resize(cx + (cw / 10), cy, (cw / 10), (ch / 8));
	nxt_month->resize(cx + (cw / 10) * 8, cy, (cw / 10), (ch / 8));
	prv_year->resize(cx, cy, (cw / 10), (ch / 8));
	nxt_year->resize(cx + (cw / 10) * 9, cy, (cw / 10), (ch / 8));
	caption->resize(cx + (cw / 10) * 2, cy, (cw / 10) * 6, (ch / 8));
    }

    Fl_Calendar_Base::csize(cx, cy + (2 * ch / m_nRows), cw,
			    (6 * ch / m_nRows));
#else

    int oi = (cw - (7 * (int) (cw / 7))) / 2;
    int of = (cw - (7 * (int) (cw / 7))) - oi;
    int xi, wxi;

    for (i = 0; i < 7; i++) {
	if (i == 0)
	    xi = 0;
	else
	    xi = oi;
	if (i == 0)
	    wxi = oi;
	else if (i == 6)
	    wxi = of;
	else
	    wxi = 0;

	weekdays[i] = new Fl_Box((cw / 7) * (i % 7) + cx + xi,
				 (ch / 8) * ((i / 7) + 1) + cy,
				 (cw / 7) + wxi, (ch / 8));
    }

    prv_month->resize(cx + (cw / 10), cy, (cw / 10), (ch / 8));
    nxt_month->resize(cx + (cw / 10) * 8, cy, (cw / 10), (ch / 8));
    prv_year->resize(cx, cy, (cw / 10), (ch / 8));
    nxt_year->resize(cx + (cw / 10) * 9, cy, (cw / 10), (ch / 8));
    caption->resize(cx + (cw / 10) * 2, cy, (cw / 10) * 6, (ch / 8));

    Fl_Calendar_Base::csize(cx, cy + (2 * ch / 8), cw, (6 * ch / 8));

#endif
}

void
Fl_Calendar::update()
{
#ifdef PIXIL
    int dow = date.day_of_week(date.year(), date.month(), 1);
    int dim = date.days_in_month(date.month(), date.leap_year(date.year()));
#else
    int dow = day_of_week(year(), month(), 1);
    int dim = days_in_month(month(), leap_year(year()));
#endif

    int i;

    for (i = dow; i < (dim + dow); i++) {
	char t[8];
	sprintf(t, "%d", (i - dow + 1));
	days[i]->label(strdup(t));
    }

    char tmp[32];
    sprintf(tmp, "%.3s %d", month_name[month() - 1], year());
    //sprintf (tmp, "%s %d", month_name[month ()-1], year ());
#ifdef PIXIL
    update_buttons();
#else
    Fl_Calendar_Base::update();
#endif

#ifdef PIXIL
    if (m_bCaption) {
	if (caption->label())
	    free((void *) caption->label());
	caption->label(strdup(tmp));
    }
#else
    if (caption->label())
	free((void *) caption->label());

    caption->label(strdup(tmp));
#endif

    redraw();
}

#ifdef PIXIL

void
Fl_Calendar::update_buttons()
{
#ifdef PIXIL
    int dow = date.day_of_week(date.year(), date.month(), 1);
    int dim = date.days_in_month(date.month(), date.leap_year(date.year()));
#else
    int dow = day_of_week(year(), month(), 1);
    int dim = days_in_month(month(), leap_year(year()));
#endif
    int i;
    int val = 0;

    for (i = 0; i < dow; i++) {
	days[i]->hide();
    }

    for (i = (dim + dow); i < (6 * 7); i++) {
	days[i]->hide();
    }

    for (i = dow; i < (dim + dow); i++) {
	char t[8];
	sprintf(t, "%d", (i - dow + 1));
	days[i]->label(strdup(t));
	days[i]->color(NxApp::GlobalColor(APP_BG));
	days[i]->labelcolor(NxApp::GlobalColor(APP_FG));

	if ((i - dow + 1) == day()) {
	    days[i]->color(NxApp::GlobalColor(HILIGHT));
	    days[i]->labelcolor(NxApp::GlobalColor(APP_BG));
	    days[i]->selection_color(NxApp::GlobalColor(APP_BG));
	    val = (i - dow + 1);
	}
	days[i]->redraw();
	days[i]->show();
    }

    if (WEEK == _type) {
	set_week_buttons(this, val);
    }
    redraw();
}
#endif

void
Fl_Calendar::previous_month()
{
#ifdef PIXIL
    date.previous_month();
#else
    previous_month();
#endif
    update();
}

void
Fl_Calendar::next_month()
{
#ifdef PIXIL
    date.next_month();
#else
    next_month();
#endif
    update();
}

void
Fl_Calendar::previous_year()
{
#ifdef PIXIL
    date.previous_year();
#else
    previous_year();
#endif
    update();
}

void
Fl_Calendar::next_year()
{
#ifdef PIXIL
    date.next_year();
#else
    next_year();
#endif

    update();
}

int
Fl_Calendar::handle(int event)
{
    int m, d, y, o = 0, md;

    switch (event) {
    case FL_FOCUS:
    case FL_UNFOCUS:
	return 1;

    case FL_KEYBOARD:
	m = month();
	d = day();
	y = year();
	switch (Fl::event_key()) {
	case FL_Enter:
	    selected_day_ = d;
	    do_callback(this, d);
	    break;
	case FL_Up:
	    o = -7;
	    break;
	case FL_Down:
	    o = 7;
	    break;
	case FL_Right:
	    o = 1;
	    break;
	case FL_Left:
	    o = -1;
	    break;
	case FL_Page_Up:
	    previous_month();
	    return 1;
	case FL_Page_Down:
	    next_month();
	    return 1;
	default:
	    return Fl_Group::handle(event);
	}
#ifdef PIXIL
	if (date.valid(y, m, d + o))
	    date.set_date(y, m, d + o);
	else {
	    if (o < 0) {
		previous_month();
		m = date.month();
		y = date.year();
		md = date.days_in_month(m, date.leap_year(y));
		d = d + o + md;
		date.set_date(y, m, d);
	    } else {
		md = date.days_in_month(m, date.leap_year(y));
		next_month();
		m = date.month();
		y = date.year();
		d = d + o - md;
		date.set_date(y, m, d);
	    }
	}
#else
	if (valid(y, m, d + o))
	    set_date(y, m, d + o);
	else {
	    if (o < 0) {
		previous_month();
		m = month();
		y = year();
		md = days_in_month(m, leap_year(y));
		d = d + o + md;
		set_date(y, m, d);
	    } else {
		md = days_in_month(m, leap_year(y));
		next_month();
		m = month();
		y = year();
		d = d + o - md;
		set_date(y, m, d);
	    }
	}
#endif
	update();
	return 1;
    }
    return Fl_Group::handle(event);
}


// The following can be static since the popup calendar behaves
// as a modal dialog

struct datestruct
{
    int flag;
    int m;
    int d;
    int y;
    char szDte[20];
};

static datestruct seldate;
static Fl_Window *popcal_form;
static Fl_Calendar *popcal;

static void
popcal_cb(Fl_Calendar * c, long d)
{
    if (d) {
	seldate.flag = 1;
	seldate.m = c->month();
	seldate.d = c->day();
	seldate.y = c->year();
    }
}

static void
makepopcal()
{
    if (popcal_form)
	return;
    Fl_Window *w = popcal_form = new Fl_Window(7 * 20 + 4, 8 * 20 + 4);
    w->clear_border();
    w->box(FL_UP_BOX);
    (popcal = new Fl_Calendar(2, 2));
    popcal->callback((Fl_Callback *) popcal_cb);
    w->end();
    w->set_modal();
    return;
}

static datestruct *
fl_popcal(int popcalfmt)
{
    makepopcal();
    seldate.flag = -1;
    popcal_form->show();
    for (;;) {
	Fl::wait();
	if (seldate.flag > -1)
	    break;
    }
    popcal_form->hide();
    if (seldate.flag == 1)
	strcpy(seldate.szDte, popcal->to_string(popcalfmt));
    else
	seldate.szDte[0] = 0;
    return &seldate;
}

#include "pixmaps/calendar.xpm"

void
Fl_Date_Input::btnDate_Input_cb_i()
{
    datestruct *retdate = fl_popcal(popcalfmt);
    if (retdate->flag == 1)
	Input.value(retdate->szDte);
    return;
}

// button callback has to execute the real callback contained in
// the parent Group widget

void
btnDate_Input_cb(Fl_Widget * v, void *d)
{
    ((Fl_Date_Input *) (v->parent()))->btnDate_Input_cb_i();
    return;
}


Fl_Date_Input::Fl_Date_Input(int X, int Y, int W, int H, char *L)
:  Fl_Group(X, Y, W, H, L),
Input(X, Y, W - H, H), Btn(X + W - H + 1, Y + 1, H - 2, H - 2)
{
    xpos = X;
    ypos = Y;
    width = W;
    height = H;

    (new Fl_Pixmap(calendar_xpm))->label(&Btn);
    Btn.callback((Fl_Callback *) btnDate_Input_cb, 0);
    popcalfmt = 0;
    end();
}

// Date_Input value is contained in the Input widget

void
Fl_Date_Input::value(const char *s)
{
    Input.value(s);
}

const char *
Fl_Date_Input::value()
{
    return (Input.value());
}

#if FL_MAJOR_VERSION == 1
void
Fl_Date_Input::text_font(int tf)
{
    Input.textfont(tf);
}
#else
void
Fl_Date_Input::text_font(Fl_Font tf)
{
    Input.text_font(tf);
}
#endif

#if FL_MAJOR_VERSION == 1
void
Fl_Date_Input::text_size(int sz)
{
    Input.textsize(sz);
}
#else
void
Fl_Date_Input::text_size(int sz)
{
    Input.text_size(sz);
}
#endif

void
Fl_Date_Input::format(int fmt)
{
    switch (fmt) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
	popcalfmt = fmt;
	break;
    default:
	popcalfmt = 0;
    }
}

Fl_Agenda_Calendar::Fl_Agenda_Calendar(int x, int y, int w, int h,
				       const char *l, int title_height):
Fl_Calendar(x, y, w, h, l)
{
    if (title_height < 0) {
	title_height = h / 8;
    }

    int i;
    for (i = 0; i < 7; i++) {
	weekdays[i]->size(weekdays[i]->w(), (h - title_height) / 7);
    }

    prv_year->size(prv_year->w(), title_height);
    prv_year->label("Y-");

    prv_month->size(prv_month->w(), title_height);
    prv_month->label("M-");

    nxt_month->size(nxt_month->w(), title_height);
    nxt_month->label("M+");

    nxt_year->size(nxt_year->w(), title_height);
    nxt_year->label("Y+");

    caption->size(caption->w(), title_height);

    Fl_Calendar_Base::csize(x, y + title_height + (h - title_height) / 7, w,
			    h - title_height - (h - title_height) / 7);
    update();
}
