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
#include "nxschedule.h"

Fl_Menu_Item repeatMenuItems[] = {
    {"No End Date", 0, NxSchedule::noDate_callback}
    ,
    {"Choose Date...", 0, NxSchedule::chooseDate_callback}
    ,
    {0}
};


void
NxSchedule::MakeRepeatWindow()
{
    //repeatWindow = new NxPimPopWindow("Change Repeat", DEF_FG, 0, 0, W_W, W_H);

    repeatWindow = new NxPimWindow(W_X, W_Y, W_W, W_H);

    add_window((Fl_Window *) repeatWindow->GetWindowPtr());

    {
	NxButton *o = no_repeat =
	    new NxButton(8, 5, 44, BUTTON_HEIGHT, "None");
	o->box(FL_FLAT_BOX);
	o->type(FL_TOGGLE_BUTTON);
	o->value(1);
	o->callback(repeatNoneButton_callback, this);
	o->movable(false);
	repeatWindow->add(o);
    }
    {
	NxButton *o = day_repeat =
	    new NxButton(53, 5, 44, BUTTON_HEIGHT, "Day");
	o->box(FL_FLAT_BOX);
	o->type(FL_TOGGLE_BUTTON);
	o->callback(repeatDayButton_callback, this);
	o->movable(false);
	repeatWindow->add(o);
    }
    {
	NxButton *o = week_repeat =
	    new NxButton(98, 5, 44, BUTTON_HEIGHT, "Week");
	o->box(FL_FLAT_BOX);
	o->type(FL_TOGGLE_BUTTON);
	o->callback(repeatWeekButton_callback, this);
	o->movable(false);
	repeatWindow->add(o);
    }
    {
	NxButton *o = month_repeat =
	    new NxButton(143, 5, 44, BUTTON_HEIGHT, "Month");
	o->box(FL_FLAT_BOX);
	o->type(FL_TOGGLE_BUTTON);
	o->callback(repeatMonthButton_callback, this);
	o->movable(false);
	repeatWindow->add(o);
    }
    {
	NxButton *o = year_repeat =
	    new NxButton(188, 5, 44, BUTTON_HEIGHT, "Year");
	o->box(FL_FLAT_BOX);
	o->type(FL_TOGGLE_BUTTON);
	o->callback(repeatYearButton_callback, this);
	o->movable(false);
	repeatWindow->add(o);
    }
    {
	NxBox *o = repeat_output = new NxBox(BUTTON_X, BUTTON_Y - 70,
					     W_W - (2 * BUTTON_X), 55);
	//NxBox *o = repeat_output = new NxBox(0, 100, W_W, 55);
	o->label("No Repeat.");
	o->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_TOP);
	//      o->movable(false);
	o->box(FL_BORDER_BOX);
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	repeatWindow->add(o);
    }

    {
	NxIntInput *o = every_input = new NxIntInput(53, 30 + BUTTON_HEIGHT,
						     30, 25, "Every:");
	o->maximum_size(2);
	o->align(FL_ALIGN_LEFT);
	o->callback(repeatEveryInput_callback, this);
	o->when(FL_WHEN_CHANGED);
	repeatWindow->add(o);
	o->hide();
    }
    {
	NxBox *o = every_box = new NxBox(85, 33 + BUTTON_HEIGHT,
					 50, 25, "Day(s)");
	repeatWindow->add(o);
	o->hide();
    }
    {
	NxBox *o = list_box =
	    new NxBox(28, 75 + BUTTON_HEIGHT, 20, BUTTON_HEIGHT);
	o->label("End on:");
	repeatWindow->add(o);
	o->hide();
    }
    {
	NxMenuButton *o = end_list = new NxMenuButton(63, 76 + BUTTON_HEIGHT,
						      100, BUTTON_HEIGHT);
	o->label("No End Date");
	o->movable(true);
	o->menu(repeatMenuItems);
	repeatWindow->add(o);
	o->hide();
    }
    {
	NxBox *o = week_box =
	    new NxBox(22, 108 + BUTTON_HEIGHT, 50, BUTTON_HEIGHT);
	o->label("Repeat on:");
	repeatWindow->add(o);
	o->hide();
    }
    {
	char *_d[] = { "S", "M", "T", "W", "T", "F", "S" };
	for (int idx = 0; idx < 7; idx++) {
	    NxButton *o = new NxButton(BUTTON_X + ((idx + 1) * DAY_S),
				       135 + BUTTON_HEIGHT,
				       DAY_S, DAY_S, _d[idx]);
	    m_WeekButtons[idx] = o;
	    o->box(FL_FLAT_BOX);
	    o->callback(weekDay_callback, this);
	    o->align(FL_ALIGN_CENTER);
	    o->type(FL_TOGGLE_BUTTON);
	    o->hide();
	    repeatWindow->add(o);
	}
    }

    {
	NxBox *o = month_box =
	    new NxBox(22, 125 + BUTTON_HEIGHT, 50, BUTTON_HEIGHT);
	o->label("Repeat by:");
	repeatWindow->add(o);
	o->hide();
    }

    {
	NxButton *o = month_day =
	    new NxButton(BUTTON_X + 73, 127 + BUTTON_HEIGHT,
			 44, BUTTON_HEIGHT, "Day");
	o->box(FL_FLAT_BOX);
	o->type(FL_TOGGLE_BUTTON);
	o->callback(monthDayDate_callback, this);
	repeatWindow->add(o);
	o->hide();
    }

    {
	NxButton *o = month_date =
	    new NxButton(BUTTON_X + 118, 127 + BUTTON_HEIGHT,
			 44, BUTTON_HEIGHT, "Date");
	o->box(FL_FLAT_BOX);
	o->type(FL_TOGGLE_BUTTON);
	o->callback(monthDayDate_callback, this);
	repeatWindow->add(o);
	o->hide();
    }
    {
	NxBox *o = message_output = new NxBox(BUTTON_X, 30 + BUTTON_HEIGHT,
					      W_W - (2 * BUTTON_X), 55);
	o->label("Tap one of the above buttons\n to set repeat interval");
	o->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	repeatWindow->add(o);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH - 5, BUTTON_HEIGHT,
			 "Ok");
	o->callback(repeatOk_callback, this);
	repeatWindow->add(o);
    }
    {
	NxButton *o = new NxButton(BUTTON_X + 56, BUTTON_Y, BUTTON_WIDTH - 5,
				   BUTTON_HEIGHT, "Cancel");
	o->callback(repeatCancel_callback, this);
	repeatWindow->add(o);
    }

    repeatWindow->GetWindowPtr()->end();
}

