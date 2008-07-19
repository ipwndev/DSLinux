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

#include <pixil_config.h>

#include <FL/Fl.H>
#include "nxtodo.h"
#include <nxbox.h>

#include <icons/echeck.xpm>
#include <icons/check.xpm>

#ifdef DEBUG
#define DPRINT(str, args...) printf("NXTODO DEBUG: " str, ## args)
#else
#define DPRINT(args...)
#endif

#define TODO "td"
#define PRIORITY "td_priority"
#define CATEGORY "td_category"
#define NX_INIFILE "td_"

extern int exit_flag;

about about_todo = {
    "About ToDo",
    "(c) 2001, Century Software.",
    "jeffm@censoft.com",
    "08/24/2001",
    "1.0"
};

////////////
// Database

// Contacts
field tdFields[] = {
    {'i', 1, 0},		// Field 0:Id
    {'i', 1, 0},		//       1:CategoryId
    {'i', 1, 0},		//       2:Complete
    {'i', 1, 0},		//       3:Priority
    {'c', TITLE, 0},		//       4:Title
    {'c', DESC, 0},		//       5:File Name
    {'l', 1, 0},		//       6:time
    {0}
};

// Database
fildes tdFile = {		// system file
    0, 0, 0,			// database file
    "dbf",			// extension
    7,				// nfields
    &tdFields[0]		// fieldlist
};

// Priority Database
#define PRI_TYPE 10

