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

#include "file.h"
/*
 * index.c - B* tree index file access routines
 *
 * from 11/18/85 mt15b by greg haerr
 */

#define RECNOSIZE	sizeof(short)	/* sizeof record number */
/* leaf & ptr block struct*/
#define PAGETYPE(p)	(p->buf[ 0])	/* Leaf or Ptr, != 0 is ptr */
#define NKEYS(p)	(get16(&p->buf[ 1]))	/* # keys this page */
#define setNKEYS(p,v)	(put16(&p->buf[ 1],v))	/* # keys this page */

/* ptr block struct*/
#define BLK0(p)		(get16(&p->buf[ 3]))	/* blk 0 ptr */
#define setBLK0(p,v)	(put16(&p->buf[ 3],v))	/* blk 0 ptr */
#define PKEY1(p)	(&p->buf[ 5])	/* address of first key, pblk */
#define BLKNO(p)	(get16(&p[ gkeylen]))	/* address of next block at p */
#define setBLKNO(p,v)	(put16(&p[ gkeylen],v))	/* address of next block at p */

/* leaf block struct*/
#define FPTR(p)		(get16(&p->buf[ 3]))	/* forward block ptr */
#define setFPTR(p,v)	(put16(&p->buf[ 3],v))	/* forward block ptr */
#define BPTR(p)		(get16(&p->buf[ 5]))	/* backward block ptr */
#define setBPTR(p,v)	(put16(&p->buf[ 5],v))	/* backward block ptr */
#define LKEY1(p)	(&p->buf[ 7])	/* address of first key, lblk */
#define RECNO(p)	(get16(&p[ gkeylen]))	/* address of recno at p */
#define setRECNO(p,v)	(put16(&p[ gkeylen],v))	/* address of recno at p */
#define KEY1OFF		7	/* offset of key #1 in buf */
#define moveleft(s,d,c)	memcpy(d,s,c)

#define F_INT		02	/* char/integer index */
#define F_MULT		04	/* unique/multiple keys */

struct passup
{				/* key pass-up structure */
    char key[11];		/* key, max len = 10 */
    short pblkno;		/* block # */
};

/* local data*/
static int gkeylen;		/* global key length */
static char *gkey;		/* global key ptr */
static char gkeytyp;		/* key type, 0=char, 1=integer */
static struct ndxfile *gfp;	/* global index file */
static int gmode;		/* global mode, 0 = search only */
static int grec;		/* global rec# */

/* local procs*/
static int isrch(int blkno, struct passup *v);
static int dsrch(int blk, struct passup *v);
static void keycopy(char *src, char *dst);
static int keycomp(char *key);
static void moveright(char *asrc, char *adst, int cnt);
static int ierr(int n);

/*
 * search for key in index file fp, if mode != 0, insert as record #mode if
 * not found
 */
int
search(char *key, struct ndxfile *fp, int mode)
{
    register struct buffer *bp;
    register struct buffer *nbp;
    register char *p;
    register int n;
    struct passup v;
    short kbuf;

    gfp = fp;
    gmode = mode;
    gkeylen = gfp->keylen;
    if ((gkeytyp = (gfp->ftype & F_INT)) != 0) {
	gkey = (char *) &kbuf;
	put16((char *) &kbuf, (int) key);
    } else
	gkey = key;
    n = isrch(0, &v);
    if (gmode == 0)		/* search only */
	return (n);
    if (n) {			/* allocate new root node */
	//fprintf(stderr, "search: root passup key=%9s, blkno=%d\n", v.key, v.pblkno);
	if ((nbp = lockb(newblk((struct fildes *) gfp))) == NULL)
	    return (ierr(0));	/* isrch: new root bp fail */
	if ((bp = getblk(0, (struct fildes *) gfp)) == NULL) {
	    unlock(nbp);
	    return (ierr(1));	/* search: bp getblk fail */
	}
	memcpy(nbp->buf, bp->buf, BLKSIZ);	/* copy block 0 */
	PAGETYPE(bp) = 'P';
	setNKEYS(bp, 1);
	setBLK0(bp, nbp->blkno);
	p = PKEY1(bp);
	keycopy(v.key, p);
	setBLKNO(p, v.pblkno);
	bp->changed = 1;
	if (n == FPTR(nbp)) {
	    bp = getblk(n, (struct fildes *) gfp);
	    setBPTR(bp, nbp->blkno);
	    bp->changed = 1;
	}
	if (n == BPTR(nbp)) {
	    bp = getblk(n, (struct fildes *) gfp);
	    setFPTR(bp, nbp->blkno);
	    bp->changed = 1;
	}
	unlock(nbp);
    }
    return (gmode);
}

