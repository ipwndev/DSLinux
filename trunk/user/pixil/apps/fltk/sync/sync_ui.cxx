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


#include <par/par.h>
#include <wm/scrtoplib.h>
#include <sync/coder.h>
#include <sync/msg_defs.h>
#include "sync.h"

#include <stdio.h>

//#define DEBUG

#ifdef DEBUG
#define DPRINT(str, args...) printf("SYNC DEBUG: " str, ## args)
#else
#define DPRINT(args...)
#endif

about about_sync = {
    "About Synchronization",
    "(c) 2001, Century Software.",
    "trumanl@censoft.com",
    "11/30/01",
    "1.0"
};

Fl_Menu_Item syncMenuItems[] = {
    {"Exit Synchronization", 0, Sync::exit_callback}
    ,
    {"Options", 0, 0, 0, FL_SUBMENU}
    ,
    {"About Synchronization", 0, NxApp::show_about}
    ,
    {0}
    ,
    {0}
};

NxWindow *
    Sync::mainWindow;

NxPimWindow *
    Sync::syncWindow;
NxPimPopWindow *
    Sync::statusWindow;

Fl_Toggle_Tree *
    Sync::syncTree;
Fl_Toggle_Node *
    Sync::curNode;

NxScroll *
    Sync::syncScroll;
NxButton *
    Sync::syncButton;
NxButton *
    Sync::abortButton;
NxButton *
    Sync::okButton;
NxOutput *
    Sync::statusMsg;
NxSlider *
    Sync::statusSlider = 0;

Fl_Pixmap *
    Sync::echeckPixmap;
Fl_Pixmap *
    Sync::checkPixmap;

bool Sync::first_app;

Sync::Sync(char *app):
NxApp(app)
{

  printf("STATE -> SYNC_WAIT\n");

    syncState = SYNC_WAIT;

    NxApp::Instance()->set_about(about_sync);

    mainWindow = new NxWindow(W_W, W_H, app);
    MakeSyncWindow();
    MakeStatusWindow();
    mainWindow->end();

    set_shown_window(mainWindow);

    echeckPixmap = new Fl_Pixmap(echeck_xpm);
    checkPixmap = new Fl_Pixmap(check_xpm);
    add_apps(syncTree);
    curNode = NULL;

    Add_Fd("nxsync", _ClientIPCHandler);
    sync_id = -1;
    //msgState = SYNC_WAIT;
    
    cur_app = "";
    first_app = false;

}

Sync::~Sync()
{
}


int 
Sync::agent_connect(void) {

  int agent_id = StartApp("syncagent", 0, 0);
  printf("Got %d while starting %s\n", agent_id, "syncagent");

  if (agent_id == -1) return -1;

  /* Now, connect the agent to the remote system */
  /* FIXME:  This is hard coded here for now, because its easier */

  printf("SENDING connect to %d\n", agent_id);

  Write_Fd(agent_id, "1000^TCPIP^LOCALHOST^2000", 25);
  return 0;
}

int 
Sync::agent_disconnect(void) {

  int agent_id = Find_Fd("syncagent");
  if (agent_id == -1) return -1;
  
  Write_Fd(agent_id, "1005^", 5);
  return 0;
}


