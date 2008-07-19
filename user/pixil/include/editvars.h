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
 * See http://embedded.centurysoftware.com/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://embedded.centurysoftware.com/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */



#ifndef VARTYPES_H		/* include only once per compile */
#define VARTYPES_H 1

#if defined(WIN32)
#include <dos.h>
#ifndef RCLDLL
#define RCLDLL
#endif /*  */
#else // not Win32
#define RCLDLL
#define CLASSEXPORT
#endif /*  */

#if (FL_MAJOR_VERSION <= 1)
#define FL_API   FL_EXPORT
#endif /*  */

// These are for the editor, but occasionally seem useful elsewhere
#define SOFT_RETURN 0x0D
#define HARD_RETURN 0x0A
#define CARRIAGE_RETURN  0x0D
#define LINE_FEED 0x0A

#ifndef TRUE
#define TRUE    1
#endif /*  */
#ifndef FALSE
#define FALSE   0
#endif /*  */

#ifdef _WINDOWS
#undef RCLDLL
#if defined(RCL_EXPORTS)
#define RCLDLL  __declspec(dllexport)
#else /*  */
#define RCLDLL
#endif /*  */
#endif /*  */

#ifdef WIN32
RCLDLL extern float PowersOfTen[];

#else /*  */
extern float PowersOfTen[];

#endif /*  */
enum ErrorSeverity
{ ERROR_OKAY, ERROR_INFO, ERROR_WARN, ERROR_FAIL,
};

#undef NOERROR
enum rclError
{ NOERROR,			/* everything OK */
    NOMEMORY,			/* out of memory */
    USERCANCEL,			/* user cancelled operation */
    GOTBREAKKEY,		/* user hit a break key */
    ABORTED,			/* operation aborted */
    FILEERROR,			/* a file error occurred */
    BUFFER_OVERFLOW,		/* operation overflowed internal buffers */
    RELOAD,			/* internally used to force data reload */
    SECURITYVIOLATION,		/* unauthorized operation attempted */
    NOCONFIG,			/* can't read config file records */
    CLIPBOARD_EMPTY,		/* nothing in clipboard to operate on */
    NO_REGION,			/* region not marked to operate on */
    RANGE_ERROR,		/* bounds check exceeded */
    LIMITS_FULL_ERROR,		/* preset limits exceeded */
    LICENSE_OK,			/* license is OK for this module */
    NOT_LICENSED,		/* no license for this module */
    LICENSE_EXPIRED,		/* past license date range */
    FORM_COMPLETE,		/* user completed input form */
    WRONG_TYPE,			/* data is not the expected type */
    VMERROR,			/* VM file error */
    INVALID_PARAMETER,		/* bad parameter given for object construction */
    UNKNOWN_USER,		/* can't identify user name */
};

/* Macro to get a random integer within a specified range */

#define getrandom( min, max ) ((rand() % (int)(((max)+1) - (min))) + (min))
#define DIVIDE(num, by)         (((by) == 0) ? 0 : ((num) / (by)))

#include <math.h>
#ifndef WIN32
#include <cmath>
#ifndef modff
//extern "C" float modff(float, float *);

#endif /*  */
#endif /*  */
inline float
Round(float num, short places)
{
    float intpart, fracpart;
    fracpart = modff(num * PowersOfTen[places], &intpart);
    if (fracpart >= 0.5)
	intpart++;
    return (intpart / PowersOfTen[places]);
}

inline bool
IsZero(float num)
{
    return ((num > -0.005) && (num < 0.005));
}


#endif /*  */
