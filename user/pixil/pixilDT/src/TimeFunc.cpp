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

//--------------------------------------------------------------//
// Date/Time utilities.                                         //
//--------------------------------------------------------------//
#include "config.h"
#include <cassert>
#include <cstdio>
#include <ctime>
#include "PixilDT.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


#ifdef WIN32
#define strcasecmp stricmp	// Microsoft names it differently
#define tzset _tzset		// And again
#endif


//--------------------------------------------------------------//
// Internal functions.                                          //
//--------------------------------------------------------------//
static time_t ToDow(int nDow);	// Get a time that represents a day of the week
static time_t ToMonth(int nDow);	// Get a time that represents a month of the year


//--------------------------------------------------------------//
// Add (or subtract) some number of days to a date, return a    //
// date for midnight of the resulting day.                      //
//--------------------------------------------------------------//
time_t
AddDays(time_t nDate, int nDays)
{
    int nDiff;
    struct tm *pTm;

#ifdef DEBUG
    assert(nDate == NormalizeDate(nDate));	// Must be a "normalized" date (midnight)
#endif

    nDate += nDays * 24 * 60 * 60;
    pTm = localtime(&nDate);
    if (pTm->tm_hour > 12) {
	nDiff = 24 - pTm->tm_hour;
    } else {
	nDiff = -pTm->tm_hour;
    }
    nDate += nDiff * 60 * 60;
    return (nDate);
}


//--------------------------------------------------------------//
// Format a date.                                               //
//--------------------------------------------------------------//
string
FormatDate(time_t nDate)
{
    char szDate[16];
    string strReturn;

    if (nDate >= 24 * 60 * 60) {
	strftime(szDate, sizeof(szDate), "%x", localtime(&nDate));
	strReturn = szDate;
    }
    return (strReturn);
}


//--------------------------------------------------------------//
// Get the day of the week for a date.                          //
//--------------------------------------------------------------//
string
FormatDayOfWeek(time_t nDate)
{
    char szText[32];
    string strReturn;

    if (nDate > 24 * 60 * 60) {
	strftime(szText, sizeof(szText), "%A", localtime(&nDate));
	strReturn = szText;
    }
    return (strReturn);
}


//--------------------------------------------------------------//
// Format a Month and Year.                                     //
//--------------------------------------------------------------//
string
FormatMonth(time_t nDate)
{
    char szText[64];
    string strReturn;

    strftime(szText, sizeof(szText), "%B, %Y", localtime(&nDate));
    strReturn = szText;
    return (strReturn);
}


//--------------------------------------------------------------//
// Format a "short" time.                                       //
//--------------------------------------------------------------//
string
FormatShortTime(time_t nTime)
{
    char szTime[24];
    string strReturn;

    if (nTime >= 24 * 60 * 60) {
	struct tm *pTm = localtime(&nTime);

	if (pTm->tm_min == 0) {
	    strftime(szTime, sizeof(szTime), "%I", pTm);
	} else {
	    strftime(szTime, sizeof(szTime), "%I:%M", pTm);
	}
	strReturn = (szTime[0] == '0' ? szTime + 1 : szTime);
	strftime(szTime, sizeof(szTime), "%p", pTm);
	strReturn += szTime[0];
    }
    return (strReturn);
}


//--------------------------------------------------------------//
// Format a time.                                               //
//--------------------------------------------------------------//
string
FormatTime(time_t nTime)
{
    char szTime[24];
    string strAM;
    string strFormat;
    string strPM;
    string strReturn;

    PixilDT::GetAMPM(strAM, strPM);
    if (nTime >= 24 * 60 * 60) {
	struct tm *pTm = localtime(&nTime);

	if (strAM.length() != 0) {
	    // Twelve hour clock
	    strFormat = "%I";
	    strFormat += PixilDT::GetTimeSeparator();
	    strFormat += "%M %p";
	    strftime(szTime, sizeof(szTime), strFormat.c_str(), pTm);
	} else {
	    // 24 hour clock
	    strFormat = "%H";
	    strFormat += PixilDT::GetTimeSeparator();
	    strFormat += "%M";
	    strftime(szTime, sizeof(szTime), strFormat.c_str(), pTm);
	}
	strReturn = szTime;
    }
    return (strReturn);
}


