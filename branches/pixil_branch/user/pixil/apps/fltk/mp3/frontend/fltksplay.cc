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
#include <getopt.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "fspl_panel.h"
#include "xplay.h"

#define OPTIONS "p:fw:h:b:t:s:H"
int w_opt = 0;
int h_opt = 0;
char *args = NULL;

extern pthread_mutex_t startuplock;

int exit_flag = 0;

int
main(int argc, char **argv)
{

    pthread_t th;
    pthread_create(&th, NULL, _startup, NULL);

    fspl_panel MainPanel(APP_NAME);

    signed char c;

    while ((c = getopt(argc, argv, OPTIONS)) != -1) {

	switch (c) {
	case 'p':
	    args = optarg;
	    struct stat dir;
	    if (stat(args, &dir) == 0) {
		;
		//MainPanel->SetDefaultMusicPath(args);
	    }
	    break;
	case 'f':
	    if (args) {
		int i = mkdir(args, 0700);
		if (i == 0);
		//MainPanel->SetDefaultMusicPath(args);
	    }
	    break;
	case 'w':
	    w_opt = atoi(optarg);
	    break;
	case 'h':
	    h_opt = atoi(optarg);
	    break;
	case 'b':
	    break;
	case 't':
	    //MainPanel.SetDefaultFFTTimer(atof(optarg));
	    break;
	case 's':
	    //MainPanel->SetDefaultFFTSkip(atoi(optarg));
	    break;
	case 'H':
	    cerr <<
		"\n*************************************************************\n";
	    cerr <<
		"* NAME                                                      *\n";
	    cerr <<
		"*\tfltksplay - Microwindows/FLNX MP3 player.           *\n";
	    cerr << "*\t\t                                            *\n";
	    cerr <<
		"* SYNOPSIS                                                  *\n";
	    cerr <<
		"*\tfltksplay                                           *\n";

	    cerr << "*\t\t -p = default music path                    *\n";

	    cerr << "*\t\t -f = force mkdir of default music path     *\n";
	    cerr << "*\t\t -w = background image width                *\n";
	    cerr << "*\t\t -h = background image height               *\n";
	    cerr << "*\t\t -b = background image                      *\n";
	    cerr << "*\t\t -t = fft timer in seconds                  *\n";
	    cerr << "*\t\t -s = fft number of frames to skip          *\n";
	    cerr << "*\t\t -H = help                                  *\n";
	    cerr <<
		"*************************************************************\n\n";
	    exit(1);
	    break;
	}			// switch

    }				// while

    // Set width and Height.
    usleep(500);
    pthread_mutex_unlock(&startuplock);


    return Fl::run();

}
