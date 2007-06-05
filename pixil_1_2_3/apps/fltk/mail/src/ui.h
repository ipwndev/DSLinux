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


#ifndef UI_H
#define UI_H

#include <signal.h>

#include <FL/Fl.H>

#include <nxbox.h>
#include <nxapp.h>
#include <nxwindow.h>
#include <nxbutton.h>
#include <nxmultilineinput.h>
#include <nxmultilineoutput.h>
#include <nxmenubutton.h>
#include <nxscroll.h>
#include <nxbrowser.h>
#include <nxsecretinput.h>

#include "mailengine.h"
#include "settings.h"

#define APP      "Email"
#define ABOUT    "About Email"
#define EMAIL_DB "email"
#define CAT_DB   "email_cat"
#define ACCT_DB  "email_acct"
#define CAT_NUM  4

class NxMail:public NxApp
{

    ////////////////////////////////////////////////////////////////////////////////
    // Instance Variables

  private:
    // Global Access Poin
    static NxMail *_inst;

    // Email Engine
    mailSettings *settings;
    mailEngine *engine;
    int current_account;

    // Database
    NxDb *db;

    // Windows
    NxWindow *containerWindow;
    NxPimWindow *mainWindow;
    NxPimWindow *editorWindow;
    NxPimWindow *settingsWindow;
    NxPimWindow *viewWindow;

    // mainWindow widgets
    NxOutput *l_Status;
    char statusBuffer[50];
    NxBrowser *m_List;
    NxMultilineOutput *m_View;

    // editorWindow widgets  
    NxInput *i_To;
    NxInput *i_CC;
    NxInput *i_Subject;
    NxMultilineInput *i_Message;

    // viewWindow widgets
#ifdef NOTUSED
    MimeGroup *g_Mime;
#endif
    Fl_Browser *m_Mime;

    // settingsWindow widets
    int accountCount;
    Fl_Menu_Item *accountArray;
    NxInput *i_Server;
    NxInput *i_Name;
    NxSecretInput *i_Pass;
    NxInput *i_SMTPServer;
    NxInput *i_SMTPName;

    NxMenuButton *m_Accounts;
    NxBox *l_Account;
#ifdef NOTUSED
    NxInput *i_Port;
#endif
    Fl_Group *g_Radio;
    NxButton *b_IMAP;
    NxButton *b_POP3;

    ////////////////////////////////////////
    // Category Lists

    NxCategoryList *cat_list[CAT_NUM];
    static NxCategoryList *main_category;
    static NxCategoryList *editor_category;
    NxCategoryList *settings_category;
    static NxCategoryList *view_category;

    ////////////////////////////////////////////////////////////////////////////////
    // Messages

  public:

    ////////////////////////////////////////
    // Public Interface

      NxMail(int argc, char *argv[]);
      virtual ~ NxMail();
    static void exit_callback(Fl_Widget * fl, void *o);

    // Window Messages
    Fl_Window *GetMainWindow();
    void ShowDefaultWindow();

    // Global Access Point
    static NxMail *Inst();

    // Email engine
    char *EngineGetSettings(char *szSetting);
    MAILERROR EngineOpenSession();
    void EngineCloseSession()
    {
	engine->close_session();
    }
    MAILERROR EngineSendMessage(char *server, int port,
				nxmail_header_t * header, char *body,
				int size);
    int EngineGetMsgCount()
    {
	return engine->message_count();
    }
    nxmail_header_t *EngineFetchHeader(int m)
    {
	return engine->fetch_header(m);
    }
    nxmail_body_t *EngineFetchMsg(int m)
    {
	return engine->fetch_message(m);
    }
    int EngineDeleteMsg(int m)
    {
	return engine->delete_message(m);
    }

    // mainWindow
    static void MainSetCategory(char *szCat);
    void MainSetStatus(const char *status);
    Fl_Browser *MainGetMList()
    {
	return m_List;
    }
    void MainShowWindow();

    // editorWindow
    void EditorClearFields();
    void EditorSetFields(char *to, char *cc, char *subject);
    void EditorIndentText(char *str);
    void EditorShowWindow();
    const char *EditorGetTo()
    {
	return i_To->value();
    }
    const char *EditorGetCC()
    {
	return i_CC->value();
    }
    const char *EditorGetSubject()
    {
	return i_Subject->value();
    }
    const char *EditorGetMsg()
    {
	return i_Message->value();
    }
    int EditorGetMsgSize()
    {
	return i_Message->size();
    }

    // settingsWindow
    void SettingsUpdateFields();
    void SettingsUpdateValues(int);
    void SettingsShowWindow();

    // viewerWindow
    void ViewerShowWindow();
    NxMultilineOutput *ViewerGetMView()
    {
	return m_View;
    }
    Fl_Browser *ViewerGetMimeWidget()
    {
	return m_Mime;
    }
    static void ViewerReplyMsgCB(Fl_Widget * fl, void *o);
    static void ViewerReplyAllMsgCB(Fl_Widget * fl, void *o);
    static void ViewerReplyFwdMsgCB(Fl_Widget * fl, void *o);

#ifdef NOTUSED
    void ViewerShowMimeWidget();
    void ViewerHideMimeWidget();
#endif

  private:
    void MakeMainWindow();
    void MakeEditorWindow();
    void MakeSettingsWindow();
    void MakeViewWindow();
    void CloseNanoMail(int signal);

    // Database messages
    char *CatRecord(int catid, string cat_name);

    // FLNX-Colosseum IPC messages
    virtual void ClientIPCHandler(int fd, void *o, int ipc_id = -1);
    void ExecuteSearch(int ipc_id, char *searchStr, int width);

    // mainWindow messages
    static void MainCloseCB(Fl_Widget * fl, void *o);

    // editorWindow messages
    static void EditorSendCB(Fl_Widget * fl, void *o);
    static void EditorCancelCB(Fl_Widget * fl, void *o);

    // viewerWindow messages
    static void ViewerAccountCB(Fl_Widget * fl, void *o);
    static void ViewerDeleteMsgCB(Fl_Widget * fl, void *o);
    static void ViewerSaveMsgCB(Fl_Widget * fl, void *o);
    static void ViewerViewMsgCB(Fl_Widget * fl, void *o);
    static void ViewerCloseCB(Fl_Widget * fl, void *o);

    // settingsWindow messages
    static void SettingsSaveCB(Fl_Widget * fl, void *o);
    static void SettingsCancelCB(Fl_Widget * fl, void *o);
    static void SettingsAccountCB(Fl_Widget * fl, void *o);

    ////////////////////////////////////////
    // Category list callbacks

    static void ChangeCatCB(Fl_Widget * fl, void *o);
    static void ChangeAcctCB(Fl_Widget * fl, void *o);

};

#ifdef NOTUSED

class MimeGroup:public Fl_Group
{
  private:

    NxButton * b_Save;
    NxButton *b_View;

    static void button_callback(Fl_Widget * widget, void *win);

  public:

      Fl_Browser * m_Mime;
      MimeGroup(int, int, int, int);
};

#endif // NOTUSED
#endif // UI_H
