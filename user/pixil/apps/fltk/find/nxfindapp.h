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

#ifndef __NXFIND_H
#define __NXFIND_H

#include <FL/Fl.H>
#include <Flek/Fl_Calendar.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Flv_Table_Child.H>
#include <FL/fl_draw.H>
#include <Flek/Fl_Toggle_Tree.H>

#include <nxapp.h>
#include <nxwindow.h>
#include <nxdb.h>

#include <nxinput.h>
#include <nxoutput.h>
#include <nxbutton.h>
#include <nxcheckbutton.h>
#include <nxscroll.h>

#include <time.h>

extern "C"
{
#include <ipc/colosseum.h>
}

#include "nxfind.h"
#include "handlestatus.h"

#define POP_BUTTON_X 5
#define POP_BUTTON_Y(_x) ((_x->GetWindowPtr()->h()) - BUTTON_HEIGHT - 10 )

#define APP_NAME "Global Search"

#define APP_DESC -1
#define APP_FILE -2

struct NxAppData
{
    char appName[50];
    char nxAppName[50];
    bool appSearch;
};

struct NxSearchData
{
    char data[CL_MAX_MSG_LEN];
    char appName[50];
    char fileName[255];
    int recno;
};

class NxFind;
class c_statusWindow;

class c_statusWindow:public NxPimPopWindow
{
  private:
    static bool expectingResults;
    static bool cancelSearch;
    static NxOutput *status_msg;
    static NxButton *cancelButton;
    char _service[128];
    static Fl_Toggle_Node *curNode;

  public:

    static void statusCancel_callback(Fl_Widget * fl, void *l);

    bool getCancelFlag()
    {
	return cancelSearch;
    }
    void setCancelFlag(bool val)
    {
	cancelSearch = val;
    }
    bool getExpectingFlag()
    {
	return expectingResults;
    }
    void setExpectingFlag(bool val)
    {
	expectingResults = val;
    }
    void setStatus(char *status);
    void setService(char *service)
    {
	strcpy(_service, service);
    }
    char *getService()
    {
	return _service;
    }
    char *getAppNameString(char *format_string);
    char *getApp();
    char *getNextService();
    void search();
    void hide();
    void show();
    void executeSearch(int ipc_id, char *msg);
    void initiateSearch(int ipc_id);
    void setCurNode(Fl_Toggle_Node * node);
    c_statusWindow();
    virtual ~ c_statusWindow();

};

class NxCalendar:public Fl_Calendar
{
  protected:
    NxFind * m_pFind;
  public:
    NxCalendar(NxFind * p, int x, int y, int w = (7 * 20), int h =
	       (8 * 20), bool bCaption = true);
    virtual void update();
};

class NxFind:public NxApp
{
  private:

    static NxWindow *mainWindow;

    static NxPimWindow *dateWindow;
    static NxPimPopWindow *errorWindow;
    static NxPimWindow *findWindow;
    static NxPimWindow *resultWindow;

    //              static void clientIpc_handler(int fd, void *o);
    virtual void ClientIPCHandler(int fd, void *o, int ipc_id = -1);

    // Find window
    static searchStatus *status;
    static NxInput *lookup_input;
    static void searchLookup_callback(Fl_Widget * fl, void *l);
    static void fromCalendar_callback(Fl_Widget * fl, void *l);
    static void toCalendar_callback(Fl_Widget * fl, void *l);
    static NxButton *fromDateButton;
    static NxButton *toDateButton;
    static NxCheckButton *stringCheck;
    static NxCheckButton *dateCheck;
    static void fromDate_callback(Fl_Widget * fl, void *l);
    static void toDate_callback(Fl_Widget * fl, void *l);
    void UpdateFromButton();
    void UpdateToButton();
    static time_t fromTime;
    static time_t toTime;

    // App tree
    static int nodeNum;
    static void checkIt_callback(Fl_Widget * fl, void *l);
    static Fl_Toggle_Tree *appTree;
    static NxScroll *appList;
    static Fl_Pixmap *echeck_pixmap;
    static Fl_Pixmap *check_pixmap;
    static void add_apps(Fl_Toggle_Tree * _appTree);

    // results window
    static int total_found;
    static Flv_Table_Child *results_table;
    static NxOutput *results_message;
    static void doneLookup_callback(Fl_Widget * fl, void *l);
    static void resultsView_callback(Fl_Widget * fl, void *l);
    static void viewRecord(NxSearchData * data);

    // error window
    static NxOutput *error_msg;
    static void errorOk_callback(Fl_Widget * fl, void *l);


    // status window
    static c_statusWindow *statusWindow;
    static NxOutput *status_msg;
    static void statusCancel_callback(Fl_Widget * fl, void *l);

    // for the calendar window
    void date_callback(void (*)(Fl_Widget *, void *));
    static void doneDate_callback(Fl_Widget * fl, void *l);
    static void cancelDate_callback(Fl_Widget * fl, void *l);
    static void todayDate_callback(Fl_Widget * fl, void *l);
    Fl_Calendar *m_pDatePickerCalendar;
    time_t m_nDatePicked;

    time_t GetPickedDate();
    void SetPickedDate(time_t t);

    void (*m_pDatePickerCallback) (Fl_Widget *, void *);

    void MakeFindWindow();
    void MakeResultsWindow();
    void MakeCalendarWindow();
    void MakeErrorWindow();
    void MakeStatusWindow();

  public:
      NxPimWindow * getFindWindow()
    {
	return findWindow;
    }
    NxPimWindow *getResultWindow()
    {
	return resultWindow;
    }
    c_statusWindow *getStatusWindow()
    {
	return statusWindow;
    }
    NxFind(int arg, char *argv[]);
    virtual ~ NxFind();
    Fl_Window *get_main_window();
    void show_default_window();
    NxInput *getLookupInput()
    {
	return lookup_input;
    }
    Flv_Table_Child *getResultsTable()
    {
	return results_table;
    }
    NxOutput *getResultsMessage()
    {
	return results_message;
    }
    Fl_Toggle_Tree *getAppTree()
    {
	return appTree;
    }
    int getTotalFound()
    {
	return total_found;
    }
    int getNodeNum()
    {
	return nodeNum;
    }
    void setNodeNum(int val)
    {
	nodeNum = val;
    }
    time_t getStartTime()
    {
	return fromTime;
    }
    time_t getEndTime()
    {
	return toTime;
    }
    bool getStringCheckValue()
    {
	return stringCheck->value();
    }
    bool getDateCheckValue()
    {
	return dateCheck->value();
    }
    static void addData(NxSearchData * data);

    // menu callbacks
    static void exit_callback(Fl_Widget * fl, void *l);
};

#endif
