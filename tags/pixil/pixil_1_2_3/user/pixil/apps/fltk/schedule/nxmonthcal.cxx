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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <FL/Fl.H>
#include "nxmonthcal.h"
#include "nxschedule.h"
#include <nxapp.h>

static void
fl_calendar_button_cb(Fl_Button * a, void *b)
{

    long j = 0;
    NxMonthCalendar *c = (NxMonthCalendar *) b;
    Fl_Button *sb;
    int numdays = c->days_in_month() + 1;

    for (int i = 1; i < numdays; i++) {
	sb = c->day_button(i);
	sb->color(NxApp::Instance()->getGlobalColor(APP_BG));
	sb->labelcolor(NxApp::Instance()->getGlobalColor(APP_FG));

	if (a == sb) {
	    c->day(i);
	    j = i;
	    //sb->color (NxApp::Instance()->getGlobalColor(APP_SEL));
	    //sb->labelcolor(NxApp::Instance()->getGlobalColor(APP_BG));
	}
    }

    c->redraw();
    c->do_callback(c, j);

    tm d;

    memset(&d, 0, sizeof(d));
    d.tm_year = c->year() - 1900;
    d.tm_mon = c->month() - 1;
    d.tm_mday = c->day();

    c->SetPickedDate(mktime(&d));
    (c->GetDateCB())(a, (void *) (NxApp::Instance()));
}

NxMonthCalendar::NxMonthCalendar(int x, int y, int w, int h,
				 const char *l, bool bCaption, int type,
				 NxDb * db, char *db_name)
    :
Fl_Calendar(x, y, w, h, l, type)
{
    int idx;
    memset(_db_name, 0, sizeof(_db_name));
    if (db_name)
	strcpy(_db_name, db_name);

    _db = db;

    for (idx = 0; idx < (7 * 6); idx++) {
	this->remove(days[idx]);
	days[idx] = NULL;
	delete days[idx];
    }

    for (idx = 0; idx < (7 * 6); idx++) {
	days[idx] = new NxMonthButton((w / 7) * (idx % 7) + x,
				      (h / (m_nRows - 2)) * (idx / 7) + y,
				      (w / 7), (h / (m_nRows - 2)));
#ifndef FLTK_2
	days[idx]->down_box(FL_THIN_DOWN_BOX);
	days[idx]->labelsize(10);
#else
	days[idx]->label_size(10);
#endif
	days[idx]->box(FL_THIN_UP_BOX);

	days[idx]->color(NxApp::Instance()->getGlobalColor(APP_BG));
	days[idx]->selection_color(NxApp::Instance()->
				   getGlobalColor(APP_SEL));
	days[idx]->labelcolor(NxApp::Instance()->getGlobalColor(APP_FG));

	days[idx]->callback((Fl_Callback *) & fl_calendar_button_cb,
			    (void *) this);
    }
    if (m_bCaption)
	Fl_Calendar_Base::csize(x, y + (2 * h / m_nRows), w,
				(6 * h / m_nRows));
    else
	Fl_Calendar_Base::csize(x, y + (h / m_nRows), w, (6 * h / m_nRows));
    update();
}

void
NxMonthCalendar::update()
{
    int dow = date.day_of_week(date.year(), date.month(), 1);
    int dim = date.days_in_month(date.month(), date.leap_year(date.year()));
    int i;

    for (i = dow; i < (dim + dow); i++) {
	char t[8];
	sprintf(t, "%d", (i - dow + 1));
	days[i]->label(strdup(t));
	days[i]->align((Fl_Align)
		       (FL_ALIGN_INSIDE | FL_ALIGN_TOP | FL_ALIGN_LEFT));
    }

    char tmp[32];
    sprintf(tmp, "%s %d", date.month_name[date.month() - 1], date.year());
    update_buttons();
    if (m_bCaption) {
	if (caption->label())
	    free((void *) caption->label());
	caption->label(strdup(tmp));
    }

    redraw();
}

void
CreateDateRange(time_t * e, time_t * l, time_t start_day, time_t end_day)
{
    tm start_tm;
    tm end_tm;

    memcpy(&start_tm, localtime(&start_day), sizeof(start_tm));
    memcpy(&end_tm, localtime(&end_day), sizeof(end_tm));

    start_tm.tm_sec = 0;
    start_tm.tm_min = 0;
    start_tm.tm_hour = 0;

    end_tm.tm_sec = 59;
    end_tm.tm_min = 59;
    end_tm.tm_hour = 23;

    *e = mktime(&start_tm);
    *l = mktime(&end_tm);
}

bool
IsForToday(time_t start_time, time_t EarlyDay, time_t LateDay)
{
    if (start_time >= EarlyDay && start_time <= LateDay)
	return true;
    else
	return false;
}

