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
#include "config.h"
#include <FL/fl_draw.H>
#include <FL/forms.H>

#include "Dialog.h"
#include "FLTKUtil.h"
#include "Images.h"
#include "PixilDT.h"
#include "SchedulerChangeTypeDlg.h"
#include "ScheduleEvent.h"
#include "SchedulerDB.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


//--------------------------------------------------------------//
// Constructor                                                  //
//--------------------------------------------------------------//
ScheduleEvent::ScheduleEvent(int nX,
			     int nY,
			     int nWidth,
			     int nHeight,
			     int nHourHeight, int nRow, time_t nDate)
    :
Fl_Box(nX, nY, nWidth, nHeight)
{
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();

#ifdef DEBUG
    assert(nDate ==::NormalizeDate(nDate));	// Must be a normalized date
#endif

    // Save the row number in the SchedulerDB
    m_nRow = nRow;

    // Save the width and height of this widget
    m_nHeight = nHeight;
    m_nWidth = nWidth;

    // Save the height of an hour in this widget
    m_nHourHeight = nHourHeight;

    // Get the date for this event
    m_nDate = nDate;

    // Set that no repeating pixmap has been used yet
    m_pPixmap = NULL;

    // Get the label for this widget
    m_strLabel = pSchedulerDB->GetDescription(nRow);
    m_strLabel =::WrapText(m_strLabel.c_str(), w() - 2 * DLG_BORDER, this);
    label(m_strLabel.c_str());
    align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE);

    // Set other parameters
    box(FL_BORDER_BOX);
    color(FL_YELLOW);

    // Set the state of this widget
    m_bResizing = false;
    m_bMoving = false;
}


//--------------------------------------------------------------//
// Destructor                                                   //
//--------------------------------------------------------------//
ScheduleEvent::~ScheduleEvent()
{
    delete m_pPixmap;
}


//--------------------------------------------------------------//
// Draw this Event                                              //
//--------------------------------------------------------------//
void
ScheduleEvent::draw()
{
    bool bRepeats;
    Fl_Label labelIcon;
    Fl_Label labelText;
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();

    // Redraw everything every time
    color(Fl::focus() == this ? FL_WHITE : FL_YELLOW);
    fl_color(color());
    draw_box();

    // If in focus, draw a heavy lower border
    fl_color(FL_BLACK);
    if (Fl::focus() == this) {
	fl_color(FL_BLACK);
	fl_rectf(x(), y() + h() - EVENT_BORDER, w() - 1, EVENT_BORDER - 1);
    }
    // Always draw the right hand border
    fl_color(FL_BLUE);
    fl_rectf(x() + w() - EVENT_BORDER, y() + 1, EVENT_BORDER - 1,
	     m_nHeight - 1);

    // Fix up the label in case the width has changed
    m_strLabel = pSchedulerDB->GetDescription(m_nRow);
    bRepeats = pSchedulerDB->GetRepeatingFlag(m_nRow);
    m_strLabel =::WrapText(m_strLabel.c_str(),
			   w() - DLG_BORDER - EVENT_BORDER - labelsize() -
			   (bRepeats ? labelsize() : 0), this);

    // Draw the repeating icon if needed
    if (bRepeats == true) {
	if (m_pPixmap == NULL) {
	    m_pPixmap = Images::GetRepeatIcon();
	}
	m_pPixmap->label(this);	// Sets up various static things in Fl_Pixmap.cxx
	labelIcon.value = (const char *) m_pPixmap;
	labelIcon.type = _FL_PIXMAP_LABEL;
	labelIcon.font = FL_HELVETICA;
	labelIcon.size = FL_NORMAL_SIZE;
	labelIcon.color = FL_BLACK;
	labelIcon.draw(x() + Fl::box_dx(box()),
		       y() + Fl::box_dy(box()) + 2,
		       labelsize(), h() - Fl::box_dh(box()), align());
    }
    // Draw the text label manually
    labelText.value = m_strLabel.c_str();
    labelText.type = FL_NORMAL_LABEL;
    labelText.font = FL_HELVETICA;
    labelText.size = FL_NORMAL_SIZE;
    labelText.color = FL_BLACK;
    labelText.draw(x() + Fl::box_dx(box()) +
		   (bRepeats ? labelsize() : labelsize() / 2),
		   y() + Fl::box_dy(box()),
		   w() - Fl::box_dw(box()) - (bRepeats ? 1 : 2) * labelsize(),
		   h() - Fl::box_dh(box()), align());
}


