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
#include <dirent.h>
#include <stdarg.h>
#include <langinfo.h>
#include <locale.h>

#ifndef __UCLIBC__
#include <monetary.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

// Local header files
#include <FL/Enumerations.H>
#include "nxreg.h"
#include <pixlib/pixlib.h>

#include <sysconf_plugin.h>

// Typedef, macro, enum/struct/union definitions
typedef struct
{
    char *locale_nm, *locale_cd;
}
LocalInfo_t;

static char *tmdt_lbls[REG_NUM_TMDT] = { "Time: ",
    "Short Date: ",
    "Long Date: "
}, *nmbrs_lbls[REG_NUM_NMBRS] =
{
"Positive num: ", "Negative num: ", "Positive cur: ", "Negative cur: "};

// This should be done dynamically, somehow, once the regional packaging gets done....
static const LocalInfo_t locale_info[] = {
    {"Danish", "da_DK"},
    {"Dutch", "nl_NL"},
    {"English (GB)", "en_GB"},
    {"English (US)", "en_US"},
    {"Finnish", "fi_FI"},
    {"French", "fr_FR"},
    {"German", "de_DE"},
    {"Italian", "it_IT"},
    {"Norwegian", "no_NO"},
    {"Portuguese", "pt_PT"},
    {"Spanish", "es_ES"},
    {"Swedish", "sv_SE"},
    {0, 0}
};				// List of all supported locales

NxRegional::~NxRegional()
{
    GetAppPrefs();
    delete _mbreg;

    for (int i = 0; i < REG_NUM_TMDT; i++) {
	delete _regtmdt[i].nb;
	delete[]_regtmdt[i].label;
    }
    for (int i = 0; i < REG_NUM_NMBRS; i++) {
	delete _regnmbrs[i].nb;
	delete[]_regnmbrs[i].label;
    }

    delete _mainw;
}

NxRegional::NxRegional(int X, int Y, int W, int H, char *appname)
{
    _winX = X;
    _winY = Y;

    _nregidx = _oregidx = -1;
    memset(_regtmdt, 0, sizeof(_regtmdt));
    memset(_regnmbrs, 0, sizeof(_regnmbrs));

    // Get the Application preferences from PAR
    GetAppPrefs();

    // Build the window and widgets
    MakeWindow(X, Y, W, H);

    // Set the initial values....
    SetValues();
}

void
NxRegional::ShowWindow(void)
{
    _mainw->show();
}

void
NxRegional::HideWindow(void)
{
    _mainw->hide();
}

#ifdef NOTUSED

void
NxRegional::clean_locale_dir(void)
{
    char sysbuf[1024];
    DIR *pdir;
    struct dirent *dentry;
    struct stat statbuf;

    if ((pdir = opendir(locale_dir)) != NULL) {
	while ((dentry = readdir(pdir)) != NULL) {
	    // Skip over the root and parent directories
	    if (!strcmp(dentry->d_name, ".") || !strcmp(dentry->d_name, ".."))
		continue;

	    // Stat the file
	    sprintf(sysbuf, "%s/%s", locale_dir, dentry->d_name);
	    if (!stat(sysbuf, &statbuf) && S_ISDIR(statbuf.st_mode)) {
		// See if the tar file exists to rebuild
		strcat(sysbuf, ".tar.gz");
		if (stat(sysbuf, &statbuf))
		    continue;
		sprintf(sysbuf, "rm -rf %s/%s", locale_dir, dentry->d_name);
		system(sysbuf);
	    }			// end of if
	}			// end of while
	closedir(pdir);
    }				// end of if

    return;
}				// end of NxRegional::clean_locale_dir(void)
#endif

int
NxRegional::FindLocale(char *key)
{
    int i, key_len, found = -1;

    // Note:        Since the list is alphabetical by long name, rather than locale
    //                      value, and the list is relative short (< 13), a linear search
    //                      is being done.  When this increases a more efficient search
    //                      algorithm will need to be used

    for (i = 0; locale_info[i].locale_nm != NULL; i++) {
	key_len = strlen(key);
	if (!memcmp(key, locale_info[i].locale_cd, key_len)) {
	    found = i;
	    break;
	}			// end of if
    }				// end of for

    return (found);
}				// end of NxRegional::FindLocale(char *)

