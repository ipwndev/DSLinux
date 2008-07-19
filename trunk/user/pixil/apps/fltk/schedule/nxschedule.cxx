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
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <limits.h>

#include <pixil_config.h>
#include <par/par.h>

#include "nxschedule.h"

#if defined(CONFIG_APP_ALARM) && defined(CONFIG_COLOSSEUM)
#define DO_ALARM 1
#endif

#ifdef DEBUG
#define DPRINT(str, args...) printf("DEBUG: " str, ## args)
#else
#define DPRINT(args...)
#endif

extern int exit_flag;

about about_schedule = {
    "About Scheduler",
    "(c) 2001, Century Software.",
    "jasonk@censoft.com",
    "08/24/01",
    "1.1"
};

////////////
// Database 

// Schedule
field sFields[] = {
    {'i', 1, 0},		// Field 0:id
    {'i', 1, 0},		//        1:categoryId
    {'l', 1, 0},		//        2:startTime
    {'l', 1, 0},		//        3:endTime
    {'i', 1, 0},		//        4:allDayFlag
    {'i', 1, 0},		//        5:repeatFlag 1 (day, week, month, year)
    {'i', 1, 0},		//        6:repeatFlag 2
    {'l', 1, 0},		//        7:repeatFlag 3
    {'l', 1, 0},		//        8:repeatWkMonFlag
    {'i', 1, 0},		//        9:entryType
    {'c', DESC, 0},		//       10:description
    {'i', 1, 0},		//                       11:execption
    {'i', 1, 0},		//                       12:recno pointer
    {'i', 1, 0},		//       13:alarm interval
    {'i', 1, 0},		//                       14:alarm_flags
    {0}
};

// Database
fildes sFile = {		// system file
    0, 0, 0,			// database file
    "dbf",			// extension
    15,				// nfields
    &sFields[0]			// fieldlist
};

#define CAT_NAME 10

// Category List
field catFields[] = {
    {'i', 1, 0},		// Field 0:catid
    {'c', CAT_NAME, 0},		//       1:cat_name
    {0}
};

// Database
fildes catFile = {
    0, 0, 0,
    "dbf",
    2,
    &catFields[0]
};

// Menus

Fl_Menu_Item schedMenuItems[] = {

    {"Edit", 0, 0, 0, FL_SUBMENU},
    {"Undo", 0, NxApp::undo_callback},
    {"Cut", 0, NxApp::cut_callback},
    {"Copy", 0, NxApp::copy_callback},
    {"Paste", 0, NxApp::paste_callback, 0, FL_MENU_DIVIDER},
    //  { "Select All"},
    //  { "Keyboard", 0, NxApp::keyboard_callback, 0, FL_MENU_DIVIDER},
    {"Exit Scheduler", 0, NxSchedule::exit_callback},
    {0},

    {"Options", 0, 0, 0, FL_SUBMENU},
    //{ "Font"},
    //{ "Phone Lookup"},
    {"Search", 0, NxSchedule::lookup_callback},
    {"About Scheduler", 0, NxApp::show_about},
    {0},

    {0},

};

NxDb *
    NxSchedule::db;
int
    NxSchedule::idNum;
int
    NxSchedule::g_EditFlag;

#ifdef DO_ALARM
int
    NxSchedule::alarm_count_;
int
    NxSchedule::alarms[MAX_ALARMS];

char
    NxSchedule::alarm_buf[255];
#endif

NxWindow *
    NxSchedule::mainWindow;

NxPimWindow *
    NxSchedule::dayWindow;
NxPimWindow *
    NxSchedule::monthWindow;
NxPimWindow *
    NxSchedule::setTimeWindow;
NxPimWindow *
    NxSchedule::weekViewWindow;
NxPimWindow *
    NxSchedule::monthViewWindow;
NxPimWindow *
    NxSchedule::yearViewWindow;

//NxPimPopWindow * NxSchedule::repeatWindow;
NxPimWindow *
    NxSchedule::repeatWindow;
NxPimPopWindow *
    NxSchedule::detailsWindow;
//NxPimPopWindow * NxSchedule::timeWindow;
NxPimWindow *
    NxSchedule::timeWindow;
//NxPimPopWindow * NxSchedule::dateWindow;
NxPimWindow *
    NxSchedule::dateWindow;
//NxPimPopWindow * NxSchedule::weekDateWindow;
NxPimWindow *
    NxSchedule::weekDateWindow;
NxPimPopWindow *
    NxSchedule::deleteWindow;
NxPimPopWindow *
    NxSchedule::resultsWindow;
//NxPimPopWindow * NxSchedule::lookupWindow;
NxPimWindow *
    NxSchedule::lookupWindow;
NxPimPopWindow *
    NxSchedule::errorWindow;
NxPimPopWindow *
    NxSchedule::repeatEventWindow;

#ifdef DO_ALARM
NxPimPopWindow *
    NxSchedule::alarmViewWindow;
#endif

Flv_Table_Child *
    NxSchedule::table;
Flv_Table_Child *
    NxSchedule::results_table;

NxCategoryList *
    NxSchedule::dayCat;
NxCategoryList *
    NxSchedule::weekCat;
NxCategoryList *
    NxSchedule::detailsCat;
NxCategoryList *
    NxSchedule::setTimeCat;
NxCategoryList *
    NxSchedule::cat_list[CAT_NUM];

NxCalendar *
    NxSchedule::pCalendar;
NxCalendar *
    NxSchedule::m_pCalendar;

NxWeekCalendar *
    NxSchedule::w_pCalendar;

NxMonthCalendar *
    NxSchedule::pMonthCal;

NxYearCal *
    NxSchedule::pYearCal;

NxInput *
    NxSchedule::lookup_input;

NxIntInput *
    NxSchedule::every_input;
#ifdef DO_ALARM
NxIntInput *
    NxSchedule::m_pDetailsAlarmInput;
#endif

NxMenuButton *
    NxSchedule::end_list;

#ifdef DO_ALARM
NxMenuButton *
    NxSchedule::m_pDetailsAlarmInt;
#endif

NxOutput *
    NxSchedule::results_message;
NxOutput *
    NxSchedule::error_msg;

NxBox *
    NxSchedule::monYearBox;

#ifdef DO_ALARM
NxBox *
    NxSchedule::alarm_msg;
#endif

NxBox *
    NxSchedule::weekBox;
NxBox *
    NxSchedule::repeat_output;
NxBox *
    NxSchedule::message_output;
NxBox *
    NxSchedule::every_box;
NxBox *
    NxSchedule::list_box;
NxBox *
    NxSchedule::week_box;
NxBox *
    NxSchedule::month_box;

NxCheckButton *
    NxSchedule::stringCheck;
NxCheckButton *
    NxSchedule::dateCheck;

#ifdef DO_ALARM
NxCheckButton *
    NxSchedule::m_pDetailsAlarmCheck;
#endif

NxButton *
    NxSchedule::fromDateButton;
NxButton *
    NxSchedule::toDateButton;
//NxButton * NxSchedule::dailyButton;
//NxButton * NxSchedule::weeklyButton;
//NxButton * NxSchedule::monthlyButton;
//NxButton * NxSchedule::yearlyButton;
NxButton *
    NxSchedule::no_repeat;
NxButton *
    NxSchedule::day_repeat;
NxButton *
    NxSchedule::week_repeat;
NxButton *
    NxSchedule::month_repeat;
NxButton *
    NxSchedule::year_repeat;
NxButton *
    NxSchedule::month_day;
NxButton *
    NxSchedule::month_date;
NxButton *
    NxSchedule::detailsDeleteButton;

//Fl_Group * NxSchedule::buttonGroup;

time_t NxSchedule::fromTime;
time_t NxSchedule::toTime;

bool NxSchedule::g_SearchFlag;

// This will update the day display with the information contained
// in m_CurrentDay (a time_t) This is only applicable for the day
// display window. This will also add items to the main display.
void
NxSchedule::UpdateDateDisplay()
{
    tm *tt = localtime(&m_CurrentDay);
    static char buf[30];

    //strftime(buf,29,"%b %d, %y",tt);
    GetDateString(buf, tt, sizeof(buf), SHORT_YEAR);
    dayWindow->menu_button->label(buf);
    dayWindow->menu_button->redraw();

    // reset all the day buttons to be off
    for (int i = 0; i < 7; i++)
	m_DayButtons[i]->value(0);

    // set the new day button on
    m_DayButtons[tt->tm_wday]->value(1);

    UpdateWeekView();

    if (weekGrid) {
	weekGrid->redraw();
	weekGrid->hide();
	weekGrid->show();
    }
    add_items(table);
}

void
NxSchedule::UpdateWeekView()
{
    static char temp_date[30];
    static char date[30];
    static char week_num[30];
    static char month_str[75];
    int week = 0;
    int wday = 0;
    int idx = 0;
    time_t new_time;
    struct tm *tt = localtime(&m_CurrentDay);
    int month = tt->tm_mon;
    struct tm *temp_tt = localtime(&m_CurrentDay);

    wday = tt->tm_wday;
    strftime(date, 29, "%b '%Y", tt);

    strftime(week_num, 29, "%V", tt);
    week = atoi(week_num);

    // check forward from day       
    sprintf(month_str, "%s", date);
    for (idx = wday; idx < 7; idx++) {
	if (idx - 1 < wday)
	    continue;
	else {
	    temp_tt->tm_mday++;
	    new_time = mktime(temp_tt);
	    temp_tt = localtime(&new_time);
	    if (month != temp_tt->tm_mon) {
		if (week == 1 || week == 53) {
		    strftime(temp_date, 29, "%b '%Y", temp_tt);
		    tt = localtime(&m_CurrentDay);
		    strftime(date, 29, "%b '%Y", tt);
		    sprintf(month_str, "%s - %s", date, temp_date);
		    break;
		} else {
		    strftime(temp_date, 29, "%b '%Y", temp_tt);
		    tt = localtime(&m_CurrentDay);
		    strftime(date, 29, "%b", tt);
		    sprintf(month_str, "%s - %s", date, temp_date);
		    break;
		}
	    }
	}
    }

    // check backward from day
    tt = localtime(&m_CurrentDay);
    temp_tt = localtime(&m_CurrentDay);
    for (idx = wday; idx >= 0; idx--) {
	if (idx + 1 > wday)
	    continue;
	else {
	    temp_tt->tm_mday--;
	    new_time = mktime(temp_tt);
	    temp_tt = localtime(&new_time);
	    if (month != temp_tt->tm_mon) {
		if (week == 1 || week == 53) {
		    strftime(temp_date, 29, "%b '%Y", tt);
		    tt = localtime(&m_CurrentDay);
		    strftime(date, 29, "%b '%Y", tt);
		    sprintf(month_str, "%s - %s", temp_date, date);
		    break;
		} else {
		    strftime(temp_date, 29, "%b", temp_tt);
		    tt = localtime(&m_CurrentDay);
		    strftime(date, 29, "%b '%Y", tt);
		    sprintf(month_str, "%s - %s", temp_date, date);
		    break;
		}
	    }
	}
    }

    monYearBox->label(month_str);
    monYearBox->redraw();

    sprintf(week_num, "Week %d", week);
    weekBox->label(week_num);
    weekBox->redraw();

    UpdateWeekViewButtons();
}