void
NxSchedule::noDate_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());

    pThis->end_list->label("No End Date");
    pThis->repeatDate = 0;
    pThis->end_list->hide();
    pThis->end_list->show();
}

void
NxSchedule::repeatDate_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    static char date_buf[30];
    tm *tt;
    int year;
    int mon;
    int m_day;

    pThis->dateWindow->GetWindowPtr()->hide();
    pThis->show_window(repeatWindow->GetWindowPtr());

    if (pCalendar->GetPickedDate()) {
	tt = localtime(&pThis->m_pCurrentItem->startTime);
	year = tt->tm_year;
	mon = tt->tm_mon;
	m_day = tt->tm_mday;

	time_t picked_date = pCalendar->GetPickedDate();
	tt = localtime(&picked_date);

	if (tt->tm_year < year || tt->tm_mon < mon || tt->tm_mday < m_day) {
	    pThis->end_list->label("No End Date");
	    pThis->end_list->hide();
	    pThis->end_list->show();
	    pThis->repeatDate = 0;
	    return;
	}

	pThis->repeatDate = pCalendar->GetPickedDate();
	pThis->m_pCurrentItem->repeatFlag_3 = pThis->repeatDate;
	strftime(date_buf, 29, "%a %m/%d/%Y",
		 localtime(&(pThis->repeatDate)));
	pThis->end_list->label(date_buf);
	pThis->end_list->hide();
	pThis->end_list->show();
    }
}

