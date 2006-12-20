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


#ifndef NXSCHEDULE_H
#define NXSCHEDULE_H

#include <FL/Fl.H>
#include <Flek/Fl_Calendar.H>
#include <nxbox.h>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Flv_Table_Child.H>
#include <FL/Fl_Editor.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Editor.H>

#include <nxapp.h>
#include <nxdb.h>
#include <nxcalendar.h>
#include <nxweekcalendar.h>
#include <nxwindow.h>

#include <nxbutton.h>
#include <nxscroll.h>
#include <nxinput.h>
#include <nxoutput.h>
#include <nxmultilineinput.h>
#include <nxcheckbutton.h>
#include <nxbox.h>
#include <nxholdbrowser.h>
#include <nxmenubutton.h>
#include <nxintinput.h>
#include <nxinput.h>

#include <catlist.h>

#include "nxyearcal.h"
#include "nxmonthcal.h"
#include "nxweek.h"
#include "nxgroupbuttons.h"
#include <time.h>

#define TRUE 1
#define FALSE 0

#define APP_NAME "Scheduler"
#define CAT_NUM 4

#define ID        4
#define CATEGORYS 50
#define DESC      100
#define GRID_W 15
#define DAY_S 15
#define WEEK_S 24
#define WEEK_H 25
#define COLS 2
#define SCHEDULE "sched"
#define CATEGORY "sched_category"

#define HR_12(_x) ( _x > 12 ? _x - 12 : ( _x ? _x : 12 ) )
#define IS_AM(_x) ( _x >= 12 ? false : true)
#define AM_PM(_x) ( IS_AM(_x) ? "am" : "pm" )

#define MAX_ALARMS		100

typedef enum
{ APPT, TASK }
entry_type;
typedef enum
{ SUN, MON, TUE, WED, THU, FRI, SAT }
day_of_wk;

//Repeat Flag 1
#define REPEAT_NONE           0x0000
#define REPEAT_DAILY          0x0001
#define REPEAT_WEEKLY         0x0002
#define REPEAT_MONTHLY        0x0004
#define REPEAT_YEARLY         0x0008

#define REPEAT_WEEK_SUNDAY    0x0001
#define REPEAT_WEEK_MONDAY    0x0002
#define REPEAT_WEEK_TUESDAY   0x0004
#define REPEAT_WEEK_WEDNESDAY 0x0008
#define REPEAT_WEEK_THURSDAY  0x0010
#define REPEAT_WEEK_FRIDAY    0x0020
#define REPEAT_WEEK_SATURDAY  0x0040
#define REPEAT_WEEK_FLAGS 		0x007f

#define REPEAT_MONTH_DAY			0x0080
#define REPEAT_MONTH_DATE			0x0100

#define REPEAT_YEAR_DAY       0x0014
#define REPEAT_YEAR_DATE      0x0018

#define REPEAT_EXCEPTION			0x0001
#define REPEAT_DELETED				0x0002

#define DELETE_FLAG						0x0001
#define CHANGED_TIME_FLAG     0X0002
#define CHANGED_DATE_FLAG			0x0004
#define CHANGED_NEW						0x0008
#define CHANGED_ALARM					0x0010

#ifndef POP_BUTTON_X
#define POP_BUTTON_X 5
#endif
#ifndef POP_BUTTON_Y
#define POP_BUTTON_Y(_x) ((_x->GetWindowPtr()->h()) - BUTTON_HEIGHT - 10 )
#endif

// These flags are for keeping track of where to fill the desc from
#define DESC_UPDATE 0x0000
#define DESC_NEW    0x0001
#define DESC_KEEP   0X0002

// Alarm flags
#define ALARM_MIN		0x0000
#define ALARM_HOUR	0x0001
#define ALARM_DAY		0x0002
#define NO_ALARM 		0x7FFF

// For weekly

struct NxTodo
{
    unsigned int recno;

