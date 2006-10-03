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

#include "nxmail.h"
#include "str_util.h"

/* RFC822 keyword callbacks */

void rfc822_ignore(char *, nxmail_header_t *);
void rfc822_date(char *, nxmail_header_t *);
void rfc822_from(char *, nxmail_header_t *);
void rfc822_to(char *, nxmail_header_t *);
void rfc822_subject(char *, nxmail_header_t *);
void rfc822_cc(char *, nxmail_header_t *);
void rfc822_replyto(char *, nxmail_header_t *);
void rfc822_build_address(nxmail_address_t * out, char *output);

struct
{
    char keyword[20];
    void (*callback) (char *, nxmail_header_t *);
}
rfc822_keywords[] =
{
    /* RFC822 Keywords */

    {
    "return-path", rfc822_ignore}
    , {
    "received", rfc822_ignore}
    , {
    "date", rfc822_date}
    , {
    "resent-date", rfc822_date}
    , {
    "from", rfc822_from}
    , {
    "sender", rfc822_from}
    , {
    "resent-from", rfc822_from}
    , {
    "resent-sender", rfc822_from}
    , {
    "to", rfc822_to}
    , {
    "Resent-to", rfc822_to}
    , {
    "cc", rfc822_cc}
    , {
    "Resent-cc", rfc822_cc}
    , {
    "bcc", rfc822_cc}
    , {
    "resent-bcc", rfc822_cc}
    , {
    "reply-to", rfc822_replyto}
    , {
    "reset-reply-to", rfc822_replyto}
    , {
    "subject", rfc822_subject}
    , {
    "message-id", rfc822_ignore}
    , {
    "resent-message-id", rfc822_ignore}
    , {
    "in-reply-to", rfc822_ignore}
    , {
    "references", rfc822_ignore}
    , {
    "keywords", rfc822_ignore}
    , {
    "comments", rfc822_ignore}
    , {
    "encrypted", rfc822_ignore}
    ,
	/* Extensions to RFC822 */
    {
    "auto-submitted", rfc822_ignore}
    , {
    "user-agent", rfc822_ignore}
    , {
    "status", rfc822_ignore}
    , {
    "<none>", 0}
};

void
rfc822_parse_header(char *buffer, nxmail_header_t * header, int totalsize)
{
    char linebuf[1024];
    char *curptr = buffer;

    /* Parse the header as it was given to us */

    while (curptr) {
	char *nextptr;
	char *keyword, *value;
	int keycount = 0;

	/* Grab the line and strip the endline */
	nextptr = str_getline(curptr, linebuf, 1024);

	/* If the size of the line is zero, then */
	/* start processing the body             */

	if (!strlen(linebuf))
	    break;
	curptr = nextptr;

	/* Go through and parse the keywords */

	if (!str_parsefield(linebuf, ':', &keyword, &value))
	    continue;
	str_lcase(keyword, strlen(keyword));

	keycount = 0;

	while (rfc822_keywords[keycount].callback) {
	    if (!strncmp(keyword, rfc822_keywords[keycount].keyword,
			 strlen(rfc822_keywords[keycount].keyword))) {
		rfc822_keywords[keycount].callback(value, header);
		break;
	    }

	    keycount++;
	}
    }

    /* Curptr is the now pointing to the start of the buffer */
    /* so we can determine the offset of the message text    */

    header->offset = (int) (curptr - buffer);
    header->msgsize = (int) (totalsize - header->offset);
}

