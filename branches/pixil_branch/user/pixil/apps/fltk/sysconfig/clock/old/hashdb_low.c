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



/* System header files */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Local header files */
#include "hashdb_api.h"

/* Typedef, macros, enum/struct/union definitions */


/* Global scope variables */


/* Local scope variables */


/* Static function prototypes */
static int h_get_free_blk(hashdb_t * dbd, size_t dsize);
static int h_makeroot_blk(hashdb_t * dbd);


/*******************************************************************************\
**
**	Function:	int h_addrec()
**	Desc:		Adds a record into the database at the next free location in
**				the block, or forces a new block to be created
**	Accepts:	hashdb_t *dbd = Ptr to the database handle
**				hash_key_t key = Hash key (index into the hash table)
**				h_data_rec *hrec = Ptr to the record to store
**	Returns:	int; 0 on success, -1 on error
**	Note:		This isn't included into the "api" file, because for the 
**				"super quick" hash db normal operation is only search, not add
**
\*******************************************************************************/
int
h_addrec(hashdb_t * dbd, hash_key_t key, h_data_rec * hrec)
{
    char *blkbuf,		/* Block buf */
     *cptr;			/* Generic pointer */
    int blkidx,			/* Index of the block */
      nrec_key,			/* New record key */
      offset;			/* Offset for the record */
    h_blk_hdr *bhdr;		/* ptr a block header */
    h_data_rec *tailrec;	/* tail record */

    /* Get to a block with space */
    blkidx = h_get_free_blk(dbd, HDB_DTAREC_SZ);
    if (blkidx == -1)
	return (-1);

    /* Get the offset for the new record */
    blkbuf = (char *) dbd->blk_ar[blkidx].addr;
    if (dbd->blk_ar[blkidx].blkno == HASH_ROOT_NODE)
	bhdr = (h_blk_hdr *) (blkbuf + HDB_INFHDR_SZ);
    else
	bhdr = (h_blk_hdr *) (blkbuf);

    offset = dbd->blksz - bhdr->free;
    cptr = blkbuf + offset;
    nrec_key = MAKE_DATA_KEY(dbd->blk_ar[blkidx].blkno, offset);
    hrec->next = 0;
    memcpy(cptr, hrec, HDB_DTAREC_SZ);
    bhdr->free -= HDB_DTAREC_SZ;

    /* Handle first add case (or only 1 item) */
    if (dbd->idx_tbl[key].head == dbd->idx_tbl[key].tail
	&& dbd->idx_tbl[key].head == 0) {
	dbd->idx_tbl[key].head = dbd->idx_tbl[key].tail = nrec_key;
    } /* end of if */
    else {
	h_get_rec(dbd, dbd->idx_tbl[key].tail, &tailrec);
	tailrec->next = nrec_key;
	dbd->idx_tbl[key].tail = nrec_key;
    }				/* end of else */

    return 0;
}				/* end of h_addrec() */

/*******************************************************************************\
**
**	Function:	int h_get_free_blk()
**	Desc:		Searches through the file block at a time, looking for the first
**				block with enough free space to hold a record of size, and maps
**				that block
**	Accepts:	hashdb_t *dbd = Ptr to the hash db handle
**				size_t size = Size value
**	Returns:	int; -1 on error, >= 0 indicating the mapped index for the block
**
\*******************************************************************************/
static int
h_get_free_blk(hashdb_t * dbd, size_t size)
{
    char *buf[HDB_BLKHDR_SZ];	/* Buffer */
    int i,			/* Loop iterator */
      offset;			/* Offset to skip */
    h_blk_hdr *bhdr = 0;	/* Ptr to the block header */

    /* Start looking */
    if (dbd == NULL)
	return (-1);

    /* Look in the root node first */
    for (i = 0; i < dbd->nblk; i++) {
	offset = i * dbd->blksz;
	if (offset == 0)
	    offset = HDB_INFHDR_SZ;
	lseek(dbd->fd, offset, SEEK_SET);
	if (read(dbd->fd, buf, HDB_BLKHDR_SZ) <= 0)
	    return (-1);
	bhdr = (h_blk_hdr *) buf;
	if (bhdr->free >= size)
	    return (h_map_block(dbd, i));
    }				/* end of for */

    /* if here, there were no free blocks, allocate a new one */
    return (h_init_new_blk(dbd, bhdr->id));
}				/* end of h_get_free_blk() */

