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

// System header files
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Local header files
#include <FL/Enumerations.H>
#include "nxabout.h"
#include <pixlib/pixlib.h>

// Typedef, macro, enum/struct/union definitions
#define			WINDOW_MAR			4

static const char *label_tags[ABOUT_DUMMY] = {
    "Operating\nSystem: ",
    "Operating\nEnvironment: ",
    "Processor: ",
    "Memory: ",
    "PCMCIA: ",
    "Owner: "
};
static char *data_tags[ABOUT_DUMMY];
static int wig_hght[ABOUT_DUMMY] = { 2,	// OS takes 1 line of data
    2,				// OE takes 3 lines (name + century)
    1,				// Processor takes 1 line
    1,				// Memory takes 1 line
    1,				// PCMCIA Takes 2 line
    1				// Owner takes 1 line
}, lbl_width = 0,		// Label width
  data_width = 0;		// Data width

NxAbout::~NxAbout()
{
    delete _mainw;
}

NxAbout::NxAbout(int X, int Y, int W, int H, char *appname)
{
    _mainw = 0;

    _winX = X;
    _winY = Y;

    SetWidths();
    GetPluginData();
    MakeWindow(X, Y, W, H);
    SetValues();
}

void
NxAbout::GetPluginData(void)
{
    char *pardb,		// name of the default database
      par_data[512] = { '\0' };	// Values from the database
    int rc;
    db_handle *hdb;		// Database handle
    struct utsname unm;		// OS info
    pixCpuInfo_t cpuinfo;	// Cpu info
    pixMemInfo_t meminfo;	// Memory info
    pixPCMCIAInfo_t cardinfo;	// Card info

    // Setup the database
    if ((pardb = db_getDefaultDB()) == NULL)
	return;

    if ((hdb = db_openDB(pardb, PAR_DB_MODE_RDONLY)) == NULL)
	return;

    /* Get the info about the system */

    if (pix_sys_osinfo(&unm) == 0)
	sprintf(par_data, "%s %s", unm.sysname, unm.release);
    else
	sprintf(par_data, "Not Available");

    FormatString(par_data, data_width, ABOUT_OSINFO);

    // Get the memory info
    if (pix_sys_meminfo(&meminfo) == 0) {
	char *mem_lbls[4] = { "Bytes", "KB", "MB", "GB" };
	int lbl_flags = 0;
	// Convert the byte values into MB
	while (meminfo.mtotal > 1000) {
	    if (lbl_flags == 0)
		meminfo.mtotal /= 1024;
	    else
		meminfo.mtotal /= 1000;
	    lbl_flags++;
	}
	sprintf(par_data, "%.2f %s", meminfo.mtotal, mem_lbls[lbl_flags]);
    }				// end of if
    else {
	sprintf(par_data, "Not Available");
    }				// end of else
    FormatString(par_data, data_width, ABOUT_MEMINFO);

    // Get the cpuinfo      
    if (pix_sys_cpuinfo(&cpuinfo) != 0) {
	sprintf(cpuinfo.cpu, "Not Available");
    }				// end of if
    FormatString(cpuinfo.cpu, data_width, ABOUT_CPUINFO);

    // Get the PCMCIA stuff
    if (pix_sys_pcmciainfo(&cardinfo) == 0)
	sprintf(par_data, "%s\n%s", cardinfo.socket0, cardinfo.socket1);
    else
	sprintf(par_data, "Not Available");
    FormatString(par_data, data_width, ABOUT_PCMCIA);

    // Get the OE stuff
    sprintf(par_data,
	    "PIXIL Operating Environment.\n(c) 2003, Century Software, Inc.\n"
	    "All Rights Reserved.");

    FormatString(par_data, data_width, ABOUT_OEINFO);

    // Get the User Information from par
    if ((rc =
	 par_getGlobalPref(hdb, "UserId", "Name", PAR_TEXT, par_data,
			   sizeof(par_data))) > 0)
	par_data[rc] = '\0';
    else
	sprintf(par_data, "Not Available");

    FormatString(par_data, data_width, ABOUT_USERINFO);

    // Close the database and return
    db_closeDB(hdb);
    return;
}				// end of NxAbout::GetAppData(void)

