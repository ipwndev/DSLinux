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
#include <ctype.h>
#include <time.h>

#include <par/pardb.h>
#include "database.h"
#include <xml/xml.h>

static void *generic_init(xml_token *, void *);

/* These are the local decoding functions */

static void *init_key(xml_token *, void *);
static void *data_key(xml_token *, void *, char *, int);
static void *init_section(xml_token *, void *);

static void *data_caplist(xml_token *, void *, char *, int);
static void *generic_textlist(xml_token *, void *, char *, int);

static void *data_color(xml_token *, void *, char *, int);
static void *data_text(xml_token *, void *, char *, int);
static void *data_value(xml_token *, void *, char *, int);

static void * applet_init(xml_token *tag, void *in);
static void *applet_data(xml_token *tag, void *data, char *text, int size);

/* These are the local encoding functions */

static void prefs_header(FILE *, xml_encode *, void *, int);
static void app_header(FILE *, xml_encode *, void *, int);

static void par_named_header(FILE * stream, xml_encode * encode,
			     void *data, int indent);

static void par_data_header(FILE *, xml_encode *, void *, int);
static void par_data_footer(FILE *, xml_encode *, void *, int);
static void par_data(FILE *, void *);
static void par_color(FILE *, void *);

static void category_header(FILE *, xml_encode *, void *, int);


/* The tags are defined here for readabilty */
#include "partags.h"

static void *
init_key(xml_token * tag, void *in)
{

    void *out = 0;
    xml_prop *prop;

    if (!in) {
	fprintf(stderr, "Error - No data for tag <%s>\n", tag->tag);
	return (0);
    }

    for (prop = tag->props; prop; prop = prop->next) {
	if (strcmp(prop->keyword, "key") == 0) {
	    out = (void *) tree_addNode((tree_t *) in, prop->value);
	    break;
	}
    }

    return (out);
}

/* This is a generic function that will take a item and append it to an */
/* ever growing list */

static void *
generic_textlist(xml_token * tag, void *data, char *text, int size)
{

    tree_t *node;
    if (!data) {
	fprintf(stderr, "Error - No data for tag <%s>\n", tag->tag);
	return (0);
    }

    /* First, we need to see if this node already exists in the tree */
    node = tree_findChildNode((tree_t *) data, tag->tag);

    if (node) {
	char *ndata = alloca(node->size + strlen(text) + 2);
	sprintf(ndata, "%s %s", (char *) node->data, text);

	tree_addData(node, ndata, strlen(ndata), PAR_TEXT);
    } else {
	node = tree_addNode((tree_t *) data, tag->tag);
	tree_addData(node, text, size, PAR_TEXT);
    }

    return ((void *) node);
}

static void *
data_caplist(xml_token * tag, void *data, char *text, int size)
{

    char *lname = 0;
    tree_t *node;
    xml_prop *prop;

    if (!data) {
	fprintf(stderr, "No data for tag %s\n", tag->tag);
	return (0);
    }

    for (prop = tag->props; prop; prop = prop->next) {
	if (strcmp(prop->keyword, "name") == 0) {
	    lname = prop->value;
	    break;
	}
    }

    if (!lname) {
	fprintf(stderr, "Couldn't find the keyword 'name' in %s\n", tag->tag);
	return (0);
    }

    printf("Adding / Modifying %s\n", lname);

    /* First, we need to see if this node already exists in the tree */
    node = tree_findChildNode((tree_t *) data, lname);

    if (node) {
	char *ndata = alloca(node->size + strlen(text) + 2);
	sprintf(ndata, "%s %s", (char *) node->data, text);

	tree_addData(node, ndata, strlen(ndata), PAR_TEXT);
    } else {
	node = tree_addNode((tree_t *) data, lname);
	tree_addData(node, text, size, PAR_TEXT);
    }

    return ((void *) node);
}


/* Generic function to add an integer to the par database */

static void *
data_value(xml_token * tag, void *data, char *text, int size)
{

    int val;
    tree_t *node;

    if (!data) {
	fprintf(stderr, "Error - No data for tag <%s>\n", tag->tag);
	return (0);
    }

    if (!text)
	return (0);

    /* If the first char is not a digit, then error */
    if (!isdigit(*text))
	return (0);

    val = atoi(text);
    node = tree_addNode((tree_t *) data, tag->tag);

    tree_addData(node, (void *) &val, sizeof(int), PAR_INT);

    return ((void *) node);
}

/* This is a generic function that will add a text item to the database */

static void *
data_text(xml_token * tag, void *data, char *text, int size)
{

    tree_t *node;

    if (!data) {
	fprintf(stderr, "Error - No data for tag <%s>\n", tag->tag);
	return (0);
    }

    node = tree_addNode((tree_t *) data, tag->tag);
    tree_addData(node, text, size, PAR_TEXT);

    return ((void *) node);
}

/* This is a generic function that adds a color to the database */