/*******************************************************************************\
**
**	Function:	int h_get_rec
**	Desc:		Returns a ptr the index table record for the specified key
**	Accepts:	hashdb_t *dbd = Ptr to the database handle
**				data_key_t key = exact key location
**				h_data_rec **rec = Ptr to the record
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
int
h_get_rec(hashdb_t * dbd, data_key_t key, h_data_rec ** rec)
{
    char *blkbuf;		/* Char block buffer */
    short blidx,		/* block index */
      blno,			/* Block number */
      offset;			/* Offset */

    /* Validate the parameters */
    if (rec == NULL || dbd == NULL) {
	errno = EINVAL;
	return (-1);
    }

    /* end of if */
    /* Get the record */
    blno = GET_DATA_BLOCK(key);
    offset = GET_DATA_OFFSET(key);

    /* Map this block */
    if ((blidx = h_map_block(dbd, blno)) == -1)
	return (-1);

    blkbuf = (char *) dbd->blk_ar[blidx].addr + offset;
    *rec = (h_data_rec *) blkbuf;
    return (0);
}				/* end of h_get_rec() */

/*******************************************************************************\
**
**	Function:	int h_init_new_blk()
**	Desc:		Allocates a new block, and maps it, updating all of the necessary
**				block headers, etc
**	Accepts:	hashdb_t *dbd = Ptr Hash db handle
**				int p_idx = parent blocks index
**	Returns:	int; idx into the mapped array for this block; -1 on error
**
\*******************************************************************************/
int
h_init_new_blk(hashdb_t * dbd, int p_idx)
{
    char *newbuf,		/* New buffer */
     *pbuf;			/* Parents buffer */
    int rc;			/* Return code */
    long fpos;			/* File position */
    h_blk_hdr *parent,		/* Ptr to the parents blk header */
     *child;			/* Ptr to the childs blk header */

    /* Grow the file */
    fpos = lseek(dbd->fd, 0, SEEK_END);
    lseek(dbd->fd, dbd->blksz, SEEK_END);
    if (write(dbd->fd, "", 1) == -1) {
	lseek(dbd->fd, 0, SEEK_END);
	return (-1);
    }
    /* end of if */
    dbd->fsize += dbd->blksz;
    dbd->nblk = (dbd->fsize / dbd->blksz) - 1;

    if ((rc = h_map_block(dbd, dbd->nblk)) == -1) {
	ftruncate(dbd->fd, fpos);
	return (-1);
    }				/* end of if */
    dbd->nblk++;

    /* Special handling for the root node */
    if (p_idx != HASH_ROOT_NODE - 1) {
	pbuf = (char *) dbd->blk_ar[p_idx].addr;
	newbuf = (char *) dbd->blk_ar[rc].addr;
	if (p_idx == HASH_ROOT_NODE)
	    parent = (h_blk_hdr *) (pbuf + HDB_INFHDR_SZ);
	else
	    parent = (h_blk_hdr *) pbuf;
	child = (h_blk_hdr *) newbuf;
	child->id = dbd->nblk - 1;
	child->free = dbd->blksz - HDB_BLKHDR_SZ;
	child->parent = parent->id;
	child->peer = -1;
	parent->peer = child->id;
    }
    /* end of if */
    return (rc);
}				/* end of h_init_new_blk() */