void
NxRegional::GetAppPrefs(void)
{
    char *pardb,		// name of the default database
      par_data[512];		// Data retrieved from PAR
    db_handle *hdb;		// Database handle
    int rc;			// Index value

    // Setup the database
    if ((pardb = db_getDefaultDB()) == NULL) {
	printf("No default database present!");
	return;
    }				// end of if
    if ((hdb = db_openDB(pardb, PAR_DB_MODE_RDONLY)) == NULL) {
	printf("Error opening %s, error=%d", pardb, pardb_errno);
	return;
    }				// end of if

    // Get the stored Locale value
    if ((rc = par_getGlobalPref(hdb, "language", "default", PAR_TEXT,
				par_data, sizeof(par_data))) > 0) {
	// Null terminate the string
	par_data[rc] = '\0';
	if ((_oregidx = FindLocale(par_data)) > 0)
	    DPRINTF("Current selected region=%s (%s)\n",
		    locale_info[_oregidx].locale_nm,
		    locale_info[_oregidx].locale_cd);
	_nregidx = _oregidx;
    }				// end of if

    // Force a default (good 'ol U.S. style engrish)
    if (_oregidx == -1) {
	sprintf(par_data, "en_US");
	_oregidx = FindLocale(par_data);
    }				// end of if          

    // Set _oregidx and _nregidx initially to be the same
    _nregidx = _oregidx;

    // Close the database and return
    db_closeDB(hdb);
    return;
}				// end of NxRegional::GetAppPrefs(void)

#ifdef NOTUSED

int
NxRegional::is_locale_dir(char *lcd)
{
    char fqpn[1024];		// Fully qualified path name
    int rc = 0;			// Result code
    struct stat statbuf;	// Stat buffer

    if (lcd == NULL || *lcd == '\0')
	return (rc);

    // See if the directory exists
    sprintf(fqpn, "%s/%s", locale_dir, lcd);
    if (stat(fqpn, &statbuf)) {
	// see if the tar file exists
	strcat(fqpn, ".tar.gz");
	if (stat(fqpn, &statbuf))
	    return (rc);
    }
    rc = 1;
    return (rc);
}				// end of NxRegional::is_locale_dir(char *)
#endif

