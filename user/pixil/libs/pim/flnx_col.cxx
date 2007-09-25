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

#ifdef TesterClass

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Button.H>

#include <iostream>
#include <unistd.h>

#include "nxapp.h"

#define BY 30

/*
 *
 * Tester Class
 *
 */

class IPCTester:public NxApp
{

  public:

    int fd;
    static int ipc;

      IPCTester()
    {
	fd = Add_Fd("flnx_col", ClientIPCHandler);
    }
     ~IPCTester()
    {
	Remove_Fd(fd);
    }

    // Client IPC Handler
    static void ClientIPCHandler(int fd, void *o);

    // Callbacks
    static void ResetCB(Fl_Widget * fl, void *o);

    // INITIATE
    static void InitiateCB(Fl_Widget * fl, void *o);

    // EXECUTE
    static void ExecuteAllClientsCB(Fl_Widget * fl, void *o);
    static void ExecuteResizeKbdCB(Fl_Widget * fl, void *o);
    static void ExecuteResizeWinCB(Fl_Widget * fl, void *o);
    static void ExecuteNoGuiSearchCB(Fl_Widget * fl, void *o);
    static void ExecuteShowCB(Fl_Widget * fl, void *o);
    static void ExecuteHideCB(Fl_Widget * fl, void *o);
    static void ExecuteSearchCB(Fl_Widget * fl, void *o);
    static void ExecuteExitCB(Fl_Widget * fl, void *o);

    // REQUEST
    static void RequestDefWinXCB(Fl_Widget * fl, void *o);
    static void RequestDefWinYCB(Fl_Widget * fl, void *o);
    static void RequestDefWinWCB(Fl_Widget * fl, void *o);
    static void RequestDefWinHCB(Fl_Widget * fl, void *o);
    static void RequestShowWinXCB(Fl_Widget * fl, void *o);
    static void RequestShowWinYCB(Fl_Widget * fl, void *o);
    static void RequestShowWinWCB(Fl_Widget * fl, void *o);
    static void RequestShowWinHCB(Fl_Widget * fl, void *o);
    static void RequestKbdRunCB(Fl_Widget * fl, void *o);

    // TERMINATE
    static void TerminateCB(Fl_Widget * fl, void *o);

};


// Control Window
Fl_Window *w;
Fl_Window *mw;
Fl_Input *application;
Fl_Menu_Button *result;
Fl_Button *reset;


// Tests Window
// INITIATE
Fl_Button *initiateBut;

// EXECUTE
Fl_Button *executeAllClientsBut;
Fl_Button *executeResizeKbdBut;
Fl_Button *executeResizeWinBut;
Fl_Button *executeNoGuiSearchBut;
Fl_Button *executeShowBut;
Fl_Button *executeHideBut;
Fl_Button *executeSearchBut;
Fl_Button *executeExitBut;

// REQUEST
Fl_Button *requestDefWinXBut;
Fl_Button *requestDefWinYBut;
Fl_Button *requestDefWinWBut;
Fl_Button *requestDefWinHBut;
Fl_Button *requestShowWinXBut;
Fl_Button *requestShowWinYBut;
Fl_Button *requestShowWinWBut;
Fl_Button *requestShowWinHBut;
Fl_Button *requestKbdRunBut;

// TERMINATE
Fl_Button *terminateBut;

