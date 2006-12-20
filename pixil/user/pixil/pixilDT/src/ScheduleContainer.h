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
#ifndef SCHEDULECONTAINER_H_

#define SCHEDULECONTAINER_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Scroll.H>
class ScheduleDay;

#include "ScheduleDay.h"
#include "ScheduleHours.h"

#include "FLTKUtil.h"
class ScheduleContainer:public Fl_Scroll
{
  public:ScheduleContainer(int nX,
		      // Constructor
		      int nY, int nWidth, int nHeight, int nDayCount);
     ~ScheduleContainer();	// Destructor
    void Delete(int nRow,	// Process a request to delete an appointment
		time_t nDate);
    void Edit(int nRow,		// Process a request to edit an appointment
	      time_t nDate);
    void EditNew(time_t nDate);	// Process a request to add a new appointment
    inline int GetContainerWidth() const	// Get the width of this widget not counting the hours and scrollbar
    {
	return (w() - m_nHourWidth - scrollbar.w());
    }
    inline int GetHoursWidth() const	// Get the width of the hours widget
    {
	return (m_nHourWidth);
    }
    inline int GetSelectedItem() const	// Get the last selected item
    {
	return (m_nSelectedRow);
    }
    void Refresh(time_t nDate);	// Refresh all displays within this container
    void resize(int nX,		// From Fl_Widget/Fl_Scroll - virtual resize method
		int nY, int nWidth, int nHeight);
    inline void SetSelectedItem(int nRow)	// Notification that an item has been selected
    {
	m_nSelectedRow = nRow;
    }
  private:  Fl_Group * m_pResizeGroup;
    // Contains all ScheduleDay objects
    int m_nDayCount;		// Count of days shown here
    int m_nHourHeight;		// Height of an hour in the display
    int m_nHourWidth;		// Width of an hour in the display
    int m_nSelectedRow;		// The selected row from child ScheduleDay/ScheduleEvent widgets
    time_t m_nDate;		// Date currently being displayed
    ScheduleDay **m_ppDay;	// Schedule days for this widget
    ScheduleHours *m_pHour;	// The hours on the left of the container
};


#endif /*  */
