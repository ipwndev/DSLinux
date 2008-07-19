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
#include <errno.h>

#include <par/pardb.h>
#include "database.h"

/* A local function used to grab the name off the top of the list */

char *
strip_keyword(char **name)
{
    char *keyword = *name;

    char *ptr = strchr(*name, '.');

    if (!ptr) {
	*name = 0;
    } else {
	*ptr = 0;
	*name = ptr + 1;
	if (!*name)
	    *name = 0;
    }

    return (keyword);
}

/* Recursively add the node (and any non existent parent nodes) to the database */

int
db_recursiveAdd(db_handle * db, index_t * parent, char *name,
		void *data, ushort size, ushort type)
{

    int ret;

    char *pointer = name;
    char *keyword = strip_keyword(&pointer);

    index_t *index = db_getChild(db, parent);
    index_t *prev = 0;

    while (index) {

	if (strcmp(index->keyword, keyword) == 0) {

	    /* If the node already exists, then just set the data and bail */
	    if (!pointer) {
		ret = db_setData(db, index, data, size, type);
	    } else {
		ret = db_recursiveAdd(db, index, pointer, data, size, type);
	    }

	    return (ret);
	}

	prev = index;
	index = db_getPeer(db, index);
    }

    /* If we get this far, the node doesn't exist */

    if (!prev) {
	index = db_addChild(db, parent, keyword);
    } else {
	index = db_addPeer(db, prev, keyword);
    }

    if (!index)
	return (-1);

    if (!pointer) {
	ret = db_setData(db, index, data, size, type);
    } else
	ret = db_recursiveAdd(db, index, pointer, data, size, type);

    return (ret);
}

/* Recursively search the database looking for the named node */

index_t *
db_recursiveFind(db_handle * db, index_t * parent, char *name)
{

    char *pointer = name;
    char *keyword = strip_keyword(&pointer);

    index_t *index = db_getChild(db, parent);

    while (index) {


	if (strcmp(index->keyword, keyword) == 0) {

	    /* Found it, return the node */
	    if (!pointer) {
		SET_ERRNO(PARDB_SUCCESS);
		return (index);
	    }

	    /* Move down a level */
	    return (db_recursiveFind(db, index, pointer));
	}

	index = db_getPeer(db, index);
    }

    SET_ERRNO(PARDB_NOTFOUND);
    return (0);
}

static void
db_recursiveKill(db_handle * db, index_t * parent)
{

    index_t *index = db_getChild(db, parent);
    index_t *next = 0;

    while (index) {
	db_recursiveKill(db, index);

	next = db_getPeer(db, index);
	db_removeIndex(db, index);	/* Kill the index and data */
	index = next;
    }
}

/* Recursively search the database looking for the named node */
/* When found, kill the entire tree below it and remove this  */
/* node from the chain                                        */

int
db_recursiveDel(db_handle * db, index_t * parent, char *name)
{

    char *pointer = name;
    char *keyword = strip_keyword(&pointer);

    index_t *index = db_getChild(db, parent);
    index_t *prev = 0;

    while (index) {
	if (strcmp(index->keyword, keyword) == 0) {

	    /* Once we find the pointer, then we recursively kill everything below it */

	    if (!pointer) {
		db_recursiveKill(db, index);

		if (prev)
		    db_setPeer(db, prev, db_getPeer(db, index));
		else
		    db_setChild(db, prev, db_getPeer(db, index));

		db_removeIndex(db, index);	/* Kill the "parent" of the doomed tree */
		return (0);
	    }

	    /* Move down a level */
	    return (db_recursiveDel(db, index, pointer));
	}

	prev = index;
	index = db_getPeer(db, index);
    }

    SET_ERRNO(PARDB_SUCCESS);
    return (0);
}

void
db_freeDB(db_handle * db)
{
    int i;

    if (!db)
	return;

    for (i = 0; i < db->mapSize; i++)
	db_unmapBlock(db, i);

    free(db->map);
    free(db);
}

db_handle *
db_allocDB(int fd, int bsize, int length)
{

    db_handle *local;

    if (!(local = (db_handle *) calloc(sizeof(db_handle), 1))) {
	SET_ERRNO(PARDB_MEMERR);
	return (0);
    }

    local->fd = fd;
    local->blkSize = bsize;
    local->length = length;

    if (!length) {
	/* Make the inital hash */
	local->map = (db_map_t *) calloc(16 * sizeof(db_map_t), 1);
	if (!local->map) {
	    free(local);
	    SET_ERRNO(PARDB_MEMERR);
	    return (0);
	}

	local->mapSize = 16;
    } else {
	/* Alloate only enough items to hold the database */

	local->map =
	    (db_map_t *) calloc((length / bsize) * sizeof(db_map_t), 1);
	if (!local->map) {
	    free(local);
	    SET_ERRNO(PARDB_MEMERR);
	    return (0);
	}

	local->mapSize = (length / bsize);
    }

    SET_ERRNO(PARDB_SUCCESS);
    return (local);
}
