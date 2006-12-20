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

#include "nxmail.h"
#include "rfc822.h"
#include "mime.h"
#include "str_util.h"

/* This is a local structure that holds all of the mime information */
/* that the client may not care about.  This cuts down on each      */
/* body section carrying around a huge mime header structure that   */
/* is basically useless                                             */

typedef struct
{
    char type[50];
    char bound[100];
    char name[50];
    char description[50];

    int encoding;
    int charset;
}
local_mimeheader_t;

/* These are all of the possible 1.1 MIME fields.  Most we can ignore, but those we care about */


static char *
mime_find_next_section(char *buffer, char *boundary)
{
    char linebuf[1024];
    char *curptr = buffer;

    while (1) {
	char *nextptr;

	if (!curptr)
	    return (0);		/* Nothing there, bail */

	/* Grab the line and strip the endline */
	nextptr = str_getline(curptr, linebuf, 1024);

	/* Jump in two dashes */

	if (linebuf[0] == '-' && linebuf[1] == '-') {
	    if (!strncmp(linebuf + 2, boundary, strlen(boundary))) {
		/* It is a boundary.  Is it the last one? */

		if (linebuf[2 + strlen(boundary)] == '-')
		    return (0);
		else
		    return (curptr);
	    }
	}

	curptr = nextptr;
    }
}

void
mime_ignore(char *inptr, local_mimeheader_t * header)
{
    return;
}

void
mime_description(char *inptr, local_mimeheader_t * header)
{
    strncpy(header->description, inptr, 50);
}

void
mime_contentEncoding(char *inptr, local_mimeheader_t * header)
{
    /* Determine the various encoding schemes here */
    /* Right now we only handle BASE64 */

    str_lcase(inptr, strlen(inptr));

    if (!strncmp(inptr, "base64", strlen("base64")))
	header->encoding = NXMAIL_ENCODING_BASE64;
    else
	header->encoding = NXMAIL_ENCODING_NONE;

    return;
}

void
mime_charset(char *inptr, local_mimeheader_t * header)
{
}

void
mime_boundary(char *inptr, local_mimeheader_t * header)
{
    /* Watch out for the quotes! */

    char *ptr = inptr + 1;

    /* Look for the other quote, igoring any escaped quotes */

    while (*ptr) {
	if (*ptr == '"' && *(ptr - 1) != '\\')
	    break;
	ptr++;
    }

    if (!*ptr)
	return;

    strncpy(header->bound, inptr + 1, (int) (ptr - inptr) - 1);
}

void
mime_name(char *inptr, local_mimeheader_t * header)
{
    strncpy(header->name, inptr, 50);
}

struct
{
    char keyword[10];
    void (*callback) (char *, local_mimeheader_t * header);
}
content_keyword[] =
{
    {
    "charset", mime_charset}
    , {
    "name", mime_name}
    , {
    "boundary", mime_boundary}
    , {
    "<none>", 0}
};

/* Stupid content type has a million entries all it own, so we need */
/* to do parsing within parsing */

void
mime_contentType(char *inptr, local_mimeheader_t * header)
{
    char *field;
    char *curptr;

    /* Get the first entry, which is the content type */

    curptr = str_getfield(inptr, ';', &field);

    if (field)
	strcpy(header->type, field);
    str_lcase(header->type, strlen(header->type));

    /* Now, parse the other items */
    while (curptr) {
	char *keyword, *value;

	curptr = str_getfield(curptr, ';', &field);

	if (str_parsefield(field, '=', &keyword, &value)) {
	    int count = 0;
	    str_lcase(keyword, strlen(keyword));

	    /* Go through the content keywords */

	    while (content_keyword[count].callback) {
		if (!strncmp(keyword, content_keyword[count].keyword,
			     strlen(content_keyword[count].keyword))) {
		    content_keyword[count].callback(value, header);
		    break;
		}

		if (content_keyword[count].callback == 0)
		    break;

		count++;
	    }
	}
    }

    return;
}

struct
{
    char keyword[30];
    void (*callback) (char *, local_mimeheader_t * header);
}
mime_keywords[] =
{
    {
    "--", mime_ignore}
    , {
    "accept", mime_ignore}
    , {
    "accept-charset", mime_ignore}
    , {
    "accept-encoding", mime_ignore}
    , {
    "accept-language", mime_ignore}
    , {
    "accept-ranges", mime_ignore}
    , {
    "authorization", mime_ignore}
    , {
    "cache-control", mime_ignore}
    , {
    "connection", mime_ignore}
    , {
    "content-encoding", mime_contentEncoding}
    , {
    "content-length", mime_ignore}
    , {
    "content-range", mime_ignore}
    , {
    "content-transfer-encoding", mime_contentEncoding}
    , {
    "content-type", mime_contentType}
    , {
    "content-description", mime_description}
    , {
    "digest-MessageDigest", mime_ignore}
    , {
    "keep-alive", mime_ignore}
    , {
    "link", mime_ignore}
    , {
    "location", mime_ignore}
    , {
    "max-forwards", mime_ignore}
    , {
    "mime-version", mime_ignore}
    , {
    "pragma", mime_ignore}
    , {
    "protocol", mime_ignore}
    , {
    "protocol-info", mime_ignore}
    , {
    "protocol-request", mime_ignore}
    , {
    "proxy-authenticate", mime_ignore}
    , {
    "proxy-authorization", mime_ignore}
    , {
    "public", mime_ignore}
    , {
    "range", mime_ignore}
    , {
    "referer", mime_ignore}
    , {
    "retry-after", mime_ignore}
    , {
    "server", mime_ignore}
    , {
    "trailer", mime_ignore}
    , {
    "transfer-encoding", mime_ignore}
    , {
    "upgrade", mime_ignore}
    , {
    "user-agent", mime_ignore}
    , {
    "vary", mime_ignore}
    , {
    "via", mime_ignore}
    , {
    "warning", mime_ignore}
    , {
    "www-authenticate", mime_ignore}
    , {
    "authentication-info", mime_ignore}
    , {
    "proxy-authentication-info", mime_ignore}
    , {
    "<none>", 0}
};