    long startTime;		// Start Time of appointment
    long endTime;		// End Time of appointment
    int allDayFlag;		// Flag for time to be ALL day
    int repeatFlag_1;		// Repeat flag #1 (for day, week, month, year)
    int repeatFlag_2;		// Repeat flag #2
    long repeatFlag_3;		// Repeat flag #3
    long repeatWkMonFlag;	// End of repeating entry
    entry_type entryType;	// Entry type APPT = Appointment, TASK = Task
    char szDescription[DESC];	// Description of appointment or task
    int exception;
    unsigned int recnoPtr;
    long fakeTime;
    int alarmInt;
    int alarmFlags;
};

class NxSchedule:public NxApp
{

    ////////////////////////////////////////
    // Private Methods
    ////////////////////////////////////////

  private:

    // FLNX-Colosseum IPC
#ifdef CONFIG_COLOSSEUM
    virtual void ClientIPCHandler(int fd, void *o, int ipc_id = -1);
    static void ExecuteSearch(int ipc_id, char *searchStr, int width);
    static void ExecuteStringDateSearch(int ipc_id, char *searchStr,
					int width, long startTime,
					long endTime);
    static void ExecuteDateSearch(int ipc_id, int width, long startTime,
				  long endTime);
#endif

    time_t m_CurrentDay;
    static time_t fromTime;
    static time_t toTime;

    static bool g_SearchFlag;

    //
    // Common 
    //

    static NxDb *db;
    static int idNum;
    static int g_EditFlag;

    //
    // Windows
    //

    static NxWindow *mainWindow;

    // Standard PIM Windows
    static NxPimWindow *dayWindow;
    static NxPimWindow *monthWindow;
    static NxPimWindow *setTimeWindow;
    static NxPimWindow *weekViewWindow;
    static NxPimWindow *monthViewWindow;
    static NxPimWindow *yearViewWindow;
    static NxPimWindow *lookupWindow;

    // Popup PIM Windows
    //static NxPimPopWindow * timeWindow;
    static NxPimWindow *timeWindow;
    static NxPimPopWindow *deleteWindow;
    //  static NxPimPopWindow * dateWindow;
    static NxPimWindow *dateWindow;
    //  static NxPimPopWindow * weekDateWindow;
    static NxPimWindow *weekDateWindow;
    static NxPimPopWindow *detailsWindow;
    static NxPimPopWindow *resultsWindow;
    static NxPimPopWindow *errorWindow;
    //static NxPimPopWindow * repeatWindow;
    static NxPimWindow *repeatWindow;
    static NxPimPopWindow *repeatEventWindow;
    static NxPimPopWindow *alarmViewWindow;

    static NxCategoryList *dayCat;
    static NxCategoryList *weekCat;
    static NxCategoryList *detailsCat;
    static NxCategoryList *setTimeCat;

    static NxCategoryList *cat_list[CAT_NUM];

    NxButton *m_DayButtons[7];

    static Fl_Group *buttonGroup;
    static NxButton *dailyButton;
    static NxButton *weeklyButton;
    static NxButton *monthlyButton;
    static NxButton *yearlyButton;

    static NxYearCal *pYearCal;
    static NxMonthCalendar *pMonthCal;
    static NxCalendar *m_pCalendar;
    static NxCalendar *pCalendar;
    static NxWeekCalendar *w_pCalendar;

    // dayWindow objects
    NxScroll *dayScroll;
    static Flv_Table_Child *table;

    // weekWindow objects

    // detailsWindow objects
    static NxButton *detailsDeleteButton;

    // setTimeWindow objects
    NxScroll *hourScroll;

    // deleteWindow objects

    //
    // Callbacks
    //
    static void dailyView_callback(Fl_Widget * w, void *l);
    static void weeklyView_callback(Fl_Widget * w, void *l);
    static void monthlyView_callback(Fl_Widget * w, void *l);
    static void yearlyView_callback(Fl_Widget * w, void *l);

