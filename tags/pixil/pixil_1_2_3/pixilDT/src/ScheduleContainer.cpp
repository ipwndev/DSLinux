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

//--------------------------------------------------------------//
// Schedule Container widget                                    //
//--------------------------------------------------------------//
#include "config.h"
#include <cstdio>
#include <FL/fl_draw.H>
#include "FLTKUtil.h"
#include "Options.h"
#include "ScheduleContainer.h"
#include "ScheduleDay.h"
#include "Scheduler.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
ScheduleContainer::ScheduleContainer(int nX,
				     int nY,
				     int nWidth, int nHeight, int nDayCount)
    :
Fl_Scroll(nX, nY, nWidth, nHeight)
{
    char szLabel[24];
    int i;
    time_t nDate = NormalizeDate(time(NULL));

    // Set that nothing is selected
    m_nSelectedRow = -1;

    // Get the size of an hour in the display, same calculation as in GetHourWidth
    m_nHourHeight = 3 * labelsize();
    sprintf(szLabel, "  12:00%s ", _("pm"));
    fl_measure(szLabel, m_nHourWidth, m_nHourHeight);
    m_nHourHeight = ((3 * m_nHourHeight + 1) & (-2));	// Make this an even number for half hours

    // Set a vertical scrollbar only]
    type(Fl_Scroll::VERTICAL);

    // Create the group for the hourly labels
    m_pHour = new ScheduleHours(nX, nY, m_nHourWidth, m_nHourHeight);

    // Create a group for resizing
    m_pResizeGroup = new Fl_Group(nX + m_nHourWidth,
				  nY,
				  nWidth - m_nHourWidth - scrollbar.w(),
				  24 * m_nHourHeight);


    // Create the days for the schedule display
    m_nDayCount = nDayCount;
    m_ppDay = new PSCHEDULEDAY[m_nDayCount];
    for (i = 0; i < nDayCount; ++i) {
	m_ppDay[i] =
	    new ScheduleDay(nX + m_nHourWidth +
			    i * ((nWidth - m_nHourWidth - scrollbar.w()) /
				 nDayCount), nY,
			    i ==
			    nDayCount - 1 ? nWidth - m_nHourWidth -
			    scrollbar.w() - (nDayCount -
					     1) * ((nWidth - m_nHourWidth -
						    scrollbar.w()) /
						   nDayCount)
			    : (nWidth - m_nHourWidth -
			       scrollbar.w()) / nDayCount, 24 * m_nHourHeight,
			    m_nHourHeight, nDate);
	nDate = AddDays(nDate, 1);
    }

    // End the resizing group
    m_pResizeGroup->end();
    resizable(m_pResizeGroup);

    // End this widget
    end();

    // Set up a frame
    box(FL_ENGRAVED_FRAME);

    // Position at the correct start time
    position(0, Options::GetDayBegins() * m_nHourHeight);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
ScheduleContainer::~ScheduleContainer()
{
    delete[]m_ppDay;
}


//--------------------------------------------------------------//
// Process a request to delete an appointment.  This            //
// ScheduleContainer is within an anonymous Fl_Group within     //
// either a SchedulerDaily or SchedulerWeekly page.  Either of  //
// those is within a Fl_Tabs within a Scheduler object.  This   //
// method notifies the Scheduler object of the request.         //
//--------------------------------------------------------------//
void
ScheduleContainer::Delete(int nRow, time_t nDate)
{
    ((Scheduler *) (parent()->parent()->parent()->parent()))->Delete(nRow,
								     nDate);
}


//--------------------------------------------------------------//
// Process a request to edit an appointment.  This              //
// ScheduleContainer is within an anonymous Fl_Group within     //
// either a SchedulerDaily or SchedulerWeekly page.  Either of  //
// those is within a Fl_Tabs within a Scheduler object.  This   //
// method notifies the Scheduler object of the request.         //
//--------------------------------------------------------------//
void
ScheduleContainer::Edit(int nRow, time_t nDate)
{
    ((Scheduler *) (parent()->parent()->parent()->parent()))->Edit(nRow,
								   nDate);
}


//--------------------------------------------------------------//
// Process a request to add a new appointment.  This            //
// ScheduleContainer is within an anonymous Fl_Group within     //
// either a SchedulerDaily or SchedulerWeekly page.  Either of  //
// those is within a Fl_Tabs within a Scheduler object.  This   //
// method notifies the Scheduler object of the request.         //
//--------------------------------------------------------------//
void
ScheduleContainer::EditNew(time_t nDate)
{
    // Go fire up the new appointment dialog
    ((Scheduler *) (parent()->parent()->parent()->parent()))->EditNew(nDate);
}


//--------------------------------------------------------------//
// Refresh all ScheduleDay objects in this container            //
//--------------------------------------------------------------//
void
ScheduleContainer::Refresh(time_t nDate)
{
    int i;

    // Normalize the date
    m_nDate = nDate =::NormalizeDate(nDate);

    // Tell each child of the refresh
    for (i = 0; i < m_nDayCount; ++i) {
	m_ppDay[i]->Refresh(nDate);
	nDate = AddDays(nDate, 1);
    }

    // Set that nothing is selected
    m_nSelectedRow = -1;
}


//--------------------------------------------------------------//
// Override the Fl_Scroll resize processing to cause the days   //
// to widen on a resize operation.                              //
//--------------------------------------------------------------//
void
ScheduleContainer::resize(int nX, int nY, int nWidth, int nHeight)
{
    // Invoke the base class
    Fl_Scroll::resize(nX, nY, nWidth, nHeight);

    // Fix the x-position and width of the resizabel group in the container
    // since Fl_Scroll only moves children and does not resize them
    m_pResizeGroup->resize(nX + m_nHourWidth,
			   m_pResizeGroup->y(),
			   nWidth - m_nHourWidth - scrollbar.w(),
			   m_pResizeGroup->h());

    // Now refresh the ScheduleDay groups to get the events to be the correct size
    Refresh(m_nDate);
}
