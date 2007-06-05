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


#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include "nxschedule.h"

#ifdef		SYMBOL_MIPS
#include <signal.h>

void
h_sigusr(int signo)
{
    Fl_Window *win = ((NxSchedule *) NxApp::Instance())->get_shown_window();
    GR_WINDOW_ID wid;
    GR_WINDOW_INFO info;

    // Get the parent window
    while (win->parent() != NULL)
	win = (Fl_Window *) win->parent();

    // Raise this window's parent
    wid = fl_xid(win);
    GrGetWindowInfo(wid, &info);
    GrMapWindow(info.parent);
    GrRaiseWindow(info.parent);
}				// end of h_sigusr()
#endif //      SYMBOL_MIPS

int exit_flag = 0;

int
main(int argc, char *argv[])
{

    NxSchedule sc(argc, argv);
#ifdef		SYMBOL_MIPS
    signal(SIGUSR1, h_sigusr);
#endif //      SYMBOL_MIPS

    while (!exit_flag) {

	if (Fl::wait() == 0) {

	    if (!sc.noguisearch_flag) {
		break;
	    }

	}

    }

    return 0;

}
