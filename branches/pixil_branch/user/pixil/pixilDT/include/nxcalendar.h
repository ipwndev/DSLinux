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

#ifndef NXCALENDAR_H_
#define NXCALENDAR_H_

#include <time.h>
#include "Fl_Calendar.H"
#include "nxapp.h"
#include "nxwindow.h"

#define POP_BUTTON_X 5
#define POP_BUTTON_Y(_x) ((_x->GetWindowPtr()->h()) - BUTTON_HEIGHT - 10 )

class NxCalendar:public Fl_Group
{
  protected:
    NxApp * p_App;
    Fl_Calendar *m_pDatePickerCalendar;
    //NxPimPopWindow *dateWindow;
    NxPimWindow *dateWindow;
    time_t m_nDatePicked;
    void (*update_function) (NxCalendar * w);
    void (*m_pDatePickerCallback) (Fl_Widget *, void *);
    static void doneDate_callback(Fl_Widget * w, void *l);
    static void cancelDate_callback(Fl_Widget * w, void *l);
    static void todayDate_callback(Fl_Widget * w, void *l);
  public:
    //              NxPimPopWindow *GetDateWindow() { return dateWindow; }
      NxPimWindow * GetDateWindow()
    {
	return dateWindow;
    }
    NxCalendar(NxApp * p, int x, int y, int w, int h, char *s);
    void CalendarUpdate(void (*pf) (NxCalendar * w))
    {
	update_function = pf;
    }
    virtual void update();
    time_t GetPickedDate();
    void SetPickedDate(time_t t);
    void DateCallback(void (*)(Fl_Widget *, void *));
    virtual void MakeDateWindow();
    Fl_Calendar *GetDatePickerCalendar()
    {
	return m_pDatePickerCalendar;
    }
    Fl_Callback *GetDatePickerCB()
    {
	return *m_pDatePickerCallback;
    }
    NxApp *GetApp()
    {
	return p_App;
    }
};
#endif
