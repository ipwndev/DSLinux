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


#ifndef __SYNC_H_
#define __SYNC_H_

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <Flek/Fl_Toggle_Tree.H>
#include <FL/Fl_Button.H>

#include <icons/echeck.xpm>
#include <icons/check.xpm>

#include <nxapp.h>
#include <nxbox.h>
#include <nxwindow.h>
#include <nxdb.h>
#include <nxbutton.h>
#include <nxscroll.h>
#include <nxslider.h>

#include <sync/coder.h>

extern "C"
{
#include <ipc/colosseum.h>
}

#define APP_NAME "Sync"
#define SYNC_DONE 	0x00
#define SYNC_ERROR 	0x01
#define SYNC_START 	0x02
#define SYNC_ABORT 	0x04
#define SYNC_WAIT 	0x08
#define SYNC_EXE		0xF0

#define SYNC_PRECONNECT 0x10
#define SYNC_CONNECT    0x11
#define SYNC_DISCONNECT 0x12
#define SYNC_PENDING    0x14

struct SyncData
{
    char appName[50];
    char nxAppName[50];
    bool appSync;
};

class Sync:public NxApp
{
  private:

    static NxWindow *mainWindow;
    static NxPimPopWindow *statusWindow;
    static NxPimWindow *syncWindow;
    static Fl_Toggle_Tree *syncTree;
    static NxScroll *syncScroll;
    static NxButton *syncButton;
    static NxButton *abortButton;
    static NxButton *okButton;
    static NxOutput *statusMsg;
    static NxSlider *statusSlider;
    static Fl_Pixmap *echeckPixmap;
    static Fl_Pixmap *checkPixmap;
    int send_update_status(string app_id);
    void get_update_status(vector < string > &vmessages);
    void update_error(vector < string > &vmessages);
    void sync_error();
    void sync_abort();
    void sync(char *service);
    int initiate_sync();
    int begin_app();
    int start_sync();
    static void checkIt_callback(Fl_Widget * fl, void *l);
    void add_apps(Fl_Toggle_Tree * syncTree);
    int syncState;
    static Fl_Toggle_Node *curNode;
    char *get_next_app();
    int start_app();
    MsgCoder coder;
    int sync_id;
    int msgState;
    string cur_app;
    void do_error(string err);
    void do_abort(void);

    static bool first_app;

    int agent_connect(void);
    int agent_disconnect(void);

    virtual void ClientIPCHandler(int fd, void *o, int ipc_id = -1);

  public:

      Sync(char *app);
     ~Sync();
    void MakeSyncWindow();
    void MakeStatusWindow();

    static void timer_callback(void *);
    static void exit_callback(Fl_Widget * fl, void *l);
    static void sync_callback(Fl_Widget * fl, void *l);
    static void abort_callback(Fl_Widget * fl, void *l);
    static void ok_callback(Fl_Widget * fl, void *l);
    Fl_Window *get_main_window();
    void show_default_window();
};


#endif
