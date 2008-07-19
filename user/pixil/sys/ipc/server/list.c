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
#include <sys/types.h>

#include <ipc/colosseum.h>
#include "server.h"

/* The master list of application structures */
static cl_app_struct *cl_main_list = 0;

/* A table hashed against the socket number */
static cl_app_struct *cl_socket_hash[CL_HASH_SIZE];

/* A table hashed against the IPC id */
static cl_app_struct *cl_id_hash[CL_HASH_SIZE];

/* A table hashed against the checksum of the name */
static cl_app_struct *cl_name_hash[CL_HASH_SIZE];

/* The pool of currently allocated packet buffers */
static cl_pkt_buff *cl_pkt_pool = 0;

/* The list of currently allocated pids */
static cl_pid_t *pidList = 0;

/* A local function that "checksums" the name  */
/* for hashing purposes */

static int
encode_name(unsigned char *name)
{

    unsigned char *ptr = name;
    unsigned char ch = 0;

    while (*ptr)
	ch += *ptr++;
    return (ch);
}

/* Given a list head and a index, add the specified item to the list */

static void
add_to_list(cl_app_struct ** head, int lindex, cl_app_struct * item)
{

    if (!*head) {
	*head = item;
	item->listptr[lindex] = 0;
    } else {
	cl_app_struct *ptr = *head;

	while (ptr->listptr[lindex])
	    ptr = ptr->listptr[lindex];

	ptr->listptr[lindex] = item;
	item->listptr[lindex] = 0;
    }
}

static void
remove_from_list(cl_app_struct ** head, int lindex, cl_app_struct * item)
{

    cl_app_struct *ptr = *head;
    cl_app_struct *prev = 0;

    while (ptr) {
	if (ptr == item) {
	    if (!prev)
		*head = ptr->listptr[lindex];
	    else
		prev->listptr[lindex] = ptr->listptr[lindex];
	    return;
	}

	prev = ptr;
	ptr = ptr->listptr[lindex];
    }
}

/* These small functions add a new item to the specified list/hash */

static void
add_to_socket(cl_app_struct * item)
{
    int index = item->cl_socket % CL_HASH_SIZE;
    add_to_list(&cl_socket_hash[index], CL_LIST_SOCKET, item);
}

static void
add_to_id(cl_app_struct * item)
{
    int index = item->cl_id % CL_HASH_SIZE;
    add_to_list(&cl_id_hash[index], CL_LIST_ID, item);
}
static void
add_to_name(cl_app_struct * item)
{
    int index = encode_name(item->cl_name) % CL_HASH_SIZE;
    add_to_list(&cl_name_hash[index], CL_LIST_NAME, item);
}

/* Thse small functions remove a item from the specified hash/list */

static void
remove_from_socket(cl_app_struct * item)
{
    int index = item->cl_socket % CL_HASH_SIZE;
    remove_from_list(&cl_socket_hash[index], CL_LIST_SOCKET, item);
}

static void
remove_from_id(cl_app_struct * item)
{
    int index = item->cl_id % CL_HASH_SIZE;
    remove_from_list(&cl_id_hash[index], CL_LIST_ID, item);
}
static void
remove_from_name(cl_app_struct * item)
{
    int index = encode_name(item->cl_name) % CL_HASH_SIZE;
    remove_from_list(&cl_name_hash[index], CL_LIST_NAME, item);
}

/* Allocate a brand new client structure in all its glory */

static cl_app_struct *
local_AllocAppStruct(void)
{

    cl_app_struct *head = cl_main_list;
    cl_app_struct *ptr;

    /* Allocate it (zeroed out) */

    if (!head)
	ptr = cl_main_list =
	    (cl_app_struct *) calloc(sizeof(cl_app_struct), 1);
    else {
	while (head->listptr[CL_LIST_MAIN])
	    head = head->listptr[CL_LIST_MAIN];
	ptr = head->listptr[CL_LIST_MAIN] =
	    (cl_app_struct *) calloc(sizeof(cl_app_struct), 1);
    }

    ptr->cl_queue.head = 1;
    ptr->cl_queue.size = CL_QUEUE_SIZE;

    return (ptr);
}

void
cl_FreeAppStruct(cl_app_struct * app)
{

    cl_app_struct *ptr = cl_main_list;
    cl_app_struct *prev = 0;

    cl_pkt_buff *pkt;

    /* First, make sure it is removed from all of the lists */

    remove_from_socket(app);
    remove_from_name(app);
    remove_from_id(app);

    /* Make sure that the queue is freed */
    while ((pkt = queue_get_packet(app)))
	pkt_buff_free(pkt);

    /* Now remove it from the main list */

    while (ptr) {

	if (ptr == app) {
	    if (!prev)
		cl_main_list = ptr->listptr[CL_LIST_MAIN];
	    else
		prev->listptr[CL_LIST_MAIN] = ptr->listptr[CL_LIST_MAIN];

	    free(app);
	    return;
	}

	prev = ptr;
	ptr = ptr->listptr[CL_LIST_MAIN];
    }
}

