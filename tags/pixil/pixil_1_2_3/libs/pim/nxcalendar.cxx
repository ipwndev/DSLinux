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

#include <FL/Fl.H>
#include "nxcalendar.h"

NxCalendar::NxCalendar(NxApp * p, int x, int y, int w, int h, char *s):
Fl_Group(x, y, w, h, s)
{
    p_App = p;
}

void
NxCalendar::update()
{
    m_pDatePickerCalendar->update();
    update_function(this);;
}

time_t NxCalendar::GetPickedDate()
{
    return m_nDatePicked;
}

void
NxCalendar::doneDate_callback(Fl_Widget * o, void *l)
{
    NxCalendar *pThis = (NxCalendar *) l;

    tm d;
    memset(&d, 0, sizeof(d));
    d.tm_year = pThis->m_pDatePickerCalendar->year() - 1900;
    d.tm_mon = pThis->m_pDatePickerCalendar->month() - 1;
    d.tm_mday = pThis->m_pDatePickerCalendar->day();

    pThis->m_nDatePicked = mktime(&d);

    (*pThis->m_pDatePickerCallback) (o, (void *) (pThis->GetApp()));
}

void
NxCalendar::todayDate_callback(Fl_Widget * o, void *l)
{
    NxCalendar *pThis = (NxCalendar *) l;

    pThis->m_nDatePicked = time(0);

    (pThis->GetDatePickerCB())(o, (void *) (pThis->GetApp()));
}

void
NxCalendar::SetPickedDate(time_t t)
{
    tm *tt = localtime(&t);

    m_pDatePickerCalendar->set_date(tt->tm_year + 1900, tt->tm_mon + 1,
				    tt->tm_mday);
    m_pDatePickerCalendar->update();
}

void
NxCalendar::cancelDate_callback(Fl_Widget * o, void *l)
{
    NxCalendar *pThis = (NxCalendar *) l;
    pThis->m_nDatePicked = 0;
    (*pThis->m_pDatePickerCallback) (o, (void *) (pThis->GetApp()));
}


void
NxCalendar::MakeDateWindow()
{

    //  dateWindow = new NxPimPopWindow("Choose Date",NxApp::Instance()->getGlobalColor(APP_FG),5,5,W_W-10,255);
    dateWindow = new NxPimWindow(W_X, W_Y, W_W, W_H);
    {
	Fl_Calendar *o =
	    new Fl_Calendar(3, 5, dateWindow->GetWindowPtr()->w() - 6, 200);
	m_pDatePickerCalendar = o;
	dateWindow->add(o);
    }

    {
	NxButton *o = new NxButton(POP_BUTTON_X, POP_BUTTON_Y(dateWindow),
				   BUTTON_WIDTH, BUTTON_HEIGHT, "Done");
	o->callback(doneDate_callback, this);
	dateWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o = new NxButton(POP_BUTTON_X + BUTTON_WIDTH + 5,
				   POP_BUTTON_Y(dateWindow),
				   BUTTON_WIDTH, BUTTON_HEIGHT, "Cancel");
	o->callback(cancelDate_callback, this);
	dateWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o =
	    new NxButton(dateWindow->GetWindowPtr()->w() - BUTTON_WIDTH - 5,
			 POP_BUTTON_Y(dateWindow),
			 BUTTON_WIDTH, BUTTON_HEIGHT, "Today");
	o->callback(todayDate_callback, this);
	dateWindow->add((Fl_Widget *) o);
    }

    dateWindow->GetWindowPtr()->end();
}

void
NxCalendar::DateCallback(void (*cb) (Fl_Widget *, void *))
{
    m_pDatePickerCallback = cb;
}