static void *
data_color(xml_token * tag, void *data, char *text, int size)
{

    tree_t *node = 0;
    unsigned long val;
    xml_prop *prop;

    if (!data || !text)
	return (0);

    for (prop = tag->props; prop; prop = prop->next) {
	if (strcmp(prop->keyword, "name") == 0) {
	    node = (void *) tree_addNode((tree_t *) data, prop->value);
	    break;
	}
    }

    if (!node)
	return (0);

    if (xml_parseColor(text, &val, size) == -1) {
	fprintf(stderr, "Error - Unable to parse the color\n");
	return (0);
    }

    tree_addData(node, &val, sizeof(unsigned long), PAR_INT);
    return ((void *) node);
}

/* This is a specific function that handles a preference value */

static void *
data_key(xml_token * tag, void *data, char *text, int size)
{

    struct bool_list
    {
	char word[5];
	char value;
    }
    bools[] =
    { {
    "yes", 1}
    , {
    "no", 0}
    , {
    "true", 1}
    , {
    "false", 0}
    , {
    "on", 1}
    , {
    "off", 0}
    };

    xml_prop *prop;
    char bool = 0;		/* Default to false */

    if (!data) {
	fprintf(stderr, "Error - No data for tag <%s>\n", tag->tag);
	return (0);
    }

    for (prop = tag->props; prop; prop = prop->next)
	if (strcmp(prop->keyword, "type") == 0)
	    break;

    /* If no keyword was specified, then assume text */

    if (!prop) {
	tree_addData((tree_t *) data, text, size, PAR_TEXT);
	return (data);
    }

    /* Set the keyword to lower case */

    xml_lowerCase(prop->value, -1);

    if (strcmp(prop->value, "str") == 0) {
	tree_addData((tree_t *) data, text, size, PAR_TEXT);
	return (data);
    }

    if (strcmp(prop->value, "int") == 0) {
	int ival;

	/* Check to see if this is a hex by chance */

	if ((size > 2) && (strncmp(text, "0x", 2) == 0))
	    ival = strtol(text, 0, 16);
	else
	    ival = atoi(text);

	tree_addData((tree_t *) data, &ival, sizeof(int), PAR_INT);
	return (data);
    }

    if (strcmp(prop->value, "float") == 0) {
	float fval = atof(text);
	tree_addData((tree_t *) data, &fval, sizeof(float), PAR_FLOAT);
	return (data);
    }

    if (strcmp(prop->value, "color") == 0) {
	unsigned long val;
	if (xml_parseColor(text, &val, size) == -1) {
	    fprintf(stderr, "Error - Unable to parse the color\n");
	    return (0);
	}

	tree_addData((tree_t *) data, &val, sizeof(unsigned long), PAR_COLOR);
	return (data);
    }

    if (strcmp(prop->value, "bool")) {
	fprintf(stderr, "Error - Invalid field type %s\n", prop->value);
	return (0);
    }

    if (*text >= '0' && *text <= '9') {
	int val = atoi(text);
	bool = ((val == 0) ? 0 : 1);
    } else {
	int i;

	xml_lowerCase(text, size);

	for (i = 0; i < 6; i++) {
	    if (size < strlen(bools[i].word))
		continue;

	    if (strncmp(bools[i].word, text, strlen(bools[i].word)) == 0)
		bool = bools[i].value;
	    break;
	}
    }

    tree_addData((tree_t *) data, &bool, sizeof(unsigned char), PAR_BOOL);
    return (data);
}

/* This is a generic function that handles tags like this: */
/* <tagname name="value"> */

static void *
init_section(xml_token * tag, void *in)
{

    void *out = 0;
    xml_prop *prop;

    if (!in) {
	fprintf(stderr, "Error - No data for tag <%s>\n", tag->tag);
	return (0);
    }

    for (prop = tag->props; prop; prop = prop->next) {
	if (strcmp(prop->keyword, "name") == 0) {
	    out = (void *) tree_addNode((tree_t *) in, prop->value);
	    break;
	}
    }

    return (out);
}

static void *
applet_data(xml_token * tag, void *data, char *text, int size) {
  tree_addData((tree_t *) data, text, size, PAR_TEXT);
  return data;
}

static void *
applet_init(xml_token *tag, void *in) {
  
  void *out = 0;
  xml_prop *prop;

  for (prop = tag->props; prop; prop = prop->next) {
    if (strcmp(prop->keyword, "name") == 0) {
      out = (void *) tree_addNode((tree_t *) in, prop->value);
      break;
    }
  }

  return out;
}
static void *
generic_init(xml_token * tag, void *in)
{

    void *out;

    /* Make a new node in the tree */
    if (!in) {
	fprintf(stderr, "Error - No data for tag <%s>\n", tag->tag);
	return (0);
    }

    out = (void *) tree_addNode((tree_t *) in, tag->tag);

    return (out);
}

/* This is the actual parsing function */