static char *marray[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
    "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char *
get_month(int value)
{
    return (marray[value]);
}

static int
get_month_value(char *month)
{
    int i;

    for (i = 0; i < 12; i++)
	if (!strcmp(month, marray[i]))
	    return (i);

    return (0);
}

static char *warray[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

static char *
get_weekday(int value)
{
    return (warray[value]);
}

static int
get_weekday_value(char *wday)
{
    int i;

    for (i = 0; i < 7; i++)
	if (!strcmp(wday, warray[i]))
	    return (i);

    return (0);
}

void
rfc822_build_date(nxmail_date_t * in, char *out)
{
    sprintf(out, "%3s, %2d %3s %4d %2d:%2d:%2d",
	    get_weekday(in->wday), in->day, get_month(in->month),
	    in->year, in->hour, in->min, in->sec);
}

/* Take a date of the format [www], dd mmm yy hh:mm:ss zone +hhh */
/* and put it into a structure suitable for use by the user      */

void
rfc822_parse_date(char *date, nxmail_date_t * out)
{
    char wdaystr[3], monthstr[3];

    int day, year;
    int hour, min, sec;

    /* First, parse the date into the component pieces */
    /* We are not going to handle time zones, yet */

    sscanf(date, "%3s, %2d %3s %4d %2d:%2d:%2d", wdaystr, &day, monthstr,
	   &year, &hour, &min, &sec);

    out->month = get_month_value(monthstr);
    out->wday = get_weekday_value(wdaystr);

    if (day >= 0 && day < 31)
	out->day = day;

    if (year >= 0 && year < 3000)
	out->year = year;

    if (hour >= 0 && hour <= 24)
	out->hour = hour;

    if (min >= 0 && min <= 60)
	out->min = min;

    if (min >= 0 && min <= 60)
	out->sec = sec;
}

char *
rfc822_append_text(char *ptr, char *str)
{
    strcat(ptr, str);
    return (ptr + (strlen(str)));
}

char *
rfc822_append_field(char *ptr, char *fieldname, char *value)
{
    char outstr[1024];
    sprintf(outstr, "%s%s\n", fieldname, value);

    return (rfc822_append_text(ptr, outstr));
}

char *
rfc822_append_addrlist(char *ptr, char *field, nxmail_address_t * list)
{
    char *lptr = ptr;
    char addrstr[100];
    nxmail_address_t *alist = list;

    lptr = rfc822_append_text(lptr, field);

    while (alist) {
	rfc822_build_address(alist, addrstr);
	lptr = rfc822_append_text(lptr, addrstr);
	if (alist->next)
	    lptr = rfc822_append_text(lptr, ", ");

	alist = alist->next;
    }

    return (rfc822_append_text(lptr, "\n"));
}

int
rfc822_build_message(nxmail_header_t * header, char *body, char *output)
{
    char addrstr[100];
    char datestr[100];
    char *bufptr = output;

    /* date, from, subject, to, cc, bcc, reply to */


    rfc822_build_date(&header->date, datestr);

    bufptr = rfc822_append_field(bufptr, "Date: ", datestr);

    rfc822_build_address(&header->from, addrstr);

    bufptr = rfc822_append_field(bufptr, "From: ", addrstr);
    bufptr = rfc822_append_field(bufptr, "Subject: ", header->subject);

    bufptr = rfc822_append_addrlist(bufptr, "To: ", &header->to);

    if (strlen(header->cc.host))
	bufptr = rfc822_append_addrlist(bufptr, "cc: ", &header->cc);

    bufptr = rfc822_append_text(bufptr, "\n");
    bufptr = rfc822_append_text(bufptr, body);

    return ((int) (bufptr - output));
}

void
rfc822_ignore(char *inptr, nxmail_header_t * header)
{
    return;
}

void
rfc822_date(char *inptr, nxmail_header_t * header)
{
    rfc822_parse_date(inptr, &header->date);
    return;
}

void
rfc822_from(char *inptr, nxmail_header_t * header)
{
    /* Get the from field */
    rfc822_parse_address(&header->from, inptr);
}

void
rfc822_to(char *inptr, nxmail_header_t * header)
{
    nxmail_address_t *addrlist = &header->to;
    int count = nxmail_count_addrstr(inptr);

    if (!count)
	return;

    count = nxmail_alloc_addrlist(addrlist, count);
    nxmail_build_addrlist(inptr, addrlist, count);
}

void
rfc822_cc(char *inptr, nxmail_header_t * header)
{
    nxmail_address_t *addrlist = &header->cc;
    int count = nxmail_count_addrstr(inptr);

    if (!count)
	return;

    count = nxmail_alloc_addrlist(addrlist, count);
    nxmail_build_addrlist(inptr, addrlist, count);
}

void
rfc822_replyto(char *inptr, nxmail_header_t * header)
{
    /* Get the to field */
    strncpy(header->replyto, inptr, sizeof(header->replyto) - 1);
}

void
rfc822_subject(char *inptr, nxmail_header_t * header)
{
    strncpy(header->subject, inptr, sizeof(header->subject) - 1);
}

void
rfc822_build_address(nxmail_address_t * out, char *output)
{
    char addrstr[256];

    if (strlen(out->host))
	sprintf(addrstr, "%s@%s", out->mailbox, out->host);
    else
	sprintf(addrstr, "%s", out->mailbox);

    if (strlen(out->name)) {
	sprintf(output, "%s <%s>", out->name, addrstr);
    } else
	sprintf(output, "%s", addrstr);
}

/* These charaters cannot be in a address string */

static char addrchars[] = { '(', ')', '<', '>', ',',
    ';', '\\', '"', '[', ']', ' ',
};


static int
rfc822_verify_addr(char *addr, int len)
{
    int i;

    for (i = 0; i < len; i++) {
	int c;

	for (c = 0; c < 11; c++)
	    if (addr[i] == addrchars[c])
		return (0);
    }

    return (1);
}

char *
rfc822_parse_name(nxmail_address_t * dest, char *address)
{
    int size;
    char *ptr, *nptr;

    if (*address == '"')
	ptr = address + 1;
    else
	ptr = address;

    /* Go until we find another quote or a < */
    for (nptr = ptr; *nptr; nptr++) {
	if (*nptr == '"')
	    break;

	if (*nptr == '<')
	    break;

	/* If we find a at sign with no quotes or qualifiers, bet that */
	/* we're barking up the wrong tree */

	if (*nptr == '@' && *address != '"')	/* uhoh, its an address.  Bail! */
	    return (address);

	/* This helps ensure that we don't get caught in a loop */
	if ((int) (nptr - ptr) > sizeof(dest->name))
	    break;
    }

    /* Ok, we have some amount of name, ensure that we can fit it */
    /* into the structure without overrunning the buffer */

    size = (int) (nptr - ptr);

    if (size >= sizeof(dest->name)) {
	size = sizeof(dest->name) - 1;
    }

    if (size)
	strncpy(dest->name, ptr, size);

    return (nptr + 1);
}

char *
rfc822_parse_mailbox(nxmail_address_t * dest, char *address)
{
    char *ptr, *nptr;

    if (*address == '<')
	ptr = address + 1;
    else
	ptr = address;

    /* Go to the end */

    for (nptr = ptr; *nptr && *nptr != '>' && *nptr != ' '; nptr++);

    if (rfc822_verify_addr(ptr, (int) (nptr - ptr)) == 1) {
	/* The address checks out with no bad chars */
	/* now try to get the mailbox and host */

	char *aptr = strchr(ptr, '@');

	if (aptr) {
	    int size;

	    if ((int) (aptr - ptr) >= sizeof(dest->mailbox)) {
		size = sizeof(dest->mailbox) - 1;
	    } else {
		size = (int) (aptr - ptr);
	    }

	    strncpy(dest->mailbox, ptr, size);
	    dest->host[size + 1] = 0;

	    if ((int) (nptr - aptr - 1) >= sizeof(dest->host)) {
		size = sizeof(dest->host) - 1;
	    } else
		size = (int) (nptr - aptr - 1);

	    strncpy(dest->host, aptr + 1, size);
	    dest->host[size + 1] = 0;
	} else {
	    int size;

	    if ((int) (nptr - ptr) >= sizeof(dest->mailbox))
		size = sizeof(dest->mailbox) - 1;
	    else
		size = (int) (nptr - ptr);

	    strncpy(dest->mailbox, ptr, size);
	}

	return (nptr + 1);
    }

    /* Otherwise, something didn't come out right */
    return (address);
}

void
rfc822_parse_address(nxmail_address_t * dest, char *address)
{
    char *ptr = address;

    while (*ptr) {
	char *nptr;

	/* First, strip any whitespace */
	while (*ptr) {
	    if (*ptr != ' ' && *ptr != '\t')
		break;

	    ptr++;
	}

	if (!*ptr)
	    break;

	if (*ptr == '"' || *ptr == ' ') {
	    ptr = rfc822_parse_name(dest, ptr);
	    continue;
	}

	if (*ptr == '<') {
	    ptr = rfc822_parse_mailbox(dest, ptr);
	    continue;
	}

	nptr = rfc822_parse_name(dest, ptr);

	if (nptr != ptr) {
	    ptr = nptr;
	    continue;
	}

	ptr = rfc822_parse_mailbox(dest, ptr);
    }
}
