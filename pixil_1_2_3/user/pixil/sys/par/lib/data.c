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


/* Given a new data block, this will format the block for inital use */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <par/pardb.h>
#include "database.h"

#ifdef DEBUG
#define DPRINT(str, args...) printf("DEBUG: " str, ## args)
#else
#define DPRINT(str, args...)
#endif

#define GET_DATA_INFO(block)  ((data_info_t *) block)
#define GET_FREE_SPACE(db, offset) (db->blkSize - offset)

void
db_formatData(db_handle * db, void *block, int offset, int size)
{

    void *start = (void *) (block + offset);
    data_info_t *info = (data_info_t *) start;

    bzero(start, size);

    info->ident = DB_DATA_BLOCK;
    info->free = offset + sizeof(data_info_t);
    info->next = 0;
}

/* Add a new data node to the system */

static data_t *
db_newDataNode(db_handle * db, index_t * index, unsigned short size,
	       unsigned short type)
{

    void *block;
    data_t *data;
    data_info_t *info = 0;

    int offset;

#ifdef HAVE_MMAPBUG
    /* We need to align the size so malloc accesses are correct */
    if ((size % 4))
	size += (4 - (size % 4));
#endif

    if (db->dataCache)
	offset = db->dataCache;
    else
	offset = db->data;

    while (offset) {
	DPRINT("Mapping block %d in db_newDataNode()\n",
	       GET_BLOCK(db, offset));

	if (!(block = db_mapBlock(db, GET_BLOCK(db, offset))))
	    return (0);

	info = GET_DATA_INFO(block);

	if (GET_FREE_SPACE(db, info->free) > (size + DATA_ITEM_LEN))
	    goto initData;

	offset = info->next;
    }

    if (!info)
	return (0);
    if (!(info->next = db_addBlock(db)))
	return (0);

    DPRINT("Mapping block %d in db_newDataNode()\n",
	   GET_BLOCK(db, info->next));
    block = db_mapBlock(db, GET_BLOCK(db, info->next));
    if (!block)
	return (0);

    /* Format the new index block */
    db_formatData(db, block, 0, db->blkSize);
    info = GET_DATA_INFO(block);

  initData:
    data = (data_t *) ((ulong) block + info->free);

    data->type = type;
    data->size = (unsigned short) size;

    data->index = db_getFileOffset(db, index);
    index->data = db_getFileOffset(db, data);

    info->free += size + DATA_ITEM_LEN;

    SET_ERRNO(PARDB_SUCCESS);
    return (data);
}

static data_t *
db_getDataNode(db_handle * db, int offset)
{

    void *block;

    if (!offset)
	return (0);

    DPRINT("Mapping block %d in db_getDataNode()\n", GET_BLOCK(db, offset));

    block = db_mapBlock(db, GET_BLOCK(db, offset));

    if (!block)
	return (0);


    SET_ERRNO(PARDB_SUCCESS);

    return ((data_t *) (block + GET_BLOCK_OFFSET(db, offset)));
}

/* Given a start and a offset, move the rest of the data block */

static void
db_moveData(db_handle * db, void *block, int start, int offset)
{

    int ptr = start;

    data_info_t *info = GET_DATA_INFO(block);

    int length;
    void *src, *dest;

    /* First, go through and update all of the index blocks */
    ptr = GET_BLOCK_OFFSET(db, start);

    while (ptr < info->free) {
	data_t *local = (data_t *) (block + ptr);
	db_setIndexDataOffset(db, local->index,
			      GET_BLOCK(db, start) + ptr + offset);
	ptr += (local->size + DATA_ITEM_LEN);
    }

    length = info->free - GET_BLOCK_OFFSET(db, start);
    src = (void *) (block + GET_BLOCK_OFFSET(db, start));
    dest = (void *) (block + GET_BLOCK_OFFSET(db, start) + offset);

    /* Now, actually move the block */
    memmove(dest, src, length);

    /* Update the free block here */
    info->free += offset;
}

int
db_removeData(db_handle * db, int offset)
{

    void *block;
    data_t *data;

    if (!offset) {
	SET_ERRNO(PARDB_BADOFFSET);
	return (-1);
    }
    DPRINT("Mapping block %d in db_removeData()\n", GET_BLOCK(db, offset));
    block = db_mapBlock(db, GET_BLOCK(db, offset));

    if (!block)
	return (-1);

    data = (data_t *) (block + GET_BLOCK_OFFSET(db, offset));

    db_moveData(db, block, offset + data->size + DATA_ITEM_LEN,
		-(data->size + DATA_ITEM_LEN));

    SET_ERRNO(PARDB_SUCCESS);
    return (0);
}

int
db_setData(db_handle * db, index_t * index, void *value, unsigned short size,
	   unsigned short type)
{

    data_t *data = 0;

    if (!index->data) {
	if (!(data = db_newDataNode(db, index, size, type)))
	    return (-1);
    } else {

	/* Try to modify the existing node */
	if (!(data = db_getDataNode(db, index->data)))
	    return (-1);

	if ((size > data->size)) {

	    int diff = size - data->size;
	    void *block = BLOCK_POINTER_START(db, data);
	    data_info_t *info = GET_DATA_INFO(block);

	    /* Check to see if we have the appropriate space available */
	    if (GET_FREE_SPACE(db, info->free) > diff) {
		db_moveData(db, block,
			    index->data + data->size + DATA_ITEM_LEN, diff);
		data->size = size;
	    } else {
		/* Delete the current data block */
		db_removeData(db, index->data);
		index->data = 0;

		/* And get a new one */
		data = db_newDataNode(db, index, size, type);
		if (!data)
		    return (-1);
	    }
	}
    }

    if ((data->type != type)) {
	SET_ERRNO(PARDB_BADTYPE);
	return (-1);
    }

    /* Zero out the existing space */

    if (size) {
	bzero((unsigned char *) &data->value, size);
	memcpy(&data->value, value, size);
    }

    SET_ERRNO(PARDB_SUCCESS);
    return (size);
}

int
db_getData(db_handle * db, index_t * index, void *value, unsigned short size,
	   unsigned short *type)
{

    unsigned short copy_size = 0;

    data_t *data = 0;

    if (!index->data) {
	SET_ERRNO(PARDB_NODATA);
	return (-1);
    }

    data = db_getDataNode(db, index->data);

    if (!data) {
	SET_ERRNO(PARDB_NODATA);
	return (-1);
    }

    /* If there is no data, than thats not an error */
    if (!value || !size) {
	SET_ERRNO(PARDB_SUCCESS);
	return (0);
    }

    if (data->size > size)
	copy_size = size;
    else
	copy_size = data->size;

    if (!copy_size) {
	SET_ERRNO(PARDB_SUCCESS);
	return (0);
    }

    memcpy(value, &data->value, copy_size);

    if (type)
	*type = data->type;

    SET_ERRNO(PARDB_SUCCESS);
    return (copy_size);
}

int
db_getDataInfo(db_handle * db, index_t * index, unsigned short *size,
	       unsigned short *type)
{

    data_t *data;

    if (!index->data) {
	SET_ERRNO(PARDB_NODATA);
	return (-1);
    }

    data = db_getDataNode(db, index->data);

    if (!data)
	return (-1);
    if (size)
	*size = data->size;
    if (type)
	*type = data->type;

    SET_ERRNO(PARDB_SUCCESS);
    return (0);
}
