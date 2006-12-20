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
 * See http://embedded.centurysoftware.com/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://embedded.centurysoftware.com/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


#ifndef FILE_H_
#define FILE_H_

/* 31 Dec 95*/
/*
 * file.h - file struct defs
 */

/* 875 */
#define MAXRECSIZ 2048		/* maximum record size */
#define BLKSIZ	1024		/* buffer block size */
#define BLKSHFT	10		/* BLKSIZ shift factor */
#define BLKMSK	01777		/* block mask */
#define RECOFF	2		/* offset of first record in block 0 */
#define APPEND	(-1)		/* to append, putrec( APPEND, &file, rbuf) */

#if 0
#define get16(addr)		(*(short *)addr)
#define put16(addr,val)		*(short *)(addr) = (val)
#define get32(addr)		((long)(*(long *)addr))
#define put32(addr,val)		*(long *)(addr) = (val)
#else
int get16(char *addr);
void put16(char *addr, unsigned short val16);
long get32(char *addr);
void put32(char *addr, long val32);
#endif

struct field
{				/* field specifiers */
    char type;			/* field type - Word(integer), Char, Date */
    char size;			/* # field type units */
    short offset;		/* field offset - filled in at open time */
};

struct fildes
{				/* file structure */
    short fildesc;		/* returned by open(), must be offset 0 */
    char fchanged;		/* set if block 0 needs rewrite */
    char ftype;			/* file type: 1=index, 0=database */
    char *filext;		/* file extension */

    char nfields;		/* # fields */
    struct field *fieldp;	/* field type, length */
    struct ndxfile *ndxfp;	/* index file list */
    short nrecs;		/* # records, read from disk */
    short recsiz;		/* record size, calc'd at open time */
    short currec;		/* current record, 1..nrecs */
    char eof;			/* end of file, currec undefined */
    long flags;			/* flags: 0x0 = 1 to mark delete */
};

struct ndxfile
{				/* index file, first 4 fields match fildes */
    short fildesc;		/* returned by open(), must be offset 0 */
    char fchanged;		/* set if block 0 needs rewrite */
    char ftype;			/* file type: 1=index, 0=database */
    char *filext;		/* file extension */

    char keylen;		/* index key size (Char only) */
    short curoff;		/* key # in curblk */
    short curblk;		/* currency leaf node block # */
    short maxkeys;		/* max keys per block =2n */
};

struct buffer
{				/* incore block buffer */
    short blkno;		/* block # */
    struct fildes *fptr;	/* file ptr */
    short usecnt;		/* lru clock value */
    char buf[BLKSIZ];		/* the buffer */
    char changed;		/* modified flag */
};

#define NUSEFILES	3
struct usefile
{				/* inuse file structure */
    struct fildes *fp;		/* use file */
    short recno;		/* buffer record no. */
    char rbuf[MAXRECSIZ];	/* record buffer */
    char uchanged;		/* modified flag */
};

union w
{				/* replace() parm 2 */
    short word;
    char *ptr;
    long lword;
};

/* use.c*/
void use(int fileno, struct fildes *fp, struct ndxfile *nfp);
char *val(int var);
short ival(int var);
long lval(int var);
void replace(int var, union w newval);
void store(void *dst, int var);
int find(int fileno, char *key);
void udelrec(int fileno);
char *fcopy(char *src, char *dst, int len);

/* db.c*/
int dbcreat(struct fildes *fp, char *name);
int dbopen(struct fildes *fp, char *name);
void dbclose(struct fildes *fp);
void dbsave(struct fildes *fp);

/* blkio.c*/
int recerased(struct fildes *fp);
int recdeleted(struct fildes *fp);
int getrec(int recno, struct fildes *fp, char *recbuf);
int putrec(int recno, struct fildes *fp, char *recbuf);
struct buffer *getblk(int blk, struct fildes *fp);
void putblk(struct buffer *bp);
struct buffer *newblk(struct fildes *fp);
struct buffer *lockb(struct buffer *bp);
void unlock(struct buffer *bp);
void dbinit(int maxbufs);
unsigned int umin(unsigned int a, unsigned int b);

/* index.c*/
int search(char *key, struct ndxfile *fp, int mode);
int Delete(char *key, int rec, struct ndxfile *fp);
int next(struct fildes *fp);
int prev(struct fildes *fp);
int gotop(struct fildes *fp);
int gobot(struct fildes *fp);
int gotorec(struct fildes *fp, int rec);

#ifndef fatal
void fatal(int);
#endif

#endif
