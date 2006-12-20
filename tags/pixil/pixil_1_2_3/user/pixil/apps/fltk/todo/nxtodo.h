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


#ifndef nxnotepad_h
#define nxnotepad_h

#include <Flek/Fl_Calendar.H>
#include <nxbox.h>
#include "FL/fl_draw.H"
#include <FL/Fl_Editor.H>
#include <Flek/Fl_Toggle_Tree.H>
#include <FL/Flv_Table_Child.H>

#include <nxapp.h>
#include <nxdb.h>

#include <nxwindow.h>
#include <nxscroll.h>
#include <nxbutton.h>
#include <nxcheckbutton.h>
#include <nxradioroundbutton.h>
#include <nxmenubutton.h>
#include <nxinput.h>
#include <nxoutput.h>

#include <catlist.h>

#include <string>

#define POP_BUTTON_X 5
#define POP_BUTTON_Y(_x) ((_x->GetWindowPtr()->h()) - BUTTON_HEIGHT - 10 )

#define APP_NAME "ToDo"
#define CAT_NUM 2

#define ID 4
#define CATEGORYS 50
#define TITLE 100
#define DESC 100
#define NO_TIME  0

enum Due_Date
{
    NO_DATE = 0,
    TODAY,
    TOMORROW,
    END_OF_WEEK,
    CHOOSE_DATE
};

struct NxTodo
{
    int nId;
    char szCategory[CATEGORYS];

    int nComplete;
    int nPriority;
    char szTitle[TITLE];
    char szFile[DESC];
    long time;

};

class NxTodoList;

class NxTodoList:public NxApp
{

  private:
    static bool AllFlag;
    static time_t set_time;
    static time_t toTime;
    static time_t fromTime;
    static int date_type;

    // FLNX-Colosseum IPC

    static int fd;

#ifdef CONFIG_COLOSSEUM
    static void RestartIPC(void *o);
    virtual void ClientIPCHandler(int fd, void *o, int ipc_id = -1);
    static void ExecuteStringSearch(int ipc_id, char *searchStr, int width);
    static void ExecuteStringDateSearch(int ipc_id, char *searchStr,
					int width, time_t startTime,
					time_t endTime);
    static void ExecuteDateSearch(int ipc_id, int width, long startTime,
				  long endTime);
#endif

    static NxDb *db;
    static int id;
    static int g_EditFlag;
    static char *nx_inidir;

    // Container Window
    static NxWindow *main_window;

    // Standard PIM Window
    static NxPimWindow *todo_list_window;
    static NxPimWindow *todo_edit_window;
    static NxPimPopWindow *todo_dellist_window;
    static NxPimPopWindow *due_date_window;

    //  static NxPimPopWindow *date_window;
    static NxPimWindow *date_window;
    static NxPimPopWindow *error_window;

    static NxOutput *error_msg;

    static Fl_Editor *g_editor;
    static NxTodo *g_CurrentNote;

    static NxCategoryList *note_category;
    static NxCategoryList *edit_category_list;

    static Fl_Toggle_Tree *tree;
    static NxMenuButton *edit_priority_list;
    static NxInput *edit_title;
    static NxCheckButton *edit_complete;
    static NxButton *due_date;
    static NxScroll *note_list;
    static NxCategoryList *cat_list[CAT_NUM];

    // Pixmaps
    static Fl_Pixmap *echeck_pixmap;
    static Fl_Pixmap *check_pixmap;

    // for the calendar window
    Fl_Calendar *m_pDatePickerCalendar;
    time_t m_nDatePicked;
    time_t GetPickedDate();
    void SetPickedDate(time_t t);
    void DateCallback(void (*)(Fl_Widget *, void *));
    static void doneDate_callback(Fl_Widget * fl, void *l);
    static void cancelDate_callback(Fl_Widget * fl, void *l);
    static void todayDate_callback(Fl_Widget * fl, void *l);

    void (*m_pDatePickerCallback) (Fl_Widget *, void *);

