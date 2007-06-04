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
#include "nxyearcal.h"

void
NxSchedule::MakeYearViewWindow()
{

    yearViewWindow = new NxPimWindow(0, 0, W_W, W_H);
    add_window((Fl_Window *) yearViewWindow->GetWindowPtr());
    {
	//            NxScroll *o  = new NxScroll(0, 0, W_W, BUTTON_Y - 1);
	//            {
	NxYearCal *o = pYearCal =
	    new NxYearCal(m_CurrentDay, 0, 0, W_W, BUTTON_Y - 2);
	o->SetMonthCallback(month_callback);
	o->SetDayCallback(day_callback);
	o->SetDb(db, SCHEDULE);
	//yearViewWindow->add(o);
	//              }
	//              o->end();
	yearViewWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o = new NxButton(W_W / 2 - 15, BUTTON_Y, 15, 15, "@<");
	o->movable(false);
	o->labeltype(FL_SYMBOL_LABEL);
	o->box(FL_FLAT_BOX);
	o->callback(pYearCal->backYear_callback, pYearCal);
	yearViewWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o = new NxButton(W_W / 2, BUTTON_Y, 15, 15, "@>");
	o->movable(false);
	o->labeltype(FL_SYMBOL_LABEL);
	o->box(FL_FLAT_BOX);
	o->callback(pYearCal->forwYear_callback, pYearCal);
	yearViewWindow->add((Fl_Widget *) o);
    }

    tm *tt;
    static char year_buf[6];
    {
	tt = localtime(&m_CurrentDay);
	strftime(year_buf, 5, "%Y", tt);
	NxBox *o = new NxBox(W_W / 2 - 55, BUTTON_Y - 1, 50, 20, year_buf);
	o->movable(false);
	yearViewWindow->add((Fl_Widget *) o);
	pYearCal->year_Box = o;
    }


    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 180, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_daily);
	o->callback(dailyView_callback, this);
	yearViewWindow->add((Fl_Widget *) o);
	o->movable(false);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 193, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_weekly);
	o->callback(weeklyView_callback, this);
	yearViewWindow->add((Fl_Widget *) o);
	o->movable(false);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 206, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_monthly);
	o->callback(monthlyView_callback, this);
	yearViewWindow->add((Fl_Widget *) o);
	o->movable(false);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 219, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_yearly);
	o->callback(yearlyView_callback, this);
	yearViewWindow->add((Fl_Widget *) o);
	o->movable(false);
    }


    yearViewWindow->GetWindowPtr()->end();
}

void
NxSchedule::day_callback(Fl_Widget * fl, void *o)
{
    time_t *time;
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());

    time = (time_t *) o;

    pThis->m_CurrentDay = *time;
    pThis->UpdateDateDisplay();
    //      pThis->dayWindow->GetWindowPtr()->add(pThis->buttonGroup);
    pThis->show_window(dayWindow->GetWindowPtr());
}

void
NxSchedule::month_callback(Fl_Widget * fl, void *o)
{
    time_t *time;
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());

    time = (time_t *) o;

    pThis->m_CurrentDay = *time;
    pThis->UpdateDateDisplay();
    //      pThis->monthViewWindow->GetWindowPtr()->add(pThis->buttonGroup);
    pThis->pMonthCal->SetPickedDate(pThis->m_CurrentDay);
    pThis->show_window(monthViewWindow->GetWindowPtr());
}
