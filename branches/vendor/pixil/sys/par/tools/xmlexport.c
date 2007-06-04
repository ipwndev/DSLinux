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
#include <getopt.h>

#include <par/pardb.h>
#include <xml/xml.h>

extern int encodeXML(char *filename, tree_t * head);

void
print_usage(void)
{
    printf("\n-------\n");
    printf("xmlexport [-h] -f <XML file> <DB file>\n\n");

    printf("[-h, --help] - print this screen\n");
    printf("[-f, --file] - The XML output file to send to (if not stdout)\n");
    printf("[-v, --verbose] - Print a ton of messages\n");
    printf("-------\n\n");
}

#define DO_VERBOSE   0x01
#define HAVE_OUTPUT  0x02

int
main(int argc, char **argv)
{

    int flags = 0, ret;

    db_handle *db;

    char input_filename[512];
    char output_filename[512];

    tree_t *tree;

    while (1) {
	signed char c;
	int option_index;

	static struct option long_options[] = {
	    {"help", 0, 0, 'h'},
	    {"file", 1, 0, 'f'},
	    {"verbose", 0, 0, 'v'},
	    {0, 0, 0, 0}
	};

	c = getopt_long(argc, argv, "hd:v", long_options, &option_index);
	if (c == -1)
	    break;

	switch (c) {
	case 'h':
	    print_usage();
	    exit(0);

	case 'f':
	    strcpy(output_filename, optarg);
	    flags |= HAVE_OUTPUT;
	    break;

	case 'v':
	    flags |= DO_VERBOSE;
	    break;

	default:
	    print_usage();
	    exit(-1);
	}
    }

    if (optind < argc)
	strcpy(input_filename, argv[optind]);
    else {
	fprintf(stderr, "ERROR:  You must specify an input database\n");
	exit(-1);
    }


    /* Create a brand new tree for this to attach to */

    if (!db_newTree(&tree)) {
	fprintf(stderr, "ERROR:  Got internal error %d\n", pardb_errno);
	exit(-1);
    }

    db = db_openDB(input_filename, PAR_DB_MODE_RDONLY);

    if (!db) {
	switch (pardb_errno) {
	case PARDB_NOFILE:
	    fprintf(stderr, "ERROR:  Database file %s doesn't exist\n",
		    input_filename);
	    break;

	case PARDB_FILEERR:
	    fprintf(stderr, "ERROR:  File error wihle opening %s\n",
		    input_filename);
	    break;

	case PARDB_BADDB:
	    fprintf(stderr, "ERROR:  %s is not a PAR database\n",
		    input_filename);
	    break;

	default:
	    fprintf(stderr,
		    "ERROR:  Got internal error %d while opening %s.\n",
		    pardb_errno, input_filename);
	    break;
	}

	goto exitError;
    }

    if (db_loadTree(db, tree)) {
	fprintf(stderr, "ERROR:  Got inernal error %d while reading %s.\n",
		pardb_errno, input_filename);
	goto exitError;
    }

    db_closeDB(db);
    db = 0;

    if (!(flags & HAVE_OUTPUT))
	ret = encodeXML(0, tree);
    else
	ret = encodeXML(output_filename, tree);

    if (ret == -1) {
	fprintf(stderr,
		"ERROR: Couldn't export the database to XML format\n");
	goto exitError;
    }

    db_freeTree(tree);
    return (0);

  exitError:
    if (tree)
	db_freeTree(tree);
    if (db)
	db_closeDB(db);

    return (-1);
}
