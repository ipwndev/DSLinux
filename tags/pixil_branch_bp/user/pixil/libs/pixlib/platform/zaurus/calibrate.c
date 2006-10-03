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



/* Feature test switches */


/* System header files */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


/* Local header files */
#include <h3600_ts.h>
#include <pixlib/pixlib.h>

/* Typedef, macros, enum/struct/union defintions */
#define					CAL_GETX(f,p)		(((f) == 0) ? ((p).x) : ((p).y))
#define					CAL_GETY(f,p)		(((f) == 0) ? ((p).y) : ((p).x))


#define					CAL_FLAGS_EHAND		0x01	/* Exit handler has been registered */
#define					CAL_FLAGS_DEVOPEN	0X02	/* Device file has been opened */

#define					CAL_DFLT_DEVNAME	"/dev/h3600_tsraw"	/* Default FQPN of the device file */
#define					CAL_MAX_SAMPLE		5
#define					CAL_NUM_CTRLPTS		5
#define					CAL_OFFVAL1			30
#define					CAL_OFFVAL2			50


/* Global scope variables */


/* Local scope variables */
static char *devname;		/* Device name */
static unsigned char cal_flags = 0;	/* Global flags variable */
static int devfd = -1,		/* Device file descriptor */
  rotated = 1,			/* Default to the color ipaq */
  screen_h,			/* Screen height */
  screen_w;			/* Screen width */

/* Static function prototypes */
static void cal_ExitHandler(void);
static int cal_OpenDev();

/*-----------------------------------------------------------------------------*\
--
--	Static function definitions
--
\*-----------------------------------------------------------------------------*/

/*******************************************************************************\
**
**	Function:	void cal_ExitHandler()
**	Desc:		Exit handler for this module, responsible for  closing the
**				file descriptor
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
static void
cal_ExitHandler(void)
{
    /* Close the open file descriptor for the device */
    if ((cal_flags & CAL_FLAGS_DEVOPEN) && devfd > -1)
	close(devfd);

    return;
}				/* end of cal_ExitHandler() */

/*******************************************************************************\
**
**	Function:	int cal_OpenDev()
**	Desc:		Opens the device file and sets up the appropriate global flags
**				to handle clean up.
**	Accepts:	Nothing (void)
**	Returns:	int; >= 0 the fd, or -1 on error
**
\*******************************************************************************/
static int
cal_OpenDev(void)
{
    int fd = -1;		/* File descriptor to pass back */

    /* Check the environment variable to see if we are overriding the default device file */
    if ((devname = getenv("CAL_DEVICE")) == NULL)
	devname = CAL_DFLT_DEVNAME;

    if ((fd = open(devname, O_RDONLY)) != -1) {
	cal_flags |= CAL_FLAGS_DEVOPEN;
	if (!(cal_flags & CAL_FLAGS_EHAND)) {
	    if (atexit(cal_ExitHandler) == 0)
		cal_flags |= CAL_FLAGS_EHAND;
	}			/* end of if */
    }
    /* end of if */
    return (fd);
}				/* end of cal_OpenDev() */

/*-----------------------------------------------------------------------------*\
--
--	External function definitions
--
\*-----------------------------------------------------------------------------*/

