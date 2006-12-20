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

/* config.h.  Manually edited for Visual C++.  */

/* Define as __inline if that's what the C compiler calls it.  */
#define inline __inline

/* Disable warnings about STL symbols lengths */
#ifdef WIN32
#pragma warning(disable:4786)
#endif /*  */

/* Define if you have the setlocale function.  */
#define HAVE_SETLOCALE 1

/* Define if you have the <locale.h> header file.  */
#define HAVE_LOCALE_H 1

/* Define to 1 if translation of program messages to the user's native language
   is requested. */
#define ENABLE_NLS 1

/*--------------------------------------------------------------*/

/* Included here for internationalization */

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif /*  */

#ifdef CONFIG_PIXILDT_INTERNATIONAL
#include <intl/libgnuintl.h>
#define _(string) gettext(string)
#define N_(string) (string)
#else
#define _(string) (string)
#define N_(string) (string)
#endif

#ifdef WIN32

// MS/Windows definitions
#define PACKAGE   "PixilDT"
#define LOCALEDIR "."

#else /*  */

// Linux definitions
#define PACKAGE   "PixilDT"
#define LOCALEDIR "."

#endif /*  */

#ifdef WIN32
extern "C"
{
    __declspec(dllimport) int __stdcall MessageBeep(unsigned int uType);
}

#define MB_ICONQUESTION 0x00000020L

#endif				/*  */
