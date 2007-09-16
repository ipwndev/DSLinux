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
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>

#include "local_route.h"
#include <pixlib/pixlib.h>

/****** UTILITY FUNCTIONS *******/

int
linux_open_socket(void)
{
    int fd;

    /* Open up a socket so we can communicate */
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0)
	perror("OPEN_SOCKET (SOCKET)");

    return (fd);
}

static int
resolve(char *name, struct sockaddr_in *sin)
{
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    if (!strcmp("default", name)) {
	sin->sin_addr.s_addr = INADDR_ANY;
	return (1);
    }
    if (inet_aton(name, &sin->sin_addr)) {
	return 0;
    }
    return 0;
}

void
linux_set_sockaddr(struct sockaddr *insock,
		   unsigned long addr, unsigned short port)
{
    struct sockaddr_in *sock = (struct sockaddr_in *) insock;

    sock->sin_family = AF_INET;
    sock->sin_addr.s_addr = addr;
    sock->sin_port = port;
}

int
get_if_flags(char *ifname, unsigned long *flags)
{
    int err = 0;
    int fd;
    struct ifreq ifr;

    /* Open up a socket so we can communicate */
    if ((fd = linux_open_socket()) < 0)
	return (-1);

    memset(&ifr, 0, sizeof(ifr));

    /* Copy the interface name over */
    strcpy(ifr.ifr_name, ifname);

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
	perror("GET_IF_FLAGS (IOCTL)");
	*flags = 0;
	err = -1;
    } else {
	*flags = ifr.ifr_flags;
    }

    close(fd);
    return (err);
}

static int
set_if_flags(char *ifname, unsigned long flags)
{
    int err = 0;
    int fd;
    struct ifreq ifr;

    /* Open up a socket so we can communicate */

    if ((fd = linux_open_socket()) < 0)
	return (-1);

    memset(&ifr, 0, sizeof(ifr));

    /* Copy the interface name over */
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_flags = flags;

    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
	perror("SET_IF_FLAGS (IOCTL)");
	err = -1;
    }

    close(fd);
    return (err);
}

int
linux_do_ip_address(int command, char *ifname, pix_comm_ipaddr_t * ipaddr)
{
    int err = PIX_COMM_IPADDR_ERROR;
    int fd;
    struct ifreq ifr;
    struct sockaddr_in *sin = (void *) &ifr.ifr_addr;

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, ifname);

    /* Open up a socket so we can communicate */
    if ((fd = linux_open_socket()) < 0)
	return (PIX_COMM_ERROR);

    switch (command) {
    case PIX_COMM_SET:
	linux_set_sockaddr((struct sockaddr *) sin, htonl(ipaddr->addr), 0);
	if (ioctl(fd, SIOCSIFADDR, &ifr) < 0) {
	    perror("SET_IP_ADDR (IOCTL)");
	    break;
	}

	linux_set_sockaddr((struct sockaddr *) sin, htonl(ipaddr->netmask),
			   0);
	if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0) {
	    perror("SET_IP_ADDR (IOCTL)");
	    break;
	}

	linux_set_sockaddr((struct sockaddr *) sin, htonl(ipaddr->broadcast),
			   0);
	if (ioctl(fd, SIOCSIFBRDADDR, &ifr) < 0) {
	    perror("SET_IP_ADDR (IOCTL)");
	    break;
	}

	err = PIX_COMM_OK;
	break;

    case PIX_COMM_GET:
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
	    perror("GET_IP_ADDR (IOCTL)");
	    break;
	}

	ipaddr->addr = ntohl(sin->sin_addr.s_addr);

	if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) {
	    perror("GET_IP_ADDR (IOCTL)");
	    break;
	}

	ipaddr->netmask = ntohl(sin->sin_addr.s_addr);

	if (ioctl(fd, SIOCGIFBRDADDR, &ifr) < 0) {
	    perror("GET_IP_ADDR (IOCTL)");
	    break;
	}

	ipaddr->broadcast = ntohl(sin->sin_addr.s_addr);

	err = PIX_COMM_OK;
	break;

    default:
	err = PIX_COMM_INVALID;
	break;
    }

    close(fd);
    return (err);
}

/* Local flags not used by the outside world */

#define DEFAULT_GATEWAY 0x01

