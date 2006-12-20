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



/* Given a new index, this will format the block for inital use */

#include <stdlib.h>
#include <string.h>

#include <par/pardb.h>
#include "database.h"

#ifdef DEBUG
#define DPRINT(str, args...) printf("DEBUG: " str, ## args)
#else
#define DPRINT(str, args...)
#endif

#define GET_INDEX_INFO(block) ((index_info_t *) block)

void
db_formatIndex(db_handle * db, void *block, int offset, int size)
{

    index_t *ptr = 0;

    void *start = (void *) ((ulong) block + offset);

    int pos;

    int next;
    index_info_t *info = (index_info_t *) start;

    bzero(start, size);

    info->ident = DB_INDEX_BLOCK;
    info->free = offset + sizeof(index_info_t);
    info->next = 0;

    pos = info->free;
    next = pos + sizeof(index_t);

    while ((size - pos) > sizeof(index_t)) {
	ptr = (index_t *) (block + pos);
	ptr->peer = next;
	pos += sizeof(index_t);
	next += sizeof(index_t);
    }

    if (ptr)
	ptr->peer = 0;
}

static index_t *
db_newIndexNode(db_handle * db, index_t * hint, char *keyword)
{

    void *block;
    index_t *index;
    index_info_t *info = 0;
    int offset;

    if (hint)
	offset = db_getFileOffset(db, hint);
    else
	offset = db->index;

    if (!offset)
	return (0);

    while (offset) {
	int start = GET_BLOCK(db, offset);
	DPRINT("Mapping the block %d in db_newIndexMode()\n",
	       GET_BLOCK(db, offset));

	if (!(block = db_mapBlock(db, GET_BLOCK(db, offset))))
	    return (0);

	if (start == 0)
	    info = GET_INDEX_INFO((void *) ((ulong) block + db->index));
	else
	    info = GET_INDEX_INFO(block);

	if (info->free)
	    goto initIndex;
	offset = info->next;
    }

    if (!(info->next = db_addBlock(db)))
	return (0);


    DPRINT("Mapping block %d in db_newIndexMode()\n",
	   GET_BLOCK(db, info->next));

    block = db_mapBlock(db, GET_BLOCK(db, info->next));

    if (!block)
	return (0);

    /* Format the new index block */
    db_formatIndex(db, block, 0, db->blkSize);

    info = GET_INDEX_INFO(block);

  initIndex:

    index = (index_t *) (block + info->free);
    info->free = index->peer;
    bzero(index, sizeof(index_t));

    strncpy(index->keyword, keyword, PAR_DB_KEYWORD_SIZE);
    index->keyword[PAR_DB_KEYWORD_SIZE] = 0;

    SET_ERRNO(PARDB_SUCCESS);
    return (index);
}

/* Return the first index node in the system (which is always the first index of the first block) */

index_t *
db_getFirstIndex(db_handle * db)
{

    index_t *index;
    void *block;

    if (!db->index) {
	SET_ERRNO(PARDB_BADDB);
	return (0);
    }
    DPRINT("Mapping block %d in db_getFirstIndex()\n",
	   GET_BLOCK(db, db->index));
    block = db_mapBlock(db, GET_BLOCK(db, db->index));
    if (!block)
	return (0);

    index =
	(index_t *) (block + GET_BLOCK_OFFSET(db, db->index) +
		     sizeof(index_info_t));
    if (index->keyword[0] == 0) {
	SET_ERRNO(PARDB_NOTFOUND);
	return (0);
    }

    SET_ERRNO(PARDB_SUCCESS);
    return (index);
}

/* Utility function to get a node living at the specified offset */
static index_t *
db_getIndexNode(db_handle * db, int offset)
{

    int start = GET_BLOCK(db, offset);
    index_info_t *info;
    void *block;

    DPRINT("Mapping block %d in db_getIndexNode()\n", start);
    block = db_mapBlock(db, start);
    if (!block)
	return (0);

    if (start == 0)
	info = GET_INDEX_INFO((void *) ((ulong) block + db->index));
    else
	info = GET_INDEX_INFO(block);

    if (info->ident != DB_INDEX_BLOCK) {
	SET_ERRNO(PARDB_BADBLOCK);
	return (0);
    }

    SET_ERRNO(PARDB_SUCCESS);
    return ((index_t *) (block + GET_BLOCK_OFFSET(db, offset)));
}