    // for the error window
    static NxOutput *error_msg;
    static void errorOk_callback(Fl_Widget * fl, void *l);

    // for the week view window
    static NxBox *monYearBox;
    static NxBox *weekBox;
    NxButton *weekViewDayButtons[7];
    GridButton *upGridButton;
    GridButton *downGridButton;
    void UpdateWeekViewButtons();
    static void weekViewButtons_callback(Fl_Widget * w, void *l);
    static void weekGridUpButton_callback(Fl_Widget * w, void *l);
    static void weekGridDownButton_callback(Fl_Widget * w, void *l);
    static void backButton_callback(Fl_Widget * w, void *l);
    static void weekGoto_callback(Fl_Widget * w, void *l);
    NxScroll *weekScroll;
    WeekGrid *weekGrid;

    // for the day window
    static void adv_week_callback(Fl_Widget * w, void *l);
    static void bak_week_callback(Fl_Widget * w, void *l);
    static void new_day_callback(Fl_Widget * w, void *l);
    static void details_callback(Fl_Widget * w, void *l);
    static void new_callback(Fl_Widget * w, void *l);
    static void goto_callback(Fl_Widget * w, void *l);
    static void mainDatePicked_callback(Fl_Widget * w, void *l);
    static void weekDatePicked_callback(Fl_Widget * w, void *l);
    static void view_callback(Fl_Widget * w, void *l);

    // for the calendar window
    static void adv_year_callback(Fl_Widget * w, void *l);
    static void adv_month_callback(Fl_Widget * w, void *l);
    static void bak_year_callback(Fl_Widget * w, void *l);
    static void bak_month_callback(Fl_Widget * w, void *l);
    void set_date_picker(NxCalendar * w);

    // for the time window
    static void doneTime_callback(Fl_Widget * w, void *l);
    static void cancelTime_callback(Fl_Widget * w, void *l);
    static void timeHour_callback(Fl_Widget * w, void *l);
    static void timeAM_callback(Fl_Widget * w, void *l);
    static void timePM_callback(Fl_Widget * w, void *l);

    static void timeStart_callback(Fl_Widget * w, void *l);
    static void timeEnd_callback(Fl_Widget * w, void *l);
    static void timeNone_callback(Fl_Widget * w, void *l);


    // for the lookup window
    static void searchLookup_callback(Fl_Widget * fl, void *l);
    static void cancelLookup_callback(Fl_Widget * fl, void *l);
    static void fromCalendar_callback(Fl_Widget * fl, void *l);
    static void toCalendar_callback(Fl_Widget * fl, void *l);
    static NxInput *lookup_input;
    static NxButton *fromDateButton;
    static NxButton *toDateButton;
    static NxCheckButton *stringCheck;
    static NxCheckButton *dateCheck;
    static void fromDate_callback(Fl_Widget * fl, void *l);
    static void toDate_callback(Fl_Widget * fl, void *l);
    static void lookupDate_callback(Fl_Widget * fl, void *l);
    void UpdateFromButton();
    void UpdateToButton();

    static NxTodo *searchString(const char *searchVal);
    static NxTodo *searchDate(time_t fromTime, time_t toTime);
    static bool checkDate(NxTodo * note, time_t fromTime, time_t toTime);
    static char *formatString(const NxTodo * note, int pixels);

    // for the results window
    static NxOutput *results_message;
    static Flv_Table_Child *results_table;
    static void doneLookup_callback(Fl_Widget * fl, void *l);
    static void resultsView_callback(Fl_Widget * fl, void *l);

    // for the month view window
    static void monthCal_callback(Fl_Widget * w, void *l);
    static void thisMonth_callback(Fl_Widget * w, void *l);

    // for the year view window
    static void day_callback(Fl_Widget *, void *);
    static void month_callback(Fl_Widget *, void *);

