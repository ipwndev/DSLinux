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

#include <pixil_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "nxfindapp.h"
#include <par/par.h>

#ifdef CONFIG_NANOX
#include <wm/scrtoplib.h>
#endif

#include "handlestatus.h"

#include <icons/echeck.xpm>
#include <icons/check.xpm>

about about_find = {
    "About Global Search",
    "(c) 2001, Century Software.",
    "trumanl@censoft.com",
    "08/24/01",
    "1.0"
};
//#define DEBUG
#ifdef DEBUG
#define DPRINT(str, args...) printf("NXFIND DEBUG: " str, ## args)
#else
#define DPRINT(args...)
#endif

Fl_Menu_Item findMenuItems[] = {
    {"Edit", 0, 0, 0, FL_SUBMENU}
    ,
    {"Undo", 0, NxApp::undo_callback}
    ,
    {"Cut", 0, NxApp::cut_callback}
    ,
    {"Copy", 0, NxApp::copy_callback}
    ,
    {"Paste", 0, NxApp::paste_callback, 0, FL_MENU_DIVIDER}
    ,
    //      { "Keyboard", 0, NxApp::keyboard_callback, 0, FL_MENU_DIVIDER },
    {"Exit Search", 0, NxFind::exit_callback}
    ,
    {0}
    ,
    {"Options", 0, 0, 0, FL_SUBMENU}
    ,
    {"About Search", 0, NxApp::show_about}
    ,
    {0}
    ,
    {0}
    ,
};

NxWindow *
    NxFind::mainWindow;

NxPimWindow *
    NxFind::findWindow;
NxPimWindow *
    NxFind::resultWindow;

NxPimWindow *
    NxFind::dateWindow;
NxPimPopWindow *
    NxFind::errorWindow;

Flv_Table_Child *
    NxFind::results_table;

NxInput *
    NxFind::lookup_input;

NxOutput *
    NxFind::results_message;
NxOutput *
    NxFind::error_msg;
NxOutput *
    c_statusWindow::status_msg;

NxButton *
    NxFind::fromDateButton;
NxButton *
    NxFind::toDateButton;
NxButton *
    c_statusWindow::cancelButton;

NxCheckButton *
    NxFind::stringCheck;
NxCheckButton *
    NxFind::dateCheck;

NxScroll *
    NxFind::appList;

Fl_Toggle_Tree *
    NxFind::appTree;

Fl_Toggle_Node *
    c_statusWindow::curNode;

Fl_Pixmap *
    NxFind::echeck_pixmap;
Fl_Pixmap *
    NxFind::check_pixmap;

time_t NxFind::fromTime;
time_t NxFind::toTime;

int
    NxFind::nodeNum;
int
    NxFind::total_found;

bool c_statusWindow::expectingResults;
bool c_statusWindow::cancelSearch;

c_statusWindow *
    NxFind::statusWindow;

searchStatus *
    NxFind::status;

NxFind::NxFind(int argc, char *argv[])
{

    NxApp::Instance()->set_about(about_find);

    NxApp::Instance()->set_keyboard(argc, argv);

    mainWindow = new NxWindow(W_W, W_H, APP_NAME);
    MakeFindWindow();
    MakeResultsWindow();
    MakeCalendarWindow();
    MakeErrorWindow();
    MakeStatusWindow();
    mainWindow->end();

    set_shown_window(mainWindow);
    set_shown_window(mainWindow);

    echeck_pixmap = new Fl_Pixmap(echeck_xpm);
    check_pixmap = new Fl_Pixmap(check_xpm);
    add_apps(appTree);
    nodeNum = 1;

    Add_Fd("nxfind", _ClientIPCHandler);
}

NxFind::~NxFind()
{
    Fl_Toggle_Node *node = appTree->traverse_start();
    while (node) {
	delete node->user_data();
	appTree->remove(node);
	node = appTree->traverse_start();
    }
}