int
main(int argc, char *argv[])
{

    IPCTester t;

    mw = new Fl_Window(0, 0, 305, 75, "Control Window");

    application = new Fl_Input(5, 25, 300, 20, "NX Application");
    application->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);

    reset = new Fl_Button(225, 3, 80, 20, "Reset");
    reset->callback(IPCTester::ResetCB);

    result = new Fl_Menu_Button(5, 50, 300, 20, "Results");
    mw->end();

    w = new Fl_Window(0, 0, 350, 350, "Tests Window");

    {
	Fl_Output *o = new Fl_Output(5, 0, 80, 30);
	o->color(FL_GRAY);
	o->box(FL_FLAT_BOX);
	o->textsize(10);
	o->value("I. Start Here.");
    }

    // INITIATE
    initiateBut = new Fl_Button(5, BY, 80, 30, "Inititate");
    initiateBut->color(FL_YELLOW);
    initiateBut->callback(IPCTester::InitiateCB);

    {

	Fl_Output *o = new Fl_Output(90, 0, 80, 30);
	o->color(FL_GRAY);
	o->box(FL_FLAT_BOX);
	o->textsize(10);
	o->value("II. Execute");
    }


    // EXECUTE
    executeAllClientsBut = new Fl_Button(90, BY, 80, 30, "All Clients");
    executeAllClientsBut->color(FL_YELLOW);
    executeAllClientsBut->callback(IPCTester::ExecuteAllClientsCB);

    executeResizeKbdBut =
	new Fl_Button(90, BY + 32, 80, 30, "Move\nKeyboard");
    executeResizeKbdBut->color(FL_YELLOW);
    executeResizeKbdBut->callback(IPCTester::ExecuteResizeKbdCB);

    executeResizeWinBut =
	new Fl_Button(90, BY + 64, 80, 30, "Resize\nWindow");
    executeResizeWinBut->color(FL_YELLOW);
    executeResizeWinBut->callback(IPCTester::ExecuteResizeWinCB);

    executeNoGuiSearchBut =
	new Fl_Button(90, BY + 96, 80, 30, "No Gui\nSearch");
    executeNoGuiSearchBut->callback(IPCTester::ExecuteNoGuiSearchCB);

    executeShowBut = new Fl_Button(90, BY + 128, 80, 30, "Show");
    executeShowBut->color(FL_YELLOW);
    executeShowBut->callback(IPCTester::ExecuteShowCB);

    executeHideBut = new Fl_Button(90, BY + 160, 80, 30, "Hide");
    executeHideBut->color(FL_YELLOW);
    executeHideBut->callback(IPCTester::ExecuteHideCB);

    executeSearchBut = new Fl_Button(90, BY + 192, 80, 30, "Search");
    executeSearchBut->color(FL_YELLOW);
    executeSearchBut->callback(IPCTester::ExecuteSearchCB);

    executeExitBut = new Fl_Button(90, BY + 224, 80, 30, "Exit");
    executeExitBut->color(FL_YELLOW);
    executeExitBut->callback(IPCTester::ExecuteExitCB);


    // REQUEST
    {
	Fl_Output *o = new Fl_Output(175, 0, 80, 30);
	o->color(FL_GRAY);
	o->box(FL_FLAT_BOX);
	o->textsize(10);
	o->value("III. Request");
    }

    requestDefWinXBut = new Fl_Button(175, BY, 80, 30, "Default X");
    requestDefWinXBut->color(FL_YELLOW);
    requestDefWinXBut->callback(IPCTester::RequestDefWinXCB);

    requestDefWinYBut = new Fl_Button(175, BY + 32, 80, 30, "Default Y");
    requestDefWinYBut->color(FL_YELLOW);
    requestDefWinYBut->callback(IPCTester::RequestDefWinYCB);

    requestDefWinWBut = new Fl_Button(175, BY + 64, 80, 30, "Default\nWidth");
    requestDefWinWBut->color(FL_YELLOW);
    requestDefWinWBut->callback(IPCTester::RequestDefWinWCB);

    requestDefWinHBut =
	new Fl_Button(175, BY + 96, 80, 30, "Default\nHeight");
    requestDefWinHBut->color(FL_YELLOW);
    requestDefWinHBut->callback(IPCTester::RequestDefWinHCB);

    requestShowWinXBut = new Fl_Button(175, BY + 128, 80, 30, "Shown X");
    requestShowWinXBut->color(FL_YELLOW);
    requestShowWinXBut->callback(IPCTester::RequestShowWinXCB);

    requestShowWinYBut = new Fl_Button(175, BY + 160, 80, 30, "Show Y");
    requestShowWinYBut->color(FL_YELLOW);
    requestShowWinYBut->callback(IPCTester::RequestShowWinYCB);

    requestShowWinWBut = new Fl_Button(175, BY + 192, 80, 30, "Show\nWidth");
    requestShowWinWBut->color(FL_YELLOW);
    requestShowWinWBut->callback(IPCTester::RequestShowWinWCB);

    requestShowWinHBut = new Fl_Button(175, BY + 224, 80, 30, "Show\nHeight");
    requestShowWinHBut->color(FL_YELLOW);
    requestShowWinHBut->callback(IPCTester::RequestShowWinHCB);

    requestKbdRunBut =
	new Fl_Button(175, BY + 256, 80, 30, "Keboard\nRunning?");
    requestKbdRunBut->color(FL_YELLOW);
    requestKbdRunBut->callback(IPCTester::RequestKbdRunCB);

    // TERMINATE

    {
	Fl_Output *o = new Fl_Output(260, 0, 80, 30);
	o->color(FL_GRAY);
	o->box(FL_FLAT_BOX);
	o->textsize(10);
	o->value("IV. Quit Here");
    }

    terminateBut = new Fl_Button(260, BY, 80, 30, "Terminate");
    terminateBut->color(FL_YELLOW);
    terminateBut->callback(IPCTester::TerminateCB);

    w->end();

    mw->show(argc, argv);
    w->show(argc, argv);

    Fl::run();

}