void
Sync::ClientIPCHandler(int fd, void *o, int ipc_id)
{
  
  char *passMsg = new char[CL_MAX_MSG_LEN];
  memset(passMsg, 0, CL_MAX_MSG_LEN);

  printf("SYNC->CLIENTIPCHANDLER (%d, %x, %d)\n", fd, 0, ipc_id);

  if (!o) {
    int length = CL_MAX_MSG_LEN - 1;
    ipc_id = NxApp::Instance()->Read_Fd(passMsg, &length);
    if (passMsg[0] == 0) return;
  }
  else if (ipc_id == -1) {
    strcpy(passMsg, (char *) o);
    ipc_id = NxApp::Instance()->Find_Fd("nxsync");
  } else {
    strcpy(passMsg, (char *) o);
  }

  short msg_id = -1;
  const string passString = passMsg;
  string msg_id_str;
  bool err = false;

  coder.vmessages.clear();
  coder.DecodeMsg(passString);
  
  msg_id_str = coder.vmessages[0];
  msg_id = atoi(msg_id_str.c_str());

  printf("MESSAGE: {%s] from [%d]\n", passMsg, ipc_id);
  printf("INCOMING:  SYNC STATE [%d] MSG ID [%d]\n", syncState, msg_id);

  switch(syncState) {
  case SYNC_CONNECT:

    if (msg_id == OK) {
      printf("STATE -> SYNC_START\n");
      syncState = SYNC_START;
    }
    else 
      do_error("Unable to connect to the remote agent");
    
    delete[]passMsg;
    return;

  case SYNC_PENDING:
    if (msg_id == OK) {
      printf("STATE -> SYNC_EXE\n");
      syncState = SYNC_EXE;           

      delete[]passMsg;
      return;
    }
    else if (msg_id == ERR)
      do_error("Error while starting sync");
    else if (msg_id == ABORT)
      do_abort();
    
    break;

  case SYNC_EXE:    
    if (msg_id == INFO) {
      printf("STATE -> SYNC_DONE\n");
      syncState = SYNC_DONE;
    }
    else if (msg_id == ERR) {
      do_error(coder.vmessages[2]);
      err = true;
    }
    else if (msg_id == ABORT) {
      do_abort();
      err = true;
    }

    break;
    
  case SYNC_DONE:
    break;
    
  case SYNC_ABORT:
    if (msg_id == ERR) {
      do_error("Syncronization aborted!");
      err = true;
    }
    
    break;
  }
  
  if (err == false)
    ServerIPCHandler(fd, ipc_id, (char *) passMsg);
  
  delete[]passMsg;
}

void
Sync::MakeSyncWindow()
{

    syncWindow = new NxPimWindow(APP_NAME, syncMenuItems, 0, "", "", 0);
    add_window((Fl_Window *) syncWindow->GetWindowPtr());

    {
	NxScroll *o = syncScroll =
	    new NxScroll(-1, 15 + BUTTON_HEIGHT, W_W + 2, 230);
	o->resize(false);
	{
	    syncTree = new Fl_Toggle_Tree(0, 15 + BUTTON_HEIGHT, W_W, 10);
	    syncTree->callback(checkIt_callback);
	}
	o->end();

	syncWindow->add((Fl_Widget *) o);

    }

    syncButton =
	new NxButton((W_W / 2) - ((BUTTON_WIDTH + 15) / 2), BUTTON_Y,
		     BUTTON_WIDTH + 15, BUTTON_HEIGHT, "Synchronize");
    syncButton->movable(false);
    syncButton->callback(sync_callback);
    syncWindow->add((Fl_Widget *) syncButton);
}


void
Sync::MakeStatusWindow()
{

    statusWindow = new NxPimPopWindow("Status");
    add_window((Fl_Window *) statusWindow->GetWindowPtr());
    {
	statusMsg =
	    new NxOutput(4, 19, statusWindow->GetWindowPtr()->w() - 6, 25);
	statusMsg->value("Synchronizing");
	statusWindow->add((Fl_Widget *) statusMsg);
    }
    {
	abortButton =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Abort");
	abortButton->callback(abort_callback);
	statusWindow->add((Fl_Widget *) abortButton);
    }
    {
	okButton =
	    new NxButton(BUTTON_X, 90, BUTTON_WIDTH, BUTTON_HEIGHT, "Ok");
	okButton->callback(ok_callback);
	statusWindow->add((Fl_Widget *) okButton);
	okButton->hide();
    }
    {
      //statusSlider = new NxSlider(5, 55,
      //		    statusWindow->GetWindowPtr()->w() - 10,
      //			    BUTTON_HEIGHT);
      //statusSlider->box(FL_BORDER_BOX);
      //statusSlider->minimum(0.0);
      //statusSlider->maximum(100.0);
      //statusSlider->step(1.0);
      //statusSlider->type(FL_HOR_FILL_SLIDER);
      //statusSlider->value(0.0);
      //statusSlider->deactivate();
      //statusWindow->add((Fl_Widget *) statusSlider);
    }

    statusWindow->GetWindowPtr()->hide();
}


void
Sync::ok_callback(Fl_Widget * fl, void *l)
{
    NxApp::Instance()->show_window(syncWindow->GetWindowPtr(), ACTIVATE);

    curNode = NULL;
    first_app = false;
}

