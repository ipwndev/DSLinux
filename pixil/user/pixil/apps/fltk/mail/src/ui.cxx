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
#include <nxscroll.h>
#include <FL/Fl_Pixmap.H>

#include "ui.h"
#include "callbacks.h"
#include "menu_defs.h"
#include "db_defs.h"

////////////////////////////////////////////////////////////////////////////////
// Global Access Point

NxMail *
    NxMail::_inst =
    0;

extern int
    exit_flag;

#ifdef NOTUSED

NxCategoryList *
    NxMail::main_category;
NxCategoryList *
    NxMail::editor_category;
NxCategoryList *
    NxMail::view_category;

#endif

////////////////////////////////////////////////////////////////////////////////
// NxMail Constructor

NxMail::NxMail(int argc, char *argv[])
    :
NxApp()
{

    ////////////////////////////////////////
    // Global Access Point Initialization

    _inst = this;

    NxApp::Instance()->set_keyboard(argc, argv);
    NxApp::Instance()->set_about(about_email);


    ////////////////////////////////////////
    // Initialize databases

#ifdef NOTUSED
    db = new NxDb(argc, argv);
    // Open or Create category database
    if (!db->Open((string) CAT_DB, &catFile, catFields, 0)) {

	if (db->Create((string) CAT_DB, &catFile, catFields, 0)) {

	    if (!db->Open((string) CAT_DB, &catFile, catFields, 0)) {
		exit(-1);
	    }
#if DATABASE_OPTION_SUPPORTED
	    char *record1 = 0;
	    record1 = CatRecord(0, "Inbox");
	    db->Insert(CAT_DB, record1);
	    delete[]record1;

	    char *record2 = 0;
	    record2 = CatRecord(1, "Outbox");
	    db->Insert(CAT_DB, record2);
	    delete[]record2;

	    char *record3 = 0;
	    record3 = CatRecord(2, "Deleted");
	    db->Insert(CAT_DB, record3);
	    delete[]record3;

	    char *record4 = 0;
	    record4 = CatRecord(3, "Filed");
	    db->Insert(CAT_DB, record4);
	    delete[]record4;

	    char *record5 = 0;
	    record5 = CatRecord(4, "Draft");
	    db->Insert(CAT_DB, record5);
	    delete[]record5;
#endif

	} else {

	    exit(-1);

	}
    }
#endif

    ////////////////////////////////////////
    // First, create the windows

    containerWindow = new NxWindow(W_W, W_H, APP);
    MakeMainWindow();
    MakeEditorWindow();
    MakeSettingsWindow();
    MakeViewWindow();
    containerWindow->end();

    ////////////////////////////////////////
    // Now, set up the signals to catch 
    // exceptions

    //  signal(SIGINT , CloseNanoMail);
    //  signal(SIGTERM, CloseNanoMail);
    //  signal(SIGHUP , CloseNanoMail);

    ////////////////////////////////////////
    // Now, make the calsses for the 
    // settings and the mail server

    settings = new mailSettings();
    settings->load_settings();

    current_account = 0;
    engine =
	new mailEngine(settings->
		       get_value(settings->get_account_name(current_account),
				 "server"),
		       atoi(settings->
			    get_value(settings->
				      get_account_name(current_account),
				      "port")), MAIL_POP3);
    printf("2\n");

    ////////////////////////////////////////
    // Category lists

#ifdef NOTUSED
    main_category = mainWindow->category_list;
    main_category->select(ChangeCatCB);

    editor_category = editorWindow->category_list;
    editor_category->select(ChangeCatCB);

    settings_category = settingsWindow->category_list;
    settings_category->select(ChangeAcctCB);

    view_category = viewWindow->category_list;
    view_category->select(ChangeCatCB);
#endif

#ifdef NOTUSED
    cat_list[0] = main_category;
    cat_list[1] = editor_category;
    cat_list[2] = settings_category;
    cat_list[3] = view_category;

    //  add_items(table, "All");
    cat_list[0]->label("All");
    cat_list[1]->label("All");
    cat_list[3]->label("All");

    set_catlist_window((Fl_Window *) mainWindow->GetEditCategoryWindowPtr());
#endif

    ////////////////////////////////////////
    // FLNX-Colosseum IPC

    set_shown_window(mainWindow->GetWindowPtr());
    Add_Fd("nxmail", _ClientIPCHandler);

}

