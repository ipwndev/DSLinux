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


#ifndef _NX_YEAR_CAL_H_
#define _NX_YEAR_CAL_H_

#include <FL/Fl_Widget.H>
#include <time.h>

#include <nxbutton.h>
#include <nxbox.h>
#include <nxdb.h>


class NxYearCal:public Fl_Group
{

    int appts[366];
    NxDb *db;
    char db_name[255];
    int draw_day_box;
    int release_box;
    void draw();
    int handle(int);
    int cur_year;
    int x_space;
    int y_space;
    int day_w;
    int day_h;
    int Month(int, int);
    int MonDay(int, int);
    void DrawDateBox(int, int);
    void (*pMonthCallback) (Fl_Widget *, void *);
    void (*pDayCallback) (Fl_Widget *, void *);
    void maybe_do_callback();
    void check_appts();

  public:
      NxYearCal(time_t time, int X, int Y, int W, int H, const char *L = 0);
     ~NxYearCal();
    NxBox *year_Box;
    void ChangeYear(int year);
    static const char *month_name[];
    void SetMonthCallback(void (*)(Fl_Widget *, void *));
    void SetDayCallback(void (*)(Fl_Widget *, void *));
    void SetDb(NxDb * pDb, char *name)
    {
	db = pDb;
	strcpy(db_name, name);
    }
    virtual void resize(int X, int Y, int W, int H);
    static void backYear_callback(Fl_Widget *, void *);
    static void forwYear_callback(Fl_Widget *, void *);
};

#endif
