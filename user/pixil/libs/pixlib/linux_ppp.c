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
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <net/if.h>
#include <net/ppp_defs.h>
#include <net/if_ppp.h>

#include <netinet/in.h>

#include <pixlib/pixlib.h>
#include <linux_if.h>
#include <linux_ppp.h>

extern int linux_open_socket();
extern int get_if_flags(char *ifname, unsigned long *flags);

typedef struct
{
    char keyword[15];
    char *value;
}
keywords_t;


/* This function replaces various keywords in a given script with the correct */
/* options */

static void
replace_keywords(keywords_t * keywords, int kcount,
		 const char *incmd, char *outcmd)
{
    const char *inptr = incmd;
    char *outptr = outcmd;

    /* Go through and look for a $.  When one is found, handle it */

    while (1) {
	for (; *inptr != '$' && *inptr; inptr++)
	    *outptr++ = *inptr;

	if (!*inptr) {
	    *outptr = 0;
	    return;
	}

	if (*inptr == '$') {
	    int i;

	    inptr++;

	    for (i = 0; i < kcount; i++) {
		if (!strncmp
		    (inptr, keywords[i].keyword,
		     strlen(keywords[i].keyword))) {
		    strcat(outptr, keywords[i].value);
		    inptr += strlen(keywords[i].value);
		    outptr += strlen(keywords[i].value);
		    break;
		}
	    }
	}
    }
}

static inline void
addr_to_ascii(unsigned long addr, char *str)
{
    sprintf(str, "%d.%d.%d.%d",
	    (unsigned char) (addr >> 24) & 0xFF,
	    (unsigned char) (addr >> 16) & 0xFF,
	    (unsigned char) (addr >> 8) & 0xFF, (unsigned char) addr & 0xFF);
}

#define MODE_SRDWR S_IRUSR|S_IWUSR
int
ppp_write_secrets(pix_comm_ppp_options_t * ppp_options)
{
    char *chap_file = "/etc/ppp/chap-secrets";
    char *pap_file = "/etc/ppp/pap-secrets";
    FILE *fp;
    char buf[255];

    fp = fopen(chap_file, "w");
    if (NULL == fp) {
	perror("PPP_WRITE_SECRETS (FOPEN)");
	return (PIX_COMM_ERROR);
    }
    fputs("# Secrets for authentication using PAP\n", fp);
    fputs("# client\tserver\tsecret\t\t\tIP addresses\n", fp);
    sprintf(buf, "\"%s\"\t*\t\"%s\"", ppp_options->account,
	    ppp_options->password);
    fputs(buf, fp);
    fclose(fp);
    fp = fopen(pap_file, "w");
    if (NULL == fp) {
	perror("PPP_WRITE_SECRETS (FOPEN)");
	return (PIX_COMM_ERROR);
    }
    fputs("# Secrets for authentication using PAP\n", fp);
    fputs("# client\tserver\tsecret\t\t\tIP addresses\n", fp);
    sprintf(buf, "\"%s\"\t*\t\"%s\"", ppp_options->account,
	    ppp_options->password);
    fputs(buf, fp);
    fclose(fp);
    chmod(chap_file, MODE_SRDWR);
    chmod(pap_file, MODE_SRDWR);
    return (PIX_COMM_OK);
}

int
locate_chat_bin(char *bin)
{
    struct stat stat_buf;
    char buf[255];
    int err;

    sprintf(buf, "/usr/sbin/chat");
    err = stat(buf, &stat_buf);
    if (err == 0) {
	strcpy(bin, buf);
	return (PIX_COMM_OK);
    }
    sprintf(buf, "/sbin/chat");
    err = stat(buf, &stat_buf);
    if (err == 0) {
	strcpy(bin, buf);
	return (PIX_COMM_OK);
    }
    sprintf(buf, "/root/chat");
    err = stat(buf, &stat_buf);
    if (err == 0) {
	strcpy(bin, buf);
	return (PIX_COMM_OK);
    }
    return (PIX_COMM_ERROR);
}


