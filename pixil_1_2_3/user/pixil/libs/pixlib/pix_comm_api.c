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
#include <pixlib/pixlib.h>
#include <pixlib/pix_comm_proto.h>
#include <linux_if.h>
#include <linux_wireless.h>

/* This is a structure (defined elsewhere for each 
 * individual platform), that give us a gateway to the
 * actualy system dependent functions for each API 
 * call 
 */

extern pix_comm_func_t pix_comm_functions;

/*
 * pix_comm_set_ip_address()
 * This function sets the IP settings for the specified interface 
 */

int
pix_comm_set_ip_address(char *ifr, pix_comm_ipaddr_t * ip_addr)
{
    if (pix_comm_functions.sysdep_do_ip_address)
	return (pix_comm_functions.
		sysdep_do_ip_address(PIX_COMM_SET, ifr, ip_addr));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_get_ip_address(char *ifr, pix_comm_ipaddr_t * ip_addr)
{
    if (pix_comm_functions.sysdep_do_ip_address)
	return (pix_comm_functions.
		sysdep_do_ip_address(PIX_COMM_GET, ifr, ip_addr));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

/*
 * pix_comm_set_nameserver()
 * This function sets the nameserver address and domain
 */

int
pix_comm_set_nameserver(char *domain, char *search, unsigned long *addr,
			int acount)
{
    if (pix_comm_functions.sysdep_set_nameserver)
	return (pix_comm_functions.
		sysdep_set_nameserver(domain, search, addr, acount));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

/*
 * pix_comm_add_default_gateway()
 * This function adds a IP default gateway to the system
 */

int
pix_comm_add_default_gateway(unsigned long addr, char *interface)
{
    if (pix_comm_functions.sysdep_do_default_gateway)
	return (pix_comm_functions.
		sysdep_do_default_gateway(PIX_COMM_ADD, &addr, interface));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_remove_default_gateway()
{
    if (pix_comm_functions.sysdep_do_default_gateway)
	return (pix_comm_functions.
		sysdep_do_default_gateway(PIX_COMM_DEL, 0, 0));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_get_default_gateway(unsigned long *addr)
{
    if (pix_comm_functions.sysdep_do_default_gateway)
	return (pix_comm_functions.
		sysdep_do_default_gateway(PIX_COMM_GET, addr, 0));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}


int
pix_comm_ppp_connect(int ctype, pix_comm_ppp_options_t * opts, char **cmds,
		     int cmdcount, int dhcp, char *config)
{
    if (pix_comm_functions.sysdep_ppp_connect)
	return (pix_comm_functions.
		sysdep_ppp_connect(ctype, opts, cmds, cmdcount, dhcp,
				   config));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_ppp_disconnect(char *ifname)
{
    if (pix_comm_functions.sysdep_ppp_disconnect)
	return (pix_comm_functions.sysdep_ppp_disconnect(ifname));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_get_if_list(int filter, pix_comm_interface_t * iflist, int *ifcount)
{
    if (pix_comm_functions.sysdep_get_if_list)
	return (pix_comm_functions.
		sysdep_get_if_list(filter, iflist, ifcount));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_if_active(char *ifname)
{
    if (pix_comm_functions.sysdep_if_active)
	return (pix_comm_functions.sysdep_if_active(ifname));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_if_up(char *ifname, int dhcp)
{
    if (0 == dhcp) {
	if (pix_comm_functions.sysdep_set_if_status)
	    return (pix_comm_functions.
		    sysdep_set_if_status(PIX_COMM_ACTIVE, ifname));
	else
	    return (PIX_COMM_NOT_IMPLEMENTED);
    } else {
	return (linux_if_dhcp(ifname));
	//      if (pix_comm_functions.sysdep_if_dhcp)
	//      return(linux_if_dhcp(ifname));
	//      else
	//      return(PIX_COMM_NOT_IMPLEMENTED);
    }
}

int
pix_comm_if_down(char *ifname)
{
    if (pix_comm_functions.sysdep_set_if_status)
	return (pix_comm_functions.
		sysdep_set_if_status(PIX_COMM_INACTIVE, ifname));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_ppp_get_stats(char *ifname, pix_comm_ppp_stats_t * pppstats)
{
    if (pix_comm_functions.sysdep_ppp_get_stats)
	return (pix_comm_functions.sysdep_ppp_get_stats(ifname, pppstats));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_ppp_write_info(char *filename, pix_comm_ppp_info_t * pppinfo)
{
    if (pix_comm_functions.sysdep_ppp_write_info)
	return (pix_comm_functions.sysdep_ppp_write_info(filename, pppinfo));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_ppp_read_info(char *filename, pix_comm_ppp_info_t * pppinfo)
{
    if (pix_comm_functions.sysdep_ppp_read_info)
	return (pix_comm_functions.sysdep_ppp_read_info(filename, pppinfo));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_ppp_get_ip_info(char *ifname, pix_comm_ppp_options_t * pppoptions)
{
    if (pix_comm_functions.sysdep_ppp_get_ip_info)
	return (pix_comm_functions.
		sysdep_ppp_get_ip_info(ifname, pppoptions));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

/* Wireless Active? */
int
pix_comm_wireless_get_name(char *interface, char *name)
{
    return (wireless_name(interface, name));
    /*
       if (pix_comm_functions.sysdep_wireless_name) {
       //    return(pix_comm_functions.sysdep_wireless_name(interface, name));
       return(wireless_name(interface, name));
       } else {
       return(PIX_COMM_NOT_IMPLEMENTED);
       }
       ******* */

}

/* WEP */
int
pix_comm_wireless_set_encode(char *interface, char *name)
{
    return (wireless_encode(PIX_COMM_SET, interface, name));
    /*
       if (pix_comm_functions.sysdep_wireless_encode)
       return(wireless_encode(PIX_COMM_SET, interface, name));
       else
       return(PIX_COMM_NOT_IMPLEMENTED);
     */
}

int
pix_comm_wireless_get_encode(char *interface, char *name)
{
    return (wireless_encode(PIX_COMM_GET, interface, name));
    /*
       if (pix_comm_functions.sysdep_wireless_encode)
       return(wireless_encode(PIX_COMM_GET, interface, name));
       else
       return(PIX_COMM_NOT_IMPLEMENTED);
     */
}

int
pix_comm_wireless_set_essid(char *interface, char *name)
{
    printf("2: pix_com_wireless_set_essid ... %s\n", name);
    return (wireless_essid(PIX_COMM_SET, interface, name));
    /*
       if (pix_comm_functions.sysdep_wireless_essid)
       return(wireless_essid(PIX_COMM_SET, interface, name));
       else
       return(PIX_COMM_NOT_IMPLEMENTED);
     */
}

int
pix_comm_wireless_get_essid(char *interface, char *name)
{
    return (wireless_essid(PIX_COMM_GET, interface, name));

    /*
       if (pix_comm_functions.sysdep_wireless_essid)
       return(wireless_essid(PIX_COMM_GET, interface, name));
       else
       return(PIX_COMM_NOT_IMPLEMENTED);
     */
}

int
pix_comm_wireless_get_stats(char *interface, pix_comm_wirestats_t * stats)
{
    if (pix_comm_functions.sysdep_wireless_get_stats)
	return (pix_comm_functions.
		sysdep_wireless_get_stats(interface, stats));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_wireless_get_if_list(pix_comm_interface_t * iflist, int *ifcount)
{
    if (pix_comm_functions.sysdep_wireless_get_if_list)
	return (pix_comm_functions.
		sysdep_wireless_get_if_list(iflist, ifcount));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_get_net_value(char *key, char *ret)
{
    if (pix_comm_functions.sysdep_get_net_value)
	return (pix_comm_functions.sysdep_get_net_value(key, ret));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_write_net_values(pix_comm_ipaddr_str_t ip_info,
			  pix_comm_dns_t dns_info)
{
    if (pix_comm_functions.sysdep_write_net_values)
	return (pix_comm_functions.
		sysdep_write_net_values(ip_info, dns_info));
    else
	return (PIX_COMM_NOT_IMPLEMENTED);
}

int
pix_comm_set_netscript_values(char *file, const char *keys[],
			      const char *vals[], int size)
{
    if (0 == linux_set_netscript_values(file, keys, vals, size))
	return (PIX_COMM_OK);
    else
	return (PIX_COMM_ERROR);
}

int
pix_comm_get_netscript_values(char *file, char *keys[], char *vals[],
			      int size)
{
    if (0 == linux_get_netscript_values(file, keys, vals, size))
	return (PIX_COMM_OK);
    else
	return (PIX_COMM_ERROR);
}