void
NxFind::ClientIPCHandler(int fd, void *o, int ipc_id)
{
    DPRINT("*********************\n");
    DPRINT("*** clientIpc_handler started\n");
    DPRINT("*********************\n");

    char *tokenMsg = new char[CL_MAX_MSG_LEN];
    memset(tokenMsg, 0, CL_MAX_MSG_LEN);
    char *passMsg = new char[CL_MAX_MSG_LEN];
    memset(passMsg, 0, CL_MAX_MSG_LEN);

    if (NULL == o) {

	int length = CL_MAX_MSG_LEN - 1;

	ipc_id = NxApp::Instance()->Read_Fd(passMsg, &length);

	if ((passMsg == NULL) || (passMsg[0] == 0)) {
	    return;
	}

	strcpy(tokenMsg, passMsg);

    } else if (ipc_id == -1) {
	strcpy(tokenMsg, (char *) o);
	strcpy(passMsg, (char *) o);
	ipc_id = NxApp::Instance()->Find_Fd("nxfind");
    } else {
	strcpy(tokenMsg, (char *) o);
	strcpy(passMsg, (char *) o);
    }

    DPRINT("clientIpc_handler has now been started. Message [%s]\n", passMsg);

    char *service = new char[CL_MAX_MSG_LEN];
    char *msg_cmd = new char[CL_MAX_MSG_LEN];
    char *data_item = new char[CL_MAX_MSG_LEN];
    char *data = new char[CL_MAX_MSG_LEN];

    // SERVICE
    char *tmp = strtok(tokenMsg, TOKEN);
    strcpy(service, tmp);

    //MSG_CMD
    tmp = strtok(NULL, TOKEN);
    strcpy(msg_cmd, tmp);

    //DATA_ITEM
    tmp = strtok(NULL, TOKEN);
    strcpy(data_item, tmp);

    DPRINT("Exploding message... [%s] [%s] [%s].\n", service, msg_cmd,
	   data_item);

    if (0 == strcmp(msg_cmd, "DATA")) {

	DPRINT("DATA message command recv.\n");

	if (0 == strcmp(data_item, "search")) {
	    DPRINT("\tGetting search results.\n");

	    if (true == statusWindow->getExpectingFlag()) {
		char *strRecno = new char[16];
		char *results = new char[CL_MAX_MSG_LEN];

		tmp = strtok(NULL, TOKEN);
		strcpy(strRecno, tmp);

		int recno = atoi(strRecno);
		NxSearchData *data = new NxSearchData;

		if (APP_FILE != recno) {	// for searches that return recnos
		    tmp = strtok(NULL, TOKEN);
		    strcpy(results, tmp);
		    strcpy(data->fileName, "");
		    if (NULL != results) {
			NxSearchData *data = new NxSearchData;

			strcpy(data->data, results);
			strcpy(data->appName, service);
			data->recno = recno;
			((NxFind *) (NxApp::Instance()))->addData(data);
		    }
		} else {	// for searches that return strings
		    tmp = strtok(NULL, TOKEN);
		    strcpy(data->fileName, tmp);
		    tmp = strtok(NULL, TOKEN);
		    strcpy(results, tmp);
		    if (NULL != results) {
			memset(data->data, 0, CL_MAX_MSG_LEN);
			strncpy(data->data, results, strlen(results));
			strcpy(data->appName, service);
			data->recno = APP_FILE;
			((NxFind *) (NxApp::Instance()))->addData(data);
		    }
		}
		delete[]strRecno;
		delete[]results;
		results = strRecno = 0;
	    }
	}
    }
    if (0 == strcmp(msg_cmd, "ACK")) {

	DPRINT("ACK message command recv.\n");

	if (0 == strcmp(data_item, "DATA")) {

	    tmp = strtok(NULL, TOKEN);
	    strcpy(msg_cmd, tmp);

	    if (0 == strcmp(msg_cmd, "search")) {
		DPRINT("\tSetting expecingResults flag to false.\n");
		status->ackSearch();
	    }
	}
    }
    // Mem stuff
    delete[]service;
    delete[]msg_cmd;
    delete[]data_item;
    delete[]tokenMsg;
    delete[]data;
    data = service = msg_cmd = data_item = tokenMsg = NULL;

    NxApp::Instance()->ServerIPCHandler(fd, ipc_id, (char *) passMsg);

    delete[]passMsg;
    passMsg = NULL;
}

