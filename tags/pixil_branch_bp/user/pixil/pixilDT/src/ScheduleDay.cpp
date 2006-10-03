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
// Schedule Day widget                                          //
//--------------------------------------------------------------//
#include "config.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/forms.H>

#include "ScheduleDay.h"
#include "ScheduleEvent.h"
#include "SchedulerDB.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


#ifdef WIN32
#define max __max
#define min __min
#endif


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
ScheduleDay::ScheduleDay(int nX,
			 int nY,
			 int nWidth,
			 int nHeight, int nHourHeight, time_t nDate)
    :
Fl_Group(nX, nY, nWidth, nHeight)
{
    // Save the height of an hour
    m_nHourHeight = nHourHeight;

    // Save the date in question
    m_nDate = nDate;

    // Finish this widget
    end();

    // Set that it is resizable
    resizable(this);

    // Set a white background
    color(FL_WHITE);
    box(FL_BORDER_BOX);
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
ScheduleDay::~ScheduleDay()
{
}


//--------------------------------------------------------------//
// Process a request to delete an appointment                   //
//--------------------------------------------------------------//
void
ScheduleDay::Delete(int nRow)
{
    ((ScheduleContainer *) (parent()->parent()))->Delete(nRow, m_nDate);
}


//--------------------------------------------------------------//
// Draw this day's appointments                                 //
//--------------------------------------------------------------//
void
ScheduleDay::draw()
{
    Fl_Widget *const *arrayWidget = array();
    Fl_Widget *pWidget;
    int i;
    int nHeight;

    if (damage() & ~FL_DAMAGE_CHILD) {
	// redraw the entire thing:
	draw_box();
	draw_label();

	// Draw the hourly lines
	fl_color(FL_BLACK);
	for (i = 1; i < 24; ++i) {
	    nHeight = y() + i * m_nHourHeight;
	    fl_line(x(), nHeight, x() + w() - 1, nHeight);
	}

	// Continue with the drawing operation
	for (i = children(); i--;) {
	    pWidget = *arrayWidget++;
	    draw_child(*pWidget);
	}
    } else {
	// only redraw the children that need it:
	for (i = children(); i--;) {
	    update_child(**arrayWidget++);
	}
    }
}


//--------------------------------------------------------------//
// Process a request to edit an appointment                     //
//--------------------------------------------------------------//
void
ScheduleDay::Edit(int nRow)
{
    ((ScheduleContainer *) (parent()->parent()))->Edit(nRow, m_nDate);
}


//--------------------------------------------------------------//
// Process a request to add a new appointment.                  //
//--------------------------------------------------------------//
void
ScheduleDay::EditNew(time_t nDate)
{
    ((ScheduleContainer *) (parent()->parent()))->EditNew(nDate);
}


//--------------------------------------------------------------//
// Handle events for this widget.                               //
//--------------------------------------------------------------//
int
ScheduleDay::handle(int nEvent)
{
    bool bSafe = true;
    bool bTakeFocus = true;
    int i;
    int nReturn = 0;

    // Bug somewhere, only give this to Fl_Group if the child array is safe
    // Somehow the child array becomes damaged
    for (i = 0; i < children(); ++i) {
	if (dynamic_cast < ScheduleEvent * >(child(children() - 1)) == NULL) {
	    bSafe = false;
	}
    }

    // Bypass all processing if the child array is damaged (bug somewhere...)
    if (bSafe == true) {
	nReturn = Fl_Group::handle(nEvent);

	switch (nEvent) {
	case FL_PUSH:
	    if (Fl::event_button() == FL_LEFT_MOUSE) {
		// Test if this click was within a child or not
		for (i = 0; i < children() && bTakeFocus == true; ++i) {
		    if (Fl::event_inside(child(i))) {
			bTakeFocus = false;
		    }
		}

		// Only take the focus if this was not in a child ScheduleEvent
		if (bTakeFocus == true) {
		    Fl::focus(this);
		    nReturn = 1;
		}
	    } else if (Fl::event_button() == FL_RIGHT_MOUSE) {
		// Right mouse - show a popup menu
		RightMouse();
	    }
	}
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Refresh these appointments.                                  //
//--------------------------------------------------------------//
void
ScheduleDay::Refresh(time_t nDate)
{
    bool bChange;
    Fl_Group *pGroup;
    Fl_Widget *pChild;
    int i;
    int j;
    int nChild;
    int nEndSlot;
    int nIncrement;
    int nMax;
    int nMinHeight;
    int nMinWidth;
    int nPerSlot[48];
    int nSlotCount;
    int nStartSlot;
    int nWidth;
    multimap < int, int >::iterator iter;
    ScheduleEvent *pScheduleEvent;
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();
    SchedulerRepeatData *pSchedulerRepeatData;
    unsigned char *ucSlots;

    // Save the current day
    m_nDate =::NormalizeDate(nDate);

    // Hide all children
    for (nChild = children(); --nChild >= 0;) {
	child(nChild)->hide();
    }

    // Set this object as the current group
    pGroup = Fl_Group::current();
    begin();

    // Get the minimum width of an event
    fl_font(labelfont(), labelsize());
    fl_measure("WWW", nMinWidth, nMinHeight);
    nMinWidth += 2 * EVENT_BORDER;

    // Get the appointments for this day
    pSchedulerDB->GetAllAppointments(m_nDate, m_mRecord);

    // Clear the count by half hour
    for (i = 0; i < 48; ++i) {
	nPerSlot[i] = 0;
    }

    // Get the number of concurrent appointments for each half hour
    for (iter = m_mRecord.begin(); iter != m_mRecord.end(); ++iter) {
	pSchedulerRepeatData =
	    pSchedulerDB->GetSchedulerRepeatData(iter->second);
	pSchedulerRepeatData->GetStartEnd(nStartSlot, nEndSlot, 30 * 60);
	for (i = nStartSlot; i <= nEndSlot; ++i) {
	    ++nPerSlot[i];
	}
	delete pSchedulerRepeatData;
    }

    // Fix this for the maximum number of appointments needed for drawing each half hour
    do {
	bChange = false;
	for (iter = m_mRecord.begin(); iter != m_mRecord.end(); ++iter) {
	    pSchedulerRepeatData =
		pSchedulerDB->GetSchedulerRepeatData(iter->second);
	    pSchedulerRepeatData->GetStartEnd(nStartSlot, nEndSlot, 30 * 60);

	    // Get the maximum number of divisions needed for each half hour for this event
	    nMax = 0;
	    for (i = nStartSlot; i <= nEndSlot; ++i) {
		if (nMax < nPerSlot[i]) {
		    nMax = nPerSlot[i];
		}
	    }

	    // Make sure that all half hours for this event are at that maximum
	    for (i = nStartSlot; i <= nEndSlot; ++i) {
		if (nPerSlot[i] < nMax) {
		    nPerSlot[i] = nMax;
		    bChange = true;
		}
	    }

	    // Clean up
	    delete pSchedulerRepeatData;
	}
    } while (bChange == true);

    // Determine the number of horizontal slots
    nSlotCount = 1;
    for (i = 0; i < 48; ++i) {
	if (nPerSlot[i] > 1) {
	    int nX;
	    int nY;
	    int nZ;

	    // Calculate the GCD
	    nX = min(nSlotCount, nPerSlot[i]);
	    nY = max(nSlotCount, nPerSlot[i]);
	    while ((nY % nX) != 0) {
		nZ = nX;
		nX = (nY % nX);
		nY = nZ;
	    }
	    nSlotCount = nSlotCount * nPerSlot[i] / nX;
	}
    }

    // Now create the events for each half hour
    ucSlots = new unsigned char[48 * nSlotCount];
    memset(ucSlots, 0, 48 * nSlotCount * sizeof(unsigned char));
    for (iter = m_mRecord.begin(); iter != m_mRecord.end(); ++iter) {
	// Find the slots for this event
	pSchedulerRepeatData =
	    pSchedulerDB->GetSchedulerRepeatData(iter->second);
	pSchedulerRepeatData->GetStartEnd(nStartSlot, nEndSlot, 30 * 60);
	nIncrement = nSlotCount / nPerSlot[nStartSlot];

	// Search horizontally
	for (i = 0; i < nSlotCount; i += nIncrement) {
	    // Verify each time slot
	    for (j = nStartSlot; j <= nEndSlot; ++j) {
		// Check each half hour for a slot large enough
		if (memchr(ucSlots + j * nSlotCount + i, 0xff, nIncrement) !=
		    NULL) {
		    break;
		}
	    }

	    // Early exit from the "k" for loop ?
	    if (j > nEndSlot) {
		// If no, then the event will fit here
		break;
	    }
	}

#ifdef DEBUG
	assert(i < nSlotCount);	// Must have found a place or else
#endif

	// Mark these slots as in use
	for (j = nStartSlot; j <= nEndSlot; ++j) {
	    memset(ucSlots + j * nSlotCount + i, 0xff, nIncrement);
	}

	// Create an event
	nWidth = (w() - 2 * EVENT_BORDER) / nPerSlot[nStartSlot];
	if (nWidth < nMinWidth) {
	    nWidth = nMinWidth;
	}
	// Find or create a widget for use with this row
	for (nChild = children(); --nChild >= 0;) {
	    pScheduleEvent = (ScheduleEvent *) child(nChild);
	    if (pScheduleEvent->GetRow() == iter->second) {
		break;
	    }
	}
	if (nChild < 0) {
	    // Mark as item not found
	    pScheduleEvent = NULL;
	}
	// Will the widget be visible
	if (i * nWidth - EVENT_BORDER < w()) {
	    if (pScheduleEvent != NULL) {
		// Existing widget, change its date and reposition it
		pScheduleEvent->SetDate(m_nDate);
		pScheduleEvent->damage_resize(x() + i * nWidth + EVENT_BORDER,
					      y() +
					      (nStartSlot * m_nHourHeight) /
					      2,
					      (i + nIncrement == nSlotCount)
					      ? w() - 3 * EVENT_BORDER -
					      i * nWidth : nWidth -
					      2 * EVENT_BORDER,
					      (m_nHourHeight *
					       (nEndSlot - nStartSlot +
						1)) / 2);
		pScheduleEvent->show();
	    } else {
		// No pre-exising widget, create it
		pScheduleEvent =
		    new ScheduleEvent(x() + i * nWidth + EVENT_BORDER,
				      y() + (nStartSlot * m_nHourHeight) / 2,
				      (i + nIncrement == nSlotCount)
				      ? w() - 3 * EVENT_BORDER -
				      i * nWidth : nWidth - 2 * EVENT_BORDER,
				      (m_nHourHeight *
				       (nEndSlot - nStartSlot + 1)) / 2,
				      m_nHourHeight, iter->second, m_nDate);
	    }
	}
	// Clean up
	delete pSchedulerRepeatData;
    }

    // End adding widgets to this container
    end();
    current(pGroup);

    // Now remove any hidden child widgets
    for (nChild = children(); --nChild >= 0;) {
	pChild = child(nChild);
	if (!pChild->visible()) {
	    remove(pChild);
	    delete pChild;
	}
    }

    // Clean up
    delete[]ucSlots;

    // Redraw this widget
    redraw();
}


//--------------------------------------------------------------//
// Handle a right mouse click.                                  //
//--------------------------------------------------------------//
void
ScheduleDay::RightMouse()
{
    static const Fl_Menu_Item menuPopup[] = {
	{N_("New Appointment")},
	{NULL},
    };
    int nEventY =
	Fl::event_y() + ((Fl_Scroll *) (parent()->parent()))->yposition() -
	(parent()->parent())->y();
    int nSelection;

    // Display the popup menu
    nSelection = DoPopupMenu(menuPopup, Fl::event_x(), Fl::event_y());

    if (nSelection >= 0) {
	// Process the selection
	switch (nSelection) {
	case 0:		// Insert a new row
	    EditNew(m_nDate + 60 * 60 * (nEventY / m_nHourHeight));
	}
    }
}


//--------------------------------------------------------------//
// Set the currently selected item.  Not inline because of      //
// header recursion problems.                                   //
//--------------------------------------------------------------//
void
ScheduleDay::SetSelectedItem(int nRow)
{
    ((ScheduleContainer *) (parent()->parent()))->SetSelectedItem(nRow);
}
