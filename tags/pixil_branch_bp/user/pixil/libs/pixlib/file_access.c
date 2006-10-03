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


#include <pixlib/pixlib.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>


// If you want more type names to associate with directories add them here.
nxFSlist FStypes[] = { {"Memory Card", "cf"}
,
{"Local", "local"}
,
{"CD-ROM", "cdrom"}
,
{"", ""}
};

// This returns the offset into FStypes[] of the present type -1 on error
int
get_devtype(const char *input)
{
    int i, o;
    char path[10];
    memset(path, '\0', 10);
    o = 0;
    for (i = 0; i < 20; i++) {
	if (input[i] == '/') {
	    o++;
	    if (o == 3) {
		path[i] = '\0';
		break;
	    }
	}
	if (o == 2)
	    path[i] = input[i];
    }
    if (o < 3)
	return -1;
    for (i = 0; FStypes[i].name[0]; i++) {
	if (!strcmp(path, FStypes[i].path))
	    return i;
    }
    return -1;
}

char *
nxFSgettwd(void)
{
    static char dirbuff[128];
    char gpbuff[128];
    char dnbuff[5];
    char *bptr;
    int i;
    int devtype;
    int devnum;
    memset(dnbuff, '\0', 5);
    if ((devtype = get_devtype(getcwd(gpbuff, 128))) == -1)
	return "path_error";
    bptr = strchr(strchr(strchr(gpbuff, '/') + 1, '/') + 1, '/') + 1;
    for (i = 0; i < 5; i++) {
	dnbuff[i] = bptr[i];
	if (dnbuff[i] == '/')
	    dnbuff[i] = '\0';
    }
    devnum = atoi(dnbuff);
    sprintf(dirbuff, "%s %i %s", &FStypes[devtype].name[0], devnum,
	    &bptr[i - 1]);
    return dirbuff;

}


// this does damage the string by adding a NULL
char *
get_mtabpath(char *str)
{
    int i;
    char *ret;
    ret = strchr(strchr(str, ' '), '/');
    i = 0;
    while (ret[i] != ' ') {
	i++;
    }
    ret[i] = '\0';
    return ret;
}

// this function takes list as a null pointer.  Free it later
int
xnFSgetfslist(nxFSlist ** list)
{
    FILE *mtabf;
    nxFSlist *tmplist;
    nxFSlist *retlist;
    char mtab_line[80];
    char *resbuf;
    char *ptrbuf;
    char dnbuff[5];
    int entries;
    int i, o;
    memset(dnbuff, '\0', 5);
    if ((mtabf = fopen("/etc/mtab", "r")) == NULL)
	return -1;
    tmplist = malloc(sizeof(nxFSlist) * 20);
    for (entries = 0;;) {
	if ((resbuf = fgets(mtab_line, 80, mtabf)) == NULL)
	    break;
	ptrbuf = get_mtabpath(resbuf);
	if ((i = get_devtype(ptrbuf)) > -1) {
	    resbuf =
		strchr(strchr(strchr(ptrbuf, '/') + 1, '/') + 1, '/') + 1;
	    for (o = 0; o < 5; o++) {
		dnbuff[o] = resbuf[o];
		if (dnbuff[o] == '/')
		    dnbuff[o] = '\0';
	    }
	    o = atoi(dnbuff);
	    sprintf(tmplist[entries].name, "%s %i", FStypes[i].name, o);
	    sprintf(tmplist[entries].path, "%s", ptrbuf);
	    entries++;
	}
    }
    retlist = malloc(sizeof(nxFSlist) * entries);
    for (i = 0; i < entries; i++) {
	strcpy(retlist[i].name, tmplist[i].name);
	strcpy(retlist[i].path, tmplist[i].path);
    }
    *list = retlist;
    fclose(mtabf);
    free(tmplist);
    return entries;
}
