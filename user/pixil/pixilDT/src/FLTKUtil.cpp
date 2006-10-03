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
// FLTK utilities.                                              //
//--------------------------------------------------------------//
#include "config.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <strstream>
#include <FL/fl_draw.H>
#include "FLTKUtil.h"

#include "VCMemoryLeak.h"


#ifdef WIN32
typedef unsigned long COLORREF;
#define COLOR_BTNSHADOW 16
#define COLOR_MENU       4
#define COLOR_3DSHADOW  COLOR_BTNSHADOW
extern "C"
{
    __declspec(dllimport) COLORREF __stdcall GetSysColor(int nIndex);
#define strcasecmp stricmp
}
#endif


#define LEADING 4		// From Fl_Menu.cxx


//--------------------------------------------------------------//
// Break a string into lines delimited by new line characters.  //
//--------------------------------------------------------------//
void
BreakLines(const char *pszText, vector < string > &vLine)
{
    char *pChr;
    string strData;

    // Clear the vector of strings
    vLine.clear();

    // Process each line
    while (*pszText != '\0') {
	// Get the next line
	pChr = strchr(pszText, '\n');
	strData = pszText;
	if (pChr == NULL) {
	    // Last line in the text
	    pszText += strlen(pszText);
	} else {
	    // Intermediate line
	    strData = strData.substr(0, pChr - pszText);
	    pszText = pChr + 1;
	}

	// Kill any trailing white space
	while (strData.length() > 0 && isspace(strData[strData.length() - 1])) {
	    strData = strData.substr(0, strData.length() - 1);
	}

	// Add this string to the vector
	vLine.push_back(strData);
    }
}


//--------------------------------------------------------------//
// Create a translated menu usable for an Fl_Choice widget.     //
//--------------------------------------------------------------//
Fl_Menu_Item *
CreateChoice(int nCount, const char *const *ppszText, bool bTranslateMenu)
{
    Fl_Menu_Item *pReturn;
    int i;

    // Create the menu
    pReturn = new Fl_Menu_Item[nCount + 1];
    memset(pReturn, 0, (nCount + 1) * sizeof(Fl_Menu_Item));
    for (i = 0; i < nCount; ++i) {
	// Use strdup so that FreeTranslatedMenu will work correctly
	if (bTranslateMenu == true) {
	    pReturn[i].text = strdup(_(ppszText[i]));
	} else {
	    // Special, menu is already translated, don't translate again
	    pReturn[i].text = strdup(ppszText[i]);
	}
    }
    return (pReturn);
}


//--------------------------------------------------------------//
// Run a modal dialog.  Returns 1 if the OK button pressed or 0 //
// if the Cancel button was pressed.                            //
//--------------------------------------------------------------//
int
DoModal(Fl_Window * pWindow, Fl_Button * pOKButton, Fl_Button * pCancelButton)
{
    // Set the window modal and show it
    pWindow->set_modal();
    pWindow->show();

    // Process until the window shuts down
    while (pWindow->shown()) {
	Fl::wait();
	for (;;) {
	    Fl_Widget *pWidget = Fl::readqueue();

	    if (pWidget == NULL) {
		break;
	    }
	    if (pWidget == pOKButton) {
		// Done
		return (1);
	    }
	    if (pWidget == pWindow || pWidget == pCancelButton) {
		return (0);
	    }
	}
    }
    return (0);
}


//--------------------------------------------------------------//
// Run a popup menu, first determine the size of the menu and   //
// from that its positioning based on the requested point.      //
// Then run the popup menu and return the 0-based selection     //
// number or -1 if nothing was selected.  Shortcuts in the menu //
// will be ignored (not even displayed).                        //
//--------------------------------------------------------------//
int
DoPopupMenu(const Fl_Menu_Item * pMenuItem, int nX, int nY)
{
    const Fl_Menu_Item *pSelection;
    Fl_Menu_Item *pTranslated;
    int nHeight = 0;
    int nItem;
    int nReturn;
    int nScreenHeight = Fl::h();
    int nScreenWidth = Fl::w();
    int nSize = pMenuItem->size();
    int nThisHeight;
    int nThisWidth;
    int nWidth = 0;

    // Get the maximum height and width for any item
    for (nItem = 0; nItem < nSize; ++nItem) {
	nThisWidth = pMenuItem[nItem].measure(&nThisHeight, NULL);
	if (nThisWidth > nWidth) {
	    nWidth = nThisWidth;
	}
	if (nThisHeight > nHeight) {
	    nHeight = nThisHeight;
	}
    }

    // Fix the height for the entire menu
    nHeight = (nHeight + LEADING) * (nSize - 1);

    // Determine the location of the menu
    if (nX + nWidth >= nScreenWidth) {
	nX = nScreenWidth - nWidth;
    }
    if (nY + nHeight >= nScreenHeight) {
	nY = nScreenHeight - nHeight;
    }
    // Now show the popup menu
    pTranslated = TranslateMenuItems(pMenuItem);
    pSelection = pTranslated->popup(nX, nY);
    nReturn = (pSelection != NULL ? pSelection - pTranslated : -1);
    FreeTranslatedMenu(pTranslated);
    return (nReturn);
}