    void Time_UpdateDisplay();
    void time_callback(void (*)(Fl_Widget *, void *));
    void SetTimes(time_t nStartTime, time_t nEndTime);
    void GetTimes(time_t * nStartTime, time_t * nEndTime);
    void (*m_pTimePickerCallback) (Fl_Widget *, void *);

    NxButton *m_pTimeAM;
    NxButton *m_pTimePM;
    NxButton *m_pTimeStart;
    NxButton *m_pTimeEnd;
    NxButton *m_pTimeNone;

    int m_nTimeState;
    tm m_StartTime;
    tm m_EndTime;

    NxHoldBrowser *m_pTimeHour;
    NxHoldBrowser *m_pTimeMinute;


    // for the detail window
    void showAlarmUi();
    void hideAlarmUi();
    void SetAlarmInt(int interval);

    void SetAlarm(NxTodo * n, time_t new_time);
    void DeleteAlarm(NxTodo * n);
    static void cancelEdit_callback(Fl_Widget * w, void *l);
    static void doneEdit_callback(Fl_Widget * w, void *l);
    static void deleteEdit_callback(Fl_Widget * w, void *l);
    static void detailDate_callback(Fl_Widget * w, void *l);
    static void detailTime_callback(Fl_Widget * w, void *l);
    static void detailRepeat_callback(Fl_Widget * w, void *l);
    static void alarmToggle_callback(Fl_Widget * w, void *l);
    static void alarmIntChanged_callback(Fl_Widget * w, void *l);
    static void detailDatePicked_callback(Fl_Widget * w, void *l);
    static void detailTimePicked_callback(Fl_Widget * w, void *l);
    static NxMenuButton *m_pDetailsAlarmInt;

    //for the repeat window
    static void repeatOk_callback(Fl_Widget * w, void *l);
    static void repeatCancel_callback(Fl_Widget * w, void *l);
    static void repeatNoneButton_callback(Fl_Widget * w, void *l);
    static void repeatDayButton_callback(Fl_Widget * w, void *l);
    static void repeatWeekButton_callback(Fl_Widget * w, void *l);
    static void repeatMonthButton_callback(Fl_Widget * w, void *l);
    static void repeatYearButton_callback(Fl_Widget * w, void *l);
    static void repeatDate_callback(Fl_Widget * w, void *l);
    static void repeatEveryInput_callback(Fl_Widget * w, void *l);
    static void weekDay_callback(Fl_Widget * w, void *l);
    static void monthDayDate_callback(Fl_Widget * w, void *l);
    void formatWeekMsg(char *msg, char *repeat_str);
    void formatMonthMsg(char *msg, char *repeat_str);
    void setWeekValue();
    long getWeekValue();
    NxButton *m_WeekButtons[7];
    static NxBox *week_box;
    static NxBox *month_box;
    void defaultUi();
    void resetUi(long repeat);
    long getMonthValue();
    void setMonthValue();
    long getRepeatValue();
    void getRepeatData();
    void repeatShow_ui();
    void noRepeat_Ui();
    void wordValue(int val, char *th_val);
    int getMonthDayRepeat(time_t date);
    time_t repeatDate;
    static NxButton *month_day;
    static NxButton *month_date;
    static NxButton *no_repeat;
    static NxButton *day_repeat;
    static NxButton *week_repeat;
    static NxButton *month_repeat;
    static NxButton *year_repeat;
    static NxBox *repeat_output;
    static NxBox *message_output;
    static NxBox *days_box;
    static NxIntInput *every_input;
    static NxMenuButton *end_list;
    static NxBox *every_box;
    static NxBox *list_box;

    // for the repeat event detailed window
    static void changeCurrent_callback(Fl_Widget * w, void *l);
    static void changeFuture_callback(Fl_Widget * w, void *l);
    static void changeAll_callback(Fl_Widget * w, void *l);
    static void changeCancel_callback(Fl_Widget * w, void *l);

    // for the delete window
    static void yesDelEdit_callback(Fl_Widget * w, void *l);
    static void noDelEdit_callback(Fl_Widget * w, void *l);

