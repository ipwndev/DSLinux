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
// Printing routines for WIN32 and Postscript based Linux/Unix  //
//                                                              //
// These printing routines are intended to be used as follows:  //
//                                                              //
// 1) Call the Open method.  This will identify the printer     //
//    from settings in the INI file.  Under WIN32 it will open  //
//    a device context for printing.  Under Linux/Unix it will  //
//    start an ostrstream object for the Postscript code and    //
//    set up a list of macros to be included in the output      //
//    file.                                                     //
// 2) Call StartPage to begin a page.  This will issue the      //
//    StartPage escape for WIN32 and do nothing for Linux/unix. //
// 3) Call the various drawing routines as needed for the page. //
// 4) Call EndPage to end this page.  This will issue an        //
//    EndPage escape for WIN32 and a Postscript showpage for    //
//    Linux/Unix.                                               //
// 5) Repeat steps 2 through 4 for each subsequent page.        //
// 6) Call the Close method.  This will issue an EndDoc escape  //
//    in WIN32 and close the device context.  For Linux/unix it //
//    will merge the required macro definitions into the        //
//    Postscript and write the file to disk.                    //
//                                                              //
// All positions are in points or 1/72 of an inch.              //
//--------------------------------------------------------------//
#ifndef PRINT_H_

#define PRINT_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <fstream>
#include <list>
#include <map>
#include <string>
#include <FL/Fl.H>
#include <FL/x.H>
using namespace std;

// Turn POSTSCRIPT printing on or off
#ifdef WIN32
#undef max			// Prevent problems later on
#define strcasecmp stricmp	// M/S names this differently
#undef POSTSCRIPT
#else /*  */
#define POSTSCRIPT
#endif /*  */

