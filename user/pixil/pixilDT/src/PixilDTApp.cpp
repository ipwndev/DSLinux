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

//--------------------------------------------------------------//
// Main routine for PixilDT prototype.                          //
//--------------------------------------------------------------//

#include <pixil_config.h>

#include "config.h"
#include <cstdlib>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>

#include "PixilDT.h"
#include "VCMemoryLeak.h"
#include "Sync.h"

#ifdef DEBUG
#ifdef WIN32
// For at exit memory leak detection
void CheckMemoryLeaks();
_CrtMemState MemState1;
#endif
#endif


int
main(int argc, char **argv)
{
    // Set up internationalization
#ifdef CONFIG_PIXILDT_INTERNATIONAL
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

#ifdef DEBUG
#ifdef WIN32
    // Visual C memory leak detection
    atexit(CheckMemoryLeaks);
    _CrtMemCheckpoint(&MemState1);
#endif
#endif

#ifdef CONFIG_SYNC
    atexit(CloseSync);
#endif

    // Save the command line arguments for the NxDbAccess class
    NxDbAccess::SaveCmdLine(argc, argv);

    // Start the real stuff
    Fl::visual(FL_RGB);
    PixilDT App(argc, argv);

#ifdef CONFIG_SYNC
    if (InitSync() == -1)
	printf("Error - Unable to start up the sync app\n");
#endif

    Fl::run();

    return (EXIT_SUCCESS);
}


#ifdef DEBUG
#ifdef WIN32
extern "C"
{
    extern struct buffer *startbuf;
}
extern
//--------------------------------------------------------------//
// Visual C memory leak detection                               //
//--------------------------------------------------------------//
    void
CheckMemoryLeaks()
{
    // Clean up after code in blkio.c
    free(startbuf);

    // Clean up after code in fl_font.cxx
    //free(fl_fontsize);

    // Test for any memory leaks
    _CrtMemDumpAllObjectsSince(&MemState1);
}
#endif
#endif
