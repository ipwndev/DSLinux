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


static xml_tag tagsState[] = {
    {"image", 0, 0, data_image, 0},
    {"fgcolor", 0, 0, data_fgcolor, 0},
    {"bgcolor", 0, 0, data_bgcolor, 0}
};

static xml_tag tagsStates[] = {
    {"active", tagsState, active_state_init, 0, 0},
    {"inactive", tagsState, inactive_state_init, 0, 0},
};

static xml_tag tagsComponent[] = {
    {"hotspot", 0, 0, data_hotspot, 0},
    {"width", 0, 0, data_width, 0},
    {"height", 0, 0, data_height, 0},
    {"states", tagsStates, 0, 0, 0}
};

static xml_tag tagsWidget[] = {
    {"widget", tagsComponent, widget_init, 0, 0}
};

static xml_tag tagsClient[] = {
    {"padding", 0, 0, data_padding, 0}
};

static xml_tag themeToplevel[] = {
    {"titlebar", tagsWidget, titlebar_init, 0, 0},
    {"border_left", tagsWidget, border_init, 0, 0},
    {"border_right", tagsWidget, border_init, 0, 0},
    {"border_bottom", tagsWidget, border_init, 0, 0},
    {"border_top", tagsWidget, border_init, 0, 0},
    {"client", tagsClient, 0, 0, 0},
};