void
NxSchedule::UpdateFromButton()
{
    static char buf[30];
    tm *tt = localtime(&fromTime);

    //strftime(buf, 29, "%b %d, %y", tt);
    GetDateString(buf, tt, sizeof(buf), SHORT_YEAR);
    fromDateButton->label(buf);
    fromDateButton->redraw();
}

void
NxSchedule::UpdateToButton()
{
    static char buf[30];
    tm *tt = localtime(&toTime);

    //strftime(buf, 29, "%b %d, %y", tt);
    GetDateString(buf, tt, sizeof(buf), SHORT_YEAR);

    toDateButton->label(buf);
    toDateButton->redraw();
}

NxSchedule::NxSchedule(int argc, char *argv[])
    :
NxApp()
{

    NxApp::Instance()->set_about(about_schedule);

    m_CurrentDay = time(0);

#ifdef DO_ALARM
    alarm_count_ = -1;
    memset(alarms, 0, sizeof(alarms));
#endif

    db = new NxDb(argc, argv);
    optind = 1;
    NxApp::Instance()->set_keyboard(argc, argv);

    g_EditFlag = 0;
    idNum = 0;

    // Open or Create schedule database
    if (!db->Open(SCHEDULE, &sFile, sFields, 0)) {

	if (db->Create(SCHEDULE, &sFile, sFields, 0)) {

	    if (!db->Open(SCHEDULE, &sFile, sFields, 0)) {
		exit(-1);
	    }

#ifdef CONFIG_SAMPLES
	    time_t t;
	    t = time(0);

	    char *record = 0;

	    record = Record(0,	// id
			    0,	// catid
			    t,	// startTime
			    t + 3600,	// endTime
			    0,	// allDayFlag 
			    0, 0, 0, 0,	// repeatWkMonflag
			    APPT,	// eventType
			    "Meeting with Gandalf.", 0, 0, 0, 0);

	    db->Insert(SCHEDULE, record);

#ifdef NOTUSED
	    put16(&record[sFields[0].offset], 1);
	    db->Insert(SCHEDULE, record);
	    put16(&record[sFields[0].offset], 2);
	    db->Insert(SCHEDULE, record);
	    put16(&record[sFields[0].offset], 3);
	    db->Insert(SCHEDULE, record);
	    put16(&record[sFields[0].offset], 4);
	    db->Insert(SCHEDULE, record);
	    put16(&record[sFields[0].offset], 5);
	    db->Insert(SCHEDULE, record);
	    put16(&record[sFields[0].offset], 6);
	    db->Insert(SCHEDULE, record);
	    put16(&record[sFields[0].offset], 7);
	    db->Insert(SCHEDULE, record);
	    put32(&record[sFields[2].offset], t + 86400);
	    put16(&record[sFields[0].offset], 8);
	    db->Insert(SCHEDULE, record);
	    put16(&record[sFields[0].offset], 9);
	    db->Insert(SCHEDULE, record);
	    put16(&record[sFields[0].offset], 10);
	    db->Insert(SCHEDULE, record);
#endif

	    delete[]record;
	    record = 0;
#endif
	} else {
	    exit(-1);
	}

    }
    // Database opened or created successfully, get number of records.
    idNum = GetKey(db, SCHEDULE, 0);

    g_SearchFlag = false;
    /*
       buttonGroup = new Fl_Group(BUTTON_X+180, 0, 50, BUTTON_HEIGHT, "");
       {
       dailyButton = new GroupButton(BUTTON_X+180, BUTTON_Y, 10, BUTTON_HEIGHT, type_daily);
       dailyButton->callback(dailyView_callback, this);
       buttonGroup->add((Fl_Widget*)dailyButton);
       }
       {
       weeklyButton = new GroupButton(BUTTON_X+193, BUTTON_Y, 10, BUTTON_HEIGHT, type_weekly);
       weeklyButton->callback(weeklyView_callback, this);
       buttonGroup->add((Fl_Widget*)weeklyButton);
       }
       {
       monthlyButton = new GroupButton(BUTTON_X+206, BUTTON_Y, 10, BUTTON_HEIGHT, type_monthly);
       monthlyButton->callback(monthlyView_callback, this);
       buttonGroup->add((Fl_Widget*)monthlyButton);
       }
       {
       yearlyButton = new GroupButton(BUTTON_X+219, BUTTON_Y, 10, BUTTON_HEIGHT, type_yearly);
       yearlyButton->callback(yearlyView_callback, this);
       buttonGroup->add((Fl_Widget*)yearlyButton);
       }
       buttonGroup->end();
     */
    mainWindow = new NxWindow(W_W, W_H, APP_NAME);

    MakeWeekViewWindow();
    MakeDayWindow();

    MakeTimeWindow();
    MakeDetailsWindow();
    MakeRepeatWindow();
    MakeMonthWindow();

    // for some reason this is created slow!
    //MakeMonthViewWindow();

    m_pCalendar->MakeDateWindow();
    dateWindow = m_pCalendar->GetDateWindow();
    add_window((Fl_Window *) dateWindow->GetWindowPtr());

    w_pCalendar->MakeDateWindow();
    weekDateWindow = w_pCalendar->GetDateWindow();
    add_window((Fl_Window *) weekDateWindow->GetWindowPtr());

    MakeYearViewWindow();
    MakeSetTimeWindow();
    MakeDeleteWindow();
    MakeLookupWindow();
    MakeResultsWindow();
    MakeErrorWindow();
    MakeRepeatEventWindow();

#ifdef DO_ALARM
    MakeAlarmViewWindow();
#endif

    mainWindow->end();

    //dayWindow->add((Fl_Widget*)buttonGroup);
    set_shown_window(dayWindow->GetWindowPtr());
    // FLNX-Colosseum IPC
    SyncRegisterDB(db, sFields, SCHEDULE, 0, NULL, 0);

#ifdef CONFIG_COLOSSEUM
    Add_Fd("nxschedule", _ClientIPCHandler);
#else
    ExecuteShow();
#endif

    MakeMonthViewWindow();
    mainWindow->add((Fl_Window *) monthViewWindow->GetWindowPtr());

    //show_default_window();
    UpdateDateDisplay();

}

NxSchedule::~NxSchedule()
{
    db->Close(SCHEDULE);
    delete db;
    db = 0;
}

void
NxSchedule::Refresh()
{
    UpdateDateDisplay();
}

void
NxSchedule::dailyView_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    if (!dayWindow->GetWindowPtr()->shown()) {
	//            dayWindow->add((Fl_Widget*)buttonGroup);
	pThis->show_window(dayWindow->GetWindowPtr());
    }
}

void
NxSchedule::weeklyView_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->UpdateWeekView();

    if (pThis->weekGrid) {
	pThis->weekGrid->hide();
	pThis->weekGrid->redraw();
	pThis->weekGrid->show();
    }
    if (!weekViewWindow->GetWindowPtr()->shown()) {
	//            weekViewWindow->add((Fl_Widget*)buttonGroup);
	pThis->show_window(weekViewWindow->GetWindowPtr());
    }

}

void
NxSchedule::monthlyView_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    if (!(pThis->monthViewWindow->GetWindowPtr()->shown())) {
	pThis->pMonthCal->SetPickedDate(pThis->m_CurrentDay);
	//              monthViewWindow->add((Fl_Widget*)buttonGroup);
	pThis->show_window(pThis->monthViewWindow->GetWindowPtr());
    }
}

void
NxSchedule::yearlyView_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    if (!(pThis->yearViewWindow->GetWindowPtr()->shown())) {
	//            yearViewWindow->add((Fl_Widget*)buttonGroup);
	pThis->show_window(pThis->yearViewWindow->GetWindowPtr());
    }
}

//
// FLNX-Colosseum IPC Methods
//