void
NxRegional::MakeWindow(int X, int Y, int W, int H)
{
    int curx,			// Current x coordinate
      cury,			// Current y coordinate
      mar = 4;			// Left margin
    Fl_Color def_bg,		// Default background color
      def_fg;			// Default forground color

    NxApp *instance = sysconf_get_instance();

    _mainw = new Fl_Group(X, Y, W, H);

    def_bg = instance->getGlobalColor(APP_BG);
    def_fg = instance->getGlobalColor(APP_FG);

    _mainw->color(def_bg);
    cury = Y + BUTTON_Y - _winY;
    curx = X + BUTTON_X;

    {
	NxButton *o;

	o = new NxButton(curx, cury, BUTTON_WIDTH, BUTTON_HEIGHT, "Save");

	o->when(FL_WHEN_RELEASE);
	o->callback(save_reset_cb, (void *) this);
	_save = o;
	curx += 63;
    }				// end of "Save" button
    {
	NxButton *o;

	o = new NxButton(curx, cury, BUTTON_WIDTH, BUTTON_HEIGHT, "Reset");

	o->when(FL_WHEN_RELEASE);
	o->callback(save_reset_cb, (void *) this);
	_reset = o;
    }				// end of "Reset button
    {
	float width;
	NxBox *o;
	NxMenuButton *p;

	curx = X + mar;
	cury = Y + mar;

	fl_font(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);
	width = fl_width("Region: ");
	width += 6;
	o = new NxBox(curx, cury, (int) width, BUTTON_HEIGHT, "Region: ");
	o->labelfont(FL_BOLD);
	o->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	curx += mar + o->w();

	p = new NxMenuButton(curx, cury, (_mainw->w() / 3) * 2,
			     BUTTON_HEIGHT);
	p->label("Select Region");
	for (int i = 0; locale_info[i].locale_nm != NULL; i++) {
	    p->add(locale_info[i].locale_nm);
	}			// end of for
	p->when(FL_WHEN_RELEASE);
	p->callback(mb_cb, (void *) this);
	_mbreg = p;
	cury += BUTTON_HEIGHT + mar;
	curx = mar;
    }				// end of menu button
    {
	NxBox *o =
	    new NxBox(curx, cury + 5, _mainw->w() - (2 * mar), BUTTON_HEIGHT);
	o->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	o->label("Appearance Samples");
	cury += BUTTON_HEIGHT;
    }				// end of bogus text
    {
	NxBox *o =
	    new NxBox(curx, cury, _mainw->w() - (2 * mar), BUTTON_HEIGHT);
	o->labeltype(FL_SYMBOL_LABEL);
	o->label("@line");
	o->labelcolor(instance->getGlobalColor(BUTTON_FACE));
	o->align(FL_ALIGN_TOP | FL_ALIGN_INSIDE);
	cury += (int) (1.5 * BUTTON_HEIGHT);
    }				// end of line
    {
	float max_width = 0.0, tmp_width;
	int lblw, lblx, dtaw, dtax;
	NxBox *lbls[REG_NUM_TMDT],	// Array of sample labels
	 *dta[REG_NUM_NMBRS];	// Array of sample data

	for (int i = 0; i < REG_NUM_TMDT; i++) {
	    if ((tmp_width = fl_width(tmdt_lbls[i])) > max_width)
		max_width = tmp_width;
	}			// end of for

	lblx = mar;
	lblw = (int) max_width;
	dtax = lblx + lblw + mar;
	dtaw = _mainw->w() - (dtax + mar);

	for (int i = 0; i < REG_NUM_TMDT; cury += BUTTON_HEIGHT, i++) {
	    lbls[i] =
		new NxBox(lblx, cury, lblw, BUTTON_HEIGHT, tmdt_lbls[i]);
	    lbls[i]->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

	    dta[i] =
		new NxBox(dtax, cury, dtaw, BUTTON_HEIGHT, "Sample Data");
	    dta[i]->box(FL_FLAT_BOX);
	    dta[i]->color(def_bg);
	    dta[i]->labelcolor(def_fg);
	    dta[i]->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	    _regtmdt[i].nb = dta[i];
	}			// end of for 
	cury += (int) (0.5 * BUTTON_HEIGHT);
    }
    {
	float max_width = 0.0, tmp_width;
	int lblw, lblx, dtaw, dtax;
	NxBox *lbls[REG_NUM_NMBRS], *dta[REG_NUM_NMBRS];

	for (int i = 0; i < REG_NUM_NMBRS; i++) {
	    if ((tmp_width = fl_width(nmbrs_lbls[i])) > max_width)
		max_width = tmp_width;
	}			// end of for

	lblx = mar;
	lblw = (int) max_width;
	dtax = lblx + lblw + mar;
	dtaw = _mainw->w() - (dtax + mar);

	for (int i = 0; i < REG_NUM_NMBRS; cury += BUTTON_HEIGHT, i++) {
	    lbls[i] =
		new NxBox(lblx, cury, lblw, BUTTON_HEIGHT, nmbrs_lbls[i]);
	    lbls[i]->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

	    dta[i] =
		new NxBox(dtax, cury, dtaw, BUTTON_HEIGHT, "Sample Data");
	    dta[i]->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	    dta[i]->box(FL_FLAT_BOX);
	    dta[i]->color(def_bg);
	    dta[i]->labelcolor(def_fg);
	    _regnmbrs[i].nb = dta[i];
	}			// end of for 
    }
    _mainw->end();
    _mainw->hide();

    return;
}				// end of NxRegional::MakeWindow(void)

