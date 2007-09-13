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


/* System header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

/* Local header files */
#include <nano-X.h>
#include <pixlib/pixlib.h>
#include "cal_ui.h"

/* Typedef, macro, enum/struct/union defnitions */


/* Global scope variables */


/* File scope variables */
static GR_WINDOW_ID cal_win;	/* Calibration window */
static GR_SCREEN_INFO si;	/* Screen info */
static int g_text_x = START_TEXT_Y;	/* Default value, taken from nxcal.c */
static CalPt_t last = { -1, -1 };	/* Previous coordinates */


/* Static function prototypes */
static void nxcal_drawCH(CalPt_t * pt, int flags);
static void nxcal_drawPt(CalPt_t * pt, int flags);
static void nxcal_drawText(char *text);
static void nxcal_getCtrlPts(int size, CalPt_t * pts);
/*static void nxcal_normalizePts(int npts, CalPt_t *ctrlpts, CalPt_t *newpts);*/
static int nxcal_getStoredPts(int *npts, CalPt_t ** cpts, CalPt_t ** dpts,
			      char *calfile);
static int nxcal_grInit(int flags);
static int nxcal_setStoredPts(int npts, CalPt_t * cpts, CalPt_t * dpts,
			      char *calfile);
static int nxcal_ValidatePt(int npts, CalPt_t * dpts);

/*-----------------------------------------------------------------------------*\
**
**	Static function definitions
**
\*-----------------------------------------------------------------------------*/

/*******************************************************************************\
**
**	Function:	void nxcal_drawCH()
**	Desc:		Handles the actual raw nanox calls to draw the cross hair at the
**				specified location
**	Accepts:	CalPt_t *pt = Point (x,y) to draw the center of the cross
**				int flag = How to draw
**	Returns:	Nothing (void)
**
\*******************************************************************************/
static void
nxcal_drawCH(CalPt_t * pt, int flag)
{
    GR_EVENT event;

    int hx_coord1,
	hx_coord2,
	hy_coord1, hy_coord2, vx_coord1, vx_coord2, vy_coord1, vy_coord2;
    GR_GC_ID gc = GrNewGC();

    /* Get the horizontal and vertical lines */
    hx_coord1 = pt->x - (SZ_XHAIR / 2);
    hx_coord2 = pt->x + (SZ_XHAIR / 2);
    hy_coord1 = hy_coord2 = pt->y;

    vy_coord1 = pt->y - (SZ_XHAIR / 2);
    vy_coord2 = pt->y + (SZ_XHAIR / 2);
    vx_coord1 = vx_coord2 = pt->x;

    GrSetGCBackground(gc, NXCAL_BACKGROUND);
    GrSetGCForeground(gc, NXCAL_FOREGROUND);

    if (!flag) {
	GrSetGCMode(gc, GR_MODE_XOR);
	GrSetGCForeground(gc, NXCAL_FOREGROUND);
    } /* end of if */
    else {
	GrSetGCMode(gc, GR_MODE_SET);
	GrSetGCForeground(gc, NXCAL_BACKGROUND);
    }				/* end of else */

    GrLine(cal_win, gc, hx_coord1, hy_coord1, hx_coord2, hy_coord2);
    GrLine(cal_win, gc, vx_coord1, vy_coord1, vx_coord2, vy_coord2);

    GrDestroyGC(gc);

    while (GrPeekEvent(&event))
	GrGetNextEvent(&event);
}				/* end of nxcal_drawCH() */

/*******************************************************************************\
**
**	Function:	void nxcal_drawPt()
**	Desc:		Draws the cross hair at the specified point,actually, sets things
**				things up and leaves the actual drawing of the cross hair to
**				nxcal_drawCH().
**	Accepts:	CalPt_t *pt = The point to draw at
**				int flags = Value to determine if drawing or erasing, etc
**	Returns:	Nothing (void)
**
\*******************************************************************************/
static void
nxcal_drawPt(CalPt_t * pt, int flags)
{
    int ani_frames = 20,	/* Number of CH to draw */
      i;			/* Loop iterator */
    CalPt_t cur_pt;		/* Current point */

    if (flags) {
	if (last.x != -1)
	    nxcal_drawCH(&last, flags);
	return;
    }
    /* end of if */
    if (last.x == -1) {
	last.x = pt->x;
	last.y = pt->y;
    }
    /* end of if if */
    if (last.x != pt->x || last.y != pt->y) {
	/* Erase the previous plus */
	nxcal_drawCH(&last, 1);

	for (i = 0; i < ani_frames; i++) {
	    cur_pt.x = last.x + ((pt->x - last.x) * i / ani_frames);
	    cur_pt.y = last.y + ((pt->y - last.y) * i / ani_frames);
	    nxcal_drawCH(&cur_pt, 0);
	    usleep(60);
	    nxcal_drawCH(&cur_pt, 0);
	}			/* end of for */
	last.x = pt->x;
	last.y = pt->y;
    }
    /* end of if */
    nxcal_drawCH(&last, 0);
}				/* end of nxcal_drawPt() */

