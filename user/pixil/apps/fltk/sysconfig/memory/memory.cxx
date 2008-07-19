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
#include <sys/vfs.h>
#include <unistd.h>

// Local header files
#include <FL/x.H>
#include <FL/Enumerations.H>
#include <nxbox.h>
#include "memory.h"
#include <pixlib/pixlib.h>

#include <sysconf_plugin.h>

// Typedef, macro, enum/struct/union definitions
#define			SZ_TAGS		4

// Global scope variables

char *size_tags[SZ_TAGS] = { "Bytes", "KB",
    "MB", "GB"
};				// Tags for size labels
static char *f_curDev;		// Ptr to the current device value


// Static function prototypes (non class)
static float getsztags(int byteval, int *idx);

NxMemory::~NxMemory()
{
    // Delete the widgets
    delete _memsl;
    delete _storsl;
    delete _stormb;

    // Delete the output widgets
    delete _memory.used;
    delete _memory.free;
    delete _memory.total;
    delete[]_memory.u_str;
    delete[]_memory.f_str;
    delete[]_memory.t_str;
    delete _storage.used;
    delete _storage.free;
    delete _storage.total;
    delete[]_storage.u_str;
    delete[]_storage.f_str;
    delete[]_storage.t_str;

    delete _mainw;
}				// end of NxMemory::~NxMemory()

NxMemory::NxMemory(int X, int Y, int W, int H, char *appname)
{
    // Set up default values....
    _winX = X;
    _winY = Y;

    _memory.u_str = _memory.f_str = _memory.t_str = NULL;
    _storage.u_str = _storage.f_str = _storage.t_str = NULL;

    // Build the window and widgets
    MakeWindow(X, Y, W, H);

    // Set the initial values....
    GetValues(MEM_GV_UPDMNT | MEM_GV_UPDMEM | MEM_GV_UPDSTO);
}

void
NxMemory::ShowWindow(void)
{
    Fl::add_timeout(2.0, proc_tmr, (void *) this);
    _mainw->show();
}

void
NxMemory::HideWindow(void)
{
    Fl::remove_timeout(proc_tmr, (void *) this);
    _mainw->hide();
}