int
    IPCTester::ipc;

void
IPCTester::ClientIPCHandler(int fd, void *o)
{

    cout << "ClientIPCHandler: file descriptor tripped.\n";

    int ipc_id = -1;

    if (o == NULL) {

	char passMsg[255];
	int length = sizeof(passMsg);
	ipc_id = NxApp::Instance()->Read_Fd(passMsg, &length);

	cout << "Returned... " << passMsg << "." << endl;

	result->add(passMsg);

    } else {

	cout << "Error... void *o not NULL.\n";

    }

}

void
IPCTester::ResetCB(Fl_Widget * fl, void *o)
{

    result->clear();

}

void
IPCTester::InitiateCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^INITIATE^0^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);

}

void
IPCTester::ExecuteAllClientsCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^EXECUTE^all_clients^hide");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::ExecuteResizeKbdCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^EXECUTE^move_keyboard^0^0^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::ExecuteResizeWinCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^EXECUTE^resize_win^0^0^240^200^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::ExecuteNoGuiSearchCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^EXECUTE^noguisearch^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);

}

void
IPCTester::ExecuteShowCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^EXECUTE^show^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::ExecuteHideCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^EXECUTE^hide^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::ExecuteSearchCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^EXECUTE^search^Jeff^240");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::ExecuteExitCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^EXECUTE^exit^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::RequestDefWinXCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^REQUEST^default_win_x^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::RequestDefWinYCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^REQUEST^default_win_y^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::RequestDefWinWCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^REQUEST^default_win_w");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::RequestDefWinHCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^REQUEST^default_win_h^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::RequestShowWinXCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^REQUEST^show_win_x^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::RequestShowWinYCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^REQUEST^show_win_y^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::RequestShowWinWCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^REQUEST^show_win_w^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::RequestShowWinHCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^REQUEST^show_win_h^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::RequestKbdRunCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^REQUEST^keyboard_running^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);


}

void
IPCTester::TerminateCB(Fl_Widget * fl, void *o)
{

    if (application->value() == NULL)
	return;

    ipc = NxApp::Instance()->Find_Fd((char *) application->value());

    char msg[255];
    int len = sizeof(msg);
    strcpy(msg, "flnx_col^TERMINATE^0^");

    NxApp::Instance()->Write_Fd(ipc, msg, len);

}

#endif