/*******************************************************************************\
**
**	Function:	int h_init_new_db()
**	Desc:		Initializes a new database
**	Accepts:	char *name = File name
**				int blocksize = blocksize of each block
**				int mxhashidx = Maximum index in the hash table to create
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
int
h_init_new_db(char *name, int blocksize, int mxhashidx)
{
    char *buf;			/* Dynamic buffer */
    int db_fd = -1,		/* Database file descriptor */
      perm,			/* Create permissions */
      rc;			/* Return code */
    long pagesz;		/* Page size */
    hashdb_t hashrec;		/* Hash database handle */

    /* Make sure the blocksize is page aligned */
    pagesz = sysconf(_SC_PAGESIZE);
    if (pagesz == -1) {
	/* Blocksize is left alone? */
	;
    } /* end of if */
    else if ((blocksize % pagesz)) {
	/* Do something to align the block size */
	blocksize = ((blocksize + pagesz) / pagesz * pagesz);
    }

    /* end of else-if */
    /* First, check to make sure that the index record will fit on 1 block!!! */
    if ((blocksize - (HDB_INFHDR_SZ + HDB_IDXHDR_SZ + HDB_BLKHDR_SZ)) <
	(mxhashidx * HDB_IDXHDR_SZ)) {
	/* Not enough space for the index to fit.... */
	errno = E2BIG;
	return (-1);
    }
    /* end of if */
    buf = (char *) calloc(blocksize, sizeof(char));
    if (!buf)
	return (-1);

    perm = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    /* open the file, and return an EEXISTS if already exists */
    if ((db_fd = open(name, O_RDWR)) > -1) {
	/* File already exists....see if it is a database file */
	if ((rc = read(db_fd, buf, HDB_INFHDR_SZ)) <= 0) {
	    /* Error reading from file, unlink it, and then create it */
	    close(db_fd);
	    unlink(name);
	    db_fd = -1;
	} /* end of if */
	else {
	    /* Data was read */
	    h_info_hdr *inf_hdr = (h_info_hdr *) buf;	/* Info header of the file */

	    if (inf_hdr->magic == HASH_MAGIC) {
		errno = EEXIST;
		close(db_fd);
		free(buf);
		return (-1);
	    }			/* end of if */
	    close(db_fd);
	    unlink(name);
	    db_fd = -1;
	}			/* end of else */
    }				/* end of if */
    free(buf);

    if (db_fd == -1
	&& (db_fd = open(name, O_RDWR | O_CREAT | O_TRUNC, perm)) == -1) {
	/* Could not create the file */
	return (db_fd);
    }				// end of if 

    /* Grow the file to the block size, and initialize the root block */
    memset(&hashrec, 0, HDB_DBHND_SZ);
    hashrec.fd = db_fd;
    hashrec.blksz = blocksize;
    hashrec.idxsz = mxhashidx;
    hashrec.mode = O_RDWR;
    rc = h_makeroot_blk(&hashrec);

    close(db_fd);
    if (rc == -1)
	unlink(name);

    return (rc);
}				/* End of h_init_new_db() */