void
NxMemory::MakeWindow(int X, int Y, int W, int H)
{
    int col_width,		// Column width
      curx,			// Current x coordinate
      cury,			// Current y coordinate
      mar = 4;			// Left margin
    float fontw0,		// Width of "0%" in current font
      fontw100;			// Width of "100%" in current font

    Fl_Color def_bg,		// Default background
      def_fg,			// Default foreground
      def_sel;			// Default selection color

    NxApp *instance = sysconf_get_instance();

    // Get the back/fore ground colors
    def_bg = instance->getGlobalColor(APP_BG);
    def_fg = instance->getGlobalColor(APP_FG);
    def_sel = instance->getGlobalColor(APP_SEL);

    // Get the default font width's
    fl_font(DEFAULT_TEXT_FONT, DEFAULT_TEXT_SIZE);
    fontw0 = fl_width("0%");
    fontw100 = fl_width("100%");

    _mainw = new Fl_Group(X, Y, W, H);
    _mainw->color(instance->getGlobalColor(APP_BG));

    curx = _winX + mar;
    cury = _winY + mar;
    col_width = (_mainw->w() - (2 * mar)) / 2;
    {
	// Memory section....
	NxBox *o =
	    new NxBox(curx, cury, _mainw->w() - (2 * mar), BUTTON_HEIGHT);

	o->color(def_bg);
	o->labelcolor(def_fg);
	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	o->label("Memory: ");
	cury += BUTTON_HEIGHT;
    }
    {
	NxBox *min, *max;
	NxSlider *o;

	o = _memsl = new NxSlider((int) (curx + fontw0 + 2), cury,
				  _mainw->w() - (int) ((2 * mar) + fontw0 +
						       fontw100 + 2),
				  BUTTON_HEIGHT);
	min = new NxBox(curx, cury, (int) fontw0, BUTTON_HEIGHT);
	max =
	    new NxBox((int) (_mainw->w() - (mar + fontw100)), cury,
		      (int) (fontw100), BUTTON_HEIGHT);

	min->color(def_bg);
	min->labelcolor(def_fg);

	min->box(FL_FLAT_BOX);
	min->label("0%");
	max->color(def_bg);
	max->labelcolor(def_fg);
	max->box(FL_FLAT_BOX);
	max->label("100%");
	o->box(FL_BORDER_BOX);
	o->minimum(0.0);
	o->maximum(100.0);
	o->step(1.0);
	o->type(FL_HOR_FILL_SLIDER);
	o->deactivate();
	o->value(50.0);
	cury += 2 * BUTTON_HEIGHT;
    }				// end of slider
    {
	NxBox *lbl1,		// Label
	 *lbl2, *lbl3, *o,	// Output text
	 *p, *q;

	lbl1 = new NxBox(curx, cury, col_width, BUTTON_HEIGHT);
	o = new NxBox(curx + col_width + mar, cury, col_width - mar,
		      BUTTON_HEIGHT);
	lbl2 =
	    new NxBox(curx, cury + BUTTON_HEIGHT, col_width, BUTTON_HEIGHT);
	p = new NxBox(curx + col_width + mar, cury + BUTTON_HEIGHT,
		      col_width - mar, BUTTON_HEIGHT);
	lbl3 =
	    new NxBox(curx, cury + (2 * BUTTON_HEIGHT), col_width,
		      BUTTON_HEIGHT);
	q = new NxBox(curx + col_width + mar, cury + (2 * BUTTON_HEIGHT),
		      col_width - mar, BUTTON_HEIGHT);

	// Set up the labels
	lbl1->color(def_bg);
	lbl1->labelcolor(def_fg);
	lbl1->box(FL_FLAT_BOX);
	lbl1->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
	lbl1->label("In Use: ");
	lbl2->color(def_bg);
	lbl2->labelcolor(def_fg);
	lbl2->box(FL_FLAT_BOX);
	lbl2->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
	lbl2->label("Free: ");
	lbl3->color(def_bg);
	lbl3->labelcolor(def_fg);
	lbl3->box(FL_FLAT_BOX);
	lbl3->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
	lbl3->label("Total: ");

	// Set up the output
	o->color(def_bg);
	o->labelcolor(def_fg);
	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	p->color(def_bg);
	p->labelcolor(def_fg);
	p->box(FL_FLAT_BOX);
	p->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	q->color(def_bg);
	q->labelcolor(def_fg);
	q->box(FL_FLAT_BOX);
	q->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

	_memory.used = o;
	_memory.free = p;
	_memory.total = q;

	cury += (4 * BUTTON_HEIGHT);
    }				// end of labels
    {
	// Storage section
	NxBox *o;		// Label for memory

	// TODO: Probably will need to adjust width to allow for the menu button
	o = new NxBox(curx, cury, (_mainw->w() - (2 * mar)), BUTTON_HEIGHT);

	instance->def_font(o);
	o->box(FL_FLAT_BOX);
	o->color(def_bg);
	o->labelcolor(def_fg);
	o->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	o->label("Storage on:");
    }				// end of storage label
    {
	// Storage menu button options
	NxMenuButton *o;	// Menu button

	o = new NxMenuButton(col_width + mar, cury, col_width - (2 * mar),
			     BUTTON_HEIGHT);

	instance->def_font(o);
	o->label("Device List");
	o->callback(stor_mb_cb, (void *) this);
	o->menu((NxMenuItem *) NULL);
	o->value(0);
	_stormb = o;
	cury += BUTTON_HEIGHT + mar;
    }				// end of storage menu button
    {
	NxBox *min, *max;
	NxSlider *o;

	o = _storsl = new NxSlider((int) (curx + fontw0 + 2), cury,
				   _mainw->w() -
				   (int) (((2 * mar) + fontw0 + fontw100 +
					   2)), BUTTON_HEIGHT);
	min = new NxBox(curx, cury, (int) fontw0, BUTTON_HEIGHT);
	max =
	    new NxBox((int) (_mainw->w() - (mar + fontw100)), cury,
		      (int) fontw100, BUTTON_HEIGHT);

	min->color(def_bg);
	min->labelcolor(def_fg);
	min->box(FL_FLAT_BOX);
	min->label("0%");
	max->color(def_bg);
	max->labelcolor(def_fg);
	max->box(FL_FLAT_BOX);
	max->label("100%");
	o->box(FL_BORDER_BOX);
	o->minimum(0.0);
	o->maximum(100.0);
	o->step(1.0);
	o->type(FL_HOR_FILL_SLIDER);
	o->deactivate();
	o->value(50.0);
	cury += 2 * BUTTON_HEIGHT;
    }				// end of slider
    {
	NxBox *lbl1,		// Label
	 *lbl2, *lbl3, *o,	// Output text
	 *p, *q;

	lbl1 = new NxBox(curx, cury, col_width, BUTTON_HEIGHT);
	o = new NxBox(curx + col_width + mar, cury, col_width, BUTTON_HEIGHT);
	lbl2 =
	    new NxBox(curx, cury + BUTTON_HEIGHT, col_width, BUTTON_HEIGHT);
	p = new NxBox(curx + col_width + mar, cury + BUTTON_HEIGHT, col_width,
		      BUTTON_HEIGHT);
	lbl3 =
	    new NxBox(curx, cury + (2 * BUTTON_HEIGHT), col_width,
		      BUTTON_HEIGHT);
	q = new NxBox(curx + col_width + mar, cury + (2 * BUTTON_HEIGHT),
		      col_width, BUTTON_HEIGHT);

	// Set up the labels
	lbl1->color(def_bg);
	lbl1->labelcolor(def_fg);
	lbl1->box(FL_FLAT_BOX);
	lbl1->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
	lbl1->label("In Use: ");
	lbl2->color(def_bg);
	lbl2->labelcolor(def_fg);
	lbl2->box(FL_FLAT_BOX);
	lbl2->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
	lbl2->label("Free: ");
	lbl3->color(def_bg);
	lbl3->labelcolor(def_fg);
	lbl3->box(FL_FLAT_BOX);
	lbl3->align(FL_ALIGN_INSIDE | FL_ALIGN_RIGHT);
	lbl3->label("Total: ");

	// Set up the output
	o->color(def_bg);
	o->labelcolor(def_fg);
	o->box(FL_FLAT_BOX);
	o->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	o->color(def_bg);
	o->labelcolor(def_fg);
	p->color(def_bg);
	p->labelcolor(def_fg);
	p->box(FL_FLAT_BOX);
	p->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	q->color(def_bg);
	q->labelcolor(def_fg);
	q->box(FL_FLAT_BOX);
	q->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
	_storage.used = o;
	_storage.free = p;
	_storage.total = q;

	cury += (3 * BUTTON_HEIGHT);
    }				// end of labels
    _mainw->end();
    _mainw->hide();
    return;
}				// end of NxMemory::MakeWindow(void)

