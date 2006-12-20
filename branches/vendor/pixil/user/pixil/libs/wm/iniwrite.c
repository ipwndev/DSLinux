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
IniWriteString(char *section, char *key, char *inbuf, char *inifile)
{
    char *p;
    FILE *fp;
    char buf[256];
    char newstring[256];
    struct stat filestat;
    off_t filesize;
    off_t file_pos = 0;
    off_t section_pos = 0;
    off_t key_pos = 0;
    char **newfile = NULL;
    int num_lines = 0;
    int idx;
    int jdx;
    int err;
    int newlineno;
    int _lineno = 0;

    if ((fp = fopen(inifile, "a+")) == NULL) {
	return 1;
    }
    fclose(fp);
    fp = fopen(inifile, "r+");
    err = stat(inifile, &filestat);
    if (err < 0) {
	fprintf(stderr, "Error [%s]\n", strerror(errno));
	fclose(fp);
	return 1;
    }
    filesize = filestat.st_size;
    while (NULL != fgets(buf, sizeof(buf) - 1, fp)) {
	num_lines++;
    }
    rewind(fp);
    newlineno = 0;
    while (NULL != fgets(buf, sizeof(buf) - 1, fp)) {
	file_pos += strlen(buf);
	_lineno++;
	if (buf[0] == '[') {
	    if ((p = strchr(buf, ']')) != NULL)
		*p = 0;
	    if (strcasecmp(buf + 1, section) == 0) {
		section_pos = file_pos;
		while (fgets(buf, sizeof(buf) - 1, fp) != NULL) {
		    _lineno++;
		    file_pos += strlen(buf);
		    if (strncasecmp(key, buf, strlen(key)) == 0) {	// found matching key change it
			key_pos = file_pos - strlen(buf);
			goto insert;
		    }
		    if (buf[0] == '[') {	// out of the section so insert new key
			rewind(fp);
			jdx = 1;
			file_pos = 0;
			while (NULL != fgets(buf, sizeof(buf) - 1, fp)) {
			    file_pos += strlen(buf);
			    jdx++;
			    if (0 == strcmp(buf, "\n")
				&& (jdx == _lineno - 1)) {
				key_pos = file_pos - strlen(buf);
				goto insert;
			    }
			    if (jdx == _lineno) {
				key_pos = file_pos;
				goto insert;
			    }
			}
		    }
		    if (file_pos == filesize) {	// insert new key at end of file
			sprintf(newstring, "%s=%s\n", key, inbuf);
			fputs(newstring, fp);
			fclose(fp);
			return 0;
		    }
		}
	    }
	}
    }
    // insert new section and key at end of file
    sprintf(newstring, "\n[%s]\n", section);
    fputs(newstring, fp);
    sprintf(newstring, "%s=%s\n", key, inbuf);
    fputs(newstring, fp);
    fclose(fp);
    return 0;

  insert:
    if ((newfile = (char **) calloc(num_lines + 1, sizeof(char *))) == NULL) {
	fclose(fp);
	return 1;
    }
    sprintf(newstring, "%s=%s\n", key, inbuf);
    newfile[newlineno] = (char *) calloc(strlen(newstring) + 1, sizeof(char));
    if (newfile[newlineno] == NULL) {
	fclose(fp);
	free(newfile);
	return 1;
    }
    file_pos -= strlen(buf);
    file_pos += strlen(newstring);
    strncpy(newfile[newlineno], newstring, strlen(newstring));
    while (NULL != fgets(buf, sizeof(buf) - 1, fp)) {
	newlineno++;
	newfile[newlineno] = (char *) calloc(strlen(buf) + 1, sizeof(char));
	if (newfile[newlineno] == NULL) {
	    fclose(fp);
	    free(newfile);
	    return 1;
	}
	file_pos += strlen(buf);
	strncpy(newfile[newlineno], buf, strlen(buf));
    }
    fsetpos(fp, (fpos_t *) & key_pos);
    for (idx = 0; idx <= newlineno; idx++) {
	fputs(newfile[idx], fp);
	free(newfile[idx]);
    }
    if (file_pos < filesize) {
	truncate(inifile, file_pos);
    }
    free(newfile[idx]);
    fclose(fp);
    return 0;

}