void
NxMonthCalendar::update_buttons()
{

    int dow = date.day_of_week(date.year(), date.month(), 1);
    int dim = date.days_in_month(date.month(), date.leap_year(date.year()));
    int i;
    int vals[3];
    int rec_array[255];
    time_t earlyDay;
    time_t lateDay;
    time_t start_time;
    time_t end_time;
    time_t day = 0;
    tm d;
    int idx = 0;
    NxTodo *note = new NxTodo;
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());

    memset(&d, 0, sizeof(d));
    d.tm_year = year() - 1900;
    d.tm_mon = month() - 1;
    d.tm_mday = 0;


    for (idx = 0; idx < 255; idx++)
	rec_array[idx] = -1;

    _db->Select(_db_name, rec_array, 255);

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
	days[i]->color(NxApp::Instance()->getGlobalColor(APP_BG));
	days[i]->labelcolor(NxApp::Instance()->getGlobalColor(APP_FG));

	d.tm_mday++;
	day = mktime(&d);

	CreateDateRange(&earlyDay, &lateDay, day, day);

	memset(vals, 0, sizeof(vals));
	for (idx = 0; idx < 255; idx++) {
	    if (-1 != rec_array[idx]) {

		pThis->ExtractRecord(note, rec_array[idx]);
		start_time = note->startTime;
		end_time = note->endTime;

		if (vals[0] && vals[1] && vals[2])
		    break;

		if (pThis->IsForToday(note, earlyDay, lateDay)) {
		    tm *tt = localtime(&start_time);
		    if (tt->tm_hour >= 0 && tt->tm_hour < 8) {
			vals[0] = 1;
		    } else if (tt->tm_hour >= 8 && tt->tm_hour < 16) {
			vals[1] = 1;
		    } else {
			vals[2] = 1;
		    }
		    tt = localtime(&end_time);
		    if (vals[0]) {
			if (tt->tm_hour >= 8 && tt->tm_hour < 16)
			    vals[1] = 1;
			if (tt->tm_hour >= 16) {
			    vals[1] = 1;
			    vals[2] = 1;
			}
		    }
		    if (vals[1]) {
			if (tt->tm_hour >= 16) {
			    vals[2] = 1;
			}
		    }
		}
	    }
	}
	((NxMonthButton *) days[i])->SetAppts(vals);
	days[i]->redraw();
	days[i]->show();
    }

    redraw();
    note = NULL;
    delete note;
}

void
NxMonthCalendar::DateCallback(void (*cb) (Fl_Widget *, void *))
{
    m_pDateCallback = cb;
}

time_t NxMonthCalendar::GetPickedDate()
{
    return m_nDatePicked;
}

void
NxMonthCalendar::SetPickedDate(time_t t)
{
    tm *tt = localtime(&t);

    set_date(tt->tm_year + 1900, tt->tm_mon + 1, tt->tm_mday);
    m_nDatePicked = t;
    update();
}

NxMonthButton::NxMonthButton(int X, int Y, int W, int H, const char *L)
    :
Fl_Button(X, Y, W, H, L)
{
    color(NxApp::Instance()->getGlobalColor(APP_BG));
    labelcolor(NxApp::Instance()->getGlobalColor(APP_FG));

    for (int idx = 0; idx < 3; idx++)
	appt[idx] = 0;
}

void
NxMonthButton::SetAppts(int vals[])
{
    for (int idx = 0; idx < 3; idx++) {
	appt[idx] = vals[idx];
    }
}

void
NxMonthButton::draw()
{
    Fl_Color col = value()? selection_color() : color();
#ifdef PDA
    if (value()) {
	Fl_Color old_label_clr = labelcolor();

	labelcolor(contrast(color(), labelcolor()));
	draw_box(down(box()), selection_color());
	draw_label();
	fl_color(labelcolor());
	for (int idx = 0; idx < 3; idx++) {
	    if (0 == idx) {
		if (appt[idx])
		    fl_rectf(x() + w() - 5 - w() / 10, y() + 2, w() / 10,
			     h() / 3 - 4);
	    } else if (1 == idx) {
		if (appt[idx])
		    fl_rectf(x() + w() - 5 - w() / 10, y() + 2 + h() / 3,
			     w() / 10, h() / 3 - 4);
	    } else {
		if (appt[idx])
		    fl_rectf(x() + w() - 5 - w() / 10,
			     y() + 2 + 2 * (h() / 3), w() / 10, h() / 3 - 4);
	    }
	}
	labelcolor(old_label_clr);
    } else {
	draw_box(box(), col);
	draw_label();
	fl_color(labelcolor());
	for (int idx = 0; idx < 3; idx++) {
	    if (0 == idx) {
		if (appt[idx])
		    fl_rectf(x() + w() - 5 - w() / 10, y() + 2, w() / 10,
			     h() / 3 - 4);
	    } else if (1 == idx) {
		if (appt[idx])
		    fl_rectf(x() + w() - 5 - w() / 10, y() + 2 + h() / 3,
			     w() / 10, h() / 3 - 4);
	    } else {
		if (appt[idx])
		    fl_rectf(x() + w() - 5 - w() / 10,
			     y() + 2 + 2 * (h() / 3), w() / 10, h() / 3 - 4);
	    }
	}
    }
#else
    draw_box(value()? (down_box()? down_box() : down(box())) : box(), col);
    draw_label();
    for (int idx = 0; idx < 3; idx++) {
	if (0 == idx) {
	    if (appt[idx])
		fl_rectf(x() + w() - 5 - w() / 10, y() + 2, w() / 10,
			 h() / 3 - 4);
	} else if (1 == idx) {
	    if (appt[idx])
		fl_rectf(x() + w() - 5 - w() / 10, y() + 2 + h() / 3,
			 w() / 10, h() / 3 - 4);
	} else {
	    if (appt[idx])
		fl_rectf(x() + w() - 5 - w() / 10, y() + 2 + 2 * (h() / 3),
			 w() / 10, h() / 3 - 4);
	}
    }
#endif
}
