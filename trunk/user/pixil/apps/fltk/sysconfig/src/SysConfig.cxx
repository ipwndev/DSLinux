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

// System header files
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <libgen.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

// Local header files
#include <ipc/colosseum.h>
#include <FL/x.H>
#include <FL/Fl_Group.H>
#include <nxmenubar.h>
#include <nxmenubutton.h>
#include <par/par.h>
#include "sysconfig.h"

#if 1	// xAMADEUS
#define dlopen(x,y) 0
#define dlsym(x,y)  0
#define dlclose(x)  0
#define dlerror()   0
#endif

SysConfig::SysConfig(void)
{
    _syswin = NULL;
    _appcnt = 0;
    _ipcid = -1;
    _ai = NULL;
    _ipcfxn = NULL;
    _current = -1;

    /* Get the list of shared libraries */

    if (BuildPluginList() < 0) {
	fprintf(stderr, "No System Configuration plugins registered.\n");
	exit(-1);
    }

    /* Create the main window */
    MakeWindow();

    /* See if we have a default application already set up */
    /* If so, use that, otherwise just pick the first app  */

    char *defplugin = GetDfltUtil();

    if (!defplugin)
	ShowPlugin(0);
    else {
	int idx = GetIndex(defplugin);
	ShowPlugin((idx == -1) ? 0 : idx);
	free(defplugin);
    }

    /* Register with Colosseum */

    if ((_ipcid = Add_Fd("sysconfig", _ClientIPCHandler) < 0))
	fprintf(stderr, "Unable to register with colosseum!\n");
}

SysConfig::~SysConfig()
{
    for (int i = 0; i < _appcnt; i++)
	_ai[i].close();

    delete _syswin;
    delete[]_ai;
}

int
SysConfig::GetIndex(char *soname)
{
    for (int i = 0; i < _appcnt; i++)
	if (!strcmp(_ai[i].soName, soname))
	    return i;

    return -1;
}

AppInfo_t *
SysConfig::GetAppInfo(char *soname)
{
    int i = GetIndex(soname);
    return GetAppInfo(i);
}

AppInfo_t *
SysConfig::GetAppInfo(int index)
{
    if (index == -1 || index > _appcnt)
	return 0;
    return &_ai[index];
}

void
SysConfig::SetLabel(char *title)
{
    if (title == NULL || *title == '\0')
	return;
    _sysmb->label(title);
}

void
SysConfig::UpdDfltUtil(char *soname)
{
    db_handle *hPdb;		// Handler to the par database

    if (soname == NULL || *soname == '\0')
	return;

    if ((hPdb = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RW)) == NULL)
	return;

    par_setGlobalPref(hPdb, "SystemConfigurator", "default_util", PAR_TEXT,
		      soname, strlen(soname) + 1);

    db_closeDB(hPdb);

    return;
}

char *
SysConfig::GetDfltUtil(void)
{
    char *ret = (char *) calloc(128, 1);

    db_handle *hPdb;		// Handler to the par database

    if ((hPdb = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RW)) == NULL)
	return 0;

    par_getGlobalPref(hPdb, "SystemConfigurator", "default_util", PAR_TEXT,
		      ret, sizeof(ret) - 1);

    db_closeDB(hPdb);
    return ret;
}

int
my_compare(const struct dirent *dir)
{
    if (!fnmatch("*.so", dir->d_name, 0))
	return 1;
    return 0;
}

int
SysConfig::BuildPluginList(void)
{
    struct dirent **entries;
    int i;

    char **av = NULL, *par_data;
    int datasz;			// Size of the data
    db_handle *hPdb;		// Handle to the par database

    if ((hPdb = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY)) == NULL) {
	fprintf(stderr, "Unable to open the par database!\n");
	return (-1);
    }

    datasz = par_getCapability(hPdb, "SystemConf", (void **) &par_data);

    _appcnt = scandir(par_data, &entries, my_compare, alphasort);
    if (_appcnt == -1) {
	printf("Error - %s\n", strerror(errno));
	return -1;
    }

    /* Stick the files into the structure */

    _ai = new AppInfo_t[_appcnt];
    av = (char **) malloc(_appcnt * sizeof(char *));

    for (i = 0; i < _appcnt; i++) {
	struct stat s;
	char path[256];

	av[i] = 0;

	sprintf(path, "%s/%s", par_data, entries[i]->d_name);

	if (stat(path, &s))
	    continue;
	if (!S_ISREG(s.st_mode) && !S_ISLNK(s.st_mode))
	    continue;

	av[i] = strdup(path);
    }

    if (entries)
	free(entries);

    int aiidx = 0;

    for (i = 0; i < _appcnt; i++) {
	if (!av[i])
	    continue;

	memset(&_ai[aiidx], 0, sizeof(_ai[aiidx]));

	/* Load the shared library */
	_ai[aiidx].handle = dlopen(av[i], RTLD_NOW | RTLD_GLOBAL);

	if (!_ai[aiidx].handle) {
	    printf("Unable to load the plugin %s\n[%s]\n", av[i], dlerror());
	    continue;
	}

	/* Copy the name of the library for reference */
	strncpy(_ai[aiidx].soName, basename(av[i]),
		sizeof(_ai[aiidx].soName));

	/* Get the hooks for the various funcs */

	_ai[aiidx].create = (void (*)(int, int, int, int))
	    dlsym(_ai[aiidx].handle, "plugin_create");

	_ai[aiidx].show = (void (*)(void))
	    dlsym(_ai[aiidx].handle, "plugin_show");

	_ai[aiidx].hide = (void (*)(void))
	    dlsym(_ai[aiidx].handle, "plugin_hide");

	_ai[aiidx].info = (void (*)(char *, int))
	    dlsym(_ai[aiidx].handle, "plugin_info");

	_ai[aiidx].close = (void (*)(void))
	    dlsym(_ai[aiidx].handle, "plugin_close");

	if (_ai[aiidx].info)
	    _ai[aiidx].info(_ai[aiidx].soTitle, sizeof(_ai[aiidx].soTitle));

	/* Escape unfriendly chars */
	char *title = strchr(_ai[aiidx].soTitle, '&');

	while (title) {
	    if (*(title + 1) != '&') {
		if (strlen(_ai[aiidx].soTitle) + 1 <
		    sizeof(_ai[aiidx].soTitle)) {
		    memmove(title + 1, title, strlen(title) + 1);
		    title = title + 1;
		} else {
		    memmove(title - 1, title, strlen(title) + 1);
		    title = title - 1;
		}
	    } else
		title++;
	    title = strchr(title, '&');
	}

	free(av[i]);
	aiidx++;
    }

    _appcnt = aiidx;

    if (par_data)
	free(par_data);
    if (av)
	free(av);

    return (_appcnt);
}