/*******************************************************************************\
**
**	Function:	void GetMemory()
**	Desc:		Gets the memory values from /proc/meminfo and stores them
**				into the widgets for display
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxMemory::GetMemory(void)
{
    char buf[255];		// Buffer
    int tag_idx = 0;		// Tag index
    float val;			// Value
    unsigned long mbuf = 0,	// Memory buffered
      mcache = 0,		// Memory cached
      mfree = 0,		// Memory free
      mtot = 0,			// Memory total
      mused = 0,		// Memory used
      sfree = 0,		// Swap free
      stot = 0,			// Swap total
      sused = 0;		// Swap used
    FILE *memf;			// Memory file ptr

    if ((memf = fopen("/proc/meminfo", "r")) != NULL) {
	// Discard the first line
	fgets(buf, sizeof(buf), memf);
	fscanf(memf, "%*s %ld %ld %ld %*d %ld %ld", &mtot, &mused, &mfree,
	       &mbuf, &mcache);
	fscanf(memf, "%*s %ld %ld %ld", &stot, &sused, &sfree);

	mused = sused + mused - mcache - mbuf;
	mfree = mtot - mused;

	fclose(memf);
    }				// end of if

    // Get the In use value
    tag_idx = 0;
    val = getsztags(mused, &tag_idx);
    sprintf(buf, "%9.2f %s", val, size_tags[tag_idx]);
    delete[]_memory.u_str;
    _memory.u_str = new char[strlen(buf) + 1];
    strcpy(_memory.u_str, buf);
    _memory.used->label(_memory.u_str);
    _memory.used->redraw();

    tag_idx = 0;
    val = getsztags(mfree, &tag_idx);
    sprintf(buf, "%9.2f %s", val, size_tags[tag_idx]);
    delete[]_memory.f_str;
    _memory.f_str = new char[strlen(buf) + 1];
    strcpy(_memory.f_str, buf);
    _memory.free->label(_memory.f_str);
    _memory.free->redraw();

    tag_idx = 0;
    val = getsztags(mtot, &tag_idx);
    sprintf(buf, "%9.2f %s", val, size_tags[tag_idx]);
    delete[]_memory.t_str;
    _memory.t_str = new char[strlen(buf) + 1];
    strcpy(_memory.t_str, buf);
    _memory.total->label(_memory.t_str);
    _memory.total->redraw();

    val = ((float) mused / mtot) * 100.0;
    _memsl->value(val);
    _memsl->redraw();

    return;
}				// end of NxMemory::GetMemory(void)

/*******************************************************************************\
**
**	Function:	void GetStorage()
**	Desc:		Determines the storage capacity and what is in use for the current
**				storage medium (as determined by f_curDev)
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxMemory::GetStorage(void)
{
    char buf[64];
    float pcnt_used,		// Percentage used
      sadjtot = 0.0, sfree = 0.0, stot = 0.0, sused = 0.0, val;	// Value
    int tag_idx = 1;		// Idx into size_tags
    struct statfs stfs;		// Filesystem stat info


    // Stat the filesystem
    if (statfs(f_curDev, &stfs))
	return;

    stot = ((float) stfs.f_blocks * stfs.f_bsize) / 1024;	// In KB
    sadjtot = stfs.f_blocks - (stfs.f_bfree - stfs.f_bavail);
    sfree = ((float) stfs.f_bavail * stfs.f_bsize) / 1024;
    sused = ((float) (sadjtot - stfs.f_bavail) * stfs.f_bsize) / 1024;
    sadjtot = sadjtot * stfs.f_bsize / 1024;
    pcnt_used = ((sused) / sadjtot) * 100.0 + 0.5;

    // Set tag_idx to reference values in KB already
    tag_idx = 1;
    val = getsztags((long) sfree, &tag_idx);
    sprintf(buf, "%9.2f %s", val, size_tags[tag_idx]);
    delete[]_storage.f_str;
    _storage.f_str = new char[strlen(buf) + 1];
    strcpy(_storage.f_str, buf);
    _storage.free->label(_storage.f_str);
    _storage.free->redraw();

    tag_idx = 1;
    val = getsztags((long) sused, &tag_idx);
    sprintf(buf, "%9.2f %s", val, size_tags[tag_idx]);
    delete[]_storage.u_str;
    _storage.u_str = new char[strlen(buf) + 1];
    strcpy(_storage.u_str, buf);
    _storage.used->label(_storage.u_str);
    _storage.used->redraw();

    tag_idx = 1;
    val = getsztags((long) stot, &tag_idx);
    sprintf(buf, "%9.2f %s", val, size_tags[tag_idx]);
    delete[]_storage.t_str;
    _storage.t_str = new char[strlen(buf) + 1];
    strcpy(_storage.t_str, buf);
    _storage.total->label(_storage.t_str);
    _storage.total->redraw();

    // Set the slider value
    _storsl->value(pcnt_used);
    _storsl->redraw();

    return;
}				// end of NxMemory::GetStorage()

/*******************************************************************************\
**
**	Function:	void GetValues()
**	Desc:		Sets the widgets values based upon the current state of the 
**				application (_mode)
**	Accpets:	int flag = Determines if the values are to be retrieved
**					MEM_GV_UPDMNT = Updates the mount list.
**					MEM_GV_UPDMEM = Updates the memory info
**					MEM_GV_UPDSTO = Updates the storage info
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxMemory::GetValues(int flag)
{
    char buf[255],		// Buffer
     *tmpDev = NULL;		// Temp device name
    int idx;			// Index
    FILE *mntf;			// Mount file
    struct NxMenuItem *mi;	// Menu item ptr

    if (flag & MEM_GV_UPDMNT) {
	// Save off the current storage device
	if (f_curDev != NULL) {
	    tmpDev = new char[strlen(f_curDev) + 1];
	    strcpy(tmpDev, f_curDev);
	}			// end of if

	if ((mntf = fopen("/proc/mounts", "r")) != NULL) {
	    if (_stormb->size()) {
		_stormb->clear();
		_stormb->menu((NxMenuItem *) NULL);
	    }

	    while (fgets(buf, sizeof(buf), mntf)) {
		/* Field 1 is the device */

		char *cp = buf;
		for (; *cp && *cp != ' '; cp++);
		if (!*cp)
		    continue;

		*cp++ = 0;
		char *start = cp;
		int slash = 0;

		for (; *start && *start != ' '; start++)
		    slash++;

		if (*start)
		    *start = 0;

		char *ptr, *str;

		ptr = str = (char *) calloc(strlen(cp) + slash + 1, 1);

		for (start = cp; *start; *start++) {
		    if (*start == '/')
			*ptr++ = '\\';
		    *ptr++ = *start;
		}

		_stormb->add(str);
		free(str);
	    }

	    _stormb->value(0);
	    fclose(mntf);
	}			// end of if
    }				// end of if

    mi = const_cast < struct NxMenuItem *>(_stormb->menu());

    // Update the label
    if (tmpDev) {
	int fnd = 0;
	for (int i = 0; i < _stormb->size() - 1; i++) {
	    if (!mi[i].text)
		continue;

	    if (!strcmp(tmpDev, mi[i].text)) {
		fnd = 1;
		_stormb->value(i);
		break;
	    }			// end of if
	}			// end of for
	if (!fnd)
	    flag |= MEM_GV_UPDSTO;
    }				// end of if

    idx = _stormb->value();
    f_curDev = const_cast < char *>(mi[idx].text);
    _stormb->label(f_curDev);
    _stormb->redraw();

    // Update the memory (if desired)
    if (flag & MEM_GV_UPDMEM)
	GetMemory();

    if (flag & MEM_GV_UPDSTO)
	GetStorage();

    delete[]tmpDev;

    return;
}				// end of NxMemory::GetValues(void)