static int
linux_add_route(unsigned long addr, unsigned long netmask,
		unsigned long gateway, char *interface)
{
    int err = PIX_COMM_OK;

    int fd;
    struct rtentry rm;
    char name[15];


    memset((char *) &rm, 0, sizeof(struct rtentry));

    rm.rt_flags = (RTF_UP | RTF_HOST);
    if (addr) {
	rm.rt_flags = RTF_HOST;
	linux_set_sockaddr(&(rm.rt_dst), addr, 0);
	linux_set_sockaddr(&(rm.rt_genmask), netmask, 0);
    } else {
	struct sockaddr mask;
	sprintf(name, "default");
	resolve(name, (struct sockaddr_in *) (&rm.rt_dst));
	sprintf(name, "%d.%d.%d.%d",
		(unsigned char) (gateway >> 24) & 0xFF,
		(unsigned char) (gateway >> 16) & 0xFF,
		(unsigned char) (gateway >> 8) & 0xFF,
		(unsigned char) gateway & 0xFF);
	resolve(name, (struct sockaddr_in *) (&rm.rt_gateway));
	resolve("0.0.0.0", (struct sockaddr_in *) (&mask));
	rm.rt_genmask = mask;
	rm.rt_flags &= ~RTF_HOST;
    }

    //linux_set_sockaddr(&(rm.rt_gateway), gateway, 0);

    if (gateway)
	rm.rt_flags |= RTF_GATEWAY;

    if (interface) {
	if (NULL == rm.rt_dev) {
	    rm.rt_dev = (char *) calloc(strlen(interface), sizeof(char));
	}
	strcpy(rm.rt_dev, interface);
    } else
	rm.rt_dev = "lo";

    /* Open up a socket so we can communicate */
    if ((fd = linux_open_socket()) < 0)
	return (PIX_COMM_ERROR);

    if (ioctl(fd, SIOCADDRT, &rm) < 0) {
	perror("ADD_ROUTE (IOCTL)");
	err = PIX_COMM_ERROR;
    }

    close(fd);
    if (NULL != rm.rt_dev) {
	free(rm.rt_dev);
    }
    return (err);
}

static int
linux_delete_route(unsigned long addr)
{
    int err = PIX_COMM_OK;
    int fd;
    struct rtentry rm;

    memset(&rm, 0, sizeof(rm));
    linux_set_sockaddr((struct sockaddr *) &rm.rt_dst, addr, 0);

    /* Open up a socket so we can communicate */
    if ((fd = linux_open_socket()) < 0)
	return (PIX_COMM_ERROR);

    if (ioctl(fd, SIOCDELRT, &rm) < 0) {
	perror("DEL_ROUTE (IOCTL)");
	err = PIX_COMM_ERROR;
    }

    close(fd);
    return (err);
}

#define ROUTE_PROC_FILE "/proc/net/route"

/* Here, gw is the returned value for addr */

static int
linux_get_route(unsigned long *gw, unsigned long addr)
{
    char buffer[BUFSIZ];

    FILE *wfile = fopen(ROUTE_PROC_FILE, "r");

    if (!wfile) {
	perror("GET_ROUTE (OPEN)");
	return (PIX_COMM_ERROR);
    }

    /* Skip the first line */
    fgets(buffer, BUFSIZ, wfile);

    /* Now, go through all the routes until we find the one that matches the flags */

    while (1) {
	char iface[15];
	unsigned long daddr, gaddr;
	int inflags;

	if (feof(wfile))
	    break;

	fgets(buffer, BUFSIZ, wfile);

	/* sscanf(buffer, "%s\t%08lX\t%08lX\t%8X\t%d\t%u\t%d\t08lX\t%d\t%u\t%d\n",
	   iface, &daddr, &addr); */

	sscanf(buffer, "%s\t%08lX\t%08lX\%8X",
	       iface, &daddr, &gaddr, &inflags);

	if (daddr == addr && (inflags & RTF_UP)) {
	    *gw = ntohl(gaddr);

	    fclose(wfile);
	    return (PIX_COMM_OK);
	}
    }

    fclose(wfile);
    return (PIX_COMM_ERROR);
}

int
linux_do_default_gateway(int command, unsigned long *addr, char *interface)
{
    unsigned long inaddr;

    switch (command) {
    case PIX_COMM_ADD:
	return (linux_add_route(0, 0, *addr, interface));

    case PIX_COMM_DEL:
	if (linux_get_route(&inaddr, 0) == PIX_COMM_ERROR)
	    return (PIX_COMM_ERROR);

	return (linux_delete_route(inaddr));
	break;

    case PIX_COMM_GET:
	return (linux_get_route(addr, 0));

    default:
	return (PIX_COMM_INVALID);
    }
}

