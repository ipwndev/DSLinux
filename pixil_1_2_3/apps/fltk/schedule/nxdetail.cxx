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


#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <pixil_config.h>
#include "nxschedule.h"

/* Please, get rid of this ugly thing */

#if defined(CONFIG_APP_ALARM) && defined(CONFIG_COLOSSEUM)
#define DO_ALARM 1
#endif

#ifdef DEBUG
#define DPRINT(str, args...) printf("DEBUG: " str, ## args)
#else
#define DPRINT(args...)
#endif
extern Fl_Menu_Item schedMenuItems[];

#ifdef DO_ALARM
Fl_Menu_Item alarmMenuItems[] = {
    {"Minutes", 0, NxSchedule::chooseAlarmMin_callback},
    {"Hours", 0, NxSchedule::chooseAlarmHour_callback},
    {"Days", 0, NxSchedule::chooseAlarmDay_callback},
    {0}
};
#endif

static char *
_FormatTimeRange(NxTodo * n)
{
    static char buf[30];
    tm startTime;
    tm endTime;

    memcpy(&startTime, localtime(&n->startTime), sizeof(startTime));
    memcpy(&endTime, localtime(&n->endTime), sizeof(endTime));

    startTime.tm_sec = 0;
    endTime.tm_sec = 0;

    sprintf(buf, "%d:%02d%s - %d:%02d%s",
	    HR_12(startTime.tm_hour),
	    startTime.tm_min,
	    AM_PM(startTime.tm_hour),
	    HR_12(endTime.tm_hour), endTime.tm_min, AM_PM(endTime.tm_hour));

    return buf;

}

#ifdef DO_ALARM
void
NxSchedule::chooseAlarmMin_callback(Fl_Widget * w, void *l)
{
    NxCheckButton *p_Button = (NxCheckButton *) w;
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());

    p_Button->label("Minutes");
    p_Button->hide();
    p_Button->show();

    pThis->SetAlarmInt(ALARM_MIN);
}

void
NxSchedule::chooseAlarmHour_callback(Fl_Widget * w, void *l)
{
    NxCheckButton *p_Button = (NxCheckButton *) w;
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());

    p_Button->label("Hours");
    p_Button->hide();
    p_Button->show();

    pThis->SetAlarmInt(ALARM_HOUR);
}

void
NxSchedule::chooseAlarmDay_callback(Fl_Widget * w, void *l)
{
    NxCheckButton *p_Button = (NxCheckButton *) w;
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());

    p_Button->label("Days");
    p_Button->hide();
    p_Button->show();

    pThis->SetAlarmInt(ALARM_DAY);
}

void
NxSchedule::SetAlarmInt(int interval)
{
    m_pCurrentItem->alarmFlags = interval;
}
#endif

void
NxSchedule::detailTimePicked_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    time_t start_time;
    time_t end_time;
    tm *time;
    int start_hour1;
    int start_min1;
    int start_hour2;
    int start_min2;
    int end_hour1;
    int end_min1;
    int end_hour2;
    int end_min2;
    NxTodo *note;
    int rec_array[1];
    char c_recno[16];

    pThis->timeWindow->GetWindowPtr()->hide();

    NxApp::Instance()->show_window(pThis->detailsWindow->GetWindowPtr(),
				   DEACTIVATE,
				   pThis->dayWindow->GetWindowPtr());

    if (w) {
	if (!(CHANGED_NEW & g_EditFlag)) {
	    note = new NxTodo;
	    rec_array[0] = -1;
	    sprintf(c_recno, "%d", pThis->m_pCurrentItem->recno);

	    pThis->GetTimes(&start_time, &end_time);

	    time = localtime(&start_time);
	    start_hour1 = time->tm_hour;
	    start_min1 = time->tm_min;

	    time = localtime(&end_time);
	    end_hour1 = time->tm_hour;
	    end_min1 = time->tm_min;

	    pThis->db->Select(SCHEDULE, c_recno, 0, rec_array, 1);
	    pThis->ExtractRecord(note, rec_array[0]);

	    time = localtime(&note->startTime);
	    start_hour2 = time->tm_hour;
	    start_min2 = time->tm_min;

	    time = localtime(&note->endTime);
	    end_hour2 = time->tm_hour;
	    end_min2 = time->tm_min;

#ifdef DEBUG
	    printf("start_hour1 [%d] start_hour2 [%d]\n", start_hour1,
		   start_hour2);
	    printf("start_min1 [%d] start_min2 [%d]\n", start_min1,
		   start_min2);
	    printf("end_hour1 [%d] end_hour2[%d]\n", end_hour1, end_hour2);
	    printf("end_min1 [%d] end_min2 [%d]", end_min1, end_min2);
#endif

	    if (start_hour1 != start_hour2 || start_min1 != start_min2 ||
		end_hour1 != end_hour2 || end_min1 != end_min2 &&
		!(REPEAT_NONE & pThis->m_pCurrentItem->repeatFlag_1)) {
		g_EditFlag |= CHANGED_TIME_FLAG;
	    } else {
		g_EditFlag = g_EditFlag & ~CHANGED_TIME_FLAG;
	    }
	    delete note;
	}
	pThis->GetTimes(&pThis->m_pCurrentItem->startTime,
			&pThis->m_pCurrentItem->endTime);
	// ok .. we just got a callback from our time picker routines
	// update the 
	pThis->FillDetailForm(pThis->m_pCurrentItem, DESC_KEEP);
    }
}