void
Sync::abort_callback(Fl_Widget * fl, void *l)
{

    Sync *pThis = (Sync *) (NxApp::Instance());
    char *msg = new char[CL_MAX_MSG_LEN];

    strcpy(msg, pThis->coder.Abort().c_str());
    curNode = NULL;
    statusMsg->value("Synchronizing aborted!");
    abortButton->hide();
    okButton->show();
    first_app = false;

    if (pThis->syncState != SYNC_ERROR) {
	pThis->Write_Fd(pThis->sync_id, msg, strlen(msg));
	pThis->sync_abort();
    }
    printf("STATE -> SYNC_ABORT\n");
    pThis->syncState = SYNC_ABORT;

    //pThis->msgState = SYNC_ABORT;

    delete[]msg;
    msg = 0;

}

void
Sync::sync_callback(Fl_Widget * fl, void *l)
{

    NxApp::Instance()->show_window(statusWindow->GetWindowPtr(),
				   DEACTIVATE, syncWindow->GetWindowPtr());

    Sync *pThis = (Sync *) (NxApp::Instance());

    if (!(pThis->get_next_app())) {
	abortButton->hide();
	okButton->show();
	//statusSlider->hide();
	statusMsg->value("No application selected to sync.");
	curNode = NULL;
    } else {
	first_app = false;
	curNode = NULL;
	okButton->hide();
	abortButton->show();
	//statusSlider->show();
	//statusSlider->value(0.0);
	statusMsg->value("Starting sync");
	printf("STATE -> SYNC_PRECONNECT\n");
	((Sync *) (NxApp::Instance()))->syncState = SYNC_PRECONNECT;
	Fl::add_timeout(0.05, timer_callback);
    }

}

void
Sync::timer_callback(void *)
{
  char msg[255];
	
    Sync *pThis = (Sync *) NxApp::Instance();
    int state = pThis->syncState;
    float timeout = 0.5;

    printf("TIMER [%d]\n", state);

    switch(state) {

    case SYNC_WAIT:
      printf("STATE -> SYNC_PRECONNNECT\n");
      pThis->syncState = SYNC_PRECONNECT;
      break;

    case SYNC_PRECONNECT:
      printf("PRECONNECT->CONNECT\n");
      if (pThis->agent_connect() == -1) {
	sprintf(msg, "Error - Unable to start the sync agent");
	statusMsg->value(msg);
	
	abortButton->hide();
	okButton->show();
	first_app = true;

	printf("STATE -> SYNC_WAIT\n");
	pThis->syncState = SYNC_WAIT;
	return;
      }
      
      printf("STATE -> SYNC_CONNECT\n");
      pThis->syncState = SYNC_CONNECT;
      break;

    case SYNC_CONNECT:
      break;           
      
    case SYNC_START: {
      char *service = pThis->get_next_app();

      printf("~~~ SYNCING [%s] ~~~~\n", service);
      if (service) {
	pThis->sync(service);
	printf("STATE -> SYNC_PENDING\n");
	pThis->syncState = SYNC_PENDING;
	break;
      }
      else {
	sprintf(msg, "Synchronization complete");
	statusMsg->value(msg);

	printf("STATE -> SYNC_DISCONNECT\n");
	pThis->syncState = SYNC_DISCONNECT;
	abortButton->hide();
	okButton->show();

	first_app = true;
	break;
      }
    }

    case SYNC_EXE:
      //if (pThis->msgState == SYNC_EXE)
      pThis->send_update_status(pThis->cur_app);

      timeout = 0.3;
      return;

    case SYNC_ERROR:
      printf("ERROR! ERROR! ERROR!\n");
      pThis->sync_error();
      printf("STATE -> SYNC_DISCONNECT\n");
      pThis->syncState = SYNC_DISCONNECT;
      break;

    case SYNC_ABORT:
      printf("ABORT! ABORT! ABORT!\n");
      pThis->sync_abort();
      printf("STATE -> SYNC_DISCONNECT\n");
      pThis->syncState = SYNC_DISCONNECT;
      break;

    case SYNC_DONE:
      printf("STATE -> SYNC_START\n");
      pThis->syncState = SYNC_START;  /* Start the next one */
      break;

    case SYNC_DISCONNECT:
      printf("DISCONNECTING AGENT\n");
      pThis->agent_disconnect();
      printf("STATE -> SYNC_WAIT\n");
      pThis->syncState = SYNC_WAIT;
      return;
    }

    Fl::add_timeout(timeout, timer_callback);
}