/* This function fills a number of FD sets with the appropriate */
/* socket numbers, returning the highest FD found */

int
cl_GetClientSockets(fd_set * in, fd_set * out, fd_set * exp)
{

    int max = 0;

    cl_app_struct *head = cl_main_list;
    if (!cl_main_list)
	return (0);

    while (head) {
	FD_SET(head->cl_socket, in);	/* Listen on all incoming ports */
	FD_SET(head->cl_socket, exp);	/* Look for exceptions on all ports */

	if (head->cl_flags & CL_F_QUEUE_MSG)
	    FD_SET(head->cl_socket, out);

	if (head->cl_socket > max)
	    max = head->cl_socket;
	head = head->listptr[CL_LIST_MAIN];
    }

    return (max);
}

cl_app_struct *
cl_AllocNewApp(int sock)
{

    cl_app_struct *app = local_AllocAppStruct();
    if (!app)
	return (0);

    app->cl_socket = sock;
    SET_STATE(app->cl_flags, CL_F_NEW);

    /* Add it to the socket hash table (for easy search) */

    add_to_socket(app);
    return (app);
}

void
cl_RegisterApp(cl_app_struct * app, cl_pkt_reg * data)
{

    /* Store all the approprate data */

    strcpy(app->cl_name, data->name);
    app->cl_id = cl_GetNextId();

    /* Insert the new item into the two main hash tables */

    add_to_id(app);
    add_to_name(app);
}

void
cl_FreeAllApps(void)
{
    cl_app_struct *lhead = cl_main_list;
    cl_pkt_buff *phead = cl_pkt_pool;

    /* Unregister all of the apps */

    while (lhead) {
	cl_app_struct *next = lhead->listptr[CL_LIST_MAIN];
	cl_CloseClient(lhead->cl_socket);
	lhead = next;
    }

    /* Make sure any pending buffers are freed */
    while (phead) {
	cl_pkt_buff *next = phead->next;
	pkt_buff_free(phead);

	phead = next;
    }
}

cl_app_struct *
get_first_app(void)
{
    return (cl_main_list);
}

cl_app_struct *
get_next_app(cl_app_struct * prev)
{
    if (prev)
	return (prev->listptr[CL_LIST_MAIN]);
    else
	return (0);
}

cl_app_struct *
get_app_by_name(unsigned char *name)
{

    int index = encode_name(name) % CL_HASH_SIZE;
    cl_app_struct *head = cl_name_hash[index];

    while (head) {
	if (!strcmp(head->cl_name, name))
	    return (head);
	head = head->listptr[CL_LIST_NAME];
    }

    return (0);
}

cl_app_struct *
get_app_by_socket(int sock)
{

    int index = sock % CL_HASH_SIZE;
    cl_app_struct *head = cl_socket_hash[index];

    while (head) {

	if (head->cl_socket == sock)
	    return (head);
	head = head->listptr[CL_LIST_SOCKET];
    }

    return (0);
}

cl_app_struct *
get_app_by_id(int id)
{

    int index = id % CL_HASH_SIZE;
    cl_app_struct *head = cl_id_hash[index];

    while (head) {
	if (head->cl_id == id)
	    return (head);
	head = head->listptr[CL_LIST_ID];
    }

    return (0);
}

cl_pkt_buff *
pkt_buff_alloc(void *data, int len)
{

    cl_pkt_buff *head = cl_pkt_pool;
    cl_pkt_buff *ptr;

    if (!head)
	ptr = cl_pkt_pool = (cl_pkt_buff *) malloc(sizeof(cl_pkt_buff));
    else {
	while (head->next)
	    head = head->next;
	ptr = head->next = (cl_pkt_buff *) malloc(sizeof(cl_pkt_buff));
    }

    if (!ptr)
	return (0);

    if (!(ptr->data = (void *) malloc(len))) {
	free(ptr);
	return (0);
    }

    memcpy(ptr->data, data, len);
    ptr->len = len;
    ptr->next = 0;

    /* Owners keeps track of how many entities are holding */
    /* this structure */

    ptr->owners = 0;

    return (ptr);
}

void
pkt_buff_free(cl_pkt_buff * pkt)
{

    cl_pkt_buff *head = cl_pkt_pool;
    cl_pkt_buff *prev = 0;

    if (--pkt->owners > 1)
	return;			/* Don't free it until everyone has signed off */

    while (head) {
	if (head == pkt) {
	    if (prev == 0)
		cl_pkt_pool = head->next;
	    else
		prev->next = head->next;

	    if (head->data)
		free(head->data);
	    free(head);
	    return;
	}

	head = head->next;
    }
}

/* These functions help manage pending lists */

static cl_pending_struct *cl_pending_list = 0;

