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


#ifndef _PARDB_H_
#define _PARDB_H_

/* Database access modes */

#define PAR_DB_MODE_RDONLY 0
#define PAR_DB_MODE_RW     1

/* Block modes */
#define PAR_DB_READ_BLOCK  0
#define PAR_DB_WRITE_BLOCK 1

/* Database creation modes */

#define PAR_DB_NORMAL     0
#define PAR_DB_EXCLUSIVE  1

/* Data types */

#define PAR_NONE  0
#define PAR_TEXT  1
#define PAR_INT   2
#define PAR_FLOAT 3
#define PAR_BOOL  4
#define PAR_COLOR 5		/* Really an unsigned long */

/* Error codes */
#define PARDB_SUCCESS    0
#define PARDB_MEMERR     1
#define PARDB_IOERR      2
#define PARDB_NOTFOUND   3
#define PARDB_BADOFFSET  4
#define PARDB_BADSIZE    5
#define PARDB_BADTYPE    6
#define PARDB_BADMODE    7
#define PARDB_BADDB      8
#define PARDB_BADBLOCK   9
#define PARDB_BADMAP     10
#define PARDB_BADHANDLE  11
#define PARDB_BADNODE    12
#define PARDB_NODATA     13
#define PARDB_DATAEXISTS 14
#define PARDB_NOFILE     15
#define PARDB_FILEEXISTS 16
#define PARDB_FILEACCESS 17
#define PARDB_FILEERR    18
#define PARDB_BLOCKINUSE 19

/* Global errno */
extern int pardb_errno;

/* Macro to easily set the errno */
#define SET_ERRNO(mode) (pardb_errno = mode)

/* Data structures */

#define PAR_DB_KEYWORD_SIZE 19

typedef struct tree_struct
{
    char keyword[PAR_DB_KEYWORD_SIZE + 1];

    unsigned short size;
    unsigned short type;
    void *data;

    struct tree_struct *peer;
    struct tree_struct *child;
}
tree_t;

typedef struct
{
    unsigned char usage;
    unsigned char mode;

    void *addr;
}
db_map_t;

typedef struct
{
    int fd;			/* Database file descriptor   */
    int length;			/* Total size of the database */

    unsigned short blkSize;	/* The size of each of the blocks */

    unsigned char access;
    unsigned char mode;

    unsigned long index;	/* Pointer to the first index block */
    unsigned long data;		/* Pointer to the first data block */

    int dataCache;		/* The last block that we succesfully added */

    int mapSize;		/* Number of blocks that have been mapped */
    db_map_t *map;		/* An hash array of mapped blocks */
}
db_handle;

/* Data management */

int db_addNode(db_handle * db, char *name, void *data,
	       unsigned short size, unsigned short type);
int db_findNode(db_handle * db, char *name, void *data,
		unsigned short size, unsigned short *type);

int db_nodeExists(db_handle * db, char *name);

int db_delNode(db_handle * db, char *name);

int db_getFirstChild(db_handle * db, char *parent, char *name, int size);
int db_getNextChild(db_handle * db, char *parent, char *child, char *name,
		    int size);

int db_getChildCount(db_handle * db, char *parent);
int db_getChildList(db_handle * db, char *parent, char **list, int maxsize);

/* Database functions */

db_handle *db_newDB(char *filename, int mode);
db_handle *db_openDB(char *filename, unsigned short mode);
void db_closeDB(db_handle * db);

char *db_getDefaultDB(void);
int db_getDataSize(db_handle * db, char *node);

/* Tree functions */

tree_t *db_newTree(tree_t ** head);
void db_freeTree(tree_t * tree);

int db_loadTree(db_handle * database, tree_t * head);
int db_saveTree(db_handle * database, tree_t * head);

#endif
