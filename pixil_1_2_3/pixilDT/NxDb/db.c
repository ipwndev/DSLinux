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
#ifndef WIN32
#include <unistd.h>
#endif
#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#endif
#include "file.h"

#include "VCMemoryLeak.h"

/*
 * db.c - database & index file open/close/create routines
 *
 * from 9/9/85 mt15b
 */

#ifndef WIN32
#define O_BINARY	0
#endif
static char *str2cat(char *f1, char *ext);

struct buffer *getblk();

extern struct buffer *startbuf, *endbuf;
extern char apflag;

/*
 * create a database or index file
 */
int
dbcreat(struct fildes *fp, char *name)
{
    int fd, ret;

    if ((fd = creat(str2cat(name, fp->filext), 0666)) < 0)
	return (0);
    close(fd);
    apflag = 1;
    ret = dbopen(fp, name);
    apflag = 0;
    return (ret);

}

/*
 * open a database or index file
 */
int
dbopen(struct fildes *fp, char *name)
{
    int fd, off, x;
    struct buffer *bp;

    if ((fd = open(str2cat(name, fp->filext), O_RDWR | O_BINARY)) < 0)
	return (0);
    fp->fildesc = fd;
    fp->fchanged = 0;
    bp = getblk(0, fp);
    if (fp->ftype) {		/* index file */
	((struct ndxfile *) fp)->maxkeys =
	    ((BLKSIZ - 7) / (2 + ((struct ndxfile *) fp)->keylen));
	((struct ndxfile *) fp)->curblk = -1;
	return (1);
    }
    fp->nrecs = get16(&bp->buf[0]);
    fp->currec = 1;

    off = 1;			/* byte 0 is deleted flag */
    for (fd = 0; fp->fieldp[fd].type; fd++) {
	//if( fd >= MAXFIELDS)
	//fatal( x);
	fp->fieldp[fd].offset = off;
	x = fp->fieldp[fd].size;
	switch (fp->fieldp[fd].type) {
	case 'c':
	case 'n':		/* pt only */
	    off += x;
	    break;
	case 'd':
	case 'i':
	    off += x * sizeof(short);
	    break;
	case 'l':
	    off += x * sizeof(long);
	    break;
	}
    }

    x = 1;
    fp->fieldp[fd].offset = off;
    off += x * sizeof(long);

    fp->nfields++;

    fp->nfields = fd;

    fp->recsiz = off;
    if (off > MAXRECSIZ)
	fatal(4);		/* max rec size overflow */
    return (1);
}

/*
 * close and write database or index file
 */
void
dbclose(struct fildes *fp)
{
    struct buffer *bp;

    if (fp->fchanged) {
	bp = getblk(0, fp);
	if (!fp->ftype)
	    put16(&bp->buf[0], fp->nrecs);
	putblk(bp);
    }

    for (bp = startbuf; bp < endbuf; bp++)
	if (bp->fptr == fp) {
	    if (bp->changed)
		putblk(bp);
	    bp->usecnt = 0;
	    bp->fptr = NULL;
	}
    close(fp->fildesc);
}

/*
 * write the database or index file
 */
void
dbsave(struct fildes *fp)
{
    struct buffer *bp;

    if (fp->fchanged) {
	bp = getblk(0, fp);
	if (!fp->ftype)
	    put16(&bp->buf[0], fp->nrecs);
	putblk(bp);
    }

    for (bp = startbuf; bp < endbuf; bp++)
	if (bp->fptr == fp) {
	    if (bp->changed)
		putblk(bp);
	    bp->usecnt = 0;
	    bp->fptr = NULL;
	}
}

static char *
str2cat(char *f1, char *ext)
{
    char *p;
    static char buf[69];

    p = buf;
    while (*f1)
	*p++ = *f1++;
    *p++ = '.';
    while ((*p++ = *ext++) != 0)
	continue;
    return (buf);
}
