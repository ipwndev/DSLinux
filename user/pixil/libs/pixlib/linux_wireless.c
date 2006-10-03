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
#include <errno.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/wireless.h>

#include <pixlib/pixlib.h>
#include <linux_wireless.h>

/* Defined in linux_if.c */

int linux_open_socket();

static int
wireless_do_ioctl(char *interface, int cmd, struct iwreq *data)
{
    int ret;
    int fd = linux_open_socket();

    if (fd < 0)
	return (errno);

    strcpy(data->ifr_ifrn.ifrn_name, interface);

    /* Now, do the ioctl call */
    ret = ioctl(fd, cmd, data);

    if (ret < 0)
	ret = errno;

    close(fd);

    return (ret);
}

int
wireless_name(char *interface, char *name)
{
    int cmd = SIOCGIWNAME;
    struct iwreq data;

    data.u.name[0] = '\0';

    if (wireless_do_ioctl(interface, cmd, &data) < 0) {
	return (PIX_COMM_ERROR);
    } else {
	strcpy(name, data.u.name);
	return (PIX_COMM_OK);
    }

}

int
wireless_encode(int operation, char *interface, char *name)
{
    int cmd = 0;
    struct iwreq data;

    switch (operation) {
    case PIX_COMM_GET:
	cmd = SIOCGIWENCODE;
	break;

    case PIX_COMM_SET:
	cmd = SIOCSIWENCODE;
	break;
    }

    data.u.encoding.pointer = (caddr_t) name;

    if (wireless_do_ioctl(interface, cmd, &data) < 0)
	return (PIX_COMM_ERROR);
    else
	return (PIX_COMM_OK);
}

int
wireless_essid(int operation, char *interface, char *name)
{
    int cmd = 0;
    struct iwreq data;

    switch (operation) {
    case PIX_COMM_GET:
	cmd = SIOCGIWESSID;
	break;

    case PIX_COMM_SET:
	cmd = SIOCSIWESSID;
	break;
    }

    printf("data.u.essid.pather = %s\n", name);
    data.u.essid.pointer = (caddr_t) name;

    if (wireless_do_ioctl(interface, cmd, &data) < 0) {
	return (PIX_COMM_ERROR);
    } else {
	return (PIX_COMM_OK);
    }

}

int
wireless_get_statistics(char *interface, pix_comm_wirestats_t * stats)
{
    char tmpstr[3];
    char buffer[BUFSIZ];

    FILE *wfile = fopen("/proc/net/wireless", "r");

    if (NULL == wfile)
	return (PIX_COMM_ERROR);

    /* Read the first couple of lines */
    fgets(buffer, BUFSIZ, wfile);
    fgets(buffer, BUFSIZ, wfile);

    /* Now, go through all the interfaces until we find one that */
    /* we want */

    while (1) {
	if (feof(wfile))
	    break;

	fgets(buffer, BUFSIZ, wfile);
	if (!strncmp(buffer, interface, strlen(interface))) {
	    sscanf(buffer,
		   "%6s: %04hx  %3hhd%c  %3hhd%c  %3hhd%c  %6ld %6ld %6ld\n",
		   stats->name, &(stats->status), &(stats->quality),
		   &tmpstr[0], &stats->level, &tmpstr[1], &stats->noise,
		   &tmpstr[2], &stats->discard.nwid, &stats->discard.code,
		   &stats->discard.misc);

	    fclose(wfile);
	    return (PIX_COMM_OK);
	}
    }

    fclose(wfile);
    return (PIX_COMM_ERROR);
}

int
wireless_get_if_list(pix_comm_interface_t * iflist, int *ifcount)
{
    char tmpstr[3];
    char ifname[10];
    char buffer[BUFSIZ];
    int idx = 0;
    int retcount = -1;
    int err = PIX_COMM_ERROR;
    pix_comm_wirestats_t stats;

    /* We want to return a list of interfaces and some stats */
    FILE *wfile = fopen("/proc/net/wireless", "r");

    if (NULL == wfile)
	return (PIX_COMM_ERROR);

    /* Read the first couple of lines */
    fgets(buffer, BUFSIZ, wfile);
    fgets(buffer, BUFSIZ, wfile);

    while (1) {
	if (feof(wfile))
	    break;
	fgets(buffer, BUFSIZ, wfile);
	sscanf(buffer,
	       "%6s: %04hx  %3hhd%c  %3hhd%c  %3hhd%c  %6ld %6ld %6ld\n",
	       stats.name, &(stats.status), &(stats.quality), &tmpstr[0],
	       &stats.level, &tmpstr[1], &stats.noise, &tmpstr[2],
	       &stats.discard.nwid, &stats.discard.code, &stats.discard.misc);

	for (idx = 0; idx < 4; idx++) {
	    sprintf(ifname, "eth%d", idx);
	    if (0 == strncmp(ifname, stats.name, strlen(ifname))) {
		retcount++;
		strcpy(iflist[retcount].name, ifname);
		if (retcount >= *ifcount)
		    break;
	    }
	    sprintf(ifname, "wvlan%d", idx);
	    if (0 == strncmp(ifname, stats.name, strlen(ifname))) {
		retcount++;
		strcpy(iflist[retcount].name, ifname);
		if (retcount >= *ifcount)
		    break;
	    }
	}
    }

    if (retcount != -1) {
	err = PIX_COMM_OK;
	*ifcount = retcount;
    } else {
	err = PIX_COMM_NO_INTERFACES;
	*ifcount = 0;
    }
    fclose(wfile);
    return (err);
}