//--------------------------------------------------------------//
// Handle events for this widget                                //
//--------------------------------------------------------------//
int
ScheduleEvent::handle(int nEvent)
{
    int nReturn = Fl_Box::handle(nEvent);

    switch (nEvent) {
    case FL_PUSH:
	// Set this widget up for dragging if the click is in the correct part
	if (Fl::event_button() == FL_LEFT_MOUSE) {
	    if (Fl::focus() == this) {
		// This widget is in focus, test where the click occurred
		if (Fl::event_x() > x() + w() - EVENT_BORDER - 1
		    && Fl::event_x() < x() + w()) {
		    // Get ready to move
		    m_bMoving = true;
		    m_nX = x();
		    m_nY = y();
		    m_nOriginalX = Fl::event_x();
		    m_nOriginalY = Fl::event_y();

		    // Get the schedule container's scroll position
		    m_nOriginalScrollY =
			((Fl_Scroll *) (parent()->parent()->parent()))->
			yposition();
		} else if (Fl::event_y() > y() + h() - EVENT_BORDER - 1
			   && Fl::event_y() < y() + h()) {
		    // Get ready to resize
		    m_bResizing = true;
		    m_nOriginalHeight = m_nHeight;
		    m_nOriginalY = Fl::event_y();
		}
	    } else {
		// Accept a click on the right border for moving even if not in focus
		if (Fl::event_x() > x() + w() - EVENT_BORDER - 1
		    && Fl::event_x() < x() + w()) {
		    // Get ready to move
		    m_bMoving = true;
		    m_nX = x();
		    m_nY = y();
		    m_nOriginalX = Fl::event_x();
		    m_nOriginalY = Fl::event_y();

		    // Get the schedule container's scroll position
		    m_nOriginalScrollY =
			((Fl_Scroll *) (parent()->parent()->parent()))->
			yposition();
		}
	    }

	    // Use the keyboard focus to indicate that this widget is currently selected
	    Fl::focus(this);
	    ((ScheduleDay *) parent())->insert(*this, (Fl_Widget *) NULL);

	    // Notify the parent of the selection
	    ((ScheduleDay *) parent())->SetSelectedItem(m_nRow);

	    // Expand this widget and change the color
	    h(m_nHeight + EVENT_BORDER);
	    color(FL_WHITE);

	    // Redraw this widget
	    redraw();
	    nReturn = 1;
	} else if (Fl::event_button() == FL_RIGHT_MOUSE) {
	    // Right mouse - show an edit menu
	    RightMouse();
	}
	break;

    case FL_RELEASE:
	// If resizing, drop it here
	if (m_bResizing == true) {
	    ResizeDrop();
	} else if (m_bMoving == true) {
	    MoveDrop();
	}
	nReturn = 1;
	break;

    case FL_ENTER:
	nReturn = 1;
	break;

    case FL_LEAVE:
	// Reset to a normal cursor
	PixilDT::GetApp()->GetMainWindow()->ResetCursor();
	break;

    case FL_MOVE:
	if (Fl::focus() == this) {
	    // Change to a move/resize cursor if needed
	    if (Fl::event_x() > x() + w() - EVENT_BORDER - 1
		&& Fl::event_x() < x() + w()) {
		PixilDT::GetApp()->GetMainWindow()->SetCursor(FL_CURSOR_MOVE);
	    } else if (Fl::event_y() > y() + h() - EVENT_BORDER - 1
		       && Fl::event_y() < y() + h()) {
		PixilDT::GetApp()->GetMainWindow()->SetCursor(FL_CURSOR_NS);
	    } else {
		PixilDT::GetApp()->GetMainWindow()->ResetCursor();
	    }
	} else {
	    // Change to a move cursor even if not in focus
	    if (Fl::event_x() > x() + w() - EVENT_BORDER - 1
		&& Fl::event_x() < x() + w()) {
		PixilDT::GetApp()->GetMainWindow()->SetCursor(FL_CURSOR_MOVE);
	    } else {
		PixilDT::GetApp()->GetMainWindow()->ResetCursor();
	    }
	}
	nReturn = 1;
	break;

    case FL_DRAG:
	// If resizing, change the size now
	if (m_bResizing == true) {
	    Resize();
	} else if (m_bMoving == true) {
	    Move();
	}
	break;

    case FL_FOCUS:
	// Always refuse the focus
	nReturn = 0;
	break;

    case FL_UNFOCUS:
	// If resizing, drop it here
	if (m_bResizing == true) {
	    ResizeDrop();
	} else if (m_bMoving == true) {
	    MoveDrop();
	}
	// Reset to the original size
	size(m_nWidth, m_nHeight);
	color(FL_YELLOW);
	parent()->redraw();

	nReturn = 1;
    }
    return (nReturn);
}


