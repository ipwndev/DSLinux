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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "nxschedule.h"

#define TIME_WINDOW_START 0
#define TIME_WINDOW_END   1
#define TIME_WINDOW_NONE  2

static char *
_FormatTime(tm * tt)
{
    static char buf[30];


    sprintf(buf, "%d:%02d%s",
	    HR_12(tt->tm_hour), tt->tm_min, AM_PM(tt->tm_hour));

    return buf;
}

void
NxSchedule::time_callback(void (*cb) (Fl_Widget *, void *))
{
    m_pTimePickerCallback = cb;
}

void
NxSchedule::SetTimes(time_t nStartTime, time_t nEndTime)
{
    memcpy(&m_StartTime, localtime(&nStartTime), sizeof(m_StartTime));
    memcpy(&m_EndTime, localtime(&nEndTime), sizeof(m_EndTime));

    m_StartTime.tm_sec = 0;
    m_EndTime.tm_sec = 0;

    Time_UpdateDisplay();
}

void
NxSchedule::GetTimes(time_t * nStartTime, time_t * nEndTime)
{
    *nStartTime = mktime(&m_StartTime);
    *nEndTime = mktime(&m_EndTime);
}

void
NxSchedule::doneTime_callback(Fl_Widget * o, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    (*pThis->m_pTimePickerCallback) (o, l);
}

void
NxSchedule::cancelTime_callback(Fl_Widget * o, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    (*pThis->m_pTimePickerCallback) (0, l);
}

void
NxSchedule::timeStart_callback(Fl_Widget * o, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->m_nTimeState = TIME_WINDOW_START;
    pThis->Time_UpdateDisplay();

}

void
NxSchedule::timeEnd_callback(Fl_Widget * o, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->m_nTimeState = TIME_WINDOW_END;
    pThis->Time_UpdateDisplay();
}

void
NxSchedule::timeNone_callback(Fl_Widget * o, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->m_nTimeState = TIME_WINDOW_NONE;
    pThis->Time_UpdateDisplay();
}

// for when the user selects a new hour
void
NxSchedule::timeHour_callback(Fl_Widget * o, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    tm *p;

    if (pThis->m_nTimeState == TIME_WINDOW_NONE)
	pThis->m_nTimeState = TIME_WINDOW_START;

    if (pThis->m_nTimeState == TIME_WINDOW_START) {
	p = &pThis->m_StartTime;
    } else {
	p = &pThis->m_EndTime;
    }

    if (pThis->m_pTimeHour->value()) {

	if (IS_AM(p->tm_hour)) {
	    p->tm_hour = pThis->m_pTimeHour->value();
	} else {
	    p->tm_hour = pThis->m_pTimeHour->value() + 12;
	}
    }

    if (pThis->m_pTimeMinute->value())
	p->tm_min = (pThis->m_pTimeMinute->value() - 1) * 5;

    if (p)
	p->tm_sec = 0;

    pThis->Time_UpdateDisplay();
}

void
NxSchedule::timeAM_callback(Fl_Widget * o, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    if (pThis->m_nTimeState == TIME_WINDOW_START) {
	if (pThis->m_StartTime.tm_hour >= 12)
	    pThis->m_StartTime.tm_hour -= 12;
    } else {
	if (pThis->m_EndTime.tm_hour >= 12)
	    pThis->m_EndTime.tm_hour -= 12;
    }

    pThis->Time_UpdateDisplay();
}

void
NxSchedule::timePM_callback(Fl_Widget * o, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    if (pThis->m_nTimeState == TIME_WINDOW_START) {
	if (pThis->m_StartTime.tm_hour < 12)
	    pThis->m_StartTime.tm_hour += 12;
    } else {
	if (pThis->m_EndTime.tm_hour < 12) {
	    pThis->m_EndTime.tm_hour += 12;
	}
    }

    if (pThis->m_EndTime.tm_mday != pThis->m_StartTime.tm_mday)
	pThis->m_EndTime.tm_mday = pThis->m_StartTime.tm_mday;
    pThis->Time_UpdateDisplay();
}

void
NxSchedule::Time_UpdateDisplay()
{
    bool start = false;
    bool end = false;
    bool none = false;
    bool am = false;
    bool pm = false;
    tm *p;
    time_t start_time = 0;
    time_t end_time = 0;

    // which mode are we in?

    switch (m_nTimeState) {
    default:
    case TIME_WINDOW_START:
	start = true;
	break;
    case TIME_WINDOW_END:
	end = true;
	break;
    case TIME_WINDOW_NONE:
	none = true;
	break;
    }

    start_time = mktime(&m_StartTime);
    end_time = mktime(&m_EndTime);
    if (end_time < start_time)
	m_EndTime = m_StartTime;

    // are we am or pm?

    if (start) {
	p = &m_StartTime;
    } else {
	p = &m_EndTime;
    }

    if (p->tm_hour >= 12)
	pm = true;
    else
	am = true;

    p->tm_sec = 0;

    // moidfy all the on screen controls
    m_pTimeStart->value(start);
    m_pTimeEnd->value(end);
    m_pTimeNone->value(none);
    m_pTimeAM->value(am);
    m_pTimePM->value(pm);

    if (m_pTimeEnd->label())
	free((void *) m_pTimeEnd->label());

    m_pTimeEnd->label(strdup(_FormatTime(&m_EndTime)));

    if (m_pTimeStart->label())
	free(const_cast < char *>(m_pTimeStart->label()));


    m_pTimeStart->label(strdup(_FormatTime(&m_StartTime)));

    m_pTimeHour->select(HR_12(p->tm_hour));
    m_pTimeMinute->select((p->tm_min / 5) + 1);

    m_pTimeAM->redraw();
    m_pTimePM->redraw();
    m_pTimeStart->redraw();
    m_pTimeNone->redraw();
    m_pTimeEnd->redraw();
    m_pTimeMinute->redraw();
    m_pTimeHour->redraw();
}

