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


#include <nxmenubar.h>
#include <nxapp.h>

NxMenuBar::NxMenuBar(int x, int y, int w, int h, Fl_Group * _grp)
    :
Fl_Menu_Bar(x, y, w, h)
{

    color(NxApp::Instance()->getGlobalColor(APP_BG));
    textcolor(NxApp::Instance()->getGlobalColor(APP_FG));
    selection_color(NxApp::Instance()->getGlobalColor(HILIGHT));
    labeltype(FL_NORMAL_LABEL);
    NxApp::Instance()->def_font(this);

    grp = _grp;

}

int
NxMenuBar::handle(int event)
{
    int xoff, yoff;

    const Fl_Menu_Item *v;
    if (menu() && menu()->text)
	switch (event) {
	case FL_ENTER:
	    return 1;
	case FL_LEAVE:
	    this->hide();
	    if (grp)
		grp->show();
	    return 1;
	case FL_PUSH:
	    v = 0;
	  J1:
	    /* JHC - This absolutely does not belong here */
	    /* but I couldn't find any other place to put it */
	    /* Basically - the menu offset needs tobe adjusted */
	    /* to account for the window decorations.  This is */
	    /* the poor man's way of determing the actual position */
	    /* of the window: */

	    xoff = Fl::event_x_root() - Fl::event_x();
	    yoff = Fl::event_y_root() - Fl::event_y();

	    v = menu()->pulldown(x() + xoff, y() + yoff,
				 w(), h(), v, this, 0, 1);
	    picked(v);
	    return 1;
	case FL_SHORTCUT:
	    v = menu()->test_shortcut();
	    if (v) {
		picked(v);
		return 1;
	    }
	    if (visible_r() && (v = menu()->find_shortcut()))
		goto J1;
	    return 0;
	}
    return 0;
}
