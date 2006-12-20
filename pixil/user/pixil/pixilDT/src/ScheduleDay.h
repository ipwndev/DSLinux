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
// Schedule Day widget                                          //
//--------------------------------------------------------------//
#ifndef SCHEDULEDAY_H_

#define SCHEDULEDAY_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <ctime>
#include <map>
#include <FL/Fl_Group.H>
#include "ScheduleContainer.h"

#include "FLTKUtil.h"
using namespace std;
class ScheduleDay:public Fl_Group
{
  public:ScheduleDay(int nX,	// Constructor
		int nY, int nWidth, int nHeight, int nHourHeight,
		time_t nDate);
     ~ScheduleDay();		// Destructor
    void Delete(int nRow);	// Process a request to delete an appointment
    void Edit(int nRow);	// Process a request to edit an appointment
    void EditNew(time_t nDate);	// Process a request to add a new appointment
    void Refresh(time_t nDate);	// Refresh this display
    void SetSelectedItem(int nRow);	// Notification that an item has been selected
  protected:void draw();	// Draw this widget
    int handle(int nEvent);	// Handle events for this widget
  private:int m_nDate;		// Date being displayed
    int m_nHourHeight;		// The height of an hour on the display
      multimap < int, int >m_mRecord;	// Record numbers of the appointments for this day
    void RightMouse();		// Show a popup menu for a right mouse click
};
typedef ScheduleDay *PSCHEDULEDAY;

#endif /*  */
