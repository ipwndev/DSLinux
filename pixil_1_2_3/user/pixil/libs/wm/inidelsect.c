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

int
IniDelSection(char *section, char *inifile)
{
    char *p;
    FILE *fp;
    char buf[256];
    int numlines = 0;
    int sectnumlines = 0;
    int sectionline = 0;
    int lineno = 0;
    char **newfile = NULL;
    struct stat filestat;
    off_t filesize;
    off_t filepos = 0;
    int err;
    int newlineno = 0;
    int idx = 0;

    if ((fp = fopen(inifile, "r+")) == NULL) {
	return (1);
    }

    err = stat(inifile, &filestat);
    if (err < 0) {
	fprintf(stderr, "Error [%s]\n", strerror(errno));
	fclose(fp);
	return 1;
    }
    filesize = filestat.st_size;
    while (NULL != fgets(buf, sizeof(buf) - 1, fp)) {
	numlines++;
    }
    rewind(fp);
    while (fgets(buf, sizeof(buf) - 1, fp) != NULL) {
	lineno++;
	if (buf[0] == '[') {
	    if ((p = strchr(buf, ']')) != NULL) {
		*p = 0;
	    }
	}
	if (strcasecmp(buf + 1, section) == 0) {
	    sectionline = lineno;
	    //sectnumlines++;
	    while (fgets(buf, sizeof(buf) - 1, fp) != NULL) {
		sectnumlines++;
		if (buf[0] == '[') {
		    sectnumlines--;
		    break;
		}
	    }
	    if ((newfile =
		 (char **) calloc(numlines + 1, sizeof(char *))) == NULL) {
		fclose(fp);
		return 1;
	    }
	    rewind(fp);
	    filepos = 0;
	    lineno = 0;
	    while (NULL != fgets(buf, sizeof(buf) - 1, fp)) {
		lineno++;
		if ((lineno < sectionline)
		    || (lineno > sectionline + sectnumlines)) {
		    newfile[newlineno] =
			(char *) calloc(strlen(buf) + 1, sizeof(char));
		    if (newfile[newlineno] == NULL) {
			fclose(fp);
			free(newfile);
			return 1;
		    }
		    filepos += strlen(buf);
		    strncpy(newfile[newlineno], buf, strlen(buf));
		    newlineno++;
		}
	    }
	    rewind(fp);
	    for (idx = 0; idx < newlineno; idx++) {
		fputs(newfile[idx], fp);
		free(newfile[idx]);
	    }
	    if (filepos < filesize) {
		truncate(inifile, filepos);
	    }
	    free(newfile[idx]);
	    fclose(fp);
	    return 0;
	}
    }
    fclose(fp);
    return 0;
}
