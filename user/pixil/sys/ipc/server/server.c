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
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>


#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>

#include <pixil_config.h>
#include <ipc/colosseum.h>
#include "server.h"

#ifdef CONFIG_PAR
#include <par/par.h>
#endif

int cl_daemon = 0;
int cl_chatty = 0;

static int g_named_socket = 0;
static int g_ipc_id = 1;

#ifdef CONFIG_PAR
db_handle *parDB = 0;

db_handle *
cl_openDatabase(void)
{
    db_handle *local = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);
    return (local);
}
#endif

int
cl_GetNextId(void)
{
    return (g_ipc_id++);
}

void
handleError(void)
{

    int i;

    char ch;
    fd_set iset, oset, eset;
    int ret = cl_GetClientSockets(&iset, &oset, &eset);

    for (i = 0; i < ret + 1; i++) {
	if (FD_ISSET(i, &iset)) {
	    if (read(i, &ch, 0) == -1)
		DO_LOG(CL_LOG_ERROR, "Linux error [%s] on fd [%d]\n",
		       strerror(errno), i);
	}
    }
}

/* The critical loop */

int
local_DoServerLoop(void)
{

    fd_set iset, oset, eset;

    int i;

    while (1) {

	struct timeval slice, pre, post;
	int maxfd = g_named_socket;
	int ret = 0;
	int timed = 0;

	FD_ZERO(&iset);
	FD_ZERO(&oset);
	FD_ZERO(&eset);

	/* Always watch the named socket */
	FD_SET(g_named_socket, &iset);

	ret = cl_GetClientSockets(&iset, &oset, &eset);
	if (ret && ret > maxfd)
	    maxfd = ret;

	/* Set the size of the timeslice */

	if (cl_havePending()) {
	    timed = 1;
	    slice.tv_sec = 1;
	    slice.tv_usec = 0;

	    gettimeofday(&pre, 0);
	    ret = select(maxfd + 1, &iset, &oset, &eset, &slice);
	    gettimeofday(&post, 0);
	} else {
	    ret = select(maxfd + 1, &iset, &oset, &eset, 0);
	}

	if (ret == -1) {
	    /* An EAGAIN or EINTR just forces an continue */
	    if (errno == EAGAIN || errno == EINTR)
		continue;

	    perror("select");
	    handleError();
	    return (-1);
	}

	if (ret) {
	    /* First, check the server socket */

	    if (FD_ISSET(g_named_socket, &iset)) {
		cl_HandleNewClient(g_named_socket);
	    }

	    /* Now process all of the client sockets */

	    for (i = 0; i <= maxfd; i++) {

		if (i == g_named_socket)
		    continue;

		/* Exception */
		if (FD_ISSET(i, &eset)) {
		    cl_CloseClient(i);
		    continue;	/* Nothing more to see here */
		}

		/* Read */
		if (FD_ISSET(i, &iset)) {
		    if (cl_HandleClientReq(get_app_by_socket(i)))
			continue;
		}

		/* Write */
		if (FD_ISSET(i, &oset)) {
		    cl_HandleQueuedMessages(get_app_by_socket(i));
		}
	    }
#ifdef NOTUSED

	    /* ** Exception handling ** */
	    /* An exception on any socket is grounds for explusion */

	    for (i = 0; i < maxfd; i++)
		if (i != g_named_socket && FD_ISSET(i, &eset)) {
		    unregister_client(get_app_by_socket(i));

		    /* Make sure we don't try to read or write it */

		    FD_CLR(i, &iset);
		    FD_CLR(i, &oset);
		}

	    /* ** Read handling ** */

	    for (i = 0; i <= maxfd; i++)
		if (i != g_named_socket && FD_ISSET(i, &iset))
		    handle_client_request(get_app_by_socket(i));

	    /* ** Write handling ** */

	    for (i = 0; i <= maxfd; i++)
		if (FD_ISSET(i, &oset))
		    cl_HandleQueuedMessages(get_app_by_socket(i));
#endif

	}

	if (timed && cl_havePending()) {
	    int lapsed = ((post.tv_sec - pre.tv_sec) * 1000000)
		+ (post.tv_usec - pre.tv_usec);

	    cl_UpdatePending(lapsed);
	}
    }
}

