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
#include "file.h"

#include "VCMemoryLeak.h"

/*
 * use.c - common database record get/put routines
 *
 * from 8/6/85 mt15b
 */

/* local data*/
static struct usefile usefile[NUSEFILES];
static struct field *p;
static char *cp;

/* local procs*/
static void chguse(int fileno);
static void chkuse(int fileno);
static void setpcp(int var);

int
get16(char *addr)
{
    return ((unsigned char) addr[0]) | (((int) (unsigned char) addr[1]) << 8);
}

void
put16(char *addr, unsigned short val16)
{

    *addr++ = val16;
    *addr = val16 >> 8;
}


long
get32(char *addr)
{

    return ((unsigned char) addr[0]) |
	(((long) (unsigned char) addr[1]) << 8) |
	(((long) (unsigned char) addr[2]) << 16) |
	(((long) (unsigned char) addr[3]) << 24);
}


void
put32(char *addr, long val32)
{

    *addr++ = val32;
    *addr++ = val32 >> 8;
    *addr++ = val32 >> 16;
    *addr = val32 >> 24;
}


void
use(int fileno, struct fildes *fp, struct ndxfile *nfp)
{

    chguse(fileno);
    usefile[fileno].fp = fp;
    if (fp != NULL) {
	fp->ndxfp = nfp;
    }
    usefile[fileno].recno = -1;
}

static void
chguse(int fileno)
{
    if (usefile[fileno].uchanged) {
	putrec(usefile[fileno].recno, usefile[fileno].fp,
	       usefile[fileno].rbuf);
	usefile[fileno].uchanged = 0;
    }
}

static void
chkuse(int fileno)
{
    struct fildes *fp;

    if (fileno < 0 || fileno > NUSEFILES)
	fatal(5);		/* chkuse: bad fileno */
    fp = usefile[fileno].fp;
    if (fp->currec != usefile[fileno].recno) {
	chguse(fileno);
	getrec(fp->currec, fp, usefile[fileno].rbuf);
	usefile[fileno].recno = fp->currec;
    }
}

/*
 * return value of file variable
 * on entry, hi byte of var is fileno, lo byte is fieldno
 */
char *
val(int var)
{
    register int len;
    static char varbuf[65];	/* max var len is 64! */

    setpcp(var);
    if ((len = p->size) > 64)
	len = 64;
    fcopy(cp, varbuf, len);
    return (varbuf);
}

short
ival(int var)
{
    setpcp(var);
    return (get16(cp));
}

long
lval(int var)
{
    setpcp(var);
    return (get32(cp));
}

void
replace(int var, union w newval)
{
    setpcp(var);
    switch (p->type) {
    case 'c':
	memcpy(cp, newval.ptr, p->size);
	break;
    case 'd':
    case 'i':
	put16(cp, newval.word);
	break;
    case 'l':
	put32(cp, newval.lword);
	break;
    }
    usefile[var >> 8].uchanged = 1;
}

int
find(int fileno, char *key)
{
    struct fildes *fp;

    fp = usefile[fileno].fp;
    return (fp->currec = search(key, fp->ndxfp, 0));
}

void
store(void *dst, int var)
{
    setpcp(var);
    switch (p->type) {
    case 'c':
	fcopy(cp, (char *) dst, p->size);
	break;
    case 'd':
    case 'i':
	put16((char *) dst, get16(cp));
	break;
    case 'l':
	put32((char *) dst, get32(cp));
	break;
    }
}

static void
setpcp(int var)
{
    register int fileno, fieldno;

    fileno = var >> 8;
    fieldno = var & 0xff;
    chkuse(fileno);
    p = &usefile[fileno].fp->fieldp[fieldno];
    cp = &usefile[fileno].rbuf[p->offset];
}

/*
 * delete current record in usefile
 */
void
udelrec(int fileno)
{
    chkuse(fileno);
    usefile[fileno].rbuf[0] = '*';
    usefile[fileno].uchanged = 1;
}

/*
 * copy a string field replacing nulls with blanks
 */
char *
fcopy(char *src, char *dst, int len)
{
#if 0
    while (len--)
	if (*src)
	    *dst++ = *src++;
	else {
	    *dst++ = ' ';
	    src++;
	}
    *dst++ = 0;			/* null terminate */
    return (dst);
#else
    while (len--)
	*dst++ = *src++;
    *dst++ = 0;
    return (dst);
#endif
}