void
NxFind::show_default_window()
{
    show_window(findWindow->GetWindowPtr());
}

void
NxFind::MakeFindWindow()
{
    static char fromBuf[30];
    static char toBuf[30];
    Fl_Box *framebox;
    toTime = fromTime = time(0);
    tm *tt = localtime(&fromTime);

    //strftime(fromBuf, 29, "%b %d, %y", tt);
    //strftime(toBuf, 29, "%b %d, %y", tt);
    GetDateString(fromBuf, tt, sizeof(fromBuf), SHORT_YEAR);
    GetDateString(toBuf, tt, sizeof(toBuf), SHORT_YEAR);

    findWindow = new NxPimWindow(APP_NAME, findMenuItems, 0, "", "", 0);
    add_window((Fl_Window *) findWindow->GetWindowPtr());
    framebox = new Fl_Box(FL_BORDER_BOX, 0, 26, W_W, 1, "");
    findWindow->add((Fl_Widget *) framebox);
    {
	NxCheckButton *o = stringCheck =
	    new NxCheckButton(BUTTON_X, 30, "Only entries containing:");
	o->movable(false);
	findWindow->add((Fl_Widget *) o);
    }

    {
	NxInput *o = lookup_input = new NxInput(BUTTON_X + 19, 50, 141, 20);
	o->movable(false);
	lookup_input->maximum_size(99);
	lookup_input->when(FL_WHEN_RELEASE_ALWAYS);
	lookup_input->callback(NxApp::Instance()->pasteTarget_callback);
	findWindow->add((Fl_Widget *) o);
    }

    {
	NxCheckButton *o = dateCheck =
	    new NxCheckButton(BUTTON_X, 70, "Limit by date range:");
	o->movable(false);
	findWindow->add((Fl_Widget *) o);
    }

    {
	NxOutput *o = new NxOutput(BUTTON_X + 19, 90, 60, BUTTON_HEIGHT);
	o->movable(false);
	o->value("From:");
	findWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o = fromDateButton =
	    new NxButton(BUTTON_X + 60, 90, 100, BUTTON_HEIGHT);
	o->movable(false);
	o->label(fromBuf);
	o->callback(fromCalendar_callback, this);
	o->redraw();
	findWindow->add((Fl_Widget *) o);
    }
    {
	NxOutput *o = new NxOutput(BUTTON_X + 29, 110, 60, BUTTON_HEIGHT);
	o->movable(false);
	o->value("To:");
	findWindow->add((Fl_Widget *) o);
    }
    {
	NxButton *o = toDateButton =
	    new NxButton(BUTTON_X + 60, 110, 100, BUTTON_HEIGHT);
	o->movable(false);
	o->label(toBuf);
	o->redraw();
	o->callback(toCalendar_callback, this);
	findWindow->add((Fl_Widget *) o);
    }
    {

	NxScroll *o = appList =
	    new NxScroll(-1, 115 + BUTTON_HEIGHT, W_W + 2, BUTTON_Y - 175);
	o->resize(false);
	{
	    appTree = new Fl_Toggle_Tree(0, 115 + BUTTON_HEIGHT, W_W, 10);
	    appTree->callback(checkIt_callback);

	}
	o->end();

	findWindow->add((Fl_Widget *) o);
    }

    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Search");
	o->movable(false);
	o->callback(searchLookup_callback, this);
	findWindow->add((Fl_Widget *) o);
    }
}

void
NxFind::UpdateFromButton()
{
    static char buf[30];
    tm *tt = localtime(&fromTime);

    //strftime(buf, 29, "%b %d, %y", tt);
    GetDateString(buf, tt, sizeof(buf), SHORT_YEAR);
    fromDateButton->label(buf);
    fromDateButton->redraw();
}