#ifdef CONFIG_COLOSSEUM
void
NxSchedule::ClientIPCHandler(int fd, void *o, int ipc_id)
{

    DPRINT("\n");
    DPRINT("ClientIPCHandler has now been started.\n");

    char *tokenMsg = new char[MAX_LENGTH];
    memset(tokenMsg, 0, MAX_LENGTH);
    char *passMsg = new char[MAX_LENGTH];
    memset(passMsg, 0, MAX_LENGTH);

    DPRINT("And this is the message... %s.\n", (char *) o);

    if (o == NULL) {

	int length = MAX_LENGTH - 1;
	ipc_id = NxApp::Instance()->Read_Fd(passMsg, &length);

	if ((passMsg == NULL) || (passMsg[0] == 0))
	    return;
	else
	    strcpy(tokenMsg, passMsg);
    } else if (ipc_id == -1) {

	strcpy(tokenMsg, (char *) o);
	strcpy(passMsg, (char *) o);
	ipc_id = NxApp::Instance()->Find_Fd("nxschedule");

    } else {

	strcpy(tokenMsg, (char *) o);
	strcpy(passMsg, (char *) o);

    }

    // Explode Message
    char *service = new char[MAX_LENGTH];
    char *msg_cmd = new char[MAX_LENGTH];
    char *data_item = new char[MAX_LENGTH];

    // SERVICE
    char *tmp = strtok(tokenMsg, TOKEN);
    if (NULL != tmp)
	strcpy(service, tmp);

    // MSG_CMD
    tmp = strtok(NULL, TOKEN);
    if (NULL != tmp)
	strcpy(msg_cmd, tmp);

    // DATA_ITEM
    tmp = strtok(NULL, TOKEN);
    if (NULL != tmp)
	strcpy(data_item, tmp);

    //  DPRINT("Expoding Message... %s, %s, %s.\n", service, msg_cmd, data_item);

#ifdef DO_ALARM
    if (strcmp(msg_cmd, "ALARM") == 0) {
	if (!NxApp::Instance()->VerifyClient(service))
	    return;

	int rec_array[1];

	rec_array[0] = -1;

	db->Select(SCHEDULE, data_item, 0, rec_array, 1);
	if (-1 != rec_array[0]) {
	    viewAlarm(rec_array[0]);
	}
    }
#endif

    if (strcmp(msg_cmd, "EXECUTE") == 0) {

	//    DPRINT("EXECUTE message command recv.\n");

	if (!NxApp::Instance()->VerifyClient(service))
	    return;

	if (strcmp(data_item, "search") == 0) {

	    DPRINT("%s, %s, %s\n", service, msg_cmd, data_item);

	    char *searchStr = new char[MAX_LENGTH];
	    char *startStr = new char[16];
	    char *endStr = new char[16];
	    char *widthStr = new char[8];

	    tmp = strtok(NULL, TOKEN);
	    if (NULL != tmp)
		strcpy(searchStr, tmp);
	    tmp = strtok(NULL, TOKEN);
	    if (NULL != tmp)
		strcpy(widthStr, tmp);
	    tmp = strtok(NULL, TOKEN);
	    if (NULL == tmp)
		ExecuteSearch(ipc_id, searchStr, atoi(widthStr));
	    else {
		strcpy(startStr, tmp);
		long startTime = strtol(startStr, NULL, 10);
		tmp = strtok(NULL, TOKEN);
		if (NULL != tmp)
		    strcpy(endStr, tmp);
		long endTime = strtol(endStr, NULL, 10);
		DPRINT("Executing StringDateSearch\n");
		ExecuteStringDateSearch(ipc_id, searchStr, atoi(widthStr),
					startTime, endTime);
	    }

	    delete[]startStr;
	    delete[]endStr;
	    delete[]searchStr;
	    delete[]widthStr;
	    searchStr = widthStr = startStr = endStr = 0;

	}

	if (0 == strcmp(data_item, "datesearch")) {
	    char *startStr = new char[16];
	    char *endStr = new char[16];
	    char *widthStr = new char[8];

	    tmp = strtok(NULL, TOKEN);
	    if (NULL != tmp)
		strcpy(widthStr, tmp);
	    tmp = strtok(NULL, TOKEN);
	    if (NULL != tmp)
		strcpy(startStr, tmp);
	    tmp = strtok(NULL, TOKEN);
	    if (NULL != tmp)
		strcpy(endStr, tmp);

	    long startTime = strtol(startStr, NULL, 10);
	    long endTime = strtol(endStr, NULL, 10);
	    ExecuteDateSearch(ipc_id, atoi(widthStr), startTime, endTime);

	    delete[]startStr;
	    delete[]endStr;
	    delete[]widthStr;

	    startStr = endStr = widthStr = 0;
	}

	if (strcmp(data_item, "showrecord") == 0) {
	    int recno = 0;
	    char *data = strtok(NULL, TOKEN);

	    recno = atoi(data);
	    viewRecord(recno);
	}
    }
    // Memory Mangement
    delete[]service;
    delete[]msg_cmd;
    delete[]data_item;
    delete[]tokenMsg;
    service = msg_cmd = data_item = tokenMsg = NULL;

    NxApp::Instance()->ServerIPCHandler(fd, ipc_id, (char *) passMsg);

    delete[]passMsg;
    passMsg = NULL;

}
#endif

time_t
GetAlarmTime(NxTodo * n)
{

    time_t alarmTime;

    switch (n->alarmFlags) {
    case ALARM_MIN:
	alarmTime = n->startTime - (60 * n->alarmInt);
	break;
    case ALARM_HOUR:
	alarmTime = n->startTime - (3600 * n->alarmInt);
	break;
    case ALARM_DAY:
	alarmTime = n->startTime - (86400 * n->alarmInt);
	break;
    default:
	alarmTime = n->startTime;
	break;
    }
    return alarmTime;
}

#ifdef DO_ALARM
void
NxSchedule::SetAlarm(NxTodo * n, time_t new_time)
{
    int ipc_id = NxApp::Instance()->Find_Fd("alarmd");
    time_t alarmTime = 0;
    DPRINT("SetAlarm ipc_id [%d]\n", ipc_id);

    if (0 == new_time) {
	alarmTime = GetAlarmTime(n);
	if (0 > alarmTime)
	    alarmTime = n->startTime;
    } else {
	alarmTime = new_time;
    }

    if (0 < ipc_id) {
	char *msg = new char[MAX_LENGTH];
	char buf[DESC];
	int recno = n->recno;

	sprintf(msg, "nxschedule^INITIATE^0^");
	NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);

	if (0 == strcmp(n->szDescription, ""))
	    strcpy(buf, " ");
	else
	    strcpy(buf, n->szDescription);

	if ((g_EditFlag & CHANGED_NEW))
	    recno = idNum + 1;

	sprintf(msg, "nxschedule^SET^%d^%s^%ld^%ld^%ld^",
		recno, buf, n->startTime, n->endTime, alarmTime);
	NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);
	delete[]msg;
	msg = 0;
    }
}

void
NxSchedule::DeleteAlarm(NxTodo * n)
{
    int ipc_id = NxApp::Instance()->Find_Fd("alarmd");

    if (0 < ipc_id) {
	char *msg = new char[MAX_LENGTH];
	int recno = n->recno;

	sprintf(msg, "nxschedule^INITIATE^0^");
	NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);

	sprintf(msg, "nxschedule^DELETE^%d^", recno);
	NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);
	delete[]msg;
	msg = 0;
    }
}

#endif

#ifdef CONFIG_COLOSSEUM

void
NxSchedule::ExecuteSearch(int ipc_id, char *searchStr, int width)
{

    if (searchStr == NULL)
	return;

    char *msg = new char[MAX_LENGTH];
    NxTodo *note;

    while ((note = searchString(searchStr)) != NULL) {

	char *result = formatString(note, width);

	// Send DATA message to client
	// (e.g. "nxschedule^DATA^1^Century *Software* is neato^")
	strcpy(msg, "nxschedule^DATA^search^");
	char recno[4];
	sprintf(recno, "%d", note->recno);
	strcat(msg, recno);
	strcat(msg, "^");
	strcat(msg, result);
	strcat(msg, "^");

	NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);

    }

    strcpy(msg, "nxschedule^ACK^DATA^search^");
    NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);

    delete[]msg;
    msg = NULL;

}

void
NxSchedule::ExecuteStringDateSearch(int ipc_id, char *searchStr, int width,
				    long startTime, long endTime)
{
    if (searchStr == NULL)
	return;

    char *msg = new char[MAX_LENGTH];
    NxTodo *note;

    while ((note = searchString(searchStr)) != NULL) {

	if (true == checkDate(note, (time_t) startTime, (time_t) endTime)) {

	    char *result = formatString(note, width);

	    // Send DATA message to client
	    strcpy(msg, "nxschedule^DATA^search^");
	    char recno[16];
	    sprintf(recno, "%d", note->recno);
	    strcat(msg, recno);
	    strcat(msg, "^");
	    strcat(msg, result);
	    strcat(msg, "^");

	    NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);
	}
    }

    strcpy(msg, "nxschedule^ACK^DATA^search^");
    NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);

    delete[]msg;
    msg = NULL;

}

void
NxSchedule::ExecuteDateSearch(int ipc_id, int width, long startTime,
			      long endTime)
{
    char *msg = new char[MAX_LENGTH];
    NxTodo *note;

    while ((note = searchDate(startTime, endTime)) != NULL) {
	char *result = formatString(note, width);

	// Send DATA message to client
	// (e.g. "nxschedule^DATA^1^Century *Software* is neato^")
	strcpy(msg, "nxschedule^DATA^search^");
	char recno[4];
	sprintf(recno, "%d", note->recno);
	strcat(msg, recno);
	strcat(msg, "^");
	strcat(msg, result);
	strcat(msg, "^");

	NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);

    }

    strcpy(msg, "nxschedule^ACK^DATA^search^");
    NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);

    delete[]msg;
    msg = NULL;
}
#endif

void
NxSchedule::exit_callback(Fl_Widget * fl, void *l)
{
    exit_flag = 1;
    mainWindow->hide();
}

//
// Database Methods
//

char *
NxSchedule::Record(int id, int catid, long startTime, long endTime,
		   int allDayFlag, int repeatFlag_1, int repeatFlag_2,
		   long repeatFlag_3, long repeatWkMonFlag,
		   entry_type entryType, char *szDescription, int except,
		   int recnoPtr, int alarmInt, int alarmFlags)
{

    char *rec = new char[MAXRECSIZ];

    if (id >= INT_MAX) {
	fprintf(stderr, "Fatal Error!!!!!\n");
	exit(1);
    }
    memset(rec, 0, MAXRECSIZ);

    put16(&rec[sFields[0].offset], id);
    put16(&rec[sFields[1].offset], catid);
    put32(&rec[sFields[2].offset], startTime);
    put32(&rec[sFields[3].offset], endTime);
    put16(&rec[sFields[4].offset], allDayFlag);
    put16(&rec[sFields[5].offset], repeatFlag_1);
    put16(&rec[sFields[6].offset], repeatFlag_2);
    put32(&rec[sFields[7].offset], repeatFlag_3);
    put32(&rec[sFields[8].offset], repeatWkMonFlag);
    put16(&rec[sFields[9].offset], (int) entryType);
    strcpy(&rec[sFields[10].offset], szDescription);
    put16(&rec[sFields[11].offset], except);
    put16(&rec[sFields[12].offset], recnoPtr);
    put16(&rec[sFields[13].offset], alarmInt);
    put16(&rec[sFields[14].offset], alarmFlags);

    return rec;

}

char *
NxSchedule::Record(int catid, string cat_name)
{

    char *rec = new char[MAXRECSIZ];

    memset(rec, 0, sizeof(rec));
    put16(&rec[catFields[0].offset], catid);
    strcpy(&rec[catFields[1].offset], cat_name.c_str());

    return rec;

}

//
// Window Methods
//

Fl_Window *
NxSchedule::get_main_window()
{
    if (mainWindow)
	return mainWindow;
    else
	return 0;
}

void
NxSchedule::show_default_window()
{
    //    dayWindow->add((Fl_Widget*)buttonGroup);
    show_window(dayWindow->GetWindowPtr());
}

#ifdef DO_ALARM

void
NxSchedule::MakeAlarmViewWindow()
{
    alarmViewWindow = new NxPimPopWindow("Alarm");
    add_window((Fl_Window *) alarmViewWindow->GetWindowPtr());
    {
	alarm_msg =
	    new NxBox(4, 19, alarmViewWindow->GetWindowPtr()->w() - 5, 25);
	alarm_msg->label("Jun 9, 1974 \n One Great Date");
	alarm_msg->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	alarmViewWindow->add((Fl_Widget *) alarm_msg);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Ok");
	o->callback(alarmOk_callback, this);
	alarmViewWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o = new NxButton(BUTTON_X + 5 + BUTTON_WIDTH, 90,
				   BUTTON_WIDTH, BUTTON_HEIGHT, "Snooze");
	o->callback(alarmSnooze_callback, this);
	alarmViewWindow->add((Fl_Widget *) o);
    }
}

