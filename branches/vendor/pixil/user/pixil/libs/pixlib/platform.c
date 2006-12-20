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

#include <pixlib/pixlib.h>
#include <linux_if.h>

int
pix_set_nameserver(char *domain, char *search, unsigned long *addr,
		   int acount)
{
    /* On the ipaq, we can only write to the /usr/local domain */
    /* so we have a symbolic link from /etc/resolv.conf to     */
    /* /usr/local/etc/resolv.conf, so thats the file we update here */

    return (linux_set_nameserver(RESOLV_FILE, domain, search, addr, acount));
}

int
pix_write_net_values(pix_sys_ipaddr_str_t ip_info, pix_sys_dns_t dns_info)
{
    FILE *net_file;
    unsigned long dns_addr[2];
    int dns_count = 0;
    int ret = 0;

    net_file = fopen(ETH0_FILE, "w");
    if (NULL == net_file) {
	fprintf(stderr, "Unbale to open file %s\n", ETH0_FILE);
	return (PIX_SYS_ERROR);
    }
    fputs("DEVICE=\"eth0\"\n", net_file);
    fputs("NETMASK=\"", net_file);
    fputs(ip_info.netmask, net_file);
    fputs("\"\n", net_file);
    fputs("ONBOOT=\"yes\"\n", net_file);
    if (1 == ip_info.dhcp) {
	fputs("IPADDR=\"\"\n", net_file);
	fputs("BOOTPROTO=\"dhcp\"\n", net_file);
    } else {
	fputs("IPADDR=\"", net_file);
	fputs(ip_info.addr, net_file);
	fputs("\"\n", net_file);
	fputs("BOOTPROTO=\"static\"\n", net_file);
    }
    fclose(net_file);

    if (0 != strcmp("...", dns_info.str_dns_1)) {
	if (0 != strcmp("", dns_info.str_dns_1)) {
	    dns_addr[dns_count] = dns_info.long_dns_1;
	    dns_count++;
	}
    }
    if (0 != strcmp("...", dns_info.str_dns_2)) {
	if (0 != strcmp("", dns_info.str_dns_2)) {
	    dns_addr[dns_count] = dns_info.long_dns_2;
	    dns_count++;
	}
    }
    if (0 != strcmp("...", dns_info.str_dns_3)) {
	if (0 != strcmp("", dns_info.str_dns_3)) {
	    dns_addr[dns_count] = dns_info.long_dns_3;
	    dns_count++;
	}
    }
    ret =
	pix_set_nameserver((char *) dns_info.domain, (char *) dns_info.search,
			   dns_addr, dns_count);

    net_file = fopen(GATEWAY_FILE, "w+");
    if (NULL == net_file) {
	fprintf(stderr, "Unbale to open file %s\n", GATEWAY_FILE);
	return (PIX_SYS_ERROR);
    }
    if (0 == strcmp("...", ip_info.gateway)) {
	fputs("", net_file);
    } else {
	fputs(ip_info.gateway, net_file);
    }
    fclose(net_file);
    return (PIX_SYS_OK);
}

int
pix_get_net_value(char *key, char *ret)
{
    FILE *net_file;
    char buf[255];
    char *str_p;
    char ret_buf[255];
    int len = 0;
    int idx = 0;
    int nameserver = 0;
    int target = 0;

    if (0 == strcasecmp(key, "DHCP")) {
	key = "BOOTPROTO";
	net_file = fopen(ETH0_FILE, "r");
	if (NULL == net_file) {
	    fprintf(stderr, "Unbale to open file %s\n", ETH0_FILE);
	    ret = "";
	    return (PIX_SYS_ERROR);
	}
	goto equal;
    } else {
	net_file = fopen(RESOLV_FILE, "r");
	if (NULL == net_file) {
	    fprintf(stderr, "Unbale to open file %s\n", ETH0_FILE);
	    ret = "";
	    return (PIX_SYS_ERROR);
	}
	goto space;
    }
    ret = "";

  equal:			//has an = for delim
    while (NULL != fgets(buf, sizeof(buf) - 1, net_file)) {
	if ((str_p = strstr(buf, key)) && ('#' != buf[0])) {
	    ret_buf[0] = '\0';
	    str_p = strstr(buf, "=");
	    if (NULL != str_p) {
		str_p++;
		if ('\"' == *str_p) {
		    str_p++;
		    if ('\"' == *str_p || '\n' == *str_p) {
			fclose(net_file);
			ret = "";
			return (PIX_SYS_OK);
		    }
		}
		len = strlen(str_p);
		for (idx = 0; idx < len; idx++) {
		    if (('\"' != *str_p) && ('\n' != *str_p)
			&& (NULL != str_p)) {
			ret_buf[idx] = *str_p;
			str_p++;
		    } else {
			ret_buf[idx] = '\0';
			break;
		    }
		}
	    }
	    fclose(net_file);
	    strncpy(ret, ret_buf, strlen(ret_buf) + 1);
	    if (0 == strcasecmp(key, "BOOTPROTO")) {
		if (0 == strcasecmp(ret, "DHCP")) {
		    strncpy(ret, "Y", strlen(ret_buf) + 1);
		}
	    }
	    break;
	}
    }

  space:			// has a " " space for a delim
    while (NULL != fgets(buf, sizeof(buf) - 1, net_file)) {
	if (0 == strcmp(key, "DNS_1")) {
	    target = 1;
	    key = "nameserver";
	}
	if (0 == strcmp(key, "DNS_2")) {
	    target = 2;
	    key = "nameserver";
	}
	if (0 == strcmp(key, "DNS_3")) {
	    target = 3;
	    key = "nameserver";
	}
	if (0 == strncasecmp(key, buf, strlen(key))) {
	    if (0 == strcmp(key, "nameserver")) {
		nameserver++;
		if (nameserver != target) {
		    continue;
		}
	    }
	    idx = 1;
	    str_p = strstr(buf, " ");
	    if (NULL != str_p) {
		str_p++;
		len = strlen(str_p);
		for (idx = 0; idx < len; idx++) {
		    ret_buf[idx] = *str_p;
		    str_p++;
		}
	    }
	    ret_buf[idx - 1] = '\0';
	    fclose(net_file);
	    strncpy(ret, ret_buf, strlen(ret_buf) + 1);
	    return (PIX_SYS_OK);
	}
    }
    return (PIX_SYS_OK);
}
