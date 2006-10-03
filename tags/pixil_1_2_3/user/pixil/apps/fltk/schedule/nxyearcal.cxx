/*                                                                       
 * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.     
 *                                                                       
 * This file is part of the PIXIL Operating Environment                 
 *                                                                       
 * The use, copying and distribution of this file is governed by one    
 * of two licenses, the PIXIL Commercial License, or the GNU General    
 * Public License, version 2.                                           
 *                                                                       
 * Licensees holding a valid PIXIL Commercial License may use this file 
 * in accordance with the PIXIL Commercial License Agreement provided   
 * with the Software. Others are governed under the terms of the GNU   
 * General Public License version 2.                                    
 *                                                                       
 * This file may be distributed and/or modified under the terms of the  
 * GNU General Public License version 2 as published by the Free        
 * Software Foundation and appearing in the file LICENSE.GPL included   
 * in the packaging of this file.                                      
 *                                                                       
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING  
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A            
 * PARTICULAR PURPOSE.                                                  
 *                                                                       
 * RESTRICTED RIGHTS LEGEND                                             
 *                                                                     
 * Use, duplication, or disclosure by the government is subject to      
 * restriction as set forth in paragraph (b)(3)(b) of the Rights in     
 * Technical Data and Computer Software clause in DAR 7-104.9(a).       
 *                                                                      
 * See http://www.pixil.org/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://www.pixil.org/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <nxapp.h>
#include <stdio.h>
#include "nxyearcal.h"
#include "nxschedule.h"

#ifdef DEBUG
#define DPRINT(str, args...) printf("DEBUG NxYearCall: " str, ## args)
#else
#define DPRINT(args...)
#endif

const char *
    NxYearCal::month_name[] = {
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

NxYearCal::NxYearCal(time_t time, int X, int Y, int W, int H, const char *L)
    :
Fl_Group(X, Y, W, H, L)
{
    tm *tt;

    tt = localtime(&time);
    cur_year = tt->tm_year;
    x_space = w() / 3;
    y_space = h() / 4;
    day_w = x_space / 7;
    day_h = y_space / 7;

    draw_day_box = 0;
    release_box = 0;

    DPRINT("day_w [%d] day_h [%d]\n", day_w, day_h);

#ifdef NOT_USED			// seperated out of widget
    static char year_buf[6];
    backYear_Button = new NxButton(w() / 2 - 15, BUTTON_Y, 15, 15, "@<");
    backYear_Button->labeltype(FL_SYMBOL_LABEL);
    backYear_Button->box(FL_FLAT_BOX);
    backYear_Button->callback(backYear_callback, this);
    add(backYear_Button);

    forwYear_Button = new NxButton(w() / 2, BUTTON_Y, 15, 15, "@>");
    forwYear_Button->labeltype(FL_SYMBOL_LABEL);
    forwYear_Button->box(FL_FLAT_BOX);
    forwYear_Button->callback(forwYear_callback, this);
    add(forwYear_Button);

    strftime(year_buf, 5, "%Y", tt);
    year_Box = new NxBox(w() / 2 - 55, BUTTON_Y - 1, 50, 20, year_buf);
    add(year_Box);
#endif
}

void
NxYearCal::resize(int X, int Y, int W, int H)
{

    x(X);
    y(Y);
    w(W);
    h(H);

    x_space = w() / 3;
    y_space = (BUTTON_Y - 1) / 4;
    day_w = x_space / 7;
    day_h = y_space / 7;

}

void
NxYearCal::ChangeYear(int year)
{
    cur_year = year;
}

int
NxYearCal::Month(int X, int Y)
{
    int row;
    int col;
    int month;

    X -= x();
    Y -= y();

    if (Y > y_space * 4) {
	DPRINT("Outsid of year grid\n");
	return -1;
    }
    col = X / x_space;
    row = Y / y_space;

    month = 3 * row + col;
    DPRINT("month [%d]\n", month);

    return month;
}

int
NxYearCal::MonDay(int X, int Y)
{
    int row, col;
    int day_x, day_y;
    int x_check, y_check;
    int month;
    int d_col, d_row;
    tm tt;

    X -= x();
    Y -= y();

    memset(&tt, 0, sizeof(tt));

    col = X / x_space;
    row = Y / y_space;
    month = 3 * row + col;

    day_x = X % x_space;
    day_y = Y % y_space;

    if (day_y <= day_h) {
	return 0;
    }

    tt.tm_year = cur_year;
    tt.tm_mon = month;
    tt.tm_mday = 1;
    mktime(&tt);

    x_check = col * x_space + (tt.tm_wday * day_w);
    y_check = row * y_space + day_h;
    d_row = 0;
    while (month == tt.tm_mon) {
	if (((X >= x_check) && (X < x_check + day_w)) &&
	    ((Y >= y_check) && (Y < y_check + day_h))) {
	    return tt.tm_mday;
	}
	tt.tm_mday++;
	mktime(&tt);
	d_col = tt.tm_wday;
	if (0 == d_col)
	    d_row++;
	x_check = col * x_space + (tt.tm_wday * day_w);
	y_check = row * y_space + day_h + (d_row * day_h);

    }

    return -1;
}

void
NxYearCal::DrawDateBox(int x, int y)
{
    int month;
    int day;
    char buf[4];


    month = Month(x, y);
    if (-1 != month) {
	day = MonDay(x, y);
	DPRINT("day [%d]\n", day);
	if (0 < day) {
	    if (x - 22 < 0)
		x = 22;
	    if (y - 22 < 0)
		y = 22;
	    DPRINT("should be showing day box\n");
	    sprintf(buf, "%d", day);
	    fl_color(FL_WHITE);
	    fl_rectf(x - 20, y - 20, 20, 20);
	    fl_color(FL_BLACK);
	    fl_rect(x - 20, y - 20, 20, 20);
	    fl_rect(x - 1 - 20, y - 1 - 20, 22, 22);
	    fl_rect(x - 2 - 20, y - 2 - 20, 24, 24);
	    //fl_font(FL_TIMES, 12);        
	    NxApp::Instance()->def_font();
	    fl_draw(buf, x - 20, y - 20, 20, 20, FL_ALIGN_CENTER);
	}
    }
}

void
NxYearCal::check_appts()
{
    int idx;
    int rec_array[255];
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());
    NxTodo *note = new NxTodo;
    tm t_year;
    time_t startDay = 0;

    memset(&appts, 0, sizeof(appts));

    for (idx = 0; idx < 255; idx++)
	rec_array[idx] = -1;

    db->Select(db_name, rec_array, 255);

    t_year.tm_sec = 0;
    t_year.tm_min = 0;
    t_year.tm_hour = 0;
    t_year.tm_mday = 1;
    t_year.tm_mon = 0;
    t_year.tm_year = cur_year;
    startDay = mktime(&t_year);

    for (idx = 0; idx < 255; idx++) {
	if (-1 == rec_array[idx])
	    continue;
	pThis->ExtractRecord(note, rec_array[idx]);

	for (int jdx = 0; jdx < 366; jdx++) {
	    if (pThis->IsForToday(note, startDay + (jdx * 86400),
				  startDay + ((jdx + 1) * 86400) - 1)) {
		appts[jdx] = 1;
	    }
	}
		/****
		date = localtime(&note->startTime);

		if(cur_year == date->tm_year) {
			appts[date->tm_yday] = 1;
		}
		*****/
    }

    note = NULL;
    delete note;
}

