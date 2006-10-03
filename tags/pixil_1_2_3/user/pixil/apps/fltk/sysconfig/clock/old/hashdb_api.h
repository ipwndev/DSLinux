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



#ifndef		HASHDB_API_INCLUDED
#define		HASHDB_API_INCLUDED		1

/* Protect against C++ name mangling */
#ifdef		__cplusplus
extern "C"
{
#endif

/* System header files */
#include <fcntl.h>
#include <sys/types.h>

/* Local header files */


/* Typedef, macros, enum/struct/union definitions */

/* Macros for datakey, block number, data offset handling */
#define		MAKE_DATA_KEY(b,o)		( (o) | (b << 16))
#define		GET_DATA_BLOCK(k)		( (k >> 16) )
#define		GET_DATA_OFFSET(k)		( (k & 0x0000FFFF) )

#define		HASH_MAGIC		0x2324	/* Pulled this out of my arse */
#define		HASH_BLK_SIZE	4096
#define		HASH_ROOT_NODE	0


/* Modes for h_getrecord() */
#define		HDB_KEY_HASH	0
#define		HDB_KEY_DATA	1
    typedef unsigned long data_key_t;
    typedef unsigned short hash_key_t;
    typedef unsigned long hdb_key_t;
//typedef data_key_t    unsigned long;
//typedef hash_key_t    unsigned short;
//typedef key_t         unsigned long;


    typedef struct h_info_hdr
    {
	unsigned short magic,	/* Magic number */
	  blksz,		/* Max block number */
	  nblk;			/* Number of blocks */
	char pad[2];		/* Padded to 4byte alignment */
    }
    h_info_hdr;
#define			HDB_INFHDR_SZ	(sizeof(h_info_hdr))

    typedef struct h_idx_hdr
    {
	unsigned short idx_size,	/* size of the index */
	  idx_max_val;		/* Max hash index */
    }
    h_idx_hdr;
#define			HDB_IDXHDR_SZ	(sizeof(h_idx_hdr))

    typedef struct h_idx_rec
    {
	data_key_t head;	/* data key for the head of the list */
	data_key_t tail;	/* Data key for the tail of the list */
    }
    h_idx_rec;
#define			HDB_IDXREC_SZ	(sizeof(h_idx_rec))

    typedef struct h_blk_hdr
    {
	unsigned short id,	/* Block id */
	  free,			/* Space free */
	  parent,		/* Parent block */
	  peer;			/* peer block */
    }
    h_blk_hdr;
#define			HDB_BLKHDR_SZ	(sizeof(h_blk_hdr))

    typedef struct h_data_rec
    {
	char city_name[20 + 1],	/* City name */
	  reg[2 + 1],		/* Region */
	  zinfo[2 + 5 + 1];	/* Timezone (2Reg/5city) */
	signed short lat,	/* Latitude */
	  lon;			/* Longitude */
	data_key_t next;	/* idx to the next record */
    }
    h_data_rec;
#define			HDB_DTAREC_SZ	(sizeof(h_data_rec))

    typedef struct
    {
	int blkno;		/* Block number */
	caddr_t *addr;		/* Address */
    }
    h_blk_map_t;
#define			HDB_MAPREC_SZ	(sizeof(h_blk_map_t))

    typedef struct
    {
	int fd,			/* File descriptor for the database file */
	  fsize,		/* database file size */
	  nblk,			/* Number of blocks */
	  blksz,		/* Size of blocks */
	  mapcnt,		/* Number of blocks mapped */
	  mapsz,		/* size of the dynamic array */
	  idxsz,		/* Number of entries in the index */
	  mode;			/* File access mode */
	h_idx_rec *idx_tbl;	/* Ptr to the index table */
	h_blk_map_t *blk_ar;	/* Mapped block array */
    }
    hashdb_t;
#define			HDB_DBHND_SZ	(sizeof(hashdb_t))

    int h_addrec(hashdb_t * dbd, hash_key_t key, h_data_rec * rec);
    void h_closedb(hashdb_t * dbd);
    int h_getrecord(int mode, hashdb_t * dbd, hdb_key_t key,
		    h_data_rec * rec);
    int h_init_new_db(char *name, int blocksize, int mxhashidx);
    int h_init_new_blk(hashdb_t * dbd, int bl_no);
    int h_map_block(hashdb_t * dbd, int blkno);
    hashdb_t *h_opendb(char *name, int perms);


#ifdef		__cplusplus
}
#endif

#endif				/*      HASHDB_API_INCLUDED     */