//-------------------------------------------------------------------------------
//
//      Private static callback methods
//              void stor_mb_cb(Fl_Widget *, void *)
//
//-------------------------------------------------------------------------------

/*******************************************************************************\
**
**	Function:	void stor_mb_cb()
**	Desc:		Callback to select info on the type of storage media currently
**				available
**	Accepts:	Fl_Widget *w = Ptr to the widget responsible for the branch
**				void *d = Ptr to any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxMemory::stor_mb_cb(Fl_Widget * w, void *d)
{
    NxMemory *me = (NxMemory *) d;
    NxMenuButton *mb = (NxMenuButton *) w;	// The menu button
    struct NxMenuItem *mi;	// Ptr to the menu items

    mi = const_cast < struct NxMenuItem *>(mb->menu());
    f_curDev = const_cast < char *>(mi[mb->value()].text);
    mb->label(f_curDev);

    me->GetStorage();
    mb->redraw();

    return;
}				// end of NxMemory::stor_mb_cb(Fl_Widget *, void *)

//------------------------------------------------------------------------------
//
//      Static timer callbacks
//
//------------------------------------------------------------------------------

/*******************************************************************************\
**
**	Function:	void proc_tmr()
**	Desc:		Timer callback to continually (re)read /proc/mounts and get all
**				of the devices mounted into the main filesystem
**	Accepts:	void *d = Any ancillary data
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
NxMemory::proc_tmr(void *d)
{
    NxMemory *me = (NxMemory *) d;
    static int mem_upd = 0;	// Static flag
    int upd_flag = MEM_GV_UPDMNT;	// Default update flags

    if (mem_upd++ == 15) {
	upd_flag |= MEM_GV_UPDMEM;
	mem_upd = 0;
    }				// end of if 

    me->GetValues(upd_flag);

    Fl::add_timeout(2.0, proc_tmr, d);
}				// end of proc_tmr(void *)

//------------------------------------------------------------------------------
//
//      Non-Class static functions
//
//------------------------------------------------------------------------------

/*******************************************************************************\
**
**	Function:	float getsztags()
**	Desc:		Converts the value into the lowest form of BYTES/KB/MB/GB that is
**				>= 1.0 and < 1024.0 units
**	Accepts:	int value = Value to convert (if *idx == 0, values are assumed bytes)
**				int *idx = Current index level
**	Returns:	Double a value >= 1.0 && < 1024.0
**
\*******************************************************************************/
static float
getsztags(int value, int *idx)
{
    int new_idx = *idx;		// New idx
    float retval = value;	// Value

    if (new_idx < 0 || new_idx > SZ_TAGS)
	new_idx = 0;

    while (retval >= 1.0 && retval >= (new_idx == 0 ? 1024 : 1000)
	   && new_idx < SZ_TAGS) {
	retval /= (new_idx == 0 ? 1024.0 : 1000.0);
	new_idx++;
    }				// end of while

    *idx = new_idx;

    return (retval);
}				// end of getsztags(int, int *)