void
NxSchedule::alarmSnooze_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    NxTodo *note = new NxTodo;
    time_t alarmTime = 0;

    DPRINT("alarmSnooze_callback alarm_count [%d]\n", alarm_count_);
    ExtractRecord(note, alarms[alarm_count_]);

    alarmTime = time(0);

    // snooze for five more minutes
    alarmTime += (60 * 5);
    pThis->SetAlarm(note, alarmTime);
    pThis->formatAlarmMsg(alarms[--alarm_count_]);

    if (0 > alarm_count_) {
	alarm_count_ = -1;
	pThis->show_window(dayWindow->GetWindowPtr());
    } else {
	pThis->show_window(alarmViewWindow->GetWindowPtr(),
			   DEACTIVATE, dayWindow->GetWindowPtr());
    }
}

void
NxSchedule::alarmOk_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    DPRINT("alarmOk_callback alarm_count [%d]\n", alarm_count_);

    pThis->formatAlarmMsg(alarms[--alarm_count_]);
    if (0 > alarm_count_) {
	alarm_count_ = -1;
	pThis->show_window(dayWindow->GetWindowPtr());
    } else {
	pThis->show_window(alarmViewWindow->GetWindowPtr(),
			   DEACTIVATE, dayWindow->GetWindowPtr());
    }
}

void
NxSchedule::formatAlarmMsg(int recno)
{
    NxTodo *note = new NxTodo;
    char buf[30];
    char tmp_buf[DESC];
    char desc_buf[DESC];

    DPRINT("formatAlarmMsg recno [%d]\n", recno);
    ExtractRecord(note, recno);

    struct tm *tt = localtime(&note->startTime);
    //strftime(buf, 29,"%b %d, %y", tt);
    GetDateString(buf, tt, sizeof(buf), SHORT_YEAR);

    int h = tt->tm_hour;
    int m = tt->tm_min;
    struct tm *tt2 = localtime(&note->endTime);


    sprintf(alarm_buf, "%s %d:%02d%s - %d:%02d%s\n", buf, HR_12(h), m,
	    AM_PM(h), HR_12(tt2->tm_hour), tt2->tm_min, AM_PM(tt2->tm_hour));

    int len = strlen(note->szDescription);
    int width = 0;
    int dot_width = 0;

    dot_width = (int) fl_width("...");

    width = (int) fl_width(note->szDescription);
    if (width <= alarm_msg->w()) {
	strcat(alarm_buf, note->szDescription);
    } else {
	strcpy(desc_buf, note->szDescription);
	for (int idx = 0; idx < len; idx++) {
	    if (desc_buf[idx] == '\n')
		desc_buf[idx] = ' ';
	    memset(tmp_buf, 0, sizeof(tmp_buf));
	    strncpy(tmp_buf, desc_buf, idx);
	    width = (int) fl_width(tmp_buf);
	    if (width + dot_width >= alarm_msg->w() - 5) {
		strcat(tmp_buf, "...");
		strcat(alarm_buf, tmp_buf);
		break;
	    }
	}
    }

    alarm_msg->label(alarm_buf);
    alarm_msg->hide();
    alarm_msg->show();
    delete note;
    note = 0;
}

void
NxSchedule::setNextAlarm(NxTodo * note)
{
    time_t today;
    time_t new_time;
    time_t nEarlyDay;
    time_t nLateDay;
    time_t alarm_time;
    int interval = 0;
    int days = 0;
    int weeks = 0;
    int mon = 0;
    int min = 0;
    int sec = 0;
    int hour = 0;
    tm *tt;
    bool found = false;

    today = time(NULL);

    // find next day that an alarm needs to be set
    // need to check for dummy and for end date
    switch (note->repeatFlag_1) {
    case REPEAT_DAILY:
	interval = note->repeatFlag_2;
	new_time = today + (interval * 86400);
	CreateDateRange(&nEarlyDay, &nLateDay, new_time, new_time);
	if (0 != note->repeatFlag_3) {
	    while (nEarlyDay <= note->repeatFlag_3) {
		if (IsForToday(note, nEarlyDay, nLateDay)) {
		    found = true;
		    break;
		} else {
		    new_time = new_time + (interval * 86400);
		    CreateDateRange(&nEarlyDay, &nLateDay, new_time,
				    new_time);
		}
	    }
	} else {
	    while (nLateDay <= LONG_MAX) {
		if (IsForToday(note, nEarlyDay, nLateDay)) {
		    found = true;
		    break;
		} else {
		    new_time = new_time + (interval * 86400);
		    CreateDateRange(&nEarlyDay, &nLateDay, new_time,
				    new_time);
		}
	    }
	}
	break;
    case REPEAT_WEEKLY:
	interval = note->repeatFlag_2;
	weeks += interval;

	tt = localtime(&today);
	tt->tm_mday -= tt->tm_wday;
	new_time = mktime(tt);
	new_time = new_time + (interval * 86400 * 7);

	days = 0;
	CreateDateRange(&nEarlyDay, &nLateDay, new_time, new_time);
	if (0 != note->repeatFlag_3) {
	    while (nEarlyDay <= note->repeatFlag_3) {
		if (IsForToday(note, nEarlyDay, nLateDay)) {
		    found = true;
		    break;
		} else {
		    if (6 == days) {	// go to next week interval
			new_time = new_time + (interval * 86400 * 7);
			days = 0;
			weeks += interval;
		    } else {
			new_time += 86400;
			days++;
		    }
		    CreateDateRange(&nEarlyDay, &nLateDay, new_time,
				    new_time);
		}
	    }
	} else {
	    while (nLateDay <= LONG_MAX) {
		if (IsForToday(note, nEarlyDay, nLateDay)) {
		    found = true;
		    break;
		} else {
		    if (6 == days) {	// go to next week interval
			new_time = new_time + (interval * 86400 * 7);
			days = 0;
			weeks += interval;
		    } else {
			new_time += 86400;
			days++;
		    }
		    CreateDateRange(&nEarlyDay, &nLateDay, new_time,
				    new_time);
		}
	    }
	}
	break;
    case REPEAT_MONTHLY:
	interval = note->repeatFlag_2;

	tt = localtime(&today);
	tt->tm_mday = 1;
	tt->tm_mon += interval;
	new_time = mktime(tt);
	mon = tt->tm_mon;

	CreateDateRange(&nEarlyDay, &nLateDay, new_time, new_time);

	if (0 != note->repeatFlag_3) {
	    while (nEarlyDay <= note->repeatFlag_3) {
		if (IsForToday(note, nEarlyDay, nLateDay)) {
		    found = true;
		    break;
		} else {
		    tt = localtime(&new_time);
		    if (tt->tm_mon != mon) {
			tt->tm_mon += interval;
			new_time = mktime(tt);
		    } else {
			new_time += 86400;
		    }
		    CreateDateRange(&nEarlyDay, &nLateDay, new_time,
				    new_time);
		}
	    }
	} else {
	    while (nLateDay <= LONG_MAX) {
		if (IsForToday(note, nEarlyDay, nLateDay)) {
		    found = true;
		    break;
		} else {
		    tt = localtime(&new_time);
		    if (tt->tm_mon != mon) {
			tt->tm_mon += interval;
			new_time = mktime(tt);
		    } else {
			new_time += 86400;
		    }
		    CreateDateRange(&nEarlyDay, &nLateDay, new_time,
				    new_time);
		}
	    }
	}
	break;
    case REPEAT_YEARLY:
	interval = note->repeatFlag_2;

	tt = localtime(&today);
	tt->tm_year += interval;
	new_time = mktime(tt);

	CreateDateRange(&nEarlyDay, &nLateDay, new_time, new_time);
	if (0 != note->repeatFlag_3) {
	    while (nEarlyDay <= note->repeatFlag_3) {
		if (IsForToday(note, nEarlyDay, nLateDay)) {
		    found = true;
		    break;
		} else {
		    tt = localtime(&new_time);
		    tt->tm_year += interval;
		    new_time = mktime(tt);
		    CreateDateRange(&nEarlyDay, &nLateDay, new_time,
				    new_time);
		}
	    }
	} else {
	    while (nLateDay <= LONG_MAX) {
		if (IsForToday(note, nEarlyDay, nLateDay)) {
		    found = true;
		    break;
		} else {
		    tt = localtime(&new_time);
		    tt->tm_year += interval;
		    new_time = mktime(tt);
		    CreateDateRange(&nEarlyDay, &nLateDay, new_time,
				    new_time);
		}
	    }
	}
	break;
    default:
	break;
    }

    if (found) {
	tt = localtime(&note->startTime);
	hour = tt->tm_hour;
	min = tt->tm_min;
	sec = tt->tm_sec;
	tt = localtime(&new_time);
	tt->tm_hour = hour;
	tt->tm_min = min;
	tt->tm_sec = sec;
	note->startTime = mktime(tt);;
	alarm_time = GetAlarmTime(note);
	SetAlarm(note, alarm_time);
    }
}

void
NxSchedule::playAlarm()
{
    pid_t childpid;
    db_handle *par_db = 0;
    char buf[255];
    int ret = 0;
    char *args[4];
    char *wave_path = "/usr/pixil/bin/waveplay";

    par_db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);
    if (!par_db) {
	printf("Error - Couldn't open the par database %s\n",
	       db_getDefaultDB());
	return;
    }

    ret =
	par_getAppPref(par_db, "nxschedule", "alarm", "alarm", buf,
		       sizeof(buf));
    if (0 > ret) {
	printf("Error - Couldn't get preference from par database\n");
	db_closeDB(par_db);
	return;
    }
    db_closeDB(par_db);
    DPRINT("buf is [%s]\n", buf);

    args[0] = wave_path;
    args[1] = buf;
    args[2] = "100";
    args[3] = NULL;
    if ((childpid = vfork()) == 0) {
	execv(wave_path, args);
    }
}

void
NxSchedule::viewAlarm(int recno)
{
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());
    db_handle *par_db = 0;
    char val;
    int ret;

    DPRINT("in View alarm\n");

    // if repeating event set next alarm
    NxTodo *note = new NxTodo;

    pThis->ExtractRecord(note, recno);
    if (REPEAT_NONE != note->repeatFlag_1) {
	pThis->setNextAlarm(note);
    }
    delete note;
    note = 0;

    if (MAX_ALARMS == pThis->alarm_count_) {
	return;
    }

    pThis->alarms[++alarm_count_] = recno;
    DPRINT("viewAlarm alarm_count_ [%d]\n", alarm_count_);
    pThis->formatAlarmMsg(recno);
    pThis->show_window(alarmViewWindow->GetWindowPtr(),
		       DEACTIVATE, dayWindow->GetWindowPtr());
    //play some musac!
    par_db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);
    if (!par_db) {
	printf("Error - Couldn't ope the par database %s\n",
	       db_getDefaultDB());
    } else {
	ret =
	    par_getGlobalPref(par_db, "alarms", "sound", PAR_BOOL, &val,
			      sizeof(val));
	if (0 > ret) {
	    printf("Error - Couldn't get preference from par database\n");
	    db_closeDB(par_db);
	} else {
	    if (val == 1)
		pThis->playAlarm();
	}
	db_closeDB(par_db);
    }
}
#endif /* DO_ALARM */