void
NxSchedule::chooseDate_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());

    pThis->set_date_picker(m_pCalendar);
    if (pThis->m_pCurrentItem->repeatFlag_3)
	pCalendar->SetPickedDate(pThis->m_pCurrentItem->repeatFlag_3);
    else
	pCalendar->SetPickedDate(pThis->m_pCurrentItem->startTime);
    pThis->show_window(dateWindow->GetWindowPtr(),
		       DEACTIVATE, repeatWindow->GetWindowPtr());
    pCalendar->DateCallback(repeatDate_callback);
}

void
NxSchedule::repeatOk_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->m_pCurrentItem->repeatFlag_1 = pThis->getRepeatValue();

    if (0 == strcmp("", pThis->every_input->value()) ||
	(0 == strcmp("1", pThis->every_input->value())) ||
	(0 == strcmp("0", pThis->every_input->value())) ||
	(0 == strcmp("00", pThis->every_input->value())))
	every_input->value("1");

    if (pThis->no_repeat->value()) {
	pThis->m_pCurrentItem->repeatFlag_2 = 0;
	pThis->m_pCurrentItem->repeatFlag_3 = 0;
    } else {
	pThis->m_pCurrentItem->repeatFlag_2 =
	    strtol(every_input->value(), NULL, 10);
	pThis->m_pCurrentItem->repeatFlag_3 = pThis->repeatDate;
    }

    if (pThis->week_repeat->value())
	pThis->m_pCurrentItem->repeatWkMonFlag = pThis->getWeekValue();
    else if (pThis->month_repeat->value())
	pThis->m_pCurrentItem->repeatWkMonFlag = pThis->getMonthValue();
    else
	pThis->m_pCurrentItem->repeatWkMonFlag = 0;

    pThis->FillDetailForm(pThis->m_pCurrentItem, DESC_KEEP);

    pThis->show_window(dayWindow->GetWindowPtr());
    pThis->show_window(detailsWindow->GetWindowPtr(),
		       DEACTIVATE, dayWindow->GetWindowPtr());
}

void
NxSchedule::repeatCancel_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->show_window(dayWindow->GetWindowPtr());
    pThis->show_window(detailsWindow->GetWindowPtr(),
		       DEACTIVATE, dayWindow->GetWindowPtr());
}

void
NxSchedule::noRepeat_Ui()
{
    list_box->hide();
    end_list->hide();
    every_input->hide();
    every_box->hide();
    for (int idx = 0; idx < 7; idx++)
	m_WeekButtons[idx]->hide();
    week_box->hide();
    month_box->hide();
    month_day->hide();
    month_date->hide();
}

void
NxSchedule::repeatShow_ui()
{
    message_output->hide();
    every_input->show();
    every_box->hide();
    every_box->show();
    end_list->show();
    list_box->show();
    for (int idx = 0; idx < 7; idx++)
	m_WeekButtons[idx]->hide();
    week_box->hide();
    month_box->hide();
    month_day->hide();
    month_date->hide();
}

void
NxSchedule::defaultUi()
{
    every_input->value("1");
    end_list->label("No End Date");
    repeatDate = 0;
}