/*******************************************************************************\
**
**	Function:	void SetAppPrefs()
**	Desc:		Stores any changed values into the PAR database
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**	
\*******************************************************************************/
void
NxRegional::SetAppPrefs(void)
{
    char *pardb;		// Database name
    db_handle *hdb;		// Database handle

    if ((pardb = db_getDefaultDB()) == NULL) {
	printf("No Default database! Changes lost");
	return;
    }				// end of if
    if ((hdb = db_openDB(pardb, PAR_DB_MODE_RW)) == NULL) {
	printf("Unable to open %s, error=%d", pardb, pardb_errno);
	return;
    }				// end of if

    // Write out changes
    if ((_nregidx != _oregidx) &&
	par_setGlobalPref(hdb, "language", "default", PAR_TEXT,
			  locale_info[_nregidx].locale_cd,
			  strlen(locale_info[_nregidx].locale_cd) + 1) <= 0) {
	printf("Unable to store node global.language.default="
	       "%s, error=%d", locale_info[_nregidx].locale_cd, pardb_errno);
    }				// end of if 

    // TODO: Make the collosseum call...
    char col_msg[CL_MAX_MSG_LEN];
    int col_len = sprintf(col_msg, "sc_regional^SYSCON_REG_CHANGE");
    sysconf_ipc_write(CL_MSG_BROADCAST_ID, col_msg, col_len);

    // Close the database
    db_closeDB(hdb);
    return;
}				// end of NxRegional::SetAppPrefs(void)

