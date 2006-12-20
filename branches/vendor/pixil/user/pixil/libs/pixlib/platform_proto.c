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


/* PIX_SYS includes */
#include <pixlib/pixlib.h>
#include <pixlib/pix_sys_proto.h>
#include <linux.h>

/* PIX_COMM includes */
#include <pixlib/pix_comm_proto.h>
#include <linux_if.h>
#include <linux_ppp.h>
#include <linux_wireless.h>

/* PIX IO includes */
#include <sound_dsp.h>
#include <pixlib/pixio_proto.h>

pix_sys_func_t pix_sys_functions = {
    linux_set_date,		/* set date */
    linux_get_date,		/* get date */
    PIX_SYS_PROTO_NO_FUNCTION,	/* set backlight */
    linux_get_cpu_load,		/* get CPU load */
    linux_get_memory_usage,	/* Get memory usage */
    PIX_SYS_PROTO_NO_FUNCTION,	/* Get battery */
    pix_get_net_value,
    pix_write_net_values
};

pix_comm_func_t pix_comm_functions = {
    linux_do_ip_address,	/* handle ip address */
    linux_do_default_gateway,	/* handle gateway */
    pix_set_nameserver,		/* Set the name server */
    linux_ppp_connect,		/* Connect a PPP session */
    linux_ppp_disconnect,	/* Disconnect a PPP session */
    linux_get_if_list,		/* Get a list of interfaces */
    linux_if_active,
    linux_set_if_status,
    linux_if_dhcp,
    linux_ppp_get_stats,
    linux_ppp_write_info,
    linux_ppp_read_info,
    linux_ppp_ip_address,
    0,
    0,
    wireless_essid,
    wireless_get_statistics,
    wireless_get_if_list,
    0,
    0
};

pix_io_functions_t pix_io_functions = {
    dspPlayWAVFile,
    dspWriteWAVFile,
    dspReadWAVFile,
    dspGetWAVFileStats,
    dspOpenStream,
    dspStreamRecord,
    dspStreamPlay,
    mixer_get_devices,
    mixer_get_level,
    mixer_set_level
};
