/***********************************************************************\
*   Copyright (C) 1992-1998 by Michael K. Johnson, johnsonm@redhat.com *
*                                                                      *
*      This file is placed under the conditions of the GNU Library     *
*      General Public License, version 2, or any later version.        *
*      See file ../COPYING for information on distribution conditions. *
\***********************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pwd.h>
#include "proc/procps.h"
#include <grp.h>

#define	HASHSIZE	16			/* power of 2 */
#define	HASH(x)		((x) & (HASHSIZE - 1))

#define NAMESIZE	16
#define NAMELENGTH	"15"

static struct pwbuf {
    uid_t uid;
    char name[NAMESIZE];
    struct pwbuf *next;
} *pwhash[HASHSIZE];

char *user_from_uid(uid_t uid)
{
    struct pwbuf **p;
    struct passwd *pw;

    p = &pwhash[HASH(uid)];
    while (*p) {
	if ((*p)->uid == uid)
	    return((*p)->name);
	p = &(*p)->next;
    }
    *p = (struct pwbuf *) xmalloc(sizeof(struct pwbuf));
    (*p)->uid = uid;
    if ((pw = getpwuid(uid)) == NULL)
	sprintf((*p)->name, "#%d", uid);
    else
	sprintf((*p)->name, "%-." NAMELENGTH "s", pw->pw_name);
    (*p)->next = NULL;
    return((*p)->name);
}

static struct grpbuf {
    gid_t gid;
    char name[NAMESIZE];
    struct grpbuf *next;
} *grphash[HASHSIZE];

char *group_from_gid(gid_t gid)
{
    struct grpbuf **g;
    struct group *gr;

    g = &grphash[HASH(gid)];
    while (*g) {
       if ((*g)->gid == gid)
           return((*g)->name);
       g = &(*g)->next;
    }
    *g = (struct grpbuf *) malloc(sizeof(struct grpbuf));
    (*g)->gid = gid;
    if ((gr = getgrgid(gid)) == NULL)
       sprintf((*g)->name, "#%d", gid);
    else
       sprintf((*g)->name, "%-." NAMELENGTH "s", gr->gr_name);
    (*g)->next = NULL;
    return((*g)->name);
}

void bad_user_access_length() { }
