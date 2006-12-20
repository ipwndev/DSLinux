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


#ifndef NXMAIL_H
#define NXMAIL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

#define NXMAIL_TYPE_POP3 0
#define NXMAIL_TYPE_LAST 1

#define NXMAIL_OK            0
#define NXMAIL_SEND_OK       0

#define NXMAIL_ERROR        -1
#define NXMAIL_STREAM_OPEN  -2
#define NXMAIL_NO_STREAM    -3
#define NXMAIL_ADDR_ERROR   -4
#define NXMAIL_NET_ERROR    -5
#define NXMAIL_SEND_FAILURE -6
#define NXMAIL_SEND_ERROR   -7
#define NXMAIL_NETWORK_ERROR -8

    typedef struct
    {
	int fd;
	int type;
    }
    nxmail_stream;

    typedef struct
    {
	int (*mail_open) (char *, int, nxmail_stream *);
	int (*mail_login) (nxmail_stream *, char *, char *);
	int (*mail_status) (nxmail_stream *, int *, int *);
	int (*mail_msginfo) (nxmail_stream *, int);
	int (*mail_fetchheader) (nxmail_stream *, int, char **);
	int (*mail_fetchbody) (nxmail_stream *, int, char **);
	int (*mail_delete) (nxmail_stream *, int);
	void (*mail_close) (nxmail_stream *);
    }
    nxmail_driver_t;

    typedef struct nxmail_addr_t
    {
	char name[40];
	char mailbox[40];
	char host[40];
	int result;
	struct nxmail_addr_t *next;
    }
    nxmail_address_t;

    typedef struct
    {
	int month;
	int day;
	int year;
	int wday;

	int hour;
	int min;
	int sec;
    }
    nxmail_date_t;

    typedef struct
    {
	char type[50];
	char name[50];
	int charset;
	int encoding;
	char description[50];
    }
    nxmail_mime_header_t;

    typedef struct
    {
	/* Message variables */
	nxmail_date_t date;
	nxmail_address_t to;
	nxmail_address_t from;
	nxmail_address_t cc;
	char replyto[50];
	char subject[100];
	nxmail_mime_header_t mimeheader;

	/* Housekeeping variables */
	int offset;
	int msgsize;
    }
    nxmail_header_t;

    typedef struct nxmail_bdy_t
    {
	nxmail_mime_header_t mimeheader;
	int size;
	char *text;
	struct nxmail_bdy_t *next;
    }
    nxmail_body_t;

#define NXMAIL_ENCODING_NONE 0
#define NXMAIL_ENCODING_BASE64 1

#define NXMAIL_CHARSET_USASCII 0

    int nxmail_get_error(void);

    nxmail_stream *nxmail_init_stream(void);
    void nxmail_close_stream(nxmail_stream * stream);

    int nxmail_open(nxmail_stream * stream, char *address, int port,
		    int type);
    int nxmail_auth(nxmail_stream * stream, char *user, char *password);
    int nxmail_status(nxmail_stream * stream, int *msgcount);

    nxmail_header_t *nxmail_fetchheader(nxmail_stream * stream, int msgno);
    nxmail_body_t *nxmail_fetchbody(nxmail_stream * stream, int msgno);

    int nxmail_sendmsg(char *server, int port,
		       nxmail_header_t * header, char *body, int size);

    void nxmail_parsedate(char *rfc822_date, nxmail_date_t * date);

    int nxmail_delete(nxmail_stream * stream, int message);
    void nxmail_close(nxmail_stream * stream);

/* Various parsers that may be useful to people */
    void rfc822_parse_address(nxmail_address_t * dest, char *address);

/* Address list management */


    void nxmail_build_addrstr(nxmail_address_t * out, char *output);
    int nxmail_count_addrstr(char *addrstring);

    int nxmail_build_addrlist(char *addrstring,
			      nxmail_address_t * addrlist, int count);

    int nxmail_alloc_addrlist(nxmail_address_t * head, int count);
    void nxmail_free_addrlist(nxmail_address_t * head);

    void nxmail_parse_dateval(unsigned long, nxmail_date_t * date);

    nxmail_body_t *nxmail_alloc_body_struct(int size);
    void nxmail_free_body_struct(nxmail_body_t * head);

#ifdef __cplusplus
}
#endif

#endif