void
NxFind::UpdateToButton()
{
    static char buf[30];
    tm *tt = localtime(&toTime);

    //strftime(buf, 29, "%b %d, %y", tt);
    GetDateString(buf, tt, sizeof(buf), SHORT_YEAR);
    toDateButton->label(buf);
    toDateButton->redraw();
}


void
NxFind::MakeResultsWindow()
{
    resultWindow = new NxPimWindow(APP_NAME, findMenuItems, 0, "", "", 0);

    add_window((Fl_Window *) resultWindow->GetWindowPtr());
    {
	results_message =
	    new NxOutput(4, MB_Y + BUTTON_HEIGHT + 10, W_W - 10, 25);
	results_message->value("Nothing Found.");
	results_message->hide();
	resultWindow->add((Fl_Widget *) results_message);
    }
    {
	Fl_Box *o = new Fl_Box(-1, MB_Y + BUTTON_HEIGHT + 9, W_W + 2,
			       (BUTTON_Y - (MB_Y + BUTTON_HEIGHT + 15) + 2));
	o->box(FL_BORDER_BOX);
	o->color(NxApp::Instance()->getGlobalColor(APP_BG));
	resultWindow->add((Fl_Widget *) o);
	results_table =
	    new Flv_Table_Child(1, MB_Y + BUTTON_HEIGHT + 10, W_W - 2,
				BUTTON_Y - (MB_Y + BUTTON_HEIGHT + 15), 0,
				W_W - 50);
	results_table->callback(resultsView_callback, this);
	results_table->selection_color(FL_DARK3);
	results_table->SetCols(1);
	resultWindow->add((Fl_Widget *) results_table);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
			 "Done");
	o->callback(doneLookup_callback, this);
	resultWindow->add((Fl_Widget *) o);
    }
}

void
NxFind::MakeErrorWindow()
{
    errorWindow = new NxPimPopWindow("Error");
    add_window((Fl_Window *) errorWindow->GetWindowPtr());
    {
	error_msg =
	    new NxOutput(4, 19, errorWindow->GetWindowPtr()->w() - 6, 25);
	error_msg->value("Error: No Search Constraint.");
	errorWindow->add((Fl_Widget *) error_msg);
    }
    {
	NxButton *o =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Ok");
	o->callback(errorOk_callback);
	errorWindow->add((Fl_Widget *) o);
    }
    errorWindow->GetWindowPtr()->hide();
}

void
NxFind::MakeStatusWindow()
{
    statusWindow = new c_statusWindow();
    add_window((Fl_Window *) statusWindow->GetWindowPtr());
}

c_statusWindow::c_statusWindow():NxPimPopWindow("Status")
{
    expectingResults = false;
    cancelSearch = false;
    curNode = NULL;

    {
	status_msg = new NxOutput(4, 19, this->GetWindowPtr()->w() - 10, 25);
	status_msg->value("");
	this->add((Fl_Widget *) status_msg);
    }
    {
	NxButton *
	    o =
	    cancelButton =
	    new
	    NxButton(BUTTON_X, 90, BUTTON_WIDTH,
		     BUTTON_HEIGHT, "Cancel");
	o->callback(statusCancel_callback, this);
	this->add((Fl_Widget *) o);
    }
}

c_statusWindow::~c_statusWindow()
{
}

void
c_statusWindow::setStatus(char *status)
{
    status_msg->value(status);
}

void
c_statusWindow::statusCancel_callback(Fl_Widget * fl, void *l)
{
    expectingResults = false;
    cancelSearch = true;

    NxApp::Instance()->show_window(((NxFind *) (NxApp::Instance()))->
				   getFindWindow()->GetWindowPtr());
}