//--------------------------------------------------------------//
// Move this widget during a drag operation.                    //
//--------------------------------------------------------------//
void
ScheduleEvent::Move()
{
    int nChangeX;
    int nChangeY;
    int nX;
    int nY;

    // Calculate the movement change so far
    nChangeX = Fl::event_x() - m_nOriginalX;
    nChangeY = Fl::event_y() - m_nOriginalY;

    // Is the new position out of range
    nX = m_nX + nChangeX;
    if (nX < parent()->x()) {
	nX = parent()->x();
    }
    if (nX > parent()->x() + parent()->w() - 1 - w()) {
	nX = parent()->x() + parent()->w() - 1 - w();
    }
    nY = m_nY + nChangeY;
    if (nY < parent()->y()) {
	nY = parent()->y();
    }
    if (nY > parent()->y() + parent()->h() - 1 - h()) {
	nY = parent()->y() + parent()->h() - 1 - h();
    }
    // Move the widget
    position(nX, nY);
    parent()->damage(FL_DAMAGE_ALL);
}


//--------------------------------------------------------------//
// Drop a move operation.                                       //
//--------------------------------------------------------------//
void
ScheduleEvent::MoveDrop()
{
    int nContinue;
    int nNewRow;
    int nScrollPosition =
	((Fl_Scroll *) (parent()->parent()->parent()))->yposition();
    int nStart;
    int nY;
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();
    time_t nStartTime;

    // Move this widget one last time
    Move();

    // Calculate the Y-offset based on the scrolling position
    nY = y() + nScrollPosition - (parent()->parent()->parent())->y();

    // Now round its start time to the nearest half hour
    nStart = (2 * nY + ((m_nHourHeight - 1) >> 1)) / m_nHourHeight;

    // Reset to the original size
    size(m_nWidth, m_nHeight);
    color(FL_YELLOW);

    // Turn off moving
    m_bMoving = false;

    // If this is a repeating event, ask about which instances to change
    if (pSchedulerDB->GetRepeatingFlag(m_nRow) == true) {
	SchedulerChangeTypeDlg *pDlg =
	    new SchedulerChangeTypeDlg(this, false);

	pDlg->DoModal();
	nContinue = pDlg->GetChangeType();
	delete pDlg;
    } else {
	nContinue = SCHEDULER_CHANGE_TYPE_ALL;
    }

    switch (nContinue) {
    case SCHEDULER_CHANGE_TYPE_ALL:
	// Move this event
	pSchedulerDB->MoveStartTime(m_nRow, nStart * 30 * 60);
	break;

    case SCHEDULER_CHANGE_TYPE_CURRENT_FUTURE:
	// Add a new row to carry on from here
	nNewRow = pSchedulerDB->CopyRow(m_nRow);

	// Make these repetitions end one day prior to the selected event
	pSchedulerDB->EndRepetitions(m_nRow,::SubtractDays(m_nDate, 1));

	// Now fix the new row
	nStartTime = pSchedulerDB->GetStartTime(nNewRow);
	nStartTime = nStartTime -::NormalizeDate(nStartTime);
	pSchedulerDB->SetStartDate(nNewRow, m_nDate, nStartTime);
	pSchedulerDB->MoveStartTime(nNewRow, nStart * 30 * 60);
	break;

    case SCHEDULER_CHANGE_TYPE_CURRENT_ONLY:
	// Add a new row for this exception date
	nNewRow = pSchedulerDB->CopyRow(m_nRow);

	// Add a deleted exception for this date
	pSchedulerDB->AddDeletedException(m_nRow, m_nDate);

	// Now fix the new row
	nStartTime = pSchedulerDB->GetStartTime(nNewRow);
	nStartTime = nStartTime -::NormalizeDate(nStartTime);
	pSchedulerDB->SetStartDate(nNewRow, m_nDate, nStartTime);
	pSchedulerDB->SetNoRepetition(nNewRow);
	pSchedulerDB->MoveStartTime(nNewRow, nStart * 30 * 60);
    }

    // Save the changes to the scheduler data base
    pSchedulerDB->Save();

    // Notify everyone of the changes, will refresh this widget also
    PixilDT::GetApp()->GetMainWindow()->Notify(SCHEDULER_CHANGED, 0);

    // Reset to a normal cursor
    PixilDT::GetApp()->GetMainWindow()->ResetCursor();
}


//--------------------------------------------------------------//
// Catch a resize operation and save the height and width.      //
//--------------------------------------------------------------//
void
ScheduleEvent::resize(int nX, int nY, int nWidth, int nHeight)
{
    m_nWidth = nWidth;
    m_nHeight = nHeight;
    Fl_Box::resize(nX, nY, nWidth, nHeight);
}