void
handle_server_error(int signal)
{

    if (signal < 0) {
	DO_LOG(CL_LOG_ERROR, "Got internal error %d. Exiting.", signal);
    } else {
	DO_LOG(CL_LOG_MESSAGE, "Got signal %d. Exiting.", signal);
    }

    /* Try to gracefully leave */
    cl_FreeAllApps();

#ifdef HAVE_LOGGING
    cl_closeLog();
#endif

#ifdef CONFIG_PAR
    if (parDB)
	db_closeDB(parDB);
#endif

    if (cl_daemon)
	closelog();

    close(g_named_socket);

    /* 02/26/02 - Get rid of the named socket */
    /* No reason to keep it around, right?    */

    unlink(CL_NAMED_SOCKET);
    exit(0);
}

int
main(int argc, char **argv)
{

#ifdef HAVE_LOGGING
    int loglevel = -1;
    int temp = 0;
#endif

    extern void cl_childHandler(int);

    /* Set up the signals */

    signal(SIGINT, handle_server_error);
    signal(SIGQUIT, handle_server_error);
    signal(SIGKILL, handle_server_error);
    signal(SIGTERM, handle_server_error);


    /* Handle the child */

    signal(SIGCHLD, cl_childHandler);
    signal(SIGPIPE, SIG_IGN);

    while (1) {
	signed char ch;
	extern int optind, opterr;
	extern char *optarg;

	ch = getopt(argc, argv, "vhlcd::");

	if (ch == -1)
	    break;

	switch (ch) {
	case 'v':
	    printf("Colosseum server version %s\n", CL_VERSION);
	    break;
	case 'h':
	    printf("Usage:./clserver -[vh]\n");
	    printf("[-l #] start logging with optional threshold\n");
	    printf("[-h] Display this help\n");
	    printf("[-v] Print the version\n");
	    printf("[-d] Daemon mode - Run as a daemon\n");
	    printf
		("[-c] Chatty mode - Show clients as they join and leave\n");
	    break;

	case 'l':
#ifdef HAVE_LOGGING
	    /* Turn logging on */

	    loglevel = 2;

	    if (optarg)
		temp = atoi(optarg);

	    if (temp != 0)
		loglevel = temp;

	    printf("Starting system logging at level %d.\n", loglevel);

#else
	    fprintf(stderr,
		    "System logging is not compiled in.  Ignoring!\n");
#endif

	    break;

	case 'd':
	    cl_daemon = 1;

	    /* Fork us away - This will esentially make a connectionless client */
	    if (vfork())
		return (0);

	    /* Redirect the terminal */
	    close(0);
	    open("/dev/null", O_RDWR);
	    dup2(0, 1);
	    dup2(0, 2);

	    openlog("clserver", LOG_CONS, LOG_DAEMON);
	    break;

	case 'c':
	    cl_chatty = 1;
	    break;
	}
    }

    /* Start the server */

    if ((g_named_socket = cl_InitServerSocket()) == -1) {
	fprintf(stderr, "Error! Unable to bind to the socket\n");
	handle_server_error(-1);
    }
#ifdef HAVE_LOGGING

    if (loglevel != -1) {

#ifdef CONFIG_PAR
	parDB = cl_openDatabase();
#endif

	cl_initLog(loglevel);
    }
#endif

    /* Finally, start the process loop. */
    if (local_DoServerLoop()) {
	fprintf(stderr, "local_DoServerLoop() returned unexpectedly\n");
	handle_server_error(-1);
    }

    cl_FreeAllApps();

#ifdef HAVE_LOGGING
    cl_closeLog();
#endif

#ifdef CONFIG_PAR
    if (parDB)
	db_closeDB(parDB);
#endif

    if (cl_daemon)
	closelog();

    close(g_named_socket);
    unlink(CL_NAMED_SOCKET);
    exit(0);
}