int
ppp_write_peer(pix_comm_ppp_options_t * ppp_options, char *config)
{
    char peer_file[255];
    char buf[255];
    FILE *fp;
    char chat_bin[255];
    int err;

    err = locate_chat_bin(chat_bin);
    if (0 > err) {
	return (PIX_COMM_ERROR);
    }
    sprintf(peer_file, "/etc/ppp/peers/%s", config);
    fp = fopen(peer_file, "w");
    if (NULL == peer_file) {
	perror("PPP_WRITE_PEER (FOPEN)");
	return (PIX_COMM_ERROR);
    }
    sprintf(buf,
	    "%s %d -detach crtscts defaultroute usepeerdns -chap user \"%s\"\n",
	    ppp_options->device, ppp_options->speed, ppp_options->account);
    fputs(buf, fp);
    sprintf(buf, "connect '%s -v -f /tmp/ppp/chat-%s'\n", chat_bin, config);
    fputs(buf, fp);
    sprintf(buf, "noauth\n");
    fputs(buf, fp);
    fclose(fp);
    return (PIX_COMM_OK);
}

#define SCRIPT_PRE "/tmp/ppp/chat-"
#define MODE_RDWR  S_IROTH|S_IWOTH|S_IRGRP|S_IWGRP|S_IRUSR|S_IWUSR
int
ppp_write_default_script(pix_comm_ppp_options_t * ppp_options, char *config)
{
    char buf[255];
    char chat_file[255];
    FILE *fp;

    sprintf(chat_file, "%s%s", SCRIPT_PRE, config);
    fp = fopen(chat_file, "w");
    if (NULL == fp) {
	perror("PPP_WRITE_SCRIPT (FOPEN)");
	return (PIX_COMM_ERROR);
    }
    sprintf(buf, "ABORT \"NO CARRIER\"\nABORT \"NO DIALTONE\"\n");
    fputs(buf, fp);
    sprintf(buf, "ABORT \"ERROR\"\nABORT \"NO ANSWER\"\nABORT \"BUSY\"\n");
    fputs(buf, fp);
    sprintf(buf, "ABORT \"Username/Password Incorrect\"\n");
    fputs(buf, fp);
    sprintf(buf, "'' 'AT'\n'OK' 'ATM0L0'\n 'OK' 'ATDT%s'\n",
	    ppp_options->telephone);
    fputs(buf, fp);
    sprintf(buf, "'CONNECT' ''\n'name:' '%s'\n", ppp_options->account);
    fputs(buf, fp);
    sprintf(buf, "'ord:' '%s'\n", ppp_options->password);
    fputs(buf, fp);
    sprintf(buf, "'TIMEOUT' '5'\n");
    fputs(buf, fp);
    sprintf(buf, "'~--' ''\n");
    fputs(buf, fp);
    fclose(fp);
    chmod(chat_file, MODE_RDWR);
    return (PIX_COMM_OK);
}

int
ppp_write_script(pix_comm_ppp_options_t * ppp_options, char *config,
		 char **pppd_commands, int count)
{
    char chat_file[255];
    FILE *fp;
    int idx;

    sprintf(chat_file, "%s%s", SCRIPT_PRE, config);
    fp = fopen(chat_file, "w");
    if (NULL == fp) {
	perror("PPP_WRITE_SCRIPT (FOPEN)");
	return (PIX_COMM_ERROR);
    }
    for (idx = 0; idx < count; idx++) {
	fputs(pppd_commands[idx], fp);
	fputs("\n", fp);
    }
    fclose(fp);
    chmod(chat_file, MODE_RDWR);
    return (PIX_COMM_OK);
}

int
locate_pppd_bin(char *bin)
{
    int err;
    char buf[255];
    struct stat stat_buf;

    sprintf(buf, "/usr/sbin/pppd");
    err = stat(buf, &stat_buf);
    if (0 == err) {
	strcpy(bin, buf);
	return (PIX_COMM_OK);
    }
    sprintf(buf, "/sbin/pppd");
    err = stat(buf, &stat_buf);
    if (0 == err) {
	strcpy(bin, buf);
	return (PIX_COMM_OK);
    }
    return (PIX_COMM_ERROR);
}