/*******************************************************************************\
**
**	Function:	void SetNmbrs()
**	Desc:		Sets the _regnmbrs[idx] member up correctly and sets the widget
**				for a redraw status
**	Accepts:	int idx = Index into _regnmbrs to process
**				char *value = label value
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxRegional::SetNmbrs(int idx, char *value)
{
    // Validate the incoming parameters
    if (value == NULL || *value == '\0')
	return;
    if (idx < 0 || idx >= REG_NUM_NMBRS)
	return;

    delete[]_regnmbrs[idx].label;
    _regnmbrs[idx].label = new char[strlen(value) + 1];
    strcpy(_regnmbrs[idx].label, value);
    _regnmbrs[idx].nb->label(_regnmbrs[idx].label);
    _regnmbrs[idx].nb->damage(FL_DAMAGE_ALL);
    _regnmbrs[idx].nb->redraw();

    return;
}				// end of NxRegional::SetNmbrs(int, char *)

/*******************************************************************************\
**
**	Function:	void SetTmdt()
**	Desc:		Sets the _regtmdt[idx] member up correctly and sets the widget
**				for a redraw status
**	Accepts:	int idx = Index into _regtmdt to process
**				char *value = label value
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxRegional::SetTmdt(int idx, char *value)
{
    // Validate the incoming parameters
    if (value == NULL || *value == '\0')
	return;
    if (idx < 0 || idx >= REG_NUM_TMDT)
	return;

    // delete[] performs the "if (ptr)" test....
    delete[]_regtmdt[idx].label;
    _regtmdt[idx].label = new char[strlen(value) + 1];
    strcpy(_regtmdt[idx].label, value);
    _regtmdt[idx].nb->label(_regtmdt[idx].label);
    _regtmdt[idx].nb->damage(FL_DAMAGE_ALL);
    _regtmdt[idx].nb->redraw();

    return;
}				// end of NxRegional::SetTmdt(int, char *)

/*******************************************************************************\
**
**	Function:	void SetValues()
**	Desc:		Sets the widgets values based upon the current state of the 
**				application (_mode)
**	Accpets:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxRegional::SetValues(void)
{
    char *cp1, *olocale, tmp_data[128];
    float conv_val = 123456789.0;


    // TODO: Set the curret locale to what was selected and redisplay values
    if (_nregidx == -1)
	return;

    // Set the locale and get the formatting info
//      set_locale_dir();
    if ((olocale =
	 setlocale(LC_ALL, locale_info[_nregidx].locale_cd)) == NULL) {
	printf("setlocale() failed for %s\n",
	       locale_info[_nregidx].locale_cd);
//              return;
    }

    _mbreg->label(locale_info[_nregidx].locale_nm);
    _mbreg->damage(FL_DAMAGE_ALL);
    _mbreg->redraw();

    // Convert the date/time values
    time_t now = time(NULL);
    struct tm *ptm = localtime(&now);

    strftime(tmp_data, sizeof(tmp_data), nl_langinfo(T_FMT), ptm);
    SetTmdt(REG_TMDT_TIME, tmp_data);
    strftime(tmp_data, sizeof(tmp_data), nl_langinfo(D_FMT), ptm);
    SetTmdt(REG_TMDT_SDT, tmp_data);
    strftime(tmp_data, sizeof(tmp_data), "%c", ptm);
    if ((cp1 = strchr(tmp_data, ':')) != NULL
	|| (cp1 = strchr(tmp_data, '.')) != NULL) {
	while (cp1 >= tmp_data && *cp1 != ' ')
	    cp1--;
	*cp1 = '\0';
	cp1++;
    }				// end of if 
    SetTmdt(REG_TMDT_LDT, tmp_data);

#ifndef __UCLIBC__
    // Convert the numeric values
    strfmon(tmp_data, sizeof(tmp_data), "%+!n", conv_val);
    SetNmbrs(REG_NMBRS_PN, tmp_data);
    strfmon(tmp_data, sizeof(tmp_data), "%+!n", -conv_val);
    SetNmbrs(REG_NMBRS_NN, tmp_data);

    // Format the currency values
    strfmon(tmp_data, sizeof(tmp_data), "%n", conv_val);
    SetNmbrs(REG_NMBRS_PC, tmp_data);
    strfmon(tmp_data, sizeof(tmp_data), "%n", -conv_val);
    SetNmbrs(REG_NMBRS_NC, tmp_data);
#endif

    return;
}				// end of NxRegional::SetValues(void)

#if 0
/*******************************************************************************\
**
**	Function:	void set_locale_dir()
**	Desc:		Makes sure that the locale information is available (untarred), 
**				that corresponds to the locale name at idx _nregidx.
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxRegional::set_locale_dir(void)
{
    char sysbuf[1024];		// Buffer for system()
    pid_t chld_pid;
    struct stat statbuf;	// Stat buffer

    sprintf(sysbuf, "%s/%s", locale_dir, locale_info[_nregidx].locale_cd);
    if (stat(sysbuf, &statbuf)) {
	strcat(sysbuf, ".tar.gz");
	if (stat(sysbuf, &statbuf))
	    return;
    }				// end of if

    sprintf(sysbuf, "cd %s; tar -zxf %s.tar.gz", locale_dir,
	    locale_info[_nregidx].locale_cd);
    if ((chld_pid = fork()) == 0) {
	char *argv[4];
	argv[0] = "sh";
	argv[1] = "-c";
	argv[2] = sysbuf;
	argv[3] = 0;
	execv("/bin/sh", argv);
	_exit(127);
    }				// end of if 
    else if (chld_pid > 0) {
	// Wait around for the child to die
	int csts;
	waitpid(chld_pid, &csts, 0);
    }				// end of if

//      system(sysbuf);
}				// end of NxRegional::set_locale_dir(void)
#endif
//-------------------------------------------------------------------------------
//
//      Private static callback methods
//              void mb_cb(Fl_Widget *, void *)
//              void save_reset_cb(Fl_Widget *, void *)
//
//-------------------------------------------------------------------------------

/*******************************************************************************\
**
**	Function:	void mb_cb()
**	Desc:		Callback for the regional selection menu button
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxRegional::mb_cb(Fl_Widget * w, void *d)
{
    NxRegional *me = (NxRegional *) d;
    NxMenuButton *nxmb = (NxMenuButton *) w;
    int idx, saveidx = me->_nregidx;

    idx = nxmb->value();
    if (idx == me->_nregidx)
	return;

    me->_nregidx = idx;

    // Set the values
    if (me->_nregidx != saveidx) {
	DPRINTF("Loading new values!\n");
	me->SetValues();
    }				// end of if

    return;
}				// end of NxRegional::mb_cb(Fl_Widget *, void *)

/*******************************************************************************\
**
**	Function:	void save_reset_cb()
**	Desc:		Handles the save/reset of this application/utility
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxRegional::save_reset_cb(Fl_Widget * w, void *d)
{
    NxRegional *me = (NxRegional *) d;
    NxButton *nxb = (NxButton *) w;

    if (!strcmp(nxb->label(), "Save") && me->_nregidx != me->_oregidx) {
	DPRINTF("Saving any changed data!\n");
	me->SetAppPrefs();
    }				// end of if
    else if (!strcmp(nxb->label(), "Reset")) {
	DPRINTF("Reseting data!\n");
	me->_nregidx = me->_oregidx;
	me->SetValues();
    }				// end of else-if

}				// end of NxRegional::save_exit_cb(Fl_Widget *, void *)