/*******************************************************************************\
**
**	Function:	int pix_cal_GetCtrlPts()
**	Desc:		Gets the calibration control points for this device
**	Accepts:	int *npts = Ptr to storage for number of poitns
**				CalPt_t **ctrldata = Ptr to Ptr of address to dynamically allocate
**									 memory
**				int w = screen width
**				int h = screen height
**				int bpp = Bits per pixel
**	Returns:	int; 0 on success -1 on error
**
\*******************************************************************************/
int
pix_cal_GetCtrlPts(int *npts, CalPt_t ** ctrldata, int w, int h, int bpp)
{
    /* Assign the memory */
    if (ctrldata == NULL)
	return (-1);

    /* We are allocating CAL_NUM_CTRLPTS */
    if ((*ctrldata =
	 (CalPt_t *) calloc(CAL_NUM_CTRLPTS, sizeof(CalPt_t))) == NULL)
	return (-1);
    if (npts)
	*npts = CAL_NUM_CTRLPTS;

    /* Determine the rotation */

    /*
       ** NOTE: This is a big time hack...Apparently the B/W ipaqs are rotated 180 degrees
       ** from their color counterparts...
     */
    if (bpp == 4)
	rotated = 2;

    if (rotated) {
	screen_w = h;
	screen_h = w;
    } /* end of if */
    else {
	screen_w = w;
	screen_h = h;
    }				/* end of else */

    (*ctrldata)[0].x = CAL_OFFVAL1;
    (*ctrldata)[0].y = CAL_OFFVAL1;
    (*ctrldata)[1].x = screen_w - CAL_OFFVAL2 - 1;
    (*ctrldata)[1].y = CAL_OFFVAL2;
    (*ctrldata)[2].x = CAL_OFFVAL2;
    (*ctrldata)[2].y = screen_h - CAL_OFFVAL2 - 1;
    (*ctrldata)[3].x = screen_w - CAL_OFFVAL1 - 1;
    (*ctrldata)[3].y = screen_h - CAL_OFFVAL1 - 1;
    (*ctrldata)[4].x = screen_w / 2;
    (*ctrldata)[4].y = screen_h / 2;

    return (0);
}				/* end of pix_cal_GetCtrlPts() */

/*******************************************************************************\
**
**	Function:	int pix_cal_GetDrawpt()
**	Desc:		Gets the draw points (based on any rotation that is necessary)
**	Accepts:	CalPt_t *ctrlpt = Ptr to the Control point
**				CalPt_t *drawpt = Ptr to the storage for draw point
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
int
pix_cal_GetDrawPt(CalPt_t * ctrlpt, CalPt_t * drawpt)
{
    if (ctrlpt == NULL || drawpt == NULL)
	return (-1);

    if (rotated == 1) {
	/* This is the color ipaq */
	drawpt->x = ctrlpt->y;
	drawpt->y = screen_w - ctrlpt->x;
    } /* end of if */
    else if (rotated == 2) {
	/* This is the gray-scale ipaq */
	drawpt->x = screen_h - ctrlpt->y;
	drawpt->y = screen_w - (screen_w - ctrlpt->x);
    } /* end of else if */
    else {
	/* No rotation necessary */
	drawpt->x = ctrlpt->x;
	drawpt->y = ctrlpt->y;
    }				/* end of else */

    return (0);
}				/* end of pix_cal_GetDrawPt() */

/*******************************************************************************\
**
**	Function:	int pix_cal_GetDataPt()
**	Desc:		Gets the raw data (point data) from the device file and stores
**				it into the CalPt_t structure passed in
**	Accepts:	CalPt_t *ptdata = Ptr to storage of x/y coordinates
**	Returns:	int; 0 on successful read, -1 on error
**
\*******************************************************************************/
int
pix_cal_GetDataPt(CalPt_t * ptdata)
{
    int nsample = 0,		/* Current number of samples received */
      rc = 0,			/* Result code */
      rdsz;			/* read size */
    CalPt_t cur_data = { 0, 0 };	/* Current data */
    TS_EVENT ts_ev;		/* Event structure */

    /*
       ** Determine if the device needs to be opened (this will only happen the first
       ** time this function is called
     */
    if (!(cal_flags & CAL_FLAGS_DEVOPEN) || devfd == -1) {
	if ((devfd = cal_OpenDev()) == -1)
	    return (devfd);
    }

    /* end of if */
    /* Get CAL_MAX_SAMPLE samples to average out */
    while (1) {
	rdsz = 0;
	while (rdsz < sizeof(TS_EVENT)) {
	    if ((rc = read(devfd, &ts_ev, sizeof(TS_EVENT))) == -1) {
		if (errno == EINTR)
		    continue;
		else
		    break;
	    }			/* end of if */
	    rdsz += rc;
	}			/* end of if */

	/* Again, check for error condition */
	if (rc == -1)
	    break;

	printf("DATA[%d] %d, %d, %d\n", nsample, ts_ev.x, ts_ev.y, ts_ev.pressure);

	if (ts_ev.pressure == 0 && nsample > CAL_MAX_SAMPLE) {
	    break;
	} /* end of if */
	else if (ts_ev.x >= 0 && ts_ev.y >= 0) {
	    /*
	       ** The if conditional was added 10/29/01 -- JMW, because holding down the stylus
	       ** would cause erroneous readings due to pressure variance....If this isn't the 
	       ** case, please correct me....
x	     */
	    if (nsample <= CAL_MAX_SAMPLE) {
		cur_data.x += ts_ev.x;
		cur_data.y += ts_ev.y;
		nsample++;
	    }			/* end of if */
	}			/* end of else-if */
    }				/* end of while */

    if (nsample) {
	cur_data.x /= nsample;
	cur_data.y /= nsample;
	memcpy(ptdata, &cur_data, sizeof(CalPt_t));
    }
    /* end of if */
    return ((nsample == 0) ? -1 : 0);
}				/* end of pix_cal_GetDataPtr() */