void
NxYearCal::draw()
{
    int row, col;
    int d_row, d_col;
    int x_day, y_day;
    int idx;
    int month;
    tm tt;
    time_t today;
    tm *t_today;
    char buf[4];
    static int old_x = -100;
    static int old_y = -100;
    static char year_buf[6];
    int day = 0;

    check_appts();

    memset(&tt, 0, sizeof(tt));
    tt.tm_year = cur_year;
    tt.tm_mday = 1;

    mktime(&tt);

    strftime(year_buf, 5, "%Y", &tt);
    year_Box->label(year_buf);
    year_Box->redraw();

    if ((-100 != old_x && -100 != old_y && draw_day_box)
	|| (1 == release_box)) {
	if (old_x - 22 < 0)
	    old_x = 22;
	if (old_y - 22 < 0)
	    old_y = 22;
	fl_clip(old_x - 22, old_y - 22, 24, 24);
    }

    fl_color(FL_WHITE);
    fl_rectf(x(), y(), 3 * x_space, 4 * y_space);

    today = time(0);
    t_today = localtime(&today);

    day = 0;
    for (month = 0; month <= 11; month++) {
	row = month / 3;
	col = month - (3 * row);
	fl_color(NxApp::Instance()->getGlobalColor(BUTTON_FACE));
	fl_rectf(col * x_space + x(), row * y_space + 1 + y(), x_space,
		 day_h + 1);
	fl_color(NxApp::Instance()->getGlobalColor(BUTTON_TEXT));
	//fl_font(FL_TIMES, day_h - 2);
	NxApp::Instance()->def_font();
	fl_draw(month_name[month], col * x_space + 7 + x(),
		row * y_space + 2 + y(), y_space, day_h - 2, FL_ALIGN_CENTER);

	tt.tm_mon = month;
	tt.tm_mday = 1;
	mktime(&tt);
	x_day = col * x_space + (tt.tm_wday * day_w) + 2 + x();
	y_day = row * y_space + day_h + 2 + y();

	fl_color(NxApp::Instance()->getGlobalColor(APP_FG));
	d_row = 0;
	NxApp::Instance()->def_small_font();
	while (month == tt.tm_mon) {
	    //DPRINT("day [%d]\n", day);
	    sprintf(buf, "%d", tt.tm_mday);
	    if (tt.tm_year == t_today->tm_year
		&& tt.tm_yday == t_today->tm_yday) {
		Fl_Color old_col = fl_color();
		fl_color(NxApp::Instance()->getGlobalColor(HILIGHT));
		fl_rectf(x_day, y_day, day_w, day_h + 1);
		fl_color(NxApp::Instance()->getGlobalColor(HILIGHT_TEXT));
		fl_draw(buf, x_day, y_day, day_w, day_h, FL_ALIGN_CENTER);
		fl_color(old_col);
	    } else
		fl_draw(buf, x_day, y_day, day_w, day_h, FL_ALIGN_CENTER);
	    if (1 == appts[day]) {
		Fl_Color old_col = fl_color();
		fl_rect(x_day, y_day, day_w, day_h + 1);
		fl_color(NxApp::Instance()->getGlobalColor(APP_FG));
		if (tt.tm_year != t_today->tm_year
		    && tt.tm_yday != t_today->tm_yday)
		    fl_draw(buf, x_day, y_day, day_w, day_h, FL_ALIGN_CENTER);
		fl_color(old_col);
	    }
	    tt.tm_mday++;
	    mktime(&tt);
	    d_col = tt.tm_wday;
	    if (0 == d_col)
		d_row++;
	    x_day = col * x_space + (tt.tm_wday * day_w) + 2 + x();
	    y_day = row * y_space + day_h + 2 + (d_row * day_h) + y();
	    day++;
	}
    }

    fl_color(FL_BLACK);

    for (idx = 1; idx <= 2; idx++)
	fl_line(x() + (idx * x_space), y(), x() + (idx * x_space),
		y() + (4 * y_space) + 1);

    for (idx = 0; idx <= 3; idx++)
	fl_line(x(), y() + (idx * y_space), x() + (3 * x_space) - 1,
		y() + (idx * y_space));

    fl_line(x(), y() + (4 * y_space) + 1, x() + (3 * x_space) - 1,
	    y() + (4 * y_space) + 1);
    if ((-100 != old_x && -100 != old_y && draw_day_box)
	|| (1 == release_box))
	fl_pop_clip();

    release_box = 0;

    if (draw_day_box) {
	DrawDateBox(Fl::event_x(), Fl::event_y());
    }

    old_x = Fl::event_x();
    old_y = Fl::event_y();
    Fl_Group::draw();

}

