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
// Class used to access all images in the application.          //
//--------------------------------------------------------------//
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <strstream>
#include "Images.h"

#include "VCMemoryLeak.h"


using namespace std;


typedef char *PCHAR;


#ifdef WIN32
typedef unsigned long COLORREF;
#define COLOR_BTNSHADOW 16
#define COLOR_3DSHADOW  COLOR_BTNSHADOW
extern "C"
{
    __declspec(dllimport) COLORREF __stdcall GetSysColor(int nIndex);
}
#endif


//--------------------------------------------------------------//
// Static singleton pointer                                     //
//--------------------------------------------------------------//
Images *
    Images::m_pThis =
    NULL;


//--------------------------------------------------------------//
// Disabled XPM's.  Created from the original disabled image,   //
// but the shadow color is changed to match the current color   //
// scheme.                                                      //
//--------------------------------------------------------------//
char **
    Images::m_ppszDisabledXPM[MAX_IMAGES];


//--------------------------------------------------------------//
// Constructor, singleton created once at the file level.       //
//--------------------------------------------------------------//
Images::Images()
{
    int i;

    // Set the singleton locator
    m_pThis = this;

    // Clear all disabled images
    for (i = 0; i < MAX_IMAGES; ++i) {
	m_ppszDisabledXPM[i] = NULL;
    }
}


//--------------------------------------------------------------//
// Destructor.                                                  //
//--------------------------------------------------------------//
Images::~Images()
{
    int i;

    // Reset the singleton pointer
    m_pThis = NULL;

    // Clear all disabled images
    for (i = 0; i < MAX_IMAGES; ++i) {
	if (m_ppszDisabledXPM[i] != NULL) {
	    delete[]m_ppszDisabledXPM[i][2];
	    delete[]m_ppszDisabledXPM[i];
	}
    }
}


//--------------------------------------------------------------//
// Destroy all image storage                                    //
//--------------------------------------------------------------//
void
Images::Destroy()
{
    if (m_pThis != NULL) {
	delete m_pThis;
	m_pThis = NULL;
    }
}


//--------------------------------------------------------------//
// Get a disabled image.  A tru disabled image has only two     //
// colors over the transparent pixil setting.  The second color //
// must be changed to match the current color scheme being      //
// displayed.                                                   //
//--------------------------------------------------------------//
Fl_Pixmap *
Images::GetDisabledImage(int nImage)
{
    int i;

#ifdef DEBUG
    assert(nImage >= 0 && nImage < MAX_IMAGES);
#endif

    // Copy the XPM if needed
    if (m_ppszDisabledXPM[nImage] == NULL) {
	int nColors;
	int nDummy;
	int nRows;
	istrstream isXPM(m_ppszXPM[nImage][0]);

	// Get the number of rows and colors in the XPM
	isXPM >> nDummy >> nRows >> nColors;

	// Allocate space for the disabled XPM
	m_ppszDisabledXPM[nImage] = new PCHAR[nRows + nColors + 1];

	// Copy to the new XPM
	for (i = 0; i < nRows + nColors + 1; ++i) {
	    // Copy the pointers only
	    m_ppszDisabledXPM[nImage][i] = (char *) m_ppszXPM[nImage][i];
	}

	// Fix the second (first non-transparent) color
	char szColor[7];

#ifdef WIN32

	// Get the Windows Button Shadow color
	COLORREF crShadow =::GetSysColor(COLOR_3DSHADOW);
	sprintf(szColor,
		"%02x%02x%02x",
		crShadow & 0xff,
		(crShadow >> 8) & 0xff, (crShadow >> 16) & 0xff);

#else

	// For Linux, don't change the color
	strncpy(szColor, m_ppszXPM[nImage][2] + 5, 6);	// 5 = '#' in "1 c #nnnnnn"
	szColor[6] = '\0';

#endif

	m_ppszDisabledXPM[nImage][2] =
	    new char[strlen(m_ppszXPM[nImage][2]) + 1];
	strcpy(m_ppszDisabledXPM[nImage][2], m_ppszXPM[nImage][2]);
	strcpy(m_ppszDisabledXPM[nImage][2] + 5, szColor);	// 5 =  '#' in "1 c #nnnnnn"
    }

    return (new Fl_Pixmap(m_ppszDisabledXPM[nImage]));
}


