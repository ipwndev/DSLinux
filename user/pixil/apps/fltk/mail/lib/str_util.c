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


#include <string.h>

static inline int
isspace(char test)
{
    return ((test == ' ' || test == '\t'));
}

/* Turn the given string into lower case */

void
str_lcase(char *value, int len)
{
    int i;

    for (i = 0; i < len; i++)
	if (value[i] >= 65 && value[i] <= 90)
	    value[i] = value[i] + 32;
}

char *
str_skipwhite(char *ptr)
{
    while (isspace(*ptr) && *ptr)
	ptr++;
    return (ptr);
}

/* WARNING:  THIS MANGLES THE STRING */
char *
str_getfield(char *in, char delim, char **field)
{
    char *ptr = in;

    ptr = str_skipwhite(ptr);

    if (!*ptr) {
	*field = 0;
	return (0);
    }

    *field = ptr;

    /* Now advance until the delim */
    while (*ptr != delim && *ptr)
	ptr++;

    if (*ptr) {
	*ptr = 0;
	if (*(ptr + 1))
	    return (ptr + 1);
    }

    return (0);
}

/* Given a buffer, parse off the keyword and value */
/* WARNING:  THIS MANGLES THE STRING */

int
str_parsefield(char *in, char delim, char **keyword, char **value)
{
    char *ptr = in;

    ptr = str_skipwhite(ptr);

    if (!*ptr)
	return (0);		/* Nothing found */

    *keyword = ptr;

    /* Now advance until the delim */
    while (*ptr != delim && ptr)
	ptr++;

    if (!*ptr)
	return (0);

    *ptr = 0;

    ptr = str_skipwhite(ptr + 1);

    if (!*ptr)
	return (0);

    *value = ptr;
    return (1);
}


/* This copies the string, but it does not mangle it! */

char *
str_getline(char *in, char *out, int outsize)
{
    char *inpos = in;
    char *outpos = out;

    int copied = 0;

    /* Zero out the whole buffer */
    bzero(out, outsize);

    while (*inpos) {
	int linesize = 0;
	char *curpos = inpos;

	/* Find the end of the current line */
	while (*curpos != '\n' && *curpos)
	    curpos++;

	/* Calculate the size of the line */
	linesize = (int) (curpos - inpos);

	/* If the total size of the string is greater than the input, then copy as much as we can */

	if (copied + linesize >= outsize) {
	    strncpy(outpos, inpos, outsize - copied);
	    return (curpos + 1);	/* And return */
	}

	/* Otherwise, attach the string */
	strncpy(outpos, inpos, linesize);

	curpos++;
	if (!*curpos)
	    return (0);		/* Nothing more to see here */

	/* If it is not white space, then this is a new line */
	if (!isspace(*curpos))
	    return (curpos);

	/* If there was whitespace, then this is a continued line, so add it on */

	/* Turn tabs into spaces */
	for (; *curpos && *curpos == '\t'; curpos++)
	    *curpos = ' ';

	inpos = curpos;

	outpos += linesize;
	copied += linesize;
    }

    return (0);
}