void
NxSchedule::detailDatePicked_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->dateWindow->GetWindowPtr()->hide();

    NxApp::Instance()->show_window(pThis->detailsWindow->GetWindowPtr(),
				   DEACTIVATE,
				   pThis->dayWindow->GetWindowPtr());


    time_t t = pCalendar->GetPickedDate();
    if (t) {

	tm date;
	tm startTime;
	tm endTime;
	tm today;
	time_t start_time;
	time_t end_time;

	memcpy(&date, localtime(&t), sizeof(date));
	memcpy(&startTime, localtime(&pThis->m_pCurrentItem->startTime),
	       sizeof(startTime));
	memcpy(&endTime, localtime(&pThis->m_pCurrentItem->endTime),
	       sizeof(endTime));
	memcpy(&today, localtime(&pThis->m_CurrentDay), sizeof(today));

	startTime.tm_sec = 0;
	startTime.tm_mon = date.tm_mon;
	startTime.tm_year = date.tm_year;
	startTime.tm_mday = date.tm_mday;

	endTime.tm_sec = 0;
	endTime.tm_mon = date.tm_mon;
	endTime.tm_year = date.tm_year;
	endTime.tm_mday = date.tm_mday;

	start_time = mktime(&startTime);
	end_time = mktime(&endTime);

	if ((date.tm_year != today.tm_year
	     || date.tm_mon != today.tm_mon
	     || date.tm_mday != today.tm_mday) &&
	    !(REPEAT_NONE & pThis->m_pCurrentItem->repeatFlag_1))
	    g_EditFlag |= CHANGED_DATE_FLAG;
	else
	    g_EditFlag = g_EditFlag & ~CHANGED_DATE_FLAG;
#ifdef DEBUG
	printf("Date callback g_EditFlag [%d]\n", g_EditFlag);
#endif

	pThis->m_pCurrentItem->fakeTime = start_time;
	pThis->m_pCurrentItem->startTime = start_time;
	pThis->m_pCurrentItem->endTime = end_time;

	// ok .. we just got a callback from our date picker routines
	// update the detail window
	pThis->FillDetailForm(pThis->m_pCurrentItem, DESC_KEEP);
    }
}

// Show the date selection window
void
NxSchedule::detailDate_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->set_date_picker(m_pCalendar);
    if (!(REPEAT_NONE & pThis->m_pCurrentItem->repeatFlag_1))
	pCalendar->SetPickedDate(pThis->m_pCurrentItem->fakeTime);
    else
	pCalendar->SetPickedDate(pThis->m_pCurrentItem->startTime);

    pThis->detailsWindow->GetWindowPtr()->deactivate();
    NxApp::Instance()->show_window(dateWindow->GetWindowPtr(),
				   DEACTIVATE, dayWindow->GetWindowPtr());

    pCalendar->DateCallback(detailDatePicked_callback);
}

#ifdef DO_ALARM

// Set the alarm value 
void
NxSchedule::alarmIntChanged_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    int interval = atoi(pThis->m_pDetailsAlarmInput->value());

    pThis->m_pCurrentItem->alarmInt = interval;

}