NxMail::~NxMail()
{
#ifdef NOTUSED
    db->Close(CAT_DB);
    delete db;
#endif

    db = 0;
    if (accountArray)
	free(accountArray);
}

void
NxMail::exit_callback(Fl_Widget * fl, void *o)
{
    exit_flag = 1;
    NxMail::Inst()->containerWindow->hide();
    exit(0);
}

////////////////////////////////////////////////////////////////////////////////
// Window Messages
Fl_Window *
NxMail::GetMainWindow()
{
    if (containerWindow)
	return containerWindow;
    else
	return 0;
}

void
NxMail::ShowDefaultWindow()
{
    show_window(mainWindow->GetWindowPtr());
}

////////////////////////////////////////////////////////////////////////////////
// Global Access Point

NxMail *
NxMail::Inst()
{
    if (_inst == NULL) {
	printf("_inst is really messed up. Bail!\n");
	exit(-1);
    }

    return _inst;
}

////////////////////////////////////////////////////////////////////////////////
// Mail Engine Messages

char *
NxMail::EngineGetSettings(char *szSetting)
{
    return (settings->get_value(settings->get_account_name(current_account),
				szSetting));
}

MAILERROR NxMail::EngineOpenSession()
{

    return (engine->
	    open_session(EngineGetSettings("username"),
			 EngineGetSettings("password")));
}

void
NxMail::MainShowWindow()
{
#ifdef NOTUSED
    set_catlist_window((Fl_Window *) mainWindow->GetEditCategoryWindowPtr());
#endif
    show_window(mainWindow->GetWindowPtr());
}

void
NxMail::EditorShowWindow()
{
#ifdef NOTUSED
    set_catlist_window((Fl_Window *) editorWindow->
		       GetEditCategoryWindowPtr());
#endif
    show_window(editorWindow->GetWindowPtr());
}

void
NxMail::SettingsShowWindow()
{
    show_window(settingsWindow->GetWindowPtr());
}

void
NxMail::ViewerShowWindow()
{
#ifdef NOTUSED
    set_catlist_window((Fl_Window *) viewWindow->GetEditCategoryWindowPtr());
#endif
    show_window(viewWindow->GetWindowPtr());
}

MAILERROR
    NxMail::EngineSendMessage(char *server, int port,
			      nxmail_header_t * header, char *body, int size)
{
    return engine->send_message(server, port, header, body, size);
}

////////////////////////////////////////////////////////////////////////////////
// Exception handling

void
NxMail::CloseNanoMail(int signal)
{
    extern char *listBuffer;

    printf("Shutting down Nanomail right now!\n");

    m_List->clear();

    if (listBuffer)
	free(listBuffer);

    delete engine;

    exit(0);
}

////////////////////////////////////////////////////////////////////////////////
// Database messages

char *
NxMail::CatRecord(int catid, string cat_name)
{

    char *rec = new char[MAXRECSIZ];

    memset(rec, 0, MAXRECSIZ);
    put16(&rec[catFields[0].offset], catid);
    strcpy(&rec[catFields[1].offset], cat_name.c_str());

    return rec;

}

////////////////////////////////////////////////////////////////////////////////
// FLNX-Colosseum IPC messages

