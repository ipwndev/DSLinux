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
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/types.h>

#include <pixlib/pixlib.h>
#include <linux.h>

/*
 * linux_set_date(date_struct)
 * Set the system date based on the passed structure
 */

int
linux_set_date(pix_sys_date_t * date_struct)
{
    struct tm newtm;
    time_t newtime;
    struct timeval tv;

    if (!date_struct)
	return (PIX_SYS_ERROR);

    newtm.tm_sec = date_struct->second;
    newtm.tm_min = date_struct->minute;
    newtm.tm_hour = date_struct->hour;

    newtm.tm_mday = date_struct->day;
    newtm.tm_mon = date_struct->month - 1;
    newtm.tm_year = date_struct->year - 1900;

    newtime = mktime(&newtm);

    if (newtime == -1)
	return (PIX_SYS_ERROR);

    tv.tv_sec = newtime;

    if (settimeofday(&tv, NULL) == -1) {
	if (errno == EPERM)
	    return (PIX_SYS_NOT_SUPERUSER);
	else
	    return (PIX_SYS_ERROR);
    }

    return (PIX_SYS_OK);
}

/*
 * linux_get_date(date_struct) 
 * Return the current system date / time 
 */

int
linux_get_date(pix_sys_date_t * date_struct)
{
    pix_sysGetCurrentTime(date_struct);
    return (PIX_SYS_OK);
}

/*
 * linux_get_cpu_load(cpu_struct)
 * Get the current cpu loads (in jiffies) 
 */

int
linux_get_cpu_load(pix_sys_cpu_t * cpu_struct)
{
    int fd;
    char str[BUFSIZ];
    char *c;

    /* Open up the file */
    fd = open("/proc/stat", O_RDONLY);

    if (fd == -1) {
	perror("linux_get_cpu_load");
	return (PIX_SYS_FILE_ERROR);
    }

    read((int) fd, str, BUFSIZ - 1);

    /* Now skip over "cpu" */
    for (c = str; *c != ' '; c++)
	continue;

    c++;

    /* Get the new values */

    cpu_struct->user = strtoul(c, &c, 0);
    cpu_struct->nice = strtoul(c, &c, 0);
    cpu_struct->system = strtoul(c, &c, 0);
    cpu_struct->idle = strtoul(c, &c, 0);

    close(fd);
    return (PIX_SYS_OK);
}

/*
 * linux_get_memory_usage(mem_struct)
 * Get the current memory usage  
 */

int
linux_get_memory_usage(pix_sys_memory_t * mem_struct)
{
    int fd;
    char str[BUFSIZ];
    char *c;

    /* Open up the file */
    fd = open("/proc/meminfo", O_RDONLY);

    if (fd == -1) {
	perror("linux_get_memory_usage");
	return (PIX_SYS_FILE_ERROR);
    }

    read((int) fd, str, BUFSIZ - 1);

    /* Completely skip the first line */
    for (c = str; *c != '\n'; c++);

    /* Now, parse the line to get all of the good stuff */
    for (c++; *c != ' '; c++);

    /* Now skip over any whitespace to get to the first number */
    for (; *c == ' '; c++);

    /* Now read each number, and put it directly into the passed */
    /* memory struct */

    mem_struct->total = strtoul(c, &c, 0);
    mem_struct->used = strtoul(c, &c, 0);
    mem_struct->free = strtoul(c, &c, 0);
    mem_struct->shared = strtoul(c, &c, 0);
    mem_struct->buffers = strtoul(c, &c, 0);
    mem_struct->cached = strtoul(c, &c, 0);

    /* Skip to the next line */
    for (; *c != '\n'; c++);
    for (c++; *c != ' '; c++);
    for (; *c == ' '; c++);

    mem_struct->stotal = strtoul(c, &c, 0);
    mem_struct->sused = strtoul(c, &c, 0);

    close(fd);

    return (PIX_SYS_OK);
}
