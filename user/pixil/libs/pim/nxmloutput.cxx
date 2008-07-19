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


// DEBUG
#include <stdio.h>
// end of DEBUG

#include <nxapp.h>
#include <FL/Enumerations.H>
#include <nxmloutput.h>

NxMlOutput::NxMlOutput(int x, int y, int w, int h):
Fl_Multiline_Output(x, y, w, h)
{

    // Provide the specific "look-and-feel"
    active = 1;
    color(NxApp::Instance()->getGlobalColor(APP_BG));
    selection_color(NxApp::Instance()->getGlobalColor(APP_SEL));
    box(FL_FLAT_BOX);
    align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
}				// end of NxOutput::NxOutput()

void
NxMlOutput::deactivate(void)
{
    if (active) {
	printf("In NxMlOutput::deactivate(), setting active=0\n");
	active = 0;
	set_flag(INACTIVE);
	this->redraw();
	clear_flag(INACTIVE);
    }				// end of if
    return;
}				// end of NxMlOutput::deactivate()

void
NxMlOutput::activate(void)
{
    if (!active) {
	printf("In NxMlOutput::activate(), setting active=1\n");
	active = 1;
	clear_flag(INACTIVE);
	this->redraw();
    }				// end of if
    return;
}				// end of NxMlOutput::activate()

void
NxMlOutput::draw(void)
{
    if (!active)
	set_flag(INACTIVE);

    Fl_Multiline_Output::draw();

    if (!active)
	clear_flag(INACTIVE);

    return;
}				// end of NxMlOutput(void)

int
NxMlOutput::handle(int event)
{
    return ((active) ? Fl_Multiline_Output::handle(event) : 0);
}				// end of NxMlOutput::handle(int)
