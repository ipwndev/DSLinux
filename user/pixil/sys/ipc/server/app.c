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
#include <string.h>

#include <sys/wait.h>

#include <pixil_config.h>

extern int cl_chatty;

#ifdef CONFIG_PAR

#include <par/par.h>

extern db_handle *parDB;
extern db_handle *cl_openDatabase(void);

static int local_spawnApp(char *name, char *args);

#endif

#include <ipc/colosseum.h>
#include "server.h"

/* Pulled directly out of ol' nxscrtop */

#ifdef CONFIG_PAR

static int
local_parseArgs(char *name, char *argstr, char ***argv)
{
    char **argvptr = 0;

    char *h = argstr;
    char *t = 0;

    int argc = 1;
    int index = 0;

    /* If there are arguments, then count them */

    if (argstr) {
	while ((t = strchr(h, ' '))) {
	    /* Skip to the next empty space */
	    for (; *t == ' ' && *t != 0; t++);

	    if (*t == 0)
		break;

	    argc++;
	    h = t;
	}
    }

    /* Now, allocate the required room */
    *argv = (char **) malloc((argc + 2) * sizeof(char *));

    if (!*argv)
	return (0);

    argvptr = *argv;

    /* First step, add the program name */
    argvptr[0] = (char *) calloc(1, strlen(name) + 1);
    strcpy(argvptr[0], name);

    if (!argstr || !strlen(argstr)) {
	argvptr[1] = (char *) 0;
	return (1);
    }

    /* Now add any args (if they exist) */

    h = argstr;

    while (index < argc) {
	int slen = 0;

	t = strchr(h, ' ');

	if (t)
	    slen = (int) (t - h);
	else
	    slen = strlen(argstr) - (int) (h - argstr);

	argvptr[++index] = (char *) calloc(1, slen + 1);

	/* If there was an error, skip to the next one */

	if (!argvptr[index]) {
	    index--;

	    if (!t)
		break;

	    /* Skip to the next word */
	    for (; *t == ' ' && *t != 0; t++);

	    if (*t == 0)
		break;

	    h = t;
	    continue;
	}

	if (t)
	    strncpy(argvptr[index], h, slen);
	else
	    strcpy(argvptr[index], h);

	if (!t)
	    break;

	/* Skip to the next word */
	for (; *t == ' ' && *t != 0; t++);

	if (*t == 0)
	    break;

	h = t;
	continue;
    }

    /* Add a null pointer to the end */
    argvptr[++index] = (char *) 0;
    return (index);
}
#endif

/* This child handler grabs a closed pid and removes it from the list */

void
cl_childHandler(int id)
{

    cl_pid_t *pid;

    int status;
    int ret = waitpid(-1, &status, WNOHANG);

    pid = cl_findPid(ret);
    if (!pid)
	return;			/* Unknown pid..  no harm no foul, I guess */

    cl_removePid(ret);
}

#ifdef CONFIG_PAR

static int
local_ForkApp(char *exec, char *astr, char *workdir)
{

    int pid = 0;
    char **argv = 0;

    local_parseArgs(exec, astr, &argv);

    if (!(pid = vfork())) {
	char *path = 0;
	/* If a work dir was specified, use that one */

	if (strlen(workdir))
	    path = workdir;
	else {
	    char *p = strrchr(exec, '/');

	    /* otherwise, try to determine a path from the exec string */

	    if (p) {
		path = alloca((int) (p - exec) + 2);
		strncpy(path, exec, (int) (p - exec));
		path[(int) (p - exec)] = 0;
	    }
	}

	if (path)
	    if (chdir(path))
		DO_LOG(CL_LOG_ERROR,
		       "Unable to switch to the working directory %s\n",
		       path);

	/* Start the desired app */
	execvp(exec, argv);
	exit(0);
    }

    return (pid);
}

#endif

cl_pending_struct *
cl_AddPending(cl_app_struct * parent,
	      unsigned char *name, int flags, int timeout, cl_pkt_start * req)
{

    cl_pending_struct *pend = cl_AllocPending();

    if (!pend)
	return (0);

    strcpy(pend->name, name);
    pend->parent = parent;
    pend->flags = flags;
    pend->timeout = timeout;
    memcpy(&pend->request, req, sizeof(cl_pkt_start));

    /* Add it to the pending list */

    return (pend);
}