// Toggle the alarm button show the alarm input and menu
void
NxSchedule::alarmToggle_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    bool val = pThis->m_pDetailsAlarmCheck->value();

    if (true == val) {
	pThis->m_pCurrentItem->alarmFlags = ALARM_MIN;
	pThis->m_pCurrentItem->alarmInt = 5;
	pThis->showAlarmUi();
    } else {
	pThis->m_pCurrentItem->alarmInt = NO_ALARM;
	pThis->hideAlarmUi();
    }
}
#endif

// Show the repeat selection window
void
NxSchedule::detailRepeat_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->getRepeatData();
    pThis->detailsWindow->GetWindowPtr()->deactivate();
    NxApp::Instance()->show_window(repeatWindow->GetWindowPtr(),
				   DEACTIVATE, dayWindow->GetWindowPtr());
}

// Show the time selection window
void
NxSchedule::detailTime_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->SetTimes(pThis->m_pCurrentItem->startTime,
		    pThis->m_pCurrentItem->endTime);

    pThis->detailsWindow->GetWindowPtr()->deactivate();
    NxApp::Instance()->show_window(timeWindow->GetWindowPtr(),
				   DEACTIVATE, dayWindow->GetWindowPtr());

    pThis->time_callback(detailTimePicked_callback);
}

void
NxSchedule::cancelEdit_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->pCalendar->SetPickedDate(0);
    pThis->show_window(dayWindow->GetWindowPtr());
    pThis->UpdateDateDisplay();
}

void
NxSchedule::deleteEdit_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    if ((REPEAT_NONE) & pThis->m_pCurrentItem->repeatFlag_1 ||
	pThis->m_pCurrentItem->repeatFlag_1 == 0)
	NxApp::Instance()->show_window(deleteWindow->GetWindowPtr(),
				       DEACTIVATE,
				       detailsWindow->GetWindowPtr());
    else {
	pThis->g_EditFlag = DELETE_FLAG;
	NxApp::Instance()->show_window(repeatEventWindow->GetWindowPtr(),
				       DEACTIVATE,
				       detailsWindow->GetWindowPtr());
    }
}

void
NxSchedule::doneEdit_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    strcpy(pThis->m_pCurrentItem->szDescription,
	   pThis->m_pDetails_DescBox->value());

    if (!(g_EditFlag & CHANGED_NEW)) {
	NxTodo *n = new NxTodo;
	int recno = pThis->m_pCurrentItem->recno;
	int rec_array[1];
	char c_recno[16];

	rec_array[0] = -1;
	sprintf(c_recno, "%d", recno);
	pThis->db->Select(SCHEDULE, c_recno, 0, rec_array, 1);
	pThis->ExtractRecord(n, rec_array[0]);

#ifdef DO_ALARM
	if (0 == strcmp("", pThis->m_pDetailsAlarmInput->value()) ||
	    0 == strcmp(" ", pThis->m_pDetailsAlarmInput->value())) {
	    pThis->m_pCurrentItem->alarmInt = 0;
	}
	if (n->alarmFlags != pThis->m_pCurrentItem->alarmFlags ||
	    n->alarmInt != pThis->m_pCurrentItem->alarmInt) {
	    // need to delete old alarm and set new one
	    if ((REPEAT_NONE == pThis->m_pCurrentItem->repeatFlag_1)) {
		DPRINT("changed alarm\n");
		DPRINT("send message to alarmd\n");

		pThis->DeleteAlarm(n);
		pThis->SetAlarm(pThis->m_pCurrentItem, 0);
	    } else
		g_EditFlag |= CHANGED_ALARM;
	}
#endif
	delete n;
	n = 0;
    } else {
#ifdef DO_ALARM
	if (pThis->m_pCurrentItem->alarmInt != NO_ALARM) {
	    DPRINT("setting alarm for new item\n");
	    pThis->SetAlarm(pThis->m_pCurrentItem, 0);
	}
#endif
    }

    if (((CHANGED_DATE_FLAG & g_EditFlag)
	 || (CHANGED_TIME_FLAG & g_EditFlag)
#ifdef DO_ALARM
	 || (CHANGED_ALARM & g_EditFlag)
#endif
	) &&
	(REPEAT_NONE != pThis->m_pCurrentItem->repeatFlag_1) &&
	!(CHANGED_NEW & g_EditFlag)) {
	NxApp::Instance()->show_window(repeatEventWindow->GetWindowPtr(),
				       DEACTIVATE,
				       detailsWindow->GetWindowPtr());
    } else {
	save(pThis->m_pCurrentItem);
	pThis->show_window(dayWindow->GetWindowPtr());
    }
    g_EditFlag = g_EditFlag & ~CHANGED_NEW;
    pThis->UpdateDateDisplay();
}

