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


#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "nxmail.h"
#include "rfc822.h"
#include "mime.h"
#include "cache.h"
#include "mail_smtp.h"
#include "net_util.h"

extern nxmail_driver_t nxmail_pop3_driver;

static nxmail_driver_t *drivers[] = { &nxmail_pop3_driver };

static nxmail_stream nxmail_openstream;

int
nxmail_get_error(void)
{
    return (GET_NET_ERROR);
}

/* This will initalize the stream and get the cache ready to go */

nxmail_stream *
nxmail_init_stream(void)
{
    cache_init();
    bzero(&nxmail_openstream, sizeof(nxmail_openstream));
    return (&nxmail_openstream);
}

void
nxmail_close_stream(nxmail_stream * stream)
{
    nxmail_close(stream);
    cache_close();
    bzero(&nxmail_openstream, sizeof(nxmail_openstream));
}

int
nxmail_open(nxmail_stream * stream, char *address, int port, int type)
{
    int ret;

    if (type > NXMAIL_TYPE_LAST)
	return (0);

    ret = drivers[type]->mail_open(address, port, stream);

    if (ret == NXMAIL_NETWORK_ERROR)
	return (NXMAIL_ERROR);

    stream->type = type;

    return (ret);
}

int
nxmail_delete(nxmail_stream * stream, int msgno)
{
    if (!stream)
	return (NXMAIL_NO_STREAM);

    if (stream->fd == 0)
	return (NXMAIL_NO_STREAM);

    return (drivers[stream->type]->mail_delete(stream, msgno));
}

int
nxmail_auth(nxmail_stream * stream, char *user, char *password)
{
    if (!stream)
	return (NXMAIL_NO_STREAM);

    if (stream->fd == 0)
	return (NXMAIL_NO_STREAM);

    return (drivers[stream->type]->mail_login(stream, user, password));
}

int
nxmail_status(nxmail_stream * stream, int *msgcount)
{
    int msize, ret;

    if (!stream)
	return (NXMAIL_NO_STREAM);

    if (stream->fd == 0)
	return (NXMAIL_NO_STREAM);

    ret = drivers[stream->type]->mail_status(stream, msgcount, &msize);

    return (ret);
}


static cache_entry_t *
nxmail_retheader(nxmail_stream * stream, int msgno)
{
    cache_entry_t *centry;
    int size;
    char *buffer;

    /* Get the header only */
    size = drivers[stream->type]->mail_fetchheader(stream, msgno, &buffer);

    /* Now allocate a cache entry and put it in */
    if ((centry = get_new_cache_entry(msgno, size)) != NULL) {
	/* Copy over what we have into the raw portion */
	memcpy(centry->raw, buffer, size);

	/* Parse the header */
	rfc822_parse_header(centry->raw, &centry->header, size);
    }
    /* end of if */
    free(buffer);
    return (centry);
}

void
nxmail_parse_full(cache_entry_t * centry)
{
    mime_parse_message(centry->raw, centry->size, &centry->header,
		       &centry->body);
}

/* Retrieve the message, and parse the header */

static cache_entry_t *
nxmail_retbody(nxmail_stream * stream, int msgno)
{
    cache_entry_t *centry = search_cache_entry(msgno);

    char *buffer;
    int size;

    if (!stream)
	return (0);
    if (stream->fd == 0)
	return (0);

    size = drivers[stream->type]->mail_fetchbody(stream, msgno, &buffer);

    /* If the cache already exists, then reuse it */

    if (centry) {
	/* Free the raw space and replace it */
	if (centry->raw)
	    free(centry->raw);
	centry->raw = (char *) malloc(size);
	centry->size = size;
    } else {
	centry = get_new_cache_entry(msgno, size);
    }

    if (!centry)
	return (0);

    memcpy(centry->raw, buffer, size);

    /* Reparse the header */
    rfc822_parse_header(centry->raw, &centry->header, size);
    nxmail_parse_full(centry);

    free(buffer);
    return (centry);
}

/* Given cache entry, parse out the full message */



/* We don't normally cache headers, but if it exists, then use it */