/*******************************************************************************\
**
**	Function:	void FormatString()
**	Desc:		Formats the string to fit within a certain width (by adding
**				newline characters.
**	Accepts:	char *str = Ptr to the string to format
**				int w = width of the space
**				int idx = Index into the global arrays
**	Returns:	Nothing (void)
**	Note:		This operates on the global variables data_tags[idx] and 
**				wig_hght[idx]
**
\*******************************************************************************/
void
NxAbout::FormatString(char *str, int w, int idx)
{
    char *lnbuffer, *cp,	// Current pointer
     *sol,			// Start of line
     *sow,			// Start of word
      svc;			// Saved char
    int bufsz = 0,		// Size of the buffer
      bufvol = 0,		// Current volume of the line
      lncnt = 0,		// Number of lines 
      lnlen = 0,		// Length of line in # of chars
      solflg = 0,		// Indicates a start of a line
      sowflg = 0;		// Indicates a start of a word
    float strwid;		// Current width of line

    if (str == NULL || w == 0)
	return;

    fl_font(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);

    // Allocate memory the same size as the str
    bufsz = strlen(str) + 1;
    if ((lnbuffer = (char *) malloc(bufsz * sizeof(char))) == NULL)
	return;

    sol = sow = str;
    for (cp = str; *cp; cp++) {
	lnlen++;
	if (solflg)
	    sol = cp;
	if (sowflg)
	    sow = cp;
	if (*cp == '\n') {
	    // maintain the newline
	    if (lnlen + bufvol >= bufsz) {
		char *tmp;
		tmp = (char *) realloc(lnbuffer, bufvol + lnlen + 1);
		if (!tmp)
		    break;
		lnbuffer = tmp;
		bufsz = bufvol + lnlen + 1;
	    }			// end of if */
	    memcpy(lnbuffer + bufvol, sol, lnlen);
	    bufvol += lnlen;
	    solflg = sowflg = 1;
	    lncnt++;
	    lnlen = 0;
	    continue;
	}			// end of if */
	if (*cp == ' ' || *cp == '\t')
	    sowflg = 1;

	// Determine the current width of the line
	svc = *(cp + 1);
	*(cp + 1) = '\0';
	strwid = fl_width(sol);
	*(cp + 1) = svc;
	if (strwid > w) {
	    if (sowflg) {
		// Convert the whitespace into a newline and let the newline case handle it
		*cp = '\n';
		cp--;
		continue;
	    }			// end of if
	    else {
		// In the middle of a word
		if (bufvol + (lnlen - (cp - sow) - 1) > bufsz) {
		    char *tmp;
		    tmp = (char *) realloc(lnbuffer, bufvol + lnlen + 1);
		    if (!tmp)
			break;
		    lnbuffer = tmp;
		    bufsz = bufvol + (lnlen - (cp - sow) + 1);
		}
		memcpy(lnbuffer + bufvol, sol, lnlen - (cp - sow) - 1);
		bufvol += lnlen - (cp - sow) + 1;
		lnbuffer[bufvol - 2] = '\n';
		lnbuffer[bufvol - 1] = '\t';

		sol = sow;
		lnlen = cp - sow + 1;
		lncnt++;
		continue;
	    }			// end of else
	}			// end of if 
	if (solflg && !isspace(*cp)) {
	    solflg = 0;
	}			// end of if 
	if (sowflg && !isspace(*cp)) {
	    sowflg = 0;
	}			// end of if 
    }				// end of for
    // Transfer the trailing line
    if (bufvol + lnlen > bufsz) {
	char *tmp;
	tmp = (char *) realloc(lnbuffer, bufvol + lnlen + 1);
	if (tmp) {
	    lnbuffer = tmp;
	    bufsz = bufvol + lnlen + 1;
	    memcpy(lnbuffer + bufvol, sol, lnlen);
	    lncnt++;
	}			/* end of if */
    }				// end of if
    else {
	memcpy(lnbuffer + bufvol, sol, lnlen);
	lncnt++;
    }				// end of else 
    lnbuffer[bufvol + lnlen] = '\0';

    if (data_tags[idx])
	delete[]data_tags[idx];
    data_tags[idx] = new char[strlen(lnbuffer) + 1];
    strcpy(data_tags[idx], lnbuffer);
    if (wig_hght[idx] < lncnt) {
	wig_hght[idx] = lncnt;
    }

    free(lnbuffer);
    return;
}				// end of NxAbout::FormatString(char *, int, int)

