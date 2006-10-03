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

extern Fl_Menu_Item schedMenuItems[];

void
NxSchedule::MakeMonthWindow()
{


    monthWindow = new NxPimWindow(APP_NAME, schedMenuItems, 0, "",
				  SCHEDULE, (void (*)(const char *)) 0);

    add_window((Fl_Window *) monthWindow->GetWindowPtr());

    {
	Fl_Button *o =
	    new Fl_Button(W_W - 10 - (4 * DAY_S), 9, DAY_S, DAY_S, "@<<");
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	o->box(FL_FLAT_BOX);
	o->labeltype(FL_SYMBOL_LABEL);
	o->callback(bak_year_callback, this);
	monthWindow->add((Fl_Widget *) o);

	o = new Fl_Button(W_W - 10 - (3 * DAY_S), 9, DAY_S, DAY_S, "@<");
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	o->box(FL_FLAT_BOX);
	o->labeltype(FL_SYMBOL_LABEL);
	o->callback(bak_month_callback, this);
	monthWindow->add((Fl_Widget *) o);

	o = new Fl_Button(W_W - 10 - (2 * DAY_S), 9, DAY_S, DAY_S, "@>");
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	o->box(FL_FLAT_BOX);
	o->labeltype(FL_SYMBOL_LABEL);
	o->callback(adv_month_callback, this);
	monthWindow->add((Fl_Widget *) o);

	o = new Fl_Button(W_W - 10 - (DAY_S), 9, DAY_S, DAY_S, "@>>");
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	o->box(FL_FLAT_BOX);
	o->labeltype(FL_SYMBOL_LABEL);
	o->callback(adv_year_callback, this);
	monthWindow->add((Fl_Widget *) o);

    }
    {
	m_pCalendar =
	    new NxCalendar((NxApp *) this, 0, 32, W_W, BUTTON_Y - 38, "");
	m_pCalendar->
	    CalendarUpdate((void (*)(NxCalendar *)) calendar_updated);
	monthWindow->add((Fl_Widget *) m_pCalendar);
    }

    {
	Fl_Button *o =
	    new Fl_Button(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			  "GoTo");
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	o->box(FL_SHADOW_BOX);
	//    o->callback(doneView_callback);
	monthWindow->add((Fl_Widget *) o);
    }

    monthWindow->GetWindowPtr()->end();
}

void
NxSchedule::bak_year_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    pThis->m_pCalendar->GetDatePickerCalendar()->previous_year();
}

void
NxSchedule::bak_month_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    pThis->m_pCalendar->GetDatePickerCalendar()->previous_month();
}

void
NxSchedule::adv_year_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    pThis->m_pCalendar->GetDatePickerCalendar()->next_year();
}


void
NxSchedule::adv_month_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    pThis->m_pCalendar->GetDatePickerCalendar()->next_month();
}

void
NxSchedule::calendar_updated(NxCalendar * w)
{
    static char buf[30];
    sprintf(buf, "%s %d",
	    w->GetDatePickerCalendar()->month_name[w->
						   GetDatePickerCalendar()->
						   month() - 1],
	    w->GetDatePickerCalendar()->year());
    monthWindow->menu_button->label(buf);
}