//--------------------------------------------------------------//
// Get an image, not inline so that memory leak detection       //
// macros can be in force.                                      //
//--------------------------------------------------------------//
Fl_Pixmap *
Images::GetImage(int nImage)
{
#ifdef DEBUG
    assert(nImage >= 0 && nImage < MAX_IMAGES);
#endif

    return (new Fl_Pixmap((char *const *) m_ppszXPM[nImage]));
}


#ifndef WIN32
extern unsigned char **fl_mask_bitmap;
//--------------------------------------------------------------//
// Get a Pixmap for an image, the caller must call eventually   //
// call XFreePixmap on the returned Pixmap.                     //
//--------------------------------------------------------------//
unsigned long
Images::GetPixmap(int nIndex)
{
    Window window;
    int nHeight;
    int nWidth;
    unsigned long nID;

#ifdef DEBUG
    assert(nIndex >= 0 && nIndex < MAX_IMAGES);
#endif

    // Get the size of the Pixmap
    fl_measure_pixmap((char *const *) m_ppszXPM[nIndex], nWidth, nHeight);

    // Create an offscreen image
    window = fl_window;
    fl_window = DefaultRootWindow(fl_display);
    nID = (unsigned long) fl_create_offscreen(nWidth, nHeight);
    fl_begin_offscreen((Fl_Offscreen) nID);
    fl_mask_bitmap = NULL;
    fl_draw_pixmap((char *const *) m_ppszXPM[nIndex], 0, 0, FL_GRAY);
    fl_end_offscreen();
    fl_window = window;
    return (nID);
}
#endif


//--------------------------------------------------------------//
// XPM's go here.                                               //
//--------------------------------------------------------------//
#include "images/AddressBook.xpm"
#include "images/BigLeft.xpm"
#include "images/BigRight.xpm"
#include "images/Box.xpm"
#include "images/Calendar.xpm"
#include "images/Checkbox.xpm"
#include "images/CurHelp.xpm"
#include "images/DisabledCalendar.xpm"
#include "images/DisabledCopy.xpm"
#include "images/DisabledCut.xpm"
#include "images/DisabledNotes.xpm"
#include "images/DisabledPaste.xpm"
#include "images/DisabledPrint.xpm"
#include "images/DisabledSave.xpm"
#include "images/DisabledUndo.xpm"
#include "images/EditCopy.xpm"
#include "images/EditCut.xpm"
#include "images/EditPaste.xpm"
#include "images/EditUndo.xpm"
#include "images/FilePrint.xpm"
#include "images/FileSave.xpm"
#include "images/Find.xpm"
#include "images/Left.xpm"
#include "images/NewItem.xpm"
#include "images/Notes.xpm"
#include "images/NotesIcon.xpm"
#include "images/pixil_icon.xpm"
#include "images/Private.xpm"
#include "images/Repeat.xpm"
#include "images/Right.xpm"
#include "images/Scheduler.xpm"
#include "images/SmallDownIcon.xpm"
#include "images/SmallUpIcon.xpm"
#include "images/TimeIcon.xpm"
#include "images/ToDoList.xpm"


//--------------------------------------------------------------//
// Pointers to source XPM's, these must be in the same order as //
// the defines in the Images.h header file.                     //
//--------------------------------------------------------------//
char const *const *
    Images::m_ppszXPM[] = {
    AddressBookXPM,
    BigLeftIconXPM,
    BigRightIconXPM,
    BoxXPM,
    CalendarIconXPM,
    CheckboxXPM,
    CurHelpIconXPM,
    DisabledCalendarIconXPM,
    DisabledCopyIconXPM,
    DisabledCutIconXPM,
    DisabledNotesXPM,
    DisabledPasteIconXPM,
    DisabledPrintIconXPM,
    DisabledSaveIconXPM,
    DisabledUndoIconXPM,
    EditCopyIconXPM,
    EditCutIconXPM,
    EditPasteIconXPM,
    EditUndoIconXPM,
    FilePrintIconXPM,
    FileSaveIconXPM,
    FindIconXPM,
    LeftIconXPM,
    NewItemIconXPM,
    NotesXPM,
    NotesIconXPM,
    PixilIconXPM,
    PrivateXPM,
    RepeatXPM,
    RightIconXPM,
    SchedulerXPM,
    SmallDownIconXPM,
    SmallUpIconXPM,
    TimeIconXPM,
    ToDoListXPM,
};