nxmail_header_t *
nxmail_fetchheader(nxmail_stream * stream, int msgno)
{
    cache_entry_t *entry = search_cache_entry(msgno);

    /* Check the cache to see if this message already exists */
    if (!entry)
	entry = nxmail_retheader(stream, msgno);
    if (!entry)
	return (0);

    return (&entry->header);
}

nxmail_body_t *
nxmail_fetchbody(nxmail_stream * stream, int msgno)
{
    cache_entry_t *entry = search_cache_entry(msgno);

    if (!entry || !entry->body)
	entry = nxmail_retbody(stream, msgno);

    if (!entry)
	return (0);
    return (entry->body);
}

void
nxmail_parsedate(char *rfc822_date, nxmail_date_t * date)
{
    rfc822_parse_date(rfc822_date, date);
}

void
nxmail_close(nxmail_stream * stream)
{
    if (!stream)
	return;

    drivers[stream->type]->mail_close(stream);
}

int
nxmail_sendmsg(char *server, int port,
	       nxmail_header_t * header, char *body, int size)
{
    int ret;
    int outsize;
    char *output;

    /* I'm not happy about this, but for now, this will work */
    /* Allocate an extra 1K for the header                   */

    output = (char *) calloc(size + 1024, 1);

    if (!output)
	return (NXMAIL_ERROR);

    outsize = rfc822_build_message(header, body, output);

    ret = smtp_send_message(server, port, header, output, outsize);
    free(output);
    return (ret);
}

int
nxmail_count_addrstr(char *addrstring)
{
    char *startptr = addrstring;
    int count = 0;

    if (*startptr)
	count = 1;
    else
	return (0);

    while (1) {
	for (; *startptr != ',' && *startptr; startptr++);
	if (!*startptr++)
	    return (count);

	count++;
    }
}

int
nxmail_build_addrlist(char *addrstring, nxmail_address_t * addrlist,
		      int count)
{
    nxmail_address_t *addr = addrlist;
    char *startptr = addrstring;
    char *endptr;
    int i;

    for (i = 0; i < count; i++) {
	char str[100];

	bzero(str, 100);

	sscanf(startptr, "%[^,],", str);

	if (!strlen(str))
	    return (i);

	/* bzero(addr, sizeof(nxmail_address_t)); */

	rfc822_parse_address(addr, str);

	endptr = (startptr + strlen(str) + 1);

	if (!*endptr)
	    return (i + 1);
	for (; *endptr == ' ' && *endptr; endptr++);
	if (!*endptr)
	    return (i + 1);

	if (addr->next)
	    addr = addr->next;
	else
	    return (i + 1);
	startptr = endptr;
    }

    return (count);
}

int
nxmail_alloc_addrlist(nxmail_address_t * head, int count)
{
    int i;

    /* The first entry (the head) is always static, so only */
    /* allocate memory if there is more than 1 address      */

    if (count == 1)
	return (1);

    for (i = 1; i < count; i++) {
	head->next = (nxmail_address_t *) calloc(sizeof(nxmail_address_t), 1);

	if (!head->next)
	    return (i);

	head = head->next;
    }

    return (count);
}

void
nxmail_free_addrlist(nxmail_address_t * head)
{
    nxmail_address_t *ptr = head->next;
    nxmail_address_t *nptr;

    while (ptr) {
	nptr = ptr->next;
	free(ptr);
	ptr = nptr;
    }
}

void
nxmail_parse_dateval(unsigned long seconds, nxmail_date_t * date)
{
    struct tm *tm = localtime(&seconds);

    date->wday = tm->tm_wday;
    date->day = tm->tm_mday;
    date->month = tm->tm_mon;
    date->year = (tm->tm_year + 1900);

    date->hour = tm->tm_hour;
    date->min = tm->tm_min;
    date->sec = tm->tm_sec;
}

nxmail_body_t *
nxmail_alloc_body_struct(int size)
{
    nxmail_body_t *ptr = (nxmail_body_t *) calloc(sizeof(nxmail_body_t), 1);
    if (!ptr)
	return (0);

    ptr->size = size;
    ptr->next = 0;

    return (ptr);
}

void
nxmail_free_body_struct(nxmail_body_t * head)
{
    nxmail_body_t *ptr = head;

    while (ptr) {
	nxmail_body_t *nptr = ptr->next;
	free(ptr);
	ptr = nptr;
    }
}