/*******************************************************************************\
**
**	Function:	void nxcal_drawText()
**	Desc:		Displays the text message to the screen
**	Accepts:	char *text = Ptr to the text message
**	Returns:	Nothing (void)
**
\*******************************************************************************/
static void
nxcal_drawText(char *text)
{
    GR_GC_ID gc = GrNewGC();
    GR_SIZE tw, th, tb;

    /* Set up the GC */
    GrSetGCBackground(gc, NXCAL_BACKGROUND);
    GrSetGCForeground(gc, NXCAL_FOREGROUND);
    GrSetGCMode(gc, GR_MODE_SET);

    /* Determine the position of the text and draw it */
    GrGetGCTextSize(gc, text, -1, 0, &tw, &th, &tb);
    GrText(cal_win, gc, (si.vs_width - tw) / 2, g_text_x, text, -1, 0);
    g_text_x += th + 3;

    /* Destroy resources and return */
    GrDestroyGC(gc);
    return;
}				/* end of nxcal_drawText() */

/*******************************************************************************\
**
**	Function:	void nxcal_getCtrlPts()
**	Desc:		Gets the CONTROL points to process against
**	Accepts:	int npts = Number of points t`o accept
**				CalPt_t *pts = array to store points into (has size of npts elements)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
static void
nxcal_getCtrlPts(int npts, CalPt_t * pts)
{
    pts[0].x = pts[0].y = CAL_OFFSET1;
    pts[1].x = si.vs_width - CAL_OFFSET2 - 1;
    pts[1].y = CAL_OFFSET2;
    pts[2].x = CAL_OFFSET2;
    pts[2].y = si.vs_height - CAL_OFFSET2 - 1;
    pts[3].x = si.vs_width - CAL_OFFSET1 - 1;
    pts[3].y = si.vs_height - CAL_OFFSET1 - 1;
    pts[4].x = si.vs_width / 2;
    pts[4].y = si.vs_height / 2;

    return;
}				/* end of nxcal_getCtrlPts() */

/*******************************************************************************\
**
**	Function:	int nxcal_getStoredPts()
**	Desc:		Function used to retreive the stored calibration points from the
**				file
**	Accepts:	int *npts = Number of pts read from file 
**				CalPt_t **cpts = Storage for control points to be dynamically created
**				CalPt_t **dpts = Storage for data points to be dynamically created
**				char *calfile = Path of the file
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
static int
nxcal_getStoredPts(int *npts, CalPt_t ** cpts, CalPt_t ** dpts, char *calfile)
{
    char *cp,			/* Generic pointer */
     *dp,			/* Pointer to data points */
      filebuf[255];		/* File buffer */
    int alc_cnt = 3,		/* Initial number of entries to expect */
      ncnt = 0;			/* Number of points read in */
    FILE *infp;			/* In file pointer */
    CalPt_t *tmp;		/* Use in reallocs */

    if (calfile == NULL || (infp = fopen(calfile, "r")) == NULL) {
	return (-1);
    }

    /* end of if */
    /* Initially allocate alc_cnt elements */
    if ((*cpts = calloc(alc_cnt, sizeof(CalPt_t))) == NULL ||
	(*dpts = calloc(alc_cnt, sizeof(CalPt_t))) == NULL) {
	return (-1);
    }

    /* end of if */
    /*
       ** Expected format of the file:
       ** x,y\n
       ** ...
     */
    while (fgets(filebuf, sizeof(filebuf), infp) != NULL) {
	/* Determine if filebuf contains valid point data */
	if (toupper(filebuf[0]) != 'C')
	    continue;

	/* See if more memory needs to be allocated */
	if (ncnt >= alc_cnt) {
	    tmp = realloc(*cpts, (alc_cnt + 1) * sizeof(CalPt_t));
	    if (tmp == NULL)
		break;
	    *cpts = tmp;
	    tmp = realloc(*dpts, (alc_cnt + 1) * sizeof(CalPt_t));
	    if (tmp == NULL)
		break;
	    *dpts = tmp;
	    alc_cnt++;
	}

	/* end of if */
	/* Parse the string */
	if ((dp = strchr(filebuf, 'D')) != NULL
	    || (dp = strchr(filebuf, 'd')) != NULL) {
	    *dp = '\0';
	    dp++;
	}

	/* end of if */
	/* Get the control points */
	if ((cp = strchr(filebuf, ',')) == NULL)
	    continue;
	*cp = '\0';
	cp++;
	(*cpts)[ncnt].x = atoi(&filebuf[1]);
	(*cpts)[ncnt].y = atoi(cp);

	/* Get the data points */
	if ((cp = strchr(dp, ',')) == NULL)
	    continue;
	*cp = '\0';
	cp++;
	(*dpts)[ncnt].x = atoi(dp);
	(*dpts)[ncnt].y = atoi(cp);

	ncnt++;
    }				/* end of while */

    if (*npts)
	*npts = ncnt;

    fclose(infp);

    return ((ncnt == 0) ? -1 : 0);
}				/* end of nxcal_getStoredPts() */

