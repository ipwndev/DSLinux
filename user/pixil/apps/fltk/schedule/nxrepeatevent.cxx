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


#include "nxschedule.h"

void
NxSchedule::MakeRepeatEventWindow()
{

    repeatEventWindow =
	new NxPimPopWindow("Repeating Event",
			   NxApp::Instance()->getGlobalColor(APP_FG), 5, 30,
			   W_W - 10, 180);

    add_window((Fl_Window *) repeatEventWindow->GetWindowPtr());

    {
	NxBox *o = new NxBox(20, 5, W_W - 40, 90,
			     "Apply the changes to:\n-the Current record,\n-this & all Future records,\n-All occurences?");
	o->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	repeatEventWindow->add(o);
    }
    {
	NxButton *o = new NxButton(POP_BUTTON_X,
				   POP_BUTTON_Y(repeatEventWindow) -
				   (2 * BUTTON_HEIGHT) - 10,
				   BUTTON_WIDTH, BUTTON_HEIGHT, "Current");
	o->callback(changeCurrent_callback, this);
	repeatEventWindow->add(o);
    }
    {
	NxButton *o = new NxButton(POP_BUTTON_X,
				   POP_BUTTON_Y(repeatEventWindow) -
				   BUTTON_HEIGHT - 5,
				   BUTTON_WIDTH, BUTTON_HEIGHT, "Future");
	o->callback(changeFuture_callback, this);
	repeatEventWindow->add(o);
    }
    {
	NxButton *o =
	    new NxButton(POP_BUTTON_X, POP_BUTTON_Y(repeatEventWindow),
			 BUTTON_WIDTH,
			 BUTTON_HEIGHT, "All");
	o->callback(changeAll_callback, this);
	repeatEventWindow->add(o);
    }
    {
	NxButton *o =
	    new NxButton(repeatEventWindow->GetWindowPtr()->w() -
			 POP_BUTTON_X - BUTTON_WIDTH,
			 POP_BUTTON_Y(repeatEventWindow), BUTTON_WIDTH,
			 BUTTON_HEIGHT, "Cancel");
	o->callback(changeCancel_callback, this);
	repeatEventWindow->add(o);
    }

    repeatEventWindow->GetWindowPtr()->end();
}

void
NxSchedule::changeCurrent_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    tm *date;
    int year;
    int mon;
    int m_day;
    time_t picked_date = pThis->pCalendar->GetPickedDate();
    time_t today_date = pThis->m_CurrentDay;
    time_t start_time;
    time_t end_time;

    if (0 == picked_date)
	picked_date = pThis->m_CurrentDay;

    // set year and day     
    date = localtime(&today_date);
    year = date->tm_year;
    mon = date->tm_mon;
    m_day = date->tm_mday;

    // set the min and sec for the startTime
    date = localtime(&(pThis->m_pCurrentItem->startTime));
    date->tm_year = year;
    date->tm_mon = mon;
    date->tm_mday = m_day;

    start_time = mktime(date);

    // set the min and sec fot the endTime
    date = localtime(&(pThis->m_pCurrentItem->endTime));
    date->tm_year = year;
    date->tm_mon = mon;
    date->tm_mday = m_day;

    end_time = mktime(date);

    pThis->m_pCurrentItem->exception = (REPEAT_EXCEPTION | REPEAT_DELETED);
    pThis->m_pCurrentItem->recnoPtr = pThis->m_pCurrentItem->recno;
    pThis->m_pCurrentItem->startTime = start_time;
    pThis->m_pCurrentItem->endTime = end_time;
    pThis->m_pCurrentItem->repeatFlag_1 = REPEAT_NONE;

    pThis->write_note(pThis->m_pCurrentItem);

    if (!(DELETE_FLAG & g_EditFlag)) {

	date = localtime(&picked_date);
	year = date->tm_year;
	mon = date->tm_mon;
	m_day = date->tm_mday;

	// set the min and sec for the startTime
	date = localtime(&(pThis->m_pCurrentItem->startTime));
	date->tm_year = year;
	date->tm_mon = mon;
	date->tm_mday = m_day;

	start_time = mktime(date);

	// set the min and sec fot the endTime
	date = localtime(&(pThis->m_pCurrentItem->endTime));
	date->tm_year = year;
	date->tm_mon = mon;
	date->tm_mday = m_day;

	end_time = mktime(date);

	pThis->m_pCurrentItem->startTime = start_time;
	pThis->m_pCurrentItem->endTime = end_time;
	pThis->m_pCurrentItem->exception = 0;
	pThis->m_pCurrentItem->recnoPtr = 0;

	pThis->write_note(pThis->m_pCurrentItem);
    }

    pThis->show_window(dayWindow->GetWindowPtr());
    pThis->UpdateDateDisplay();

    g_EditFlag = 0;

    pThis->pCalendar->SetPickedDate(0);

}

