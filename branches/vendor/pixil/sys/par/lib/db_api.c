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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <par/pardb.h>
#include "database.h"

/* This is the global error code variable */
int pardb_errno = PARDB_SUCCESS;

/* Add a new node to the system */

int
db_addNode(db_handle * db, char *name, void *data, unsigned short size,
	   unsigned short type)
{

    int ret = 0;

    char *localname, *pointer, *keyword;
    index_t *index = 0, *prev = 0;

    if (db_getAccess(db) != PAR_DB_MODE_RW) {
	SET_ERRNO(PARDB_BADMODE);
	return (-1);
    }

    /* Lock the whole database at this point */
    /* And set the system up for writing     */

    db_lockDB(db);
    db_setMode(db, PAR_DB_WRITE_BLOCK);

    index = db_getFirstIndex(db);

    /* The recursive function will mangle the name */
    /* so we need to save a backup */

    localname = (char *) malloc(strlen(name) + 1);
    pointer = strcpy(localname, name);

    /* The first keyword is the toplevel */
    keyword = strip_keyword(&pointer);

    /* Find the correct toplevel node */

    while (index) {

	if (index->keyword[0] == 0)
	    break;

	if (strcmp(index->keyword, keyword) == 0) {

	    if (pointer)
		ret = db_recursiveAdd(db, index, pointer, data, size, type);
	    else
		ret = db_setData(db, index, data, size, type);

	    free(localname);

	    /* Ensure that all of the open write blocks are freed at this point */

	    db_flushWrite(db);
	    db_setMode(db, PAR_DB_READ_BLOCK);
	    db_unlockDB(db);

	    return (ret);
	}

	prev = index;
	index = db_getPeer(db, index);
    }

    if (!prev)
	index = db_addFirstIndex(db, keyword);
    else
	index = db_addPeer(db, prev, keyword);

    if (!index)
	return (-1);

    if (pointer)
	ret = db_recursiveAdd(db, index, pointer, data, size, type);
    else
	ret = db_setData(db, index, data, size, type);

    free(localname);

    db_flushWrite(db);
    db_setMode(db, PAR_DB_READ_BLOCK);
    db_unlockDB(db);

    return (ret);
}

static index_t *
db_getNode(db_handle * db, char *name)
{

    char *local = name, *keyword;
    index_t *index = db_getFirstIndex(db);

    /* Find the toplevel node */
    keyword = strip_keyword(&local);

    while (index) {
	if (strcmp(index->keyword, keyword) == 0) {

	    if (local)
		return (db_recursiveFind(db, index, local));
	    else {
		SET_ERRNO(PARDB_SUCCESS);
		return (index);
	    }
	}

	index = db_getPeer(db, index);
    }

    SET_ERRNO(PARDB_NOTFOUND);
    return (0);
}

int
db_getChildCount(db_handle * db, char *parent)
{

    index_t *child, *index;
    int count = 0;
    char *localname = (char *) malloc(strlen(parent) + 1);

    strcpy(localname, parent);
    index = db_getNode(db, localname);
    free(localname);

    if (!index)
	return (-1);

    child = db_getChild(db, index);
    while (child) {
	count++;
	child = db_getPeer(db, child);
    }

    return (count);
}

int
db_getChildList(db_handle * db, char *parent, char **list, int maxsize)
{

    int count = 0;
    index_t *child, *index;
    char *localname = (char *) malloc(strlen(parent) + 1);

    strcpy(localname, parent);

    index = db_getNode(db, localname);
    free(localname);

    child = db_getChild(db, index);

    while (child && count < maxsize) {
	list[count] = (char *) calloc(strlen(child->keyword) + 1, 1);
	if (!list[count])
	    return (-1);

	strcpy(list[count], child->keyword);
	child = db_getPeer(db, child);
	count++;
    }

    return (count);
}

int
db_getFirstChild(db_handle * db, char *parent, char *name, int size)
{

    int csize;

    index_t *index, *child;
    char *localname = (char *) malloc(strlen(parent) + 1);
    strcpy(localname, parent);

    index = db_getNode(db, localname);
    free(localname);

    if (!index)
	return (-1);

    child = db_getChild(db, index);
    if (!child)
	return (0);

    if (size > strlen(child->keyword))
	csize = strlen(child->keyword);
    else
	csize = size;

    strncpy(name, child->keyword, csize);
    return (size);
}