/*******************************************************************************\
**
**	Function:	int nxcal_grInit()
**	Desc:		Initializes the underlying graphics engine (in this particular
**				case, microwindows) and setups up the main window for this
**	Accepts:	int flags = flags for nanox initialization; where:
**					NXCAL_NONINT_MODE = Initialize the graphics engine
**	Retruns:	int; 0 on success, -1 otherwise
**
\*******************************************************************************/
static int
nxcal_grInit(int flags)
{
    GR_WM_PROPERTIES win_props;	/* Window properties */
    GR_GC_ID gc;

    /* Make connection to the Nano-X server */
#if 0
    if (flags & NXCAL_NONINT_MODE) {
	if (GrOpen() < 0)
	    return (-1);
    }				/* end of if  */
#endif

    /* Get the screen sizes */
    GrGetScreenInfo(&si);

    /* Create a new window, to cover the entire width of the screen */
    cal_win = GrNewWindow(GR_ROOT_WINDOW_ID, 0, 0, si.vs_width, si.vs_height,
			  1, NXCAL_BACKGROUND, NXCAL_BACKGROUND);

    memset(&win_props, 0, sizeof(win_props));
    GrGetWMProperties(cal_win, &win_props);
    win_props.props |= GR_WM_PROPS_NODECORATE;

    win_props.flags |= GR_WM_FLAGS_PROPS;
    GrSetWMProperties(cal_win, &win_props);

    /* Map the window */
    GrMapWindow(cal_win);

    /* Force the window to be blank */
    gc = GrNewGC();
    GrSetGCBackground(gc, NXCAL_BACKGROUND);
    GrSetGCForeground(gc, NXCAL_FOREGROUND);
    GrSetGCMode(gc, GR_MODE_SET);
    GrRect(cal_win, gc, 0, 0, si.vs_width, si.vs_height);

    GrDestroyGC(gc);
    return 0;
}				/* end of nxcal_grInit(void) */