void
NxMail::ClientIPCHandler(int fd, void *o, int ipc_id)
{

    char *tokenMsg = new char[MAX_LENGTH];
    char *passMsg = new char[MAX_LENGTH];

    if (o == NULL) {

	int length = MAX_LENGTH;
	ipc_id = NxApp::Instance()->Read_Fd(passMsg, &length);


	if (passMsg == NULL)
	    return;
	else
	    strcpy(tokenMsg, passMsg);

    } else if (ipc_id == -1) {

	strcpy(tokenMsg, (char *) o);
	strcpy(passMsg, (char *) o);
	ipc_id = NxApp::Instance()->Find_Fd("nxaddress");

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
    strcpy(msg_cmd, tmp);

    // DATA_ITEM
    tmp = strtok(NULL, TOKEN);
    strcpy(data_item, tmp);

    //  DPRINT("Expoding Message... %s, %s, %s.\n", service, msg_cmd, data_item);

    if (strcmp(msg_cmd, "EXECUTE") == 0) {

	if (!NxApp::Instance()->VerifyClient(service))
	    return;

	if (strcmp(data_item, "search") == 0) {

	    char *searchStr = new char[MAX_LENGTH];
	    char *width = new char[4];

	    char *data = strtok(NULL, TOKEN);
	    strcpy(searchStr, data);
	    data = strtok(NULL, TOKEN);
	    strcpy(width, data);

	    //      NxMail::Inst()->ExecuteSearch(ipc_id, searchStr, atoi(width));

	    delete[]searchStr;
	    delete[]width;
	    searchStr = width = NULL;

	}

	if (strcmp(data_item, "showrecord") == 0) {

	    int recno = 0;
	    char *data = strtok(NULL, TOKEN);

	    recno = atoi(data);
	    //viewRecord(recno);
	}

    }
    // Memory Mangement
    delete[]service;
    delete[]msg_cmd;
    delete[]data_item;
    delete[]tokenMsg;
    service = msg_cmd = data_item = tokenMsg = NULL;

    NxApp::Instance()->ServerIPCHandler(fd, ipc_id, (char *) passMsg);

}

void
NxMail::ExecuteSearch(int ipc_id, char *searchStr, int width)
{

}

////////////////////////////////////////////////////////////////////////////////
// Make Windows messages

void
NxMail::MakeMainWindow()
{

    // Make a NxPimWindow
    printf("Opening category database [%s]\n", CAT_DB);

    mainWindow = new NxPimWindow(APP, mainMenuItems,
				 0, "", "",
				 (void (*)(const char *)) MainSetCategory);

    add_window((Fl_Window *) mainWindow->GetWindowPtr());

    // Message list
    // We're going to switch this one out with Flv_Table_Child ... later... later.HEIGHT
    m_List = new NxBrowser(1, 30, W_W - 5, W_H - 85);
    m_List->type(FL_HOLD_BROWSER);
    m_List->movable(false);
    //m_List->color(NxApp::Instance()->getGlobalColor(APP_BG));
    //m_List->textcolor(NxApp::Instance()->getGlobalColor(APP_FG));
    m_List->callback((Fl_Callback *) get_message_callback);

    mainWindow->add((Fl_Widget *) m_List);

    l_Status = new NxOutput(1, BUTTON_Y - BUTTON_HEIGHT - 15, W_W - 4, 15);
    l_Status->value("Welcome!");
    l_Status->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    mainWindow->add((Fl_Widget *) l_Status);

    // Buttons

    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "New Mail");
	o->callback((Fl_Callback *) new_message_callback);
	NxApp::Instance()->def_font((Fl_Widget *) o);

	mainWindow->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 63, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Get Mail");
	o->callback((Fl_Callback *) check_mail_callback);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	mainWindow->add((Fl_Widget *) o);
    }

    {
	NxButton *o = new NxButton(BUTTON_X + 126, BUTTON_Y, BUTTON_WIDTH,
				   BUTTON_HEIGHT, "Delete");
	o->callback((Fl_Callback *) delete_message_callback);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	mainWindow->add((Fl_Widget *) o);
    }

}

#define PDA

void
NxMail::MakeEditorWindow()
{

    // Make a NxPimWindow
    editorWindow = new NxPimWindow(APP, editorMenuItems,
				   0, "", "",
				   (void (*)(const char *)) MainSetCategory);

    add_window((Fl_Window *) editorWindow->GetWindowPtr());

    int y = 30;

    // To Line
    i_To = new NxInput(53, y, W_W - 55, 20, "To:");
#ifdef PDA
    i_To->box(FL_BOTTOM_BOX);
#endif
    NxApp::Instance()->def_font(i_To);
    i_To->align(FL_ALIGN_LEFT);
    i_To->callback(NxApp::Instance()->pasteTarget_callback);


    editorWindow->add((Fl_Widget *) i_To);

    // Carbon Copy
    i_CC = new NxInput(53, y += 22, W_W - 55, 20, "CC:");
    NxApp::Instance()->def_font(i_CC);
    i_CC->align(FL_ALIGN_LEFT);
    i_CC->callback(NxApp::Instance()->pasteTarget_callback);

    editorWindow->add((Fl_Widget *) i_CC);

    // Subject Line
    i_Subject = new NxInput(53, y += 22, W_W - 55, 20, "Subject:");
    NxApp::Instance()->def_font(i_Subject);
    i_Subject->align(FL_ALIGN_LEFT);
    i_Subject->callback(NxApp::Instance()->pasteTarget_callback);

    editorWindow->add((Fl_Widget *) i_Subject);

    // The Email message
    i_Message = new NxMultilineInput(2, y += 30, W_W - 4, (W_W - y - 30));
    editorWindow->add((Fl_Widget *) i_Message);

    // Buttons
    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Send");
	o->callback((Fl_Callback *) EditorSendCB);
	NxApp::Instance()->def_font((Fl_Widget *) o);
	editorWindow->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 63, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Cancel");
	o->callback((Fl_Callback *) EditorCancelCB);
	editorWindow->add((Fl_Widget *) o);
    }

}