void
NxSchedule::viewRecord(int recno)
{

    NxTodo *note = new NxTodo;

    int rec_array[1];
    rec_array[0] = -1;

    char c_recno[16];

    sprintf(c_recno, "%d", recno);
    DPRINT("c_recno: [%s]\n", c_recno);

    db->Select(SCHEDULE, c_recno, 0, rec_array, 1);
    if (-1 != rec_array[0]) {
	recno = rec_array[0];
	note->recno = recno;

	((NxSchedule *) (NxApp::Instance()))->
	    set_date_picker(((NxSchedule *) (NxApp::Instance()))->
			    m_pCalendar);
	ExtractRecord(note, recno);
	NxApp::Instance()->show_window(detailsWindow->GetWindowPtr(),
				       DEACTIVATE, dayWindow->GetWindowPtr());

	((NxSchedule *) (NxApp::Instance()))->FillDetailForm(note,
							     DESC_UPDATE);
    }
}

void
NxSchedule::details_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    NxTodo *n = 0;

    if (g_SearchFlag) {
	n = (NxTodo *) results_table->selected();
	g_SearchFlag = false;
    } else
	n = (NxTodo *) table->selected();

    if (n) {
	n->fakeTime = pThis->m_CurrentDay;
	pThis->set_date_picker(m_pCalendar);
	pThis->FillDetailForm(n, DESC_UPDATE);
	pThis->detailsDeleteButton->activate();

	NxApp::Instance()->show_window(detailsWindow->GetWindowPtr(),
				       DEACTIVATE, dayWindow->GetWindowPtr());
    }
}

static void
_FillDefaults(NxTodo * n, time_t current_day)
{
    time_t t;
    tm *tt = localtime(&current_day);
    //tm * tt;
    memset(n, 0, sizeof(NxTodo));

    t = time(0);
    //tt = localtime(&t);

    tt->tm_hour = 8;
    tt->tm_min = 0;

    n->startTime = mktime(tt);

    tt->tm_hour = 18;
    tt->tm_min = 0;

    n->endTime = mktime(tt);

    n->fakeTime = current_day;

    n->alarmInt = NO_ALARM;
}

void
NxSchedule::view_callback(Fl_Widget * w, void *l)
{
    if (Fl::event_clicks()) {
	if ((Fl::event_x() >= table->x()
	     && Fl::event_x() <= (table->x() + table->w()))
	    && (Fl::event_y() >= table->y()
		&& Fl::event_y() <= (table->y() + table->h())))
	    details_callback(w, l);
    }
    Fl::event_clicks(0);
}

void
NxSchedule::new_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    NxTodo *n = new NxTodo;


    _FillDefaults(n, pThis->m_CurrentDay);

    if (n) {
	NxApp::Instance()->show_window(detailsWindow->GetWindowPtr(),
				       DEACTIVATE, dayWindow->GetWindowPtr());

	pThis->set_date_picker(m_pCalendar);
	g_EditFlag = CHANGED_NEW;
	pThis->detailsDeleteButton->deactivate();
	pThis->FillDetailForm(n, DESC_NEW);
    }
}

void
NxSchedule::mainDatePicked_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->dateWindow->GetWindowPtr()->hide();
    pThis->show_window(dayWindow->GetWindowPtr());

    if (pCalendar->GetPickedDate()) {
	pThis->m_CurrentDay = pCalendar->GetPickedDate();
	pThis->pCalendar->SetPickedDate(pThis->m_CurrentDay);
	pThis->UpdateDateDisplay();
    }
}


void
NxSchedule::goto_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->set_date_picker(m_pCalendar);

    pCalendar->SetPickedDate(pThis->m_CurrentDay);

    NxApp::Instance()->show_window(dateWindow->GetWindowPtr(),
				   DEACTIVATE, dayWindow->GetWindowPtr());

    pCalendar->DateCallback(mainDatePicked_callback);
}

// Advance the currnet day by a week
void
NxSchedule::adv_week_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    pThis->m_CurrentDay += (86400 * 7);
    pThis->UpdateDateDisplay();
}



// Move the current day back by a week
void
NxSchedule::bak_week_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    pThis->m_CurrentDay -= (86400 * 7);
    pThis->UpdateDateDisplay();
}

// Callback for the day buttons at the top (not the week arrows)
void
NxSchedule::new_day_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;
    int nDOW = -1;

    // Find which button made the callback. nDOW will become the
    // day of the week that was selected.
    for (int i = 0; i < 7; i++)
	if (pThis->m_DayButtons[i] == w) {
	    nDOW = i;
	    break;
	}
    // Just checking...
    if (nDOW == -1)
	return;

    tm *tt = localtime(&pThis->m_CurrentDay);

    // Calculate the new current day; subtract the old
    // "current day" day of week from the new day of week,
    // multiply by 86400, add it to the current day,
    // and bob's your uncle.
    pThis->m_CurrentDay += (86400 * (nDOW - tt->tm_wday));
    pThis->UpdateDateDisplay();
}

void
NxSchedule::MakeDayWindow()
{

    dayWindow = new NxPimWindow(APP_NAME, schedMenuItems, 0, "", SCHEDULE, 0);

    add_window((Fl_Window *) dayWindow->GetWindowPtr());


    {
	NxButton *o =
	    new NxButton(W_W - 3 - (9 * DAY_S), 7, DAY_S, DAY_S, "@<");
	o->movable(false);
	o->labeltype(FL_SYMBOL_LABEL);
	o->callback(bak_week_callback, this);
	o->box(FL_FLAT_BOX);
	dayWindow->add((Fl_Widget *) o);

	o = new NxButton(W_W - 3 - (DAY_S), 7, DAY_S, DAY_S, "@>");
	o->movable(false);
	o->labeltype(FL_SYMBOL_LABEL);
	o->callback(adv_week_callback, this);
	o->box(FL_FLAT_BOX);
	dayWindow->add((Fl_Widget *) o);

	char *_d[] = { "S", "M", "T", "W", "T", "F", "S" };
	for (int i = 7; i > 0; i--) {
	    NxButton *o =
		new NxButton(W_W - 3 - ((i + 1) * DAY_S), 7, DAY_S, DAY_S,
			     _d[7 - i]);
	    o->movable(false);
	    o->box(FL_FLAT_BOX);
	    m_DayButtons[7 - i] = o;
	    o->callback(new_day_callback, this);
	    o->align(FL_ALIGN_CENTER);
	    dayWindow->add((Fl_Widget *) o);
	}
    }
    {
	NxBox *o = new NxBox(-1, 30, W_W + 2, BUTTON_Y - 32);
	o->movable(false);
	o->box(FL_BORDER_BOX);
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	dayWindow->add((Fl_Widget *) o);

	{
	    table = new Flv_Table_Child(0, 31, W_W, BUTTON_Y - 40, 0, 25, 76);
	    table->movable(false);
	    table->SetCols(COLS);
	    table->callback(view_callback, this);
	    dayWindow->add((Fl_Widget *) table);
	}

    }

    {
	NxBox *o = new NxBox(5, 5, 85, 25, "");
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	o->labelfont(1);
	dayWindow->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH - 5, BUTTON_HEIGHT,
			 "New");
	o->callback(new_callback, this);
	dayWindow->add((Fl_Widget *) o);
    }

    {
	NxButton *o = new NxButton(BUTTON_X + 58, BUTTON_Y, BUTTON_WIDTH - 5,
				   BUTTON_HEIGHT, "Details");
	o->callback(details_callback, this);
	dayWindow->add((Fl_Widget *) o);
    }

    {
	NxButton *o = new NxButton(BUTTON_X + 116, BUTTON_Y, BUTTON_WIDTH - 5,
				   BUTTON_HEIGHT, "Goto");
	o->callback(goto_callback, this);
	dayWindow->add((Fl_Widget *) o);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 180, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_daily);
	o->callback(dailyView_callback, this);
	dayWindow->add((Fl_Widget *) o);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 193, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_weekly);
	o->callback(weeklyView_callback, this);
	dayWindow->add((Fl_Widget *) o);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 206, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_monthly);
	o->callback(monthlyView_callback, this);
	dayWindow->add((Fl_Widget *) o);
    }

    {
	GroupButton *o =
	    new GroupButton(BUTTON_X + 219, BUTTON_Y, 10, BUTTON_HEIGHT,
			    type_yearly);
	o->callback(yearlyView_callback, this);
	dayWindow->add((Fl_Widget *) o);
    }

    dayWindow->GetWindowPtr()->end();

    //UpdateDateDisplay();
}

void
NxSchedule::MakeSetTimeWindow()
{

    setTimeWindow = new NxPimWindow(APP_NAME, schedMenuItems, db, CATEGORY,
				    SCHEDULE, (void (*)(const char *)) 0);
    add_window((Fl_Window *) setTimeWindow->GetWindowPtr());

    {

	NxScroll *hourScroll = new NxScroll(0, 31, W_W, BUTTON_Y - 38);
	setTimeWindow->add((Fl_Widget *) hourScroll);
    }

    {
	NxButton *o = new NxButton(4, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Ok");
	//    o->callback(doneDetails_callback);
	setTimeWindow->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(64, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Cancel");
	o->box(FL_SHADOW_BOX);
	//    o->callback(cancelDetails_callback);
	setTimeWindow->add((Fl_Widget *) o);
    }

    setTimeWindow->GetWindowPtr()->end();

}


void
NxSchedule::yesDelEdit_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

#ifdef DO_ALARM
    // delete the alarm for this record
    pThis->DeleteAlarm(pThis->m_pCurrentItem);
#endif

    delete_note(pThis->m_pCurrentItem);
    pThis->show_window(dayWindow->GetWindowPtr());
    pThis->UpdateDateDisplay();

}

void
NxSchedule::noDelEdit_callback(Fl_Widget * w, void *l)
{
    NxSchedule *pThis = (NxSchedule *) l;

    pThis->show_window(detailsWindow->GetWindowPtr());
    // this is needed in order for the day window widgets not to be hidden
    dayWindow->GetWindowPtr()->show();
    detailsWindow->GetWindowPtr()->show();
}

void
NxSchedule::MakeDeleteWindow()
{

    deleteWindow = new NxPimPopWindow("Delete");
    add_window((Fl_Window *) deleteWindow->GetWindowPtr());

    {
	NxBox *o = new NxBox(BUTTON_X, 43, W_W - BUTTON_X - 15, 0,
			     "Delete current event ?");
	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_WRAP | FL_ALIGN_TOP | FL_ALIGN_LEFT);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	deleteWindow->add((Fl_Widget *) o);
    }


    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Yes");
	o->callback(yesDelEdit_callback, this);
	deleteWindow->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 61, 90, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "No");
	o->callback(noDelEdit_callback, this);
	deleteWindow->add((Fl_Widget *) o);
    }

    deleteWindow->GetWindowPtr()->end();

}