// Info
field pFields[] = {
    {'i', 1, 0},		// Field 0:pri_id
    {'c', PRI_TYPE, 0},		//       1:pri_type
    {0}
};
// Database
fildes pFile = {
    0, 0, 0,
    "dbf",
    2,
    &pFields[0]
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

Fl_Menu_Item todoMenuItem[] = {

    {"Record", 0, 0, 0, FL_SUBMENU},
    {"Delete Task", 0, NxTodoList::delList_callback},
    {"Purge", 0, NxTodoList::purge_callback, 0, FL_MENU_DIVIDER},
    {"Exit ToDo", 0, NxTodoList::exit_callback},
    {0},

    {"Edit", 0, 0, 0, FL_SUBMENU},
    {"Undo", 0, NxApp::undo_callback},
    {"Cut", 0, NxApp::cut_callback},
    {"Copy", 0, NxApp::copy_callback},
    {"Paste", 0, NxApp::paste_callback},
    //  { "Select All"},
    //  { "Keyboard", 0, NxApp::keyboard_callback},
    {0},

    {"Options", 0, 0, 0, FL_SUBMENU},
    //{ "Font"},
    {"Search", 0, NxTodoList::lookup_callback},
    {"About ToDo", 0, NxApp::show_about},
    {0},

    {0},

};

Fl_Menu_Item editMenuItem[] = {

    {"Edit", 0, 0, 0, FL_SUBMENU},
    {"Undo", 0, NxApp::undo_callback},
    {"Cut", 0, NxApp::cut_callback},
    {"Copy", 0, NxApp::copy_callback},
    {"Paste", 0, NxApp::paste_callback, 0, FL_MENU_DIVIDER},
    //  { "Select All"},
    //  { "Keyboard", 0, NxApp::keyboard_callback},
    {"Exit ToDo", 0, NxTodoList::exit_callback},
    {0},

    {"Options", 0, 0, 0, FL_SUBMENU},
    //{ "Font"},
    //{ "Phone Lookup"},
    {"About ToDo", 0, NxApp::show_about},
    {0},

    {0},

};

NxWindow *
    NxTodoList::main_window;

NxPimWindow *
    NxTodoList::todo_list_window;
NxPimWindow *
    NxTodoList::todo_edit_window;

//NxCalendar * NxTodoList::m_pDatePickerCalendar;

NxPimPopWindow *
    NxTodoList::todo_dellist_window;
NxPimPopWindow *
    NxTodoList::todo_results_window;
//NxPimPopWindow * NxTodoList::todo_lookup_window;
NxPimWindow *
    NxTodoList::todo_lookup_window;
NxPimPopWindow *
    NxTodoList::due_date_window;
//NxPimPopWindow * NxTodoList::date_window;
NxPimWindow *
    NxTodoList::date_window;
NxPimPopWindow *
    NxTodoList::error_window;

int
    NxTodoList::fd;
int
    NxTodoList::id;
int
    NxTodoList::g_EditFlag;
int
    NxTodoList::date_type;

char *
    NxTodoList::nx_inidir;

NxDb *
    NxTodoList::db;

NxCategoryList *
    NxTodoList::cat_list[CAT_NUM];

Fl_Editor *
    NxTodoList::g_editor;

NxTodo *
    NxTodoList::g_CurrentNote;

NxCategoryList *
    NxTodoList::note_category;
NxCategoryList *
    NxTodoList::edit_category_list;

Fl_Toggle_Tree *
    NxTodoList::tree;

NxMenuButton *
    NxTodoList::edit_priority_list;

NxInput *
    NxTodoList::edit_title;
NxInput *
    NxTodoList::lookup_input;

NxCheckButton *
    NxTodoList::edit_complete;

NxScroll *
    NxTodoList::note_list;

Fl_Pixmap *
    NxTodoList::echeck_pixmap;
Fl_Pixmap *
    NxTodoList::check_pixmap;

bool NxTodoList::g_SearchFlag;
bool NxTodoList::AllFlag;

Flv_Table_Child *
    NxTodoList::results_table;

NxOutput *
    NxTodoList::results_message;
NxOutput *
    NxTodoList::error_msg;

NxButton *
    NxTodoList::due_date;
NxButton *
    NxTodoList::chooseDate;
NxButton *
    NxTodoList::toDateButton;
NxButton *
    NxTodoList::fromDateButton;

NxCheckButton *
    NxTodoList::stringCheck;
NxCheckButton *
    NxTodoList::dateCheck;

NxRadioRoundButton *
    NxTodoList::todayDateRadio;
NxRadioRoundButton *
    NxTodoList::tomorrowDateRadio;
NxRadioRoundButton *
    NxTodoList::noDateRadio;
NxRadioRoundButton *
    NxTodoList::chooseDateRadio;
NxRadioRoundButton *
    NxTodoList::EoWDateRadio;

time_t NxTodoList::set_time;
time_t NxTodoList::toTime;
time_t NxTodoList::fromTime;

NxTodoList::NxTodoList(int argc, char *argv[])
    :
NxApp()
{

    db = new NxDb(argc, argv);
    optind = 1;
    NxApp::Instance()->set_keyboard(argc, argv);
    nx_inidir = db->GetPath();

    NxApp::Instance()->set_about(about_todo);
    g_EditFlag = 0;

    // Open or Create contacts database
    if (!db->Open(TODO, &tdFile, tdFields, 0)) {

	if (db->Create(TODO, &tdFile, tdFields, 0)) {
	  if (!db->Open(TODO, &tdFile, tdFields, 0)) {
	    exit(-1);
	  }
	} else {
	  exit(-1);	  
	}
	
    }
    // Database opened or created successfully, get number of records.
    int recno = db->NumRecs(TODO);
    id = recno;
    if (recno > 1) {

	char buf[4];
	db->Extract(TODO, recno, 0, buf);
	id = atoi(buf);

    }
    // Open or Create priority database
    if (!db->Open(PRIORITY, &pFile, pFields, 0)) {

	if (db->Create(PRIORITY, &pFile, pFields, 0)) {

	    if (!db->Open(PRIORITY, &pFile, pFields, 0)) {
		exit(-1);
	    }

	    char *record = 0;

	    record = Record(0, "High", 0);
	    db->Insert(PRIORITY, record);
	    record = Record(1, "Medium", 0);
	    db->Insert(PRIORITY, record);
	    record = Record(2, "Normal", 0);
	    db->Insert(PRIORITY, record);
	    record = Record(3, "Low", 0);
	    db->Insert(PRIORITY, record);
	    record = Record(4, "Very Low", 0);
	    db->Insert(PRIORITY, record);

	    delete[]record;
	    record = 0;

	} else {

	    exit(-1);

	}

    }
    // Open or Create category database
    if (!db->Open(CATEGORY, &catFile, catFields, 0)) {

	if (db->Create(CATEGORY, &catFile, catFields, 0)) {

	    if (!db->Open(CATEGORY, &catFile, catFields, 0)) {
		exit(-1);
	    }

	    char *record = 0;

	    record = Record(0, "Unfiled");
	    db->Insert(CATEGORY, record);
	    record = Record(1, "Business");
	    db->Insert(CATEGORY, record);
	    record = Record(2, "Personal");
	    db->Insert(CATEGORY, record);

	    delete[]record;
	    record = 0;

	} else {

	    exit(-1);

	}

    }

    main_window = new NxWindow(W_W, W_H, APP_NAME);

    make_list_window();
    make_edit_window();
    make_dellist_window();
    make_lookup_window();
    make_results_window();
    make_due_date_window();

    //m_pDatePickerCalendar->MakeDateWindow();
    //date_window = m_pDatePickerCalendar->GetDateWindow();
    //dd_window((Fl_Window*)date_window->GetWindowPtr());
    MakeCalendarWindow();

    make_error_window();
    main_window->end();

    note_category = todo_list_window->category_list;
    note_category->select(category_callback);
    edit_category_list = todo_edit_window->category_list;
    edit_category_list->select(list_callback);

    set_shown_window(todo_list_window->GetWindowPtr());
    echeck_pixmap = new Fl_Pixmap(echeck_xpm);
    check_pixmap = new Fl_Pixmap(check_xpm);

    cat_list[0] = note_category;
    cat_list[1] = edit_category_list;
    fill_categories();

    add_items(tree, "All");

    set_catlist_window((Fl_Window *) todo_list_window->
		       GetEditCategoryWindowPtr());

    date_type = TODAY;

    int file_fields[1];

    file_fields[0] = 5;

    SyncRegisterDB(db, tdFields, TODO, 1, file_fields, 1);
    SyncRegisterDB(db, catFields, CATEGORY, 0, NULL, 0);
    SyncRegisterDB(db, pFields, PRIORITY, 2, NULL, 0);

#ifdef CONFIG_COLOSSEUM
    // FLNX-Colosseum IPC
    fd = Add_Fd("nxtodo", _ClientIPCHandler);
#else
    ExecuteShow();
#endif

}

NxTodoList::~NxTodoList()
{
    db->Close(TODO);
    db->Close(PRIORITY);
    db->Close(CATEGORY);
    delete db;
    db = 0;
}

void
NxTodoList::Refresh()
{
    set_category(edit_category_list->label());
}

//
// FLNX-Colosseum IPC Methods
//

#ifdef CONFIG_COLOSSEUM

void
NxTodoList::RestartIPC(void *o)
{
    DPRINT("ReconnectIPC()...\n");

    fd = NxApp::Instance()->Add_Fd("nxtodo", _ClientIPCHandler);

    if (fd < 0) {

	Fl::add_timeout(10.0, RestartIPC);

    }

}

void
NxTodoList::ClientIPCHandler(int fd, void *o, int ipc_id)
{

    DPRINT("\n");
    DPRINT("ClientIPCHandler has now been started with message %s.\n",
	   (char *) o);

    char *tokenMsg = new char[MAX_LENGTH];
    char *passMsg = new char[MAX_LENGTH];

    if (o == NULL) {

	DPRINT("Read_Fd() called.\n");
	int length = MAX_LENGTH - 1;

	ipc_id = NxApp::Instance()->Read_Fd(passMsg, &length);

	if (ipc_id < 0) {
	    NxApp::Instance()->Remove_Fd(fd);
	    Fl::add_timeout(10.0, RestartIPC);
	    return;
	}

	DPRINT("And this is the message... %s, %d bytes.\n", passMsg, length);

	if ((passMsg == NULL) || (passMsg[0] == 0))
	    return;
	else
	    strcpy(tokenMsg, passMsg);

    } else if (ipc_id == -1) {

	strcpy(tokenMsg, (char *) o);
	strcpy(passMsg, (char *) o);
	ipc_id = NxApp::Instance()->Find_Fd("nxtodo");
	DPRINT("And this is the message... %s.\n", passMsg);

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
    strcpy(service, tmp);

    // MSG_CMD
    tmp = strtok(NULL, TOKEN);
    if (NULL != tmp)
	strcpy(msg_cmd, tmp);

    // DATA_ITEM
    tmp = strtok(NULL, TOKEN);
    if (NULL != tmp)
	strcpy(data_item, tmp);

    DPRINT("Expoding Message... %s, %s, %s.\n", service, msg_cmd, data_item);

    if (strcmp(msg_cmd, "EXECUTE") == 0) {

	DPRINT("EXECUTE message command recv.\n");

	if (!NxApp::Instance()->VerifyClient(service))
	    return;

	if (strcmp(data_item, "search") == 0) {

	    DPRINT("%s, %s, %s\n", service, msg_cmd, data_item);

	    char *searchStr = new char[MAX_LENGTH];
	    char *width = new char[4];
	    char *startStr = new char[16];
	    char *endStr = new char[16];

	    tmp = strtok(NULL, TOKEN);
	    if (NULL != tmp)
		strcpy(searchStr, tmp);
	    tmp = strtok(NULL, TOKEN);
	    if (NULL != tmp)
		strcpy(width, tmp);
	    tmp = strtok(NULL, TOKEN);
	    if (NULL == tmp)
		ExecuteStringSearch(ipc_id, searchStr, atoi(width));
	    else {
		strcpy(startStr, tmp);
		long startTime = strtol(startStr, NULL, 10);
		tmp = strtok(NULL, TOKEN);
		if (NULL != tmp)
		    strcpy(endStr, tmp);
		long endTime = strtol(endStr, NULL, 10);
		ExecuteStringDateSearch(ipc_id, searchStr, atoi(width),
					startTime, endTime);
	    }
	    delete[]startStr;
	    delete[]endStr;
	    delete[]searchStr;
	    delete[]width;
	    searchStr = width = endStr = startStr = NULL;

	}

	if (strcmp(data_item, "datesearch") == 0) {
	    char *startStr = new char[16];
	    char *endStr = new char[16];
	    char *width = new char[8];

	    tmp = strtok(NULL, TOKEN);
	    if (NULL != tmp)
		strcpy(width, tmp);
	    tmp = strtok(NULL, TOKEN);
	    if (NULL != tmp)
		strcpy(startStr, tmp);
	    tmp = strtok(NULL, TOKEN);
	    if (NULL != tmp)
		strcpy(endStr, tmp);

	    long startTime = strtol(startStr, NULL, 10);
	    long endTime = strtol(endStr, NULL, 10);
	    ExecuteDateSearch(ipc_id, atoi(width), startTime, endTime);

	    delete[]startStr;
	    delete[]endStr;
	    delete[]width;

	    startStr = endStr = width = 0;
	}

	if (0 == strcmp(data_item, "showrecord")) {
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

void
NxTodoList::ExecuteStringSearch(int ipc_id, char *searchStr, int width)
{

    if (searchStr == NULL)
	return;

    char *msg = new char[MAX_LENGTH];
    NxTodo *note;

    while ((note = searchString(searchStr)) != NULL) {

	char *result = formatLabel(note, width, true);

	// Send DATA message to client
	// (e.g. "nxtodo^DATA^1^Century *Software* is neato^")
	strcpy(msg, "nxtodo^DATA^search^");
	//strcat(msg, note->szId);
	char id[4];
	sprintf(id, "%d", note->nId);
	strcat(msg, id);
	strcat(msg, "^");
	strcat(msg, result);
	strcat(msg, "^");

	int length = MAX_LENGTH;
	NxApp::Instance()->Write_Fd(ipc_id, msg, length);

    }

    strcpy(msg, "nxtodo^ACK^DATA^search^");
    NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);

    delete[]msg;
    msg = NULL;

}

void
NxTodoList::ExecuteStringDateSearch(int ipc_id, char *searchStr, int width,
				    long startTime, long endTime)
{
    if (NULL == searchStr)
	return;

    char *msg = new char[MAX_LENGTH];
    NxTodo *note;

    while ((note = searchString(searchStr)) != NULL) {
	if (true == checkDate(note, (time_t) startTime, (time_t) endTime)) {
	    char *result = formatLabel(note, width, true);

	    // Send DATE message to client
	    strcpy(msg, "nxtodo^DATA^search^");
	    //strcat(msg, note->szId);
	    char id[4];
	    sprintf(id, "%d", note->nId);
	    strcat(msg, id);
	    strcat(msg, "^");
	    strcat(msg, result);
	    strcat(msg, "^");

	    NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);
	}
    }

    strcpy(msg, "nxtodo^ACK^DATA^search^");
    NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);

    delete[]msg;
    msg = NULL;
}


void
NxTodoList::ExecuteDateSearch(int ipc_id, int width, long startTime,
			      long endTime)
{
    char *msg = new char[MAX_LENGTH];
    NxTodo *note;

    while ((note = searchDate(startTime, endTime)) != NULL) {
	char *result = formatLabel(note, width, true);

	strcpy(msg, "nxtodo^DATA^search^");
	sprintf(msg, "%s^%d^%s^", msg, note->nId, result);

	NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);
    }

    strcpy(msg, "nxtodo^ACK^DATA^seach^");
    NxApp::Instance()->Write_Fd(ipc_id, msg, MAX_LENGTH);

    delete[]msg;
    msg = NULL;
}

#endif /* CONFIG_COLOSSEUM */

//
// Database methods
//

void
NxTodoList::exit_callback(Fl_Widget * fl, void *l)
{
    exit_flag = 1;
    main_window->hide();
}

char *
NxTodoList::Record(int id, int cat_id, int complete, int priority,
		   string title, string desc, long time)
{

    char *rec = new char[MAXRECSIZ];

    memset(rec, 0, MAXRECSIZ);

    put16(&rec[tdFields[0].offset], id);
    DPRINT("Record id [%d]\n", id);
    put16(&rec[tdFields[1].offset], cat_id);
    put16(&rec[tdFields[2].offset], complete);
    put16(&rec[tdFields[3].offset], priority);
    strcpy(&rec[tdFields[4].offset], title.c_str());
    strcpy(&rec[tdFields[5].offset], desc.c_str());
    put32(&rec[tdFields[6].offset], time);

    return rec;

}

char *
NxTodoList::Record(int pri_id, string pri_type, int dummy)
{

    char *rec = new char[MAXRECSIZ];

    memset(rec, 0, MAXRECSIZ);
    put16(&rec[pFields[0].offset], pri_id);
    strcpy(&rec[pFields[1].offset], pri_type.c_str());

    return rec;

}

char *
NxTodoList::Record(int catid, string cat_name)
{

    char *rec = new char[MAXRECSIZ];

    memset(rec, 0, MAXRECSIZ);
    put16(&rec[catFields[0].offset], catid);
    strcpy(&rec[catFields[1].offset], cat_name.c_str());

    return rec;

}

//
// Window Methods
//

Fl_Window *
NxTodoList::get_main_window()
{
    if (main_window)
	return main_window;
    else
	return 0;
}

void
NxTodoList::show_default_window()
{
    show_window(todo_list_window->GetWindowPtr());
}

void
NxTodoList::make_list_window()
{

    todo_list_window =
	new NxPimWindow(APP_NAME, todoMenuItem, db, CATEGORY, TODO,
			(void (*)(const char *)) set_category);
    add_window((Fl_Window *) todo_list_window->GetWindowPtr());

    {

	NxScroll *o = note_list =
	    new NxScroll(-1, 31, W_W + 2, BUTTON_Y - 33);
	o->movable(false);

	{
	    tree = new Fl_Toggle_Tree(0, 31, W_W, 10);
	    tree->callback(checkit_callback);
	}

	o->end();
	todo_list_window->add((Fl_Widget *) o);

    }

    {
	NxBox *o = new NxBox(5, 5, 85, 25, "");
	o->labelfont(1);
	todo_list_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH - 5, BUTTON_HEIGHT,
			 "Add");

	o->box(FL_SHADOW_BOX);
	o->labelfont(1);
	o->callback(add_callback);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	todo_list_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o = new NxButton(BUTTON_X + 58, BUTTON_Y, BUTTON_WIDTH - 5,
				   BUTTON_HEIGHT, "Edit");

	o->box(FL_SHADOW_BOX);
	o->labelfont(1);
	o->callback(edit_callback);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	todo_list_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o = new NxButton(BUTTON_X + 116, BUTTON_Y, BUTTON_WIDTH - 5,
				   BUTTON_HEIGHT, "Delete");

	o->box(FL_SHADOW_BOX);
	o->labelfont(1);
	o->callback(delList_callback);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	todo_list_window->add((Fl_Widget *) o);
    }

}