void
NxMail::MakeSettingsWindow()
{


    // Make a NxPimWindow
    settingsWindow = new NxPimWindow(APP, settingsMenuItems,
				     0, "", "", (void (*)(const char *))
				     MainSetCategory);

    add_window((Fl_Window *) settingsWindow->GetWindowPtr());


    ////////////////////////////////////////
    // Init local variables

    int x = BUTTON_X + 100;
    int y = 0;

    accountArray = 0;
    accountCount = 0;

    m_Accounts = new NxMenuButton(CL_X, CL_Y, CL_W, CL_H, "");

    m_Accounts->callback(SettingsAccountCB);
    settingsWindow->add((Fl_Widget *) m_Accounts);

    // Server name
    i_Server = new NxInput(x, y += 25, 100, 20, "Server:              ");

    settingsWindow->add((Fl_Widget *) i_Server);

    // Port

#ifdef NOTUSED
    i_Port = new NxInput(x, y += 25, 50, 20, "Port:                     ");

    settingsWindow->add((Fl_Widget *) i_Port);
#endif

    // Make a new radio button group
    g_Radio = new Fl_Group(x, y += 25, 100, 20, "");
    g_Radio->box(FL_FLAT_BOX);
    g_Radio->color(NxApp::Instance()->getGlobalColor(APP_BG));

    NxButton *b_IMAP = new NxButton(140, y, 10, 10, "IMAP");
    b_IMAP->type(102);
    b_IMAP->align(FL_ALIGN_RIGHT);
    b_IMAP->deactivate();

    NxButton *b_POP3 = new NxButton(x, y, 10, 10, "POP3");
    b_POP3->type(102);
    b_POP3->align(FL_ALIGN_RIGHT);
    b_POP3->setonly();

    g_Radio->end();
    g_Radio->hide();

    settingsWindow->add((Fl_Widget *) g_Radio);

    // User information
    i_Name = new NxInput(x, y += 25, 100, 20, "Username:         ");

    settingsWindow->add((Fl_Widget *) i_Name);

    i_Pass = new NxSecretInput(x, y += 25, 100, 20, "Password:          ");

    settingsWindow->add((Fl_Widget *) i_Pass);

    i_SMTPServer = new NxInput(x, y += 25, 100, 20, "SMTP Server:     ");

    settingsWindow->add((Fl_Widget *) i_SMTPServer);

    i_SMTPName = new NxInput(x, y += 25, 100, 20, "SMTP Username:");
    settingsWindow->add((Fl_Widget *) i_SMTPName);

    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Save");
	o->callback((Fl_Callback *) SettingsSaveCB);
	NxApp::Instance()->def_font((Fl_Widget *) o);

	settingsWindow->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 63, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Cancel");
	o->callback((Fl_Callback *) SettingsCancelCB);
	NxApp::Instance()->def_font((Fl_Widget *) o);

	settingsWindow->add((Fl_Widget *) o);
    }

}

