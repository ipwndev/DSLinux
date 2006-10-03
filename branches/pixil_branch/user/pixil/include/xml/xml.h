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


#ifndef _XML_H_
#define _XML_H_

#include <stdio.h>

#ifdef DEBUG
#ifndef DPRINT
#define DPRINT(str, args...) printf("DEBUG: " str, ## args)
#endif /* ndef DPRINT */
#else /* DEBUG */
#define DPRINT(str, args...)
#endif /* DEBUG */

typedef struct xml_prop_t
{
    char *keyword;
    char *value;
    struct xml_prop_t *next;
}
xml_prop;

typedef struct
{
    char *tagstr;		/* Local copy of the tag string (mangled) */
    char *tag;			/* The main tag */
    xml_prop *props;		/* A linked list of the provided properties */
}
xml_token;

typedef struct tag_struct
{
    char tag[25];
    struct tag_struct *subtags;

    void *(*init) (xml_token *, void *);
    void *(*data) (xml_token *, void *, char *, int);
    void *(*end) (xml_token *, void *);
}
xml_tag;

typedef struct
{
    void *start;
    int length;
    int lineno;
}
xml_file;

/* This is the parsing engine defined by the user */

typedef struct
{
    xml_tag *tags;
}
xml_parser;

typedef struct encode_struct
{
    char tag[25];
    char match[25];

    struct encode_struct *subtags;

    void (*header) (FILE *, struct encode_struct *, void *, int);
    void (*footer) (FILE *, struct encode_struct *, void *, int);
    void (*data) (FILE *, void *);
}
xml_encode;

/* This is the encoding engine defined by the user */

typedef struct
{
    void (*header) (FILE *);
    void (*footer) (FILE *);
    void *(*find) (char *, void *);
    void *(*next) (char *, void *);
}
xml_encoder;


#define PARSE_TAG     1
#define PARSE_COMMENT 2
#define PARSE_CLOSE   3
#define PARSE_TEXT    4
#define PARSE_EOF     5
#define PARSE_ERROR   6

/* parser.c */
int xml_parseFile(xml_parser *, char *, void *);
void xml_lowerCase(char *, int);

/* encoder.c */
void indentLine(FILE *, int);
int xml_encodeFile(xml_encoder *, xml_encode *, char *, void *);

/* Utilities */
void xml_lowerCase(char *string, int count);
int xml_parseColor(char *string, unsigned long *val, int size);

#endif
