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
// Center of the window for the Scheduler display               //
//--------------------------------------------------------------//
#ifndef SCHEDULER_H_

#define SCHEDULER_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>
#include "Messages.h"
#include "SchedulerDaily.h"
#include "SchedulerMonthly.h"
#include "SchedulerWeekly.h"
#include "SchedulerYearly.h"
class Scheduler:public Fl_Group
{
  public:Scheduler(Fl_Widget * pParent);
    // Default Constructor
    ~Scheduler();		// Destructor
    void Delete(int nRow,	// Delete an appointment
		time_t nDate);
    void Edit(int nRow,		// Edit an appointment
	      time_t nDate);
    void EditNew(time_t nDate);	// Add a new appointment
    int Message(PixilDTMessage nMessage,	// Notification from the parent widget
		int nInfo);
    void SelectDay(time_t nDate);	// Select a given day and go to the daily page
  private:  Fl_Tabs * m_pTab;	// The tab widget
    SchedulerDaily *m_pDailyPage;	// The daily tab page
    SchedulerMonthly *m_pMonthlyPage;	// The monthly tab page
    SchedulerWeekly *m_pWeeklyPage;	// The weekly tab page
    SchedulerYearly *m_pYearlyPage;	// The Yearly tab page
  public:void Refresh()
    {
	m_pDailyPage->Refresh();
	m_pMonthlyPage->Refresh();
	m_pWeeklyPage->Refresh();
	m_pYearlyPage->Refresh();
    }
};


#endif /*  */