/*
 * recursive B* tree search (two level only for now)
 */
static int
isrch(int blkno, struct passup *v)
{
    register struct buffer *bp, *nbp;
    char *p, *ap;
    int n, r;
    struct passup u;

  again:
    if ((bp = getblk(blkno, (struct fildes *) gfp)) == NULL)
	return (ierr(2));	/* isrch: bp getblk fail */
    n = NKEYS(bp);
#ifdef UNIV
    if ((gfp == &FAndx2 || gfp == &FAndx3) && n == RECMAX - 5 && gmode) {
	printf("\007***Warning: 5 activities left, space to continue");
	chrin();
    }
    if ((gfp == &FAndx2 || gfp == &FAndx3) && n > RECMAX)
	return (fatal(8));
#endif
    if (PAGETYPE(bp) == 0) {	/* leaf node */
	p = LKEY1(bp);
	while (n > 0 && keycomp(p) < 0) {
	    p += gkeylen + 2;
	    n--;
	}
	if (gmode == 0) {	/* search only */
	    if ((gfp->ftype & F_MULT) && n <= 0)	/* mult keys */
		if ((blkno = FPTR(bp)) != 0) {
		    /*dprintf( "isrch: mult next blk"); */
		    goto again;
		}
	    if (n <= 0 || keycomp(p) != 0)
		return (0);
	    else {
		gfp->curblk = bp->blkno;
		gfp->curoff = NKEYS(bp) - n + 1;
		return (RECNO(p));
	    }
	}
	if (gfp->maxkeys > NKEYS(bp)) {	/* insert, no split needed */
	    while (n > 0 && keycomp(p) == 0) {
		p += gkeylen + 2;
		n--;
	    }
	    if (n != 0)
		moveright(p, p + gkeylen + 2, n * (gkeylen + 2));
	    keycopy(gkey, p);
	    setRECNO(p, gmode);	/* gmode = recno */
	    setNKEYS(bp, NKEYS(bp) + 1);
	    bp->changed = 1;
	    return (0);		/* no passup */
	} else {		/* insert, split leaf */
	    /*dprintf( "isrch: lnode split\n"); */
	    lockb(bp);
	    if ((nbp = lockb(newblk((struct fildes *) gfp))) == NULL) {
		unlock(bp);
		return (ierr(3));	/* isrch: nbp getblk fail */
	    }
	    r = (gfp->maxkeys + 1) / 2;
	    /* copy right page to new block */
	    memcpy(LKEY1(nbp),
		   LKEY1(bp) + (gkeylen + 2) * (gfp->maxkeys / 2),
		   (gkeylen + 2) * (gfp->maxkeys - (gfp->maxkeys / 2)));
	    setNKEYS(nbp, r);
	    setNKEYS(bp, r);
	    bp->changed = 1;
	    if (n >= r) {	/* insert left page */
		n -= r;
		setNKEYS(bp, NKEYS(bp) + 1);
	    } else {		/* insert right page */
		p = LKEY1(nbp) + (p - LKEY1(bp)) -
		    (gkeylen + 2) * (gfp->maxkeys / 2);
		setNKEYS(nbp, NKEYS(nbp) + 1);
	    }
	    if (n != 0)
		moveright(p, p + gkeylen + 2, n * (gkeylen + 2));
	    keycopy(gkey, p);
	    setRECNO(p, gmode);
	    setFPTR(nbp, n = FPTR(bp));
	    setFPTR(bp, nbp->blkno);
	    setBPTR(nbp, bp->blkno);
	    unlock(bp);
	    if (n != 0) {
		if ((bp = getblk(n, (struct fildes *) gfp)) == NULL) {
		    unlock(nbp);
		    return (ierr(4));	/* isrch: bptr getblk fail */
		}
		setBPTR(bp, nbp->blkno);
		bp->changed = 1;
	    }
	    keycopy(LKEY1(nbp), v->key);
	    v->pblkno = nbp->blkno;
	    unlock(nbp);
	    return (1);		/* passup */
	}
    } else {			/* ptr node */
	ap = p = PKEY1(bp);
	if (gfp->ftype & F_MULT)	/* multiple keys */
	    for (r = 0; r < n; r++) {
		if (keycomp(ap) >= 0)
		    break;
		p = ap;
		ap += gkeylen + 2;
	} else			/* unique keys */
	    for (r = 0; r < n; r++) {
		if (keycomp(ap) > 0)
		    break;
		p = ap;
		ap += gkeylen + 2;
	    }
	if (r == 0)
	    n = BLK0(bp);
	else
	    n = BLKNO(p);
	/*dprintf( "psrch %d\n", n); */
	lockb(bp);
	n = isrch(n, &u);
	unlock(bp);
	if (gmode == 0)
	    return (n);
	if (n) {		/* passup */
	    /*dprintf( "isrch: passup key=%9s, blkno=%d\n", u.key, u.pblkno); */
	    n = NKEYS(bp);
	    if (n >= gfp->maxkeys)
		return (ierr(5));	/* isrch: pnode overflow */
	    if (r < n)
		moveright(ap, ap + gkeylen + 2, (n - r) * (gkeylen + 2));
	    keycopy(u.key, ap);
	    setBLKNO(ap, u.pblkno);
	    setNKEYS(bp, NKEYS(bp) + 1);
	    bp->changed = 1;
	}
	return (0);
    }
}