void
NxTodoList::make_edit_window()
{

    todo_edit_window =
	new NxPimWindow(APP_NAME, editMenuItem, db, CATEGORY, TODO,
			(void (*)(const char *)) set_category);
    add_window((Fl_Window *) todo_edit_window->GetWindowPtr());

    {
	NxOutput *o = new NxOutput(BUTTON_X, 30, BUTTON_WIDTH, 25);
	o->value(" Priority:");
	o->movable(false);
	todo_edit_window->add((Fl_Widget *) o);
    }

    {
	NxMenuButton *o = edit_priority_list =
	    new NxMenuButton(63, 34, BUTTON_WIDTH + 10, BUTTON_HEIGHT, "");


	o->box(FL_SHADOW_BOX);
	o->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

	int numRecs = db->NumRecs(PRIORITY);

	int get_recno[1];
	get_recno[0] = -1;
	char buf[10];

	for (int i = 0; i < numRecs; i++) {

	    sprintf(buf, "%d", i);
	    db->Select((string) PRIORITY, buf, 0, get_recno, 255);

	    // id
	    db->Extract((string) PRIORITY, get_recno[0], 1, buf);

	    o->add(buf);

	}

	o->callback(priority_callback);
	o->value(0);


	todo_edit_window->add((Fl_Widget *) o);

    }

    {
	NxOutput *o = new NxOutput(BUTTON_X, 56, BUTTON_WIDTH, 25);
	o->value("     Title:");
	o->movable(false);
	o->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
	todo_edit_window->add((Fl_Widget *) o);
    }

    {
	NxInput *o = edit_title = new NxInput(63, 58, W_W - 93, 20, "");
	o->movable(false);
	o->when(FL_WHEN_CHANGED | FL_WHEN_NOT_CHANGED);
	o->maximum_size(TITLE - 1);
	todo_edit_window->add((Fl_Widget *) o);
    }

    {
	NxOutput *o = new NxOutput(BUTTON_X - 10, 79, BUTTON_WIDTH + 5, 25);
	o->value("Due Date:");
	o->movable(false);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	todo_edit_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o = due_date =
	    new NxButton(BUTTON_X + 56, 83, BUTTON_WIDTH + 10, BUTTON_HEIGHT,
			 "Jun 09, 74");
	o->movable(false);
	o->callback(dueDate_callback);
	o->box(FL_SHADOW_BOX);

	todo_edit_window->add((Fl_Widget *) o);
    }

    {
	NxCheckButton *o = edit_complete =
	    new NxCheckButton(BUTTON_X + 30, 99, "Complete");
	// NxCheckButton *o = stringCheck = new NxCheckButton(BUTTON_X, 35, 25, 25, "Only entries containing:");
	o->movable(false);
	todo_edit_window->add((Fl_Widget *) o);
    }

    {

	{
	    Fl_Editor *o = g_editor =
		new Fl_Editor(5, 124, W_W - 10, BUTTON_Y - 131);
	    o->movable(false);
	    o->callback(NxApp::Instance()->pasteTarget_callback, (void *) 1);
	    o->when(FL_WHEN_CHANGED | FL_WHEN_NOT_CHANGED);
	    todo_edit_window->add((Fl_Widget *) o);
	}

    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Done");

	o->box(FL_SHADOW_BOX);
	o->labelfont(1);
	o->callback(done_callback);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	todo_edit_window->add((Fl_Widget *) o);
    }
}

void
NxTodoList::make_dellist_window()
{

    todo_dellist_window = new NxPimPopWindow("Delete");
    add_window((Fl_Window *) todo_dellist_window->GetWindowPtr());

    {
	NxBox *o = new NxBox(BUTTON_X, 43, W_W - BUTTON_X - 15, 0,
			     "Delete current task ?");

	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_WRAP | FL_ALIGN_LEFT | FL_ALIGN_TOP);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	todo_dellist_window->add((Fl_Widget *) o);
    }


    {
	NxButton *o =
	    new NxButton(15, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Yes");

	o->box(FL_SHADOW_BOX);
	o->callback(yesDelList_callback);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	todo_dellist_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_WIDTH + 17, 90, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "No");

	o->box(FL_SHADOW_BOX);
	o->callback(noDelList_callback);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	todo_dellist_window->add((Fl_Widget *) o);
    }

}