int
parseXML(char *file, tree_t * head)
{

    xml_parser engine;
    engine.tags = tagToplevel;

    /* Now, start the parser */
    return (xml_parseFile(&engine, file, ((void *) head)));
}

/* These are some generic headers and footers that we use */

static void
par_data_header(FILE * stream, xml_encode * encode, void *data, int indent)
{
    indentLine(stream, indent);
    fprintf(stream, "<%s>", encode->tag);
}

/* This is like some other headers, but without the \n at the end */

static void
par_named_header(FILE * stream, xml_encode * encode, void *data, int indent)
{

    tree_t *node = data;
    if (!node)
	return;

    indentLine(stream, indent);
    fprintf(stream, "<%s name=\"%s\">", encode->tag, node->keyword);
}

static void
par_data_footer(FILE * stream, xml_encode * encode, void *data, int indent)
{
    fprintf(stream, "</%s>\n", encode->tag);
}

static void
par_data(FILE * stream, void *in)
{

    tree_t *node = (tree_t *) in;

    if (!node)
	return;

    switch (node->type) {

    case PAR_NONE:
	break;

    case PAR_TEXT:
	fprintf(stream, "%s", (char *) node->data);
	break;

    case PAR_INT:
	fprintf(stream, "%i", *((int *) node->data));
	break;

    case PAR_BOOL:
	fprintf(stream, "%s", ((*((int *) node->data) == 0) ? "Yes" : "No"));
	break;

    case PAR_FLOAT:
	fprintf(stream, "%f", *((float *) node->data));
	break;

    case PAR_COLOR:
	fprintf(stream, "#%6.6lX",
		(*((unsigned long *) node->data)) & 0xFFFFFF);
	break;
    }
}

static void
par_color(FILE * stream, void *in)
{

    unsigned long val;
    tree_t *node = (tree_t *) in;
    if (!node)
	return;
    if (node->type != PAR_INT)
	return;

    val = *((unsigned long *) node->data);

    fprintf(stream, "#%6.6lX", val & 0xFFFFFF);
}



static void
prefs_header(FILE * stream, xml_encode * encode, void *data, int indent)
{

    tree_t *node = (tree_t *) data;
    if (!node)
	return;

    indentLine(stream, indent);

    fprintf(stream, "<%s key=\"%s\"", encode->tag, node->keyword);

    switch (node->type) {
    case PAR_NONE:
	break;

    case PAR_TEXT:
	fprintf(stream, " type=STR");
	break;

    case PAR_INT:
	fprintf(stream, " type=INT");
	break;

    case PAR_BOOL:
	fprintf(stream, " type=BOOL");
	break;

    case PAR_FLOAT:
	fprintf(stream, " type=FLOAT");
	break;

    case PAR_COLOR:
	fprintf(stream, " type=COLOR");
	break;
    }

    fprintf(stream, ">");
}



static void
app_header(FILE * stream, xml_encode * encode, void *data, int indent)
{

    tree_t *node = data;
    if (!node)
	return;

    indentLine(stream, indent);
    fprintf(stream, "<%s name=\"%s\">\n", encode->tag, node->keyword);
}

static void
category_header(FILE * stream, xml_encode * encode, void *data, int indent)
{

    tree_t *node = data;
    if (!node)
	return;

    indentLine(stream, indent);
    fprintf(stream, "<%s name=\"%s\">\n", encode->tag, node->keyword);
}


static void
par_xml_header(FILE * stream)
{

    char buffer[26];

    time_t curtime;

    time(&curtime);
    strcpy(buffer, ctime(&curtime));
    buffer[strlen(buffer) - 1] = 0;

    fprintf(stream, "<!-- Pixil Application Registry Database -->\n");
    fprintf(stream, "<!-- Written %s -->\n\n", buffer);
}

static void *
par_xml_find(char *keyword, void *in)
{

    tree_t *node = (tree_t *) in;

    if (!node)
	return (0);
    node = node->child;

    /* If it is a wildcard, then get the first child */
    if (keyword[0] == '*')
	return (node);

    /* Otherwise, search for the specific keyword */

    while (node) {
	if (strcmp(keyword, node->keyword) == 0)
	    return ((void *) node);

	node = node->peer;
    }

    return (0);
}

static void *
par_xml_next(char *keyword, void *in)
{

    tree_t *node = (tree_t *) in;
    if (!node)
	return (0);

    node = node->peer;

    if (keyword[0] == '*')
	return (node);

    while (node) {
	if (strcmp(keyword, node->keyword) == 0)
	    return ((void *) node);

	node = node->peer;
    }

    return (0);
}

int
encodeXML(char *filename, tree_t * head)
{

    xml_encoder engine;

    engine.header = par_xml_header;
    engine.footer = 0;
    engine.find = par_xml_find;
    engine.next = par_xml_next;

    return (xml_encodeFile(&engine, encodeGlobal, filename, (void *) head));
}