/*******************************************************************************\
**
**	Function:	int h_makeroot_blk()
**	Desc:		Makes the root block on the newly created file
**	Accepts:	hashdb_t *dbd = Ptr to the hashed db record
**	Returns:	int; 0 on success, -1 on error (errno is set)
**
\*******************************************************************************/
static int
h_makeroot_blk(hashdb_t * dbd)
{
    char *rootbuf;		/* Dynamic buffer for the root */
    int bufsz,			/* Buffer size */
      rc;			/* Result code */
    h_info_hdr *infh;		/* Ptr into info header */
    h_blk_hdr *blkh;		/* Ptr to the blkh header */
    h_idx_hdr *idxh;		/* Ptr into the index header */

    /* Make the block */
    if ((rc = h_init_new_blk(dbd, HASH_ROOT_NODE - 1)) == -1) {
	return (-1);
    }

    /* end of if */
    /* Allocate the buffer */
    bufsz = HDB_INFHDR_SZ + HDB_BLKHDR_SZ + HDB_IDXHDR_SZ +
	(dbd->idxsz * HDB_IDXREC_SZ);

    /* Figure out how to write this out (either via map or file io) */
    rootbuf = (char *) dbd->blk_ar[rc].addr;

    infh = (h_info_hdr *) rootbuf;
    blkh = (h_blk_hdr *) (rootbuf + HDB_INFHDR_SZ);
    idxh = (h_idx_hdr *) (rootbuf + HDB_INFHDR_SZ + HDB_BLKHDR_SZ);

    /* Fill in the info header stuff */
    infh->magic = HASH_MAGIC;
    infh->blksz = dbd->blksz;
    infh->nblk = 0;

    /* Fill in the block header stuff */
    blkh->id = HASH_ROOT_NODE;
    blkh->free = dbd->blksz - bufsz;
    blkh->parent = -1;
    blkh->peer = 0;

    /* Fill in the index header */
    idxh->idx_size = dbd->idxsz * HDB_IDXREC_SZ;
    idxh->idx_max_val = dbd->idxsz;

    munmap(dbd->blk_ar[rc].addr, dbd->blksz);

    return (rc);
}				/* end of h_makeroot_blk() */


/*******************************************************************************\
**
**	Function:	int h_map_block()
**	Desc:		Maps the requested block into memory
**	Accepts:	hashdb_t *dbd = Ptr to the database handle record
**				int blkno = Number of the block to map
**	Returns:	int; -1 on error, or the index into the array of the mapped
**				block
**
\*******************************************************************************/
int
h_map_block(hashdb_t * dbd, int blkno)
{
    int i,			/* Loop iterator */
      mmode = 0,		/* Map mode */
      rc;			/* Result code */

    /* Check the sanity of the block */
    if ((blkno * dbd->blksz) >= dbd->fsize) {
	errno = EINVAL;
	return (-1);
    }

    /* end of if */
    /* Check to see if this block is already mapped */
    for (i = 0; i < dbd->mapcnt; i++) {
	if (dbd->blk_ar[i].blkno == blkno)
	    return (i);
    }				/* end of for */

    /* Map this */
    switch (dbd->mode) {
    case O_RDONLY:
	mmode = PROT_READ;
	break;
    case O_WRONLY:
	mmode = PROT_WRITE;
	break;
    case O_RDWR:
	mmode = PROT_READ | PROT_WRITE;
	break;
    }				/* end of switch */

    if ((dbd->mapcnt + 1) >= dbd->mapsz) {
	if (dbd->mapsz == 0) {
	    if ((dbd->blk_ar =
		 (h_blk_map_t *) calloc(5, HDB_MAPREC_SZ)) == NULL)
		return (-1);
	} /* end of if */
	else {
	    /* Need to reallocate */
	    h_blk_map_t *tmp;

	    tmp =
		(h_blk_map_t *) realloc(dbd->blk_ar,
					((dbd->mapsz + 5) * HDB_MAPREC_SZ));
	    if (!tmp)
		return (-1);
	    dbd->blk_ar = tmp;
	}			/* end of else */
	dbd->mapsz += 5;
    }

    /* end of if */
    /* Seek to this area? */
    //      lseek(dbd->fd, blkno * dbd->blksz, SEEK_SET);
    dbd->blk_ar[dbd->mapcnt].addr =
	mmap(0, dbd->blksz, mmode, MAP_SHARED, dbd->fd, dbd->blksz * blkno);
    if (dbd->blk_ar[dbd->mapcnt].addr == MAP_FAILED) {
	rc = -1;
    }				// end of if
    else {
	dbd->blk_ar[dbd->mapcnt].blkno = blkno;
	rc = dbd->mapcnt++;
    }				// end of else

    return (rc);
}				/* end of h_map_block() */