void
NxSchedule::resetUi(long repeat)
{
    static char val[4];
    static char date[30];

    if (0 >= m_pCurrentItem->repeatFlag_3)
	strcpy(date, "No End Date");
    else
	strftime(date, 29, "%a %m/%d/%Y",
		 localtime(&(m_pCurrentItem->repeatFlag_3)));
    defaultUi();

    switch (repeat) {
    case REPEAT_DAILY:
	if (REPEAT_DAILY == m_pCurrentItem->repeatFlag_1) {
	    sprintf(val, "%d", m_pCurrentItem->repeatFlag_2);
	    every_input->value(val);
	    end_list->label(date);
	}
	break;
    case REPEAT_WEEKLY:
	if (REPEAT_WEEKLY == m_pCurrentItem->repeatFlag_1) {
	    sprintf(val, "%d", m_pCurrentItem->repeatFlag_2);
	    every_input->value(val);
	    end_list->label(date);
	}
	break;
    case REPEAT_MONTHLY:
	if (REPEAT_MONTHLY == m_pCurrentItem->repeatFlag_1) {
	    sprintf(val, "%d", m_pCurrentItem->repeatFlag_2);
	    every_input->value(val);
	    end_list->label(date);
	}
	break;
    case REPEAT_YEARLY:
	if (REPEAT_YEARLY == m_pCurrentItem->repeatFlag_1) {
	    sprintf(val, "%d", m_pCurrentItem->repeatFlag_2);
	    every_input->value(val);
	    end_list->label(date);
	}
	break;
    default:
	defaultUi();
	break;
    }
}

void
NxSchedule::repeatNoneButton_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->noRepeat_Ui();
    pThis->message_output->show();
    pThis->repeat_output->label("No Repeat.");
    pThis->no_repeat->value(1);
    pThis->day_repeat->value(0);
    pThis->week_repeat->value(0);
    pThis->month_repeat->value(0);
    pThis->year_repeat->value(0);
    pThis->every_input->do_callback();
    pThis->repeatDate = 0;
}

void
NxSchedule::repeatDayButton_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->resetUi(REPEAT_DAILY);
    pThis->every_box->label("Day(s)");
    pThis->no_repeat->value(0);
    pThis->day_repeat->value(1);
    pThis->week_repeat->value(0);
    pThis->month_repeat->value(0);
    pThis->year_repeat->value(0);
    pThis->repeatShow_ui();
    pThis->every_input->do_callback();

}

void
NxSchedule::repeatWeekButton_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->resetUi(REPEAT_WEEKLY);
    pThis->every_box->label("Week(s)");
    pThis->no_repeat->value(0);
    pThis->day_repeat->value(0);
    pThis->week_repeat->value(1);
    pThis->month_repeat->value(0);
    pThis->year_repeat->value(0);
    pThis->repeatShow_ui();
    pThis->setWeekValue();
    for (int idx = 0; idx < 7; idx++)
	pThis->m_WeekButtons[idx]->show();
    pThis->week_box->show();
    pThis->every_input->do_callback();
}

void
NxSchedule::repeatMonthButton_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->resetUi(REPEAT_MONTHLY);
    pThis->every_box->label("Month(s)");
    pThis->no_repeat->value(0);
    pThis->day_repeat->value(0);
    pThis->week_repeat->value(0);
    pThis->month_repeat->value(1);
    pThis->year_repeat->value(0);
    pThis->setMonthValue();
    pThis->repeatShow_ui();
    pThis->month_day->show();
    pThis->month_date->show();
    pThis->month_box->show();
    pThis->every_input->do_callback();
}

void
NxSchedule::repeatYearButton_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->resetUi(REPEAT_YEARLY);
    pThis->every_box->label("Year(s)");
    pThis->no_repeat->value(0);
    pThis->day_repeat->value(0);
    pThis->week_repeat->value(0);
    pThis->month_repeat->value(0);
    pThis->year_repeat->value(1);
    pThis->repeatShow_ui();
    pThis->every_input->do_callback();
}