int
db_getNextChild(db_handle * db, char *parent, char *child, char *name,
		int size)
{

    index_t *index, *cindex;
    char *localname = (char *) malloc(strlen(parent) + 1);

    strcpy(localname, parent);

    index = db_getNode(db, localname);
    free(localname);

    if (!index)
	return (-1);

    cindex = db_getChild(db, index);

    while (cindex) {
	int csize;

	if (strcmp(cindex->keyword, child) == 0) {
	    cindex = db_getPeer(db, cindex);
	    if (!cindex)
		return (0);
	    if (size > strlen(cindex->keyword))
		csize = strlen(cindex->keyword);
	    else
		csize = size;

	    strncpy(name, cindex->keyword, csize);
	    return (size);
	}

	cindex = db_getPeer(db, cindex);
    }

    return (0);
}

/* Search the database for the specified node */

int
db_findNode(db_handle * db, char *name, void *data,
	    unsigned short size, unsigned short *type)
{

    index_t *index;
    char *localname;

    /* Save the name because we will mangle it */

    localname = (char *) malloc(strlen(name) + 1);
    strcpy(localname, name);

    index = db_getNode(db, localname);
    free(localname);

    if (!index) {
	SET_ERRNO(PARDB_NOTFOUND);
	return (-1);
    }

    return (db_getData(db, index, data, size, type));
}

int
db_nodeExists(db_handle * db, char *name)
{

    index_t *index;
    char *localname;

    /* Save the name because we will mangle it */

    localname = (char *) malloc(strlen(name) + 1);
    strcpy(localname, name);

    index = db_getNode(db, localname);
    free(localname);
    if (!index)
	return (0);
    return (1);
}

db_handle *
db_newDB(char *filename, int open_mode)
{

    int mode = 0;
    int perm = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

    db_info_t *info;
    db_handle *local;
    void *index, *data;

    int fd, offset;

    /* Get the optimal block size for this system */
    int blkSize = getpagesize();

    /* Create the exclusive file, and give it user and group write access */

    if (mode == PAR_DB_EXCLUSIVE)
	mode = O_RDWR | O_CREAT | O_EXCL;
    else
	mode = O_RDWR | O_CREAT | O_TRUNC;

    fd = open(filename, mode, perm);

    /* Set the appropriate error code */

    if (!fd) {
	switch (errno) {
	case EEXIST:
	    SET_ERRNO(PARDB_FILEEXISTS);
	    break;

	case EACCES:
	    SET_ERRNO(PARDB_FILEACCESS);
	    break;

	default:
	    SET_ERRNO(PARDB_FILEERR);
	    break;
	}

	return (0);
    }

    /* Allocate the database structure */

    if (!(local = db_allocDB(fd, blkSize, 0))) {
	close(fd);
	return (0);
    }

    /* When a new database is created, it is always read/write */
    db_setAccess(local, PAR_DB_MODE_RW);

    /* Lock it to be on the safe side */

    db_lockDB(local);
    db_setMode(local, PAR_DB_WRITE_BLOCK);

    /* Create and map the first index block */
    if ((offset = db_addBlock(local)) == -1)
	goto exitNewDB;
    if (!(index = db_mapBlock(local, offset)))
	goto exitNewDB;

    /* Now, add the inital header stuff for the first index block */

    info = (db_info_t *) index;

    info->magic = PARDB_MAGIC;
    info->blksize = blkSize;

    info->index = sizeof(db_info_t);

    if ((info->data = db_addBlock(local)) == -1)
	goto exitNewDB;

    if (!(data = db_mapBlock(local, info->data)))
	goto exitNewDB;

    /* Next, format the first index and data blocks */
    db_formatIndex(local, index, sizeof(db_info_t),
		   blkSize - sizeof(db_info_t));

    db_formatData(local, data, 0, blkSize);

    local->index = info->index;
    local->data = info->data;

    db_flushWrite(local);
    db_setMode(local, PAR_DB_READ_BLOCK);

    db_unlockDB(local);

    SET_ERRNO(PARDB_SUCCESS);
    return (local);

  exitNewDB:

    db_freeDB(local);
    close(fd);

    /* The errno will be set by the appropriate call */
    return (0);
}