/*
 * delete key with record # rec from index file fp
 */
int
Delete(char *key, int rec, struct ndxfile *fp)
{
    struct passup v;
    short kbuf;

    grec = rec;
    gfp = fp;
    gkeylen = gfp->keylen;
    if ((gkeytyp = (gfp->ftype & F_INT)) != 0) {
	gkey = (char *) &kbuf;
	put16((char *) &kbuf, (int) key);
    } else
	gkey = key;
    gfp->curblk = -1;
    dsrch(0, &v);
    return (grec);
}

/*
 * recursive B* tree delete
 */
static int
dsrch(int blk, struct passup *v)
{
    register struct buffer *bp, *nbp;
    char *p, *ap;
    int n, r;
    struct passup u;

  again:
    if ((bp = getblk(blk, (struct fildes *) gfp)) == NULL)
	return (0);
    n = NKEYS(bp);
    if (PAGETYPE(bp) == 0) {	/* root node is leaf node */
	p = LKEY1(bp);
	while (n > 0 && RECNO(p) != grec) {
	    if (keycomp(p) > 0)
		break;
	    p += gkeylen + 2;
	    n--;
	}
	if ((gfp->ftype & F_MULT) && n <= 0)	/* mult keys */
	    if ((blk = FPTR(bp)) != 0) {
		/*dprintf( "dsrch: mult nxt blk"); */
		goto again;
	    }
	if (n <= 0 || keycomp(p) != 0 || RECNO(p) != grec) {
	    grec = 0;
	    return (ierr(6));	/* dsrch: key not found */
	}
	if (n > 1)
	    moveleft(p + gkeylen + 2, p, (n - 1) * (gkeylen + 2));
	setNKEYS(bp, NKEYS(bp) - 1);
	bp->changed = 1;
	if ((NKEYS(bp) > gfp->maxkeys / 2) || (blk == 0))
	    return (0);		/* aok */
	lockb(bp);
	if ((r = FPTR(bp)) != 0) {	/* fwd blk not empty */
	    if ((nbp = getblk(r, (struct fildes *) gfp)) == NULL) {
		unlock(bp);
		return (ierr(7));	/* dsrch: nbp getblk fail */
	    }
	    p = LKEY1(bp) + (NKEYS(bp) * (gkeylen + 2));
	    ap = LKEY1(nbp);
	    if (NKEYS(nbp) > gfp->maxkeys / 2) {	/* annect 1 */
		/*dprintf( "dsrch: annect 1\n"); */
		memcpy(p, ap, gkeylen + 2);	/* copy key */
		setNKEYS(bp, NKEYS(bp) + 1);
		moveleft(ap + gkeylen + 2, ap,
			 (NKEYS(nbp) - 1) * (gkeylen + 2));
		setNKEYS(nbp, NKEYS(nbp) - 1);
		nbp->changed = 1;
		keycopy(ap, v->key);
		v->pblkno = nbp->blkno;
		unlock(bp);
		return (1);	/* change key in ptr blk */
	    }
	    /*dprintf( "dsrch: annect all\n"); */
	    v->pblkno = nbp->blkno;	/* save blk # to delete */
	    memcpy(p, ap, NKEYS(nbp) * (gkeylen + 2));	/* annect all */
	    setNKEYS(bp, NKEYS(bp) + NKEYS(nbp));
	    setFPTR(bp, r = FPTR(nbp));	/* relink fwd ptr */
	    unlock(bp);
	    if (r != 0) {
		n = bp->blkno;
		if ((bp = getblk(r, (struct fildes *) gfp)) == NULL) {
		    ierr(21);	/* dsrch: nbp fptr fail */
		    return (2);
		}
		setBPTR(bp, n);
		bp->changed = 1;
	    }
	    return (2);		/* delete blk # in ptr blk */
	}
	if (NKEYS(bp) == 0) {	/* fptr null, no keys */
	    /*dprintf( "dsrch: fptr null, no keys\n"); */
	    v->pblkno = bp->blkno;
	    r = BPTR(bp);	/* set bwd ptr to NULL */
	    unlock(bp);
	    if (r != 0) {
		if ((bp = getblk(r, (struct fildes *) gfp)) == NULL) {
		    ierr(22);	/* dsrch: nbp bptr fail */
		    return (2);
		}
		setFPTR(bp, 0);
		bp->changed = 1;
	    }
	    return (2);
	}
	unlock(bp);
	return (0);		/* aok */
    } else {			/* ptr node */
	ap = p = PKEY1(bp);
	if (gfp->ftype & F_MULT)	/* mult keys */
	    for (r = 0; r < n; r++) {
		if (keycomp(ap) >= 0)
		    break;
		p = ap;
		ap += gkeylen + 2;
	} else			/* unique keys */
	    for (r = 0; r < n; r++) {
		if (keycomp(ap) > 0)
		    break;
		p = ap;
		ap += gkeylen + 2;
	    }
	if (r == 0)
	    n = BLK0(bp);
	else
	    n = BLKNO(p);
	lockb(bp);
	/*dprintf( "dsrch %d\n", n); */
	n = dsrch(n, &u);
	switch (n) {
	case 2:		/* delete passup blk # from ptr blk */
	    /*dprintf( "dsrch: delete blk #%d\n", u.pblkno); */
	    if (NKEYS(bp) == 1) {	/* ptr back to leaf node */
		if (u.pblkno == BLK0(bp)) {
		    p = PKEY1(bp);
		    blk = BLKNO(p);
		} else
		    blk = BLK0(bp);
		if ((nbp = getblk(blk, (struct fildes *) gfp)) == NULL) {
		    unlock(bp);
		    return (ierr(8));	/* dsrch: nbp leaf fail */
		}
		memcpy(bp->buf, nbp->buf, BLKSIZ);
		setFPTR(bp, 0);
		setBPTR(bp, 0);
		bp->changed = 1;
		unlock(bp);
		return (0);	// was ;
	    }
	    p = PKEY1(bp);
	    n = NKEYS(bp);
	    if (u.pblkno == BLK0(bp)) {
		setBLK0(bp, BLKNO(p));
		r = 0;
	    } else {
		for (r = 0; r < n; r++) {
		    if (u.pblkno == BLKNO(p))
			break;
		    p += gkeylen + 2;
		}
		if (r >= n) {
		    unlock(bp);
		    return (ierr(9));	/* dsrch: blk find fail */
		}
	    }
	    if (n - r - 1 > 0)
		moveleft(p + gkeylen + 2, p, (n - r - 1) * (gkeylen + 2));
	    setNKEYS(bp, NKEYS(bp) - 1);
	    bp->changed = 1;
	    unlock(bp);
	    return (0);		// was ;
	case 1:		/* change passup blk #'s key */
	    n = NKEYS(bp);
	    p = PKEY1(bp);
	    /*dprintf( "dsrch: change blk #%d key to %9s\n", u.pblkno, u.key); */
	    for (r = 0; r < n; r++) {
		if (u.pblkno == BLKNO(p)) {
		    keycopy(u.key, p);
		    bp->changed = 1;
		    unlock(bp);
		    return (0);	// was ;
		}
		p += gkeylen + 2;
	    }
	    unlock(bp);
	    return (ierr(10));
	}
	unlock(bp);
    }

    return 0;
}

