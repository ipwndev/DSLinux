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
NxSchedule::MakeMonthViewWindow()
{
    monthViewWindow = new NxPimWindow(0, 0, W_W, W_H);

    add_window((Fl_Window *) monthViewWindow->GetWindowPtr());
	/***
	{
	  NxButton* o = new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH - 5, BUTTON_HEIGHT, "Back");
	  o->callback(backButton_callback, this);
	  monthViewWindow->add((Fl_Widget*)o);
	}
	****/
    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH + 25, BUTTON_HEIGHT,
			 "This Month");
	o->callback(thisMonth_callback, this);
	monthViewWindow->add((Fl_Widget *) o);
    }
    {
	pMonthCal =
	    new NxMonthCalendar(3, 5, W_W - 5, BUTTON_Y - 3, "", true, MONTH,
				db, SCHEDULE);
	monthViewWindow->add(pMonthCal);
	pMonthCal->DateCallback(monthCal_callback);
    }


    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 180, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_daily);
	o->callback(dailyView_callback, this);
	monthViewWindow->add((Fl_Widget *) o);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 193, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_weekly);
	o->callback(weeklyView_callback, this);
	monthViewWindow->add((Fl_Widget *) o);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 206, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_monthly);
	o->callback(monthlyView_callback, this);
	monthViewWindow->add((Fl_Widget *) o);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 219, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_yearly);
	o->callback(yearlyView_callback, this);
	monthViewWindow->add((Fl_Widget *) o);
    }


    monthViewWindow->GetWindowPtr()->end();
}

void
NxSchedule::monthCal_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->monthViewWindow->GetWindowPtr()->hide();
    if (pThis->pMonthCal->GetPickedDate()) {
	pThis->m_CurrentDay = pMonthCal->GetPickedDate();
	pThis->UpdateDateDisplay();
    }
    //    dayWindow->add((Fl_Widget*)buttonGroup);
    pThis->show_window(dayWindow->GetWindowPtr());

}


void
NxSchedule::thisMonth_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->pMonthCal->SetPickedDate(time(0));
    pThis->pMonthCal->update();
}