void
NxTodoList::make_results_window()
{

    todo_results_window =
	new NxPimPopWindow("Search Results",
			   NxApp::Instance()->getGlobalColor(APP_FG), 5,
			   (W_W / 3), W_W - 10, (W_H - (W_W / 2)));

    add_window((Fl_Window *) todo_results_window->GetWindowPtr());

    {
	results_message = new NxOutput(4, 19, W_W - 19, 25);
	results_message->value("Nothing Found.");
	results_message->hide();
	todo_results_window->add((Fl_Widget *) results_message);
    }

    {
	results_table =
	    new Flv_Table_Child(4, 19, (W_W - 19),
				(W_H - (W_W / 2) - 3 * (BUTTON_HEIGHT)), 0,
				(W_W - 25));
	results_table->callback(view_callback);
	results_table->SetCols(1);
	todo_results_window->add((Fl_Widget *) results_table);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, (W_H - (W_W / 2) - BUTTON_HEIGHT - 9),
			 BUTTON_WIDTH, BUTTON_HEIGHT, "Done");
	o->callback(doneLookup_callback);
	todo_results_window->add((Fl_Widget *) o);
    }

}

void
NxTodoList::make_lookup_window()
{

    static char fromBuf[30];
    static char toBuf[30];

    toTime = fromTime = time(0);
    tm *tt = localtime(&fromTime);

    //strftime(fromBuf, 29, "%b %d, %y", tt);
    //strftime(toBuf, 29, "%b %d, %y", tt);
    GetDateString(fromBuf, tt, sizeof(fromBuf), SHORT_YEAR);
    GetDateString(toBuf, tt, sizeof(toBuf), SHORT_YEAR);

    todo_lookup_window =
	//new NxPimPopWindow("Todo Lookup", NxApp::Instance()->getGlobalColor(APP_FG), 10, 10, W_W -20, W_H - 100);
	new NxPimWindow(W_X, W_Y, W_W, W_H);

    add_window((Fl_Window *) todo_lookup_window->GetWindowPtr());

    {
	NxCheckButton *o = stringCheck =
	    new NxCheckButton(BUTTON_X, 10, "Only entries containing:");
	o->movable(false);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	todo_lookup_window->add((Fl_Widget *) o);
    }

    {
	NxInput *o = lookup_input = new NxInput(BUTTON_X + 19, 35, 141, 20);
	o->movable(false);
	lookup_input->maximum_size(99);
	todo_lookup_window->add((Fl_Widget *) o);

    }

    {
	NxCheckButton *o = dateCheck =
	    new NxCheckButton(BUTTON_X, 60, "Limit by date range:");
	o->movable(false);
	todo_lookup_window->add((Fl_Widget *) o);
    }

    {
	NxOutput *o = new NxOutput(BUTTON_X + 19, 85, 60, BUTTON_HEIGHT);
	o->movable(false);
	o->value("From:");
	todo_lookup_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o = fromDateButton =
	    new NxButton(BUTTON_X + 60, 85, 100, BUTTON_HEIGHT);
	o->movable(false);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	o->label(fromBuf);
	o->callback(fromCalendar_callback, this);
	o->redraw();
	todo_lookup_window->add((Fl_Widget *) o);
    }

    {
	NxOutput *o = new NxOutput(BUTTON_X + 19, 105, 60, BUTTON_HEIGHT);
	o->movable(false);
	o->value("To:");
	todo_lookup_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o = toDateButton =
	    new NxButton(BUTTON_X + 60, 105, 100, BUTTON_HEIGHT);
	o->movable(false);
	o->label(toBuf);
	o->redraw();
	o->callback(toCalendar_callback, this);
	todo_lookup_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Search");
	o->callback(searchLookup_callback, this);
	todo_lookup_window->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 63, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Cancel");
	o->callback(cancelLookup_callback);
	todo_lookup_window->add((Fl_Widget *) o);
    }

}

void
NxTodoList::make_due_date_window()
{
    due_date_window =
	new NxPimPopWindow("Due Date", FL_DARK2, 10, W_H / 6, W_W - 20,
			   W_H - 110);

    add_window((Fl_Window *) due_date_window->GetWindowPtr());

    {
	NxRadioRoundButton *o = noDateRadio =
	    new NxRadioRoundButton(10, 20, 20, 20, "No Date");
	due_date_window->add((Fl_Widget *) o);
	o->callback(noDueDate_callback, this);
    }

    {
	NxRadioRoundButton *o = todayDateRadio =
	    new NxRadioRoundButton(10, 45, 20, 20, "Today");
	due_date_window->add((Fl_Widget *) o);
	o->callback(todayDueDate_callback, this);
    }

    {
	NxRadioRoundButton *o = tomorrowDateRadio =
	    new NxRadioRoundButton(10, 70, 20, 20, "Tomorrow");
	due_date_window->add((Fl_Widget *) o);
	o->callback(tomorrowDueDate_callback, this);
    }

    {
	NxRadioRoundButton *o = EoWDateRadio =
	    new NxRadioRoundButton(10, 95, 20, 20, "End of Week");
	due_date_window->add((Fl_Widget *) o);
	o->callback(endOfWeek_callback, this);
    }

    {
	NxRadioRoundButton *o = chooseDateRadio =
	    new NxRadioRoundButton(10, 120, 20, 20, "");
	due_date_window->add((Fl_Widget *) o);
	o->callback(chooseDueDate_callback, this);
    }

    {
	NxButton *o = chooseDate =
	    new NxButton(28, 120, BUTTON_WIDTH + 20, BUTTON_HEIGHT,
			 "June 09, 74");
	due_date_window->add((Fl_Widget *) o);
	o->callback(chooseDate_callback, this);
	o->deactivate();
    }

    {
	NxButton *o =
	    new NxButton(POP_BUTTON_X, POP_BUTTON_Y(due_date_window),
			 BUTTON_WIDTH, BUTTON_HEIGHT, "Done");
	o->callback(doneChoose_callback, this);
	due_date_window->add((Fl_Widget *) o);
    }
	/***
	{
 		m_pDatePickerCalendar = new NxCalendar((NxApp*)this, 0, 32, W_W, BUTTON_Y-38,"");
		//m_pDatePickerCalendar->CalendarUpdate((void(*)(NxCalendar *))calendar_updated);
		due_date_window->add((Fl_Widget*)m_pDatePickerCalendar);
	}
	****/
    ResetRadioButtons();
}

void
NxTodoList::make_error_window()
{
    error_window = new NxPimPopWindow("Error");
    add_window((Fl_Window *) error_window->GetWindowPtr());

    {
	error_msg =
	    new NxOutput(4, 19, error_window->GetWindowPtr()->w() - 10, 25);
	error_msg->value("Error: No Search Constraint.");
	error_window->add((Fl_Widget *) error_msg);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Ok");
	o->callback(errorOK_callback);
	error_window->add((Fl_Widget *) o);
    }

}

void
NxTodoList::errorOK_callback(Fl_Widget * fl, void *l)
{
    error_window->GetWindowPtr()->hide();
    NxApp::Instance()->show_window(todo_lookup_window->GetWindowPtr(),
				   DEACTIVATE,
				   todo_list_window->GetWindowPtr());
}

void
NxTodoList::noDueDate_callback(Fl_Widget * fl, void *l)
{
    date_type = NO_DATE;
    chooseDate->deactivate();
}

void
NxTodoList::todayDueDate_callback(Fl_Widget * fl, void *l)
{
    date_type = TODAY;
    chooseDate->deactivate();
}

void
NxTodoList::tomorrowDueDate_callback(Fl_Widget * fl, void *l)
{
    date_type = TOMORROW;
    chooseDate->deactivate();
}

void
NxTodoList::endOfWeek_callback(Fl_Widget * fl, void *l)
{
    date_type = END_OF_WEEK;
    chooseDate->deactivate();
}

void
NxTodoList::chooseDueDate_callback(Fl_Widget * fl, void *l)
{
    date_type = CHOOSE_DATE;
    chooseDate->activate();
}