static char *
_FormatDate(NxTodo * n)
{
    static char buf[30];
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());
    time_t date = pThis->GetCurrentDay();
    int year = 0;
    int mon = 0;
    int day = 0;
    tm *tt = NULL;
    int flag = pThis->GetEditFlag();

    tt = localtime(&date);
    day = tt->tm_mday;
    mon = tt->tm_mon;
    year = tt->tm_year;

    if ((REPEAT_NONE & n->repeatFlag_1)) {
	//strftime(buf,29,"%b %d, %Y",localtime(&n->startTime));
	pThis->GetDateString(buf, localtime(&n->startTime), sizeof(buf),
			     LONG_YEAR);
	return buf;
    }

    if (flag & CHANGED_DATE_FLAG) {
	date = pThis->GetpCalendar()->GetPickedDate();
	if (date == 0)
	    date = pThis->GetCurrentDay();
	//strftime(buf,29,"%b %d, %Y",localtime(&date));
	pThis->GetDateString(buf, localtime(&date), sizeof(buf), LONG_YEAR);
    } else {
	//strftime(buf,29,"%b %d, %Y",localtime(&date));
	pThis->GetDateString(buf, localtime(&date), sizeof(buf), LONG_YEAR);
    }
    return buf;
}

// This function will fill the detail form as well as set
// the "Current" schedule item to be edited.

void
NxSchedule::FillDetailForm(NxTodo * n, int flags)
{
#ifdef DO_ALARM
    char buf[16];
#endif

    if (!n)
	return;

    m_pCurrentItem = n;

    m_pDetails_TimeBox->label(_FormatTimeRange(n));
    m_pDetails_TimeBox->redraw();
    m_pDetails_DateBox->label(_FormatDate(n));
    m_pDetails_DateBox->redraw();

    switch (n->repeatFlag_1) {
    case REPEAT_YEARLY:
	m_pDetails_RepeatBox->label("Yearly");
	break;
    case REPEAT_MONTHLY:
	m_pDetails_RepeatBox->label("Monthly");
	break;
    case REPEAT_WEEKLY:
	m_pDetails_RepeatBox->label("Weekly");
	break;
    case REPEAT_DAILY:
	m_pDetails_RepeatBox->label("Daily");
	break;
    default:
    case REPEAT_NONE:
	m_pDetails_RepeatBox->label("None");
	break;
    }

    if (DESC_NEW & flags)
	m_pDetails_DescBox->value("");
    else if (DESC_KEEP & flags);
    else
	m_pDetails_DescBox->value(n->szDescription);

#ifdef DO_ALARM
    sprintf(buf, "%d", n->alarmInt);
    m_pDetailsAlarmInput->value(buf);

    if (n->alarmInt != NO_ALARM)
	showAlarmUi();
    else
	hideAlarmUi();
#endif
}

#ifdef DO_ALARM
void
NxSchedule::hideAlarmUi()
{
    m_pDetailsAlarmCheck->value(0);
    m_pDetailsAlarmInput->hide();
    m_pDetailsAlarmInt->hide();
}

void
NxSchedule::showAlarmUi()
{
    static char buf[3];

    m_pDetailsAlarmCheck->value(1);

    switch (m_pCurrentItem->alarmFlags) {
    case ALARM_MIN:
	m_pDetailsAlarmInt->label("Minutes");
	break;
    case ALARM_HOUR:
	m_pDetailsAlarmInt->label("Hours");
	break;
    case ALARM_DAY:
	m_pDetailsAlarmInt->label("Days");
	break;
    default:
	m_pDetailsAlarmInt->label("Minutes");
	break;
    }

    sprintf(buf, "%d", m_pCurrentItem->alarmInt);
    m_pDetailsAlarmInput->value(buf);
    m_pDetailsAlarmInput->hide();
    m_pDetailsAlarmInput->show();
    m_pDetailsAlarmInt->show();
}
#endif

