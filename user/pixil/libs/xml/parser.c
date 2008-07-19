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


/* TODO:  Turn the tag structures into a binary tree for faster searching */
/*        Add a stream mode so we can read from STDIN                     */

#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#ifndef __USE_ISOC99
#define __USE_ISOC99
#endif

#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <sys/mman.h>

#include <xml/xml.h>

inline static char
file_getChar(xml_file * file, char *ch)
{
    return (((ulong) ch < ((ulong) file->start + file->length)) ? *ch : 0);
}

static void
local_freeTag(xml_token * tag)
{
    xml_prop *ptr = tag->props;

    while (ptr) {
	xml_prop *temp = ptr->next;
	free(ptr);
	ptr = temp;
    }

    if (tag->tagstr)
	free(tag->tagstr);
    free(tag);
}

static xml_prop *
local_newProp(xml_prop ** head)
{
    if (!*head) {
	*head = (xml_prop *) calloc(sizeof(xml_prop), 1);
	return (*head);
    } else {
	xml_prop *ptr = *head;
	while (ptr->next)
	    ptr = ptr->next;

	ptr->next = (xml_prop *) calloc(sizeof(xml_prop), 1);
	return (ptr->next);

    }
}

static int
local_getTag(xml_file * file, void **pos, char **token, int *size)
{

    int ret;

    char ch;

    char *start = (char *) *pos;
    char *end;

    /* Strip any whitespace */

    while (1) {
	ch = file_getChar(file, start);
	if (!ch)
	    return (PARSE_EOF);

	if (!isblank(ch) && ch != '\n' && ch != '\r')
	    break;

	if (ch == '\n')
	    file->lineno++;
	start++;
    }

    /* If this is the beginning of a tag, then search for the end */

    if (file_getChar(file, start) == '<') {

	start++;
	end = start;

	if (!(ch = file_getChar(file, end)))
	    return (PARSE_EOF);

	if (ch == '!') {
	    ret = PARSE_COMMENT;
	    end++;
	} else if (ch == '/') {
	    ret = PARSE_CLOSE;
	    end++;
	} else
	    ret = PARSE_TAG;

	while ((ch = file_getChar(file, end))) {
	    if (ch != '>') {
		end++;
		continue;
	    }

	    *size = (int) (end - start);
	    *end = 0;
	    *token = start;
	    *((unsigned char **) pos) = end + 1;
	    return (ret);
	}

	fprintf(stderr, "Parse error:  Unterminated tag at line %d\n",
		file->lineno);

	/* The tag was never closed Thats an error */
	return (PARSE_ERROR);
    }

    end = start;

    while ((ch = file_getChar(file, end))) {
	if (ch == '<' || ch == '>') {

	    if (ch == '<')
		ret = PARSE_TEXT;
	    else
		ret = PARSE_ERROR;

	    *size = (end - start);
	    *token = start;

	    *((unsigned char **) pos) = end;
	    return (ret);
	}
	end++;
    }

    return (PARSE_EOF);
}

