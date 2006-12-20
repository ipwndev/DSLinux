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



/* Feature test switches */


/* System header files */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

/* Local header files */
#include "hashdb_api.h"


/* Typedef, macros, enum/struct/union definitions */


/* Global scope variables */


/* File scope variables */


/* Static function protoypes */


/*******************************************************************************\
**
**	External function definitions
**
\*******************************************************************************/

/*******************************************************************************\
**
**	Function:	void h_closedb()
**	Desc:		Closes the database file (and unmaps/unallocates) all memory
**	Accepts:	hashdb_t *dbd = Ptr to the database descriptor
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
h_closedb(hashdb_t * dbd)
{
    int i;			/* Loop iterator */

    if (dbd != NULL) {
	/* Unmap all datafiles */
	if (dbd->blk_ar != NULL) {
	    for (i = 0; i < dbd->mapcnt; i++)
		munmap(dbd->blk_ar[i].addr, dbd->blksz);
	    free(dbd->blk_ar);
	}

	/* end of if */
	/* Close the file */
	if (dbd->fd > -1)
	    close(dbd->fd);

	free(dbd);
    }
    /* end of if */
    return;
}				/* end of h_closedb() */

/*******************************************************************************\
**
**	Function:	int h_getrecord()
**	Desc:		Gets the record from the specified location
**	Accepts:	int mode = Type of key
**					HDB_KEY_HASH = hash key (look up in table)
**					HDB_KEY_DATA = data key 
**				hashdb_t *dbd = Ptr to the db handle
**				key_t key = Key of record to get
**				h_data_rec *rec = Ptr to storage to copy memory to
**	Returns:	int; 0 on success, -1 on error 
**
\*******************************************************************************/
int
h_getrecord(int mode, hashdb_t * dbd, hdb_key_t key, h_data_rec * rec)
{
    h_data_rec *lrec;		/* local record */

    /* Check the parameters */
    if (dbd == NULL || rec == NULL) {
	errno = EINVAL;
	return (-1);
    }

    /* end of if */
    /* Determine the mode */
    if (mode == HDB_KEY_HASH) {
	if (key >= dbd->idxsz) {
	    errno = EINVAL;
	    return (-1);
	}			/* end of if */
	key = dbd->idx_tbl[key].head;
    }
    /* end of if */
    if (key == 0) {
	errno = ENODATA;
	return (-1);
    }

    /* end of if */
    /* Get the record */
    if (h_get_rec(dbd, key, &lrec) == -1) {
	/* error, errno should be set */
	return (-1);
    }
    /* end of if */
    memcpy(rec, lrec, HDB_DTAREC_SZ);
    return (0);
}				/* end of h_getrecord() */

/*******************************************************************************\
**
**	Function:	hashdb_t *h_opendb()
**	Desc:		Opens the database specified by file and maps the first block
**	Accepts:	char *name = Name of the file to open
**				int mode = Mode to open (using std C mode values (O_RDONLY, etc)
**	Returns:	hashdb_t *; Ptr to the handle structure, or NULL on error
**
\*******************************************************************************/
hashdb_t *
h_opendb(char *name, int mode)
{
    char buf[HDB_INFHDR_SZ + HDB_BLKHDR_SZ + HDB_IDXHDR_SZ],	/* buffer to read the file */
     *cp;
    int fd,			/* normal file i/o descriptor */
      rc;			/* Return code */
    struct stat statbuf;	/* stat buffer */
    h_info_hdr *infh;		/* Ptr to the info header */
    h_idx_hdr *idxh;		/* Ptr to the idx header */
    hashdb_t *local;		/* database descriptor */

    /* Validate parameters */
    if (mode == 0)
	mode = O_RDONLY;

    if (name == NULL || *name == '\0') {
	errno = EINVAL;
	return (NULL);
    }

    /* end of if */
    /* Open the file */
    if ((fd = open(name, mode)) == -1)
	return (NULL);

    /* Read the necessary headers */
    if ((rc =
	 read(fd, buf, HDB_INFHDR_SZ + HDB_BLKHDR_SZ + HDB_IDXHDR_SZ)) <= 0) {
	/* Use default errno */
	close(fd);
	return (NULL);
    }
    /* end of if */
    infh = (h_info_hdr *) buf;
    idxh = (h_idx_hdr *) (buf + HDB_INFHDR_SZ + HDB_BLKHDR_SZ);

    if (infh->magic != HASH_MAGIC) {
	close(fd);
	errno = EINVAL;
	return (NULL);
    }
    /* end of if */
    if ((local = (hashdb_t *) calloc(1, sizeof(hashdb_t))) == NULL) {
	close(fd);
	return (NULL);
    }

    /* end of if */
    /* Fill in the struct */
    fstat(fd, &statbuf);
    local->fd = fd;
    local->fsize = statbuf.st_size;
    local->blksz = infh->blksz;
    local->nblk = statbuf.st_size / local->blksz;
    local->idxsz = idxh->idx_max_val;
    local->mode = mode;
    if ((rc = h_map_block(local, 0)) == -1) {
	close(fd);
	free(local);
	return (NULL);
    }

    /* end of if */
    /* Get the index table record */
    cp = (char *) local->blk_ar[rc].addr + HDB_INFHDR_SZ + HDB_BLKHDR_SZ +
	HDB_IDXHDR_SZ;
    local->idx_tbl = (h_idx_rec *) cp;

    return (local);
}				/* end of h_opendb() */

/*******************************************************************************\
**
**	Internally Callable (static) functions
**
\*******************************************************************************/
