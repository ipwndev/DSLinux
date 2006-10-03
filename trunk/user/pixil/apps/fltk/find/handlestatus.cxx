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
#ifdef NOTUSED
#include "nxfindstatus.h"
#endif

#include "nxfindapp.h"
#include "handlestatus.h"

#ifdef NOTUSED
static searchStatus *getThread;

static c_statusWindow *statusGetUI;
#endif

char statusStr[150];

#ifdef NOTUSED

static void
threadMonitor(void *data)
{
    searchStatus *curThread = (searchStatus *) data;
    statusGetUI->setStatus(curThread->getStatus());

    bool val = statusGetUI->getExpectingFlag();
    bool cancel = statusGetUI->getCancelFlag();

    unsigned char done = 0;

    switch (curThread->getState()) {
    case STATE_INIT:

	if (cancel) {
	    curThread->setState(STATE_CANCEL);
	} else {
	    statusGetUI->setStatus("Starting Search...");
	    curThread->setState(STATE_READY);
	}
	break;

#ifdef NOTUSED
    case STATE_IDLE:
	if (val) {
	    curThread->setState(STATE_READY);
	}

	break;
#endif

    case STATE_READY:
	if (cancel)
	    curThread->setState(STATE_CANCEL);
	else {
	    statusGetUI->show();
	    curThread->setState(STATE_NEXT);
	}

	break;

    case STATE_DONE:
	done = 1;
	if (false == cancel) {
	    statusGetUI->hide();
	    statusGetUI->setStatus("Done searching...");
	}

	statusGetUI->setExpectingFlag(false);
	statusGetUI->setCancelFlag(false);
	break;

    case STATE_BUSY:
	if (cancel)
	    curThread->setState(STATE_CANCEL);
	if (!val)
	    curThread->setState(STATE_NEXT);
	else {
	    char status[150];
	    sprintf(status, "Searching %s", statusGetUI->getService());
	    statusGetUI->setStatus(status);
	    statusGetUI->search();
	    curThread->setState(STATE_NEXT);
	}
	break;

    case STATE_NEXT:
	char *service;

	if (cancel) {
	    curThread->setState(STATE_CANCEL);
	} else {
	    service = statusGetUI->getNextService();
	    if (NULL == service)
		curThread->setState(STATE_DONE);
	    else {
		statusGetUI->setExpectingFlag(true);
		statusGetUI->setService(service);
		curThread->setState(STATE_BUSY);
	    }
	}
	break;

    case STATE_CANCEL:
	statusGetUI->setCurNode(NULL);
	statusGetUI->setStatus("Canceling search...");
	curThread->setState(STATE_DONE);
	statusGetUI->setExpectingFlag(false);
	statusGetUI->setCancelFlag(true);
	break;

    default:
	break;
    }

    if (0 == done)
	Fl::add_timeout(.25, threadMonitor, data);
}

searchStatus *
startSearchStatus()
{

    statusGetUI = ((NxFind *) (NxApp::Instance))->getStatusWindow();

    getThread = new searchStatus();

    Fl::add_timeout(.25, threadMonitor, (void *) getThread);

    atexit(leave_program);
    return (getThread);
}
#endif

void
searchTimeout(void *data)
{

    searchStatus *ui = (searchStatus *) data;

    if (ui->_UI->getCancelFlag() == true)
	ui->searchCancel();

    if (ui->startSearch() == 0)
	ui->searchDone();
}

void
searchStatus::searchCancel(void)
{

    setState(STATE_DONE);

    _UI->setExpectingFlag(false);
    _UI->setCancelFlag(false);

    _UI->hide();
}

void
searchStatus::searchDone(void)
{

    setState(STATE_DONE);

    _UI->setExpectingFlag(false);
    _UI->setCancelFlag(false);

    _UI->hide();
}

void
searchStatus::ackSearch(void)
{

    if (getState() != STATE_SEARCH)
	return;

    Fl::remove_timeout(searchTimeout, (void *) this);

    if (_UI->getCancelFlag() == true)
	searchCancel();

    if (startSearch() == 0)
	searchDone();
}


int
searchStatus::startSearch(void)
{

    char *app = 0;
    char *service = 0;

    service = _UI->getNextService();
    if (!service)
	return (0);
    app = _UI->getApp();
    if (!app)
	return (0);

    if (_UI->getCancelFlag() == true)
	return (0);

    _UI->setExpectingFlag(true);
    _UI->setService(service);

    Fl::add_timeout(5, searchTimeout, (void *) this);

    sprintf(statusStr, "Searching %s...", app);
    _UI->setStatus(statusStr);

    setState(STATE_SEARCH);
    _UI->search();

    return (1);
}

searchStatus::searchStatus(c_statusWindow * ui)
{
    _UI = ui;
    setState(STATE_READY);
}

searchStatus::~searchStatus(void)
{

    if (getState() == STATE_SEARCH)
	Fl::remove_timeout(searchTimeout);

}

searchStatus *
startSearchStatus()
{

    searchStatus *search;
    c_statusWindow *ui;

    ui = ((NxFind *) (NxApp::Instance))->getStatusWindow();
    ui->show();

    search = new searchStatus(ui);

    search->startSearch();

    return (search);
}