// Positioning by points
#define INCH 72
class Printer
{
  public:Printer();		// Default constructor
    ~Printer();			// Destructor, will close the print job normally if needed
    bool CalendarBox(time_t nDate,	// Draw a day's box on a monthly calendar
		     int nDayOfMonth, int nShade, bool bCurrentDay,
		     int nX, int nY, int nWidth, int nHeight,
		     vector < string > &vShortTime,
		     vector < string > &vTime,
		     vector < string > &vDescription, bool & bOverflow);
    bool CalendarEnd(const char *pszTitle,	// Finish overflow processing for calendar boxes
		     int nShading, vector < time_t > &vDate,
		     vector < vector < string > >&vvTime,
		     vector < vector < string > >&vvDescription);
    bool CalendarStart(vector < time_t > &vDate,	// Start overflow processing for calendar boxes
		       vector < vector < string > >&vvTime,
		       vector < vector < string > >&vvDescription);
    bool Close();		// Close the current print job
    bool ColumnBox(const char *pszText,	// Draw a shaded box in two column mode
		   int nFontHeight, int nBoxHeight, int nShading);
    bool ColumnIconComplete(int nMargin,	// Output a complete/incomplete icon (checkbox with or w/o a check)
			    bool bComplete);
    bool ColumnNewLine();	// Advance a line in two column mode
    bool ColumnShow(const char *pszText,	// Show text in two column mode
		    int nMargin, int nIndent);
    bool ColumnShowNoAdvance(const char *pszText,	// Show text in two column mode without advancing a line
			     int nMargin);
    bool ColumnShowNoAdvanceRight(const char *pszText,	// Show right justified text in two column mode without advancing a line
				  int nMargin);
    bool ColumnSpaceLine();	// Advance a blank line in two column mode
    bool DrawBox(int nX,	// Draw a shaded box with left justified text
		 int nY, int nWidth, int nHeight, int nShading,
		 const char *pszText);
    bool DrawText(int nX,	// Draw text
		  int nY, int nWidth, int nHeight, const char *pszText,
		  Fl_Align nAlign);
    bool EndPage();		// End the current page
    bool EndTwoColumnMode();	// End the two column printing mode
    inline int GetCopies() const	// Get the number of copies requested
    {
	return (m_nCopies);
    }
    inline int GetFontSize() const	// Get the current font size
    {
	return (m_nFontSize);
    }
    inline int GetHeight() const	// Get the current paper height (Points)
    {
	return (m_nPaperHeight);
    }
    inline int GetLineSpacing() const	// Get a good value for line spacing
    {
	return (m_nFontSize + m_nFontSize / 10);
    }
    inline int GetWidth() const	// Get the current paper width (Points)
    {
	return (m_nPaperWidth);
    }
    bool Open(const char *pszFileName);	// Open the print file
    bool PageTrailer(int nHMargin,	// Output a page trailer
		     int nVMargin);
    void ResetPageNumber();	// Reset the page number to page 1
    inline bool SetBoldSerifFont(int nFontSize)	// Set a bold serif font
    {
	return (SetFont("Times-Bold", nFontSize));
    }
    bool SetFont(const char *pszFontName,	// Set the font to be used for text
		 int nFontSize);
    inline bool SetSerifFont(int nFontSize)	// Set a serif font
    {
	return (SetFont("Times-Roman", nFontSize));
    }
    bool SetTwoColumnMode(int nLinelength,	// Start up two column mode
			  int nStartX, const char *pszTitle,
			  const char *pszRightTitle, int nHeaderShade);
    bool StartPage();		// Start a new page
  private:bool m_bInDoc;	// A document has been started
    bool m_bInColumnMode;	// Running in two column mode
    bool m_bInPage;		// A page has been started
    int m_nCopies;		// Number of copies requested
    int m_nFontSize;		// Last font size loaded
    int m_nPaperHeight;		// The height of the paper
    int m_nPaperWidth;		// The width of the paper
    string m_strPageTrailer;	// String on left of page trailer

#ifdef WIN32			// Windows printing properties
    const char *m_pszFontFace;	// Current font face
    HDC m_hDC;			// Device context
    HFONT m_hBoldFont;		// A bold font at the current size
    HFONT m_hNormalFont;	// A normal font at the current size
    HFONT m_hFont;		// The currently selected font
    HFONT m_hOldFont;		// The original font from the display context
    int m_nColStartX;		// Starting X offset within the column
    int m_nColStartY;		// Starting Y offset for a column
    int m_nColumnNumber;	// The current column number
    int m_nColWidth;		// The usable width of a column
    int m_nColX;		// Two-column vertical position
    int m_nColY;		// Two-column horizontal position
    int m_nFontWeight;		// Current font weight
    int m_nHeaderShade;		// Shading of the page header for 2-column mode
    int m_nOffsetX;		// X offset for printing
    int m_nOffsetY;		// Y offset for printing
    int m_nPageNo;		// The current page number
    int m_nPitchFamily;		// Current font pitch and family
    string m_strRightTitle;	// Right part of the page title
    string m_strTitle;		// The page title for 2-column mode
    void Win32BreakString(const char *pszText,	// Break a string into portions that will fit on a line
			  int nLineLength, int nIndent,
			  vector < string > &vText);
    bool Win32ColumnBox(const char *pszText,	// Draw a shaded box in two column mode
			int nFontHeight, int nBoxHeight, int nShading);
    bool Win32ColumnNewLine();	// Advance a line in two column mode
    bool Win32ColumnShow(const char *pszText,	// Show text in two column mode
			 int nMargin, int nIndent);
    bool Win32ColumnShowNoAdvance(const char *pszText,	// Show text in two column mode without advancing a line
				  int nMargin);
    bool Win32ColumnSpaceLine();	// Space down a blank line
    void Win32DeselectFont();	// Unselect any custom font from the display context
    bool Win32DrawBox(int nX,	// Draw a shaded box with left justified text
		      int nY, int nWidth, int nHeight, int nShading,
		      const char *pszText);
    bool Win32DrawText(int nX,	// Draw text (WIN32 specific)
		       int nY, int nWidth, int nHeight,
		       const char *pszText, Fl_Align nAlign);
    bool Win32EndColumn();	// End the current column in two column mode
    bool Win32EndColumnPage();	// End the current page in two column mode
    bool Win32EndPage();	// End the current page
    bool Win32EndTwoColumnMode();	// End the two column printing mode
    bool Win32MakeBoldFont();	// Create a bold font at the correct size
    HFONT Win32MakeFont(int nWeight);	// Create a font of the requested weight
    bool Win32MakeNormalFont();	// Make a normal font at the correct size
    bool Win32PageTrailer(int nHMargin,	// Output a page trailer (WIN32 specific)
			  int nVMargin);
    bool Win32SetBoldSerifFont(int nFontSize);	// Set a bold serif font
    bool Win32SetFont(const char *pszFace,	// Set a Windows font
		      int nWeight, int nPitchFamily, int nFontSize);
    bool Win32SetSerifFont(int nFontSize);	// Set a serif font
    bool Win32SetTwoColumnMode(int nLinelength,	// Start up two column mode
			       int nStartX, const char *pszTitle,
			       const char *pszRightTitle, int nHeaderShade);
    void Win32StartColumn();	// Start a new column
    inline bool Win32StartPage()	// Start another page
    {
	return (::StartPage(m_hDC) > 0);
    }
    string Win32String(const char *pszText);	// Remove special characters from a string
    void Win32ToPoints(RECT & rect,	// Convert a rectangle to the Points mapping mode
		       int nX, int nY, int nWidth, int nHeight);
    inline bool Win32TestColumnEnd()	// Test if at the end of a column
    {
	return (m_nColY <= (3 * m_nFontSize) / 2 + INCH / 2);
    }
    bool Win32WrapTextWithHeader(int nX,	// Wrap text with bold headers within an area (WIN32 specific)
				 int nY, int nWidth, int nHeight,
				 int nLimit, const string & strHeader,
				 const string & strText, int &nLinesUsed);

#endif /*  */

#ifdef POSTSCRIPT		// Linux/unix Postscript printing properties
    enum Macros			// Index numbers for each required macro
    { Macro_Box = 0, Macro_BoxClip, Macro_BoxDraw, Macro_TextBottom, Macro_TextCenter, Macro_TextLeft, Macro_TextLeftBottom, Macro_TextLeftTop, Macro_TextRight, Macro_TextRightBottom, Macro_TextRightTop, Macro_TextTop, Macro_BreakWord, Macro_BreakString, Macro_CalendarBox, Macro_CalendarEnd, Macro_CalendarStart, Macro_PageTrailer, Macro_StartColumnMode, Macro_StartColumn, Macro_EndColumnPage, Macro_EndColumn, Macro_EndColumnMode, Macro_ColumnNewLine, Macro_ColumnSpaceLine, Macro_ColumnShow, Macro_ColumnShowNoAdvance, Macro_ColumnShowNoAdvanceRight, Macro_ColumnBox, Macro_DrawBox, Macro_ColumnIconComplete, Macro_Max,	// Highest assigned macro index
    };
    char *m_ppszMacro[Macro_Max];	// Array of pointers to translated postscript macros
    static const char *m_ppszMacro2[Macro_Max];	// Array of pointers to postscript macros
    static const char *m_pszMacroBox;	// Macro to create a rectangular path
    static const char *m_pszMacroBoxClip;	// Macro to set clipping to a box
    static const char *m_pszMacroBoxDraw;	// Macro to draw a box
    static const char *m_pszMacroBreakString;	// Macro to break the part of string off of a larger string
    static const char *m_pszMacroBreakWord;	// Macro to break a string when the first word is too large for the width
    char *m_pszMacroCalendarBox;	// Macro to print a day's box for a monthly calendar (with translation of "more...")
    static const char *m_pszMacroCalendarBox2;	// Macro to print a day's box for a monthly calendar (needs translation of "more...")
    static const char *m_pszMacroCalendarEnd;	// Macro to print all days which overflowed their respective calendar boxes
    static const char *m_pszMacroCalendarStart;	// Macro to start tracking all calendar boxes which overflow their bounds
    static const char *m_pszMacroColumnBox;	// Macro to draw a shaded box with text in column mode
    static const char *m_pszMacroColumnNewLine;	// Macro to advance a line in two-column mode
    static const char *m_pszMacroColumnShow;	// Macro to show text within a column in two column mode
    static const char *m_pszMacroColumnShowNoAdvance;	// Macro to show text without advancing within a column in two column mode
    static const char *m_pszMacroColumnShowNoAdvanceRight;	// Macro to show right justified text without advancing within a column in two column mode
    static const char *m_pszMacroColumnSpaceLine;	// Macro to advance a blank line in two column mode
    static const char *m_pszMacroDrawBox;	// Macro to end a column in two column mode
    static const char *m_pszMacroEndColumn;	// Macro to end a column in two column mode
    static const char *m_pszMacroEndColumnMode;	// Macro to end two column mode
    static const char *m_pszMacroEndColumnPage;	// Macro to end a page in two column mode
    static const char *m_pszMacroColumnIconComplete;	// Macro to draw a checkbox
    char *m_pszMacroPageTrailer;	// Macro to print a page trailer (with translation of "Page")
    static const char *m_pszMacroPageTrailer2;	// Macro to print a page trailer (needs translation of "Page")
    static const char *m_pszMacroSetupFont;	// Macro to set a font and other related values
    static const char *m_pszMacroStartColumn;	// Macro to start a new column
    static const char *m_pszMacroStartColumnMode;	// Macro to start two column mode
    static const char *m_pszMacroTextBottom;	// Macro to place text centered at the bottom of a box
    static const char *m_pszMacroTextCenter;	// Macro to center text in a box
    static const char *m_pszMacroTextLeft;	// Macro to place text left but centered vertically in a box
    static const char *m_pszMacroTextLeftBottom;	// Macro to place text at the bottom left of a box
    static const char *m_pszMacroTextLeftTop;	// Macro to place text at the top left of a box
    static const char *m_pszMacroTextRight;	// Macro to place text at the right but vertically centered in a box
    static const char *m_pszMacroTextRightBottom;	// Macro to place text at the bottom right of a box
    static const char *m_pszMacroTextRightTop;	// Macro to place test at the top right of a box
    static const char *m_pszMacroTextTop;	// Macro to center text at the top of a box
    static const unsigned int m_nMacroDependencies[Macro_Max];	// Macro dependencies
    unsigned int m_nRequiredMacros;	// Bitmask of required postscript macros
    ofstream m_filePrint;	// The printer output file
    list < string > m_listPostscript;	// The body of the postscript program
    string PostscriptString(const char *pszText);	// Make a string legal for postscript

#endif /*  */
};


#endif /*  */