/* This spawns an given application from the PAR database    */
/* For safety, we can only spawn apps that come from the PAR */

#ifdef CONFIG_PAR

static int
local_spawnApp(char *name, char *args)
{

    par_app_t parapp;
    int pid;

    if (!parDB)
	parDB = cl_openDatabase();

    if (!parDB) {
	DO_LOG(CL_LOG_ERROR, "Unable to load the PAR database\n");
	return (-1);
    }

    memset(&parapp, 0, sizeof(parapp));

    if (par_getApplication(parDB, name, &parapp) == -1)
	return (-1);

    if (!args)
	pid = local_ForkApp(parapp.path, parapp.defargs, parapp.workdir);
    else
	pid = local_ForkApp(parapp.path, args, parapp.workdir);

    /* Error - bail out! */
    if (pid == -1)
	return (-1);

    /* Add the pid to the pid list */
    cl_addPid(name, pid);

    /* Return the pid for fun */
    return (pid);
}

#endif /* CONFIG_PAR */

int
cl_HandleAppInfo(cl_app_struct * req, cl_pkt_appinfo * pkt)
{

    cl_pid_t *pid;

    switch (pkt->flags) {

    case CL_APP_INFO_PID:
	pid = cl_findNameByPid(pkt->processid);
	if (!pid)
	    goto send_error;

	strncpy(pkt->name, pid->name, sizeof(pkt->name));
	break;

    case CL_APP_INFO_NAME:
	pid = cl_findPidByName(pkt->name);
	if (!pid)
	    goto send_error;

	pkt->processid = pid->pid;
	break;

    default:
	DO_LOG(CL_LOG_MESSAGE, "Warning - Unknown flag [%x]\n", pkt->flags);
	break;
    }

    if (cl_chatty)
	DO_LOG(CL_LOG_MESSAGE, "APPINFO [%s]\n", req->cl_name, pkt->name);

    cl_SendPacket(req, (cl_packet *) pkt, sizeof(cl_pkt_appinfo));
    return (0);

  send_error:
    CL_ERROR(req, CL_E_NOSUCHAPP, pkt);
    return (-1);
}

/* This will return the PID of the spawned app */

int
cl_HandleSpawnApp(cl_app_struct * req, cl_pkt_spawn * pkt)
{

#ifdef CONFIG_PAR
    int res;
#endif

    /* See if the app is already started */
    cl_pid_t *pid = cl_findPidByName(pkt->name);

    if (pid) {
	pkt->pid = pid->pid;
	cl_SendPacket(req, (cl_packet *) pkt, sizeof(cl_pkt_spawn));
	return (0);
    }

    /* A quick check for par support */
#ifndef CONFIG_PAR
    DO_LOG(CL_LOG_MESSAGE, "SpawnApp requires the PAR database.\n");
    CL_ERROR(req, CL_E_NOTIMPLEMENT, pkt);
    return (-1);
#else

    if (cl_chatty)
	DO_LOG(CL_LOG_MESSAGE, "SPAWN [%s]: Application [%s]\n", req->cl_name,
	       pkt->name);

    if (strlen(pkt->argstr))
	res = local_spawnApp(pkt->name, pkt->argstr);
    else
	res = local_spawnApp(pkt->name, 0);

    if (res > 0) {
	pkt->pid = res;
	cl_SendPacket(req, (cl_packet *) pkt, sizeof(cl_pkt_spawn));
	return (0);
    } else {
	CL_ERROR(req, CL_E_SPAWNERR, pkt);
	return (-1);
    }

#endif /* CONFIG_PAR */
}