void
c_statusWindow::hide()
{
    DPRINT("in hide window\n");
    if (0 == ((NxFind *) (NxApp::Instance()))->getTotalFound()) {
	((NxFind *) (NxApp::Instance()))->getResultsMessage()->show();
	((NxFind *) (NxApp::Instance()))->getResultsTable()->hide();
    } else {
	((NxFind *) (NxApp::Instance()))->getResultsMessage()->hide();
	((NxFind *) (NxApp::Instance()))->getResultsTable()->show();
    }

    ((NxFind *) (NxApp::Instance()))->getResultsTable()->
	rows(((NxFind *) (NxApp::Instance()))->getTotalFound());

    NxApp::Instance()->show_window(((NxFind *) (NxApp::Instance()))->
				   getResultWindow()->GetWindowPtr());
}

void
c_statusWindow::show()
{
    //((NxFind*)(NxApp::Instance()))->getFindWindow()->GetWindowPtr()->deactivate();
    ((NxFind *) (NxApp::Instance()))->getStatusWindow()->GetWindowPtr()->
	show();
}

void
c_statusWindow::setCurNode(Fl_Toggle_Node * node)
{
    curNode = node;
}

char *
c_statusWindow::getNextService()
{
    if (NULL == curNode)
	curNode =
	    ((NxFind *) (NxApp::Instance()))->getAppTree()->traverse_start();
    else
	curNode =
	    ((NxFind *) (NxApp::Instance()))->getAppTree()->
	    traverse_forward();

    while (curNode) {
	if (true == ((NxAppData *) curNode->user_data())->appSearch)
	    return ((NxAppData *) curNode->user_data())->nxAppName;
	else
	    curNode =
		((NxFind *) (NxApp::Instance()))->getAppTree()->
		traverse_forward();
    }
    DPRINT("return NULL\n");
    return NULL;
}

char *
c_statusWindow::getApp()
{
    if (curNode) {
	return ((NxAppData *) curNode->user_data())->appName;
    }
    return NULL;
}

void
c_statusWindow::search()
{
    int ipc_id = -1;
    char *msg = new char[CL_MAX_MSG_LEN];

    //sleep(1);

    // FIXME start app if needed something like this FIXME
    // if (0 > oinf_fd) {
    //      flags = some_flags
    //      oin_fd = colosseum_start(appName, flags);       
    //      }
    bool stringCheck =
	((NxFind *) (NxApp::Instance()))->getStringCheckValue();
    bool dateCheck = ((NxFind *) (NxApp::Instance()))->getDateCheckValue();

    if (stringCheck == true && dateCheck == false) {
	// FIXME
	//  when the PAR database is done need to determine if 
	//  service can search on strings. Currently all service
	//  can search on string values
	// send the search request
	ipc_id = NxApp::Instance()->StartApp(getService(), 0, 2, 10);
	DPRINT("service [%s] for ipc_id[%d]\n", getService(), ipc_id);
	if (ipc_id <= 0)
	    return;
	initiateSearch(ipc_id);
	sprintf(msg, "nxfind^EXECUTE^search^%s^%d^",
		((NxFind *) (NxApp::Instance()))->getLookupInput()->value(),
		((NxFind *) (NxApp::Instance()))->getResultsTable()->
		text_width());
	executeSearch(ipc_id, msg);
    }

    if (stringCheck == false && dateCheck == true) {
	// FIXME
	// when the PAR database is done need to determine if
	// service can search on dates.  Currently only schedule
	// searches on dates.
	if (0 == strcmp("nxschedule", getService()) ||
	    0 == strcmp("nxtodo", getService())) {
	    // send the search request
	    ipc_id = NxApp::Instance()->StartApp(getService(), 0, 2, 10);
	    DPRINT("service [%s] for ipc_id[%d]\n", getService(), ipc_id);
	    if (ipc_id <= 0)
		return;
	    initiateSearch(ipc_id);
	    sprintf(msg, "nxfind^EXECUTE^datesearch^%d^%ld^%ld^",
		    ((NxFind *) (NxApp::Instance()))->getResultsTable()->
		    text_width(),
		    ((NxFind *) (NxApp::Instance()))->getStartTime(),
		    ((NxFind *) (NxApp::Instance()))->getEndTime());
	    executeSearch(ipc_id, msg);
	}
    }

    if (stringCheck == true && dateCheck == true) {
	// FIXME
	// when the PAR database is done need to determine if 
	// the sevice can search on dates and string. Currently only schedule
	// searches on dates and strings.
	if (0 == strcmp("nxschedule", getService()) ||
	    0 == strcmp("nxtodo", getService())) {
	    ipc_id = NxApp::Instance()->StartApp(getService(), 0, 2, 10);
	    DPRINT("service [%s] for ipc_id[%d]\n", getService(), ipc_id);
	    if (ipc_id <= 0)
		return;
	    initiateSearch(ipc_id);
	    sprintf(msg, "nxfind^EXECUTE^search^%s^%d^%ld^%ld^",
		    ((NxFind *) (NxApp::Instance()))->getLookupInput()->
		    value(),
		    ((NxFind *) (NxApp::Instance()))->getResultsTable()->
		    text_width(),
		    ((NxFind *) (NxApp::Instance()))->getStartTime(),
		    ((NxFind *) (NxApp::Instance()))->getEndTime());
	    executeSearch(ipc_id, msg);
	}
    }

    delete[]msg;
    msg = 0;

}

