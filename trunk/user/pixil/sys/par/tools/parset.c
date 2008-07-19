 /* (c) 2003 Century Software, Inc.   All Rights Reserved.     
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
#include <ctype.h>

#include <par/pardb.h>

void
print_usage(void)
{
    printf("\n-------\n");
    printf("parset [-h] -d <DB file> -t <type> <node> <value>\n\n");

    printf("[-h, --help] - print this screen.\n");
    printf("[-d, --db] - The database to read from.\n");
    printf("             (Uses the value of environment variable\n");
    printf("              $PARFILE if not specified).\n");
    printf("[-t, --type] - The type of variable being written.\n");
    printf("               (assumes TEXT if not specified).\n");
    printf("-------\n\n");
}

#define HAVE_DB_FILE 0x1

int
main(int argc, char **argv)
{

    db_handle *db;

    int ret = 0;

    unsigned short type = PAR_TEXT;
    unsigned char flags = 0;

    char *database_filename = 0;
    char *value = 0, *node;

    while (1) {
	signed char c;
	int option_index;
	char letter;

	static struct option long_options[] = {
	    {"help", 0, 0, 'h'},
	    {"db", 1, 0, 'd'},
	    {"type", 1, 0, 't'},
	    {0, 0, 0, 0}
	};

	c = getopt_long(argc, argv, "d:t:h", long_options, &option_index);
	if (c == -1)
	    break;

	switch (c) {
	case 'h':
	    print_usage();
	    return (0);

	case 't':
	    letter = tolower(optarg[0]);

	    switch (letter) {
	    case 't':
		type = PAR_TEXT;
		break;

	    case 'f':
		type = PAR_FLOAT;
		break;

	    case 'i':
		type = PAR_INT;
		break;

	    case 'b':
		type = PAR_BOOL;
		break;
	    }

	    break;

	case 'd':
	    database_filename = alloca(strlen(optarg) + 1);
	    strcpy(database_filename, optarg);

	    flags |= HAVE_DB_FILE;
	    break;

	default:
	    print_usage();
	    return (-1);
	}
    }

    if (optind < argc)
	node = argv[optind++];
    else {
	fprintf(stderr, "ERROR: You must specific a node to search for\n");
	print_usage();
	return (-1);
    }

    if (optind < argc)
	value = argv[optind++];
    else {
	fprintf(stderr, "ERROR:  You must specify a value to set\n");
	print_usage();
	return (-1);
    }

    if (!(flags & HAVE_DB_FILE)) {
	database_filename = db_getDefaultDB();
	if (!database_filename) {
	    fprintf(stderr,
		    "ERROR:  Could not determine the default database.\n");
	    print_usage();
	    return (-1);
	}
    }

    /* Open the file, search for the node, and output it to stdout */
    db = db_openDB(database_filename, PAR_DB_MODE_RW);
    if (!db) {
	switch (pardb_errno) {
	case PARDB_NOFILE:
	    fprintf(stderr, "ERROR:  File %s does not exist.\n",
		    database_filename);
	    break;

	case PARDB_FILEERR:
	    fprintf(stderr, "ERROR:  Error while opening %s.\n",
		    database_filename);
	    break;

	case PARDB_BADDB:
	    fprintf(stderr, "ERROR:  %s is not a PAR database\n",
		    database_filename);
	    break;

	default:
	    fprintf(stderr,
		    "ERROR:  Internal error %d occured while opening %s\n",
		    pardb_errno, database_filename);
	    break;
	}

	return (-1);
    }

    switch (type) {

    case PAR_TEXT:
	ret = db_addNode(db, node, value, strlen(value), PAR_TEXT);
	break;

    case PAR_INT:{
	    int ival;

	    if (strlen(value) > 2 && (strncmp(value, "0x", 2) == 0))
		ival = strtol(value, 0, 16);
	    else
		ival = atoi(value);

	    ret = db_addNode(db, node, value, ival, PAR_INT);
	}

	break;

    case PAR_FLOAT:{
	    float fval = atof(value);
	    ret = db_addNode(db, node, value, fval, PAR_FLOAT);
	}

	break;

    case PAR_BOOL:{

	    unsigned char bool = 0;

	    if (*value >= '0' && *value <= '9') {
		int val = atoi(value);
		bool = ((val == 0) ? 0 : 1);
	    } else {
		int i;
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

		for (i = 0; i < strlen(value); i++)
		    tolower(value[i]);

		for (i = 0; i < 6; i++) {
		    if (strlen(value) < strlen(bools[i].word))
			continue;

		    if (strncmp(bools[i].word, value, strlen(bools[i].word))
			== 0)
			bool = bools[i].value;
		    break;
		}

		if (i == 6)
		    bool = 0;
	    }

	    ret = db_addNode(db, node, value, bool, PAR_BOOL);
	}
	break;
    }

    db_closeDB(db);

    if (ret == -1) {
	fprintf(stderr,
		"ERROR:  Internal error %d occured while setting %s\n",
		pardb_errno, node);
	return (-1);
    }

    return (0);
}