//--------------------------------------------------------------//
// Free up a translated menu.                                   //
//--------------------------------------------------------------//
void
FreeTranslatedMenu(Fl_Menu_Item * pMenuItem)
{
    Fl_Menu_Item *pMenuItem2 = (Fl_Menu_Item *) pMenuItem;
    int nNest;

    // Free each string in the menu
    nNest = 0;
    while (nNest > 0 || pMenuItem2->text != NULL) {
	if (pMenuItem2->text != NULL) {
	    // These were allocated with strdup, so free (not delete) them
	    free((char *) pMenuItem2->text);
	}
	// Reset the nesting as needed
	if (pMenuItem2->text == NULL) {
	    // Reduce the nesting count
	    --nNest;
	} else if ((pMenuItem2->flags & FL_SUBMENU) != 0) {
	    // Increment the nesting
	    ++nNest;
	}
	// Go to the next menu item
	++pMenuItem2;
    }

    // Now free the entire menu (use delete [])
    delete[]pMenuItem;
}


//--------------------------------------------------------------//
// Get an FLTK color.                                           //
//--------------------------------------------------------------//
Fl_Color
GetFLTKColor(int nRGB)
{
    return (GetFLTKColor((nRGB >> 16), (nRGB >> 8), nRGB));
}


//--------------------------------------------------------------//
// Get an FLTK color.                                           //
//--------------------------------------------------------------//
Fl_Color
GetFLTKColor(int nRed, int nGreen, int nBlue)
{
    return (fl_color_cube((nRed & 0xff) * FL_NUM_RED / 256,
			  (nGreen & 0xff) * FL_NUM_GREEN / 256,
			  (nBlue & 0xff) * FL_NUM_BLUE / 256));
}


//--------------------------------------------------------------//
// Translate menu items using gettext.  The original menu is    //
// const so the entire menu is duplicated with the translated   //
// strings.                                                     //
//--------------------------------------------------------------//
Fl_Menu_Item *
TranslateMenuItems(const Fl_Menu_Item * pMenuItem)
{
    Fl_Menu_Item *pMenuItem2 = (Fl_Menu_Item *) pMenuItem;
    int i;
    int nCount = pMenuItem->size();

    // Allocate space for the new menu
    pMenuItem2 = new Fl_Menu_Item[ /*++ */ nCount];
    memcpy(pMenuItem2, pMenuItem, sizeof(Fl_Menu_Item) * nCount);

    // Now translate the strings in the new menu
    for (i = 0; i < nCount; ++i) {
	if (pMenuItem2[i].text != NULL) {
	    // Use strdup so that FreeTranslatedMenu will work correctly
	    pMenuItem2[i].text = strdup(_(pMenuItem2[i].text));
	}
    }

    return (pMenuItem2);
}


//--------------------------------------------------------------//
// Translate menu items using gettext.  The original menu is    //
// non-const so the menu is not copied, only the strings are    //
// replaced.                                                    //
//--------------------------------------------------------------//
Fl_Menu_Item *
TranslateMenuItems(Fl_Menu_Item * pMenuItem)
{
    const char *pszTranslated;
    int i;
    int nCount /* = 0 */ ;

    // Count the number of menu items to be translated
    nCount = pMenuItem->size();

    // Now translate the strings in the menu
    for (i = 0; i < nCount; ++i) {
	if (pMenuItem[i].text != NULL) {
	    pszTranslated = _(pMenuItem[i].text);
	    free((char *) pMenuItem[i].text);

	    // Use strdup so that FreeTranslatedMenu will work correctly
	    pMenuItem[i].text = strdup(pszTranslated);
	}
    }

    return (pMenuItem);
}