#if 0
/*******************************************************************************\
**
**	Function:	void nxcal_normalizePts()
**	Desc:		Determines if any of the X/Y axis are flipped, and if they are
**				it will rearrange the order of the newpts array to feed into
**				the calibrate function properly.
**	Accepts:	int npts = Number of points to deal with
**				CalPt_t *ctrlpts = Array of "control" points
**				CalPt_t *newpts = Array of points from the user
**	Returns:	Nothing (void)
**
\*******************************************************************************/
static void
nxcal_normalizePts(int npts, CalPt_t * ctrlpts, CalPt_t * newpts)
{
    AxisMode_t eAxMode = amNONE;	/* No axis rotation/flipping required */
    CalPt_t tmppt1,		/* Temporary point */
      tmppt2;			/* Tmp pt #2 */

    /*
       ** Determine the type of axis flipping/rotation (if any) that may be 
       ** required on this device:
       **
       **   Map of the pts on the device:
       **
       **   +-------------+
       **  |0            |
       **  |           1 |
       **  |             |
       **  |             |
       **  |      4      |
       **  |             |
       **  |             |
       **  | 2           |
       **  |            3|
       **  +-------------+
     */


    /* Adjust the array based on type of rotation */
    switch (eAxMode) {
    case amXFLIP:
	break;
    case amYFLIP:
	break;
    case amXYFLIP:
	break;
    }				/* end of switch */

    return;
}				/* end of nxcal_normalizePts() */
#endif
/*******************************************************************************\
**
**	Function:	int nxcal_setStoredPts()
**	Desc:		Stores the current calibrated points into the file specified by
**				the calfile parameter
**	Accepts:	int npts = The number of pts stored in pts array
**				CalPt_t *cpts = The array of control pts to store
**				CalPt_t *dpts = The array of data pts to store
**				char *calfile = File to write data out to
**	Returns:	int; 0 on successful write of data, -1 otherwise
**
\*******************************************************************************/
static int
nxcal_setStoredPts(int npts, CalPt_t * cpts, CalPt_t * dpts, char *calfile)
{
    int i,			/* Loop iterator */
      retval = -1;		/* Return value */
    FILE *outfp;		/* Output file */

    if (calfile == NULL || (outfp = fopen(calfile, "w")) == NULL)
	return (retval);

    for (i = 0; i < npts; i++) {
	char buf[20 + 1];	/* Data */
	int dta_len;		/* Length of data */

	/* Get the length if the data */
	snprintf(buf, sizeof(buf), "C%d,%dD%d,%d", cpts[i].x, cpts[i].y,
		 dpts[i].x, dpts[i].y);
	dta_len = strlen(buf);
	if (fprintf(outfp, "%s\n", buf) != dta_len + 1)
	    break;
    }				/* end of for */

    if (i == npts)
	retval = 0;

    fclose(outfp);

    return (retval);
}				/* end of nxcal_setStoredPts() */

/******************************************************************************\
**
**	Function:	int nxcal_ValidatePt()
**	Desc:		Calculates the final points based on opposing points in the
**				array.  Right now, for this initial version, it only recognizes
**				calibration data with 3 or 5 points.
**	Accepts:	int npts = Number of points in array
**				CalPt_t *dpts = Array of data points
**	Returns:	int;	0 = Actual point was within calculated range
**						-1 = Actual point was outside of calculated range
**
\******************************************************************************/
static int
nxcal_ValidatePt(int npts, CalPt_t * dpts)
{
    int deltax,			/* Change in x */
      deltay,			/* Change in y */
      retval = -1;		/* Return value, default to fail */
    float fudgex,		/* Fudge factor in x */
      fudgey;			/* Fudge factor in y */
    CalPt_t validpt1,		/* Validation point #1 */
      validpt2;			/* Validation point #2 */

    /*
       ** NOTE:  This is an initial attempt at calculating a "good calibration", things may need
       ** to be adjusted depending on how loose/tight the definition of "good calibration" is 
       ** defined.
     */

    switch (npts) {
    case 3:
	/* Need to check the points to make sure they are not the same value, or around the same value */
	deltax = abs(dpts[0].x - dpts[1].x);
	deltay = abs(dpts[0].y - dpts[1].y);
	if (deltax < 100 || deltay < 50)
	    break;

	validpt1.x = (dpts[0].x + dpts[1].x) / 2;
	validpt1.y = (dpts[0].y + dpts[1].y) / 2;
	fudgex = abs(validpt1.x - (validpt1.x * 0.85));
	fudgey = abs(validpt1.y - (validpt1.y * 0.85));

	printf("Sanity check: v1 = (%d,%d) fx = %f fy = %f pt = (%d,%d)\n",
	       validpt1.x, validpt1.y, fudgex, fudgey, dpts[2].x, dpts[2].y);
	if (abs(dpts[2].x - validpt1.x) <= fudgex &&
	    abs(dpts[2].y - validpt1.y) <= fudgey) {
	    retval = 0;
	}			/* end of if */
	break;
    case 5:
	/* Need to check the points to make sure they are not the same value, or around the same value */
	deltax = abs(dpts[0].x - dpts[3].x);
	deltay = abs(dpts[0].y - dpts[3].y);
	if (deltax < 100 || deltay < 50)
	    break;

	/* Check point pairs are indices (0,3) and (1,2) */
	validpt1.x = (dpts[0].x + dpts[3].x) / 2;
	validpt1.y = (dpts[0].y + dpts[3].y) / 2;
	validpt2.x = (dpts[1].x + dpts[2].x) / 2;
	validpt2.y = (dpts[1].y + dpts[2].y) / 2;
	fudgex =
	    abs(((validpt1.x + validpt2.x) / 2) -
		((validpt1.x + validpt2.x) / 2) * 0.96);
	fudgey =
	    abs(((validpt1.y + validpt2.y) / 2) -
		((validpt1.y + validpt2.y) / 2) * 0.96);

	printf
	    ("Sanity Check: v1 = (%d,%d), v2 = (%d,%d), fx=%f, fy=%f, pt=(%d,%d).\n",
	     validpt1.x, validpt1.y, validpt2.x, validpt2.y, fudgex, fudgey,
	     dpts[4].x, dpts[4].y);

	/*
	   ** 10/29/01 -- JMW
	   ** Modified to be within range of both midpoint values, not just one, to avoid
	   ** a crappy reading on the first point and still succeed
	 */
	if ((abs(dpts[4].x - validpt1.x) <= fudgex
	     && abs(dpts[4].y - validpt1.y) <= fudgey)
	    && (abs(dpts[4].x - validpt2.x) <= fudgex
		&& abs(dpts[4].y - validpt2.y) <= fudgey)) {
/*				printf("Calibration would have succeeded!!!!\n"); */
	    retval++;
	}
#if 0
	/* Compare the values, as long as they are in line with at least one of the checkpoints... */
	if (abs(dpts[4].x - validpt1.x) <= fudgex
	    && abs(dpts[4].y - validpt1.y) <= fudgey)
	    retval++;
	if (abs(dpts[4].x - validpt2.x) <= fudgex
	    && abs(dpts[4].y - validpt2.y) <= fudgey)
	    retval++;
#endif
	break;
    default:
	break;
    }				/* end of switch */

    printf("returning %d from nxcal_ValidatePt()\n", retval);
    return ((retval >= 0 ? 0 : -1));
}				/* end of nxcal_ValidatePt() */