db_handle *
db_openDB(char *filename, unsigned short mode)
{

    db_handle *local;
    db_info_t info;

    int fd;

    struct stat statt;

    /* Stat the file, make sure it exists and get a maximum file size */

    if (stat(filename, &statt) < 0) {
	SET_ERRNO(PARDB_NOFILE);
	return (0);
    }

    /* Open the file */
    if (mode == PAR_DB_MODE_RW)
	fd = open(filename, O_RDWR);
    else
	fd = open(filename, O_RDONLY);

    if (!fd) {
	SET_ERRNO(PARDB_FILEERR);
	return (0);
    }

    /* Read the first index info block */

    if (read(fd, &info, sizeof(info)) == -1) {
	SET_ERRNO(PARDB_IOERR);
	close(fd);
	return (0);
    }

    if (info.magic != PARDB_MAGIC) {
	SET_ERRNO(PARDB_BADDB);
	close(fd);
	return (0);
    }

    /* Allocate the database structure */

    if (!(local = db_allocDB(fd, info.blksize, statt.st_size))) {
	close(fd);
	return (0);
    }

    /* Set the pointers to the data */
    local->index = info.index;
    local->data = info.data;

    db_setAccess(local, mode);
    db_setMode(local, PAR_DB_READ_BLOCK);

    SET_ERRNO(PARDB_SUCCESS);
    return (local);
}

void
db_closeDB(db_handle * db)
{

    int i;

    if (!db)
	return;

    /* Close the database */
    /* First step, unmap all the mapped items */

    for (i = 0; i < db->mapSize; i++)
	if (db->map[i].addr)
	    db_unmapBlock(db, i);

    free(db->map);
    close(db->fd);
    free(db);
}

/* Search the database for the specified node */

int
db_delNode(db_handle * db, char *name)
{

    char *localname, *pointer, *keyword;
    index_t *index = 0, *prev = 0;

    if (db_getAccess(db) != PAR_DB_MODE_RW) {
	SET_ERRNO(PARDB_BADMODE);
	return (-1);
    }

    /* Lock the DB while we are doing this */

    db_lockDB(db);
    db_setMode(db, PAR_DB_WRITE_BLOCK);

    index = db_getFirstIndex(db);

    /* Save the name because we will mangle it */

    localname = (char *) malloc(strlen(name) + 1);
    pointer = strcpy(localname, name);

    /* Find the toplevel node */
    keyword = strip_keyword(&pointer);

    while (index) {

	if (strcmp(index->keyword, keyword) == 0) {

	    if (pointer)
		db_recursiveDel(db, index, pointer);
	    else {

		index_t *peer = db_getPeer(db, index);
		db_removeIndex(db, index);

		if (prev)
		    db_setPeer(db, prev, peer);
	    }

	    db_flushWrite(db);
	    db_setMode(db, PAR_DB_READ_BLOCK);
	    db_unlockDB(db);

	    free(localname);
	    return (0);
	}

	prev = index;
	index = db_getPeer(db, index);
    }

    db_flushWrite(db);
    db_setMode(db, PAR_DB_READ_BLOCK);
    db_unlockDB(db);

    free(localname);
    return (0);
}

/* Random utility functions */

char *
db_getDefaultDB(void)
{
    char *ptr = getenv("PARDB");
    return ptr ? ptr : DEFPARDB;
}

int
db_getDataSize(db_handle * db, char *name)
{

    unsigned short size = 0;
    index_t *index;
    char *localname = (char *) malloc(strlen(name) + 1);
    strcpy(localname, name);

    index = db_getNode(db, localname);
    free(localname);

    if (!index)
	return (-1);

    if (db_getDataInfo(db, index, &size, 0) == -1)
	return (-1);
    else
	return (size);
}

tree_t *
db_newTree(tree_t ** head)
{
    *head = tree_newTree();
    return (*head);
}

void
db_freeTree(tree_t * tree)
{
    tree_recursiveFree(tree);
}

/* Load the entire database into a tree */
/* The top node in the tree is just a place holder to unify */
/* all of the various toplevel nodes */

int
db_loadTree(db_handle * db, tree_t * head)
{
    return (tree_recursiveBuild(db, db_getFirstIndex(db), head));
}

/* Save a tree into the database file */

int
db_saveTree(db_handle * db, tree_t * head)
{
    int ret;
    tree_t *child = head->child;

    if (db_getAccess(db) != PAR_DB_MODE_RW) {
	SET_ERRNO(PARDB_BADMODE);
	return (-1);
    }
    if (!child)
	return (-1);

    db_lockDB(db);
    db_setMode(db, PAR_DB_WRITE_BLOCK);

    ret = tree_recursiveSave(db, child, 0);

    db_flushWrite(db);
    db_setMode(db, PAR_DB_READ_BLOCK);
    db_unlockDB(db);

    return (ret);
}
