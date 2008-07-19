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


/*
 * Sun clock - astronomical routines.
 */

#include "sunclock.h"

/*  JDATE  --  Convert internal GMT date and time to Julian day
	       and fraction.  */

long
jdate(t)
     struct tm *t;
{
    long c, m, y;

    y = t->tm_year + 1900;
    m = t->tm_mon + 1;

    if (m > 2)
	m = m - 3;
    else {
	m = m + 9;
	y--;
    }
    c = y / 100L;		/* Compute century */
    y -= 100L * c;

    return t->tm_mday + (c * 146097L) / 4 + (y * 1461L) / 4 +
	(m * 153L + 2) / 5 + 1721119L;
}

/* JTIME --    Convert internal GMT  date  and	time  to  astronomical
	       Julian  time  (i.e.   Julian  date  plus  day fraction,
	       expressed as a float).	*/

float
jtime(t)
     struct tm *t;
{
#ifdef NOTUSED
    long val = t->tm_sec + (60L * (t->tm_min + 60L * t->tm_hour));
    float ret = (((float) (jdate(t) + val)) - 0.5) / 86400.0;
    return (ret);
#endif

    return ((jdate(t) - 0.5) +
	    (((long) t->tm_sec) +
	     60L * (t->tm_min + 60L * t->tm_hour)) / 86400.0);
}

/*  KEPLER  --	Solve the equation of Kepler.  */

float
kepler(m, ecc)
     float m, ecc;
{
    float e, delta;
#define EPSILON 1E-6

    e = m = dtr(m);
    do {
	delta = e - ecc * sin(e) - m;
	e -= delta / (1 - ecc * cos(e));
    } while (abs(delta) > EPSILON);
    return e;
}

/*  SUNPOS  --	Calculate position of the Sun.	JD is the Julian  date
		of  the  instant for which the position is desired and
		APPARENT should be nonzero if  the  apparent  position
		(corrected  for  nutation  and aberration) is desired.
                The Sun's co-ordinates are returned  in  RA  and  DEC,
		both  specified  in degrees (divide RA by 15 to obtain
		hours).  The radius vector to the Sun in  astronomical
                units  is returned in RV and the Sun's longitude (true
		or apparent, as desired) is  returned  as  degrees  in
		SLONG.	*/

void
sunpos(jd, apparent, ra, dec, rv, slong)
     float jd;
     int apparent;
     float *ra, *dec, *rv, *slong;
{
    float t, t2, t3, l, m, e, ea, v, theta, omega, eps;

    /* Time, in Julian centuries of 36525 ephemeris days,
       measured from the epoch 1900 January 0.5 ET. */

    t = (jd - 2415020.0) / 36525.0;
    t2 = t * t;
    t3 = t2 * t;

    /* Geometric mean longitude of the Sun, referred to the
       mean equinox of the date. */

    l = fixangle(279.69668 + 36000.76892 * t + 0.0003025 * t2);

    /* Sun's mean anomaly. */

    m = fixangle(358.47583 + 35999.04975 * t - 0.000150 * t2 -
		 0.0000033 * t3);

    /* Eccentricity of the Earth's orbit. */

    e = 0.01675104 - 0.0000418 * t - 0.000000126 * t2;

    /* Eccentric anomaly. */

    ea = kepler(m, e);

    /* True anomaly */

    v = fixangle(2 * rtd(atan(sqrt((1 + e) / (1 - e)) * tan(ea / 2))));

    /* Sun's true longitude. */

    theta = l + v - m;

    /* Obliquity of the ecliptic. */

    eps = 23.452294 - 0.0130125 * t - 0.00000164 * t2 + 0.000000503 * t3;

    /* Corrections for Sun's apparent longitude, if desired. */

    if (apparent) {
	omega = fixangle(259.18 - 1934.142 * t);
	theta = theta - 0.00569 - 0.00479 * sin(dtr(omega));
	eps += 0.00256 * cos(dtr(omega));
    }

    /* Return Sun's longitude and radius vector */

    *slong = theta;
    *rv = (1.0000002 * (1 - e * e)) / (1 + e * cos(dtr(v)));

    /* Determine solar co-ordinates. */

    *ra =
	fixangle(rtd
		 (atan2(cos(dtr(eps)) * sin(dtr(theta)), cos(dtr(theta)))));
    *dec = rtd(asin(sin(dtr(eps)) * sin(dtr(theta))));
}

/*  GMST  --  Calculate Greenwich Mean Siderial Time for a given
	      instant expressed as a Julian date and fraction.	*/

float
gmst(jd)
     float jd;
{
    float t, theta0;


    /* Time, in Julian centuries of 36525 ephemeris days,
       measured from the epoch 1900 January 0.5 ET. */

    t = ((floor(jd + 0.5) - 0.5) - 2415020.0) / 36525.0;

    theta0 = 6.6460656 + 2400.051262 * t + 0.00002581 * t * t;

    t = (jd + 0.5) - (floor(jd + 0.5));

    theta0 += (t * 24.0) * 1.002737908;

    theta0 = (theta0 - 24.0 * (floor(theta0 / 24.0)));

    return theta0;
}