/*-----------------------------------------------------------------------------*\
**
**	Extern function definitions
**
\*-----------------------------------------------------------------------------*/

/*******************************************************************************\
**
**	Function:	int nxcal()
**	Desc:		Starting point of the calibration (i.e. sets up the GUI, controls
**				interaction with platform specific stuff, etc)
**	Accepts:	int flags = Determines how this function is to operate, where
**					NXCAL_NONINT_MODE = Non interactive mode, needs to init graphics
**										and allow reading of coordinate from file
**				char *calfile = Path to the calibration file to either retreive
**								data from or to store data into.
**	Returns:	int; 0 on success, -1 on error (errno *should* be set)
**
\*******************************************************************************/
int
nxcal(int flags, char *calfile)
{
    int good_cal = 0,		/* Good calibration flag */
      i = 0,			/* Loop iterator */
      npts,			/* Number of points */
      rc,			/* return code */
      retval = 0;		/* Return value */
    CalPt_t *ctrl_ptdata = NULL,	/* Data for the control points */
      draw_pt,			/* Draw point */
     *new_ptdata;		/* Data for the new points (read from device) */
    GR_GC_ID gc;		/* Filled rectangle */

    /*
       ** Determine if the data is stored in a file, and read it/process it
       ** without doing the GUI and exit.
     */
    if ((flags & NXCAL_NONINT_MODE) &&
	!nxcal_getStoredPts(&npts, &ctrl_ptdata, &new_ptdata, calfile)) {
	/* Pixlib call to set the calibration */
	printf("Retrieved data from %s, using stored coordinates!\n",
	       calfile);
	retval = pix_cal_Calibrate(CAL_MAX_CTRL_PTS, ctrl_ptdata, new_ptdata);
	if (retval == PIXLIB_STUB_VAL)
	    retval = 0;
	free(ctrl_ptdata);
	free(new_ptdata);
	if (npts > 0 && retval == 0)
	    return (retval);
    }

    /* end of if */
    /*
       **  Set up window
     */
    if ((retval = nxcal_grInit(flags)) < 0)
	return (retval);

    nxcal_drawText("Touch the center of");
    nxcal_drawText("each cross to calibrate");
    nxcal_drawText("the screen");

    /*
       ** Determine the set of points to process
     */
    if ((rc =
	 pix_cal_GetCtrlPts(&npts, &ctrl_ptdata, si.vs_width, si.vs_height,
			    si.bpp)) == -1) {
	return (-1);
    } /* end of if */
    else if (rc == PIXLIB_STUB_VAL) {
	ctrl_ptdata = (CalPt_t *) calloc((npts = 5), sizeof(CalPt_t));
	if (ctrl_ptdata == NULL)
	    return (-1);
	nxcal_getCtrlPts(npts, ctrl_ptdata);
    }

    /* end of else if */
    /* Allocate memory for the data points */
    if ((new_ptdata = (CalPt_t *) calloc(npts, sizeof(CalPt_t))) == NULL) {
	free(ctrl_ptdata);
	return (-1);
    }

    /* end of if */
    /*
       ** Get individual points based on the control point positions
     */
    while (!good_cal) {
	for (i = 0; i < npts; i++) {
	    /*
	       ** Draw point
	     */
	    if (pix_cal_GetDrawPt(&ctrl_ptdata[i], &draw_pt) == -1) {
		printf("pix_cal_GetDrawPt() failed.\n");
		memcpy(&draw_pt, &ctrl_ptdata[i], sizeof(CalPt_t));
	    }
	    nxcal_drawPt(&draw_pt, 0);

	    /*
	       ** Get point data from touch screen (pixlib call)
	     */
	    if ((rc = pix_cal_GetDataPt(&new_ptdata[i])) == -1) {
		/* There is an error, abandon */
		retval = -1;
		break;
	    } /* end of if */
	    else if (rc == PIXLIB_STUB_VAL) {
		/*
		   ** This function was "stubbed" out, so wait around for
		   ** a mouse up event.
		 */
		int brk_flg = 0;	/* Break flag */
		GR_EVENT event;	/* Event */
		GR_EVENT_MASK new_mask;	/* New mask */
		GR_WINDOW_INFO winf;	/* Window info */

		/* Get the current event mask and add the button up event to it */
		GrGetWindowInfo(cal_win, &winf);
		new_mask = winf.eventmask | GR_EVENT_MASK_BUTTON_UP;
		GrSelectEvents(cal_win, new_mask);

		while (!brk_flg) {
		    GrGetNextEvent(&event);
		    switch (event.type) {
		    case GR_EVENT_TYPE_BUTTON_UP:
			/* Fill this in, just for the heck of it */
			new_ptdata[i].x = event.button.x;
			new_ptdata[i].y = event.button.y;
			brk_flg = 1;
			break;
		    }		/* end of switch */
		}		/* end of while */

		/* Restore the original mask */
		new_mask = winf.eventmask;
		GrSelectEvents(cal_win, new_mask);
	    }			/* end of else if */
	}			/* end of for */

	/*
	   **   Check the last point against pts (0,3) and (1,2) for consistancy)
	 */
	if (nxcal_ValidatePt(npts, new_ptdata) == 0)
	    good_cal = 1;
    }				/* end of for */

    if (i >= npts) {
	/*
	   ** TODO: Handle any rotational differences
	 */
/*		nxcal_normalizePts(npts, ctrl_ptdata, new_ptdata); */

	/*
	   ** Calibrate it (pixlib call) 
	 */
	retval = pix_cal_Calibrate(npts, ctrl_ptdata, new_ptdata);
	if (retval == PIXLIB_STUB_VAL)
	    retval = 0;

	/*
	   ** Store the data into a file
	 */
	nxcal_setStoredPts(CAL_MAX_CTRL_PTS, ctrl_ptdata, new_ptdata,
			   calfile);

	/*
	   ** Close window (and destroy all resources)
	 */

	/* Clear the window to the background color */
	gc = GrNewGC();
	GrSetGCBackground(gc, NXCAL_BACKGROUND);
	GrSetGCForeground(gc, NXCAL_BACKGROUND);
	GrSetGCMode(gc, GR_MODE_SET);
	GrFillRect(cal_win, gc, 1, 1, si.vs_width - 2, si.vs_height - 2);

	GrDestroyWindow(cal_win);

	/* Reset some of the static's back to original value */
	cal_win = (GR_WINDOW_ID) - 1;
	g_text_x = START_TEXT_Y;
	last.x = last.y = -1;

#if 0
	if (flags & NXCAL_NONINT_MODE)
	    GrClose();
#endif
    }

    /* end of if */
    /* Free the dynamically allocated memory */
    if (ctrl_ptdata)
	free(ctrl_ptdata);
    if (new_ptdata)
	free(new_ptdata);

    return (retval);
}				/* end of nxcal */