#define PPPD_STR 		"-detach defaultroute lock lcp-echo-interval 5 lcp-echo-failure 3 noipdefault"
#define PPPD_SETUP 	"-detach noauth nocrtscts lock"
#define PPPD_OGIN  	"-v -t3 ogin--ogin: ppp"
static int
ppp_nullmodem_connect(pix_comm_ppp_options_t * ppp_options)
{
    char ipaddr[25];
    char cmds[128];
    char pppd_command[1024];
    char *pptr = pppd_command;
    pid_t childpid;
    int err, tries;
    int max_tries = 10;
    int idx;
    /* Determine the commands we want to send to the pppd */
    cmds[0] = 0;

    if (ppp_options->flags & PIX_COMM_PPP_AUTH)
	strcat(cmds, "auth ");
    else
	strcat(cmds, "noauth ");

    if (ppp_options->flags & PIX_COMM_PPP_HWFLOW)
	strcat(cmds, "ctrscts ");
    else
	strcat(cmds, "nocrtscts ");

    sprintf(pptr, "pppd %s %s %s %d ",
	    PPPD_STR, cmds, ppp_options->device, ppp_options->speed);

    if (ppp_options->local_ipaddr || ppp_options->remote_ipaddr) {
	addr_to_ascii(ppp_options->local_ipaddr, ipaddr);
	strcat(pptr, ipaddr);
	strcat(pptr, ":");
	addr_to_ascii(ppp_options->remote_ipaddr, ipaddr);
	strcat(pptr, ipaddr);
    }

    if ((childpid = vfork()) == -1) {
	perror("PPP_MODEM_CONNECT (FORK)");
	return (PIX_COMM_ERROR);
    } else if (childpid == 0) {	// in child
	for (idx = 3; idx < 20; idx++)
	    close(idx);
	err = system(pptr);
	if (0 > err)
	    perror("PPP_MODEM_CONNECT (SYSTEM)");
	exit(1);
    } else {			//in parent
	//wait(&child_status);
	tries = 0;
	while ((PIX_COMM_INACTIVE == linux_if_active("ppp0"))
	       && (tries < max_tries)) {
	    tries++;
	    sleep(3);
	}
	if (PIX_COMM_INACTIVE == linux_if_active("ppp0")) {
	    linux_ppp_disconnect("ppp0");
	    return (PIX_COMM_ERROR);
	} else
	    return (PIX_COMM_OK);
    }
    return (PIX_COMM_OK);
}

/*
 *
 */
int
ppp_modem_connect(pix_comm_ppp_options_t * ppp_options,
		  char **ppp_commands, int ccount, int dhcp, char *config)
{
    int i;

    keywords_t ppp_keyword[] = { {"TELEPHONE", ppp_options->telephone},
    {"ACCOUNT", ppp_options->account},
    {"PASSWORD", ppp_options->password}
    };

    char pppd_command[4500];
    char connect_script[4096];
    char ipaddr[25];
    char pppd_bin[255];
    char chat_bin[255];

    char *pptr = pppd_command;
    char *cptr = connect_script;

    pid_t childpid;
    int err;
    int max_tries = 20;
    int tries = 0;
    int idx;
    /* Determine where pppd and chat live */
    err = locate_pppd_bin(pppd_bin);
    if (0 > err)
	return (PIX_COMM_ERROR);

    /* Construct the output script */
    if (1 == dhcp && NULL == ppp_commands) {
	err = ppp_write_secrets(ppp_options);
	if (err)
	    return (PIX_COMM_ERROR);
	err = ppp_write_peer(ppp_options, config);
	if (err)
	    return (PIX_COMM_ERROR);
	err = ppp_write_default_script(ppp_options, config);
	if (err)
	    return (PIX_COMM_ERROR);
	sprintf(pptr, "%s call %s", pppd_bin, config);

    } else if (1 == dhcp && NULL != ppp_commands) {
	err = ppp_write_secrets(ppp_options);
	if (err)
	    return (PIX_COMM_ERROR);
	err = ppp_write_peer(ppp_options, config);
	if (err)
	    return (PIX_COMM_ERROR);
	err = ppp_write_script(ppp_options, config, ppp_commands, ccount);
	if (err)
	    return (PIX_COMM_ERROR);
	sprintf(pptr, "%s call %s", pppd_bin, config);
    } else if (0 == dhcp && NULL == ppp_commands) {

	err = ppp_nullmodem_connect(ppp_options);
	if (0 > err)
	    return (PIX_COMM_ERROR);
	else
	    return (PIX_COMM_OK);
    } else {
	for (i = 0; i < ccount; i++) {
	    char cline[256];
	    replace_keywords(ppp_keyword, 3, ppp_commands[i], cline);
	    strcat(cptr, cline);
	}

	sprintf(pptr, "pppd %s %d ", ppp_options->device, ppp_options->speed);

	addr_to_ascii(ppp_options->local_ipaddr, ipaddr);
	strcat(pptr, ipaddr);

	/* Add the remote address */

	addr_to_ascii(ppp_options->remote_ipaddr, ipaddr);
	strcat(pptr, ":");
	strcat(pptr, ipaddr);
	strcat(pptr, " ");

	strcat(pptr, PPPD_SETUP);
	strcat(pptr, " ");

	/* Add the connect address */
	strcat(pptr, "connect '");
	err = locate_chat_bin(chat_bin);
	if (err != 0)
	    return (PIX_COMM_ERROR);
	strcat(pptr, chat_bin);
	strcat(pptr, " ");
	strcat(pptr, PPPD_OGIN);
	strcat(pptr, "'");
    }
    /* Fire away! */
    if ((childpid = vfork()) == -1) {
	perror("PPP_MODEM_CONNECT (FORK)");
	return (PIX_COMM_ERROR);
    } else if (childpid == 0) {	// in child
	/* system(pppd_command); */
	err = locate_pppd_bin(pppd_bin);
	if (0 > err)
	    return (PIX_COMM_ERROR);
	for (idx = 3; idx < 20; idx++)
	    close(idx);
	err = system(pptr);
	if (0 > err)
	    perror("PPP_MODEM_CONNECT (SYSTEM)");
	exit(1);
    } else {			//in parent
	//wait(&child_status);
	tries = 0;
	while ((PIX_COMM_INACTIVE == linux_if_active("ppp0"))
	       && (tries < max_tries)) {
	    tries++;
	    sleep(3);
	}
	if (PIX_COMM_INACTIVE == linux_if_active("ppp0")) {
	    linux_ppp_disconnect("ppp0");
	    return (PIX_COMM_ERROR);
	} else
	    return (PIX_COMM_OK);
    }

    return (PIX_COMM_OK);
}

