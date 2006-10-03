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
#include <unistd.h>

#include <par/pardb.h>
#include <xml/xml.h>

#define DO_APPEND     0x1
#define DO_INPUT_FILE 0x2
#define DO_VERBOSE    0x4

extern int parseXML(char *file, tree_t * head);


void
print_usage(void)
{
    printf("\n-------\n");
    printf("xmlimport [-h] [-a] -i <xml file> <DB file>\n\n");

    printf
	("[-a, --append] - If the database file exists, then append the new data to it\n");
    printf("[-h, --help] - print this screen\n");
    printf("[-i, --input] - The input XML file\n");
    printf("[-v, --verbose] - Print a ton of messages\n");
    printf("-------\n\n");
}

int
main(int argc, char **argv)
{

    unsigned short flags = 0;

    char input_filename[512];
    char output_filename[512];

    db_handle *db = 0;
    tree_t *tree = 0;

    while (1) {
	signed char c;
	int option_index;

	static struct option long_options[] = {
	    {"append", 0, 0, 'a'},
	    {"help", 0, 0, 'h'},
	    {"input", 1, 0, 'i'},
	    {"verbose", 0, 0, 'v'},
	    {0, 0, 0, 0}
	};

	c = getopt_long(argc, argv, "ahi:v", long_options, &option_index);
	if (c == -1)
	    break;

	switch (c) {
	case 'a':
	    flags |= DO_APPEND;
	    break;

	case 'h':
	    print_usage();
	    exit(0);

	case 'i':
	    strcpy(input_filename, optarg);
	    flags |= DO_INPUT_FILE;
	    break;

	case 'v':
	    flags |= DO_VERBOSE;
	    break;

	default:
	    print_usage();
	    exit(-1);
	}
    }

    /* Grab the output file */

    if (optind < argc)
	strcpy(output_filename, argv[optind]);
    else {
	fprintf(stderr, "ERROR: You must specify an output database file\n");
	print_usage();
	exit(-1);
    }

    /* FIXME:  Allow stdin!!!! */

    if (!(flags & DO_INPUT_FILE)) {
	fprintf(stderr, "ERROR: You must specify an input XML file\n");
	print_usage();
	exit(-1);
    }

    if (!db_newTree(&tree)) {
	fprintf(stderr,
		"ERROR: Got internal error %d while creating the tree\n",
		pardb_errno);
	exit(-1);
    }

    if (!access(output_filename, R_OK)) {
	if (flags & DO_APPEND) {
	    int ret;

	    if (flags & DO_VERBOSE)
		printf("Reading database file %s into memory...\n",
		       output_filename);

	    db = db_openDB(output_filename, PAR_DB_MODE_RDONLY);

	    if (!db) {
		switch (pardb_errno) {
		case PARDB_FILEERR:
		    fprintf(stderr, "ERROR: File error while opening %s.\n",
			    output_filename);
		    break;

		case PARDB_BADDB:
		    fprintf(stderr, "ERROR: %s is not a PAR database!\n",
			    output_filename);
		    break;

		default:
		    fprintf(stderr,
			    "ERROR: Got internal error %d while opening %s.\n",
			    pardb_errno, output_filename);
		    break;
		}

		goto exitError;
	    }

	    ret = db_loadTree(db, tree);
	    db_closeDB(db);
	    db = 0;

	    if (ret == -1) {
		fprintf(stderr,
			"ERROR:  Got internal error %d while reading %s.\n",
			pardb_errno, output_filename);
		goto exitError;
	    }
	} else {
	    fprintf(stderr,
		    "ERROR:  %s already exists. Use --append to add data to an existing file.\n",
		    output_filename);
	    goto exitError;
	}
    }

    /* Now parse the XML into the tree */
    if (flags & DO_VERBOSE)
	printf("Parsing the XML file %s....\n", input_filename);

    if (parseXML(input_filename, tree) != 0) {
	fprintf(stderr, "ERROR:  Got an error while parsing the XML file\n");
	goto exitError;
    }

    if (flags & DO_VERBOSE)
	printf("Opening the database %s for writing...\n", output_filename);

    /* Open the database to write out */
    db = db_newDB(output_filename, PAR_DB_NORMAL);

    if (!db) {
	if (pardb_errno == PARDB_FILEERR)
	    fprintf(stderr, "ERROR:  File error while creating %s.\n",
		    output_filename);
	else
	    fprintf(stderr,
		    "ERROR:  Got internal error %d while creating %s.\n",
		    pardb_errno, output_filename);

	goto exitError;
    }

    /* Now save the entire tree */

    if (flags & DO_VERBOSE)
	printf("Writing the database...\n");

    if (db_saveTree(db, tree)) {
	fprintf(stderr, "ERROR:  Got internal error %d while saving %s.\n",
		pardb_errno, output_filename);
	goto exitError;
    }

    if (flags & DO_VERBOSE)
	printf("The file was succesfully imported.\n");

    /* Sucess all around.  Close the database and leave */
    db_closeDB(db);
    return (0);

  exitError:

    if (tree)
	db_freeTree(tree);
    if (db)
	db_closeDB(db);

    return (-1);
}