int
linux_set_nameserver(char *filen, char *domain, char *search,
		     unsigned long *addr, int addrcount)
{
    int i;

    char outbuf[256];

    /* Open up the specified file */
    int fd = open(filen, O_RDWR | O_TRUNC);

    if (fd == -1)
	return (PIX_COMM_ERROR);

    if (0 != strcmp("", domain)) {
	sprintf(outbuf, "domain %s\n", domain);
	write(fd, outbuf, strlen(outbuf));
    }
    if (0 != strcmp("", search)) {
	sprintf(outbuf, "search %s\n", search);
	write(fd, outbuf, strlen(outbuf));
    }

    /* Now start writing all of the nameserver information out */

    for (i = 0; i < addrcount; i++) {
	sprintf(outbuf, "nameserver %d.%d.%d.%d\n",
		(unsigned char) (addr[i] >> 24) & 0xFF,
		(unsigned char) (addr[i] >> 16) & 0xFF,
		(unsigned char) (addr[i] >> 8) & 0xFF,
		(unsigned char) addr[i] & 0xFF);

	write(fd, outbuf, strlen(outbuf));
    }

    close(fd);
    return (PIX_COMM_OK);
}

int
linux_if_active(char *ifname)
{
    unsigned long flags;

    if (get_if_flags(ifname, &flags) == -1)
	return (PIX_COMM_INACTIVE);

    if (!(flags & IFF_UP))
	return (PIX_COMM_INACTIVE);
    else
	return (PIX_COMM_ACTIVE);
}

int
linux_if_dhcp(char *ifname)
{
    pid_t childpid;
    int err = 0;
    int child_status;
    int idx;

    if ((childpid = vfork()) == -1) {
	perror("IF_DHCP (FORK)");
	return (PIX_COMM_ERROR);
    } else if (childpid == 0)	// in child
    {
	for (idx = 3; idx < 20; idx++)
	    close(idx);
	//      err = execl("/sbin/pump", "/sbin/pump", "-i", ifname, NULL);

	//      err = execl("/bin/sh", "/bin/sh" "/etc/sysconfig/network-scripts/ifup", ifname, NULL);

	err =
	    execl("/bin/sh", "sh", "/etc/sysconfig/network-scripts/ifup",
		  ifname, NULL);

	//      err = execl("/bin/sh", "sh");

	if (0 > err)
	    perror("IF_DHCP (EXECL)");
	exit(1);
    } else			//in parent
    {
	wait(&child_status);
	if (PIX_COMM_INACTIVE == linux_if_active(ifname))
	    return (PIX_COMM_ERROR);
	else
	    return (PIX_COMM_OK);
    }
}

int
linux_set_if_status(int status, char *ifname)
{
    unsigned long flags;
    pid_t childpid;
    int err = 0;
    int child_status;
    int idx;

    if (get_if_flags(ifname, &flags) == -1)
	return (PIX_COMM_ERROR);

    if (status == PIX_COMM_ACTIVE)
	flags |= IFF_UP;
    else
	flags &= ~IFF_UP;

    if ((childpid = vfork()) == -1) {
	perror("IF_DHCP (FORK)");
	return (PIX_COMM_ERROR);
    } else if (childpid == 0)	// in child
    {
	for (idx = 3; idx < 20; idx++)
	    close(idx);
	//      err = execl("/sbin/pump", "/sbin/pump", "-i", ifname, NULL);

	//      err = execl("/bin/sh", "/bin/sh" "/etc/sysconfig/network-scripts/ifup", ifname, NULL);
	printf("ifdown *******\n");
	err =
	    execl("/bin/sh", "sh", "/etc/sysconfig/network-scripts/ifdown",
		  ifname, NULL);

	//      err = execl("/bin/sh", "sh");

	if (0 > err)
	    perror("IF_DHCP (EXECL)");
	exit(1);
    } else			//in parent
    {
	wait(&child_status);
	return (set_if_flags(ifname, flags));
    }
}