/*******************************************************************************\
**
**	Function:	void MakeWindow()
**	Desc:		Creates the main fltk window and adds the appropriate widgets to
**				it
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxAbout::MakeWindow(int X, int Y, int W, int H)
{
    if (_mainw)
	return;

    _mainw = new Fl_Group(X, Y, W, H);

    //_mainw->color(NxApp::Instance()->getGlobalColor(APP_BG));

    int cury, cur_lblx, cur_dtax, i, intr_ln_sp;
    NxBox *dta[ABOUT_DUMMY], *lbl[ABOUT_DUMMY];

    // Get the starting y value
    cur_lblx = WINDOW_MAR;
    cur_dtax = ((int) lbl_width + (2 * WINDOW_MAR));
    for (cury = i = 0; i < ABOUT_DUMMY; i++) {
	cury += wig_hght[i];
    }

    intr_ln_sp = ((Y + H) -
		  ((2 * WINDOW_MAR) +
		   (cury * BUTTON_HEIGHT))) / (ABOUT_DUMMY + 1);

    cury = WINDOW_MAR + intr_ln_sp;

    if (cury < Y)
	cury = Y;

    // Build the widgets
    for (i = 0; i < ABOUT_DUMMY; i++) {
	// Label widget
	lbl[i] = new NxBox(cur_lblx, cury, lbl_width, BUTTON_HEIGHT);
	lbl[i]->label(label_tags[i]);
	lbl[i]->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	// Data widgets

	dta[i] =
	    new NxBox(cur_dtax, cury, data_width,
		      wig_hght[i] * BUTTON_HEIGHT);
	dta[i]->label("Not available.");
	dta[i]->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	_info_wid[i] = dta[i];

	cury += intr_ln_sp + dta[i]->h();
    }

    _mainw->end();

    _mainw->hide();
}

void
NxAbout::ShowWindow(void)
{
    _mainw->show();
}

void
NxAbout::HideWindow(void)
{
    _mainw->hide();
}

/*******************************************************************************\
**
**	Function:	void SetWidths()
**	Desc:		Calculates the widths of the label/data columns for this
**				utility
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxAbout::SetWidths(void)
{
    float max_lbl_width = 0;

    fl_font(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);
    for (int i = 0; i < ABOUT_DUMMY; i++) {
	char *cp;
	float dbltmp;

	if ((cp = strchr(label_tags[i], '\n')) != NULL) {
	    char *tmpbuf;

	    // Get the width of each line
	    tmpbuf = strdup(label_tags[i]);
	    cp = tmpbuf + (cp - label_tags[i]);
	    *cp = '\0';
	    cp++;

	    if ((dbltmp = fl_width(tmpbuf)) > max_lbl_width)
		max_lbl_width = dbltmp;
	    if ((dbltmp = fl_width(cp)) > max_lbl_width)
		max_lbl_width = dbltmp;
	    free(tmpbuf);
	} /* end of if */
	else {
	    if ((dbltmp = fl_width(label_tags[i])) > max_lbl_width)
		max_lbl_width = dbltmp;
	}			/* end of else */
    }				// end of for 


    lbl_width = (int) max_lbl_width;
    data_width = ((int) (W_W - _winX) - ((3 * WINDOW_MAR) + lbl_width));
}				// end of NxAbout::SetWidths(void)

/*******************************************************************************\
**
**	Function:	void SetValue()
**	Desc:		Sets the values in the widgets
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxAbout::SetValues(void)
{
    for (int i = 0; i < ABOUT_DUMMY; i++) {
	_info_wid[i]->label(data_tags[i]);
	_info_wid[i]->damage(FL_DAMAGE_ALL);
	_info_wid[i]->redraw();
    }				// end of for 
}				// end of NxAbout::SetValues(void)