int
Sync::begin_app()
{


    char *msg = new char[CL_MAX_MSG_LEN];

    strcpy(msg,
	   coder.BeginPimSync(((SyncData *) curNode->user_data())->nxAppName).
	   c_str());
    cur_app = ((SyncData *) curNode->user_data())->nxAppName;
    DPRINT("begin_app [%s]\n", cur_app.c_str());

    //int ret = Write_Fd(sync_id, msg, strlen(msg));
    int ret = 100;

    delete[]msg;
    msg = 0;

    return ret;
}

int
Sync::initiate_sync()
{
  printf("---- initiate_sync() ----\n");

  int ret = 0;
  
  ret = start_app();
  if (ret <= 0) return -1;
    
  DPRINT("initiate_sync() sync_id [%d]\n", sync_id);
    
  char *msg = new char[CL_MAX_MSG_LEN];
  sprintf(msg, "nxsync^INITIATE^0^");
  ret = Write_Fd(sync_id, msg, strlen(msg));

  if (ret < 0) 
    do_error("Unable to initiate sync!");

  delete[]msg;
  msg = 0;

  printf(" ---- return %d ----\n", ret);
  return ret;
}

int
Sync::start_sync()
{

  printf("---- start_sync() ----\n");

  char *msg = new char[CL_MAX_MSG_LEN];

  strcpy(msg,
	 coder.BeginSync(((SyncData *) curNode->user_data())->nxAppName).
	 c_str());
  
  //msgState = SYNC_START;
  
  int ret = Write_Fd(sync_id, msg, strlen(msg));

  delete[]msg;
  msg = 0;
  return ret;
}

void
Sync::sync(char *service)
{

    char msg[255];
    int ret = 0;

    DPRINT("sync() [%s]\n", service);
    if (first_app == false) {
	first_app = true;
	DPRINT("service [%s]\n", service);
	statusMsg->value("Starting Synchronization");
	statusMsg->redraw();
	//statusSlider->value(0.0);
	//statusSlider->redraw();
	sprintf(msg, "Synchronizing %s",
		((SyncData *) curNode->user_data())->appName);
	ret = initiate_sync();
	DPRINT("initiate_sync() ret [%d]\n", ret);
	if (0 > ret) {
	    return;
	}
	ret = start_sync();
	if (0 != ret) {
	    DPRINT("Unable to start!\n");
	    do_error("Unable to start!");
	    return;
	};
	statusMsg->value(msg);
	statusMsg->redraw();
	DPRINT("Fisrt app is FALSE\n");
	begin_app();
    } else {
      //statusSlider->value(0.0);
      //statusSlider->redraw();
	sprintf(msg, "Synchronizing %s",
		((SyncData *) curNode->user_data())->appName);
	DPRINT("first_app is true\n");
	ret = initiate_sync();
	DPRINT("initiate_sync() ret [%d]\n", ret);
	if (0 > ret) {
	    return;
	}
	ret = start_sync();
	if (0 != ret) {
	    DPRINT("Unable to start sync!\n");
	    do_error("Unable to start sync!");
	    return;
	};
	begin_app();
    }
}

void
Sync::sync_abort()
{
    string msg = coder.Abort();

    Write_Fd(sync_id, (char *) msg.c_str(), msg.length());
}

void
Sync::sync_error()
{
    // recover from error
    DPRINT("need to send error to sync agent\n");
}

int
Sync::send_update_status(string app_id)
{
    static float s_value = 0;
    s_value += 10;
    //statusSlider->value(s_value);
    if (s_value >= 100)
	s_value = 0;
    //statusSlider->redraw();
}

void
Sync::update_error(vector < string > &vmessages)
{
    DPRINT("should do update_error!\n");
}