/* Here we need to define the various commands that are required */
/* This is the current standard for the ipaq, but they should probably */
/* be passed in by a client */



/* For Century Software PPP connections, we have a script that 
 * gets called when IP goes up and down.  It writes a file in 
 * /tmp called censoft-ppp-<dev>.info.  Thats what we read to
 * get the IP info to return to the system.  
 */

#ifdef NOTUSED

static int
read_line(char **ptr, char *str, int maxlen)
{
    char *tptr = *ptr;
    int len;

    for (; *tptr != '\n' && *tptr; tptr++);
    len = (int) (tptr - *ptr);

    strncpy(str, *ptr, len);
    *ptr = tptr + 1;
    return (len);
}
#endif

int
linux_ppp_get_interface(char *device, char *ifname, int ifnamesize)
{
    char *dptr;
    char filen[BUFSIZ];
    int fd;
    ssize_t len;

    /* First, strip everything except for the end device name off */
    for (dptr = device + strlen(device); *dptr != '/' && dptr != device;
	 dptr--);

    if (dptr != device)
	dptr++;

    /* Now try to open up the info file */
    sprintf(filen, "/tmp/censoft-ppp-%s.info", dptr);
    fd = open(filen, O_RDONLY);

    if (fd == -1)
	return (PIX_COMM_INACTIVE);

    /* We write lots of stuff in that file, but for now we really only care */
    /* about the interface name, since we can get everything else from that */

    len = read(fd, ifname, ifnamesize - 1);
    close(fd);

    if (len == -1)
	return (PIX_COMM_ERROR);
    else {
	ifname[len] = 0;
	return (PIX_COMM_OK);
    }
}

/* *********** API CALLS ***********/

int
linux_ppp_connect(int type, pix_comm_ppp_options_t * ppp_options,
		  char **ppp_commands, int ccount, int dhcp, char *config)
{
    switch (type) {
    case PIX_COMM_PPP_NULLMODEM:
	return (ppp_nullmodem_connect(ppp_options));

    case PIX_COMM_PPP_MODEM:
	return (ppp_modem_connect
		(ppp_options, ppp_commands, ccount, dhcp, config));
    }

    return (PIX_COMM_ERROR);
}

/*
 * linux_read_ppp_info()
 * Read in PPP info
 */

static void
strip_newline(char *src, char *dest)
{
    char *sptr = src;
    char *dptr = dest;

    for (; *sptr != 0 && *sptr != '\n'; sptr++, dptr++)
	*dptr = *sptr;

    *dptr = 0;
}