cl_pending_struct *
cl_AllocPending(void)
{

    cl_pending_struct *head = cl_pending_list;
    cl_pending_struct *ptr = 0;

    if (!head)
	ptr = cl_pending_list = calloc(sizeof(cl_pending_struct), 1);
    else {
	while (head->next)
	    head = head->next;
	ptr = head->next = calloc(sizeof(cl_pending_struct), 1);
    }

    return (ptr);
}

int
cl_havePending(void)
{
    return ((cl_pending_list == 0) ? 0 : 1);
}

#ifdef NOTUSED
int
cl_havePending(void)
{
    return (0);
}
#endif

void
cl_FreePending(cl_pending_struct * pend)
{

    cl_pending_struct *head = cl_pending_list;
    cl_pending_struct *prev = 0;

    while (head) {

	if (head == pend) {
	    if (!prev)
		cl_pending_list = head->next;
	    else
		prev->next = head->next;
	    free(head);
	    return;
	}

	head = head->next;
    }
}

/* Update each pending app */

void
cl_UpdatePending(int elapsed)
{

    cl_pending_struct *ptr = cl_pending_list;

    while (ptr) {

	cl_pending_struct *next = ptr->next;

	if (ptr->timeout != -1) {
	    ptr->timeout = ptr->timeout - elapsed;

	    if (ptr->timeout <= 0) {
		CL_ERROR(ptr->parent, CL_E_TIMEOUT, &ptr->request);
		cl_FreePending(ptr);
	    }
	}

	ptr = next;
    }
}

cl_pending_struct *
cl_SearchPending(unsigned char *name)
{
    cl_pending_struct *head = cl_pending_list;

    while (head) {
	if (!strcmp(head->name, name))
	    return (head);

	head = head->next;
    }

    return (0);
}

void
cl_printAppList(void)
{

    cl_app_struct *head = cl_main_list;

    while (head) {
	head = head->listptr[CL_LIST_MAIN];
    }
}

/* The pid list is maintained both on disk and in RAM */
/* this is just an easy way see if we have an app already running */

cl_pid_t *
cl_addPid(char *name, int pidnum)
{

    /* Allocate a structure for RAM */
    cl_pid_t *pid = (cl_pid_t *) calloc(sizeof(cl_pid_t), 1);

    if (!pid)
	return (0);

    if (!pidList)
	pidList = pid;
    else {
	cl_pid_t *p = pidList;
	while (p->next)
	    p = p->next;
	p->next = pid;
    }

    strncpy(pid->name, name, sizeof(pid->name));
    pid->pid = pidnum;

#ifdef DEBUG
    fprintf(stderr,
	    "DEBUG:  Added pid %d (%s) to the list...\n", pidnum, pid->name);
#endif

    return (pid);
}

cl_pid_t *
cl_findPid(int pidnum)
{

    cl_pid_t *pid = pidList;

#ifdef DEBUG
    fprintf(stderr, "DEBUG:  Looking for pid %d...", pidnum);
#endif

    while (pid) {
	if (pid->pid == pidnum) {
#ifdef DEBUG
	    fprintf(stderr, "DEBUG:  pid %d found\n", pidnum);
#endif
	    return (pid);
	}
	pid = pid->next;
    }

#ifdef DEBUG
    fprintf(stderr, "DEBUG: pid %d not found\n", pidnum);
#endif
    return (0);
}

void
cl_removePid(int pidnum)
{

    cl_pid_t *prev = 0, *pid = pidList;

    /* Remove it from the list */

    while (pid) {
	if (pid->pid == pidnum) {
	    if (prev)
		prev->next = pid->next;
	    else
		pidList = pid->next;
	    break;
	}

	prev = pid;
	pid = pid->next;
    }

#ifdef DEBUG
    fprintf(stderr,
	    "DEBUG:  Removed pid %d (%s) from the list...\n", pidnum,
	    pid->name);
#endif
    if (pid)
	free(pid);
}

cl_pid_t *
cl_findPidByName(char *app)
{

    cl_pid_t *pid = pidList;

    if (!app)
	return (0);

#ifdef DEBUG
    fprintf(stderr, "DEBUG:  Looking for app %s...", app);
#endif

    while (pid) {
	if (strcmp(app, pid->name) == 0) {
#ifdef DEBUG
	    fprintf(stderr, "DEBUG:  App %s is found\n", app);
#endif
	    return (pid);
	}

	pid = pid->next;
    }

#ifdef DEBUG
    fprintf(stderr, "DEBUG:  App %s not found\n", app);
#endif
    return (0);
}

cl_pid_t *
cl_findNameByPid(int pidnum)
{

    cl_pid_t *pid = pidList;

    if (!pidnum)
	return (0);

    while (pid) {
	if (pid->pid == pidnum)
	    return (pid);

	pid = pid->next;
    }

    return (0);
}