static xml_token *
local_tokenize(char *tag)
{
    char endchar;
    xml_token *local;
    char *start, *end;

    if (!tag) {
	DPRINT("INTERNAL:  No string was presented to local_tokenize\n");
	return (0);
    }

    /* Now, create a new tag structure */

    if (!(local = (xml_token *) calloc(sizeof(xml_token), 1))) {
	DPRINT("INTERNAL:  Error on malloc in local_tokenize\n");
	return (0);
    }

    local->tagstr = (char *) malloc(strlen(tag) + 1);

    if (!local->tagstr) {
	free(local);
	return (0);
    }

    strcpy(local->tagstr, tag);

    /* Now we can start parsing out the keywords.  Since */
    /* the string was copied, it is easy for us to mangle it */

    /* First step, look for the inital tag */
    start = tag;

    for (end = tag; *end && !isblank(*end); end++);

    /* If this is just the tag, then return */
    if (!*end) {
	local->tag = start;
	return (local);
    }

    *end = 0;			/* terminate the keyword */
    local->tag = start;

    start = end + 1;

    /* Now, go through and parse all of the keywords */
    while (*start) {
	xml_prop *local_prop;

	/* Strip any inital whitespace */
	for (; isblank(*start) || *start == '\n'; start++);
	if (!*start)
	    return (local);	/* Nothing else left */

	/* Now we have a keyword, make a new structure */
	if (!(local_prop = local_newProp(&local->props))) {
	    DPRINT("INTERNAL:  Error on malloc in local_newProp()\n");
	    return (local);
	}

	/* Now find the end of the keyword (it will be either */
	/* an = or a whitespace */

	for (end = start; (*end) && (*end != '=') && (!isblank(*end)); end++);

	endchar = *end;
	*end = 0;

	local_prop->keyword = start;
	xml_lowerCase(local_prop->keyword, -1);

	if (!endchar)
	    return (local);

	if (endchar == '=') {
	    start = end + 1;

	    /* Once again, skip any whitespace */
	    for (; isblank(*start); start++);
	    if (!*start) {
		fprintf(stderr, "Parse warning:  Keyword %s has no value\n",
			local_prop->keyword);
		return (local);	/* Nothing else left */
	    }

	    /* If the first char is a '"' then go until the last '"' otherwise, 
	       go until the last space / end-of-line */

	    if (*start == '\"') {
		for (end = start + 1; *end && *end != '\"'; end++);
		if (!*end) {
		    fprintf(stderr,
			    "Parse error:  Unterminated quote near %s.\n",
			    start);
		    local_freeTag(local);
		    return (0);	/* Return with an error! */
		} else {
		    start++;
		    *end = 0;
		    local_prop->value = start;
		}
	    } else {
		for (end = start; *end && !isblank(*end); end++);

		if (!*end) {
		    local_prop->value = start;
		    return (local);	/* Done! */
		} else
		    *end = 0;

		local_prop->value = start;
	    }
	} else
	    fprintf(stderr, "Parse warning: keyword %s has no value\n",
		    local_prop->keyword);

	/* End the keyword / value, and start over again */

	*end = 0;
	start = end + 1;
    }

    return (local);
}

xml_tag *
local_findTag(xml_tag * list, char *tag)
{

    xml_tag *local = list;
    if (!list)
	return (0);

    while (strlen(local->tag)) {
	if (strcmp(local->tag, tag) == 0)
	    return (local);
	local++;
    }

    return (0);
}

/* xml_recursiveParse() 
   Description:  This is the main parsing engine 
   
   engine - the XML engine (mainly the tags)
   file - the current file we are parsing
   pos - the current position in the file
   cur - The tag that prompted this search
   parent - The parent of the tag 
   parent_data - Data from the parent 
*/

static int
xml_recursiveParse(xml_parser * engine, xml_file * file,
		   void **pos, xml_token * current,
		   xml_tag * parent, void *parent_data)
{

    int ret;

    xml_token *next;
    xml_tag *tag;

    void *data = 0;

    tag = local_findTag(parent, current->tag);
    if (!tag) {
	fprintf(stderr,
		"Parse error:  Tag <%s> unknown for <%s> on line %d\n",
		current->tag, parent->tag, file->lineno);
	return (-1);
    }

    /* Run the init function (if it exists) */
    if (tag->init)
	data = tag->init(current, parent_data);
    else
	data = parent_data;

    /* Now, we want to parse the rest of the block */

    while (1) {
	int dsize = 0;
	char *token;

	switch (local_getTag(file, pos, &token, &dsize)) {

	case PARSE_EOF:
	    return (0);

	case PARSE_ERROR:
	    return (-1);	/* Bail on errors */

	case PARSE_COMMENT:
	    break;		/* Skip comments */

	case PARSE_TEXT:	/* This is data, if the tag wants it */
	    if (tag->data)
		tag->data(current, data, token, dsize);
	    break;

	case PARSE_TAG:
	    /* A new tag was found, go ahead and parse it */
	    next = local_tokenize(token);
	    if (!next)
		return (-1);	/* DOH! */

	    /* And recursively parse this sucker */
	    ret =
		xml_recursiveParse(engine, file, pos, next, tag->subtags,
				   data);

	    /* Free the tag now that we are done with it */
	    local_freeTag(next);
	    if (ret == -1)
		return (-1);	/* On error, return the same */
	    break;

	case PARSE_CLOSE:
	    /* A close tag, is it for the correct token? */

	    if (strcmp(current->tag, &token[1]) == 0) {
		if (tag->end)
		    tag->end(current, data);
		return (0);	/* Success */
	    } else
		return (-1);	/* Error - Bad close */
	}
    }
}