int
linux_ppp_read_info(char *filename, pix_comm_ppp_info_t * ppp_info)
{
    int err = PIX_COMM_ERROR;

    char buffer[512];
    FILE *stream = 0;

    stream = fopen(filename, "r");

    if (!stream)
	return (PIX_COMM_ERROR);

    fgets(buffer, sizeof(buffer) - 1, stream);
    if (ferror(stream))
	goto end;
    strip_newline(buffer, ppp_info->device);

    fgets(buffer, sizeof(buffer) - 1, stream);
    if (ferror(stream))
	goto end;
    strip_newline(buffer, ppp_info->ifname);

    fgets(buffer, sizeof(buffer) - 1, stream);
    if (ferror(stream))
	goto end;
    strip_newline(buffer, ppp_info->remote_ipaddr);

    fgets(buffer, sizeof(buffer) - 1, stream);
    if (ferror(stream))
	goto end;
    strip_newline(buffer, ppp_info->local_ipaddr);

    fgets(buffer, sizeof(buffer) - 1, stream);
    if (ferror(stream))
	goto end;
    ppp_info->speed = atoi(buffer);

    fgets(buffer, sizeof(buffer) - 1, stream);
    if (ferror(stream))
	goto end;
    ppp_info->start_time = atoi(buffer);

    err = PIX_COMM_OK;
  end:
    fclose(stream);
    return (err);
}

/*
 * linux_write_ppp_info()
 * Write the specified information to a file 
 */

int
linux_ppp_write_info(char *filename, pix_comm_ppp_info_t * ppp_info)
{
    char outbuf[sizeof(ppp_info) + 20];

    int fd, len;

    fd = open(filename, O_WRONLY);

    if (fd == -1)
	return (PIX_COMM_ERROR);

    sprintf(outbuf, "%s\n%s\n%s\n%s\n%d\n%ld\n",
	    ppp_info->device, ppp_info->ifname, ppp_info->remote_ipaddr,
	    ppp_info->local_ipaddr, ppp_info->speed, ppp_info->start_time);

    len = write(fd, &outbuf, strlen(outbuf));

    close(fd);

    if (len != strlen(outbuf))
	return (PIX_COMM_ERROR);
    else
	return (PIX_COMM_OK);
}

/*
 * linux_ppp_get_stats()
 * Given an interface name, return the statistics 
 */

int
linux_ppp_get_stats(char *ifname, pix_comm_ppp_stats_t * ppp_stats)
{
    int fd;
    unsigned long flags;
    struct ifpppstatsreq req;

    ppp_stats->in_bytes = 0;
    ppp_stats->out_bytes = 0;

    if (get_if_flags(ifname, &flags) == -1)
	return (PIX_COMM_ERROR);

    if (!(flags & IFF_UP))
	return (PIX_COMM_ERROR);

    req.stats_ptr = (caddr_t) & req.stats;

    strncpy(req.ifr__name, ifname, sizeof(req.ifr__name));

    /* Open up a socket so we can communicate */

    if ((fd = linux_open_socket()) < 0)
	return (PIX_COMM_ERROR);

    if (ioctl(fd, SIOCGPPPSTATS, &req) != 0)
	return (PIX_COMM_ERROR);

    ppp_stats->in_bytes = (unsigned long) req.stats.p.ppp_ibytes;
    ppp_stats->out_bytes = (unsigned long) req.stats.p.ppp_obytes;

    return (PIX_COMM_OK);
}

/*
 * linux_ppp_disconnect()
 * Given an interface name, shut down the PPPD 
 */

int
linux_ppp_disconnect(char *ifname)
{
    char buf[15];
    char filename[128];
    int fd;
    int err;

    /* Look for the pid file */

    if (!ifname)
	sprintf(filename, "/var/run/%s.pid", "ppp0");
    else
	sprintf(filename, "/var/run/%s.pid", ifname);

    fd = open(filename, O_RDONLY);

    if (fd == -1) {
	if (errno != EACCES)
	    return (PIX_COMM_ERROR);
	else
	    return (PIX_COMM_OK);
    }

    err = -1;

    if (read(fd, &buf, sizeof(buf)))
	err = kill(atoi(buf), SIGINT);

    close(fd);

    /* If there was an error, then erase the file */

    if (err == -1)
	unlink(filename);

    return (PIX_COMM_OK);
}

int
linux_ppp_ip_address(char *ifname, pix_comm_ppp_options_t * ppp_options)
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
    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
	perror("GET_IP_ADDR (IOCTL)");
	err = PIX_COMM_INVALID;
    }
    ppp_options->local_ipaddr = ntohl(sin->sin_addr.s_addr);

    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0) {
	perror("GET_IP_ADDR (IOCTL)");
	err = PIX_COMM_INVALID;
    }
    ppp_options->netmask = ntohl(sin->sin_addr.s_addr);


    if (ioctl(fd, SIOCGIFDSTADDR, &ifr) < 0) {
	perror("GET_DSTIP_ADDR (IOCTL)");
	err = PIX_COMM_INVALID;
    }
    ppp_options->remote_ipaddr = ntohl(sin->sin_addr.s_addr);
    close(fd);
    return (err);
}