void
NxSchedule::MakeLookupWindow()
{

    static char fromBuf[30];
    static char toBuf[30];

    toTime = fromTime = time(0);
    tm *tt = localtime(&fromTime);

    //strftime(fromBuf, 29,"%b %d, %y", tt);
    GetDateString(fromBuf, tt, sizeof(fromBuf), SHORT_YEAR);
    //strftime(toBuf, 29,"%b %d, %y", tt);
    GetDateString(toBuf, tt, sizeof(toBuf), SHORT_YEAR);

    lookupWindow = new NxPimWindow(W_X, W_Y, W_W, W_H);
    add_window((Fl_Window *) lookupWindow->GetWindowPtr());
    {
	NxCheckButton *o = stringCheck =
	    new NxCheckButton(BUTTON_X, 35, "Only entries containing:");
	o->movable(false);
	lookupWindow->add((Fl_Widget *) o);
    }
    {
	NxInput *o = lookup_input = new NxInput(BUTTON_X + 19, 60, 141, 20);
	o->movable(false);
	lookup_input->maximum_size(99);
	lookupWindow->add((Fl_Widget *) o);
    }
    {
	NxCheckButton *o = dateCheck =
	    new NxCheckButton(BUTTON_X, 85, "Limit by date range:");
	o->movable(false);
	lookupWindow->add((Fl_Widget *) o);
    }
    {
	NxOutput *o = new NxOutput(BUTTON_X + 19, 110, 60, BUTTON_HEIGHT);
	o->value("From:");
	o->movable(false);
	lookupWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o = fromDateButton =
	    new NxButton(BUTTON_X + 60, 110, 100, BUTTON_HEIGHT);
	o->movable(false);
	o->label(fromBuf);
	o->callback(fromCalendar_callback, this);
	o->redraw();
	lookupWindow->add((Fl_Widget *) o);
    }
    {
	NxOutput *o = new NxOutput(BUTTON_X + 19, 135, 60, BUTTON_HEIGHT);
	o->value("To:");
	o->movable(false);
	lookupWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o = toDateButton =
	    new NxButton(BUTTON_X + 60, 135, 100, BUTTON_HEIGHT);
	o->label(toBuf);
	o->movable(false);
	o->redraw();
	o->callback(toCalendar_callback, this);
	lookupWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Search");
	o->callback(searchLookup_callback, this);
	lookupWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X + 61, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Cancel");
	o->callback(cancelLookup_callback);
	lookupWindow->add((Fl_Widget *) o);
    }
}

void
NxSchedule::fromDate_callback(Fl_Widget * fl, void *l)
{

    NxSchedule *pThis = (NxSchedule *) l;
    pThis->dateWindow->GetWindowPtr()->hide();
    pThis->show_window(lookupWindow->GetWindowPtr());

    if (pCalendar->GetPickedDate()) {
	pThis->fromTime = pCalendar->GetPickedDate();
	pThis->UpdateFromButton();
    }
}

void
NxSchedule::toDate_callback(Fl_Widget * fl, void *l)
{

    NxSchedule *pThis = (NxSchedule *) l;
    pThis->dateWindow->GetWindowPtr()->hide();
    pThis->show_window(lookupWindow->GetWindowPtr());

    if (pCalendar->GetPickedDate()) {
	pThis->toTime = pCalendar->GetPickedDate();
	pThis->UpdateToButton();
    }
}

void
NxSchedule::fromCalendar_callback(Fl_Widget * fl, void *l)
{

    NxSchedule *pThis = (NxSchedule *) l;

    pThis->set_date_picker(m_pCalendar);
    pCalendar->SetPickedDate(pThis->fromTime);
    NxApp::Instance()->show_window(dateWindow->GetWindowPtr(),
				   DEACTIVATE, lookupWindow->GetWindowPtr());
    pCalendar->DateCallback(fromDate_callback);
}

void
NxSchedule::toCalendar_callback(Fl_Widget * fl, void *l)
{

    NxSchedule *pThis = (NxSchedule *) l;

    pThis->set_date_picker(m_pCalendar);
    pCalendar->SetPickedDate(pThis->fromTime);
    NxApp::Instance()->show_window(dateWindow->GetWindowPtr(),
				   DEACTIVATE, lookupWindow->GetWindowPtr());
    pCalendar->DateCallback(toDate_callback);
}

char *
NxSchedule::formatString(const NxTodo * note, int pixels)
{
    int width = 0;
    int dot_width = 0;
    int idx = 0;
    unsigned int jdx = 0;
    int date_len = 0;
    static char temp_date[33];
    static char date[33];
    char temp_title[DESC + 3];
    char title[DESC + 3];
    char *new_string = new char[DESC + 33];
    char temp_string[DESC + 33];
    tm *tt = localtime(&note->startTime);
    NxSchedule *pThis = (NxSchedule *) (NxApp::Instance());

    fl_font(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);
    dot_width = (int) fl_width("...");

    memset(temp_title, 0, sizeof(temp_title));
    memset(title, 0, sizeof(title));
    memset(new_string, 0, DESC + 33);

    strcpy(temp_title, note->szDescription);
    for (jdx = 0; jdx <= strlen(temp_title); jdx++) {
	if ('\n' == temp_title[jdx])
	    temp_title[jdx] = ' ';
    }

    // format description to fit half of width
    idx = 0;
    strcpy(title, temp_title);
    width = (int) fl_width(title);
    if (width >= (pixels / 2)) {
	while (width > (pixels / 2)) {
	    idx++;
	    memset(title, 0, sizeof(title));
	    strncpy(title, temp_title, strlen(temp_title) - idx);
	    width = (int) fl_width(title) + dot_width;
	}
	sprintf(title, "%s...", title);
    } else {
	while (width < (pixels / 2)) {
	    if (strlen(title) >= DESC + 3)
		break;
	    sprintf(title, "%s ", title);
	    width = (int) fl_width(title);
	}
    }

    //strftime(date,29,"%b %d, %y",tt);
    pThis->GetDateString(date, tt, sizeof(date), SHORT_YEAR);
    date_len = strlen(date);
    memset(temp_date, 0, sizeof(temp_date));

    width = (int) fl_width(date);
    if (width >= (pixels / 2)) {
	sprintf(temp_date, "%*.*s", 30, date_len, date);
	strcpy(date, temp_date);
    }
    //strftime(temp_date,29,"%b %d, %y",tt);
    pThis->GetDateString(temp_date, tt, sizeof(temp_date), SHORT_YEAR);

    // format date to fit half ot the input left justified
    idx = 0;
    jdx = 0;
    width = (int) fl_width(date);
    while (width >= (pixels / 2)) {
	if (isspace(date[0])) {
	    idx++;
	    memmove(date, date + 1, sizeof(date) - idx);
	    width = (int) fl_width(date);
	} else {
	    jdx++;
	    memset(date, 0, sizeof(date));
	    strncpy(date, temp_date, strlen(temp_date) - jdx);
	    width = (int) fl_width(date) + dot_width;
	}
    }

    if (0 != jdx)
	sprintf(date, "%s...", date);

    sprintf(new_string, "%s%s", title, date);

    // now align the second string to middle pixel  
    char *pStr = strstr(new_string, date);
    int len = strlen(new_string) - strlen(pStr);

    memset(temp_string, 0, sizeof(temp_string));
    strncpy(temp_string, new_string, len);
    width = (int) fl_width(temp_string);
    while (width <= (pixels / 2 - 1)) {
	sprintf(temp_string, "%s ", temp_string);
	width = (int) fl_width(temp_string);
    }
    sprintf(new_string, "%s%s", temp_string, date);

    return (char *) new_string;
}


NxTodo *
NxSchedule::searchString(const char *searchVal)
{
    static int cur_record = 0;
    static int rec_array[255];
    int jdx;
    char *needle = strup(searchVal, strlen(searchVal));

    if (0 == cur_record) {
	for (int idx = 0; idx < 255; idx++) {
	    rec_array[idx] = -1;
	}
	db->Select(SCHEDULE, rec_array, 255);
    }

    if (255 == cur_record) {
	cur_record = 0;
	delete[]needle;
	needle = 0;
	return NULL;
    }

    bool found = false;
    NxTodo *note = new NxTodo;

    while (cur_record < 255) {
	found = false;

	jdx = rec_array[cur_record];
	if (-1 == jdx) {
	    cur_record++;
	    continue;
	}

	db->Extract(SCHEDULE, jdx, 10, note->szDescription);
	char *haystack = strup(note->szDescription, DESC);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	if (true == found) {
	    ExtractRecord(note, jdx);
	    cur_record++;
	    delete[]needle;
	    needle = 0;
	    return note;
	} else {
	    cur_record++;
	    continue;
	}
    }
    cur_record = 0;
    delete[]note;
    note = 0;
    delete[]needle;
    needle = 0;
    return NULL;
}

bool NxSchedule::checkDate(NxTodo * note, time_t fromTime, time_t toTime)
{
    DPRINT("In check date: fromTime[%ld], toTime[%ld\n", fromTime, toTime);
    DPRINT("note startTime: [%ld] endTime: [%ld\n", note->startTime,
	   note->endTime);

    struct tm *
	from_tm =
	localtime(&fromTime);
    int
	fd =
	from_tm->
	tm_mday;
    int
	fm =
	from_tm->
	tm_mon;
    int
	fy =
	from_tm->
	tm_year;
    struct tm *
	to_tm =
	localtime(&toTime);
    int
	td =
	to_tm->
	tm_mday;
    int
	tm =
	to_tm->
	tm_mon;
    int
	ty =
	to_tm->
	tm_year;

    struct tm *
	tt =
	localtime(&note->startTime);
    int
	d =
	tt->
	tm_mday;
    int
	m =
	tt->
	tm_mon;
    int
	y =
	tt->
	tm_year;
    struct tm *
	tt2 =
	localtime(&note->endTime);
    int
	d2 =
	tt2->
	tm_mday;
    int
	m2 =
	tt2->
	tm_mon;
    int
	y2 =
	tt2->
	tm_year;
    if (((fd <= d) && (fm <= m) && (fy <= y))
	&& ((td >= d2) && (tm >= m2) && (ty >= y2))) {
	return true;
    }

    return false;
}