void
SysConfig::ShowPlugin(int idx)
{

    if (_current != -1)
	_ai[_current].hide();

    _ai[idx].show();
    _current = idx;

    SetLabel(_ai[_current].soTitle);
}

void
SysConfig::HidePlugin(int idx)
{

    if (_current != -1)
	_ai[_current].hide();

    _current = -1;
}

void
SysConfig::ForcePlugin(char *plugin)
{

    int i = GetIndex(plugin);
    if (i != -1)
	ShowPlugin(i);
    else
	printf("Error - plugin %s is not available\n");
}

void
SysConfig::MakeWindow(void)
{
    _syswin = new Fl_Window(0, 0, W_W, W_H, "System Configuration");
    _syswin->color(NxApp::Instance()->getGlobalColor(APP_BG));

    NxMenuButton *mb = new NxMenuButton(MB_X, MB_Y, (2 * MB_W), MB_H);
    mb->label("");
    mb->callback(*mb_cb);
    for (int i = 0; i < _appcnt; i++)
	mb->add(_ai[i].soTitle);

    _sysmb = mb;

    /* Now that we have the window, make the individual plugin windows */

    for (int i = 0; i < _appcnt; i++)
	_ai[i].create(0, MB_Y + MB_H + 10, W_W, W_H - (MB_H + 10));

    _syswin->end();
    set_shown_window(_syswin);

    return;
}				// end of SysConfig::MakeWindow()

void
SysConfig::showMain(void)
{
    if (_syswin)
	_syswin->show();
}

/////////////////////////////////////////////////////////////////////////////////
//
//      SysConfig: Static FLTK Callback functions
//
/////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************\
**
**	Function:	static void mb_cb()
**	Desc:		Callback for the menu bar
**	Accepts:	Fl_Widget *w = Widget which invoked the call back
**				void *d = Any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
SysConfig::mb_cb(Fl_Widget * w, void *d)
{
    SysConfig *me = (SysConfig *) NxApp::Instance();
    NxMenuButton *mb = (NxMenuButton *) w;

    me->UpdDfltUtil(me->_ai[mb->value()].soName);
    ((SysConfig *) NxApp::Instance())->ShowPlugin(mb->value());
    return;
}

/*******************************************************************************\
**
**	Function:	void ClientIPCHandler()
**	Desc:		Default system configuration ipc handler
**	Accepts:	int fd = file descriptor
**				void *msg = The message to send or storage for incoming message
**				int ipc_id = ????
**	Returns: 	Nothing (void)
**
\*******************************************************************************/
void
SysConfig::ClientIPCHandler(int fd, void *msg, int ipc_id)
{
    SysConfig *me = (SysConfig *) NxApp::Instance();
    char *pservice = NULL, *pcmd = NULL, *pdata = NULL;

    // Parse out the message
    pservice = (char *) msg;
    if ((pcmd = strchr(pservice, '^')) != NULL) {
	*pcmd++ = '\0';
	if ((pdata = strchr(pcmd, '^')) != NULL)
	    *pdata++ = '\0';
    }				// end of if

    // Process standard messages, or send it to a registered function
    if (!strcmp(pcmd, "SHOW")) {
	Fl_Window *w = me->get_main_win();

	if (w == NULL)
	    return;

	while (w->parent() != NULL)
	    w = (Fl_Window *) w->parent();
#ifdef NANOX
	GR_WINDOW_ID wid = fl_xid(w);
	GR_WINDOW_INFO info;

	GrGetWindowInfo(wid, &info);
	GrMapWindow(info.parent);
	GrRaiseWindow(info.parent);
	GrSetFocus(wid);
#endif
    }				// end of if 
    else if (!strcmp(pcmd, "SYSCONF_TERMINATE")) {
    }				// end of else if
    else {
	if (me->_ipcfxn)
	    me->_ipcfxn(fd, msg, ipc_id);
    }				// end of else

    return;
}
