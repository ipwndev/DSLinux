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

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/mman.h>

#include <par/pardb.h>
#include "database.h"

#define CALC_BLOCK_NUM(db, offset) (offset / db->blkSize)

/* db_mapBlock() */

/* Given an offset into the file, either map it, 
   or return the address of a previous mapping 
*/

void *
db_mapBlock(db_handle * db, int offset)
{

    int bnum = CALC_BLOCK_NUM(db, offset);
    int i, mode;

    if (offset >= db->length) {
	SET_ERRNO(PARDB_BADOFFSET);
	return (0);
    }

    if (bnum >= db->mapSize) {
	db_map_t *temp =
	    (db_map_t *) realloc(db->map, (bnum + 8) * sizeof(db_map_t));

	if (!temp) {
	    SET_ERRNO(PARDB_MEMERR);
	    return (0);
	}
	db->map = temp;

	/* Zero the new items */
	for (i = db->mapSize; i < bnum + 8; i++)
	    bzero(&db->map[i], sizeof(db_map_t));

	db->mapSize = bnum + 8;
    }

    if (db->map[bnum].addr && db->map[bnum].mode == db->mode) {
	db->map[bnum].usage++;
	SET_ERRNO(PARDB_SUCCESS);

	return (db->map[bnum].addr);
    }

    if (db->map[bnum].addr && db->map[bnum].usage) {
	SET_ERRNO(PARDB_BLOCKINUSE);
	return (0);
    }

    /* Map the block */

    if (db->mode == PAR_DB_READ_BLOCK || db->access == PAR_DB_MODE_RDONLY)
	mode = PROT_READ;
    else
	mode = PROT_READ | PROT_WRITE;

    {
	int ret;

	db->map[bnum].addr = (void *) malloc(db->blkSize);

	if (db->map[bnum].addr) {
	    lseek(db->fd, db->blkSize * bnum, SEEK_SET);
	    ret = read(db->fd, (unsigned char *) db->map[bnum].addr, db->blkSize);
	    if (ret <= 0) {
		free(db->map[bnum].addr);
		db->map[bnum].addr = MAP_FAILED;
	    }
	} else
	    db->map[bnum].addr = MAP_FAILED;
    }

    if (db->map[bnum].addr == MAP_FAILED) {
	bzero(&db->map[bnum], sizeof(db_map_t));
	SET_ERRNO(PARDB_BADMAP);
	return (0);
    }

    db->map[bnum].mode = db->mode;
    db->map[bnum].usage = 1;

    SET_ERRNO(PARDB_SUCCESS);
    return (db->map[bnum].addr);
}

void
db_unmapBlock(db_handle * db, int block)
{
    if (db->map[block].usage > 0)
	db->map[block].usage--;

    if (!db->map[block].usage) {
    	if (db->map[block].addr) {
		if ((db->map[block].mode != PAR_DB_READ_BLOCK) && (db->access != PAR_DB_MODE_RDONLY)) {
			int ret;
	    		lseek(db->fd, db->blkSize * block, SEEK_SET);
	    		ret = write(db->fd, db->map[block].addr, db->blkSize);
		}
		free(db->map[block].addr);
    	}
    	bzero(&db->map[block], sizeof(db_map_t));
    }
}

void
db_flushWrite(db_handle * db)
{

    int i;

    /* Flush all of the existing write blocks to the disk */
    /* this may be painful if there are lingering links to the memory addresses */

    for (i = 0; i < db->mapSize; i++) {
	if (db->map[i].mode != PAR_DB_WRITE_BLOCK)
	    continue;

	db->map[i].usage = 0;
	db_unmapBlock(db, i);
    }
}

/* db_mapBlock() */
/* Add a new physical block to the database */

int
db_addBlock(db_handle * db)
{

    int end = db->length;

    if (db->mode != PAR_DB_MODE_RW) {
	SET_ERRNO(PARDB_BADMODE);
	return (-1);
    }

    if (lseek(db->fd, db->blkSize, SEEK_END) == -1) {
	SET_ERRNO(PARDB_IOERR);
	return (-1);
    }

    if (write(db->fd, "", 1) == -1) {
	SET_ERRNO(PARDB_IOERR);
	return (-1);
    }

    db->length += db->blkSize;

    SET_ERRNO(PARDB_SUCCESS);
    return (end);
}

unsigned long
db_getFileOffset(db_handle * db, void *ptr)
{

    int i;
    unsigned long offset;

    if (!db) {
	SET_ERRNO(PARDB_BADHANDLE);
	return (0);
    }

    /* Search the list of blocks and get the correct block */

    for (i = 0; i < db->mapSize; i++) {
	if (!db->map[i].addr)
	    continue;

	if (ptr < db->map[i].addr)
	    continue;

	offset = ((unsigned long) ptr) - ((unsigned long) db->map[i].addr);

	/* If the block pointers match, then return the file offset */

	if (BLOCK_POINTER_START(db, ptr) == db->map[i].addr) {
	    SET_ERRNO(PARDB_SUCCESS);
	    return ((unsigned long) ((i * db->blkSize) + offset));
	}
    }

    SET_ERRNO(PARDB_BADOFFSET);
    return (0);
}

void *
db_getBlockStart(db_handle * db, void *ptr)
{

    int i;
    unsigned long val = (unsigned long) ptr;

    for (i = 0; i < db->mapSize; i++) {
	unsigned long pos;

	if (!db->map[i].addr)
	    continue;
	pos = (unsigned long) db->map[i].addr;
	if (val >= pos && val < pos + db->blkSize)
	    return (db->map[i].addr);
    }

    return (0);
}

unsigned long
db_getBlockOffset(db_handle * db, void *ptr)
{

    int i;
    unsigned long val = (unsigned long) ptr;

    for (i = 0; i < db->mapSize; i++) {
	unsigned long pos;

	if (!db->map[i].addr)
	    continue;

	pos = (unsigned long) db->map[i].addr;
	if (val >= pos && val < pos + db->blkSize)
	    return ((unsigned long) (val - pos));
    }

    return (0);
}

void
db_lockDB(db_handle * db)
{
    struct flock lock = { F_WRLCK, 0, SEEK_SET, 0 };
    fcntl(db->fd, F_SETLKW, &lock);
}

void
db_unlockDB(db_handle * db)
{
    struct flock lock = { F_UNLCK, 0, SEEK_SET, 0 };
    fcntl(db->fd, F_SETLKW, &lock);
}