//--------------------------------------------------------------//
// Get the first part of a line of text.                        //
//--------------------------------------------------------------//
string
WrapText(const char *pszText, int nMaxWidth, Fl_Widget * pWidget)
{
    static const char *pszEllipsis = "...";
    char *pszBufEnd;
    char *pszBuffer;
    char *pszBuffer2;
    char *pszBufStart;
    const char *pszEnd;
    const char *pszEnd2;
    char *pszOldBufEnd;
    int i;
    int nChr;
    int nFlHeight;
    int nHeight;
    int nLength;
    int nMax;
    int nWidth;
    string strReturn;

    // Initialize the font for measurements
    fl_font(pWidget->labelfont(), pWidget->labelsize());
    nFlHeight = fl_height() + fl_descent();

    // Get a new-line delimited block of characters
    pszEnd = strchr(pszText, '\n');
    if (pszEnd == NULL) {
	pszEnd = pszText + strlen(pszText);
    }
    pszEnd2 = strchr(pszText, '\r');
    if (pszEnd2 == NULL) {
	pszEnd2 = pszText + strlen(pszText);
    }
    if (pszEnd2 < pszEnd) {
	pszEnd = pszEnd2;
    }
    // Will this line fit into the length allowed
    nLength = pszEnd - pszText + 1;
    pszBuffer = new char[nLength];
    strncpy(pszBuffer, pszText, nLength - 1);
    pszBuffer[nLength - 1] = '\0';
    pszBufStart = pszBuffer;
    while (*pszBufStart == ' ') {
	++pszBufStart;
    }
    nWidth = nMaxWidth;
    fl_measure(pszBufStart, nWidth, nHeight);

    // Break out a smaller portion of the line if needed
    if (nHeight >= nFlHeight || nWidth > nMaxWidth) {
	// Get the width of an ellipsis that will be added to the end of the text
	fl_measure(pszEllipsis, nWidth, nHeight);
	if (nMaxWidth >= 2 * nWidth) {
	    nMaxWidth -= nWidth;
	}
	// Find a small enough portion of this line to fit
	pszBufEnd = pszBufStart;
	do {
	    pszOldBufEnd = pszBufEnd;
	    pszBufEnd = strchr(pszBufEnd + 1, ' ');
	    if (pszBufEnd == NULL) {
		// Too long a word at the end of the line
		break;
	    }
	    *pszBufEnd = '\0';
	    nWidth = nMaxWidth;
	    fl_measure(pszBufStart, nWidth, nHeight);
	    *pszBufEnd = ' ';
	} while (nHeight == nFlHeight && nWidth <= nMaxWidth);

	// Output the first portion of the remaining line
	if (pszOldBufEnd == pszBufStart) {
	    // Word too long, just output characters
	    nMax = strlen(pszBufStart) - 1;
	    nWidth = nMaxWidth + 1;
	    for (i = nMax;
		 i > 1 && (nHeight > nFlHeight || nWidth > nMaxWidth); --i) {
		nChr = pszBufStart[i];
		pszBufStart[i] = '\0';
		nWidth = nMaxWidth;
		fl_measure(pszBufStart, nWidth, nHeight);
		pszBufStart[i] = nChr;
	    }

	    // Get these characters and set up to return them
	    if (i < 0) {
		i = 0;
	    }
	    pszBuffer2 = new char[i + 3 + 1];
	    strncpy(pszBuffer2, pszBufStart, i);
	    pszBuffer2[i] = '\0';
	} else {
	    // Can break at white space (blank)
	    i = pszOldBufEnd - pszBufStart;
	    pszBuffer2 = new char[i + 3 + 1];
	    strncpy(pszBuffer2, pszBufStart, i);
	    while (i >= 0 && pszBuffer2[i] == ' ') {
		--i;
	    }
	    pszBuffer2[i] = '\0';
	}

	// Add an ellipsis to the end of the text
	strcat(pszBuffer2, pszEllipsis);
    } else {
	// Just use the buffer as is
	pszBuffer2 = new char[strlen(pszBuffer) + 1];
	strcpy(pszBuffer2, pszBuffer);
    }

    // Clean up
    delete[]pszBuffer;
    strReturn = pszBuffer2;
    delete[]pszBuffer2;
    return (strReturn);
}
