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


#ifndef _SERVER_H_
#define _SERVER_H_

#include <sys/time.h>

#ifdef DEBUG
#define DPRINT(str, args...) printf("DEBUG: " str, ## args)
#else
#define DPRINT(str, args...)
#endif

#define CL_HASH_SIZE      256
#define CL_QUEUE_SIZE     5

/* This is a pkt buff that we use to shuttle packets around 
   internally 
*/

typedef struct cl_pkt_t
{
    unsigned short len;
    void *data;
    int owners;
    struct cl_pkt_t *next;
}
cl_pkt_buff;

/* Copious notes follow */

typedef struct cl_app_t
{
    unsigned char cl_name[CL_MAX_NAME_LEN + 1];	/* The unique name of the app */
    unsigned short cl_socket;	/* The socket the app is attached to */
    unsigned short cl_id;	/* The ipc ID assigned to the app */
    unsigned char cl_flags;	/* Various flags */

    /* every app can have a queue of waiting packets */

    struct
    {
	unsigned char head;	/* Head of the ring queue */
	unsigned char tail;	/* Tail of the ring queue */
	unsigned char size;	/* Size of the queue      */

	/* I am anticipating a day when the size of the queue */
	/* will be dynamic */

	cl_pkt_buff *ring[CL_QUEUE_SIZE];
    }
    cl_queue;

    /* Because we use the same structure in several different */
    /* lists and hashes, I thought it would just be easier to */
    /* have an array with the pointers here instead of        */
    /* allocating tiny structures for each list.  This is     */
    /* either genius or stupidity.  You decide.               */

    struct cl_app_t *listptr[4];
}
cl_app_struct;

typedef struct cl_pending_t
{

    cl_app_struct *parent;
    unsigned char flags;
    int timeout;

    struct cl_pending_t *next;

    unsigned char name[CL_MAX_NAME_LEN + 1];
    cl_pkt_start request;

}
cl_pending_struct;

typedef struct cl_pid
{
    unsigned char name[CL_MAX_NAME_LEN + 1];
    int pid;
    struct cl_pid *next;
}
cl_pid_t;

/* These are the names for each of the lists that the cl_app_struct */
/* cal be a part of */

#define CL_LIST_MAIN    0
#define CL_LIST_SOCKET  1
#define CL_LIST_NAME    2
#define CL_LIST_ID      3

/* Status flags */

#define CL_F_STATE_MASK   0x0F
#define CL_F_NEW          0x01
#define CL_F_ACTIVE       0x02
#define CL_F_PENDING      0x04

#define CL_F_QUEUE_MSG    0x10
#define CL_F_WAITING      0x12
#define CL_F_RESTRICTED   0x14

#define SET_STATE(flags, state) (flags = (flags & ~CL_F_STATE_MASK) | state)
#define SET_FLAG(flags, state)  (flags |= state)
#define CLEAR_FLAG(flags, state) (flags &= ~state)
#define ISSET_FLAG(flags, state) (flags & state)

/* Prototypes */

/* Socket / Client functions */

int cl_InitServerSocket(void);
int cl_HandleNewClient(int);
int cl_ClientWrite(int, unsigned char *, int);
int cl_ClientRead(int, cl_pkt_buff **);
int cl_GetClientSockets(fd_set *, fd_set *, fd_set *);
void cl_CloseClient(int);

/* Packet functions */

int cl_SendError(cl_app_struct *, int, cl_packet *, int);
int cl_SendPacket(cl_app_struct *, cl_packet *, int);
int cl_SendMessage(cl_app_struct *, cl_pkt_buff *);

/* Searching functions */

cl_app_struct *get_app_by_name(unsigned char *);
cl_app_struct *get_app_by_id(int);
cl_app_struct *get_app_by_socket(int);

cl_app_struct *get_first_app(void);
cl_app_struct *get_next_app(cl_app_struct *);

/* Queue functions */

int queue_add_packet(cl_app_struct *, cl_pkt_buff *);
cl_pkt_buff *queue_get_packet(cl_app_struct *);
cl_pkt_buff *queue_peek_packet(cl_app_struct * app);
int data_in_queue(cl_app_struct * app);

/* Packet buffer functions */

cl_pkt_buff *pkt_buff_alloc(void *, int);
void pkt_buff_free(cl_pkt_buff *);


/* Application structure functions */

cl_app_struct *cl_AllocNewApp(int);
void cl_RegisterApp(cl_app_struct * app, cl_pkt_reg * data);
void cl_FreeAppStruct(cl_app_struct *);
void cl_FreeAllApps(void);
void cl_CloseApp(cl_app_struct * app);

/* Pending structure functions */
cl_pending_struct *cl_AllocPending(void);
void cl_FreePending(cl_pending_struct *);
void cl_UpdatePending(int);
cl_pending_struct *cl_SearchPending(unsigned char *);
int cl_havePending(void);

/* Packet handling functions */
/* FIXME:  In the future, we should move to a structure better */
/* suited for handling lots of different kinds of packets      */

int cl_HandleClientReq(cl_app_struct *);
int cl_HandleRegisterApp(cl_app_struct *, cl_pkt_reg *);
int cl_HandleStartApp(cl_app_struct *, cl_pkt_start *);
int cl_HandleFindApp(cl_app_struct *, cl_pkt_findapp *);
int cl_HandleSpawnApp(cl_app_struct * req, cl_pkt_spawn * pkt);
int cl_HandleAppInfo(cl_app_struct * req, cl_pkt_appinfo * pkt);

int cl_HandleQueuedMessages(cl_app_struct *);

/* PID functions */
cl_pid_t *cl_addPid(char *name, int pidnum);
cl_pid_t *cl_findPid(int pidnum);
void cl_removePid(int pidnum);
cl_pid_t *cl_findPidByName(char *app);
cl_pid_t *cl_findNameByPid(int pidnum);

/* Misc functions */
int cl_GetNextId(void);
int cl_doLog(int level, char *app, char *message, ...);

#ifdef HAVE_LOGGING

int cl_initLog(int level);
int cl_changeLevel(int new);
void cl_closeLog(void);

#endif

#define DO_LOG(level, message, args...) cl_doLog(level, "clserver", message, ## args)

/* Several assistance macros to make life easier */

#define CL_ERROR(app, err, pkt) (cl_SendError(app, err, (cl_packet *) pkt, sizeof(*pkt)))

#define PKT_BUFF_ADD_OWNER(pkt) (pkt->owners++)

#endif /* _SERVER_H_ */