int
next(struct fildes *fp)
{
    register struct ndxfile *nfp;
    register struct buffer *bp;
    register int blk;
    register int n;
    char rbuf[MAXRECSIZ];

    //fprintf(stderr, "next(fp)\n");

    fp->eof = 0;

    if (!fp->ndxfp) {

	//fprintf(stderr, "\t!fp->ndxfp()\n");  
	do {
	    if (++fp->currec > fp->nrecs) {
		fp->currec = fp->nrecs;
		fp->eof = 1;
		return (0);
	    }
	    getrec(fp->currec, fp, rbuf);
	} while (rbuf[0] == '*');
	//fprintf(stderr, "\treturn fp->currec\n");
	return (fp->currec);
    }

    nfp = fp->ndxfp;
    gfp = nfp;

    if ((blk = nfp->curblk) == -1) {
	//fp->eof = 1;
	return (ierr(12));	/* next: no currency */
    }

  nxtblk:
    if ((bp = getblk(blk, (struct fildes *) nfp)) == NULL)
	return (ierr(11));	/* next: bp getblk fail */
#ifdef UNIV
    n = NKEYS(bp);
    if ((gfp == &FAndx2 || gfp == &FAndx3) && n > RECMAX)
	return (fatal(8));
#endif
    if (++(nfp->curoff) > NKEYS(bp)) {
	if ((blk = FPTR(bp)) != 0) {
	    nfp->curblk = blk;
	    nfp->curoff = 0;
	    goto nxtblk;
	}
	nfp->curoff = NKEYS(bp);
	fp->eof = 1;
	return (0);
    }
    n = (nfp->keylen + 2) * (nfp->curoff - 1) + nfp->keylen + KEY1OFF;
    fp->currec = get16(&bp->buf[n]);
    if (fp->currec > fp->nrecs || fp->currec <= 0)
	return (ierr(13));	/* next: invalid currec */
    return (fp->currec);
}

