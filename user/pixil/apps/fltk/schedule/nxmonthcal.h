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


#ifndef _NX_MONCAL_H_
#define _NX_MONCAL_H_

#include <Flek/Fl_Calendar.H>
#include <nxdb.h>

class NxMonthButton;

class NxMonthCalendar:public Fl_Calendar
{
    char _db_name[255];
    NxDb *_db;


  protected:
    void (*m_pDateCallback) (Fl_Widget * w, void *l);
    time_t m_nDatePicked;

  public:

      NxMonthCalendar(int x, int y, int w = (7 * 20), int h = (8 * 20),
		      const char *l = 0, bool bCaption = true, int type =
		      0, NxDb * db = 0, char *db_name = 0);

    virtual void update();
    virtual void update_buttons();
    void SetPickedDate(time_t t);
    time_t GetPickedDate();
    void DateCallback(void (*)(Fl_Widget *, void *));
    Fl_Callback *GetDateCB()
    {
	return *m_pDateCallback;
    }

};

class NxMonthButton:public Fl_Button
{
    int appt[3];
    void draw();

  public:
      NxMonthButton(int X, int Y, int W, int H, const char *L = 0);
    void SetAppts(int appt[]);
    void GetAppts(int appt[]);
};

#endif