NxTodo *
NxSchedule::searchDate(time_t fromTime, time_t toTime)
{
    static int cur_record = 0;
    static int rec_array[255];
    int jdx;

    if (0 == cur_record) {
	for (int idx = 0; idx < 255; idx++) {
	    rec_array[idx] = -1;
	}
	db->Select(SCHEDULE, rec_array, 255);
    }

    if (255 == cur_record) {
	cur_record = 0;
	return NULL;
    }

    NxTodo *note = new NxTodo;
    while (cur_record < 255) {
	jdx = rec_array[cur_record];
	if (-1 == jdx) {
	    cur_record++;
	    continue;
	}

	ExtractRecord(note, jdx);

	if (true == checkDate(note, fromTime, toTime)) {
	    cur_record++;
	    return note;
	} else {
	    cur_record++;
	    continue;
	}
    }
    cur_record = 0;
    delete[]note;
    note = 0;
    return NULL;
}

void
NxSchedule::searchLookup_callback(Fl_Widget * fl, void *l)
{

    if ((stringCheck->value() == 0) && (dateCheck->value() == 0)) {
	NxApp::Instance()->show_window(errorWindow->GetWindowPtr(),
				       DEACTIVATE,
				       lookupWindow->GetWindowPtr());
	return;
    }

    g_SearchFlag = true;

    int total_found = 0;
    char *needle = 0;
    NxTodo *note = 0;

    results_table->Init(255);

    if (stringCheck->value() == true && dateCheck->value() == false) {
	char *searchVal = (char *) lookup_input->value();
	needle = strup(searchVal, strlen(searchVal));
	while ((note = searchString(searchVal)) != NULL) {
	    results_table->Add(total_found, note);
	    char *label = formatString(note, results_table->text_width());
	    results_table->set_value(total_found, 0, label);
	    total_found++;
	}
    }

    if (stringCheck->value() == false && dateCheck->value() == true) {
	while ((note = searchDate(fromTime, toTime)) != NULL) {
	    results_table->Add(total_found, note);
	    char *label = formatString(note, results_table->text_width());
	    results_table->set_value(total_found, 0, label);
	    total_found++;
	}
    }

    if (stringCheck->value() == true && dateCheck->value() == true) {
	char *searchVal = (char *) lookup_input->value();
	needle = strup(searchVal, strlen(searchVal));
	while ((note = searchString(searchVal)) != NULL) {

	    if (true == checkDate(note, fromTime, toTime)) {
		results_table->Add(total_found, note);
		char *label = formatString(note, results_table->text_width());
		results_table->set_value(total_found, 0, label);
		total_found++;
	    }
	}
    }

    delete[]needle;
    needle = 0;

    if (total_found == 0) {
	results_message->show();
	results_table->hide();
    } else {
	results_message->hide();
	results_table->show();
    }

    results_table->rows(total_found);

    NxApp::Instance()->show_window(dayWindow->GetWindowPtr());
    NxApp::Instance()->show_window(resultsWindow->GetWindowPtr(),
				   DEACTIVATE, dayWindow->GetWindowPtr());

}

void
NxSchedule::cancelLookup_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(dayWindow->GetWindowPtr(), ACTIVATE);
}

void
NxSchedule::MakeResultsWindow()
{

    resultsWindow =
	new NxPimPopWindow("Search Results",
			   NxApp::Instance()->getGlobalColor(APP_FG), 5,
			   (W_W / 3), W_W - 10, (W_H - (W_W / 2)));
    add_window((Fl_Window *) resultsWindow->GetWindowPtr());
    {
	results_message = new NxOutput(4, 19, W_W - 19, 25);
	results_message->value("Nothing Found.");
	results_message->hide();
	resultsWindow->add((Fl_Widget *) results_message);
    }
    {
	results_table =
	    new Flv_Table_Child(4, 19, (W_W - 19),
				(W_H - (W_W / 2) - 3 * (BUTTON_HEIGHT)), 0,
				(W_W - 25));
	results_table->callback(resultsView_callback, this);
	results_table->SetCols(1);
	resultsWindow->add((Fl_Widget *) results_table);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X, (W_H - (W_W / 2) - BUTTON_HEIGHT - 9),
			 BUTTON_WIDTH, BUTTON_HEIGHT, "Done");
	o->callback(doneLookup_callback);
	resultsWindow->add((Fl_Widget *) o);
    }
}

void
NxSchedule::doneLookup_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(dayWindow->GetWindowPtr(), ACTIVATE);
}

void
NxSchedule::resultsView_callback(Fl_Widget * fl, void *l)
{
    if (Fl::event_clicks()) {
	resultsWindow->GetWindowPtr()->hide();
	details_callback(fl, l);
    }
    Fl::event_clicks(0);
}

void
NxSchedule::MakeErrorWindow()
{
    errorWindow = new NxPimPopWindow("Error");
    add_window((Fl_Window *) errorWindow->GetWindowPtr());
    {
	error_msg =
	    new NxOutput(4, 19, errorWindow->GetWindowPtr()->w() - 5, 25);
	error_msg->value("Error: No Search Constraint.");
	errorWindow->add((Fl_Widget *) error_msg);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Ok");
	o->callback(errorOk_callback);
	errorWindow->add((Fl_Widget *) o);
    }
}

void
NxSchedule::errorOk_callback(Fl_Widget * fl, void *l)
{
    errorWindow->GetWindowPtr()->hide();
    NxApp::Instance()->show_window(lookupWindow->GetWindowPtr(),
				   DEACTIVATE, dayWindow->GetWindowPtr());
}

void
NxSchedule::edit_note(NxTodo * note, int recno)
{

    char *record = Record(note->recno, 0, note->startTime, note->endTime,
			  note->allDayFlag, note->repeatFlag_1,
			  note->repeatFlag_2,
			  note->repeatFlag_3, note->repeatWkMonFlag,
			  note->entryType, note->szDescription,
			  note->exception,
			  note->recnoPtr, note->alarmInt, note->alarmFlags);

    db->Edit(SCHEDULE, recno, record);

    delete[]record;
    record = 0;

}

void
NxSchedule::delete_note(NxTodo * note)
{
    int ret[1];
    char c_recno[8];

    sprintf(c_recno, "%d", note->recno);
    ret[0] = -1;
    db->Select(SCHEDULE, c_recno, 0, ret, 1);
    if (-1 != ret[0]) {
	db->DeleteRec(SCHEDULE, ret[0]);
    }
}

void
NxSchedule::save(NxTodo * note)
{
    int ret[1];
    char c_recno[8];

    ret[0] = -1;
    sprintf(c_recno, "%d", note->recno);
    db->Select(SCHEDULE, c_recno, 0, ret, 1);
    if (-1 != ret[0]) {
	edit_note(note, ret[0]);
    } else {
	write_note(note);
	delete note;
    }
}

void
NxSchedule::write_note(NxTodo * note)
{
    idNum = NxApp::Instance()->GetKey(db, SCHEDULE, 0) + 1;
    char *record = Record(idNum, 0, note->startTime, note->endTime,
			  note->allDayFlag, note->repeatFlag_1,
			  note->repeatFlag_2,
			  note->repeatFlag_3, note->repeatWkMonFlag,
			  note->entryType, note->szDescription,
			  note->exception, note->recnoPtr, note->alarmInt,
			  note->alarmFlags);

    db->Insert(SCHEDULE, record);

    delete[]record;
    record = 0;

}


void
NxSchedule::ExtractRecord(NxTodo * note, int nRecId)
{

    //note->recno = nRecId;

    char buf[16];

    db->Extract(SCHEDULE, nRecId, 0, buf);
    note->recno = atoi(buf);

    db->Extract(SCHEDULE, nRecId, 1, buf);

    // startHour
    db->Extract(SCHEDULE, nRecId, 2, buf);
    note->startTime = strtol(buf, NULL, 10);

    // endHour
    db->Extract(SCHEDULE, nRecId, 3, buf);
    note->endTime = strtol(buf, NULL, 10);

    // allDayFlag
    db->Extract(SCHEDULE, nRecId, 4, buf);
    note->allDayFlag = atoi(buf);

    // repeatFlag #1
    db->Extract(SCHEDULE, nRecId, 5, buf);
    note->repeatFlag_1 = atoi(buf);

    // repeatFlag #2
    db->Extract(SCHEDULE, nRecId, 6, buf);
    note->repeatFlag_2 = atoi(buf);

    // repeatFlag #3
    db->Extract(SCHEDULE, nRecId, 7, buf);
    note->repeatFlag_3 = strtol(buf, NULL, 10);

    // repeatWkMonFlag
    db->Extract(SCHEDULE, nRecId, 8, buf);
    note->repeatWkMonFlag = strtol(buf, NULL, 10);

    // entryType
    db->Extract(SCHEDULE, nRecId, 9, buf);
    note->entryType = (entry_type) atoi(buf);

    // szDescription
    db->Extract(SCHEDULE, nRecId, 10, note->szDescription);

    // exception
    db->Extract(SCHEDULE, nRecId, 11, buf);
    note->exception = atoi(buf);

    // recnoPtr
    db->Extract(SCHEDULE, nRecId, 12, buf);
    note->recnoPtr = atoi(buf);

    // alarm time
    db->Extract(SCHEDULE, nRecId, 13, buf);
    note->alarmInt = strtol(buf, NULL, 10);

    // alarm flags
    db->Extract(SCHEDULE, nRecId, 14, buf);
    note->alarmFlags = atoi(buf);
}

void
NxSchedule::add_item(int i, Flv_Table_Child * t, NxTodo * note)
{
    char buf[30];

    // Add node to tree .. This allows the later retrieval of
    // the note pointer.
    t->Add(i, note);

    // Add Table Cell
    struct tm *tt = localtime(&note->startTime);
    int h = tt->tm_hour;
    int m = tt->tm_min;
    struct tm *tt2 = localtime(&note->endTime);

    sprintf(buf, "%d:%02d%s -\n%d:%02d%s", HR_12(h), m, AM_PM(h),
	    HR_12(tt2->tm_hour), tt2->tm_min, AM_PM(tt2->tm_hour));

    // This is the time range for the schedule item
    t->set_value(i, 0, buf);
    // This is the description for the schedule item
    t->set_value(i, 1, note->szDescription);

    // make sure that this description is aligned left and wrapped
    t->col_style[1].align((Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_WRAP));

    // Calculate the needed height of the inserted item
    int wi = t->col_width(1), hi = 0;
    fl_measure(note->szDescription, wi, hi);

    int _wi = t->col_width(0), _hi = 0;
    fl_measure(buf, _wi, _hi);

    // Set the desired height for the row
    t->row_height(hi > _hi ? hi : _hi, i);
}