void
NxTodoList::checkit_callback(Fl_Widget * fl, void *l)
{

    Fl_Toggle_Node *node = tree->selected();

    if (tree->selection_count() > 1)
	tree->unselect();

    if (node) {

	NxTodo *n = (NxTodo *) node->user_data();

	if ((Fl::event_x() >= 19) && (Fl::event_x() <= 27)) {

	    tree->unselect();

	    int recno[1];
	    recno[0] = -1;

	    char id[4];
	    sprintf(id, "%d", n->nId);
	    db->Select(TODO, id, 0, recno, 1);

	    if (recno[0] != -1) {

		n->nComplete = !n->nComplete;

		edit_note(n, recno[0]);

		if (node->pixmap() == echeck_pixmap)
		    node->pixmap(check_pixmap);
		else if (node->pixmap() == check_pixmap)
		    node->pixmap(echeck_pixmap);

		if (0 != strcmp("All", note_category->label()))
		    set_category(n->szCategory, 0);

	    } else {

		if (0 != strcmp("All", note_category->label()))
		    set_category(n->szCategory, 0);
		return;

	    }

	} else if (Fl::event_clicks()) {

	    g_EditFlag = 1;

	    NxApp::Instance()->
		set_catlist_window((Fl_Window *) todo_edit_window->
				   GetEditCategoryWindowPtr());

	    NxTodo *n = (NxTodo *) node->user_data();
	    g_editor->Clear();

	    if (n->szFile[0] != '^') {

		FILE *fd = fopen(((NxTodo *) node->user_data())->szFile, "r");

		if (fd) {

		    g_editor->LoadFrom(fd);
		    fclose(fd);

		}
	    }

	    _fill_form(n);

	    if (0 == strcmp("All", note_category->label()))
		AllFlag = true;

	    NxApp::Instance()->show_window(todo_edit_window->GetWindowPtr(),
					   ACTIVATE);

	}
    }

    Fl::event_clicks(0);

}

void
NxTodoList::add_callback(Fl_Widget * fl, long l)
{
    //int myfd = NxApp::Instance()->Find_Fd("nxaddress");

    //char msg[255];
    //int length = sizeof(msg);
    //strcpy(msg, "nxtodo^INITIATE^0");
    //NxApp::Instance()->Write_Fd(myfd, &msg[0], length);

    //strcpy(msg, "nxtodo^EXECUTE^search^Jeff^246");
    //NxApp::Instance()->Write_Fd(myfd, &msg[0], length);

    tree->unselect();

    NxTodo *n = new NxTodo;

    NxApp::Instance()->set_catlist_window((Fl_Window *) todo_edit_window->
					  GetEditCategoryWindowPtr());

    memset(n->szCategory, 0, CATEGORYS);
    memset(n->szTitle, 0, TITLE);
    memset(n->szFile, 0, DESC);

    if (0 == strcmp("All", note_category->label())) {
	strcpy(n->szCategory, "Unfiled");
	AllFlag = true;
    } else
	strcpy(n->szCategory, note_category->label());

    edit_category_list->label(n->szCategory);
    edit_category_list->hide();
    edit_category_list->show();
    note_category->label(n->szCategory);

    strcpy(n->szTitle, "Untitled");
    strcpy(n->szFile, "^");
    n->nComplete = 0;
    n->nPriority = 1;
    n->time = NO_TIME;

    g_editor->Clear();

    g_EditFlag = 0;

    _fill_form(n);

    delete(n);
    n = 0;

    // Show edit window and hide list window.
    NxApp::Instance()->show_window(todo_edit_window->GetWindowPtr(),
				   ACTIVATE);

}

void
NxTodoList::edit_callback(Fl_Widget * fl, long l)
{

    Fl_Toggle_Node *node = tree->selected();

    g_EditFlag = 1;

    NxApp::Instance()->set_catlist_window((Fl_Window *) todo_edit_window->
					  GetEditCategoryWindowPtr());

    if (node) {

	NxTodo *n = (NxTodo *) node->user_data();
	g_editor->Clear();

	if (n->szFile[0] != '^') {

	    FILE *fd = fopen(n->szFile, "r");

	    if (fd) {

		g_editor->LoadFrom(fd);
		g_editor->MoveTo(0, 0);
		fclose(fd);

	    }

	}

	_fill_form(n);

	if (0 == strcmp("All", note_category->label()))
	    AllFlag = true;
	// Show edit window and hide list window
	NxApp::Instance()->show_window(todo_edit_window->GetWindowPtr(),
				       ACTIVATE);


    } else {

	return;

    }

}

void
NxTodoList::delList_callback(Fl_Widget * fl, void *l)
{

    Fl_Toggle_Node *node = tree->selected();

    if (!node) {
	return;
    }

    if (0 == strcmp("All", note_category->label()))
	AllFlag = true;

    NxApp::Instance()->show_window(todo_dellist_window->GetWindowPtr(),
				   DEACTIVATE,
				   todo_list_window->GetWindowPtr());
}

