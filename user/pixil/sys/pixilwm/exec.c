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
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "apps.h"
#include "nanowm.h"

#define ASSOC_FILE "config.magic"

NXLISTHEAD active_list;

static PROCESS *
find_process(int pid)
{
    PNXLIST p = active_list.head;

    for (; p; p = p->next) {
	PROCESS *process = nxItemAddr(p, PROCESS, next);

	if (process->pid == pid)
	    return (process);
    }

    return NULL;
}

void
kill_running_processes()
{
    PNXLIST p = active_list.head;

    while (p) {
	PNXLIST n = p->next;
	PROCESS *process = nxItemAddr(p, PROCESS, next);

	warning("Terminating process %d\n", process->pid);

	kill(process->pid, SIGTERM);
	p = n;
    }
}

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

static void
local_freeArgs(int argc, char **argv)
{

    int i;

    for (i = 0; i < argc; i++)
	free(argv[i]);

    free(argv);
}

void
application_handler(int id)
{
    int status;
    int ret;

    while (1) {
	ret = waitpid(-1, &status, WNOHANG);

	if (ret == -1 || ret == 0)
	    break;
	else {
	    PROCESS *proc = find_process(ret);

	    warning("Process %d died with status %d\n", ret, status);

	    if (!proc)
		return;

	    if (proc->app) {
		proc->app->m_process_id = 0;
		proc->app->m_container_wid = 0;

		if (proc->app->m_flags & FL_INPUT)
		    killInput(proc->app);
	    }

	    /* Remove it from the active list */
	    nxListRemove(&active_list, &proc->next);
	}
    }
}

static int
local_spawn(char *exec, char *argstr, char *workdir)
{

    char **argv = 0;
    int pid;

    int argc = local_parseArgs(exec, argstr, &argv);
    if (!argc)
	return (-1);

    if (!(pid = vfork())) {
	if (strlen(workdir))
	    if (chdir(workdir))
		error("Unable to switch to working directory '%s'.\n",
		      workdir);

	execvp(exec, argv);
	exit(0);
    }

    local_freeArgs(argc, argv);
    if (pid == -1)
	error("Unable to fork app: %s\n", strerror(errno));

    return (pid);
}

static int
show_window(APP * app)
{
    g_last_app = app;

    container_activate(app->m_container_wid);
    if (isInputVisible() == 0)
	inputResetWorkspace();
    return (0);
}

int
launch_application(APP * app)
{
    int pid;

    /* Check to see if we have a container wid */

    if (app->m_container_wid)
	return (show_window(app));

    /* Don't worry about the process list if we are using ipcActive() */
    if (ipcActive()) {
	pid = ipcSpawnApp(app->m_name, 0);

	if (pid < 0) {
	    error("Problem while spawning '%s' (error code = %d).\n",
		  app->m_name, pid);

	    return (-1);
	}

	app->m_process_id = pid;
    } else {
	PROCESS *proc = nxItemNew(PROCESS);

	pid = local_spawn(app->m_exec, app->m_args, app->m_workdir);

	if (pid < 0) {
	    error("Problem while spawning '%s'.\n", app->m_name, pid);

	    free(proc);
	    return (-1);
	}

	proc->pid = pid;
	proc->app = app;
	app->m_process_id = pid;

	nxListAdd(&active_list, &proc->next);
    }

    return (0);
}

inline void
hide_application(APP * app)
{
    container_hide(app->m_container_wid);
}

void
show_application(APP * app)
{
    if (!app || !app->m_container_wid)
	return;
    container_activate(app->m_container_wid);
}

inline int
is_app_running(APP * app)
{
    return (app->m_process_id || app->m_container_wid);
}

/* File association stuff that may be used in the future? */

#ifdef NOTUSED

#define FILE_ASSOC_COUNT 3

typedef struct
{
    int active;
    int age;
    char application[512];
    char args[256];
}
file_assoc_cache_t;

file_assoc_cache_t file_assoc_cache[FILE_ASSOC_COUNT];

static int
get_new_cache_entry()
{
    int i = 0;
    int use_slot = -1;
    int oldest_slot = -1;
    int oldest_age = 0;

    /* Determine a cache location for this entry. */
    /* Look for an empty entry and age the other entries at the same time */

    for (i = 0; i < FILE_ASSOC_COUNT; i++) {
	if (!file_assoc_cache[i].active) {
	    if (use_slot == -1)
		use_slot = i;
	} else {
	    if (++file_assoc_cache[i].age > oldest_age) {
		oldest_age = file_assoc_cache[i].age;
		oldest_slot = i;
	    }
	}
    }

    if (use_slot == -1)
	use_slot = oldest_slot;

    return (use_slot);
}


/* get_file_association()
   Search the associations file to see if 
   an application has been specified for this file 
*/

static void
assoc_callback(char *key, char *value, int data)
{
    if (!strcmp(key, "app")) {
	strcpy(file_assoc_cache[data].application, value);
	return;
    }

    if (!strcmp(key, "args")) {
	strcpy(file_assoc_cache[data].args, value);
	return;
    }
}

/* Stuff for the future? */

int
get_file_association(char *filename, char *type, app_info_t * ainfo)
{
    char buf[4096];
    int slot;

    char *afile;

    /* Search for the global configuration file and store the location */
    nxLoadConfigFile(ASSOC_FILE, NX_CONFIG_ASSOC);

    afile = nxGetConfigFile(NX_CONFIG_ASSOC);
    if (!afile) {
	printf("Unable to read the file %s\n");
	return (-1);
    }

    slot = get_new_cache_entry();

    bzero(&file_assoc_cache[slot], sizeof(file_assoc_cache_t));

    file_assoc_cache[slot].active = 1;

    if (IniGetString(type, NULL, "", buf, sizeof(buf), afile)) {
	IniEnumKeyValues(buf, assoc_callback, slot);

	/* Now, construct the app info */
	strcpy(ainfo->path, file_assoc_cache[slot].application);

	str_replace_variable("$file", filename, file_assoc_cache[slot].args,
			     ainfo->args, sizeof(ainfo->args));

	return (0);
    }

    return (-1);
}

#endif