/* Get the child of the given index */
index_t *
db_getChild(db_handle * db, index_t * node)
{
    if (!node || !node->child)
	return (0);
    return (db_getIndexNode(db, node->child));
}

index_t *
db_getPeer(db_handle * db, index_t * node)
{
    if (!node || !node->peer) {
	SET_ERRNO(PARDB_NOTFOUND);
	return (0);
    }
    SET_ERRNO(PARDB_SUCCESS);
    return (db_getIndexNode(db, node->peer));
}

/* Set the specified index to be the child of the specified parent */

void
db_setChild(db_handle * db, index_t * node, index_t * index)
{

    if (!node)
	return;

    if (!index)
	node->child = 0;
    else
	node->child = db_getFileOffset(db, index);
}

/* Set the specified index to be the peer of the specified node */

void
db_setPeer(db_handle * db, index_t * node, index_t * index)
{

    if (!node)
	return;

    if (!index)
	node->peer = 0;
    else
	node->peer = db_getFileOffset(db, index);
}

index_t *
db_addFirstIndex(db_handle * db, char *keyword)
{

    index_t *index;
    index_info_t *info;
    int offset;
    void *block, *iptr;

    if (!db->index) {
	SET_ERRNO(PARDB_BADDB);
	return (0);
    }

    DPRINT("Mapping block %d in db_addFirstIndex()\n",
	   GET_BLOCK(db, db->index));
    block = db_mapBlock(db, GET_BLOCK(db, db->index));

    if (!block)
	return (0);

    offset = GET_BLOCK_OFFSET(db, db->index) + sizeof(index_info_t);

    iptr = (void *) ((ulong) block + GET_BLOCK_OFFSET(db, db->index));

    info = GET_INDEX_INFO(iptr);

    index = (index_t *) (block + offset);

    if (index->keyword[0]) {
	SET_ERRNO(PARDB_DATAEXISTS);
	return (0);		/* If the first index is set, return 0 */
    }

    if (info->free != offset) {
	SET_ERRNO(PARDB_BADBLOCK);
	return (0);
    }

    info->free = index->peer;
    bzero(index, sizeof(index_t));

    strncpy(index->keyword, keyword, PAR_DB_KEYWORD_SIZE);
    index->keyword[PAR_DB_KEYWORD_SIZE] = 0;

    SET_ERRNO(PARDB_SUCCESS);
    return (index);
}

index_t *
db_addPeer(db_handle * db, index_t * prev, char *keyword)
{
    index_t *index = db_newIndexNode(db, prev, keyword);

    if (!index) {
	SET_ERRNO(PARDB_NOTFOUND);
	return (0);
    }
    db_setPeer(db, prev, index);

    SET_ERRNO(PARDB_SUCCESS);
    return (index);
}

index_t *
db_addChild(db_handle * db, index_t * parent, char *keyword)
{

    index_t *index = db_newIndexNode(db, parent, keyword);

    if (!index) {
	SET_ERRNO(PARDB_NOTFOUND);
	return (0);
    }
    db_setChild(db, parent, index);

    SET_ERRNO(PARDB_SUCCESS);
    return (index);
}

int
db_setIndexDataOffset(db_handle * db, int offset, int data)
{

    index_t *index = db_getIndexNode(db, offset);
    if (!index) {
	SET_ERRNO(PARDB_NOTFOUND);
	return (-1);
    }

    DPRINT("Setting data for %s to %d\n", index->keyword, data);

    index->data = data;
    SET_ERRNO(PARDB_SUCCESS);
    return (0);
}

int
db_removeIndex(db_handle * db, index_t * index)
{

    index_info_t *info;
    if (!index) {
	SET_ERRNO(PARDB_NOTFOUND);
	return (-1);
    }

    info = GET_INDEX_INFO(BLOCK_POINTER_START(db, index));

    /* Remove the data block */
    db_removeData(db, index->data);

    /* Now, clear out the index */
    bzero(index, sizeof(index_t));

    /* Set the pointer to the next free space */
    index->peer = info->free;
    info->free = BLOCK_POINTER_OFFSET(db, index);

    SET_ERRNO(PARDB_SUCCESS);
    return (0);
}