// add items to the tree view
void
NxSchedule::add_items(Flv_Table_Child * t)
{
    // Fix me!
    NxTodo *note;
    int rec_array[255];
    int idx = 0;
    int num_recs = 0;

    t->row(0);

    //for (idx = 0; idx < 255; idx++) {
    //  rec_array[idx] = -1;
    //}
    memset(rec_array, -1, sizeof(rec_array));

    num_recs = db->Select(SCHEDULE, rec_array, 255);

    int cnt = 0;
    for (idx = 0; idx < num_recs; idx++) {
	if (-1 != rec_array[idx]) {
	    note = new NxTodo;
	    ExtractRecord(note, rec_array[idx]);
	    if (IsForToday(note))
		cnt++;
	    delete note;
	}
    }

    NxTodo *notes[cnt];
    for (idx = 0; idx < cnt; idx++)
	notes[idx] = new NxTodo;

    int num = 0;
    for (idx = 0; idx < num_recs; idx++) {
	if (-1 != rec_array[idx]) {
	    note = new NxTodo;
	    ExtractRecord(note, rec_array[idx]);
	    if (IsForToday(note)) {
		notes[num] = note;
		num++;
	    } else
		delete note;
	}
    }

    t->Init(cnt);

    qsort(notes, cnt, sizeof(NxTodo *),
	  (int (*)(const void *, const void *)) compar);

    int lineCount = 0;
    for (idx = 0; idx < cnt; idx++) {
	t->rows(lineCount + 1);
	add_item(idx, t, notes[idx]);
	lineCount++;
    }
    t->rows(cnt);
}

int
NxSchedule::compar(NxTodo ** rec1, NxTodo ** rec2)
{

    NxTodo *record1 = *rec1;
    NxTodo *record2 = *rec2;

    if (record1->startTime > record2->startTime)
	return 1;
    else if (record1->startTime < record2->startTime)
	return -1;
    else {
	if (record1->endTime == record2->endTime)
	    return 0;
	else if (record1->endTime < record2->endTime)
	    return -1;
	else {
	    return 1;
	}
    }
    return 0;
}

void
NxSchedule::CreateDateRange(time_t * e, time_t * l, time_t start_day,
			    time_t end_day)
{
    tm start_tm;
    tm end_tm;

    memcpy(&start_tm, localtime(&start_day), sizeof(start_tm));
    memcpy(&end_tm, localtime(&end_day), sizeof(end_tm));

    start_tm.tm_sec = 0;
    start_tm.tm_min = 0;
    start_tm.tm_hour = 0;

    end_tm.tm_sec = 59;
    end_tm.tm_min = 59;
    end_tm.tm_hour = 23;

    *e = mktime(&start_tm);
    *l = mktime(&end_tm);
}

// Return true if this item is to be displayed for a given day.
bool NxSchedule::IsForToday(NxTodo * note)
{
    time_t
	nEarlyDay;
    time_t
	nLateDay;

    CreateDateRange(&nEarlyDay, &nLateDay, m_CurrentDay, m_CurrentDay);

    return IsForToday(note, nEarlyDay, nLateDay);
}

int
NxSchedule::MonthDiff(time_t time1, time_t time2)
{
    tm *tt;
    int year1;
    int year2;
    int mon1;
    int mon2;
    int year_swap;
    int time_swap;
    int year_diff;

    tt = localtime(&time1);
    year1 = tt->tm_year;

    tt = localtime(&time2);
    year2 = tt->tm_year;

    if (year1 > year2) {
	year_swap = year2;
	year2 = year1;
	year1 = year_swap;

	time_swap = time2;
	time2 = time1;
	time1 = time_swap;
    }

    tt = localtime(&time1);
    year1 = tt->tm_year;
    mon1 = tt->tm_mon;

    tt = localtime(&time2);
    year2 = tt->tm_year;
    mon2 = tt->tm_mon;

    if (year1 == year2)
	return mon2 - mon1;
    else {
	year_diff = year2 - year1;
	if (1 == year_diff) {
	    return (11 - mon1 + mon2);
	}
	return (((year_diff * 12)) + (11 - mon1) + mon2);
    }

    return 0;
}

bool NxSchedule::IsForToday(NxTodo * note, time_t nEarlyDay, time_t nLateDay)
{
    long
	repeat_val =
	note->
	repeatFlag_2;
    tm *
	today =
	NULL;
    float
	diff =
	0;
    time_t
	app_time =
	0;
    int
	app_day =
	0;
    int
	app_wday =
	0;
    int
	app_mon =
	0;
    int
	app_year =
	0;
    int
	app_weeks =
	0;
    int
	today_day =
	0;
    int
	today_mon =
	0;
    int
	today_wday =
	0;
    int
	today_year =
	0;
    int
	today_weeks =
	0;
    long
	days =
	0;
    long
	weeks =
	0;
    int
	months =
	0;
    int
	years =
	0;
    int
	mod =
	0;
    int
	rec_array[255];
    int
	idx =
	0;
    int
	num_recs =
	0;
    NxTodo *
	temp_note;


    if (note->exception & REPEAT_DELETED)
	return false;

    // only for repeating events
    // check for exeception
    if (REPEAT_NONE != note->repeatFlag_1) {
	// check the end date
	if (0 != note->repeatFlag_3) {
	    if (note->repeatFlag_3 < nEarlyDay) {
		return false;
	    }
	}
	memset(rec_array, -1, sizeof(rec_array));
	//for(idx = 0; idx < 255; idx++)
	//      rec_array[idx] = -1;

	num_recs = db->Select(SCHEDULE, "3", 11, rec_array, 255);

	DPRINT("note->recno [%d]\n", note->recno);
	temp_note = new NxTodo;
	for (idx = 0; idx < num_recs; idx++) {
	    if (-1 != rec_array[idx]) {
		ExtractRecord(temp_note, rec_array[idx]);
		if ((note->recno == temp_note->recnoPtr)	/*&& (temp_note->exception & REPEAT_DELETED) */
		    ) {
		    DPRINT
			("temp_note->startTime [%ld] nEarlyDay[%ld] nLateDay[%ld]\n",
			 temp_note->startTime, nEarlyDay, nLateDay);
		    if (temp_note->startTime >= nEarlyDay
			&& temp_note->startTime <= nLateDay) {
			delete temp_note;
			return false;
		    }
		}
	    }
	}
	delete temp_note;
	DPRINT("note->recno [%d]\n", note->recno);

	DPRINT("repeat_val [%ld]\n", repeat_val);

    }

    today = localtime(&note->startTime);
    today->tm_sec = 0;
    today->tm_min = 0;
    today->tm_hour = 0;
    app_time = mktime(today);

    if (REPEAT_DAILY & note->repeatFlag_1) {
	DPRINT("daily\n");
	if (app_time <= nEarlyDay) {
	    diff = difftime(nEarlyDay, app_time);
	    days = (long) (diff / 86400);
	    if (days == 0)
		return true;
	    mod = days % repeat_val;
	    if (0 == mod)
		return true;
	} else
	    return false;
    } else if (REPEAT_WEEKLY & note->repeatFlag_1) {
	if (app_time <= nEarlyDay) {
	    diff = difftime(nEarlyDay, app_time);
	    weeks = (long) (diff / (86400 * 7));
	    DPRINT("weeks [%ld]\n", weeks);
	    if (0 == weeks) {
		today = localtime(&nEarlyDay);
		if (note->
		    repeatWkMonFlag & (REPEAT_WEEK_SUNDAY << today->tm_wday))
		    return true;
	    }
	    mod = weeks % repeat_val;
	    if (0 == mod) {
		today = localtime(&nEarlyDay);
		if (note->
		    repeatWkMonFlag & (REPEAT_WEEK_SUNDAY << today->tm_wday))
		    return true;
	    }
	} else
	    return false;
    } else if (REPEAT_MONTHLY & note->repeatFlag_1) {
	months = MonthDiff(note->startTime, nEarlyDay);
	DPRINT("months [%d]\n", months);
	if (REPEAT_MONTH_DAY & note->repeatWkMonFlag) {
	    today = localtime(&nEarlyDay);
	    today_wday = today->tm_wday;
	    today_weeks = getMonthDayRepeat(nEarlyDay);

	    today = localtime(&note->startTime);
	    app_wday = today->tm_wday;
	    app_weeks = getMonthDayRepeat(note->startTime);
	    if (0 == months) {
		if (app_wday == today_wday && app_weeks == today_weeks)
		    return true;
	    }
	    mod = months % repeat_val;
	    if (0 == mod) {
		if (app_wday == today_wday && app_weeks == today_weeks)
		    return true;
	    }
	} else {		// REPEAT_MONTH_DATE
	    today = localtime(&nEarlyDay);
	    today_day = today->tm_mday;

	    today = localtime(&note->startTime);
	    app_day = today->tm_mday;
	    if (0 == months) {
		if (app_day == today_day)
		    return true;
	    }
	    mod = months % repeat_val;
	    if (0 == mod) {
		if (app_day == today_day)
		    return true;
	    }
	}
    } else if (REPEAT_YEARLY & note->repeatFlag_1) {
	today = localtime(&nEarlyDay);
	today_year = today->tm_year;
	today_mon = today->tm_mon;
	today_day = today->tm_mday;

	today = localtime(&note->startTime);
	app_year = today->tm_year;
	app_mon = today->tm_mon;
	app_day = today->tm_mday;

	if (note->startTime <= nLateDay) {
	    years = today_year - app_year;
	    if (0 == years) {
		if (app_day == today_day && today_mon == app_mon)
		    return true;
	    }
	    mod = years % repeat_val;
	    if (0 == mod) {
		if (app_day == today_day && today_mon == app_mon)
		    return true;
	    }
	}
    } else {			// REPEAT_NONE
	DPRINT("repeat_none\n");
	if (note->startTime >= nEarlyDay && note->startTime <= nLateDay)
	    return true;
	else
	    return false;
    }

    return false;

}

void
NxSchedule::clear_table()
{
    /*
       Fl_ToggleNode * n = tree->traverse_start();
       while(n) {
       delete n->data();
       tree->remove(n);
       n = tree->traverse_start();
       }
     */
}

void
NxSchedule::set_date_picker(NxCalendar * w)
{
    pCalendar = w;
}

void
NxSchedule::lookup_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(lookupWindow->GetWindowPtr(),
				   DEACTIVATE, dayWindow->GetWindowPtr());
    lookup_input->value("");
    stringCheck->value(false);
    dateCheck->value(false);
    Fl::focus((Fl_Widget *) lookup_input);
}
