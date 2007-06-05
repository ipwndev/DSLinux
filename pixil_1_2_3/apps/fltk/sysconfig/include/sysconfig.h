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



#ifndef		SYSCONFIG_H_INCLUDED
#define		SYSCONFIG_H_INCLUDED	1

// System header files 
#include <sys/types.h>


// Local header files 
#include <nxapp.h>
#include <FL/Fl_Window.H>
#include <nxmenubutton.h>


// Typedef, macro, enum/struct/union definitions
typedef struct
{
    void *handle;
    Fl_Group *drawable;

    char soName[50 + 1];
    char soTitle[25 + 1];

    void (*create) (int, int, int, int);
    void (*show) (void);
    void (*hide) (void);
    void (*info) (char *, int);
    void (*close) (void);
}
AppInfo_t;

// Class definition
class SysConfig:public NxApp
{
  private:
    char *GetDfltUtil(void);

    void ShowPlugin(int);
    void HidePlugin(int);

    int GetIndex(char *soname);
    int BuildPluginList(void);

    int _current;

  public:
      SysConfig(void);
     ~SysConfig(void);

    void ForcePlugin(char *);

    // Public members
    static int _gflags;		// This is a static member

    // Public methods
    Fl_Window *get_main_win()
    {
	return _syswin;
    }				// Returns the main window
    AppInfo_t *GetAppInfo(char *appname);	// Returns the appinfo for appname
    AppInfo_t *GetAppInfo(int idx);	// Returns the appinfo at idx
    void SetLabel(char *title);	// Sets the label of this title
    void RegisterIPC(void (*ipcf) (int fd, void *msg, int ipc_id))
    {
	if (ipcf)
	    _ipcfxn = ipcf;
	return;
    }
    void UpdDfltUtil(char *appname);	// Updates the value of the default utility

    void showMain(void);

  private:
    // Private members
    Fl_Window * _syswin;	// Main system configuration window
    NxMenuButton *_sysmb;	// Main system menu button widget
    int _appcnt,		// Number of apps
      _ipcid;			// Colosseum ipc id
    AppInfo_t *_ai;		// App info list (dynamically created)
    void (*_ipcfxn) (int fd, void *msg, int ipc_id);	// Registered function


    // Private methods

    int FindAppIdx(char *appname);	// Finds the index of the appname
    void SortAppV(char *pvec, char **av, int ac);	// Sorts the vector list
    int LaunchApp(int idx);	// "Launches" the application
    void MakeWindow();		// Makes the window

    // Static private FLTK call backs
    static void mb_cb(Fl_Widget * w, void *d);	// MenuButton callback

    // Static private NON-GUI call backs
    virtual void ClientIPCHandler(int fd, void *msg, int ipc_id = -1);
};

#endif //      SYSCONFIG_H_INCLUDED