void
NxSchedule::MakeTimeWindow()
{

    //timeWindow = new NxPimPopWindow("Choose Time",NxApp::Instance()->getGlobalColor(APP_FG),5,5,W_W-10,235);

    timeWindow = new NxPimWindow(W_X, W_Y, W_W, W_H);

    add_window((Fl_Window *) timeWindow->GetWindowPtr());

    m_nTimeState = TIME_WINDOW_START;

    // for start time input
    {
	NxOutput *o = new NxOutput(20, 35 - 5, 0, 0, "Start Time:");
	//    o->labelfont(FL_TIMES_BOLD);
	o->align(FL_ALIGN_RIGHT);
	o->movable(false);
	timeWindow->add(o);
    }

    {
	NxButton *o = new NxButton(20, 50 - 5, 60, 20);
	o->box(FL_THIN_UP_BOX);
	o->callback(timeStart_callback, this);
	m_pTimeStart = o;
	o->movable(false);
	timeWindow->add(o);
	//    m_pDetails_TimeBox = o;
    }

    // for end time input
    {
	NxOutput *o = new NxOutput(20, 85 - 5, 0, 0, "End Time:");
	timeWindow->add(o);
	o->align(FL_ALIGN_RIGHT);
	o->movable(false);
    }

    {
	NxButton *o = new NxButton(20, 100 - 5, 60, 20);
	o->box(FL_THIN_UP_BOX);
	o->callback(timeEnd_callback, this);
	m_pTimeEnd = o;
	o->movable(false);
	timeWindow->add(o);
	//    m_pDetails_TimeBox = o;
    }


    {
	NxButton *o = new NxButton(20, 135 - 5, 60, 20);
	o->box(FL_THIN_UP_BOX);
	o->label(" No Time ");
	o->callback(timeNone_callback, this);
	m_pTimeNone = o;
	o->movable(false);
	timeWindow->add(o);
    }

    {
	NxHoldBrowser *o = new NxHoldBrowser(145, 32, 30, 157);
	o->add("1");
	o->add("2");
	o->add("3");
	o->add("4");
	o->add("5");
	o->add("6");
	o->add("7");

	o->add("8");
	o->add("9");
	o->add("10");
	o->add("11");
	o->add("12");

	o->callback(timeHour_callback, this);
	o->has_scrollbar(0);

	m_pTimeHour = o;
	timeWindow->add(o);
    }

    {
	NxHoldBrowser *o = new NxHoldBrowser(180, 32, 30, 157);
	o->add("0");
	o->add("5");
	o->add("10");
	o->add("15");
	o->add("20");
	o->add("25");
	o->add("30");

	o->add("35");
	o->add("40");
	o->add("45");
	o->add("50");
	o->add("55");

	o->has_scrollbar(0);
	o->callback(timeHour_callback, this);

	m_pTimeMinute = o;
	timeWindow->add(o);
    }

    {
	NxButton *o = new NxButton(100, 40 - 5, 20, 20);
	o->label("AM");
	o->box(FL_FLAT_BOX);
	o->callback(timeAM_callback, this);
	m_pTimeAM = o;
	o->movable(false);
	timeWindow->add(o);
    }
    {
	NxButton *o = new NxButton(100, 60 - 5, 20, 20);
	o->label("PM");
	o->box(FL_FLAT_BOX);
	o->callback(timePM_callback, this);
	m_pTimePM = o;
	o->movable(false);
	timeWindow->add(o);
    }


    {
	NxButton *o = new NxButton(POP_BUTTON_X, POP_BUTTON_Y(timeWindow),
				   BUTTON_WIDTH, BUTTON_HEIGHT, "Done");
	o->callback(doneTime_callback, this);
	timeWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o = new NxButton(POP_BUTTON_X + BUTTON_WIDTH + 5,
				   POP_BUTTON_Y(timeWindow),
				   BUTTON_WIDTH, BUTTON_HEIGHT, "Cancel");
	o->callback(cancelTime_callback, this);
	timeWindow->add((Fl_Widget *) o);
    }

    timeWindow->GetWindowPtr()->end();
}