void
c_statusWindow::initiateSearch(int ipc_id)
{
    char *msg = new char[CL_MAX_MSG_LEN];
    sprintf(msg, "nxfind^INITIATE^0^");
    int x = NxApp::Instance()->Write_Fd(ipc_id, msg, CL_MAX_MSG_LEN);
    delete[]msg;
    msg = 0;
}

void
c_statusWindow::executeSearch(int ipc_id, char *msg)
{
    char format_string[512];
    NxSearchData *data = new NxSearchData;;

    DPRINT("Writing message [%s] for ipc_id[%d]\n", msg, ipc_id);
    int x = NxApp::Instance()->Write_Fd(ipc_id, msg, CL_MAX_MSG_LEN);

    data->recno = APP_DESC;
    strcpy(data->appName, getService());
    strcpy(data->data, getAppNameString(format_string));
    ((NxFind *) (NxApp::Instance()))->addData(data);
}

char *
c_statusWindow::getAppNameString(char *format_string)
{
    Fl_Toggle_Tree *pTree = ((NxFind *) (NxApp::Instance()))->getAppTree();
    Flv_Table_Child *pTable =
	((NxFind *) (NxApp::Instance()))->getResultsTable();
    int width = pTable->text_width();
    int sep_width = (int) fl_width("-");
    int string_width = 0;
    int diff_width = 0;
    int sep_count = 0;
    Fl_Toggle_Node *node = 0;
    NxAppData *data = 0;
    char sep_string[128];

    fl_font(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);
    node = pTree->traverse_start();
    while (node) {
	data = ((NxAppData *) node->user_data());
	if (0 == strcmp(data->nxAppName, getService())) {
	    string_width = (int) fl_width(data->appName);
	    diff_width = (width - string_width) / 2;
	    if (0 >= diff_width)
		return data->appName;
	    else {
		sep_count = diff_width / sep_width;
		if (0 < sep_count) {
		    if (sep_count > 128)
			sep_count = 128;
		    memset(sep_string, 0, sizeof(sep_string));
		    for (int idx = 0; idx <= sep_count; idx++)
			sep_string[idx] = '-';
		    sprintf(format_string, "%s%s%s", sep_string,
			    data->appName, sep_string);
		    return format_string;
		} else {
		    return data->appName;
		}
	    }
	}
	node = pTree->traverse_forward();
    }
    return NULL;
}