void
NxMail::MakeViewWindow()
{
    // Make a NxPimWindow
    viewWindow = new NxPimWindow(APP, viewMenuItems,
				 0, "", "",
				 (void (*)(const char *)) MainSetCategory);

    add_window((Fl_Window *) viewWindow->GetWindowPtr());


    NxScroll *scroll = new NxScroll(1, 30, W_W - 1, (W_H - 100));
    m_View = new NxMultilineOutput(1, 30, W_W - 13, W_H + 2048);
    //m_View->set_wrap(1);
    scroll->end();
    scroll->scrollbar.size(12, W_H - 22 - 79);
    scroll->hscrollbar.size(W_W - 1, 12);

    viewWindow->add((Fl_Widget *) scroll);

    m_Mime = new NxBrowser(1, W_H - 65, W_W - 2, 32);
    m_Mime->type(FL_HOLD_BROWSER);

    viewWindow->add((Fl_Widget *) m_Mime);

    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Save");
	o->callback((Fl_Callback *) ViewerSaveMsgCB);
	viewWindow->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X + 63, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Delete");
	o->box(FL_SHADOW_BOX);
	o->callback((Fl_Callback *) ViewerCloseCB);
	viewWindow->add((Fl_Widget *) o);
    }

}

////////////////////////////////////////////////////////////////////////////////
// mainWindow messages

void
NxMail::MainCloseCB(Fl_Widget * fl, void *o)
{
    //  CloseNanoMail(0);
}

void
NxMail::MainSetCategory(char *szCat)
{

}

void
NxMail::MainSetStatus(const char *status)
{
    l_Status->value(status);
    l_Status->redraw();
}

////////////////////////////////////////////////////////////////////////////////
// viewerWindow messages


void
NxMail::ViewerAccountCB(Fl_Widget * fl, void *o)
{

    NxMail *inst = NxMail::Inst();

    inst->current_account = ((Fl_Menu_Button *) fl)->value();

    printf("Now using the [%s] account!\n",
	   inst->settings->get_account_name(inst->current_account));

    inst->engine->reset_engine(inst->settings->
			       get_value(inst->settings->
					 get_account_name(inst->
							  current_account),
					 "server"),
			       atoi(inst->settings->
				    get_value(inst->settings->
					      get_account_name(inst->
							       current_account),
					      "port")), MAIL_POP3);

    ((Fl_Menu_Button *) fl)->label(inst->settings->
				   get_account_name(inst->current_account));

    ((Fl_Menu_Button *) fl)->redraw();
}

// We're doing it this way why?
// 1. Keep the email engine callbacks seperate
// 2. Further expansion of the email client (don't edit the email engine callbacks)
void
NxMail::ViewerReplyMsgCB(Fl_Widget * fl, void *o)
{
    reply_message_callback(REPLY_SINGLE);
}

void
NxMail::ViewerReplyAllMsgCB(Fl_Widget * fl, void *o)
{
    reply_message_callback(REPLY_ALL);
}

void
NxMail::ViewerReplyFwdMsgCB(Fl_Widget * fl, void *o)
{
    reply_message_callback(REPLY_FORWARD);
}

void
NxMail::ViewerDeleteMsgCB(Fl_Widget * fl, void *o)
{
    delete_message_callback();
}

void
NxMail::ViewerSaveMsgCB(Fl_Widget * fl, void *o)
{
    save_message_callback();
}

void
NxMail::ViewerViewMsgCB(Fl_Widget * fl, void *o)
{
    view_message_callback();
}

void
NxMail::ViewerCloseCB(Fl_Widget * fl, void *o)
{
    delete_message_callback();
}

//////////////////////////////////////////////////////////////////////////////// 
// editorWindow messages

void
NxMail::EditorSendCB(Fl_Widget * fl, void *o)
{
    NxMail *inst = NxMail::Inst();
    send_message_callback();
    inst->EditorClearFields();
    inst->editorWindow->GetWindowPtr()->hide();
}

void
NxMail::EditorCancelCB(Fl_Widget * fl, void *o)
{
    NxMail *inst = NxMail::Inst();
    inst->EditorClearFields();
    inst->MainShowWindow();
}

void
NxMail::EditorClearFields(void)
{
    i_To->value("");
    i_CC->value("");
    i_Subject->value("");
    i_Message->value("");
}

void
NxMail::EditorSetFields(char *to, char *cc, char *subject)
{
    i_To->value(to);
    i_CC->value(cc);
    i_Subject->value(subject);
    i_Message->value("");
}

void
NxMail::EditorIndentText(char *ptr)
{
    i_Message->insert("> ");
    i_Message->insert(ptr);
    i_Message->insert("\n");
}

////////////////////////////////////////////////////////////////////////////////
// settingsWindow messages