void
NxTodoList::yesDelList_callback(Fl_Widget * fl, void *l)
{

    // Delete Note
    Fl_Toggle_Node *node = tree->selected();

    if (node) {

	int recno[1];
	recno[0] = -1;

	NxTodo *n = (NxTodo *) node->user_data();

	db->Select(TODO, n->szFile, 5, recno, 1);
	unlink(n->szFile);

	char id[4];
	sprintf(id, "%d", n->nId);
	db->Select(TODO, id, 0, recno, 1);

	if (recno[0] == -1) {
	    return;
	}

	db->DeleteRec(TODO, recno[0]);
	if (AllFlag) {
	    set_category("All");
	    AllFlag = false;
	} else
	    set_category(n->szCategory);

    } else {

	return;

    }

    note_list->position(0, 0);
    NxApp::Instance()->show_window(todo_list_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxTodoList::noDelList_callback(Fl_Widget * fl, void *l)
{
    note_list->position(0, 0);
    NxApp::Instance()->show_window(todo_list_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxTodoList::purge_callback(Fl_Widget * fl, void *o)
{

    db->Purge(TODO, 2, "1");
    set_category("All");
    note_list->position(0, 0);

}

void
NxTodoList::_fill_form(NxTodo * n)
{

    char *cat = new char[CATEGORYS];
    char *title = new char[TITLE];

    strcpy(cat, n->szCategory);
    strcpy(title, n->szTitle);

    edit_category_list->label(cat);
    edit_priority_list->label(edit_priority_list->text(n->nPriority));
    edit_priority_list->value(n->nPriority);
    edit_complete->value(n->nComplete);
    edit_title->value(title);

    ((NxTodoList *) (NxApp::Instance()))->UpdateTime(n->time);
}

void
NxTodoList::edit_note(NxTodo * note, int recno)
{

    DPRINT("Edit note\n");
    int id = note->nId;
    int catid = GetCatId(note->szCategory);

    char *record = Record(id, catid, note->nComplete, note->nPriority,
			  note->szTitle, note->szFile, note->time);

    db->Edit(TODO, recno, record);

    delete[]record;
    record = 0;


}

void
NxTodoList::write_note(NxTodo * note)
{

    int catid = GetCatId(note->szCategory);

    char *record = Record(note->nId, catid, note->nComplete, note->nPriority,
			  note->szTitle, note->szFile, note->time);

    db->Insert(TODO, record);

    delete[]record;
    record = 0;

}

int
NxTodoList::GetCatId(char *szCategory)
{

    int recno[1];
    recno[0] = -1;

    char catid[255];

    db->Select(CATEGORY, szCategory, 1, recno, 1);

    if (recno[0] == -1)
	return (0);

    db->Extract(CATEGORY, recno[0], 0, catid);

    int cid = atoi(catid);

    return cid;


}

void
NxTodoList::add_items(Fl_Toggle_Tree * t, const char *szCategory, int refresh)
{

    int rec_array[255];
    int cat_array[1];
    char value[16];
    char cat[16];
    Fl_Toggle_Node *n;
    NxTodo *note;
    int idx = 0;

    cat_array[0] = -1;

    for (idx = 0; idx < 255; idx++) {
	rec_array[idx] = -1;
    }

    sprintf(value, " ");

    db->Select(CATEGORY, (char *) szCategory, 1, cat_array, 1);

    if ((cat_array[0] >= 0))
	db->Extract(CATEGORY, cat_array[0], 0, value);

    if (0 == strcmp("All", szCategory))
	db->Select(TODO, rec_array, 255);
    else
	db->Select(TODO, value, 1, rec_array, 255);

    int i = 0;

    while (rec_array[i] != -1) {

	int j = -1;
	int jdx = rec_array[i];
	i++;

	note = new NxTodo;

	if ((cat_array[0] >= 0) || (0 == strcmp("All", szCategory))) {

	    if (cat_array[0] >= 0) {
		db->Extract(TODO, jdx, 1, cat);
		j = strcmp(value, cat);
	    }


	    if ((j == 0) || (strcmp("All", szCategory) == 0)) {

		// id
		char id[16];
		db->Extract(TODO, jdx, 0, id);
		note->nId = atoi(id);
		sprintf(id, "%d", note->nId);
		DPRINT("add_items note->nID [%d] jdx [%d]\n", note->nId, jdx);

		// categoryid
		int catid_array[1];
		char catid[8];

		catid_array[0] = -1;

		memset(note->szCategory, 0, CATEGORYS);
		db->Extract(TODO, jdx, 1, catid);
		db->Select(CATEGORY, catid, 0, catid_array, 1);
		if (-1 != catid_array[0])
		    db->Extract(CATEGORY, catid_array[0], 1,
				note->szCategory);
		else
		    strcpy(note->szCategory, "Unfiled");

		// nComplete
		char buf1[4];
		db->Extract(TODO, jdx, 2, buf1);
		note->nComplete = atoi(buf1);

		// nPriority
		char buf2[4];
		db->Extract(TODO, jdx, 3, buf2);
		note->nPriority = atoi(buf2);

		// szTitle
		db->Extract(TODO, jdx, 4, note->szTitle);

		// szFile
		db->Extract(TODO, jdx, 5, note->szFile);

		char buf3[16];
		db->Extract(TODO, jdx, 6, buf3);
		note->time = strtol(buf3, NULL, 10);

		if (refresh) {

		    int width = tree->w() - note_list->scrollbar.w();
		    char *label = formatLabel(note, width, false);

		    if (note->nComplete) {
			n = t->add_next(label, 0, check_pixmap);
		    } else {
			n = t->add_next(label, 0, echeck_pixmap);
		    }

		    if (n) {
			n->user_data(note);
			t->traverse_up();
		    } else {
			exit(1);
		    }

		}
	    }

	}
    }

}

void
NxTodoList::clear_tree()
{

    Fl_Toggle_Node *n = tree->traverse_start();
    while (n) {
	delete((char *) n->user_data());
	tree->remove(n);
	n = tree->traverse_start();
    }

}

void
NxTodoList::set_category(char *szCat, int refresh)
{

    char szRealCategory[CATEGORYS];

    if (!szCat[0])
	strcpy(szRealCategory, "All");
    else
	strcpy(szRealCategory, szCat);

    if (refresh)
	clear_tree();

    add_items(tree, szRealCategory, refresh);

    fill_categories();
    edit_category_list->label(szRealCategory);
    edit_category_list->hide();
    edit_category_list->show();
    note_category->label(szRealCategory);
    note_category->hide();
    note_category->show();

}

void
NxTodoList::reset_category(char *szCat)
{
    char szRealCategory[CATEGORYS];

    if (!szCat[0])
	strcpy(szRealCategory, "All");
    else
	strcpy(szRealCategory, szCat);

    fill_categories();
    edit_category_list->label(szRealCategory);
    edit_category_list->hide();
    edit_category_list->show();
    note_category->label(szRealCategory);
    note_category->hide();
    note_category->show();
}

void
NxTodoList::set_priority(const char *szP)
{

    edit_priority_list->label(szP);
    edit_priority_list->hide();
    edit_priority_list->show();

}

void
NxTodoList::priority_callback(Fl_Widget * fl, void *l)
{
    set_priority(((NxMenuButton *) fl)->text());
}

void
NxTodoList::category_callback(Fl_Widget * fl, void *l)
{
    set_category((char *) l);
}

void
NxTodoList::list_callback(Fl_Widget * fl, void *l)
{
    reset_category((char *) l);
}

void
NxTodoList::ResetRadioButtons()
{
    noDateRadio->value(0);
    chooseDateRadio->value(0);
    tomorrowDateRadio->value(0);
    EoWDateRadio->value(0);
    todayDateRadio->value(1);
    chooseDate->deactivate();
    date_type = TODAY;
}

void
NxTodoList::dueDate_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(due_date_window->GetWindowPtr(),
				   DEACTIVATE,
				   todo_edit_window->GetWindowPtr());
}

void
NxTodoList::chooseDate_callback(Fl_Widget * fl, void *l)
{
    NxTodoList *pThis = (NxTodoList *) l;
    time_t cur_time = time(0);
    tm *tt = localtime(&cur_time);
    int day = 0;
    int diff = 0;
    int w_day = 0;

    switch (date_type) {
    case NO_DATE:
	cur_time = NO_TIME;
	NxApp::Instance()->show_window(todo_edit_window->GetWindowPtr());
	break;
    case TODAY:
	NxApp::Instance()->show_window(todo_edit_window->GetWindowPtr());
	break;
    case TOMORROW:
	day = tt->tm_mday;
	day++;
	tt->tm_mday = day;
	cur_time = mktime(tt);
	NxApp::Instance()->show_window(todo_edit_window->GetWindowPtr());
	break;
    case END_OF_WEEK:
	w_day = tt->tm_wday;
	diff = 6 - w_day;
	tt->tm_mday += diff;
	cur_time = mktime(tt);
	NxApp::Instance()->show_window(todo_edit_window->GetWindowPtr());
	break;
    case CHOOSE_DATE:
	if (NO_TIME == set_time)
	    pThis->SetPickedDate(cur_time);
	else
	    pThis->SetPickedDate(set_time);
	if (fl == chooseDate) {
	    //date_window->GetWindowPtr()->show();
	    NxApp::Instance()->show_window(date_window->GetWindowPtr(),
					   DEACTIVATE,
					   due_date_window->GetWindowPtr());
	    pThis->DateCallback(setDate_callback);
	} else
	    cur_time = pThis->GetPickedDate();
	break;
    default:
	NxApp::Instance()->show_window(todo_edit_window->GetWindowPtr());
	break;
    }

    pThis->UpdateTime(cur_time);
}

void
NxTodoList::UpdateTime(long time)
{
    static char dateBuf[30];
    tm *tt = localtime((time_t *) & time);

    if (NO_DATE == time)
	sprintf(dateBuf, "No Date");
    else
	//strftime(dateBuf, 29, "%b %d, %y", tt);
	GetDateString(dateBuf, tt, sizeof(dateBuf), SHORT_YEAR);
    chooseDate->label(dateBuf);
    chooseDate->redraw();
    due_date->label(dateBuf);
    due_date->redraw();
    set_time = time;
}

void
NxTodoList::doneChoose_callback(Fl_Widget * fl, void *l)
{
    NxTodoList *pThis = (NxTodoList *) l;
    NxApp::Instance()->show_window(todo_edit_window->GetWindowPtr());
    chooseDate_callback(fl, pThis);
    pThis->ResetRadioButtons();
}

void
NxTodoList::setDate_callback(Fl_Widget * fl, void *l)
{
    NxTodoList *pThis = (NxTodoList *) l;

    pThis->date_window->GetWindowPtr()->hide();
    pThis->show_window(todo_edit_window->GetWindowPtr());

    NxApp::Instance()->show_window(due_date_window->GetWindowPtr(),
				   DEACTIVATE,
				   todo_edit_window->GetWindowPtr());

    if (pThis->GetPickedDate()) {
	pThis->UpdateTime(pThis->GetPickedDate());
    }
}

void
NxTodoList::done_callback(Fl_Widget * fl, long l)
{

    int bDeleteMe = 0;
    Fl_Toggle_Node *node = tree->selected();
    NxTodo *n = 0;

    if (!g_SearchFlag) {
	if (node) {
	    n = (NxTodo *) node->user_data();
	    DPRINT("done_callbackdone_callback  n->nId [%d]\n", n->nId);
	} else {
	    n = new NxTodo;
	    strcpy(n->szFile, "^");
	    bDeleteMe = 1;
	}

	NxApp::Instance()->set_catlist_window((Fl_Window *) todo_list_window->
					      GetEditCategoryWindowPtr());

	n->nPriority = edit_priority_list->value();
	n->nComplete = edit_complete->value();
	n->time = set_time;

	strcpy(n->szTitle, edit_title->value());
	strcpy(n->szCategory, edit_category_list->label());


	char *f = n->szFile;
	FILE *fd = 0;

	if (f[0] == '^') {
	    char tplate[128];
	    int tfd = 0;

	    snprintf(tplate, sizeof(tplate), "%s/%sXXXXXX", nx_inidir,
		     NX_INIFILE);
	    tfd = mkstemp(tplate);
	    fd = fdopen(tfd, "w+");
	} else
	    fd = fopen(f, "w+");

	if (fd) {

	    g_editor->SaveTo(fd);
	    fclose(fd);

	}

	DPRINT("g_EditFlag [%d]\n", g_EditFlag);

	if (g_EditFlag) {

	    int recno[1];
	    recno[0] = -1;
	    char id[16];
	    sprintf(id, "%d", n->nId);
	    db->Select(TODO, id, 0, recno, 1);
	    DPRINT("id [%s]\n", id);

	    if (recno[0] != -1) {
		DPRINT("GOING TO EDIT\n");
		edit_note(n, recno[0]);
	    }

	    g_EditFlag = 0;

	} else {

	    n->nId = 0;
	    n->nId = NxApp::Instance()->GetKey(db, TODO, 0);
	    n->nId++;

	    write_note(n);

	}

	if (AllFlag) {
	    set_category("All");
	    AllFlag = false;
	} else
	    set_category(n->szCategory);

	if (bDeleteMe)
	    delete n;

	note_list->position(0, 0);
    } else {
	g_SearchFlag = false;
    }
    NxApp::Instance()->show_window(todo_list_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxTodoList::doneLookup_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(todo_list_window->GetWindowPtr(),
				   ACTIVATE);
}

void
NxTodoList::cancelLookup_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(todo_list_window->GetWindowPtr(),
				   ACTIVATE);
}

char *
NxTodoList::formatLabel(const NxTodo * note, int pixels, bool fullpix)
{

    int width = 0;
    int dot_width = 0;
    int idx = 0;
    int tmp_pix;
    unsigned int jdx = 0;
    int date_len = 0;
    static char temp_date[33];
    static char date[33];
    char title[TITLE + 3];
    char temp_title[TITLE + 3];
    char *new_string = new char[TITLE + 33];
    char temp_string[TITLE + 33];
    tm *tt = localtime(&note->time);
    NxTodoList *pThis = (NxTodoList *) (NxApp::Instance());

    fl_font(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);
    dot_width = (int) fl_width("...");

    memset(temp_title, 0, sizeof(temp_title));
    memset(title, 0, sizeof(title));
    memset(new_string, 0, TITLE + 33);

    if (true == fullpix)
	sprintf(temp_title, "%s", note->szTitle);
    else
	sprintf(temp_title, "%d %s", note->nPriority, note->szTitle);

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
	    if (strlen(title) >= TITLE + 3)
		break;
	    sprintf(title, "%s ", title);
	    width = (int) fl_width(title);
	}
    }

    if (NO_TIME == note->time)
	strcpy(date, "No Date");
    else
	//strftime(date, 29,"%b %d", tt);
	pThis->GetDateString(date, tt, sizeof(date), SHORT_YEAR);
    date_len = strlen(date);
    memset(temp_date, 0, sizeof(temp_date));

    if (true == fullpix)
	tmp_pix = pixels / 2;
    else
	tmp_pix = (pixels / 2) - 20;

    width = (int) fl_width(date);
    if (width >= (tmp_pix)) {
	sprintf(temp_date, "%*.*s", 30, date_len, date);
	strcpy(date, temp_date);
    }

    if (NO_TIME == note->time)
	strcpy(date, "No Date");
    else
	//strftime(date, 29,"%b %d", tt);
	pThis->GetDateString(date, tt, sizeof(date), SHORT_YEAR);

    idx = 0;
    jdx = 0;
    width = (int) fl_width(date);
    while (width >= (tmp_pix)) {
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

char *
NxTodoList::formatString(const NxTodo * note, int pixels)
{
    int width = 0;
    int dot_width = 0;
    int idx = 0;
    char *new_string = new char[TITLE + 3];

    fl_font(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);
    width = (int) fl_width(note->szTitle);
    dot_width = (int) fl_width("...");

    if (width <= pixels) {
	return (char *) note->szTitle;
    } else {
	while (width > pixels) {
	    idx++;
	    memset(new_string, 0, TITLE + 3);
	    strncpy(new_string, note->szTitle, strlen(note->szTitle) - idx);
	    width = (int) fl_width(new_string) + dot_width;
	}
	sprintf(new_string, "%s...", new_string);
	return new_string;
    }
    return (char *) note->szTitle;
}

NxTodo *
NxTodoList::searchString(const char *searchVal)
{
    static int cur_record = 0;
    static int rec_array[255];
    int jdx;
    char *needle = strup(searchVal, strlen(searchVal));

    if (0 == cur_record) {
	for (int idx = 0; idx < 255; idx++) {
	    rec_array[idx] = -1;
	}
	//Select all records
	db->Select(TODO, rec_array, 255);
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
	int catid_array[1];
	char catid[8];
	found = false;

	catid_array[0] = -1;

	jdx = rec_array[cur_record];
	if (-1 == jdx) {
	    cur_record++;
	    continue;
	}

	db->Extract(TODO, jdx, 1, catid);
	db->Select(CATEGORY, catid, 0, catid_array, 1);
	if (-1 != catid_array[0])
	    db->Extract(CATEGORY, catid_array[0], 1, note->szCategory);
	else
	    strcpy(note->szCategory, "Unfiled");

	// id
	char id[4];
	sprintf(id, "%d", note->nId);
	db->Extract(TODO, jdx, 0, id);

	char buf1[4];
	db->Extract(TODO, jdx, 2, buf1);
	note->nComplete = atoi(buf1);

	char buf2[4];
	db->Extract(TODO, jdx, 3, buf2);
	note->nPriority = atoi(buf2);

	db->Extract(TODO, jdx, 4, note->szTitle);
	char *haystack = strup(note->szTitle, TITLE);
	if (strstr(haystack, needle))
	    found = true;
	delete[]haystack;
	haystack = 0;

	db->Extract(TODO, jdx, 5, note->szFile);

	char buf3[8];
	db->Extract(TODO, jdx, 6, buf3);
	note->time = strtol(buf3, NULL, 10);

	//haystack = strup(note->szFile, DESC);
	//if (strstr(haystack, needle))
	//      found = true;
	//delete[] haystack;
	//haystack = 0;

	found |= searchFile(searchVal, note->szFile);

	if (found) {
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

bool NxTodoList::checkDate(NxTodo * note, time_t fromTime, time_t toTime)
{
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
	localtime(&note->time);
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
    if (((fd <= d) && (fm <= m) && (fy <= y))
	&& ((td >= d) && (tm >= m) && (ty >= y))) {
	return true;
    }

    return false;
}

NxTodo *
NxTodoList::searchDate(time_t fromTime, time_t toTime)
{
    static int cur_record = 0;
    static int rec_array[255];
    int jdx;

    if (0 == cur_record) {
	for (int idx = 0; idx < 255; idx++) {
	    rec_array[idx] = -1;
	}
	db->Select(TODO, rec_array, 255);
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

	int catid_array[1];
	char catid[8];

	catid_array[0] = -1;
	db->Extract(TODO, jdx, 1, catid);
	db->Select(CATEGORY, catid, 0, catid_array, 1);
	if (-1 != catid_array[0])
	    db->Extract(CATEGORY, catid_array[0], 1, note->szCategory);
	else
	    strcpy(note->szCategory, "Unfiled");

	// id
	char id[4];
	sprintf(id, "%d", note->nId);
	db->Extract(TODO, jdx, 0, id);

	char buf1[4];
	db->Extract(TODO, jdx, 2, buf1);
	note->nComplete = atoi(buf1);

	char buf2[4];
	db->Extract(TODO, jdx, 3, buf2);
	note->nPriority = atoi(buf2);

	db->Extract(TODO, jdx, 4, note->szTitle);
	db->Extract(TODO, jdx, 5, note->szFile);

	char buf3[8];
	db->Extract(TODO, jdx, 6, buf3);
	note->time = strtol(buf3, NULL, 10);

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
NxTodoList::UpdateFromButton()
{
    static char buf[30];
    tm *tt = localtime(&fromTime);

    //strftime(buf, 29, "%b %d, %y", tt);
    GetDateString(buf, tt, sizeof(buf), SHORT_YEAR);
    fromDateButton->label(buf);
    fromDateButton->redraw();
}

void
NxTodoList::UpdateToButton()
{
    static char buf[30];
    tm *tt = localtime(&toTime);

    //strftime(buf, 29, "%b %d, %y", tt);
    GetDateString(buf, tt, sizeof(buf), SHORT_YEAR);
    toDateButton->label(buf);
    toDateButton->redraw();
}

void
NxTodoList::fromDate_callback(Fl_Widget * fl, void *l)
{
    NxTodoList *pThis = (NxTodoList *) l;

    pThis->date_window->GetWindowPtr()->hide();

    NxApp::Instance()->show_window(todo_lookup_window->GetWindowPtr(),
				   DEACTIVATE,
				   todo_edit_window->GetWindowPtr());
    if (pThis->GetPickedDate()) {
	pThis->fromTime = pThis->GetPickedDate();
	pThis->UpdateFromButton();
    }
}

void
NxTodoList::toDate_callback(Fl_Widget * fl, void *l)
{
    NxTodoList *pThis = (NxTodoList *) l;

    pThis->date_window->GetWindowPtr()->hide();

    NxApp::Instance()->show_window(todo_lookup_window->GetWindowPtr(),
				   DEACTIVATE,
				   todo_edit_window->GetWindowPtr());
    if (pThis->GetPickedDate()) {
	pThis->toTime = pThis->GetPickedDate();
	pThis->UpdateToButton();
    }
}

void
NxTodoList::fromCalendar_callback(Fl_Widget * fl, void *l)
{
    NxTodoList *pThis = (NxTodoList *) l;

    pThis->SetPickedDate(pThis->fromTime);
    NxApp::Instance()->show_window(date_window->GetWindowPtr(),
				   DEACTIVATE,
				   todo_lookup_window->GetWindowPtr());
    pThis->DateCallback(fromDate_callback);
}

void
NxTodoList::toCalendar_callback(Fl_Widget * fl, void *l)
{
    NxTodoList *pThis = (NxTodoList *) l;

    pThis->SetPickedDate(pThis->toTime);
    NxApp::Instance()->show_window(date_window->GetWindowPtr(),
				   DEACTIVATE,
				   todo_lookup_window->GetWindowPtr());
    pThis->DateCallback(toDate_callback);
}

void
NxTodoList::searchLookup_callback(Fl_Widget * fl, void *l)
{
    if ((0 == stringCheck->value()) && (0 == dateCheck->value())) {
	NxApp::Instance()->show_window(error_window->GetWindowPtr(),
				       DEACTIVATE,
				       todo_lookup_window->GetWindowPtr());
	return;
    }

    g_SearchFlag = false;

    int total_found = 0;
    char *needle = 0;
    NxTodo *note = 0;


    results_table->Init(255);

    if (true == stringCheck->value() && false == dateCheck->value()) {
	char *searchVal = (char *) lookup_input->value();
	needle = strup(searchVal, strlen(searchVal));
	while ((note = searchString(searchVal)) != NULL) {
	    results_table->Add(total_found, note);
	    char *label =
		formatLabel(note, results_table->text_width(), true);
	    results_table->set_value(total_found, 0, label);
	    total_found++;
	}
    }

    if (false == stringCheck->value() && true == dateCheck->value()) {
	while ((note = searchDate(fromTime, toTime)) != NULL) {
	    results_table->Add(total_found, note);
	    char *label =
		formatLabel(note, results_table->text_width(), true);
	    results_table->set_value(total_found, 0, label);
	    total_found++;
	}
    }

    if (true == stringCheck->value() && true == dateCheck->value()) {
	char *searchVal = (char *) lookup_input->value();
	needle = strup(searchVal, strlen(searchVal));
	while ((note = searchString(searchVal)) != NULL) {
	    if (true == checkDate(note, fromTime, toTime)) {
		results_table->Add(total_found, note);
		char *label =
		    formatLabel(note, results_table->text_width(), true);
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
	results_message->show();
    }

    results_table->rows(total_found);

    NxApp::Instance()->show_window(todo_list_window->GetWindowPtr());
    NxApp::Instance()->show_window(todo_results_window->GetWindowPtr(),
				   DEACTIVATE,
				   todo_list_window->GetWindowPtr());
}

void
NxTodoList::lookup_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(todo_lookup_window->GetWindowPtr(),
				   DEACTIVATE,
				   todo_list_window->GetWindowPtr());
    lookup_input->value("");
    stringCheck->value(false);
    dateCheck->value(false);
    Fl::focus((Fl_Widget *) lookup_input);
}

void
NxTodoList::select_note(NxTodo * note)
{
    Fl_Toggle_Node *n = tree->traverse_start();
    int note_id = note->nId;
    int n_id;

    while (NULL != n) {
	n_id = ((NxTodo *) n->user_data())->nId;
	if (n_id == note_id) {
	    tree->select_range(n, n, 0);
	    break;
	}
	n = tree->traverse_forward();
    }
}

void
NxTodoList::viewRecord(int recno)
{
    DPRINT("recno: [%d]\n", recno);
    NxApp::Instance()->set_catlist_window((Fl_Window *) todo_edit_window->
					  GetEditCategoryWindowPtr());

    NxTodo *note = new NxTodo;

    int rec_array[1];

    rec_array[0] = -1;
    char c_recno[8];
    sprintf(c_recno, "%d", recno);
    db->Select(TODO, c_recno, 0, rec_array, 1);
    recno = rec_array[0];

    int catid_array[1];
    char catid[8];

    catid_array[0] = -1;
    db->Extract(TODO, recno, 1, catid);
    db->Select(CATEGORY, catid, 0, catid_array, 1);
    if (-1 != catid_array[0])
	db->Extract(CATEGORY, catid_array[0], 1, note->szCategory);
    else
	strcpy(note->szCategory, "Unfiled");

    char id[4];
    sprintf(id, "%d", note->nId);
    db->Extract(TODO, recno, 0, id);

    char buf1[4];
    db->Extract(TODO, recno, 2, buf1);
    note->nComplete = atoi(buf1);

    db->Extract(TODO, recno, 3, buf1);
    note->nPriority = atoi(buf1);

    db->Extract(TODO, recno, 4, note->szTitle);
    db->Extract(TODO, recno, 5, note->szFile);

    char buf3[8];
    db->Extract(TODO, recno, 6, buf3);
    note->time = strtol(buf3, NULL, 10);

    FILE *fd = fopen(note->szFile, "r");
    g_editor->Clear();
    g_editor->LoadFrom(fd);
    g_editor->MoveTo(0, 0);
    fclose(fd);

    _fill_form(note);

    g_SearchFlag = false;
    g_EditFlag = true;
    select_note(note);
    edit_category_list->label(note->szCategory);

    NxApp::Instance()->show_window(todo_edit_window->GetWindowPtr());
    delete note;

}

void
NxTodoList::view_callback(Fl_Widget * fl, void *l)
{
    NxTodo *n = 0;
    if (Fl::event_clicks()) {
	g_SearchFlag = false;
	g_EditFlag = true;
	NxApp::Instance()->set_catlist_window((Fl_Window *) todo_edit_window->
					      GetEditCategoryWindowPtr());
	n = (NxTodo *) results_table->selected();

	if (n) {
	    char tempCat[CATEGORYS];
	    memset(tempCat, 0, CATEGORYS);

	    strcpy(tempCat, n->szCategory);
	    if (!tempCat[0]) {
		strcpy(tempCat, "Unfiled");
	    }
	    g_editor->Clear();
	    if (n->szFile[0] != '^') {
		FILE *fd = fopen(n->szFile, "r");
		g_editor->LoadFrom(fd);
		g_editor->MoveTo(0, 0);
		fclose(fd);
	    }
	    select_note(n);
	    _fill_form(n);
	    edit_category_list->label(tempCat);
	}
	NxApp::Instance()->show_window(todo_edit_window->GetWindowPtr());
    }
}

void
NxTodoList::fill_categories()
{

    char ret_buf[CAT_NAME];
    int rec_count = db->NumRecs(CATEGORY);

    for (int idx = 0; idx < CAT_NUM; idx++) {
	cat_list[idx]->clear();
	cat_list[idx]->add("All");
	for (int jdx = 1; jdx <= rec_count; jdx++) {
	    db->Extract(CATEGORY, jdx, 1, ret_buf);
	    cat_list[idx]->add(ret_buf);
	}
	cat_list[idx]->label(const_cast <
			     char *>((char *) cat_list[idx]->text(0)));
    }
    todo_list_window->GetEditCategoryPtr()->clear_tree();
    todo_list_window->GetEditCategoryPtr()->add_items(todo_list_window->
						      GetEditCategoryPtr()->
						      get_category_tree());
    todo_edit_window->GetEditCategoryPtr()->clear_tree();
    todo_edit_window->GetEditCategoryPtr()->add_items(todo_edit_window->
						      GetEditCategoryPtr()->
						      get_category_tree());

}
