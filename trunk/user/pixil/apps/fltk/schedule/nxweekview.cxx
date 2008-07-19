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

#ifdef DEBUG
#define DPRINT(str, args...) printf("DEBUG: " str, ## args)
#else
#define DPRINT(args...)
#endif

void
NxSchedule::MakeWeekViewWindow()
{
    weekViewWindow = new NxPimWindow(0, 0, W_W, W_H);
    add_window((Fl_Window *) weekViewWindow->GetWindowPtr());

    weekViewWindow->GetWindowPtr()->resizable(NULL);

    {
	monYearBox =
	    new NxBox(5, 5, BUTTON_WIDTH + 25, BUTTON_HEIGHT, "Jun '01");
	monYearBox->resize(false);
	monYearBox->movable(false);
	weekViewWindow->add(monYearBox);
    }
    {
	weekBox = new NxBox(W_W - 5 - DAY_S - BUTTON_WIDTH, 5, BUTTON_WIDTH,
			    BUTTON_HEIGHT, "");
	weekBox->resize(false);
	weekBox->movable(false);
	weekBox->box(FL_FLAT_BOX);
	monYearBox->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	weekBox->color(NxApp::Instance()->getGlobalColor(APP_BG));
	NxApp::Instance()->def_font(weekBox);
	weekViewWindow->add(weekBox);
    }
    {
	NxButton *o = new NxButton(W_W - 5 - DAY_S, 5, DAY_S, DAY_S, "@>");
	o->movable(false);
	o->box(FL_FLAT_BOX);
	o->labeltype(FL_SYMBOL_LABEL);
	o->callback(adv_week_callback, this);
	weekViewWindow->add((Fl_Widget *) o);

	o = new NxButton(W_W - 5 - (2 * DAY_S) - BUTTON_WIDTH, 5, DAY_S,
			 DAY_S, "@<");
	o->box(FL_FLAT_BOX);
	o->labeltype(FL_SYMBOL_LABEL);
	o->callback(bak_week_callback, this);
	weekViewWindow->add((Fl_Widget *) o);
    }
    {
	for (int i = 7; i > 0; i--) {
	    NxButton *o = new NxButton(W_W - 12 - ((i) * WEEK_S),
				       12 + BUTTON_HEIGHT, WEEK_S, WEEK_H,
				       "");
	    o->movable(false);
	    o->callback(weekViewButtons_callback, this);
	    weekViewDayButtons[7 - i] = o;
	    o->box(FL_FLAT_BOX);
	    weekViewWindow->add((Fl_Widget *) o);
	}
    }
    {
	weekGrid =
	    new WeekGrid(W_W - 12 - ((7) * WEEK_S),
			 BUTTON_HEIGHT + WEEK_S + 18, 7 * WEEK_S,
			 24 * WEEK_S);
	weekGrid->SetDb(db, SCHEDULE);
	weekGrid->SetFont(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);
	weekViewWindow->add(weekGrid);
    }

    {

	NxBox *o = new NxBox(BUTTON_X,
			     weekGrid->y() + weekGrid->h() + 5,
			     (weekGrid->x() + weekGrid->w() - GRID_W) -
			     BUTTON_X - 1,
			     2 * GRID_W);

	o->box(FL_BORDER_BOX);
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	//NxApp::Instance()->def_font((Fl_Widget*)o);
	weekViewWindow->add((Fl_Widget *) o);
	weekGrid->SetOutputBox(o);
    }

    {
	GridButton *o = upGridButton =
	    new GridButton(weekGrid->x() + weekGrid->w() - GRID_W,
			   weekGrid->y() + weekGrid->h() + 5, GRID_W, GRID_W,
			   "@UpArrow");
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	o->SetGrid(weekGrid);
	o->callback(weekGridDownButton_callback, this);
	weekViewWindow->add((Fl_Widget *) o);
    }
    {
	GridButton *o = downGridButton =
	    new GridButton(weekGrid->x() + weekGrid->w() - GRID_W,
			   weekGrid->y() + weekGrid->h() + 5 + GRID_W, GRID_W,
			   GRID_W, "@DownArrow");
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	o->SetGrid(weekGrid);
	o->callback(weekGridUpButton_callback, this);
	weekViewWindow->add((Fl_Widget *) o);
    }

	/***
	{
    NxButton* o = new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH - 5, BUTTON_HEIGHT, "Back");
		o->callback(backButton_callback, this);
		weekViewWindow->add((Fl_Widget*)o);
	}
	****/

    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH - 5, BUTTON_HEIGHT,
			 "Goto");
	o->movable(false);
	o->callback(weekGoto_callback, this);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	weekViewWindow->add((Fl_Widget *) o);
    }

    {
	w_pCalendar =
	    new NxWeekCalendar((NxApp *) this, 0, 32, W_W, BUTTON_Y - 38, "");
	DPRINT("Made weekview calendar [%p]\n", w_pCalendar);
	w_pCalendar->
	    CalendarUpdate((void (*)(NxCalendar *)) calendar_updated);
	weekViewWindow->add(w_pCalendar);
    }


    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 180, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_daily);
	o->movable(false);
	o->callback(dailyView_callback, this);
	weekViewWindow->add((Fl_Widget *) o);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 193, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_weekly);
	o->callback(weeklyView_callback, this);
	weekViewWindow->add((Fl_Widget *) o);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 206, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_monthly);
	o->callback(monthlyView_callback, this);
	weekViewWindow->add((Fl_Widget *) o);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 219, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_yearly);
	o->callback(yearlyView_callback, this);
	weekViewWindow->add((Fl_Widget *) o);
    }


    weekViewWindow->GetWindowPtr()->end();
    UpdateWeekViewButtons();
}