void
NxSchedule::changeFuture_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    int this_rec[1];
    int rec_array[255];
    char c_recno[8];
    NxTodo *n;
    int idx = 0;
    time_t picked_date = pThis->pCalendar->GetPickedDate();
    time_t old_start = pThis->m_pCurrentItem->startTime;
    time_t old_end = pThis->m_pCurrentItem->endTime;
    time_t today_time = pThis->m_CurrentDay;
    time_t end_on = 0;
    time_t new_time = 0;
    unsigned long days = 0;
    float diff = 0;
    tm *tt;
    int year = 0;
    int mon = 0;
    int m_day = 0;

    this_rec[0] = -1;

    //delete dummy record that have start time bigger than this day
    for (idx = 0; idx < 255; idx++)
	rec_array[idx] = -1;

    pThis->db->Select(SCHEDULE, c_recno, 12, rec_array, 255);
    for (idx = 0; idx < 255; idx++) {
	if (-1 != rec_array[idx]) {
	    NxTodo *note = new NxTodo;
	    pThis->ExtractRecord(note, rec_array[idx]);
	    if (note->startTime > pThis->m_CurrentDay) {
		delete_note(note);
	    }
	    delete note;
	}
    }

    n = new NxTodo;

    end_on = pThis->m_pCurrentItem->repeatFlag_3;
    pThis->m_pCurrentItem->repeatFlag_3 = pThis->m_CurrentDay;

    sprintf(c_recno, "%d", pThis->m_pCurrentItem->recno);
    pThis->db->Select(SCHEDULE, c_recno, 0, this_rec, 1);

    // put an end date on the orig record
    if (-1 != this_rec[0]) {
	pThis->ExtractRecord(n, this_rec[0]);
	pThis->m_pCurrentItem->startTime = n->startTime;
	pThis->m_pCurrentItem->endTime = n->endTime;
	pThis->edit_note(pThis->m_pCurrentItem, this_rec[0]);
    }

    if (!(DELETE_FLAG & g_EditFlag)) {

	pThis->ExtractRecord(n, this_rec[0]);

	if (0 != end_on) {

	    diff = difftime(end_on, n->startTime);

	    if (0 > diff)
		diff = diff * -1;
	    if (0 == diff)
		days = 0;
	    else
		days = (long) (diff / 86400);

	    tt = localtime(&old_start);
	    tt->tm_mday += (days + 1);

	    new_time = mktime(tt);

	    n->repeatFlag_3 = new_time;

	} else
	    n->repeatFlag_3 = 0;

	if (CHANGED_DATE_FLAG & g_EditFlag) {
	    // need to set the new date
	    tt = localtime(&picked_date);
	    year = tt->tm_year;
	    mon = tt->tm_mon;
	    m_day = tt->tm_mday;

	    tt = localtime(&old_start);
	    tt->tm_year = year;
	    tt->tm_mon = mon;
	    tt->tm_mday = m_day;

	    old_start = mktime(tt);
	    today_time = old_start;

	    tt = localtime(&old_end);
	    tt->tm_year = year;
	    tt->tm_mon = mon;
	    tt->tm_mday = m_day;

	    old_end = mktime(tt);
	}

	if (CHANGED_TIME_FLAG & g_EditFlag) {

	    // do this becasue start_time may have new date 
	    // so set the new min and hour
	    tt = localtime(&today_time);
	    year = tt->tm_year;
	    mon = tt->tm_mon;
	    m_day = tt->tm_mday + 1;

	    tt = localtime(&old_start);
	    tt->tm_year = year;
	    tt->tm_mon = mon;
	    tt->tm_mday = m_day;

	    old_start = mktime(tt);

	    tt = localtime(&old_end);
	    tt->tm_year = year;
	    tt->tm_mon = mon;
	    tt->tm_mday = m_day;

	    old_end = mktime(tt);

	}

	if (CHANGED_ALARM & g_EditFlag) {
	    // set the new alarm int and flags
	    n->alarmInt = pThis->m_pCurrentItem->alarmInt;
	    n->alarmFlags = pThis->m_pCurrentItem->alarmFlags;
	}

	n->startTime = old_start;
	n->endTime = old_end;
	pThis->write_note(n);
    }

    delete n;

    pThis->show_window(dayWindow->GetWindowPtr());
    pThis->UpdateDateDisplay();
    pThis->pCalendar->SetPickedDate(0);
    g_EditFlag = 0;
}

