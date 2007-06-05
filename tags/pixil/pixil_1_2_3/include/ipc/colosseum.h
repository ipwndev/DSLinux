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
 * See http://embedded.centurysoftware.com/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://embedded.centurysoftware.com/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


#ifndef _COLOSSEUM_H_
#define _COLOSSEUM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define CL_VERSION        "1.11 01/02/02"
#define CL_NAMED_SOCKET   "/tmp/.cl_socket"

#define CL_MAX_NAME_LEN   32	/* Thats a huge name */
#define CL_MAX_ARG_LEN   128
#define CL_MAX_MSG_LEN  4096	/* 4K messages are the max */

#define CL_MAX_LOG_LEN    50

#define CL_NORMAL_CLIENT  1
#define CL_LOG_CLIENT     2
#define CL_APPEND_ARGS	  3

/* This is the universal broadcast address */
#define CL_MSG_BROADCAST_ID  0xFFFF
#define CL_MSG_SERVER_ID     0x0000

#define CL_PKT_ERROR        0x01
#define CL_PKT_MESSAGE      0x02
#define CL_PKT_STARTAPP     0x03
#define CL_PKT_FINDAPP      0x04
#define CL_PKT_REGISTER     0x05
#define CL_PKT_FINDNAME     0x06
#define CL_PKT_LOG          0x07
#define CL_PKT_SPAWNAPP     0x08
#define CL_PKT_APP_INFO     0x09
#define CL_PKT_MSG_RESPONSE 0x10

/* Error messages from the server to the client */

#define CL_E_NONE          0
#define CL_E_NOAPP         1
#define CL_E_BADCMD        2
#define CL_E_TIMEOUT       3
#define CL_E_NACTIVE       4
#define CL_E_APPERR        5
#define CL_E_DESTFULL      6
#define CL_E_NODEST        7
#define CL_E_EMPTY         8
#define CL_E_NOSTART       9
#define CL_E_APPEXISTS    10
#define CL_E_APPUNKWN     11
#define CL_E_APPACTIVE    12
#define CL_E_NOTIMPLEMENT 13
#define CL_E_DBERR        14
#define CL_E_SPAWNERR     15
#define CL_E_NOSUCHAPP    16

/* Error messages to the client */

#define CL_CLIENT_BROADCAST   1
#define CL_CLIENT_SUCCESS     0
#define CL_CLIENT_ERROR      -1
#define CL_CLIENT_SOCK_ERROR -2
#define CL_CLIENT_NOSRVR     -3
#define CL_CLIENT_TIMEOUT    -4
#define CL_CLIENT_NODATA     -5
#define CL_CLIENT_NOCONN     -6
#define CL_CLIENT_INVALID    -7
#define CL_CLIENT_CONNECTED  -8
#define CL_CLIENT_BADNAME    -9
#define CL_CLIENT_NOTFOUND   -10
#define CL_CLIENT_NODEST     -11

#define CL_LOG_CRITICAL 0
#define CL_LOG_ERROR    1
#define CL_LOG_MESSAGE  2
#define CL_LOG_DEBUG    3

#define CL_LEVEL_COUNT 4

    /* This is a client side structure for app info */

#define CL_APP_INFO_PID  0
#define CL_APP_INFO_NAME 1

    typedef struct
    {
	int flags;
	unsigned char name[CL_MAX_NAME_LEN + 1];
	int processid;
    }
    cl_app_info;

/* The following are the different packet types we have */

/* This is sent when a recoverable error is encountered */
/* unrecoverable errors, of course, preclude any communications */

    typedef struct
    {
	unsigned char type;
	unsigned short len;
	unsigned char resp;
    }
    cl_pkt_header;

    typedef struct
    {
	cl_pkt_header header;
	unsigned short error_code;
    }
    cl_pkt_error;

/* This is the basic type for passing a true message */

    typedef struct
    {
	cl_pkt_header header;

	unsigned short src;	/* This is the app that sent the message */
	unsigned short dest;	/* This is the destination IPC id */

	/* This is the message payload.  We automatically set it */
	/* to make life easier right now.  This should change */

	int msglen;
	unsigned char message;	/* This is actually a place holder to the rest of the message */

    }
    cl_pkt_message;

    typedef struct
    {
	cl_pkt_header header;
	int dummy;
    }
    cl_pkt_msg_response;

/* A quick little macro to calculate the size of a message packet */
/* with a given message size */

#define MESSAGE_PKT_SIZE(len) ( sizeof(cl_pkt_message) - 1 + len )

    typedef struct
    {
	cl_pkt_header header;
	/* These are passed by the client */

	short timeout;
	unsigned short start_flags;
	unsigned char name[CL_MAX_NAME_LEN + 1];
	unsigned char argstr[CL_MAX_ARG_LEN + 1];

	/* This is set by the server on response */
	int ipc_id;
    }
    cl_pkt_start;

    typedef struct
    {
	cl_pkt_header header;
	/* These are passed by the client */

	unsigned char name[CL_MAX_NAME_LEN + 1];
	unsigned char argstr[CL_MAX_ARG_LEN + 1];

	/* This is set by the server on response */
	int pid;
    }
    cl_pkt_spawn;

    typedef struct
    {
	cl_pkt_header header;

	int flags;

	unsigned char name[CL_MAX_NAME_LEN + 1];
	int processid;
    }
    cl_pkt_appinfo;

    typedef struct
    {
	cl_pkt_header header;

	/* THis is sent by the client */
	unsigned char name[CL_MAX_NAME_LEN + 1];

	/* These are set by the server on response */
	int ipc_id;
    }
    cl_pkt_findapp;

    typedef struct
    {
	cl_pkt_header header;

	/* This is a set value, it should *not* be dynamic */
	unsigned char name[CL_MAX_NAME_LEN + 1];
	int start_flags;

	/* This is returned by the server */
	int ipc_id;

    }
    cl_pkt_reg;

    typedef struct
    {
	cl_pkt_header header;

	/* This is a set value, it should *not* be dynamic */
	int id;

	/* This is returned by the server */
	unsigned char name[CL_MAX_NAME_LEN + 1];

    }
    cl_pkt_findname;

    typedef struct
    {
	cl_pkt_header header;

	int level;
	unsigned char message[CL_MAX_LOG_LEN + 1];
    }
    cl_pkt_log;


/* This is the main union, that is passed about as part */
/* of a cl_pkt_buff */

    typedef union
    {
	cl_pkt_header header;
	cl_pkt_reg reg;
	cl_pkt_start start;
	cl_pkt_spawn spawn;
	cl_pkt_appinfo appinfo;

	cl_pkt_message message;
	cl_pkt_error error;

	cl_pkt_findapp findapp;
	cl_pkt_findname findname;
	cl_pkt_log log;
    }
    cl_packet;

#define CL_MAX_PKT_SIZE  (sizeof(cl_packet) + CL_MAX_MSG_LEN)

    int ClRegister(unsigned char *, int *);
    int ClReconnect(unsigned char *);

    int ClClose(void);
    int ClFindApp(unsigned char *);
    int ClStartApp(unsigned char *, unsigned char *, int, int);
    int ClSpawnApp(unsigned char *, unsigned char *);

    int ClGetAppInfo(cl_app_info * info);

    int ClSendMessage(int, void *, int);
    int ClGetMessage(void *, int *, unsigned short *);
    int ClGetNextMessage(void *, int *);
    int ClLookupName(int id, unsigned char *, int *);

    int ClRegisterLogger(char *name);
    int ClLogMessage(int level, unsigned char *message);

    int clGetAppInfo(cl_app_info * info);

#ifdef __cplusplus
}
#endif

#endif