void
NxMail::SettingsSaveCB(Fl_Widget * fl, void *o)
{

    NxMail *inst = NxMail::Inst();
    int changed = 0;

    // Make sure that we have the most recent values for the current one
    inst->SettingsUpdateValues(inst->current_account);

    changed = inst->settings->confirm_all_changes();

    if (changed) {
	printf("The mail settings have changed!\n");

	inst->settings->save_settings();

	inst->engine->reset_engine(inst->settings->
				   get_value(inst->settings->
					     get_account_name(inst->
							      current_account),
					     "server"),
				   atoi(inst->settings->
					get_value(inst->settings->
						  get_account_name(inst->
								   current_account),
						  "port")), MAIL_POP3);
    }

    inst->MainShowWindow();

}


void
NxMail::SettingsCancelCB(Fl_Widget * fl, void *o)
{
    NxMail *inst = NxMail::Inst();
    inst->settings->reject_all_changes();
    inst->MainShowWindow();
}


void
NxMail::SettingsAccountCB(Fl_Widget * fl, void *o)
{

    NxMail *inst = NxMail::Inst();

    // Update the current values for the given account
    inst->SettingsUpdateValues(inst->current_account);

    inst->current_account = ((Fl_Menu_Button *) fl)->value();

    // Now update the fields for the new accont
    inst->SettingsUpdateFields();
}

void
NxMail::SettingsUpdateFields()
{

    int a;

    int acount = settings->get_account_count();

    /* Set the accounts */

    if (acount != accountCount) {
	/* Reallocate the array of menu items */
	if (accountArray)
	    free(accountArray);

	accountArray =
	    (Fl_Menu_Item *) calloc((acount + 1) * sizeof(Fl_Menu_Item), 1);
	accountCount = acount;
    }

    for (a = 0; a < acount; a++) {
	accountArray[a].label(settings->get_account_name(a));
	accountArray[a].shortcut(0);
    }

    accountArray[acount].label(0);

    m_Accounts->menu(accountArray);
    m_Accounts->label(settings->get_account_name(current_account));
    m_Accounts->redraw();

    // Set the accounts
    i_Server->value(settings->
		    get_value(settings->get_account_name(current_account),
			      "server"));

#ifdef NOTUSED
    i_Port->value(settings->
		  get_value(settings->get_account_name(current_account),
			    "port"));
#endif

    // b_POP3->set();

    i_Name->value(settings->
		  get_value(settings->get_account_name(current_account),
			    "username"));
    i_Pass->value(settings->
		  get_value(settings->get_account_name(current_account),
			    "password"));
    i_SMTPServer->value(settings->
			get_value(settings->get_account_name(current_account),
				  "smtpserver"));
    i_SMTPName->value(settings->
		      get_value(settings->get_account_name(current_account),
				"smtpname"));

}

void
NxMail::SettingsUpdateValues(int account)
{
    // Save the settings

    printf("Recording temp values for %s\n",
	   settings->get_account_name(account));

    settings->set_temp_value(settings->get_account_name(account),
			     "server", i_Server->value());

    settings->set_temp_value(settings->get_account_name(account),
			     "username", i_Name->value());

    settings->set_temp_value(settings->get_account_name(account),
			     "password", i_Pass->value());

    settings->set_temp_value(settings->get_account_name(account),
			     "smtpserver", i_SMTPServer->value());

    settings->set_temp_value(settings->get_account_name(account),
			     "smtpname", i_SMTPName->value());
}

////////////////////////////////////////////////////////////////////////////////
// Category list callbacks

void
NxMail::ChangeCatCB(Fl_Widget * fl, void *o)
{
    char *p_String = (char *) o;
    char RealCat[TEXT];

    memset(RealCat, 0, sizeof(RealCat));
    if (!p_String[0])
	strcpy(RealCat, "All");
    else
	strcpy(RealCat, p_String);

#ifdef NOTUSED
    main_category->label(RealCat);
    main_category->hide();
    main_category->show();
    editor_category->label(RealCat);
    editor_category->hide();
    editor_category->show();
    view_category->label(RealCat);
    view_category->hide();
    view_category->show();
#endif

}

void
NxMail::ChangeAcctCB(Fl_Widget * fl, void *o)
{

}