//--------------------------------------------------------------//
// Get an error message about a date using an error message set //
// by ValidateDate.                                             //
//--------------------------------------------------------------//
const char *
GetDateError(int nError)
{
    static const char *pszError[7] = {
	N_("Year is not in the range of 1970 through 2036."),
	N_("Month is not in the range of 1 through 12."),
	N_("Day is not in the range for the entered month."),
	N_("The year contains non-numeric characters"),
	N_("The month contains non-numeric characters"),
	N_("The day-of-month contains non-numeric characters"),
	N_("The date must contain \"/\" or \"-\" delimiters between the year, month and day."),
    };
    const char *pszReturn;

    if (nError < 0 && nError >= -7) {
	pszReturn = _(pszError[-nError - 1]);
    } else {
	pszReturn = _("Unknown Date Format Error");
    }
    return (pszReturn);
}


//--------------------------------------------------------------//
// Get the day of the week for an integer day of week.          //
//--------------------------------------------------------------//
string
GetDayOfWeek(int nDay)
{
    return (GetDayOfWeek(ToDow(nDay)));
}


//--------------------------------------------------------------//
// Get the day of the week for an integer day of week.          //
//--------------------------------------------------------------//
string
GetDayOfWeek(time_t nDate)
{
    char szText[32];
    string strReturn;

    strftime(szText, sizeof(szText), "%A", localtime(&nDate));
    strReturn = szText;

    return (strReturn);
}


//--------------------------------------------------------------//
// Get the day of the week abbreviation for an integer day of   //
// week.                                                        //
//--------------------------------------------------------------//
string
GetDayOfWeekAbbr(int nDay)
{
    return (GetDayOfWeekAbbr(ToDow(nDay)));
}


//--------------------------------------------------------------//
// Get the day of the week abbreviation for an integer day of   //
// week.                                                        //
//--------------------------------------------------------------//
string
GetDayOfWeekAbbr(time_t nDate)
{
    char szText[32];
    string strReturn;

    strftime(szText, sizeof(szText), "%a", localtime(&nDate));
    strReturn = szText;

    return (strReturn);
}


//--------------------------------------------------------------//
// Get the day of the week as an integer.                       //
//--------------------------------------------------------------//
int
GetDow(time_t nDate)
{
    struct tm *pTm;

    pTm = localtime(&nDate);
    return (pTm->tm_wday);
}


//--------------------------------------------------------------//
// Get the abbreviated name of a month.                         //
//--------------------------------------------------------------//
string
GetMonthAbbr(int nMonth)
{
    return (GetMonthAbbr(ToMonth(nMonth)));
}


//--------------------------------------------------------------//
// Get the abbreviated name of a month.                         //
//--------------------------------------------------------------//
string
GetMonthAbbr(time_t nDate)
{
    char szText[32];
    string strReturn;

    strftime(szText, sizeof(szText), "%b", localtime(&nDate));
    strReturn = szText;

    return (strReturn);
}


//--------------------------------------------------------------//
// Get the name of a month.                                     //
//--------------------------------------------------------------//
string
GetMonthName(int nMonth)
{
    return (GetMonthName(ToMonth(nMonth)));
}


//--------------------------------------------------------------//
// Get the name of a month.                                     //
//--------------------------------------------------------------//
string
GetMonthName(time_t nDate)
{
    char szText[32];
    string strReturn;

    strftime(szText, sizeof(szText), "%B", localtime(&nDate));
    strReturn = szText;

    return (strReturn);
}


//--------------------------------------------------------------//
// Get week of the month for a date.  The week is calculated    //
// from the start of the month no matter which day-of-week the  //
// 1st of the month was on.  This means that, for instance, the //
// first Tuesday of the month will always be in week zero.      //
//--------------------------------------------------------------//
int
GetMonthWeek(time_t nDate)
{
    struct tm *pTm = localtime(&nDate);

    return (GetMonthWeek(pTm->tm_mday));
}


//--------------------------------------------------------------//
// Get an error message about a time using an error message set //
// by ValidateTime.                                             //
//--------------------------------------------------------------//
const char *
GetTimeError(int nError)
{
    static const char *pszError[7] = {
	N_("The time is followed by an invalid AM/PM string."),
	N_("The minutes must be zero for the last hour of a 24 hour clock."),
	N_("The minutes must be between 0 and 59."),
	N_("The hours must be between 0 and 24 for a 24 hour clock."),
	N_("The hours must be between 1 and 12."),
	N_("The hours must be numeric."),
	N_("The minutess must be numeric."),
    };
    const char *pszReturn;

    if (nError < 0 && nError >= -7) {
	pszReturn = _(pszError[-nError - 1]);
    } else {
	pszReturn = _("Unknown Time Format Error");
    }
    return (pszReturn);
}


