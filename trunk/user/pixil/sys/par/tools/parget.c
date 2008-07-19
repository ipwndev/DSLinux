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

void
print_usage(void)
{
    printf("\n-------\n");
    printf("parget [-h] -d <DB file> <node>\n\n");

    printf("[-h, --help] - print this screen\n");
    printf("[-d, --db] - The database to read from\n");
    printf("-------\n\n");
}

#define HAVE_DB_FILE 0x1
#define LIST_NODE 0x2

void
printValue(int type, void *value)
{

    switch (type) {

    case PAR_TEXT:
	printf("%s\n", (char *) value);
	break;

    case PAR_INT:
	printf("%d\n", *((int *) value));
	break;

    case PAR_FLOAT:
	printf("%f\n", *((float *) value));

    case PAR_BOOL:
	if (*((int *) value))
	    printf("TRUE\n");
	else
	    printf("FALSE\n");
	break;

    default:
	printf("\n");
	break;
    }
}

int
doStandard(db_handle * db, char *node)
{

    char *value;
    unsigned short type;
    int size = db_getDataSize(db, node);

    if (size == -1) {
	fprintf(stderr, "ERROR:  Internal error %d occured\n", pardb_errno);
	return (-1);
    }

    if (!size)
	return (0);

    /* The size will have the NULL already added to it */

    value = alloca(size);
    bzero(value, size);

    if (db_findNode(db, node, value, size, &type) == -1) {

	if (pardb_errno == PARDB_NOTFOUND)
	    fprintf(stderr, "ERROR:  Node %s was not found\n", node);
	else
	    fprintf(stderr, "ERROR:  Internal error %d occured\n",
		    pardb_errno);

	return (-1);
    }

    printValue(type, value);
    return (0);
}

int
doWildcard(db_handle * db, char *node)
{

    char **list;

    char *p;
    char *parent = alloca(strlen(node) + 1);

    int count, i;

    if (!parent)
	return (-1);
    strcpy(parent, node);

    p = strrchr(parent, '.');
    if (!p)
	return (-1);		/* We don't handle toplevel search yet */

    *p = 0;

    count = db_getChildCount(db, parent);

    if (count == -1)
	return (-1);
    if (count == 0)
	return (0);

    list = (char **) calloc(sizeof(char *) * count, 1);

    if (db_getChildList(db, parent, list, count) <= 0)
	return (-1);

    for (i = 0; i < count; i++) {
	char *child;

	child = alloca(strlen(parent) + strlen(list[i]) + 2);
	sprintf(child, "%s.%s", parent, list[i]);

	printf("%s: ", child);

	doStandard(db, child);
    }

    for (i = 0; i < count; i++)
	if (list[i])
	    free(list[i]);

    free(list);

    return (0);
}

int
main(int argc, char **argv)
{

    db_handle *db;

    unsigned char flags = 0;

    char *database_filename = 0;
    char *node;

    while (1) {
	signed char c;
	int option_index;

	static struct option long_options[] = {
	    {"help", 0, 0, 'h'},
	    {"db", 1, 0, 'd'},
	    {"list", 0, 0, 'l'},
	    {0, 0, 0, 0}
	};

	c = getopt_long(argc, argv, "d:h", long_options, &option_index);
	if (c == -1)
	    break;

	switch (c) {
	case 'h':
	    print_usage();
	    return (0);

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

    if (optind < argc) {
	node = alloca(strlen(argv[optind]) + 1);
	strcpy(node, argv[optind]);
    } else {
	fprintf(stderr, "ERROR: You must specific a node to search for\n");
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
    db = db_openDB(database_filename, PAR_DB_MODE_RDONLY);
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

    /* We have the undocumented pleasure of allowing a wildcard as the leaf node */
    if (node[strlen(node) - 1] == '*')
	doWildcard(db, node);
    else
	doStandard(db, node);



    db_closeDB(db);
    return (0);
}