void
NxSchedule::changeAll_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    tm *date;
    time_t picked_date = pThis->pCalendar->GetPickedDate();
    time_t start_time = pThis->m_pCurrentItem->startTime;
    time_t end_time = pThis->m_pCurrentItem->endTime;

    if (0 == picked_date)
	picked_date = pThis->m_CurrentDay;

    if (DELETE_FLAG & g_EditFlag) {
	pThis->repeatEventWindow->GetWindowPtr()->hide();
	pThis->show_window(deleteWindow->GetWindowPtr(),
			   DEACTIVATE, dayWindow->GetWindowPtr());
    } else {			// either CHANGED_DATE_FLAG or CHANGED_TIME_FLAG or CHANGED_ALARM was set
	int this_rec[1];
	int rec_array[255];
	int idx = 0;
	char c_recno[8];
	int year = 0;
	int mon = 0;
	int m_day = 0;
	time_t end_on = 0;
	float diff = 0;
	long days = 0;
	time_t new_date;
	tm *tt;

	for (idx = 0; idx < 255; idx++)
	    rec_array[idx] = -1;

	sprintf(c_recno, "%d", pThis->m_pCurrentItem->recno);

	if (CHANGED_DATE_FLAG & g_EditFlag) {
	    // remove all dummy entries for this record
	    pThis->db->Select(SCHEDULE, c_recno, 12, rec_array, 255);
	    for (idx = 0; idx < 255; idx++) {
		if (-1 != rec_array[idx]) {
		    pThis->db->DeleteRec(SCHEDULE, rec_array[idx]);
		}
	    }
	    // reset the year and day for the start and end time
	    date = localtime(&picked_date);
	    year = date->tm_year;
	    mon = date->tm_mon;
	    m_day = date->tm_mday;

	    // preserve the hour and min for the startTime
	    date = localtime(&(pThis->m_pCurrentItem->startTime));
	    date->tm_year = year;
	    date->tm_mon = mon;
	    date->tm_mday = m_day;

	    start_time = mktime(date);

	    // preserve the hour and min for the endTime
	    date = localtime(&(pThis->m_pCurrentItem->endTime));
	    date->tm_year = year;
	    date->tm_mon = mon;
	    date->tm_mday = m_day;

	    end_time = mktime(date);

	    end_on = pThis->m_pCurrentItem->repeatFlag_3;
	    if (0 != end_on) {
		NxTodo *n = new NxTodo;

		this_rec[0] = -1;
		pThis->db->Select(SCHEDULE, c_recno, 0, this_rec, 1);
		if (-1 != this_rec[0]) {
		    pThis->ExtractRecord(n, this_rec[0]);
		}

		diff = difftime(end_on, n->startTime);
		if (0 > diff)
		    diff = diff * -1;
		if (0 == diff) {
		    days = 0;
		} else {
		    days = (long) (diff / 86400);
		}

		tt = localtime(&start_time);
		tt->tm_mday += (days + 1);

		new_date = mktime(tt);

		pThis->m_pCurrentItem->repeatFlag_3 = new_date;

		delete n;
	    }

	}

	if (CHANGED_TIME_FLAG & g_EditFlag) {
	    // need to change all dummy entries for this record
	    pThis->db->Select(SCHEDULE, c_recno, 12, rec_array, 255);
	    for (idx = 0; idx < 255; idx++) {
		// this should only be done when time changed only.
		// if date was changed these record should have been deleted
		if (-1 != rec_array[idx]) {
		    NxTodo *n = new NxTodo;
		    pThis->ExtractRecord(n, rec_array[idx]);

		    date = localtime(&n->startTime);
		    year = date->tm_year;
		    mon = date->tm_mon;
		    m_day = date->tm_mday;

		    date = localtime(&pThis->m_pCurrentItem->startTime);
		    date->tm_year = year;
		    date->tm_mon = mon;
		    date->tm_mday = m_day;

		    n->startTime = mktime(date);

		    date = localtime(&pThis->m_pCurrentItem->endTime);
		    date->tm_year = year;
		    date->tm_mon = mon;
		    date->tm_mday = m_day;

		    n->endTime = mktime(date);

		    pThis->edit_note(n, rec_array[idx]);
		    delete n;
		}
	    }
	}

	pThis->m_pCurrentItem->endTime = end_time;
	pThis->m_pCurrentItem->startTime = start_time;

	this_rec[0] = -1;
	pThis->db->Select(SCHEDULE, c_recno, 0, this_rec, 1);
	if (-1 != this_rec[0]) {
	    pThis->edit_note(pThis->m_pCurrentItem, this_rec[0]);
	}
	pThis->show_window(dayWindow->GetWindowPtr());
	pThis->UpdateDateDisplay();
    }

    pThis->pCalendar->SetPickedDate(0);
    g_EditFlag = 0;

}

void
NxSchedule::changeCancel_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->show_window(dayWindow->GetWindowPtr());
    pThis->UpdateDateDisplay();
    pThis->pCalendar->SetPickedDate(0);
    g_EditFlag = 0;
}