int
prev(struct fildes *fp)
{
    register struct ndxfile *nfp;
    register struct buffer *bp;
    register int blk;
    register int n;
    char rbuf[MAXRECSIZ];

    fp->eof = 0;
    if (!fp->ndxfp) {
	do {
	    if (--fp->currec <= 0) {
		fp->currec = 1;
		fp->eof = 1;
		return (0);
	    }
	    getrec(fp->currec, fp, rbuf);
	} while (rbuf[0] == '*');
	return (fp->currec);
    }
    nfp = fp->ndxfp;
    gfp = nfp;
    if ((blk = nfp->curblk) == -1)
	return (ierr(14));	/* prev: no currency */
    if ((bp = getblk(blk, (struct fildes *) nfp)) == NULL)
	return (ierr(15));	/* prev: bp getblk fail */
    if (--(nfp->curoff) <= 0) {
	if ((blk = BPTR(bp)) != 0) {
	    if ((bp = getblk(blk, (struct fildes *) nfp)) == NULL)
		return (ierr(16));	/* prev: bptr getblk fail */
	    nfp->curblk = blk;
	    nfp->curoff = NKEYS(bp);
	    goto ok;
	}
	nfp->curoff = 1;
	fp->eof = 1;
	return (0);
    }
  ok:
    n = (nfp->keylen + 2) * (nfp->curoff - 1) + nfp->keylen + KEY1OFF;
    fp->currec = get16(&bp->buf[n]);
    return (fp->currec);
}