void
NxSchedule::getRepeatData()
{
    NxTodo *n = m_pCurrentItem;
    static char date_buf[30];
    static char repeat_num[4];

    memset(date_buf, 0, sizeof(date_buf));

    if (0 >= n->repeatFlag_3) {
	end_list->label("No End Date");
	repeatDate = 0;
	every_input->value("1");
    } else {
	strftime(date_buf, 29, "%a %m/%d/%Y", localtime(&n->repeatFlag_3));
	repeatDate = n->repeatFlag_3;
	end_list->label(date_buf);
	if (n->repeatFlag_2 > 99)
	    n->repeatFlag_2 = 99;
	if (n->repeatFlag_2 < 0)
	    n->repeatFlag_2 = 0;
	sprintf(repeat_num, "%d", n->repeatFlag_2);
	every_input->value(repeat_num);
    }

    switch (n->repeatFlag_1) {
    case REPEAT_NONE:
	repeatDate = 0;
	end_list->label("No End Date");
	no_repeat->do_callback();
	break;
    case REPEAT_DAILY:
	day_repeat->do_callback();
	break;
    case REPEAT_WEEKLY:
	week_repeat->do_callback();
	break;
    case REPEAT_MONTHLY:
	month_repeat->do_callback();
	break;
    case REPEAT_YEARLY:
	year_repeat->do_callback();
	break;
    default:
	break;
    }

}

void
NxSchedule::setWeekValue()
{
    long val = m_pCurrentItem->repeatWkMonFlag;
    time_t today = m_pCurrentItem->startTime;	//time(0);
    tm *tt = localtime(&today);

    for (int idx = 0; idx < 7; idx++)
	m_WeekButtons[idx]->value(0);

    if (0 == val) {
	m_WeekButtons[tt->tm_wday]->value(1);
	return;
    }

    if (REPEAT_WEEK_SUNDAY & val)
	m_WeekButtons[0]->value(1);
    if (REPEAT_WEEK_MONDAY & val)
	m_WeekButtons[1]->value(1);
    if (REPEAT_WEEK_TUESDAY & val)
	m_WeekButtons[2]->value(1);
    if (REPEAT_WEEK_WEDNESDAY & val)
	m_WeekButtons[3]->value(1);
    if (REPEAT_WEEK_THURSDAY & val)
	m_WeekButtons[4]->value(1);
    if (REPEAT_WEEK_FRIDAY & val)
	m_WeekButtons[5]->value(1);
    if (REPEAT_WEEK_SATURDAY & val)
	m_WeekButtons[6]->value(1);
}

void
NxSchedule::setMonthValue()
{
    long val = m_pCurrentItem->repeatWkMonFlag;

    if (0 == val) {
	month_date->value(1);
	month_day->value(0);
	return;
    }

    if (REPEAT_MONTH_DAY & val) {
	month_date->value(0);
	month_day->value(1);
    } else {
	month_day->value(0);
	month_date->value(1);
    }
}

long
NxSchedule::getWeekValue()
{
    long val = 0;

    for (int idx = 0; idx < 7; idx++) {
	if (m_WeekButtons[idx]->value()) {
	    switch (idx) {
	    case 0:
		val |= REPEAT_WEEK_SUNDAY;
		break;
	    case 1:
		val |= REPEAT_WEEK_MONDAY;
		break;
	    case 2:
		val |= REPEAT_WEEK_TUESDAY;
		break;
	    case 3:
		val |= REPEAT_WEEK_WEDNESDAY;
		break;
	    case 4:
		val |= REPEAT_WEEK_THURSDAY;
		break;
	    case 5:
		val |= REPEAT_WEEK_FRIDAY;
		break;
	    case 6:
		val |= REPEAT_WEEK_SATURDAY;
		break;
	    default:
		break;
	    }
	}
    }
    return val;
}

long
NxSchedule::getMonthValue()
{

    if (month_date->value())
	return REPEAT_MONTH_DATE;
    else
	return REPEAT_MONTH_DAY;
}

long
NxSchedule::getRepeatValue()
{
    if (no_repeat->value())
	return REPEAT_NONE;
    else if (day_repeat->value())
	return REPEAT_DAILY;
    else if (week_repeat->value())
	return REPEAT_WEEKLY;
    else if (month_repeat->value())
	return REPEAT_MONTHLY;
    else if (year_repeat->value())
	return REPEAT_YEARLY;
    else
	return REPEAT_NONE;
}

