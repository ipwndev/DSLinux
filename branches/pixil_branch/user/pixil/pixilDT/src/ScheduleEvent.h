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
// Schedule Event widget                                        //
//--------------------------------------------------------------//
#ifndef SCHEDULEEVENT_H_

#define SCHEDULEEVENT_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include "config.h"
#include <string>
#include <FL/Fl_Box.H>
#include <FL/Fl_Pixmap.H>

#define EVENT_BORDER 6
using namespace std;
class ScheduleEvent:public Fl_Box
{
  public:ScheduleEvent(int nX,
		  // Constructor
		  int nY, int nWidth, int nHeight, int nHourHeight,
		  int nRow, time_t nDate);
     ~ScheduleEvent();		// Destructor
    int GetRow() const		// Get the row number for this event
    {
	return (m_nRow);
    }
    void resize(int nX,		// From Fl_Widget/Fl_Box - virtual resize method
		int nY, int nWidth, int nHeight);
    void SetDate(time_t nDate)	// Reset the date of this widget
    {
	m_nDate = nDate;
    }
  protected:void draw();	// Draw this widget
    int handle(int nEvent);	// Handle events for this widget
  private:bool m_bMoving;	// Moving operation is in progress
    bool m_bResizing;		// Resizing operation is in progress
    Fl_Pixmap *m_pPixmap;	// Repeating event pixmap
    int m_nHeight;		// The original height of the event
    int m_nHourHeight;		// The height of a single hour
    int m_nOriginalHeight;	// Original Height during a resize operation
    int m_nOriginalScrollY;	// The original scrolling position of the ScheduleContainer when a drag move was started
    int m_nOriginalX;		// The original X coordinate of a drag operation
    int m_nOriginalY;		// The original Y coordinate of a drag operation
    int m_nRow;			// The row in the SchedulerDB
    int m_nWidth;		// The original width of the widget
    int m_nX;			// The original location prior to a drag move
    int m_nY;			// The original location prior to a drag move
    string m_strLabel;		// The label for this widget
    time_t m_nDate;		// The date of this event
    void Move();		// Process a drag move
    void MoveDrop();		// End a drag move
    void Resize();		// Resize this widget during a drag
    void ResizeDrop();		// Finish a resize operation
    void RightMouse();		// Handle a right mouse click
};


#endif /*  */