//--------------------------------------------------------------//
// Resize this widget during a drag operation.                  //
//--------------------------------------------------------------//
void
ScheduleEvent::Resize()
{
    int nChange;
    int nOldHeight;

    // Calculate the height change so far
    nChange = Fl::event_y() - m_nOriginalY;

    // Is the duration too small
    m_nHeight = m_nOriginalHeight + nChange;
    if (m_nHeight < (m_nHourHeight >> 1)) {
	m_nHeight = (m_nHourHeight >> 1);
    }
    // Resize the widget
    nOldHeight = h();
    damage_resize(x(), y(), w(), m_nHeight + EVENT_BORDER);

    // Needed or the widget will not re-draw correctly (why?)
    if (nOldHeight > m_nHeight) {
	parent()->damage(FL_DAMAGE_ALL);
    }
}


//--------------------------------------------------------------//
// Drop a resize operation.                                     //
//--------------------------------------------------------------//
void
ScheduleEvent::ResizeDrop()
{
    int nContinue;
    int nDuration;
    int nNewRow;
    SchedulerDB *pSchedulerDB = SchedulerDB::GetSchedulerDB();
    time_t nStartTime;

    // Resize this widget one last time
    Resize();
    damage(FL_DAMAGE_ALL);

    // Now round its end time to the nearest half hour
    nDuration = (2 * h() + ((m_nHourHeight - 1) >> 1)) / m_nHourHeight;

    // Reset the height of this widget
    m_nHeight = ((nDuration * m_nHourHeight) >> 1);
    h(m_nHeight);

    // Reset to the original size
    size(m_nWidth, m_nHeight);
    color(FL_YELLOW);

    // Turn off resizing
    m_bResizing = false;

    // If this is a repeating event, ask about which instances to change
    if (pSchedulerDB->GetRepeatingFlag(m_nRow) == true) {
	SchedulerChangeTypeDlg *pDlg =
	    new SchedulerChangeTypeDlg(this, false);

	pDlg->DoModal();
	nContinue = pDlg->GetChangeType();
	delete pDlg;
    } else {
	nContinue = SCHEDULER_CHANGE_TYPE_ALL;
    }

    switch (nContinue) {
    case SCHEDULER_CHANGE_TYPE_ALL:
	// Resize this event
	pSchedulerDB->SetRoundedDuration(m_nRow, nDuration * 30 * 60);
	break;

    case SCHEDULER_CHANGE_TYPE_CURRENT_FUTURE:
	// Add a new row to carry on from here
	nNewRow = pSchedulerDB->CopyRow(m_nRow);

	// Make these repetitions end one day prior to the selected event
	pSchedulerDB->EndRepetitions(m_nRow,::SubtractDays(m_nDate, 1));

	// Now fix the new row
	nStartTime = pSchedulerDB->GetStartTime(nNewRow);
	nStartTime = nStartTime -::NormalizeDate(nStartTime);
	pSchedulerDB->SetStartDate(nNewRow, m_nDate, nStartTime);
	pSchedulerDB->SetRoundedDuration(nNewRow, nDuration * 30 * 60);
	break;

    case SCHEDULER_CHANGE_TYPE_CURRENT_ONLY:
	// Add a new row for this exception date
	nNewRow = pSchedulerDB->CopyRow(m_nRow);

	// Add a deleted exception for this date
	pSchedulerDB->AddDeletedException(m_nRow, m_nDate);

	// Now fix the new row
	nStartTime = pSchedulerDB->GetStartTime(nNewRow);
	nStartTime = nStartTime -::NormalizeDate(nStartTime);
	pSchedulerDB->SetStartDate(nNewRow, m_nDate, nStartTime);
	pSchedulerDB->SetNoRepetition(nNewRow);
	pSchedulerDB->SetRoundedDuration(nNewRow, nDuration * 30 * 60);
    }

    // Save the changes to the scheduler data base
    pSchedulerDB->Save();

    // If an actual change then notify everyone of the changes
    PixilDT::GetApp()->GetMainWindow()->Notify(SCHEDULER_CHANGED, 0);

    // Reset to a normal cursor
    PixilDT::GetApp()->GetMainWindow()->ResetCursor();
}


//--------------------------------------------------------------//
// Handle a right mouse click.                                  //
//--------------------------------------------------------------//
void
ScheduleEvent::RightMouse()
{
    static const Fl_Menu_Item menuPopup[] = {
	{N_("Delete Appointment")},
	{N_("Edit Appointment"), 0, 0, 0, FL_MENU_DIVIDER},
	{N_("New Appointment")},
	{NULL},
    };
    int nSelection;

    // Display the popup menu
    nSelection = DoPopupMenu(menuPopup, Fl::event_x(), Fl::event_y());

    if (nSelection >= 0) {
	// Process the selection
	switch (nSelection) {
	case 0:		// Delete this row
	    ((ScheduleDay *) parent())->Delete(m_nRow);
	    break;

	case 1:		// Edit this row
	    ((ScheduleDay *) parent())->Edit(m_nRow);
	    break;

	case 2:		// Insert a new row
	    ((ScheduleDay *) parent())->EditNew(m_nDate);
	}
    }
}