static int
xml_parseToplevel(xml_parser * engine, xml_file * file, void *udata)
{

    int ret;

    void *pos = file->start;	/* Start at the beginning */
    char *token = 0;

    xml_token *local;

    while (1) {
	int dsize = 0;

	/* Get the next token */
	switch (local_getTag(file, &pos, &token, &dsize)) {

	case PARSE_EOF:
	    return (0);		/* EOF, get outta here */

	case PARSE_CLOSE:
	    fprintf(stderr, "Error - Unexpected '>' at line %d\n",
		    file->lineno);
	    return (-1);

	case PARSE_ERROR:
	    fprintf(stderr, "Error - Parse error at line %d\n", file->lineno);
	    return (-1);

	case PARSE_COMMENT:
	    break;		/* Skip over comments */

	case PARSE_TAG:
	    /* Parse the token */
	    local = local_tokenize(token);
	    if (!local) {
		fprintf(stderr, "Error - Token error at line %d\n",
			file->lineno);
		return (-1);
	    }

	    ret =
		xml_recursiveParse(engine, file, &pos, local, engine->tags,
				   udata);

	    /* Free the current token */
	    local_freeTag(local);

	    /* If the recursive parse failed, then bail */
	    if (ret == -1) {
		printf("Error - Parse error at line %d\n", file->lineno);
		return (-1);
	    }
	    break;
	}
    }

    return (0);
}

int
xml_parseFile(xml_parser * engine, char *filename, void *userdata)
{

    int fd, ret;
    struct stat s;
    xml_file file;

    /* Stat the file to make sure it exists */

    if (stat(filename, &s) == -1)
	return (-1);

    /* Open the file and map it */
    fd = open(filename, O_RDONLY, 0);
    if (!fd)
	return (-1);

    file.start =
	mmap(0, s.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (!file.start) {
	close(fd);
	return (-1);
    }

    file.length = s.st_size;
    file.lineno = 1;

    /* Start the parser */
    ret = xml_parseToplevel(engine, &file, userdata);

    /* Clean up after ourselves */
    munmap(file.start, file.length);
    close(fd);
    return (ret);
}

/* These are a few often used utilities that are useful to define here */

/* Convert the specified string to lower case */

void
xml_lowerCase(char *string, int count)
{

    int size, i;

    if (count == -1)
	size = strlen(string);
    else
	size = count;

    for (i = 0; i < size; i++)
	string[i] = tolower(string[i]);
}

/* Converted the specified color triplet */
/* (#RRGGBB) to the individual colors    */

int
xml_parseColor(char *string, unsigned long *val, int size)
{

    char *ptr;

    /* We should only accept the standard color form */

    if (string[0] != '#') {
	*val = 0;
	return (-1);
    }

    ptr = &string[1];

    /* We only accept the 6 digit form */
    if (size != 7) {
	*val = 0;
	return (-1);
    }

    /* Now, this is pretty easy, just convert the color to a long */
    *val = strtoul(ptr, NULL, 16);

    return (0);
}
