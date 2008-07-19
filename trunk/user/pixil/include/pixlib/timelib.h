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


#ifndef TIMELIB_H
#define TIMELIB_H



/*
**
** Imported "Include" Files
**
*/
#include <time.h>



/*
**
** Global Structure Definitions
**
*/
typedef struct			/* 7 bytes */
{
    unsigned short year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
}
pix_sys_date_t;



/*
**
**  NAME: pix_sysConvertTMStruct()
**
** USAGE: int pix_sysConvertTMStruct(struct tm *tstruct, pix_sys_date_t *result);
**
** DESCR: This function will convert the time/date information found in the
**        specified 'tm' structure into an pix_sys_date_t structure.
**
** PARMS: The "tstruct" parameter is a pointer to the 'tm' structure. The
**        "result" parameter is a pointer to an pix_sys_date_t structure
**        that will contain the result.
**
** RETRN: If successful, '0' is returned. If the time/date specified by the
**        'tm' structure is outside the bounds of an pix_sys_date_t
**        structure, a non-zero value is returned.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    int pix_sysConvertTMStruct(struct tm *, pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysGetCurrentTime()
**
** USAGE: void pix_sysGetCurrentTime(pix_sys_date_t *result);
**
** DESCR: This function will initialize an pix_sys_date_t structure with
**        the current time and date.
**
** PARMS: The "result" parameter points to the pix_sys_date_t structure
**        that will contain the result.
**
** RETRN: Nothing.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    void pix_sysGetCurrentTime(pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysCompareDates()
**
** USAGE: int pix_sysCompareDates(pix_sys_date_t *stamp1, pix_sys_date_t *stamp2);
**
** DESCR: This function will compare the dates found in two pix_sys_date_t
**        structures.
**
** PARMS: The "stamp1" and "stamp2" parameters are pointers to initialized
**        pix_sys_date_t structures.
**
** RETRN: If the first date is earlier than the second, a negative
**        value is returned. If the first date is later than the second,
**        a positive value is returned. If the two dates are the same,
**        '0' is returned.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    int pix_sysCompareDates(pix_sys_date_t *, pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysCompareTimeStamps()
**
** USAGE: int pix_sysCompareTimeStamps(pix_sys_date_t *stamp1, pix_sys_date_t *stamp2);
**
** DESCR: The following function will compare the dates and times found
**        in two pix_sys_date_t structures.
**
** PARMS: The "stamp1" and "stamp2" parameters are pointers to initialized
**        pix_sys_date_t structures.
**
** RETRN: If the first timestamp is earlier than the second, a negative
**        value is returned. If the first timestamp is later than the
**        second, a positive value is returned. If the two timestamps
**        are the same, '0' is returned.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    int pix_sysCompareTimeStamps(pix_sys_date_t *, pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysCompareBounds()
**
** USAGE: int pix_sysCompareBounds(pix_sys_date_t *first,
**                               pix_sys_date_t *last,
**                               pix_sys_date_t *subject);
**
** DESCR: This function will check to see if the subject timestamp falls
**        within the bounds specified by the first and last timestamps.
**
** PARMS: The "first" and "last" parameters are pointers to initialized
**        pix_sys_date_t structures that define the bounds. The "subject"
**        parameter is a pointer to an initialized pix_sys_date_t structure
**        that specifies the time to be tested.
**
** RETRN: If the subject timestamp is greater than or equal to the first
**        timestamp and is less than or equal to the last timestamp, '0'
**        is returned. Otherwise, the subject timestamp is outside of the
**        bounds specified and a non-zero value is returned.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    int pix_sysCompareBounds(pix_sys_date_t *, pix_sys_date_t *,
			     pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysIncrementDate()
**
** USAGE: void pix_sysIncrementDate(pix_sys_date_t *stamp);
**
** DESCR: This function will add one day to the date found in an
**        pix_sys_date_t structure.
**
** PARMS: The "stamp" parameter is a pointer to an initialized pix_sys_date_t
**        structure.
**
** RETRN: Nothing.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    void pix_sysIncrementDate(pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysDecrementDate()
**
** USAGE: void pix_sysDecrementDate(pix_sys_date_t *stamp);
**
** DESCR: This function will subtract one day from the date found in an
**        pix_sys_date_t structure.
**
** PARMS: The "stamp" parameter is a pointer to an initialized pix_sys_date_t
**        structure.
**
** RETRN: Nothing.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    void pix_sysDecrementDate(pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysIncrementHour()
**
** USAGE: void pix_sysIncrementHour(pix_sys_date_t *stamp);
**
** DESCR: This funtion will add one hour to the date found in an pix_sys_date_t
**        structure.
**
** PARMS: The "stamp" parameter is a pointer to an initialized pix_sys_date_t
**        structure.
**
** RETRN: Nothing.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    void pix_sysIncrementHour(pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysDecrementHour()
**
** USAGE: void pix_sysDecrementHour(pix_sys_date_t *stamp);
**
** DESCR: This funtion will subtract one hour from the date found in an
**        pix_sys_date_t structure.
**
** PARMS: The "stamp" parameter is a pointer to an initialized pix_sys_date_t
**        structure.
**
** RETRN: Nothing.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    void pix_sysDecrementHour(pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysIncrementMinute()
**
** USAGE: void pix_sysIncrementMinute(pix_sys_date_t *stamp);
**
** DESCR: This funtion will add one minute to the date found in an pix_sys_date_t
**        structure.
**
** PARMS: The "stamp" parameter is a pointer to an initialized pix_sys_date_t
**        structure.
**
** RETRN: Nothing.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    void pix_sysIncrementMinute(pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysDecrementMinute()
**
** USAGE: void pix_sysDecrementMinute(pix_sys_date_t *stamp);
**
** DESCR: This funtion will subtract one minute from the date found in an
**        pix_sys_date_t structure.
**
** PARMS: The "stamp" parameter is a pointer to an initialized pix_sys_date_t
**        structure.
**
** RETRN: Nothing.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    void pix_sysDecrementMinute(pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysIncrementSecond()
**
** USAGE: void pix_sysIncrementSecond(pix_sys_date_t *stamp);
**
** DESCR: This funtion will add one second to the date found in an pix_sys_date_t
**        structure.
**
** PARMS: The "stamp" parameter is a pointer to an initialized pix_sys_date_t
**        structure.
**
** RETRN: Nothing.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    void pix_sysIncrementSecond(pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysDecrementSecond()
**
** USAGE: void pix_sysDecrementSecond(pix_sys_date_t *stamp);
**
** DESCR: This funtion will subtract one second from the date found in an
**        pix_sys_date_t structure.
**
** PARMS: The "stamp" parameter is a pointer to an initialized pix_sys_date_t
**        structure.
**
** RETRN: Nothing.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    void pix_sysDecrementSecond(pix_sys_date_t *);
#ifdef __cplusplus
}
#endif




/*
**
**  NAME: pix_sysGetFullDayOfWeek()
**
** USAGE: char *pix_sysGetDayOfWeek(pix_sys_date_t *stamp);
**
** DESCR: This function will calculate what day of the week corresponds
**        to the specified pix_sys_date_t structure.
**
** PARMS: The "stamp" parameter is a pointer to an initialized pix_sys_date_t
**        structure.
**
** RETRN: A pointer to the text string that specifies the day of the week
**        is returned.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    char *pix_sysGetFullDayOfWeek(pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysGetShortDayOfWeek()
**
** USAGE: char *pix_sysGetDayOfWeek(pix_sys_date_t *stamp);
**
** DESCR: This function will calculate what day of the week corresponds
**        to the specified pix_sys_date_t structure.
**
** PARMS: The "stamp" parameter is a pointer to an initialized pix_sys_date_t
**        structure.
**
** RETRN: A pointer to the text string that specifies the day of the week
**        is returned.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    char *pix_sysGetShortDayOfWeek(pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysGetShortMonth()
**
** USAGE: char *pix_sysGetShortMonth(unsigned char month);
**
** DESCR: This function will recover a pointer to a text string that describes
**        the specified month with a three letter abbreviation.
**
** PARMS: The "month" parameter specifies the month's numeric value.
**
** RETRN: If successful, a pointer to the text string for the specified month
**        is returned. If an error occures during function execution, a pointer
**        to an empty string is returned.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    char *pix_sysGetShortMonth(unsigned char);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysGetFullMonth()
**
** USAGE: char *pix_sysGetFullMonth(unsigned char month);
**
** DESCR: This function will recover a pointer to a text string that describes
**        the specified month.
**
** PARMS: The "month" parameter specifies the month's numeric value.
**
** RETRN: If successful, a pointer to the text string for the specified month
**        is returned. If an error occures during function execution, a pointer
**        to an empty string is returned.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    char *pix_sysGetFullMonth(unsigned char);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysGetDaysInMonth()
**
** USAGE: int pix_sysGetDaysInMonth(pix_sys_date_t *stamp);
**
** DESCR: This function will recover the number of days in the month specified
**        in the associated pix_sys_date_t structure. Leap year calculations are
**        included.
**
** PARMS: The "stamp" parameter is a pointer to an initialized pix_sys_date_t
**        structure.
**
** RETRN: If successful, the number of days in the specified month are returned.
**        If an error occures during function execution, '0' is returned.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    int pix_sysGetDaysInMonth(pix_sys_date_t *);
#ifdef __cplusplus
}
#endif



/*
**
**  NAME: pix_sysGetTimeDateString()
**
** USAGE: void pix_sysGetTimeDateString(pix_sys_date_t *stamp,
**                                    short format,
**                                    char *result);
**
** DESCR: This function will create an ASCIIZ string that represents the date
**        and time found in the specified pix_sys_date_t structure. The resulting
**        string will be in one of the following formats:
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
** PARMS: The "stamp" parameter is a pointer to an initialized pix_sys_date_t
**        structure. The "format" parameter specifies the string's format.
**        The "result" parameter is a pointer to the buffer where the 
**        resulting ASCIIZ string will be stored.
**
** RETRN: Nothing.
**
*/
#ifdef __cplusplus
extern "C"
{
#endif
    void pix_sysGetTimeDateString(pix_sys_date_t *, short, char *);
#ifdef __cplusplus
}
#endif

#endif				/* TIMELIB_H */
