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
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <unistd.h>
#endif
#include <malloc.h>
#ifdef WIN32
#include <io.h>
#include <memory.h>
#endif
#include "file.h"

#include "VCMemoryLeak.h"
/* 
 * blkio.c - block i/o routines
 *
 * from 8/25/85 mt15b
 */
#define	MIN(a,b)		((a) < (b) ? (a) : (b))
#define	MAX(a,b) 		((a) > (b) ? (a) : (b))
#define dprintf printf
#define MAXBUFS	20		/* max number of in-core buffers */
#define LOCKVAL	32767		/* value of usecnt to lock buffer incore */
struct buffer *startbuf;	/* ptr to first buffer */
struct buffer *endbuf;		/* ptr past last buffer */
char apflag = 0;		/* append mode flag for getblk() */

/* local data*/
static int lockclock = 0;

/* local procs*/
static int bread(int fd, char *buf, int blk);
static int bwrite(int fd, char *buf, int blk);
static int bfilesize(int fd);
static struct buffer *berr(char *str, int blk, struct fildes *fp);

/*
 * read record recno from file fp into recbuf, return 1 if ok
 */
int
getrec(int recno, struct fildes *fp, char *recbuf)
{
    int off, cnt, n, blk;
    struct buffer *bp;
    long offset;
    char *tmpBuf;
    if ((recno == 0))
	return (0);
    cnt = fp->recsiz;
    offset = (recno - 1) * (long) cnt + RECOFF;
    tmpBuf = recbuf;

    do {
	blk = (int) (offset >> BLKSHFT);	/* logical blk # */
	off = (int) (offset & BLKMSK);	/* offset in blk */
	n = umin(BLKSIZ - off, cnt);	/* calc read cnt */
	if ((bp = getblk(blk, fp)) == NULL)
	    return (0);
	memcpy(recbuf, &bp->buf[off], n);
	recbuf += n;
	cnt -= n;
	offset += n;
    } while (cnt != 0);
    fp->flags = get32(&tmpBuf[fp->recsiz - sizeof(long)]);

#if 0
    if (fp->flags) {
	fprintf(stderr, "getrec: %ld\n", fp->flags);
    }
#endif /*  */
    return (!recdeleted(fp));
}


// returns true if the record can be safely overwritten
int
recerased(struct fildes *fp)
{
    return (fp->flags & 0xF0);
}

int
recdeleted(struct fildes *fp)
{

    /*printf("blkio.c recdeleted may need to check more flags\n"); */
    return ((fp->flags & 0xF0) | (fp->flags & 0x01));
}

int
putrec(int recno, struct fildes *fp, char *recbuf)
{
    int off, cnt, n, blk;
    struct buffer *bp;
    long offset;

#ifndef WIN32
    int i;

#endif
    if (recno == 0 || recno > fp->nrecs) {
	fprintf(stderr, "\007*** Putrec: invalid record number %d\n", recno);
	return (0);
    }
    if (recno == APPEND) {	/* check for append mode */
	recno = fp->currec = ++fp->nrecs;
	fp->fchanged = 1;
	apflag = 1;
    }
    cnt = fp->recsiz;
    offset = (recno - 1) * (long) cnt + RECOFF;
    put32(&recbuf[cnt - sizeof(long)], fp->flags);

    do {
	blk = (int) (offset >> BLKSHFT);
	off = (int) (offset & BLKMSK);
	n = umin(BLKSIZ - off, cnt);
	if ((bp = getblk(blk, fp)) == NULL) {
	    return (0);
	}
	memcpy(&bp->buf[off], recbuf, n);
	bp->changed = 1;
	recbuf += n;
	cnt -= n;
	offset += n;
    } while (cnt != 0);
    apflag = 0;
    return (1);
}
struct buffer *
getblk(int blk, struct fildes *fp)
{
    struct buffer *bp, *savbp;
    if (++lockclock >= LOCKVAL)	/* lru timing */
	lockclock = 1;
    savbp = NULL;
    for (bp = startbuf; bp < endbuf; bp++) {
	if (bp->blkno == blk && bp->fptr == fp) {
	    bp->usecnt = lockclock;
	    return (bp);
	}
	if (savbp == NULL || bp->usecnt < savbp->usecnt)
	    savbp = bp;
    }
    if (savbp->usecnt == LOCKVAL)
	return berr("all bufs locked", blk, fp);
    if (savbp->changed) {	/* write buffer if modified */
	putblk(savbp);
    }
    if (!bread(fp->fildesc, savbp->buf, blk)) {
	if (apflag) {
	    memset(savbp->buf, 0, BLKSIZ);
	    savbp->changed = 1;
	} else {
	    return berr("read error", blk, fp);
	}
    }
    savbp->blkno = blk;
    savbp->fptr = fp;
    savbp->usecnt = lockclock;
    return (savbp);
}

void
putblk(struct buffer *bp)
{
    if (bp == NULL)
	return;
    bwrite(bp->fptr->fildesc, bp->buf, bp->blkno);
    bp->changed = 0;
}
struct buffer *
newblk(struct fildes *fp)
{
    struct buffer *bp;
    int blk;
    blk = 0;
    for (bp = startbuf; bp < endbuf; bp++)
	if (bp->fptr == fp)
	    blk = MAX(blk, bp->blkno);
    fprintf(stderr, "newblk: filesize=%d, max incore blk=%d,",
	    bfilesize(fp->fildesc), blk);
    blk = MAX(bfilesize(fp->fildesc), blk + 1);
    fprintf(stderr, "new=%d\n", blk);
    apflag = 1;
    bp = getblk(blk, fp);
    apflag = 0;
    return (bp);
}
struct buffer *
lockb(struct buffer *bp)
{
    if (bp)
	bp->usecnt = LOCKVAL;
    return (bp);
}

void
unlock(struct buffer *bp)
{
    if (++lockclock >= LOCKVAL)	/* lru timing */
	lockclock = 1;
    bp->usecnt = lockclock;
}

void
dbinit(int maxbufs)
{
    register struct buffer *bp;
    if (maxbufs <= 0)
	maxbufs = MAXBUFS;
    while ((startbuf = malloc(sizeof(struct buffer) * maxbufs)) == NULL) {
	if (--maxbufs < 4)
	    fatal(9);		/* no memory */
    }
    endbuf = startbuf + maxbufs;
    for (bp = startbuf; bp < endbuf; bp++) {
	bp->usecnt = 0;
	bp->fptr = NULL;
	bp->changed = 0;
    }
}
static int
bread(int fd, char *buf, int blk)
{
    lseek(fd, (long) blk << 10, 0);
    if (read(fd, buf, 1024) != 1024)
	return (0);
    return (1);
}
static int
bwrite(int fd, char *buf, int blk)
{

#ifndef WIN32
    int i;

#endif
    lseek(fd, (long) blk << 10, 0);
    if (write(fd, buf, 1024) != 1024) {
	return (0);
    }
    return (1);
}


/*
 * return file size in 1K blocks
 */
static int
bfilesize(int fd)
{
#if UNIX
    struct stat sbuf;
    if (fstat(fd, &sbuf) < 0)
	return (0);
    return ((int) (sbuf.st_size / (long) BLKSIZ));
#else
    return ((int) (lseek(fd, 0L, 2) / (long) BLKSIZ));
#endif
}
unsigned int
umin(unsigned int a, unsigned int b)
{
    if (a < b)
	return (a);
    return (b);
}
static struct buffer *
berr(char *str, int blk, struct fildes *fp)
{
    printf("\007*** getblk: %s, blk=%d, file=%s\n", str, blk, fp->filext);
    return (NULL);
}
