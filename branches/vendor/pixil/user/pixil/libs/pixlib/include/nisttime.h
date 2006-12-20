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


#ifndef NISTTIME_H
#define NISTTIME_H



/*
**
** Imported "Include" Files
**
*/
#include "timelib.h"



/*
**
** Global Constant Definitions
**
*/



/*
**
** Global Enumeration Definitions
**
*/



/*
**
** Global Structure Definitions
**
*/



/*
**
**  NAME: nxsysGetTimeServer()
**
** USAGE: int nxsysGetTimeServer(char *result);
**
** DESCR: This function will recover the current NIST time server name, IP
**        address, and geographical location.
**
** PARMS: The "result" parameter is a pointer to a buffer where the time
**        server information will be stored.
**
** RETRN: If successful, '0' is returned. If an error occurs during method
**        execution, a non-zero value is returned that describes the error.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    int nxsysGetTimeServer(char *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: nxsysSetTimeServer()
**
** USAGE: int nxsysSetTimeServer(char *ipaddr);
**
** DESCR: This function will set the IP address used for subsequent NIST time
**        server queries.
**
** PARMS: The "ipaddr" parameter is a pointer to a buffer that contains the
**        text string that specifies the new IP address. The text string is
**        in standard four-entry dotted notation.
**
** RETRN: If successful, '0' is returned. If an error occurs during method
**        execution, a non-zero value is returned that describes the error.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    int nxsysSetTimeServer(char *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: nxsysGetLocalOffset()
**
** USAGE: int nxsysGetLocalOffset(void);
**
** DESCR: This function will recover the current local time offset used to
**        calculate local time.
**
** PARMS: None.
**
** RETRN: The number of hours offset from UCT (Universal Coordinated Time)
**        is returned.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    int nxsysGetLocalOffset(void);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: nxsysSetLocalOffset()
**
** USAGE: int nxsysSetLocalOffset(int offset);
**
** DESCR: This function will set the local time offset used to calculate
**        local time.
**
** PAMRS: The "offset" parameter specifies the number of hours earlier or
**        later than UCT for the local time. For example, set this to -7
**        for Mountain Standard Time or -6 for Mountain Daylight Time.
**
** RETRN: If successful, '0' is returned. If an error occurs during method
**        execution, a non-zero value is returned that describes the error.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    int nxsysSetLocalOffset(int);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: nxsysGetDateTime()
**
** USAGE: int nxsysGetDateTime(nxsys_date_t *stamp);
**
** DESCR: This function will recover the current date/time from the NIST, and
**        initialize the specified timestamp with the date/time information.
**
** PARMS: The "stamp" parameter is a pointer to the nxsys_date_t structure that
**        will be loaded with the NIST date/time information.
**
** RETRN: If successful, '0' is returned. If an error occurs during function
**        exeuction, a non-zero value is returned that describes the error.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    int nxsysGetDateTime(nxsys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: nxsysGetDateTimeStr()
**
** USAGE: int nxsysGetDateTimeStr(int format, char *result);
**
** DESCR: This function will recover the current date/time from the NIST, and
**        format the result as an ASCIIZ string that is copied to the specified
**        result buffer. The string format is specified as follows:
**
**        format=0:   June 27, 2001
**        format=1:   June 27, 2001, 10:58 AM
**        format=2:   June 27, 2001, 10:58:30 AM
**        format=3:   Wednesday, June 27, 2001
**        format=4:   Wednesday, June 27, 2001, 10:58 AM
**        format=5:   Wednesday, June 27, 2001, 10:58:30 AM
**        format=6:   06/27/01
**        format=7:   06/27/01 10:58a
**        format=8:   06/27/01 10:58:30a
**        format=9:   Wed 06/27/01
**        format=10:  Wed 06/27/01 10:58a
**        format=11:  Wed 06/27/01 10:58:30a
**        format=12:  27 Jun 2001
**        format=13:  27 Jun 2001 10:58 AM
**        format=14:  27 Jun 2001 10:58:00 AM
**        format=15:  Wed 27 Jun 2001
**        format=16:  Wed 27 Jun 2001 10:58 AM
**        format=17:  Wed 27 Jun 2001 10:58:30 AM
**        format=18:  10:58 AM
**        format=19:  10:58:30 AM
**
** PARMS: The "format" parameter specifies the format of the date/time string
**        and is chosen from the list above.
**
** RETRN: If successful, '0' is returned. If an error occurs during method
**        execution, a non-zero value is returned that describes the error.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    int nxsysGetDateTimeStr(int, char *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: nxsysGetDateTimeValue();
**
** USAGE: unsigned int nxsysGetDateTimeValue(void);
**
** DESCR: This function will recover the current date/time from the NIST, and
**        convert it to the number of seconds that have elapsed since 12:00am
**        on January 1, 1970 (UTC).
**
** PARMS: None.
**
** RETRN: The number of seconds elapsed is returned. If an error occurs during
**        method execution, 0xFFFFFFFF (4294967295) is returned.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    unsigned int nxsysGetDateTimeValue(void);
#ifdef __cplusplus
}
#endif

#endif				/* NISTTIME_H */