int
linux_get_if_list(int type, pix_comm_interface_t * iflist, int *ifcount)
{
    int retcount = -1;
    int err = PIX_COMM_ERROR;
    int fd, n;
    struct ifconf ifc;
    struct ifreq *ifr;

    int maxreq = *ifcount;	/* Maximum number of interfaces to return */

    /* We want to return a list of interfaces and some stats */
    fd = socket(PF_INET, SOCK_DGRAM, 0);

    if (fd == -1) {
	perror("GET_IF_LIST (SOCKET)");
	return (PIX_COMM_ERROR);
    }

    ifc.ifc_len = sizeof(struct ifreq) * maxreq;
    ifc.ifc_buf = (char *) malloc(ifc.ifc_len);

    if (!ifc.ifc_buf)
	return (PIX_COMM_ERROR);

    if (ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
	perror("GET_IF_LIST (IOCTL)");
	err = PIX_COMM_ERROR;
	goto out;
    }

    /* Now, go through and get the interface flags */

    ifr = ifc.ifc_req;

    for (n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq), ifr++) {
	unsigned long flags;

	/* Get the flags */
	if (get_if_flags(ifr->ifr_name, &flags) == 0) {
	    if (flags & IFF_LOOPBACK)
		continue;

	    if (type == PIX_COMM_TYPE_PPP && !(flags & IFF_POINTOPOINT))
		continue;

	    if (type == PIX_COMM_TYPE_ETHERNET && (flags & IFF_POINTOPOINT))
		continue;

	    retcount++;
	    strcpy(iflist[retcount].name, ifr->ifr_name);

	    if (flags & IFF_POINTOPOINT)
		iflist[retcount].type = PIX_COMM_TYPE_PPP;
	    else
		iflist[retcount].type = PIX_COMM_TYPE_ETHERNET;

	    if (flags & IFF_UP)
		iflist[retcount].active = 1;
	    else
		iflist[retcount].active = 0;
	}
    }

    if (retcount != -1) {
	err = PIX_COMM_OK;
	*ifcount = retcount;
    } else
	err = PIX_COMM_NO_INTERFACES;

  out:
    free(ifc.ifc_buf);
    return err;
}

int
linux_set_netscript_values(char *file, const char *keys[], const char *vals[],
			   int size)
{

    FILE *net_file;
    int i = 0;

    net_file = fopen(file, "w");
    if (NULL == net_file) {
	fprintf(stderr, "Unable to open file %s\n", file);
	return (PIX_COMM_ERROR);
    }

    while (i < size) {
	fputs(keys[i], net_file);
	fputs("=", net_file);
	fputs(vals[i], net_file);
	fputs("\n", net_file);
	i++;
    }

    fclose(net_file);
    return (PIX_COMM_OK);
}

int
linux_get_netscript_values(char *file, char *keys[], char *vals[], int size)
{

    FILE *net_file;
    char buf[255];
    char key_buf[255];
    char val_buf[255];
    int i = 0, j = 0, k = 0;
    int len = 0;
    char *str_p = NULL, *eq_p = NULL;

    net_file = fopen(file, "r");
    if (NULL == net_file) {
	fprintf(stderr, "Unable to open file %s\n", file);
	return (PIX_COMM_ERROR);
    }

    while (NULL != fgets(buf, sizeof(buf) - 1, net_file) && (i < size)) {

	/* Verify line as a "=" */
	eq_p = strstr(buf, "=");
	if (NULL == eq_p) {
	    fprintf(stderr, "Invalid key=value pairs.\n");
	    continue;
	}

	if ('#' != buf[0]) {

	    j = 0;
	    key_buf[j] = '\0';
	    val_buf[j] = '\0';

	    /* Get keys */
	    str_p = buf;
	    while (('=' != *str_p) && (NULL != str_p)) {
		key_buf[j] = *str_p;
		str_p++;
		j++;
	    }
	    key_buf[j] = '\0';
	    strcpy(keys[i], key_buf);

	    /* Increment past the "=" */
	    str_p = strstr(buf, "=");
	    str_p++;

	    /* Get values */
	    len = strlen(str_p);
	    for (k = 0; k < len; k++) {
		if (('\n' != *str_p) && (NULL != str_p)) {
		    val_buf[k] = *str_p;
		    str_p++;
		} else {
		    val_buf[k] = '\0';
		}
	    }
	    strcpy(vals[i], val_buf);
	}

	i++;			/* increment to next line in file */
    }

    fclose(net_file);
    return (PIX_COMM_OK);

}