void
NxFind::viewRecord(NxSearchData * data)
{
    int ipc_id = -1;
    char *msg = new char[CL_MAX_MSG_LEN];

#ifdef NOTUSED
    ipc_id = NxApp::Instance()->Find_Fd(data->appName);
    if (0 > ipc_id)
	return;
    // start app if needed something like this      
    // if (0 > oinf_fd) {
    //      flags = some_flags
    //      oin_fd = colosseum_start(appName, flags);       
    //      }
#endif

    ipc_id = NxApp::Instance()->StartApp(data->appName, 0, 0);
    if (ipc_id <= 0)
	return;

    sprintf(msg, "nxfind^INITIATE^0^");
    NxApp::Instance()->Write_Fd(ipc_id, msg, CL_MAX_MSG_LEN);

    if (APP_FILE == data->recno)
	sprintf(msg, "nxfind^EXECUTE^showrecord^%s^", data->fileName);
    else
	sprintf(msg, "nxfind^EXECUTE^showrecord^%d^", data->recno);
    NxApp::Instance()->Write_Fd(ipc_id, msg, CL_MAX_MSG_LEN);

    delete[]msg;
    msg = 0;

#ifdef CONFIG_NANOX
    /* Now, contact the screentop and force the app to raise */
    /* Only available in Nano-X (for now)                    */
    scrtopShowApp(data->appName);
#endif
}

void
NxFind::checkIt_callback(Fl_Widget * fl, void *l)
{
    Fl_Toggle_Node *node = appTree->selected();


    if (node) {
	if ((Fl::event_x() >= 17) && (Fl::event_x() <= 29)) {
	    appTree->unselect();
	    if (node->pixmap() == echeck_pixmap) {
		((NxAppData *) node->user_data())->appSearch = true;
		node->pixmap(check_pixmap);
	    } else if (node->pixmap() == check_pixmap) {
		node->pixmap(echeck_pixmap);
		((NxAppData *) node->user_data())->appSearch = false;
	    }
	}
    }
    if (appTree->selection_count() > 0)
	appTree->unselect();

}

void
NxFind::addData(NxSearchData * data)
{
    results_table->Add(total_found, data);
    results_table->set_value(total_found, 0, data->data);
    total_found++;
}

void
NxFind::searchLookup_callback(Fl_Widget * fl, void *l)
{
    if ((stringCheck->value() == 0) && (dateCheck->value() == 0)) {
	error_msg->value("Error: No Search Constraint.");
	NxApp::Instance()->show_window(errorWindow->GetWindowPtr(),
				       DEACTIVATE,
				       findWindow->GetWindowPtr());
	return;
    }
    if (0 == strcmp(lookup_input->value(), "") && (0 != stringCheck->value())) {
	error_msg->value("Error: No search value!");
	NxApp::Instance()->show_window(errorWindow->GetWindowPtr(),
				       DEACTIVATE,
				       findWindow->GetWindowPtr());
	return;

    }

    Fl_Toggle_Node *node =
	((NxFind *) (NxApp::Instance()))->getAppTree()->traverse_start();
    int have_search = 0;

    while (node) {
	if (true == ((NxAppData *) node->user_data())->appSearch) {
	    have_search = 1;
	    break;
	} else
	    node =
		((NxFind *) (NxApp::Instance()))->getAppTree()->
		traverse_forward();
    }

    if (!have_search) {
	error_msg->value("Error: No Application selected!");
	NxApp::Instance()->show_window(errorWindow->GetWindowPtr(),
				       DEACTIVATE,
				       findWindow->GetWindowPtr());
	return;
    }

    results_table->Init(255);
    total_found = 0;
    results_table->row(0);
    status = startSearchStatus();

    //statusWindow->setStatus("Searching...");
    statusWindow->setExpectingFlag(true);
    // have a thread to monitor status
    //findWindow->GetWindowPtr()->hide();
    //NxApp::Instance()->show_window(resultWindow->GetWindowPtr());

}

void
NxFind::fromCalendar_callback(Fl_Widget * fl, void *l)
{
    NxFind *pThis = (NxFind *) l;

    pThis->SetPickedDate(pThis->fromTime);
    NxApp::Instance()->show_window(dateWindow->GetWindowPtr(),
				   DEACTIVATE, findWindow->GetWindowPtr());
    pThis->date_callback(fromDate_callback);
}

void
NxFind::toCalendar_callback(Fl_Widget * fl, void *l)
{
    NxFind *pThis = (NxFind *) l;

    pThis->SetPickedDate(pThis->toTime);
    NxApp::Instance()->show_window(dateWindow->GetWindowPtr(),
				   DEACTIVATE, findWindow->GetWindowPtr());
    pThis->date_callback(toDate_callback);
}