int
cl_HandleStartApp(cl_app_struct * req, cl_pkt_start * pkt)
{

    cl_app_struct *app;

#ifdef CONFIG_PAR
    int result;
    cl_pending_struct *pending = 0;
#endif

    /* First, look for the app locally to see if it is available */

    app = get_app_by_name(pkt->name);

    if (app) {
	pkt->ipc_id = app->cl_id;
	cl_SendPacket(req, (cl_packet *) pkt, sizeof(cl_pkt_start));
	return (0);
    }

    /* Next, check to see if this app name is in the PID list. */
    /* if it is, then send an error back, because it should    */
    /* have been flagged above.                                */

    if (cl_findPidByName(pkt->name)) {
	DO_LOG(CL_LOG_MESSAGE,
	       "Application [%s] exists, but Colosseum is not available.\n",
	       pkt->name);
	CL_ERROR(req, CL_E_SPAWNERR, pkt);
	return (-1);
    }
#ifndef CONFIG_PAR
    DO_LOG(CL_LOG_ERROR, "PAR support is required for StartApp.\n");
    CL_ERROR(req, CL_E_NOTIMPLEMENT, pkt);
    return (-1);
#else

    /* Add this to the pending list (Doing this here first avoids any timing issues with the fork) */

    if (pkt->timeout >= 0) {
	int to = 0;
	if (pkt->timeout == 0)
	    to = -1;
	else
	    to = pkt->timeout * 1000000;

	pending = cl_AddPending(req, pkt->name, pkt->start_flags, to, pkt);

	if (!pending) {
	    CL_ERROR(req, CL_E_APPERR, pkt);
	    return (-1);
	}

	SET_FLAG(req->cl_flags, CL_F_WAITING);
    }

    if (cl_chatty)
	DO_LOG(CL_LOG_MESSAGE, "START [%s]: Application [%s]\n", req->cl_name,
	       pkt->name);

    if (strlen(pkt->argstr))
	result = local_spawnApp(pkt->name, pkt->argstr);
    else
	result = local_spawnApp(pkt->name, 0);

    if (result == -1) {
	if (pending)
	    cl_FreePending(pending);
	CL_ERROR(req, CL_E_APPERR, pkt);
	return (-1);
    }

    return (0);
#endif /* CONFIG_PAR */
}



int
cl_HandleFindApp(cl_app_struct * req, cl_pkt_findapp * pkt)
{

    cl_app_struct *app = get_app_by_name(pkt->name);

    if (cl_chatty)
	DO_LOG(CL_LOG_MESSAGE, "FIND [%s]: Application [%s]\n", req->cl_name,
	       pkt->name);

    if (app) {
	pkt->ipc_id = app->cl_id;
	cl_SendPacket(req, (cl_packet *) pkt, sizeof(cl_pkt_findapp));
	return (0);
    }

    CL_ERROR(req, CL_E_NOAPP, pkt);
    return (-1);
}

int
cl_HandleRegisterApp(cl_app_struct * req, cl_pkt_reg * pkt)
{

    unsigned short flags = 0;

    cl_pending_struct *pend;
    cl_app_struct *old;

    if ((old = get_app_by_name(pkt->name))) {

	/* Ok, so thats a little strange.  Either we */
	/* have a stupid program that re-registered  */
	/* or the app crashed and we didn't know about it */

	/* Send the error packet to the old socket. */
	/* if it is successful, then we have a supid program */
	/* otherwise, the socket is invalid, and we'll kill  */
	/* it automagically */

	if (CL_ERROR(old, CL_E_APPEXISTS, pkt) == 0) {
	    CL_ERROR(req, CL_E_APPEXISTS, pkt);
	    return (-1);
	}

	/* The old client was removed, long live the new client */
    }

    /* Register the client and store the values */

    cl_RegisterApp(req, pkt);

    if (pkt->start_flags == CL_NORMAL_CLIENT)
	SET_STATE(req->cl_flags, CL_F_ACTIVE);
    else
	SET_STATE(req->cl_flags, CL_F_RESTRICTED);

    /* Now, check the pending list and see if we have been */
    /* waiting for this app */

    printf("CLSERVER:  Registered [%s] as [%d]\n", pkt->name, req->cl_id);

    pend = cl_SearchPending(pkt->name);

    if (pend) {

	pend->request.ipc_id = req->cl_id;

	cl_SendPacket(pend->parent, (cl_packet *) & pend->request,
		      sizeof(cl_pkt_start));

	flags = pend->flags;
	cl_FreePending(pend);
    }

    pkt->ipc_id = req->cl_id;
    pkt->start_flags = flags;

    if (cl_chatty)
	DO_LOG(CL_LOG_MESSAGE, "OPEN [%s]: IPC id [%d]\n", req->cl_name,
	       req->cl_id);

    cl_SendPacket(req, (cl_packet *) pkt, sizeof(cl_pkt_reg));

    return (0);
}

void
cl_CloseApp(cl_app_struct * app)
{

    if (cl_chatty)
	DO_LOG(CL_LOG_MESSAGE, "CLOSE [%s]: IPC id [%d]\n", app->cl_name,
	       app->cl_id);

    if (app)
	cl_FreeAppStruct(app);

}