void
NxSchedule::wordValue(int val, char *th_val)
{
    char val_buf[8];
    int len;
    char num_char;

    sprintf(val_buf, "%d", val);
    len = strlen(val_buf);

    if (11 <= val && 13 >= val) {
	sprintf(th_val, "%dth", val);
	return;
    }

    num_char = val_buf[len - 1];

    switch (num_char) {
    case '1':
	sprintf(th_val, "%dst", val);
	return;
    case '2':
	sprintf(th_val, "%dnd", val);
	return;
    case '3':
	sprintf(th_val, "%drd", val);
	return;
    default:
	sprintf(th_val, "%dth", val);
	return;
    }
}

void
NxSchedule::repeatEveryInput_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    long repeat_val = pThis->getRepeatValue();
    int sing_flag = 0;
    int dbl_flag = 0;
    int th_flag = 0;
    static char message_buf[100];
    char repeat_str[6];
    int len = 0;
    char val_buf[8];
    char num_buf[4];
    char num_char;
    int num_val = 0;

    if (0 == strcmp("", pThis->every_input->value()) ||
	(0 == strcmp("1", pThis->every_input->value())) ||
	(0 == strcmp("0", pThis->every_input->value())) ||
	(0 == strcmp("00", pThis->every_input->value()))) {
	sing_flag = 1;
	strcpy(repeat_str, "");
    }

    if (0 == strcmp("2", pThis->every_input->value())) {
	dbl_flag = 1;
	strcpy(repeat_str, "other");
    }

    strcpy(val_buf, pThis->every_input->value());
    len = strlen(val_buf);

    if (len > 1) {
	memset(num_buf, 0, sizeof(num_buf));
	strcpy(num_buf, val_buf);
	num_val = atoi(num_buf);
	if (11 <= num_val && 13 >= num_val) {
	    th_flag = 1;
	    sprintf(repeat_str, "%dth", num_val);
	}
    }

    if (!sing_flag && !dbl_flag && !th_flag) {
	num_char = val_buf[len - 1];

	num_val = atoi(pThis->every_input->value());

	switch (num_char) {
	case '1':
	    sprintf(repeat_str, "%dst", num_val);
	    break;
	case '2':
	    sprintf(repeat_str, "%dnd", num_val);
	    break;
	case '3':
	    sprintf(repeat_str, "%drd", num_val);
	    break;
	default:
	    sprintf(repeat_str, "%dth", num_val);
	    break;
	}
    }

    tm *tt;
    switch (repeat_val) {
    case REPEAT_NONE:
	sprintf(message_buf, "No Repeat");
	break;
    case REPEAT_DAILY:
	sprintf(message_buf, "Every %s day", repeat_str);
	break;
    case REPEAT_WEEKLY:
	pThis->formatWeekMsg(message_buf, repeat_str);
	break;
    case REPEAT_MONTHLY:
	pThis->formatMonthMsg(message_buf, repeat_str);
	break;
    case REPEAT_YEARLY:
	tt = localtime(&(pThis->m_CurrentDay));
	strftime(message_buf, 99, "%b ", tt);
	pThis->wordValue(tt->tm_mday, val_buf);
	strcat(message_buf, val_buf);
	strcat(message_buf, " Every ");
	strcat(message_buf, repeat_str);
	strcat(message_buf, " year");
	break;
    default:
	sprintf(message_buf, "Need to handle case %s", repeat_str);
	break;
    }
    pThis->repeat_output->label(message_buf);
    //      pThis->repeat_output->hide();
    //      pThis->repeat_output->show();
}

int
NxSchedule::getMonthDayRepeat(time_t date)
{
    tm *tt = localtime(&date);
    int month = tt->tm_mon;
    int count = 0;

    while (month == tt->tm_mon) {
	count++;
	tt->tm_mday = tt->tm_mday - 7;
	mktime(tt);
    }
    return count;
}