int
NxYearCal::handle(int event)
{
    switch (event) {
    case FL_PUSH:
	if (Fl::event_inside((Fl_Widget *) this)) {
	    if (Fl::event_y() <= y() + (4 * y_space)) {
		draw_day_box = 1;
		redraw();
	    }
	    Fl_Group::handle(event);
	}
	return 1;
    case FL_RELEASE:
	draw_day_box = 0;
	release_box = 1;
	maybe_do_callback();
	redraw();
	Fl_Group::handle(event);
	return 1;
    case FL_DRAG:
	if (Fl::event_inside((Fl_Widget *) this)) {
	    draw_day_box = 1;
	    release_box = 0;
	    redraw();
	}
	Fl_Group::handle(event);
	return 1;
    default:
	return 0;
    }
    Fl_Group::handle(event);
}

NxYearCal::~NxYearCal()
{
}

void
NxYearCal::maybe_do_callback()
{
    int month;
    int day;
    time_t time;
    tm tt;

    memset(&tt, 0, sizeof(tt));

    month = Month(Fl::event_x(), Fl::event_y());
    if (month >= 0) {
	day = MonDay(Fl::event_x(), Fl::event_y());
	tt.tm_year = cur_year;
	tt.tm_mday = day;
	if (day == 0)
	    tt.tm_mday = 1;
	tt.tm_mon = month;
	time = mktime(&tt);
	if (day == 0) {
	    if (pMonthCallback)
		pMonthCallback(this, &time);
	    draw_day_box = 0;
	    release_box = 0;
	    return;
	}
	if (day > 0) {
	    if (pDayCallback)
		pDayCallback(this, &time);
	    draw_day_box = 0;
	    release_box = 0;
	    return;
	}
    }
}

void
NxYearCal::SetMonthCallback(void (*cb) (Fl_Widget *, void *))
{
    pMonthCallback = cb;
}

void
NxYearCal::SetDayCallback(void (*cb) (Fl_Widget *, void *))
{
    pDayCallback = cb;
}

void
NxYearCal::backYear_callback(Fl_Widget * fl, void *l)
{
    NxYearCal *pThis = (NxYearCal *) l;

    pThis->cur_year--;
    pThis->year_Box->redraw();
    pThis->year_Box->hide();
    pThis->year_Box->show();
}

void
NxYearCal::forwYear_callback(Fl_Widget * fl, void *l)
{
    NxYearCal *pThis = (NxYearCal *) l;

    pThis->cur_year++;
    pThis->year_Box->redraw();
    pThis->year_Box->hide();
    pThis->year_Box->show();
}
