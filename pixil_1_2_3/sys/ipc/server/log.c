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



#include <syslog.h>
#include <string.h>

#include <ipc/colosseum.h>
#include "server.h"

extern int cl_daemon;

#ifdef HAVE_LOGGING

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef CONFIG_PAR
#include <par/par.h>

extern db_handle *parDB;
#endif

static int cl_logLevel = 0;

static FILE *cl_logArray[CL_LEVEL_COUNT] = { 0, 0, 0, 0 };

/* Given a level and a filename, open a stream to that log */

/* If multiple files try to open the same filename, this may */
/* be inefficient, but I believe it still works */

static char *cl_defLogFiles[] = { "/var/log/pixil_critical.log",
    "/var/log/pixil_error.log",
    "/var/log/pixil_message.log",
    "/var/log/pixil_debug.log"
};

#ifdef CONFIG_PAR
static char *cl_levelNames[] = { "critical", "error", "message", "debug" };
#endif

static int
local_openLogFile(int level)
{

    char filename[512];

#ifdef CONFIG_PAR
    if (!parDB)
	strcpy(filename, cl_defLogFiles[level]);
    else {
	if (par_getGlobalPref(parDB, "syslog", cl_levelNames[level],
			      PAR_TEXT, filename, sizeof(filename)) == -1)
	    strcpy(filename, cl_defLogFiles[level]);
    }
#else
    strcpy(filename, cl_defLogFiles[level]);
#endif

    if (level >= CL_LEVEL_COUNT)
	return (-1);

    if (cl_logArray[level]) {
	fprintf(stderr, "Warning - Log file %s already opened\n", filename);
	return (-1);
    }

    cl_logArray[level] = fopen(filename, "a");

    if (cl_logArray[level] == 0)
	fprintf(stderr, "Unable to open the log file!\n");

    return ((cl_logArray[level] == 0) ? -1 : 0);
}

static int
local_closeLogFile(int level)
{

    if (cl_logArray[level])
	fclose(cl_logArray[level]);

    cl_logArray[level] = 0;
    return (0);
}

int
cl_initLog(int level)
{

    int i;

    /* Establish the new level */
    cl_logLevel = level;

    for (i = 0; i < CL_LEVEL_COUNT; i++) {
	if (i > cl_logLevel)
	    break;
	local_openLogFile(i);
    }

    return (0);
}

int
cl_changeLevel(int new)
{

    int i;

    if (new == cl_logLevel)
	return (0);

    /* If the new level is lower than the previous level */
    /* close the files that are no longer needed */

    /* Otherwise, open up any new files that are needed */

    if (new < cl_logLevel) {
	for (i = new; i <= cl_logLevel; i++)
	    local_closeLogFile(i);
    } else {
	for (i = cl_logLevel; i <= new; i++)
	    local_openLogFile(i);
    }

    cl_logLevel = new;
    return (0);
}

/* Format:  MM/DD HH:MM LEV <message> */

int
cl_doLog(int level, char *app, char *message, ...)
{

    va_list ap;
    time_t t;

    struct tm *tv;

    if (level >= CL_LEVEL_COUNT)
	return (-1);
    if (!cl_logArray[level])
	return (-1);

    /* Thresholding */

    if (level >= cl_logLevel)
	return (0);

    /* First, print out the time/date */

    t = time(0);
    tv = localtime(&t);

    fprintf(cl_logArray[level], "%2.2d/%2.2d %2.2d:%2.2d ",
	    tv->tm_mon, tv->tm_mday, tv->tm_hour, tv->tm_min);

    /* Next, print out the log level of this message */

    switch (level) {
    case CL_LOG_CRITICAL:
	fprintf(cl_logArray[level], "CRI ");
	break;

    case CL_LOG_ERROR:
	fprintf(cl_logArray[level], "ERR ");
	break;

    case CL_LOG_MESSAGE:
	fprintf(cl_logArray[level], "MSG ");
	break;

    case CL_LOG_DEBUG:
	fprintf(cl_logArray[level], "DBG ");
	break;
    }

    /* Next, print the application name */
    fprintf(cl_logArray[level], "%5s: ", app);

    /* lastly, print the message */

    va_start(ap, message);
    vfprintf(cl_logArray[level], message, ap);
    va_end(ap);

    /* Add a carrige return at the end */
    fprintf(cl_logArray[level], "\n");

    /* If we got an error at all, then close the log */

    if (ferror(cl_logArray[level])) {
	fclose(cl_logArray[level]);
	cl_logArray[level] = 0;
    }

    return (0);
}


void
cl_closeLog(void)
{

    int i;

    for (i = 0; i < CL_LEVEL_COUNT; i++)
	local_closeLogFile(i);

}

#else /* HAVE_LOGGING */

#include <stdarg.h>
#include <stdio.h>

#ifdef DEBUG
static int cl_logLevel = CL_LOG_DEBUG;
#else
static int cl_logLevel = CL_LOG_MESSAGE;
#endif

int
cl_initLog(void)
{
    return (0);
}

void
cl_closeLog(void)
{
}

int
cl_doLog(int level, char *app, char *message, ...)
{

    va_list ap;

    if (level > cl_logLevel)
	return (0);

    /* If we are in daemon mode, then we have no controlling deamon */
    /* Redirect our stuff to syslog instead */

    /* ARGH! */

    va_start(ap, message);

    if (cl_daemon) {
	char str[255];

	vsnprintf(str, 255, message, ap);

	switch (level) {
	case CL_LOG_CRITICAL:
	case CL_LOG_ERROR:
	    syslog(LOG_ERR, str);
	    break;
	case CL_LOG_MESSAGE:
	    syslog(LOG_INFO, str);
	    break;
	case CL_LOG_DEBUG:
	    syslog(LOG_DEBUG, str);
	    break;
	}
    } else {
	printf("%s:", app);
	vprintf(message, ap);
	if (message[strlen(message) - 1] != '\n')
	    printf("\n");
    }

    va_end(ap);

    return (0);
}

#endif /* HAVE_LOGGING */