/*******************************************************************************\
**
**	Function:	int pix_cal_Calibrate()
**	Desc:		ipaq calibration routine which calculates the calibration values
**				based on the control and user specific data points
**	Accepts:	int npts = Number of control/data points to process
**				CalPt_t *ctrldata = Array of npts control points
**				CalPt_t *userdata = Array of npts user points
**	Returns:	int; 0 on success, -1 on error, PIXLIB_STUB_VAL for a stubbed
**				function.
**
\*******************************************************************************/
int
pix_cal_Calibrate(int npts, CalPt_t * ctrldata, CalPt_t * userdata)
{
    int x, y, dx, dy, flag;
    int xscale0, xscale1, xtrans0, xtrans1;
    int yscale0, yscale1, ytrans0, ytrans1;
    int xyswap, xraw, yraw;
    TS_CAL new_cal;		/* Calibration values to send to kernel */

    flag = 0;

    /* See if the device file needs to be opened */
    if (!(cal_flags & CAL_FLAGS_DEVOPEN) || devfd == -1) {
	if ((devfd = cal_OpenDev()) == -1) {
	    printf("Unable to open device!\n");
	    return (devfd);
	}
    }

    /* end of if */
    /* Calculate ABS(x1-x0) */
    dx = ((x = userdata[1].x - userdata[0].x) < 0 ? -x : x);
    dy = ((y = userdata[1].y - userdata[0].y) < 0 ? -y : y);
    /* CF

       (0,0) --------->Y
       X        +-------+---------------+-
       |        |       |               |
       V        |       |xa0,ya0        |
       +-------O---------------+ --
       |        |               | dx or dy
       +-------+---------------O --
       ^        |       |               | xa1,ya1
       |        |       |               |
       Y        +-------+---------------+
       (0,0) -------->X

       Work out where the origin is, either at the TLH or BLH corners.
       Initialise xp such that it points to the array containing the X coords
       Initialise yp such that it points to the array containing the Y coords
     */
    if (dx < dy) {
	xyswap = 1;
    } else {
	xyswap = 0;
    }

    xraw = CAL_GETX(xyswap, userdata[4]);
    yraw = CAL_GETY(xyswap, userdata[4]);
    /*

       We have MAX_CAL_POINTS sets of x,y coordinates.

       If we plot Xcal against Xraw we get two equations, each of a straight
       line. One for the Xcoord and the other for the Y coord.
       This line models the linear characteristics of the ts A/D
       converter.

       Xcal = m*Xraw + Cx
       Ycal = m*Yraw + Cy

       X/Ycal is the calibrated coord which is the pixel pos on the screen.
       X/Yraw is the uncalibrated X coord.
       m is the xscale ( gradient of line)
       Cx/y is the trans (constant)

       xscale
       'm' can be got by calculating the gradient between two data points
       Example Xscale0 = (Xcal1 - Xcal0 ) / (Xraw1 - Xraw0)

       trans = Xcal - mXraw
       What is actualy done is to take the Ave of two measurements
       Example  Xtrans0 = ( (Xcal0 - mXraw0) + (Xcal3 - mXraw3) ) / 2

       We repeat the above procedure to calculate 
       Yscale0 and Ytrans0 and repeat the whole lot again using two
       new data indexes 1 and 2 giving 4 new variables
       Xscale1, Xtrans1, Yscale1,Ytrans1, making a total of eight.

       The final calibration variables are the average of data ponts 
       0,3 and 1,2
       xscale = (Xscale0 + Xscale1) / 2
       yscale = (Yscale0 + Yscale1) / 2
       xtrans = (Xtrans0 + Xtrans1) /2
       ytrans = (Ytrans0 + Ytrans1) /2

     */
    xscale0 = ((ctrldata[0].x - ctrldata[3].x) << 8) /
	(CAL_GETX(xyswap, userdata[0]) - CAL_GETX(xyswap, userdata[3]));

    xtrans0 =
	((ctrldata[0].x - ((CAL_GETX(xyswap, userdata[0]) * xscale0) >> 8)) +
	 (ctrldata[3].x -
	  ((CAL_GETX(xyswap, userdata[3]) * xscale0) >> 8))) / 2;

    yscale0 = ((ctrldata[0].y - ctrldata[3].y) << 8) /
	(CAL_GETY(xyswap, userdata[0]) - CAL_GETY(xyswap, userdata[3]));

    ytrans0 =
	((ctrldata[0].y - ((CAL_GETY(xyswap, userdata[0]) * yscale0) >> 8)) +
	 (ctrldata[3].y -
	  ((CAL_GETY(xyswap, userdata[3]) * yscale0) >> 8))) / 2;

    xscale1 = ((ctrldata[1].x - ctrldata[2].x) << 8) /
	(CAL_GETX(xyswap, userdata[1]) - CAL_GETX(xyswap, userdata[2]));

    xtrans1 =
	((ctrldata[1].x - ((CAL_GETX(xyswap, userdata[1]) * xscale1) >> 8)) +
	 (ctrldata[2].x -
	  ((CAL_GETX(xyswap, userdata[2]) * xscale1) >> 8))) / 2;

    yscale1 = ((ctrldata[1].y - ctrldata[2].y) << 8) /
	(CAL_GETY(xyswap, userdata[1]) - CAL_GETY(xyswap, userdata[2]));

    ytrans1 =
	((ctrldata[1].y - ((CAL_GETY(xyswap, userdata[1]) * yscale1) >> 8))
	 + (ctrldata[2].y -
	    ((CAL_GETY(xyswap, userdata[2]) * yscale1) >> 8))) / 2;

    new_cal.xscale = (xscale0 + xscale1) / 2;
    new_cal.xtrans = (xtrans0 + xtrans1) / 2;
    new_cal.yscale = (yscale0 + yscale1) / 2;
    new_cal.ytrans = (ytrans0 + ytrans1) / 2;
    new_cal.xyswap = xyswap;
#ifdef FOO
    new_cal.yscale = (xscale0 + xscale1) / 2;
    new_cal.ytrans = (xtrans0 + xtrans1) / 2;
    new_cal.xscale = (yscale0 + yscale1) / 2;
    new_cal.xtrans = (ytrans0 + ytrans1) / 2;
    new_cal.xyswap = xyswap;
#endif

    /* Now check it with center coords */
    printf("CHECK with Center Coords (160,120): xraw=%d yraw=%d\n",xraw,yraw);
    x = ((new_cal.xscale * xraw) >> 8) + new_cal.xtrans;
    y = ((new_cal.yscale * yraw) >> 8) + new_cal.ytrans;
    printf("CHECK: x(center)=%d y(center)=%d\n",x,y);

    /* store this calibration in the device */
    if (ioctl(devfd, TS_SET_CAL, (void *) &new_cal) != 0) {
	perror("TS_SET_CALIBRATION ioctl fail\n");
	return -1;
    }

    printf("SET %d, %d, %d, %d, %d\n", 
	   new_cal.xscale, new_cal.yscale, new_cal.xtrans, new_cal.ytrans,
	   new_cal.xyswap);

    printf("Set the calibration\n");
    return 0;
}				/* end of pix_cal_Calibrate() */