void
NxFind::fromDate_callback(Fl_Widget * fl, void *l)
{
    NxFind *pThis = (NxFind *) l;

    pThis->dateWindow->GetWindowPtr()->hide();
    pThis->show_window(findWindow->GetWindowPtr());

    if (pThis->GetPickedDate()) {
	pThis->fromTime = pThis->GetPickedDate();
	pThis->UpdateFromButton();
    }
}

void
NxFind::toDate_callback(Fl_Widget * fl, void *l)
{
    NxFind *pThis = (NxFind *) l;

    pThis->dateWindow->GetWindowPtr()->hide();
    pThis->show_window(findWindow->GetWindowPtr());

    if (pThis->GetPickedDate()) {
	pThis->toTime = pThis->GetPickedDate();
	pThis->UpdateToButton();
    }
}

void
NxFind::doneLookup_callback(Fl_Widget * fl, void *l)
{
    NxFind *pThis = (NxFind *) l;
    static char fromBuf[30];
    static char toBuf[30];

    NxApp::Instance()->show_window(findWindow->GetWindowPtr(), ACTIVATE);
    lookup_input->value("");
    stringCheck->value(false);
    dateCheck->value(false);

    toTime = fromTime = time(0);
    tm *tt = localtime(&fromTime);

    //strftime(fromBuf, 29, "%b %d, %y", tt);
    //strftime(toBuf, 29, "%b %d, %y", tt);
    pThis->GetDateString(fromBuf, tt, sizeof(fromBuf), SHORT_YEAR);
    pThis->GetDateString(toBuf, tt, sizeof(toBuf), SHORT_YEAR);
    pThis->UpdateFromButton();
    pThis->UpdateToButton();
}

void
NxFind::resultsView_callback(Fl_Widget * fl, void *l)
{
    NxFind *pThis = (NxFind *) l;

    NxSearchData *data = 0;

    data = (NxSearchData *) results_table->selected();

    if (Fl::event_clicks()) {
	if (data) {
	    if (APP_DESC == data->recno)
		return;
	    else {
		pThis->viewRecord(data);
	    }
	}
    }

    Fl::event_clicks(0);
}

void
NxFind::errorOk_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(findWindow->GetWindowPtr(),
				   DEACTIVATE, errorWindow->GetWindowPtr());
}

void
NxFind::exit_callback(Fl_Widget * fl, void *l)
{
    mainWindow->hide();
    exit(0);
}

void
NxFind::add_apps(Fl_Toggle_Tree * _appTree)
{
    int size = 0;
    int count, i;

    db_handle *db;
    char *searchList, *l;

    /* Dum, dah, dum, dum, dummmmmm! */
    /* PAR to the rescue */

    /* Fire up the PAR database, and load all the applications */
    /* That are interested in global search */
    db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);

    if (!db) {
	printf("Error - Couldn't open the par database %s\n",
	       db_getDefaultDB());
	return;
    }

    size = par_getCapability(db, "nxgblsearch", (void **) &searchList);

    if (size <= 0) {
	return;
    }

    count = par_getStringListCount(searchList, ' ');
    l = searchList;

    for (i = 0; i < count; i++) {
	Fl_Toggle_Node *appNode;
	char title[50];

	char *application = par_parseStringList(&l, ' ');

	if (par_getAppTitle(db, application, title, sizeof(title)) <= 0)
	    continue;

	appNode = _appTree->add_next(title, 0, echeck_pixmap);



	if (appNode) {
	    NxAppData *data = new NxAppData;
	    strcpy(data->appName, title);
	    strcpy(data->nxAppName, application);
	    data->appSearch = false;
	    appNode->user_data(data);
	    _appTree->traverse_up();
	}
    }

    db_closeDB(db);
}

Fl_Window *
NxFind::get_main_window()
{
    if (mainWindow)
	return mainWindow;
    else
	return 0;
}