    NxButton *m_pDetails_TimeBox;
    NxButton *m_pDetails_DateBox;
    NxButton *m_pDetails_RepeatBox;
    NxMultilineInput *m_pDetails_DescBox;
    static NxCheckButton *m_pDetailsAlarmCheck;
    static NxIntInput *m_pDetailsAlarmInput;

    NxTodo *m_pCurrentItem;

    void FillDetailForm(NxTodo * n, int flags);

    // Database functions
    static char *Record(int id, int catid, long startTime, long endTime,
			int allDayFlag, int repeatFlag_1, int repeatFlag_2,
			long repeatFlag_3, long repeatWkMonFlag,
			entry_type entryType, char *szDescription, int except,
			int recnoPtr, int alarmInt, int alarmFlags);

    static char *Record(int catid, string cat_name);

    static void clear_table();
    void add_items(Flv_Table_Child * t);
    void add_item(int i, Flv_Table_Child * t, NxTodo * note);
    void UpdateDateDisplay();
    void UpdateWeekView();

    void MakeDayWindow();
    void MakeDateWindow();
    void MakeTimeWindow();
    void MakeMonthWindow();
    void MakeDetailsWindow();
    void MakeRepeatWindow();
    void MakeSetTimeWindow();
    void MakeDeleteWindow();
    void MakeLookupWindow();
    void MakeResultsWindow();
    void MakeErrorWindow();

    void MakeWeekViewWindow();
    void MakeMonthViewWindow();
    void MakeYearViewWindow();
    void MakeRepeatEventWindow();
    void MakeAlarmViewWindow();

    void CreateDateRange(time_t * e, time_t * l, time_t start_day,
			 time_t end_day);

    // for the alarm view window  
    static NxBox *alarm_msg;
    static void viewAlarm(int recno);
    void setNextAlarm(NxTodo * note);
    void formatAlarmMsg(int recno);
    static void alarmOk_callback(Fl_Widget * w, void *l);
    static void alarmSnooze_callback(Fl_Widget * w, void *l);
    static int alarm_count_;
    static int alarms[MAX_ALARMS];
    static char alarm_buf[255];
    void playAlarm();

    static void viewRecord(int recno);

    static int compar(NxTodo ** rec1, NxTodo ** rec2);

  protected:
      virtual void Refresh();
    ////////////////////////////////////////
    // Public Methods
    ////////////////////////////////////////

  public:

      NxSchedule(int argc, char *argv[]);
      virtual ~ NxSchedule();
    Fl_Window *get_main_window();
    void show_default_window();
    static void exit_callback(Fl_Widget * fl, void *l);

    // repeatWindow menu items
    static void noDate_callback(Fl_Widget * w, void *l);
    static void chooseDate_callback(Fl_Widget * w, void *l);
    //
    // Public menu callbacks
    //

    static void calendar_updated(NxCalendar * w);
    static void lookup_callback(Fl_Widget * fl, void *l);

    static void chooseAlarmMin_callback(Fl_Widget * w, void *l);
    static void chooseAlarmHour_callback(Fl_Widget * w, void *l);
    static void chooseAlarmDay_callback(Fl_Widget * w, void *l);

    static void edit_note(NxTodo * note, int recno);
    static void write_note(NxTodo * note);
    static void save(NxTodo * note);
    static void delete_note(NxTodo * note);
    static void ExtractRecord(NxTodo * note, int nRecId);
    bool IsForToday(NxTodo * note);
    bool IsForToday(NxTodo * note, time_t nEarlyDay, time_t nLateDay);
    int MonthDiff(time_t time1, time_t time2);
    time_t GetCurrentDay()
    {
	return m_CurrentDay;
    }
    int GetEditFlag()
    {
	return g_EditFlag;
    }
    NxCalendar *GetpCalendar()
    {
	return pCalendar;
    }
};

#endif