char *
mime_parse_header(char *input, local_mimeheader_t * header)
{
    char linebuf[1024];

    char *keyword, *value;

    char *pos = input;

    while (pos) {
	int count = 0;

	if (!pos)
	    return (0);		/* Nothing there, bail out early */

	/* Get the line from the input buffer */
	pos = str_getline(pos, linebuf, 1024);

	if (strlen(linebuf) == 0)
	    break;

	/* Parse it */
	if (str_parsefield(linebuf, ':', &keyword, &value)) {
	    str_lcase(keyword, strlen(keyword));

	    while (mime_keywords[count].callback) {
		if (!strncmp(keyword, mime_keywords[count].keyword,
			     strlen(mime_keywords[count].keyword))) {
		    mime_keywords[count].callback(value, header);
		    break;
		}

		count++;
	    }
	}
    }

    return (pos);
}

nxmail_body_t *
mime_parse_section(char *input, int sectionsize, local_mimeheader_t * header)
{
    local_mimeheader_t localheader;

    nxmail_body_t *body;
    char *bodyptr;

    bzero(&localheader, sizeof(local_mimeheader_t));

    /* Create a body structure */
    body = nxmail_alloc_body_struct(sectionsize);

    if (!body)
	return (0);

    /* Now, check out the section header */
    bodyptr = mime_parse_header(input, &localheader);

    if (!bodyptr) {
	free(body);
	return (0);
    }

    /* Store the mime information */

    strcpy(body->mimeheader.type, localheader.type);
    strcpy(body->mimeheader.description, localheader.description);
    strcpy(body->mimeheader.name, localheader.name);

    body->mimeheader.encoding = localheader.encoding;
    body->mimeheader.charset = localheader.charset;

    body->text = bodyptr;
    body->size = (input + sectionsize) - bodyptr;
    return (body);
}

/* This function parses the given message into a series of body sections for each mime type */
void
mime_parse_message(char *message, int msgsize, nxmail_header_t * header,
		   nxmail_body_t ** body)
{
    char *curptr = message;
    local_mimeheader_t localheader;

    bzero(&localheader, sizeof(local_mimeheader_t));

    /* Step 1:  Go through the main header again, looking for mime related keywords */

    curptr = mime_parse_header(curptr, &localheader);

    /* The curptr should be pointing at the start of the message, but we're going to cheat */
    /* and grab the offset from the nxmail_header_t, just to be safe */

    curptr = message + header->offset;

    /* Easiest case.  No mime encoding at all.  Call it a text/plain and bail */

    if (!strlen(localheader.type)) {
	int size;

	nxmail_body_t *bodyptr;

	sprintf(header->mimeheader.type, "text/plain");

	header->mimeheader.encoding = NXMAIL_ENCODING_NONE;
	header->mimeheader.charset = NXMAIL_CHARSET_USASCII;

	size = (int) (header->msgsize);

	bodyptr = *body = nxmail_alloc_body_struct(size);

	bodyptr->size = size;
	bodyptr->text = curptr;
	return;
    }

    /* MIME encoding was specified.  If the mimetype is not multipart/, then */
    /* assume its all one big happy message */

    if (strncmp(localheader.type, "multipart", strlen("multipart"))) {
	int size;
	nxmail_body_t *bodyptr;

	strcpy(header->mimeheader.type, localheader.type);

	header->mimeheader.encoding = localheader.encoding;
	header->mimeheader.charset = localheader.charset;

	size = (int) (header->msgsize);
	bodyptr = *body = nxmail_alloc_body_struct(size);

	bodyptr->size = size;
	bodyptr->text = curptr;
	return;
    }

    /* So its a multipart message.  Find the first section */

    curptr = mime_find_next_section(curptr, localheader.bound);

    /* No message.  Thats not right.... */

    while (1) {
	char linebuf[1024];
	char *nextptr;
	nxmail_body_t *section = 0;
	int size = 0;

	if (!curptr)
	    return;

	/* First order of business, get rid of the boundary line */
	curptr = str_getline(curptr, linebuf, 1024);

	if (!curptr)
	    return;

	/* Now find the next section */

	nextptr = mime_find_next_section(curptr, localheader.bound);

	if (!nextptr)
	    size = (int) ((message + msgsize) - curptr);
	else
	    size = (int) (nextptr - curptr);

	/* Now parse the section */

	if (!*body) {
	    *body = mime_parse_section(curptr, size, &localheader);
	    section = *body;
	} else {
	    section->next = mime_parse_section(curptr, size, &localheader);
	    if (section->next)
		section = section->next;
	}

	/* Go back around for the next section */
	curptr = nextptr;
    }
}
