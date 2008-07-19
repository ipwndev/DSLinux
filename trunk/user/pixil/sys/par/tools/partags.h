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


static xml_tag tagPref[] = {
    {"pref", 0, init_key, data_key, 0},
    {"", 0, 0}
};

static xml_tag tagCategory[] = {
    {"category", tagPref, init_section, 0, 0},
    {"", 0, 0}
};

static xml_tag tagPreferences[] = {
    {"preferences", tagCategory, generic_init, 0, 0},
    {"", 0, 0}
};

static xml_tag tagAppSettings[] = {
    {"preferences", tagCategory, generic_init, 0, 0},
    {"title", 0, 0, data_text, 0},
    {"exec", 0, 0, data_text, 0},
    {"workdir", 0, 0, data_text, 0},
    {"icon", 0, 0, data_text, 0},
    {"defargs", 0, 0, data_text, 0},
    {"", 0, 0}
};

static xml_tag tagApp[] = {
    {"app", tagAppSettings, init_section, 0, 0},
    {"", 0, 0}
};

static xml_tag tagCap[] = {
    {"cap", 0, 0, data_caplist, 0},
    {"", 0, 0}
};

static xml_tag tagDirectories[] = {
    {"icondir", 0, 0, data_text, 0},
    {"bindir", 0, 0, data_text, 0},
    {"themedir", 0, 0, data_text, 0},
    {"fontdir", 0, 0, data_text, 0 },
    {"", 0, 0}
};

static xml_tag tagScrtopCat[] = {
    {"title", 0, 0, data_text, 0},
    {"applist", 0, 0, generic_textlist, 0},
    {"", 0, 0}
};

static xml_tag tagCategories[] = {
    {"category", tagScrtopCat, init_section, 0, 0},
    {"", 0, 0}
};

/* The bgstyle is unique because it is a list of keywords */

static xml_tag colorSettings[] = {
    {"color", 0, 0, data_color, 0},
    {"", 0, 0}
};

static xml_tag appletSettings[] = {
  { "applet", 0, applet_init, applet_data, 0 },
  {"", 0, 0}
};

static xml_tag tagSettings[] = {
    {"bgimage", 0, 0, data_text, 0},
    {"bgstyle", 0, 0, data_text, 0},
    {"appwidth", 0, 0, data_value, 0},
    {"appheight", 0, 0, data_value, 0},
    {"colors", colorSettings, generic_init, 0, 0},
    {"applets", appletSettings, generic_init, 0, 0 },
    {"", 0, 0}
};

static xml_tag tagInputOptions[] = {
    {"title", 0, 0, data_text, 0},
    {"app", 0, 0, data_text, 0},
    {"icon", 0, 0, data_text, 0},
    {"", 0, 0}
};

static xml_tag tagInputs[] = {
    {"input", tagInputOptions, init_section, 0, 0},
    {"", 0, 0}
};

/* These are the tags for the screentop */

static xml_tag tagScreentop[] = {
    {"directories", tagDirectories, generic_init, 0, 0},
    {"categories", tagCategories, generic_init, 0, 0},
    {"inputs", tagInputs, generic_init, 0, 0},
    {"settings", tagSettings, generic_init, 0, 0},
    {"", 0, 0}
};

/* These are the toplevel tags */

static xml_tag tagToplevel[] = {
    {"global", tagPreferences, generic_init, 0, 0},
    {"application", tagApp, generic_init, 0, 0},
    {"capabilities", tagCap, generic_init, 0, 0},
    {"screentop", tagScreentop, generic_init, 0, 0},
    {"", 0, 0}
};



/* Global PAR settings used for encoding */

xml_encode encodePref[] = {
    {"pref", "*", 0, prefs_header, par_data_footer, par_data},
    {"", "", 0, 0, 0}
};

xml_encode encodeCategory[] = {
    {"category", "*", encodePref, category_header, 0, 0},
    {"", "", 0, 0, 0}
};

xml_encode encodePreferences[] = {
    {"preferences", "preferences", encodeCategory, 0, 0, 0},
    {"", "", 0, 0, 0}
};

xml_encode encodeAppSettings[] = {
    {"title", "title", 0, par_data_header, par_data_footer, par_data},
    {"exec", "exec", 0, par_data_header, par_data_footer, par_data},
    {"workdir", "workdir", 0, par_data_header, par_data_footer, par_data},
    {"icon", "icon", 0, par_data_header, par_data_footer, par_data},
    {"defargs", "defargs", 0, par_data_header, par_data_footer, par_data},
    {"wmcategory", "wmcategory", 0, par_data_header, par_data_footer,
     par_data},
    {"preferences", "preferences", encodeCategory, 0, 0, 0},
    {"", "", 0, 0, 0}
};

xml_encode encodeApp[] = {
    {"app", "*", encodeAppSettings, app_header, 0, 0},
    {"", "", 0, 0, 0}
};

xml_encode encodeCap[] = {
    {"cap", "*", 0, par_named_header, par_data_footer, par_data},
    {"", "", 0, 0, 0}
};

xml_encode encodeDirectories[] = {
    {"icondir", "icondir", 0, par_data_header, par_data_footer, par_data},
    {"bindir", "bindir", 0, par_data_header, par_data_footer, par_data},
    {"", "", 0, 0, 0}
};

xml_encode encodeScrtopCats[] = {
    {"title", "title", 0, par_data_header, par_data_footer, par_data},
    {"applist", "applist", 0, par_data_header, par_data_footer, par_data},
    {"", "", 0, 0, 0}
};

xml_encode encodeColors[] = {
    {"color", "*", 0, par_named_header, par_data_footer, par_color},
    {"", "", 0, 0, 0}
};

xml_encode encodeSettings[] = {
    {"bgimage", "bgimage", 0, par_data_header, par_data_footer, par_data},
    {"bgstyle", "bgstyle", 0, par_data_header, par_data_footer, par_data},
    {"appwidth", "appwidth", 0, par_data_header, par_data_footer, par_data},
    {"appheight", "appheight", 0, par_data_header, par_data_footer, par_data},

    {"colors", "colors", encodeColors, 0, 0, 0},
    {"theme", "theme", 0, par_data_header, par_data_footer, par_data},
    {"", "", 0, 0, 0}
};

xml_encode encodeScrtopInput[] = {
    {"title", "title", 0, par_data_header, par_data_footer, par_data},
    {"app", "app", 0, par_data_header, par_data_footer, par_data},
    {"icon", "icon", 0, par_data_header, par_data_footer, par_data},
    {"", "", 0, 0, 0}
};

xml_encode encodeCategories[] = {
    {"category", "*", encodeScrtopCats, category_header, 0, 0},
    {"", "", 0, 0, 0}
};

xml_encode encodeInputs[] = {
    {"input", "*", encodeScrtopInput, category_header, 0, 0},
    {"", "", 0, 0, 0}
};

xml_encode encodeScrtop[] = {
    {"directories", "directories", encodeDirectories, 0, 0, 0},
    {"categories", "categories", encodeCategories, 0, 0, 0},
    {"inputs", "inputs", encodeInputs, 0, 0, 0},
    {"settings", "settings", encodeSettings, 0, 0, 0},
    {"", "", 0, 0, 0}
};

xml_encode encodeGlobal[] = {
    {"global", "global", encodePreferences, 0, 0, 0},
    {"application", "application", encodeApp, 0, 0, 0},
    {"capabilities", "capabilities", encodeCap, 0, 0, 0},
    {"screentop", "screentop", encodeScrtop, 0, 0, 0},
    {"", "", 0, 0, 0}
};