    // search stuff
    //  static NxPimPopWindow *todo_lookup_window;
    static NxPimWindow *todo_lookup_window;
    static NxPimPopWindow *todo_results_window;
    static bool g_SearchFlag;
    static NxInput *lookup_input;
    static Flv_Table_Child *results_table;
    static NxOutput *results_message;
    static void doneLookup_callback(Fl_Widget * fl, void *l);
    static void searchLookup_callback(Fl_Widget * fl, void *l);
    static void cancelLookup_callback(Fl_Widget * fl, void *l);
    static void view_callback(Fl_Widget * fl, void *l);
    static void toCalendar_callback(Fl_Widget * fl, void *l);
    static void fromCalendar_callback(Fl_Widget * fl, void *l);
    static void fromDate_callback(Fl_Widget * fl, void *l);
    static void toDate_callback(Fl_Widget * fl, void *l);
    static NxCheckButton *stringCheck;
    static NxCheckButton *dateCheck;
    static NxButton *fromDateButton;
    static NxButton *toDateButton;
    void UpdateToButton();
    void UpdateFromButton();

    static void viewRecord(int recno);
    static void select_note(NxTodo * note);

    static void checkit_callback(Fl_Widget * fl, void *l);

    static void add_callback(Fl_Widget * fl, long l);
    static void edit_callback(Fl_Widget * fl, long l);
    static void done_callback(Fl_Widget * fl, long l);
    static void priority_callback(Fl_Widget * fl, void *l);
    static void category_callback(Fl_Widget * fl, void *l);
    static void list_callback(Fl_Widget * fl, void *l);
    //  static void delList_callback(Fl_Widget * fl, void * o);
    static void yesDelList_callback(Fl_Widget * fl, void *o);
    static void noDelList_callback(Fl_Widget * fl, void *o);
    static void dueDate_callback(Fl_Widget * fl, void *l);
    static void chooseDate_callback(Fl_Widget * fl, void *l);
    static void doneChoose_callback(Fl_Widget * fl, void *l);

    // Database functions
    static char *Record(int id, int cat_id, int complete, int priority,
			string title, string desc, long time);
    static char *Record(int pri_id, string pri_type, int dummy);
    static char *Record(int catid, string cat_name);
    static int GetCatId(char *szCategory);

    static void fill_categories();
    static void set_category(char *szCat, int refresh = 1);
    static void reset_category(char *szCat);
    static void set_priority(const char *szP);
    static void clear_tree();
    static void add_items(Fl_Toggle_Tree * t, const char *szCategory,
			  int refresh = 1);

    static void _fill_view_form(NxTodo * n);
    static void _fill_form(NxTodo * n);
    static void edit_note(NxTodo * n, int recno);
    static void write_note(NxTodo * note);

    static NxTodo *searchString(const char *searchVal);
    static NxTodo *searchDate(time_t fromTime, time_t toTime);
    static char *formatString(const NxTodo * note, int pixels);
    static char *formatLabel(const NxTodo * note, int pixels, bool fullpix);
    static bool checkDate(NxTodo * note, time_t fromTime, time_t toTime);

    // for the Set Date window
    static void noDueDate_callback(Fl_Widget * fl, void *l);
    static void todayDueDate_callback(Fl_Widget * fl, void *l);
    static void tomorrowDueDate_callback(Fl_Widget * fl, void *l);
    static void endOfWeek_callback(Fl_Widget * fl, void *l);
    static void chooseDueDate_callback(Fl_Widget * fl, void *l);
    static void setDate_callback(Fl_Widget * fl, void *l);
    void UpdateTime(long time);
    static NxRadioRoundButton *todayDateRadio;
    static NxRadioRoundButton *noDateRadio;
    static NxRadioRoundButton *chooseDateRadio;
    static NxRadioRoundButton *tomorrowDateRadio;
    static NxRadioRoundButton *EoWDateRadio;
    static NxButton *chooseDate;
    void ResetRadioButtons();


    static void errorOK_callback(Fl_Widget * fl, void *l);

  private:
    void make_list_window();
    void make_edit_window();
    void make_dellist_window();
    void make_lookup_window();
    void make_results_window();
    void make_due_date_window();
    void MakeCalendarWindow();
    void make_error_window();

  public:
      NxTodoList(int argc, char *argv[]);
      virtual ~ NxTodoList();
    Fl_Window *get_main_window();
    void show_default_window();

  protected:
      virtual void Refresh();
    // Menu Item callbacks
  public:
    static void delList_callback(Fl_Widget * fl, void *o);
    static void purge_callback(Fl_Widget * fl, void *o);
    static void lookup_callback(Fl_Widget * fl, void *l);
    static void exit_callback(Fl_Widget * fl, void *l);

};

#endif