void
NxSchedule::MakeDetailsWindow()
{


    detailsWindow =
	new NxPimPopWindow("Event Details",
			   NxApp::Instance()->getGlobalColor(APP_FG), 5, 5,
			   W_W - 10, 205);

    add_window((Fl_Window *) detailsWindow->GetWindowPtr());

    // for the time input
    {
	NxOutput *o = new NxOutput(20, 35, 0, 0, "Time:");
	NxApp::Instance()->def_font(o);
	detailsWindow->add(o);
	o->align(FL_ALIGN_RIGHT);
    }

    {
	NxButton *o = new NxButton(55, 27, 120, BUTTON_HEIGHT);
	o->box(FL_THIN_UP_BOX);
	o->label("11:00am - 10:00pm");
	o->callback(detailTime_callback, this);
	detailsWindow->add(o);
	m_pDetails_TimeBox = o;
    }

    // for the date input
    {
	NxOutput *o = new NxOutput(20, 60, 0, 0, "Date:");
	detailsWindow->add(o);
	o->align(FL_ALIGN_RIGHT);
    }

    {
	NxButton *o = new NxButton(58, 52, 110, BUTTON_HEIGHT);
	o->box(FL_THIN_UP_BOX);
	o->label("Aug 10, 2001");
	o->callback(detailDate_callback, this);
	detailsWindow->add(o);
	m_pDetails_DateBox = o;
    }

    // for the repeat
    {
	NxOutput *o = new NxOutput(8, 85, 0, 0, "Repeat:");
	detailsWindow->add(o);
	o->align(FL_ALIGN_RIGHT);
    }
    {
	NxButton *o = new NxButton(58, 77, 70, BUTTON_HEIGHT);
	o->box(FL_THIN_UP_BOX);
	o->label("Monthly");
	detailsWindow->add(o);
	m_pDetails_RepeatBox = o;
	o->callback(detailRepeat_callback, this);
    }
#ifdef DO_ALARM
    {
	NxOutput *o = new NxOutput(14, 110, 0, 0, "Alarm:");
	detailsWindow->add(o);
	o->align(FL_ALIGN_RIGHT);
    }
    {
	NxCheckButton *o = new NxCheckButton(58, 99, "");
	o->callback(alarmToggle_callback, this);
	detailsWindow->add((Fl_Widget *) o);
	m_pDetailsAlarmCheck = o;
    }
    {
	NxIntInput *o = new NxIntInput(70, 99, 30, 20, "");
	detailsWindow->add(o);
	o->maximum_size(2);
	o->callback(alarmIntChanged_callback, this);
	o->when(FL_WHEN_CHANGED);
	m_pDetailsAlarmInput = o;
    }
    {
	NxMenuButton *o = new NxMenuButton(103, 101, 60, BUTTON_HEIGHT);
	o->label("Minutes");
	o->menu(alarmMenuItems);
	detailsWindow->add(o);
	m_pDetailsAlarmInt = o;
    }
#endif
    {
	NxMultilineInput *o = new NxMultilineInput(3, 127,
						   detailsWindow->
						   GetWindowPtr()->w() - 6,
						   50);
	o->maximum_size(99);
	o->align(FL_ALIGN_WRAP);
	detailsWindow->add(o);
	m_pDetails_DescBox = o;
    }

    {
	NxButton *o = new NxButton(POP_BUTTON_X, POP_BUTTON_Y(detailsWindow),
				   BUTTON_WIDTH, BUTTON_HEIGHT, "Ok");
	o->callback(doneEdit_callback, this);
	detailsWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o = new NxButton(POP_BUTTON_X + BUTTON_WIDTH + 2,
				   POP_BUTTON_Y(detailsWindow),
				   BUTTON_WIDTH, BUTTON_HEIGHT, "Cancel");
	o->callback(cancelEdit_callback, this);
	detailsWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o = new NxButton(POP_BUTTON_X + (BUTTON_WIDTH * 2) + 4,
				   POP_BUTTON_Y(detailsWindow),
				   BUTTON_WIDTH, BUTTON_HEIGHT, "Delete");
	o->callback(deleteEdit_callback, this);
	detailsDeleteButton = o;
	detailsWindow->add((Fl_Widget *) o);
    }

    detailsWindow->GetWindowPtr()->end();

}
