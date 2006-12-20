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
#include <xml/xml.h>

void
indentLine(FILE * stream, int count)
{
    int i;
    for (i = 0; i < count; i++)
	fprintf(stream, "\t");
}

int
recursiveEncode(FILE * stream, xml_encoder * engine,
		xml_encode * encode, void *in, int indent)
{

    /* Write the header */

    if (!encode->header) {
	indentLine(stream, indent);
	fprintf(stream, "<%s>\n", encode->tag);
    } else
	encode->header(stream, encode, in, indent);

    /* Write the data */
    if (encode->data)
	encode->data(stream, in);

    /* Write any subtags */
    if (encode->subtags) {
	xml_encode *list = encode->subtags;

	while (strlen(list->tag)) {
	    void *out = engine->find(list->match, in);

	    while (out) {
		recursiveEncode(stream, engine, list, out, indent + 1);
		out = engine->next(list->match, out);
	    }

	    list++;
	}
    }

    /* Write the footer */

    if (!encode->footer) {
	indentLine(stream, indent);
	fprintf(stream, "</%s>\n", encode->tag);
    } else
	encode->footer(stream, encode, in, indent);

    return (0);
}


int
xml_encodeFile(xml_encoder * engine, xml_encode * list,
	       char *filename, void *data)
{

    FILE *stream;
    xml_encode *encode = list;

    if (!filename)
	stream = stdout;
    else
	stream = fopen(filename, "w");

    if (!stream)
	return (-1);

    if (engine->header)
	engine->header(stream);

    while (strlen(encode->tag)) {

	void *in = engine->find(encode->match, data);
	if (in)
	    recursiveEncode(stream, engine, encode, in, 0);
	encode++;

    }

    if (engine->footer)
	engine->footer(stream);

    if (filename)
	fclose(stream);
    return (0);
}