int
gotop(struct fildes *fp)
{

    register struct ndxfile *nfp;
    register struct buffer *bp;

    fp->eof = 0;
    if (!fp->ndxfp) {
	fp->currec = 0;
	return (next(fp));
    }
    nfp = fp->ndxfp;
    gfp = nfp;
    if ((bp = getblk(0, (struct fildes *) nfp)) == NULL)
	return (ierr(17));	/* gotop: bp getblk fail */
    if (PAGETYPE(bp) != 0)
	if ((bp = getblk(BLK0(bp), (struct fildes *) nfp)) == NULL)
	    return (ierr(18));	/* gotop: blk0 getblk fail */
    nfp->curblk = bp->blkno;
    if (NKEYS(bp) == 0) {
	nfp->curoff = 0;
	fp->eof = 1;
	return (0);
    }
    nfp->curoff = 1;
    return (fp->currec = get16(&bp->buf[nfp->keylen + KEY1OFF]));
}

int
gobot(struct fildes *fp)
{
    register struct ndxfile *nfp;
    register struct buffer *bp;
    register char *p;
    register int n;

    fp->eof = 0;
    if (!fp->ndxfp) {
	fp->currec = fp->nrecs + 1;
	return (prev(fp));
    }
    nfp = fp->ndxfp;
    gfp = nfp;
    if ((bp = getblk(0, (struct fildes *) nfp)) == NULL)
	return (ierr(19));	/* gobot: bp getblk fail */
    gkeylen = nfp->keylen;
    if (PAGETYPE(bp) != 0) {
	p = PKEY1(bp) + ((NKEYS(bp) - 1) * (gkeylen + 2));
	if ((bp = getblk(BLKNO(p), (struct fildes *) nfp)) == NULL)
	    return (ierr(20));	/* gobot: blkno getblk fail */
    }
    nfp->curblk = bp->blkno;
    if (NKEYS(bp) == 0) {
	nfp->curoff = 0;
	fp->eof = 1;
	return (0);
    }
    nfp->curoff = NKEYS(bp);
    n = (nfp->keylen + 2) * (nfp->curoff - 1) + nfp->keylen + KEY1OFF;
    return (fp->currec = get16(&bp->buf[n]));
}

int
gotorec(struct fildes *fp, int rec)
{
    fp->eof = 0;
    if (fp->nrecs == 0 || rec == 0) {
	fp->currec = 0;
	fp->eof = 1;
    } else {
	fp->currec = rec - 1;
	next(fp);
    }
    return (fp->currec);
}

static void
keycopy(char *src, char *dst)
{
    register int n;

    if (gkeytyp)
	put16(dst, (int) get16(src));
    else {
	n = gkeylen;
	while (n--)
	    *dst++ = *src++;
    }
}

static int
keycomp(char *key)
{
    register int n;
    register char *p;

    if (gkeytyp)
	return (get16(key) - get16(gkey));
    n = gkeylen;
    p = gkey;
    while (--n >= 0 && *p++ == *key++)
	continue;
    return (n < 0 ? 0 : *--key - *--p);
}

static void
moveright(char *asrc, char *adst, int cnt)
{
    register char *src, *dst;

    src = asrc + cnt;
    dst = adst + cnt;
    while (cnt--)
	*--dst = *--src;
}

static int
ierr(int n)
{
    printf("\007*** Index Error #%d, file=%s, ", n, gfp->filext);
    if (gkeytyp)
	printf("key=%d", get16(gkey));
    else
	printf("key=%9s", gkey);
    printf("\n");
    return (0);
}