void
NxSchedule::weekDatePicked_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->weekDateWindow->GetWindowPtr()->hide();
    pThis->show_window(weekViewWindow->GetWindowPtr());

    if (pCalendar->GetPickedDate()) {
	pThis->m_CurrentDay = pCalendar->GetPickedDate();
	pThis->pCalendar->SetPickedDate(pThis->m_CurrentDay);
	pThis->UpdateDateDisplay();
    }
}

void
NxSchedule::weekGoto_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->set_date_picker((NxCalendar *) w_pCalendar);

    pCalendar->SetPickedDate(pThis->m_CurrentDay);

    NxApp::Instance()->show_window(weekDateWindow->GetWindowPtr(),
				   DEACTIVATE,
				   weekViewWindow->GetWindowPtr());

    pCalendar->DateCallback(weekDatePicked_callback);

}

void
NxSchedule::backButton_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    if (!dayWindow->GetWindowPtr()->shown()) {
	//            dayWindow->add((Fl_Widget*)buttonGroup);
	pThis->show_window(dayWindow->GetWindowPtr());
    }
}

void
NxSchedule::weekGridUpButton_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    int count = pThis->weekGrid->UpCount();

    if (0 != count) {
	pThis->weekGrid->hide();
	pThis->weekGrid->show();
    }
}

void
NxSchedule::weekGridDownButton_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    int count = pThis->weekGrid->DownCount();

    if (0 != count) {
	pThis->weekGrid->hide();
	pThis->weekGrid->show();
    }
}

void
NxSchedule::UpdateWeekViewButtons()
{
    tm *tt = localtime(&m_CurrentDay);
    static char buf[7][8];
    time_t new_time;
    int wday = tt->tm_wday;
    char *_d[] = { "S", "M", "T", "W", "T", "F", "S" };

    DPRINT("Update Week View Buttons\n");
    for (int i = 7; i > 0; i--) {
	for (int idx = 6; idx >= 0; idx--) {
	    tt = localtime(&m_CurrentDay);
	    tt->tm_mday = tt->tm_mday + (idx - wday);
	    new_time = mktime(tt);
	    tt = localtime(&new_time);
	    if (0 == tt->tm_wday) {
		DPRINT("SETTING sunday date\n");
		weekGrid->SetDateSunday(new_time);
	    }
	    if (tt->tm_wday != (7 - i))
		continue;
	    else {
		sprintf(buf[7 - i], "%s\n%d", _d[7 - i], tt->tm_mday);
		break;
	    }
	}
	weekViewDayButtons[7 - i]->label(buf[7 - i]);
	weekViewDayButtons[7 - i]->redraw();
    }

}

void
NxSchedule::weekViewButtons_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    int nDOW = -1;

    for (int idx = 0; idx < 7; idx++) {
	if (pThis->weekViewDayButtons[idx] == w) {
	    nDOW = idx;
	    break;
	}
    }

    if (nDOW == -1)
	return;

    tm *tt = localtime(&pThis->m_CurrentDay);

    // Calcuate the new current day; subtract the old
    // "current day" doay of the week from the new day of week.
    // and Bob's your uncle
    pThis->m_CurrentDay += (86400 * (nDOW - tt->tm_wday));
    pThis->UpdateDateDisplay();

    //      pThis->dayWindow->add((Fl_Widget*)buttonGroup);
    pThis->show_window(dayWindow->GetWindowPtr());

}