//--------------------------------------------------------------//
// Test whether a date is on the same day of month and also     //
// provide an exception for shorter months.                     //
//--------------------------------------------------------------//
bool
IsDayOfMonth(time_t nDate, time_t nDom)
{
    bool bReturn = false;
    int nDay;
    struct tm *pTm = localtime(&nDom);

    // Get the day of month
    nDay = pTm->tm_mday;

    pTm = localtime(&nDate);
    if (pTm->tm_mday == nDay) {
	// Same day of month
	bReturn = true;
    } else if (nDay > 30 && pTm->tm_mday == 30 && (pTm->tm_mon == 3	// April
						   || pTm->tm_mon == 5	// June
						   || pTm->tm_mon == 8	// September
						   || pTm->tm_mon == 10))	// November
    {
	// Short month, call it the same day
	bReturn = true;
    } else if (nDay > 28 && pTm->tm_mon == 1)	// February
    {
	// Is this a leap year
	if ((pTm->tm_year % 4) == 0) {
	    // Leap year
	    if (nDay > 29 && pTm->tm_mday == 29) {
		// Past end of February
		bReturn = true;
	    }
	} else if (pTm->tm_mday == 28) {
	    // Past end of February
	    bReturn = true;
	}
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Test whether a date is on the same day of year and also      //
// provide an exception for February 29th.                      //
//--------------------------------------------------------------//
bool
IsDayOfYear(time_t nDate, time_t nDoy)
{
    bool bReturn = false;
    int nDay;
    int nMonth;
    struct tm *pTm = localtime(&nDoy);

    nDay = pTm->tm_mday;
    nMonth = pTm->tm_mon;

    pTm = localtime(&nDate);
    if (pTm->tm_mon == nMonth) {
	if (pTm->tm_mday == nDay
	    || (nMonth == 1 && nDay == 29 && (pTm->tm_year % 4) != 0
		&& pTm->tm_mday == 28)) {
	    // Same day of the year
	    bReturn = true;
	}
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Make a date from the year, month and day.                    //
//--------------------------------------------------------------//
time_t
MakeDate(int nYear, int nMonth, int nDay)
{
    time_t nDate;
    struct tm tmStruct;

    tzset();
    memset(&tmStruct, 0, sizeof(tmStruct));
    tmStruct.tm_isdst = -1;
    tmStruct.tm_mday = nDay;
    tmStruct.tm_mon = nMonth;
    tmStruct.tm_year = nYear;
    nDate = mktime(&tmStruct);
#ifdef DEBUG
    assert((int) nDate != -1);
#endif
    return (nDate);
}


//--------------------------------------------------------------//
// Return the number of months between two dates.               //
//--------------------------------------------------------------//
int
MonthsBetween(time_t nDate1, time_t nDate2)
{
    int nMonth1;
    int nYear1;
    struct tm *pTm = localtime(&nDate1);

    nMonth1 = pTm->tm_mon;
    nYear1 = pTm->tm_year;
    pTm = localtime(&nDate2);
    return (12 * (nYear1 - pTm->tm_year) + nMonth1 - pTm->tm_mon);
}


//--------------------------------------------------------------//
// Normalize a date to midnight local time.                     //
//--------------------------------------------------------------//
time_t
NormalizeDate(time_t nDate)
{
    struct tm *pTm = localtime(&nDate);

    tzset();
    pTm->tm_sec = 0;
    pTm->tm_min = 0;
    pTm->tm_hour = 0;
    return (mktime(pTm));
}


//--------------------------------------------------------------//
// Test a string for all numeric digits.                        //
//--------------------------------------------------------------//
bool
TestNumeric(const string & strData)
{
    bool bReturn = true;
    int i;
    int nMax = strData.length();

    for (i = 0; i < nMax; ++i) {
	if (!isdigit(strData[i])) {
	    bReturn = false;
	    break;
	}
    }
    return (bReturn);
}


//--------------------------------------------------------------//
// Set up a date equivalent to a given day of week.             //
//--------------------------------------------------------------//
time_t
ToDow(int nDow)
{
    struct tm *pTm;
    time_t timeValue = time(NULL);

#ifdef DEBUG
    assert(nDow >= 0 && nDow < 7);
#endif

    pTm = localtime(&timeValue);
    pTm->tm_hour = 12;
    pTm->tm_min = 0;
    pTm->tm_sec = 0;
    timeValue = mktime(pTm);
    timeValue += (7 + nDow - pTm->tm_wday) * 24 * 60 * 60;
    return (timeValue);
}


//--------------------------------------------------------------//
// Set up a date equivalent to a given month of the year.       //
//--------------------------------------------------------------//
time_t
ToMonth(int nMonth)
{
    struct tm TM;

#ifdef DEBUG
    assert(nMonth >= 0 && nMonth < 12);
#endif

    memset(&TM, 0, sizeof(TM));
    TM.tm_year = 80;
    TM.tm_mon = nMonth;
    TM.tm_mday = 2;
    TM.tm_hour = 12;
    return (mktime(&TM));
}


//--------------------------------------------------------------//
// Validate a newly entered date.  The entered date must be a   //
// valid date in order of the locale.  The year can be a 2 or 4 //
// digit year.  If only one delimiter is found then the date is //
// assumed to be in MM/DD or DD/MM format for the current year. //
// The returned value will be 0 if the date is valid or a       //
// negative number indicating the error type if the date is not //
// valid.                                                       //
//--------------------------------------------------------------//
int
ValidateDate(const char *pszDate, time_t & nDate)
{
    char szData[16];
    int nReturn;
    string strDate = pszDate;
    unsigned int nPos1;
    unsigned int nPos2;

    // Test the date format - find the delimiters
    nPos1 = strDate.find(PixilDT::GetDateSeparator(), 0);
    if (nPos1 != string::npos) {
	nPos2 = strDate.find(PixilDT::GetDateSeparator(), nPos1 + 1);
    } else {
	nPos1 = strDate.find('/', 0);
	if (nPos1 != string::npos) {
	    nPos2 = strDate.find('/', nPos1 + 1);
	} else {
	    nPos1 = strDate.find('-', 0);
	    if (nPos1 != string::npos) {
		nPos2 = strDate.find('-', nPos1 + 1);
	    }
	}
    }

    // Was any delimiter found ?
    if (nPos1 != string::npos) {
	string strDay;
	string strMonth;
	string strYear;

	// Was only a single delimiter found ?
	if (nPos2 == string::npos) {
	    long nTime = time(NULL);
	    struct tm *pTm;

	    // Only one delimiter, the date must be MM/DD or DD/MM for the current year
	    switch (PixilDT::GetDateFormat()) {
	    case PixilDT::DATE_DMY:
	    case PixilDT::DATE_DYM:
	    case PixilDT::DATE_YDM:
		if (nPos1 > 0) {
		    strDay = strDate.substr(0, nPos1 - 1);
		}
		strMonth =
		    strDate.substr(nPos1 + 1, strDate.length() - nPos1 - 1);
		break;

		//case PixilDT::DATE_MDY:
		//case PixilDT::DATE_MYD:
		//case PixilDT::DATE_YMD:
	    default:
		if (nPos1 > 0) {
		    strMonth = strDate.substr(0, nPos1 - 1);
		}
		strDay =
		    strDate.substr(nPos1 + 1, strDate.length() - nPos1 - 1);
	    }
	    pTm = localtime(&nTime);
	    sprintf(szData, "%d", pTm->tm_year + 1900);
	    strYear = szData;
	} else {
	    // Two delimiters found, date must be in the format for the locale
	    switch (PixilDT::GetDateFormat()) {
	    case PixilDT::DATE_DMY:
		strDay = strDate.substr(0, nPos1);
		strMonth = strDate.substr(nPos1 + 1, nPos2 - nPos1 - 1);
		strYear =
		    strDate.substr(nPos2 + 1, strDate.length() - nPos2 - 1);
		break;

	    case PixilDT::DATE_DYM:
		strDay = strDate.substr(0, nPos1);
		strYear = strDate.substr(nPos1 + 1, nPos2 - nPos1 - 1);
		strMonth =
		    strDate.substr(nPos2 + 1, strDate.length() - nPos2 - 1);
		break;

	    case PixilDT::DATE_MYD:
		strMonth = strDate.substr(0, nPos1);
		strYear = strDate.substr(nPos1 + 1, nPos2 - nPos1 - 1);
		strDay =
		    strDate.substr(nPos2 + 1, strDate.length() - nPos2 - 1);
		break;

	    case PixilDT::DATE_YMD:
		strYear = strDate.substr(0, nPos1);
		strMonth = strDate.substr(nPos1 + 1, nPos2 - nPos1 - 1);
		strDay =
		    strDate.substr(nPos2 + 1, strDate.length() - nPos2 - 1);
		break;

	    case PixilDT::DATE_YDM:
		strYear = strDate.substr(0, nPos1);
		strDay = strDate.substr(nPos1 + 1, nPos2 - nPos1 - 1);
		strYear =
		    strDate.substr(nPos2 + 1, strDate.length() - nPos2 - 1);
		break;

		//case PixilDT::DATE_MDY:
	    default:
		strMonth = strDate.substr(0, nPos1);
		strDay = strDate.substr(nPos1 + 1, nPos2 - nPos1 - 1);
		strYear =
		    strDate.substr(nPos2 + 1, strDate.length() - nPos2 - 1);
	    }
	}

	// Test each component for numerics
	if (::TestNumeric(strYear)
	    &&::TestNumeric(strMonth)
	    &&::TestNumeric(strDay)) {
	    static const int nMonthDays[2][12] = {
		{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
		{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	    };
	    int nDay;
	    int nLeapYear;
	    int nMonth;
	    int nYear;

	    // Now test the range for each
	    nYear = atoi(strYear.c_str());

	    // Fix up a 2 digit year
	    if (nYear < 100) {
		if (nYear < 70) {
		    nYear += 2000;
		} else {
		    nYear += 1900;
		}
	    }
	    // Get the rest
	    nMonth = atoi(strMonth.c_str()) - 1;
	    nDay = atoi(strDay.c_str());
	    nLeapYear = ((nYear % 4) == 0);
	    if (nYear >= 1970 && nYear <= 2036
		&& nMonth >= 0 && nMonth <= 11
		&& nDay >= 1 && nDay <= nMonthDays[nLeapYear][nMonth]) {
		// The range check is good, get the seconds for this date
		nDate = MakeDate(nYear - 1900, nMonth, nDay);
		nReturn = 0;
	    } else {
		if (nYear >= 1970 && nYear <= 2036) {
		    nReturn = -1;
		} else if (nMonth >= 0 && nMonth <= 11) {
		    nReturn = -2;
		} else		//if (nDay>=1&&nDay<=nMonthDays[nLeapYear][nMonth])
		{
		    nReturn = -3;
		}
	    }
	} else {
	    if (!TestNumeric(strYear)) {
		nReturn = -4;
	    }
	    if (!TestNumeric(strMonth)) {
		nReturn = -5;
	    } else		//if (!TestNumeric(strDay))
	    {
		nReturn = -6;
	    }
	}
    } else if (strDate.length() == 0) {
	// Blank date = no date
	nDate = 0;
	nReturn = 0;
    } else {
	nReturn = -7;
    }

    return (nReturn);
}


//--------------------------------------------------------------//
// Validate a newly entered time.  The entered time must be in  //
// a valid time in HH:MM (AM/PM), HH (AM/PM) or just HH format. //
// If the AM.PM string is missing then PM will be assumed.  The //
// returned value will be 0 if the date is valid or a negative  //
// number indicating the error type if the date is not valid.   //
//--------------------------------------------------------------//
int
ValidateTime(const char *pszTime, int &nTime)
{
    int nAmPm;
    int nHour;
    int nMinute;
    int nReturn = 0;
    string strAm;
    string strAmPm;
    string strHour;
    string strMinute;
    string strPm;
    string strTime = pszTime;
    unsigned int i;
    unsigned int nPos;

    // Determine the AM/PM strings
    PixilDT::GetAMPM(strAm, strPm);

    // Test the format - find the colon delimiter
    nPos = strTime.find(PixilDT::GetTimeSeparator(), 0);
    if (nPos == string::npos) {
	nPos = strTime.find(':', 0);
    }
    // Was any delimiter found ?
    if (nPos != string::npos) {
	// Get the hour
	strHour = strTime.substr(0, nPos);
	i = nPos + 1;
	while (i < strTime.length() && isdigit(strTime[i])) {
	    strMinute += strTime[i++];
	}
	strAmPm = strTime.substr(i, strTime.length() - i);
    } else {
	// No delimiter was found
	i = 0;
	while (i < strTime.length() && isdigit(strTime[i])) {
	    strHour += strTime[i++];
	}
	strAmPm = strTime.substr(i, strTime.length() - i);
    }

    // Validate the AM/PM field
    while (strAmPm.length() > 0 && isspace(strAmPm[0])) {
	strAmPm = strAmPm.substr(1, strAmPm.length() - 1);
    }
    while (strAmPm.length() > 0 && isspace(strAmPm[strAmPm.length() - 1])) {
	strAmPm = strAmPm.substr(0, strAmPm.length() - 1);
    }
    if (strAmPm.length() == 0) {
	nHour = atoi(strHour.c_str());
	if (strAm.length() != 0) {
	    // Twelve hour clock
	    if (nHour < 6 || nHour > 12) {
		// Assume that this PM
		nAmPm = 12 * 60 * 60;
	    } else {
		// Assume that this is AM
		nAmPm = 0;
	    }
	} else {
	    // 24 hour clock
	    nAmPm = 0;
	}
    } else if (strcasecmp(strAmPm.c_str(), strPm.c_str()) == 0) {
	nAmPm = 12 * 60 * 60;
    } else if (strcasecmp(strAmPm.c_str(), strAm.c_str()) == 0) {
	nAmPm = 0;
    } else {
	nReturn = -1;		// AM/PM indicator is invalid
    }

    // Continue if no errors
    if (nReturn == 0) {
	// Is everything else numeric
	if (::TestNumeric(strHour)
	    &&::TestNumeric(strHour)) {
	    nHour = atoi(strHour.c_str());
	    nMinute = atoi(strMinute.c_str());
	    if ((strAmPm.length() == 0 && nHour >= 0 && nHour <= 24)
		|| (strAmPm.length() > 0 && nHour >= 1 && nHour <= 12)) {
		// The hour is good, so far
		if (nMinute >= 0 && nMinute <= 59) {
		    // Don't allow minutes other than zero for last hour of 24 hour clock
		    if ((strAmPm.length() != 0 || nHour != 24
			 || nMinute != 0)) {
			// The minutes are good also, set up the time
			nTime = nAmPm + nHour * 60 * 60 + nMinute * 60;
			if (strAmPm.length() > 0 && nHour == 12) {
			    // Correct for 12:00 noon - 12:59 PM and 12:00 midnight - 12:59 AM
			    nTime -= 12 * 60 * 60;
			}
		    } else {
			nReturn = -2;	// Minutes too high for last hour of 24 hour clock
		    }
		} else {
		    nReturn = -3;	// Minutes must be between 0 and 59
		}
	    } else {
		if (strAmPm.length() == 0) {
		    nReturn = -4;	// The hour is out of range 0 through 24
		} else {
		    nReturn = -5;	// The hour is out of range 1 through 12
		}
	    }
	} else {
	    if (::TestNumeric(strHour) == false) {
		nReturn = -6;	// The hour is not numeric
	    } else		// if (::TestNumeric(strHour)==false)
	    {
		nReturn = -7;	// The minutes are not numeric
	    }
	}
    }

    return (nReturn);
}


//--------------------------------------------------------------//
// Determine the number of weeks between two dates.  The number //
// of weeks is calculated as the number of weeks between the    //
// two Sunday's prior to both dates.                            //
//--------------------------------------------------------------//
int
WeeksBetween(time_t nDate1, time_t nDate2)
{
    struct tm *pTm;

    // Don't need the Sunday prior to the first date

    // Get the Sunday prior to the second date
    pTm = localtime(&nDate2);
    nDate2 = SubtractDays(nDate2, pTm->tm_wday);

    // Get the difference in weeks
    return (DaysBetween(nDate1, nDate2) / 7);
}


//--------------------------------------------------------------//
// Determine the number of years between two dates.             //
//--------------------------------------------------------------//
int
YearsBetween(time_t nDate1, time_t nDate2)
{
    int nYear1;
    struct tm *pTm = localtime(&nDate1);

    nYear1 = pTm->tm_year;
    pTm = localtime(&nDate2);
    return (nYear1 - pTm->tm_year);
}