void
Sync::get_update_status(vector < string > &vmessages)
{
    DPRINT("Get Update Status start\n");
    //get sync status
    float value = 0;
    static float slider_value = 0;
    string svalue;
    char msg[CL_MAX_MSG_LEN];

    svalue = vmessages[2];
    value = strtod(svalue.c_str(), NULL);
    DPRINT("Set value to [%s]\n", svalue.c_str());
    slider_value += 25;
    //statusSlider->value(slider_value);
    if (slider_value >= 100)
	slider_value = 0;
    //statusSlider->redraw();
    if (value >= 100) {
	DPRINT("Set syncState SYNC_DONE\n");
	//statusSlider->value(value);
	//statusSlider->redraw();
	printf("STATE -> SYNC_DONE\n");
	syncState = SYNC_DONE;
	//msgState = SYNC_DONE;
	sprintf(msg, "%s Synchronization Complete",
		((SyncData *) curNode->user_data())->appName);
	statusMsg->value(msg);
	//strcpy(msg, coder.EndSync(((SyncData*)curNode->user_data())->nxAppName).c_str());
	//ret = Write_Fd(sync_id, msg, strlen(msg));
    }
}

int
Sync::start_app()
{

  printf("---- start_app() ----\n");
  string msg;
  
  msg = "Synchronizing ";
  msg.append(((SyncData *) curNode->user_data())->appName);
  
  statusMsg->value(msg.c_str());
  sync_id = StartApp(((SyncData *) curNode->user_data())->nxAppName, 0, 2);

  DPRINT("start_app() sync_id [%d]\n", sync_id);
  
  if (sync_id < 0) {
    msg = "Unable to start ";
    msg.append(((SyncData *) curNode->user_data())->appName);
    do_error(msg);
  }
 
  printf("---- return %d ----\n", sync_id);
  return sync_id;
}

char *
Sync::get_next_app()
{

    if (NULL == curNode)
	curNode = syncTree->traverse_start();
    else
	curNode = syncTree->traverse_forward();

    while (curNode) {
	if (true == ((SyncData *) curNode->user_data())->appSync)
	    return ((SyncData *) curNode->user_data())->nxAppName;
	else
	    curNode = syncTree->traverse_forward();
    }
    return NULL;
}

void
Sync::checkIt_callback(Fl_Widget * fl, void *l)
{

    Fl_Toggle_Node *node = syncTree->selected();

    if (node) {
	if ((Fl::event_x() >= 17) && (Fl::event_x() <= 29)) {
	    syncTree->unselect();
	    if (node->pixmap() == echeckPixmap) {
		((SyncData *) node->user_data())->appSync = true;
		node->pixmap(checkPixmap);
	    } else if (node->pixmap() == checkPixmap) {
		((SyncData *) node->user_data())->appSync = false;
		node->pixmap(echeckPixmap);
	    }
	}
    }
    if (syncTree->selection_count() > 0)
	syncTree->unselect();
}

void
Sync::exit_callback(Fl_Widget * fl, void *l)
{

    mainWindow->hide();
    exit(0);
}

Fl_Window *
Sync::get_main_window()
{
    if (mainWindow)
	return mainWindow;
    else
	return 0;
}

void
Sync::show_default_window()
{
    show_window(syncWindow->GetWindowPtr());
}

void
Sync::do_error(string err)
{
  if (err.c_str()) 
    statusMsg->value(err.c_str());
  
  statusMsg->redraw();
  abortButton->do_callback();
  printf("STATE -> SYNC_ERROR\n");
  syncState = SYNC_ERROR;
  
  first_app = false;
}

void
Sync::do_abort(void)
{
  printf("**** ABORTING ****\n");

  statusMsg->value("Synchronization aborted");
  statusMsg->redraw();

  abortButton->do_callback();
  printf("STATE -> SYNC_ABORT\n");
  syncState = SYNC_ABORT;

  first_app = false;
}

void
Sync::add_apps(Fl_Toggle_Tree * syncTree)
{

    int size;
    int count, i;

    db_handle *db;
    char *searchList, *l;

    db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);

    if (!db) {
	printf("Error - Couldn't open the par database %s\n",
	       db_getDefaultDB());
	return;
    }
    //FIXME this may need to be changed!
    size = par_getCapability(db, "nxsync", (void **) &searchList);

    if (size <= 0) {
	db_closeDB(db);
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

	appNode = syncTree->add_next(title, 0, echeckPixmap);

	if (appNode) {
	    SyncData *data = new SyncData;
	    strcpy(data->appName, title);
	    strcpy(data->nxAppName, application);
	    data->appSync = false;
	    appNode->user_data(data);
	    syncTree->traverse_up();
	}
    }
    db_closeDB(db);

}