void
NxSchedule::formatMonthMsg(char *msg, char *repeat_str)
{
    long val = 0;
    static char date[30];
    char day_val[8];
    tm *tt = localtime(&m_CurrentDay);
    int week_repeat = 0;

    val = getMonthValue();

    if (REPEAT_MONTH_DATE & val) {
	wordValue(tt->tm_mday, day_val);
	sprintf(msg, "The %s of every %s month", day_val, repeat_str);
    } else {
	strftime(date, 29, "%A", tt);
	week_repeat = getMonthDayRepeat(m_CurrentDay);
	wordValue(week_repeat, day_val);
	sprintf(msg, "The %s %s of every %s month", day_val, date,
		repeat_str);
    }
}

void
NxSchedule::formatWeekMsg(char *msg, char *repeat_str)
{
    int count = 0;
    int day_count = 0;
    int idx = 0;
    long val = 0;

    for (idx = 0; idx < 7; idx++) {
	if (m_WeekButtons[idx]->value())
	    count++;
    }

    if (1 == count) {
	val = getWeekValue();
	if (REPEAT_WEEK_SUNDAY & val)
	    sprintf(msg, "Every %s week on Sunday", repeat_str);
	if (REPEAT_WEEK_MONDAY & val)
	    sprintf(msg, "Every %s week on Monday", repeat_str);
	if (REPEAT_WEEK_TUESDAY & val)
	    sprintf(msg, "Every %s week on Tuesday", repeat_str);
	if (REPEAT_WEEK_WEDNESDAY & val)
	    sprintf(msg, "Every %s week on Wednesday", repeat_str);
	if (REPEAT_WEEK_THURSDAY & val)
	    sprintf(msg, "Every %s week on Thursday", repeat_str);
	if (REPEAT_WEEK_FRIDAY & val)
	    sprintf(msg, "Every %s week on Friday", repeat_str);
	if (REPEAT_WEEK_SATURDAY & val)
	    sprintf(msg, "Every %s week on Friday", repeat_str);
    } else {
	sprintf(msg, "Every %s week on", repeat_str);
	day_count = 0;
	for (idx = 0; idx < 7; idx++) {
	    if (m_WeekButtons[idx]->value()) {
		day_count++;
		if (0 != day_count) {
		    if (count > 2 && day_count > 1)
			strcat(msg, ", ");
		    if (count == day_count)
			strcat(msg, " and");
		}
		switch (idx) {
		case 0:
		    strcat(msg, " Sun");
		    break;
		case 1:
		    strcat(msg, " Mon");
		    break;
		case 2:
		    strcat(msg, " Tue");
		    break;
		case 3:
		    strcat(msg, " Wed");
		    break;
		case 4:
		    strcat(msg, " Thur");
		    break;
		case 5:
		    strcat(msg, " Fri");
		    break;
		case 6:
		    strcat(msg, " Sat");
		    break;
		default:
		    break;
		}
	    }
	}
    }

    int width = (int) fl_width(msg);

    if (width > repeat_output->w()) {

	int len = strlen(msg);
	idx = 0;

	while (width > repeat_output->w() - 5) {

	    char test[100];

	    idx++;
	    memset(test, 0, sizeof(test));
	    strncpy(test, msg, len - idx);
	    width = (int) fl_width(test);
	}

	while (!isspace(msg[len - idx])) {
	    idx++;
	}
	msg[len - idx] = '\n';
    }
}

void
NxSchedule::weekDay_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    tm *tt = localtime(&pThis->m_CurrentDay);

    if (0 == pThis->getWeekValue()) {
	pThis->m_WeekButtons[tt->tm_wday]->value(1);
    }
    every_input->do_callback();
}

void
NxSchedule::monthDayDate_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    if (w == pThis->month_date) {
	pThis->month_date->value(1);
	pThis->month_day->value(0);
    } else {
	pThis->month_day->value(1);
	pThis->month_date->value(0);
    }

    every_input->do_callback();
}
