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
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <wm/nxlib.h>
#include <errno.h>

//#define USERAMFILE 1
//#include "ramfile.h"

/*
 * This file contains all the INI read/write functions, as well as
 * anything having to do with finding, reading or writing config files 
 */

#if MAC
#define EOLCHAR	'\r'
#else
#define EOLCHAR	'\n'
#endif

#define MODE_RT		"rt"	/* read text */
#define ISblank(c)	((c) == ' ' || (c) == '\t')

/* private data*/
int _sawsection;
int _lineno;
char _lineblank;

int
IniGetString(char *section, char *key, char *defval, char *retbuf, int bufsiz,
	     char *inifile)
{
    char *p;
    char *q = 0;
    FILE *fp;
    char buf[256];
    char buf2[256];

    _sawsection = 0;
    _lineno = 0;
    _lineblank = 1;
    if ((fp = fopen(inifile, MODE_RT)) == NULL) {
      def:
	strzcpy(retbuf, defval, bufsiz);
	return (0);		/* not found */
    }
    if (key != NULL) {
	if (bufsiz > sizeof(buf))
	    bufsiz = sizeof(buf);
    } else
	q = retbuf;
    while (fgets(buf, sizeof(buf) - 1, fp) != NULL) {
	_lineno++;
	_lineblank = (buf[0] == EOLCHAR);
	if (buf[0] == '[') {
	    if ((p = strchr(buf, ']')) != NULL)
		*p = 0;
	    if (strcasecmp(buf + 1, section) == 0) {
		_sawsection = _lineno;
		while (fgets(buf, sizeof(buf) - 1, fp) != NULL) {
		    if (buf[0] == '[')
			goto notfound;
		    _lineno++;
		    _lineblank = (buf[0] == EOLCHAR);
		    /* enumeration case */
		    if (key == NULL) {
			if (_lineblank || buf[0] == ';')
			    continue;
			p = buf;
			while (bufsiz > 1) {
			    if (*p == EOLCHAR || *p == 0) {
				*q++ = 0;
				bufsiz--;
				break;
			    } else {
				*q++ = *p;
				bufsiz--;
			    }
			    p++;
			}
			continue;
		    }
		    p = buf;
		    q = buf2;
		    while (ISblank(*p))
			p++;
		    while (*p && *p != '=' /***&& !ISblank( *p)***/ )
			*q++ = *p++;
		    while (q > buf2 && ISblank(q[-1]))
			--q;
		    *q = 0;
		    if (strcasecmp(key, buf2) == 0) {
						/***while( ISblank( *p))
							p++;***/
			if (*p == '=')
			    p++;
			while (ISblank(*p))
			    p++;
			for (q = retbuf; bufsiz-- > 1;) {
			    if (*p == EOLCHAR || *p == 0)
				break;
			    *q++ = *p++;
			}
			*q = 0;
			fclose(fp);
			/* we return 1 instead of q-retbuf here
			 * so that IniWriteString() works with key=(null) strs
			 */
			return (q - retbuf ? q - retbuf : 1);
		    }
		}
	    }
	}
    }
  notfound:
    fclose(fp);
    if (key == NULL) {
	if (q == retbuf)	/* handle no enumeration case */
	    *q++ = 0;
	*q = 0;			/* double nul char terminate */
	return (q - retbuf);	/* return byte count copied minus NUL */
    }
    goto def;
}

/* 
 * Call a function for each enumerated key/value pair
 */
void
IniEnumKeyValues(char *buf, void (*pfn) (char *, char *, int), int data)
{
    char *list;
    char *q;
    char key[256];

    for (list = buf; *list;) {
	/* parse key */
	q = key;
	while (ISblank(*list))
	    list++;
	while (*list && *list != '=' && !ISblank(*list))
	    *q++ = *list++;
	*q = 0;

	/* skip separator */
	if (*list == '=')
	    list++;
	while (ISblank(*list))
	    list++;

	pfn(key, list, data);
	list += strlen(list) + 1;
    }
}

/* Replace the given keyword in the given string with the given value */

void
str_replace_variable(char *keyword, char *value,
		     char *input, char *output, int outsize)
{
    char *in = input;
    char *out = output;
    int size = 0;

    char *pos;

    if (!in || !out)
	return;

    if (!strlen(in))
	return;

    /* Zero out the output just in case */
    bzero(out, outsize);

    while (1 && *in) {
	/* If the string doesn't exist, then copy the rest out and bail */
	pos = strstr(in, keyword);

	if (!pos) {
	    if ((size + strlen(in)) > outsize)
		strncpy(out, in, outsize - size - 1);
	    else
		strcpy(out, in);

	    return;
	} else {
	    int subsize = 0;

	    if ((subsize = (int) (pos - in))) {
		if ((size + subsize) > outsize) {
		    strncpy(out, in, outsize - size - 1);
		    return;
		} else
		    strncpy(out, in, subsize);

		in += subsize;
		out += subsize;
		size += subsize;
	    }

	    if ((size + strlen(value)) > outsize) {
		strncpy(out, value, outsize - size);
		return;
	    } else
		strcpy(out, value);

	    in += strlen(keyword);
	    out += strlen(value);
	    size += strlen(value);
	}
    }
}

#ifdef NOTUSED

/* We can now have mulitiple configuration files.  To make it */
/* easy, we have an array of pointers that store the locations */
/* of the various config files */

static char nxConfigFiles[256][NX_CONFIG_COUNT];

/* This routine will find a config file in the standard locations */
/* and store it's value in the array at the indicated index */

int
nxLoadConfigFile(char *filename, int index)
{
    char foundfile[512];

    char etcdir[] = "/etc";
    char *searchdirs[3];
    char cwd[512];

    int res;
    int scount;
    uid_t uid;
    struct passwd *pwd;

    if (index > NX_CONFIG_COUNT)
	return (-1);

    if ((strlen(nxConfigFiles[index]))) {
	printf("Found config file [%s]\n", nxConfigFiles[index]);
	return (1);
    }

    printf("Searching for config file [%s]\n", filename);

    getcwd(cwd, 512);
    searchdirs[0] = cwd;	/* Always look in the local dir first */

    uid = getuid();		/* Get the uid of the current process */
    pwd = getpwuid(uid);	/* Get the user information from passwd */

    if (pwd) {			/* If the user was found in the password file */
	searchdirs[1] = pwd->pw_dir;	/* Search homedir second */
	searchdirs[2] = etcdir;	/* Search /etc third */
	scount = 3;
    } else {
	searchdirs[1] = etcdir;	/* Search /etc second */
	scount = 2;
    }

    res = nxSearchForFile(filename, foundfile, sizeof(foundfile),
			  searchdirs, scount);

    if (res == 0) {
	printf("Unable to find config file [%s] in path\n", filename);
    } else {
	printf("Found config file [%s]\n", foundfile);

	bzero(&nxConfigFiles[index], 256);

	strncpy(nxConfigFiles[index], foundfile, 255);
    }

    return (res);
}

char *
nxGetConfigFile(int index)
{
    if (strlen(nxConfigFiles[index]))
	return (nxConfigFiles[index]);

    return (0);
}

void
nxFreeConfigFiles()
{
}

#endif
