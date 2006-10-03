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
//--------------------------------------------------------------//
#include "config.h"
#include <cassert>
#include <cstdio>
#include <ctime>
#include <vector>
#include "FLTKUtil.h"
#include "Options.h"
#include "Printer.h"
#include "TimeFunc.h"

#include "VCMemoryLeak.h"


using namespace std;


// The size of the font used in the page trailer
#define PAGE_TRAILER_FONT_SIZE  8
#define PAGE_TRAILER_FONT_SIZEX "8"


// Postscript macros
#ifdef POSTSCRIPT
const char *
    Printer::m_pszMacroBox =
    "/box { newpath 3 index 3 index moveto 1 index 0 rlineto 0 exch "
    "rlineto -1 mul 0 rlineto pop pop closepath } def";

const char *
    Printer::m_pszMacroBoxClip =
    "/boxclip { 3 index 3 index 3 index 3 index box clip } def";

const char *
    Printer::m_pszMacroBoxDraw =
    "/boxdraw { box stroke } def";

const char *
    Printer::m_pszMacroTextBottom =
    "/textbottom { gsave boxclip 4 index stringwidth pop 2 index "
    "sub 2 div 4 index exch sub exch pop exch pop exch fontheight 0.2 mul add moveto pop "
    "show grestore } def";

const char *
    Printer::m_pszMacroTextCenter =
    "/textcenter { gsave boxclip lineheight sub 2 div 2 index add "
    "exch 4 index stringwidth pop sub 2 div 3 index add exch fontheight 0.2 mul add moveto "
    "pop pop show grestore } def";

const char *
    Printer::m_pszMacroTextLeft =
    "/textleft { gsave boxclip lineheight sub 2 div exch pop add fontheight 0.2 mul add "
    "moveto show grestore } def";

const char *
    Printer::m_pszMacroTextLeftBottom =
    "/textleftbottom { gsave boxclip pop pop fontheight 0.2 mul add moveto show grestore } "
    "def";

const char *
    Printer::m_pszMacroTextLeftTop =
    "/textlefttop { gsave boxclip lineheight sub 2 index add exch "
    "pop exch pop moveto show grestore } def";

const char *
    Printer::m_pszMacroTextRight =
    "/textright { gsave boxclip lineheight sub 2 div 2 index add exch "
    "4 index stringwidth pop sub 3 index add exch fontheight 0.2 mul add moveto pop pop show "
    "grestore } def";

const char *
    Printer::m_pszMacroTextRightBottom =
    "/textrightbottom { gsave boxclip 4 index stringwidth pop 4 index "
    "exch sub 2 index add exch pop exch pop exch fontheight 0.2 mul add moveto pop show "
    "grestore } def";

const char *
    Printer::m_pszMacroTextRightTop =
    "/textrighttop { gsave boxclip lineheight sub 2 index add exch 4 "
    "index stringwidth pop sub 3 index add exch moveto pop pop show "
    "grestore } def";

const char *
    Printer::m_pszMacroTextTop =
    "/texttop { gsave boxclip lineheight sub 2 index add exch 4 "
    "index stringwidth pop sub 2 div 3 index add exch moveto pop pop "
    "show grestore } def";

// Macro to break a string containing a word too long into a part that will fit within a given length
// Requires string, length will leave remainder-of-string and first-part on stack
const char *
    Printer::m_pszMacroBreakWord =
    "/breakword\n"
    "{ /bwlength exch def /bwstring exch def 0 1 bwstring length\n"
    " {\n"
    "  dup /bwchar exch def bwstring exch 0 exch getinterval stringwidth pop bwlength gt\n"
    "  { exit }\n"
    "  if\n"
    " }\n"
    " for /bwchar bwchar 1 sub def bwstring bwchar bwstring length bwchar sub getinterval "
    "bwstring 0 bwchar getinterval\n"
    "} def";

// Macro to break a string so that it will fit within a length
// Requires string, length will leave remainder-of-string and first-part on stack
const char *
    Printer::m_pszMacroBreakString =
    "/breakstring\n"
    "{ /bslinelength exch def /bsstring exch def /bsbreakwidth ( ) stringwidth pop "
    "def /bsrestoftext bsstring def /bscurwidth 0 def /bslastwordbreak 0 def "
    "bsrestoftext stringwidth pop bslinelength gt\n"
    " {\n"
    "  { bsrestoftext ( ) search\n"
    "   { /bsnextword exch def pop /bsrestoftext exch def /bswordwidth bsnextword "
    "stringwidth pop def bscurwidth bswordwidth add bslinelength gt\n"
    "    { bslastwordbreak 0 ne\n"
    "     { bsstring bslastwordbreak bsstring length bslastwordbreak sub "
    "getinterval bsstring 0 bslastwordbreak 1 sub getinterval exit\n"
    "     }\n"
    "     { bsstring bslinelength breakword exit\n"
    "     }\n"
    "     ifelse\n"
    "    }\n"
    "    { /bscurwidth bscurwidth bswordwidth add bsbreakwidth add def\n"
    "    }\n"
    "    ifelse /bslastwordbreak bslastwordbreak bsnextword length add 1 add def\n"
    "   }\n"
    "   { pop bsrestoftext stringwidth pop bsbreakwidth add bscurwidth add bslinelength gt\n"
    "    { bslastwordbreak 0 ne\n"
    "     { bsrestoftext bsstring 0 bslastwordbreak 1 sub getinterval exit\n"
    "     }\n"
    "     { bsstring bslinelength breakword exit\n"
    "     }\n"
    "     ifelse\n"
    "    }\n"
    "    { () bsstring exit\n"
    "    }\n"
    "    ifelse\n"
    "   }\n"
    "   ifelse\n"
    "  }\n"
    "  loop\n"
    " }\n"
    " { () bsstring\n"
    " }\n"
    " ifelse\n"
    "} def";

// Requires array of times and titles,x,y,w,h,day of month,shading,current date
const char *
    Printer::m_pszMacroCalendarBox2 =
    "/calendarbox\n"
    "{\n"
    " /cbspace ( ) stringwidth pop def /cbcurrent exch def /cbshading exch def "
    "/cbday exch def /cbheight exch def /cbwidth exch def /cby exch def /cbx "
    "exch def gsave cbshading 255 div setgray cbx cby cbwidth cbheight rectfill "
    "grestore cbx cby cbwidth cbheight rectstroke /cbx cbx cbspace add def "
    "/cbwidth cbwidth cbspace 2 mul sub def fontheight rootfont cbcurrent 0 ne\n"
    " { /Times-Bold }\n"
    " { /Times-Roman }\n"
    " ifelse\n"
    " findfont fontheight 1.5 mul dup /fontheight exch def dup 1.2 mul "
    "/lineheight exch def scalefont setfont cbday 2 string cvs cbx cby cbheight "
    "add lineheight sub dup /cbcury exch def cbwidth lineheight textright "
    "/cbcury cbcury lineheight sub def setfont dup 1.2 mul /lineheight exch def "
    "/fontheight exch def /cbcount 0 def dup length 1 sub 1 1 3 -1 roll\n"
    " {\n"
    "  dup 1 eq\n"
    "  { columnspaceline }\n"
    "  if 1 index exch get dup 0 get fontheight /Times-Bold setupfont stringwidth "
    "pop fontheight /Times-Roman setupfont 1 index 1 get exch\n"
    "  {\n"
    "   cbwidth exch sub breakstring /cbcount cbcount 1 add def pop dup "
    "stringwidth pop 0 eq\n"
    "   { exit }\n"
    "   if 0\n"
    "  }\n"
    "  loop pop cbcount cbheight lineheight 2.5 mul sub 2 sub lineheight div "
    "floor le\n"
    "  {\n"
    "   dup 0 get cbx cbcury cbwidth lineheight fontheight /Times-Bold setupfont "
    "5 -1 roll dup stringwidth pop cbspace add exch 6 2 roll textleft fontheight "
    "/Times-Roman setupfont exch 1 get exch\n"
    "   {\n"
    "    dup 3 1 roll cbwidth exch sub breakstring 3 -1 roll dup cbx add cbcury 3 "
    "-1 roll cbwidth exch sub lineheight textleft /cbcury cbcury lineheight sub "
    "def dup length 0 eq\n"
    "    { exit }\n"
    "    if 0\n"
    "   }\n"
    "   loop pop\n"
    "  }\n"
    "  {\n"
    "   /coverflow coverflow 1 add def pop exit\n"
    "  }\n"
    "  ifelse\n"
    " }\n"
    " for cbcount cbheight lineheight 2.5 mul sub 2 sub lineheight div floor gt\n"
    " {\n"
    "  fontheight /Times-Bold setupfont (xmore...) dup stringwidth pop cbspace 2 "
    "mul add exch cbx cbwidth add 2 index sub cby 3 index lineheight textright "
    "cbx cbwidth add 1 index sub cby 3 -1 roll lineheight boxdraw\n"
    " }\n"
    " { pop }\n"
    " ifelse\n"
    "} def";

// Finish a calendar with overflow days
// Requires (arrays from calendarbox), title, right-title, shading, linelength, startx
const char *
    Printer::m_pszMacroCalendarEnd =
    "/calendarend\n"
    "{\n"
    " coverflow 0 gt\n"
    " {\n"
    "  10 /Times-Roman setupfont 2 index /ceshade exch def startcolumnmode coverflow "
    "array astore 0 1 2 index length 1 sub\n"
    "  {\n"
    "   dup 2 index exch get exch 0 eq\n"
    "   {\n"
    "    dup 0 get columnspaceline columnspaceline columnspaceline 15 INCH 5 "
    "mul 16 div ceshade columnbox\n"
    "   }\n"
    "   { columnnewline }\n"
    "   ifelse 1 1 2 index length 1 sub\n"
    "   {\n"
    "    1 index exch get dup 2 get fontheight /Times-Bold setupfont 0 "
    "columnshownoadvance 1 get fontheight /Times-Roman setupfont INCH 3 mul 2 "
    "div 0 columnshow\n"
    "   }\n"
    "   for pop\n"
    "  }\n"
    "  for pop endcolumnmode\n"
    " }\n"
    " { pop pop pop pop pop }\n"
    " ifelse\n"
    "} def";

// Start overflow mode for calendar boxes
const char *
    Printer::m_pszMacroCalendarStart =
    "/calendarstart { /coverflow 0 def } def";

// Requires horizontal margin, vertical margin
// Must be translated
const char *
    Printer::m_pszMacroPageTrailer2 =
    "/pagetrailer { fontheight rootfont "
    PAGE_TRAILER_FONT_SIZEX
    " /Times-Roman "
    "setupfont 3 index 3 index lineheight 3 mul 2 div add moveto 3 index "
    "2 mul 8.5 INCH mul exch sub 0 rlineto stroke pagetrailerstring 4 index 4 index "
    "4 INCH mul 7 index sub lineheight textleft pagenumber 1 add dup /pagenumber "
    "exch def 20 string cvs dup stringwidth pop 6 1 roll 4.25 INCH mul 4 index 4.25 "
    "INCH mul 6 index sub lineheight textright (%s) 4.5 INCH mul 5 -1 roll 4 INCH "
    "mul 8 -1 roll sub 7 -1 roll sub ( ) stringwidth pop sub lineheight textright "
    "setfont /fontheight exch dup 1.2 mul /lineheight exch def def } def";

// Macro to start two column mode
// Requires page heading, right heading, box shading, linelength and start x position within column
const char *
    Printer::m_pszMacroStartColumnMode =
    "/startcolumnmode { /collinestartx exch def /collinelength exch def /colheadershade "
    "exch def /colrightheading exch def /colpageheading exch def /collinestarty 9.5 INCH "
    "mul lineheight sub def /columnno 2 def startcolumn } def";

// Macro to set a font
// Requires font size, font face
const char *
    Printer::m_pszMacroSetupFont =
    "/setupfont { findfont exch dup 1.2 mul /lineheight exch def dup /fontheight exch "
    "def scalefont setfont } def";

// Macro to start a new column
const char *
    Printer::m_pszMacroStartColumn =
    "/startcolumn { columnno 1 add dup /columnno exch def 2 gt\n"
    " { /columnno 1 def fontheight rootfont 20 /Times-Bold setupfont colpageheading "
    "collinestartx dup 3 1 roll 10.5 INCH mul fontheight 3 mul 2 div sub dup 4 1 roll "
    "8.5 INCH mul collinestartx 2 mul sub dup 5 1 roll fontheight 3 mul 2 div dup 6 1 "
    "roll colheadershade drawbox colrightheading 5 1 roll 16 /Times-Bold setupfont "
    "exch lineheight 2 div sub exch textright setfont /fontheight exch dup 1.2 mul "
    "/lineheight def def\n"
    " }\n"
    " if /collinex collinestartx 4.25 INCH mul columnno 1 sub mul add def /colliney "
    "collinestarty def\n"
    "} def";

// Macro to end a page in two column mode
const char *
    Printer::m_pszMacroEndColumnPage =
    "/endcolumnpage { 0.5 INCH mul dup pagetrailer 4.25 INCH mul 0.5 "
    "INCH mul "
    PAGE_TRAILER_FONT_SIZEX
    " 3 mul 2 div add dup 3 1 roll "
    "moveto 0 exch 10 INCH mul exch sub rlineto stroke } def";

// Macro to end a column in two column mode
const char *
    Printer::m_pszMacroEndColumn =
    "/endcolumn { columnno 1 ne\n"
    " { endcolumnpage showpage }\n"
    " if startcolumn } def";

// Macro to end two column mode
const char *
    Printer::m_pszMacroEndColumnMode =
    "/endcolumnmode { endcolumnpage showpage } def";

// Macro to advance a line in two column mode
const char *
    Printer::m_pszMacroColumnNewLine =
    "/columnnewline { /colliney colliney lineheight sub def } def";

// Macro to leave a blank line in two column mode
const char *
    Printer::m_pszMacroColumnSpaceLine =
    "/columnspaceline { colliney collinestarty ne { columnnewline } if } def";

// Macro to break and show text within a column
// Requires string, offset, indent
const char *
    Printer::m_pszMacroColumnShow =
    "/columnshow { /csbindent exch def /csboffset exch def /csbtext exch def\n"
    " { csbtext collinelength csboffset sub csbindent sub dup /csblength "
    "exch def breakstring collinex csboffset add csbindent add colliney "
    "csblength lineheight textleft columnnewline colliney 0.5 INCH mul "
    "lineheight 5 mul 2 div add le\n"
    "  { endcolumn }\n"
    "  if /csbtext exch def /csbindent 0 def csbtext length 0 le\n"
    "  { exit }\n"
    "  if }\n"
    " loop } def";

// Macro to draw text within a column but not advance the horizontal position
// Requires string, offset
const char *
    Printer::m_pszMacroColumnShowNoAdvance =
    "/columnshownoadvance { /csboffset exch def /csbtext exch def csbtext "
    "collinelength csboffset sub dup /csblength exch def breakstring "
    "collinex csboffset add colliney csblength lineheight textleft pop } def";

// Macro to draw right justified text but not advance within columns
// Requires string, offset
const char *
    Printer::m_pszMacroColumnShowNoAdvanceRight =
    "/columnshownoadvanceright { /csboffset exch def /csbtext exch def csbtext "
    "collinelength csboffset sub dup /csblength exch def breakstring collinex "
    "colliney csblength lineheight textright pop } def";

// Macro to draw text in a shaded box in a column
// Requires text, fontheight, boxheight, shading
const char *
    Printer::m_pszMacroColumnBox =
    "/columnbox { exch /cbboxheight exch def colliney cbboxheight sub 0.5 INCH "
    "mul lineheight 5 mul 2 div add le\n"
    " { endcolumn }\n"
    " if\n"
    " 255 div setgray collinex colliney collinelength cbboxheight rectfill 0 "
    "setgray fontheight rootfont 3 -1 roll /Times-Bold setupfont 3 -1 roll "
    "collinex colliney collinelength cbboxheight textcenter setfont /fontheight "
    "exch dup 1.2 mul /lineheight def def /colliney colliney cbboxheight sub "
    "lineheight 2 div sub def } def";

// Macro to draw a shaded box with left justified text
// Requires text, x, y, width, height, shading
const char *
    Printer::m_pszMacroDrawBox =
    "/drawbox { 255 div setgray 3 index 3 index 3 index 3 index rectfill 0 "
    "setgray 4 -1 roll lineheight 2 div add 4 1 roll 2 -1 roll lineheight sub 2 "
    "1 roll textleft } def";

// Macro to draw a checkbox with or without a checkmark
// Requires margin, complete flag
const char *
    Printer::m_pszMacroColumnIconComplete =
    "/columniconcomplete { /cicomp exch def /cimargin exch def /ciratio 0.75 def "
    "/ciportion 0.4 def colliney lineheight sub 0.5 INCH mul lineheight 5 mul 2 div add le\n"
    "{ endcolumn }\n"
    "if collinex cimargin add colliney lineheight ciratio mul "
    "lineheight ciratio mul gsave 1 setlinewidth boxdraw 0 cicomp ne\n"
    "{ collinex cimargin add 1 add colliney lineheight ciratio ciportion mul mul add moveto "
    "collinex cimargin add lineheight ciratio ciportion mul mul add colliney 1 add lineto "
    "collinex cimargin add lineheight ciratio mul add 1 sub colliney fontheight ciratio mul "
    "add lineto stroke\n"
    "}\n"
    "if grestore } def";

// Array of pointers to postscript macros
const char *
    Printer::m_ppszMacro2[Macro_Max] = {
    m_pszMacroBox,
    m_pszMacroBoxClip,
    m_pszMacroBoxDraw,
    m_pszMacroTextBottom,
    m_pszMacroTextCenter,
    m_pszMacroTextLeft,
    m_pszMacroTextLeftBottom,
    m_pszMacroTextLeftTop,
    m_pszMacroTextRight,
    m_pszMacroTextRightBottom,
    m_pszMacroTextRightTop,
    m_pszMacroTextTop,
    m_pszMacroBreakWord,
    m_pszMacroBreakString,
    m_pszMacroCalendarBox2,	// This will be translated
    m_pszMacroCalendarEnd,
    m_pszMacroCalendarStart,
    m_pszMacroPageTrailer2,	// This will be translated
    m_pszMacroStartColumnMode,
    m_pszMacroStartColumn,
    m_pszMacroEndColumnPage,
    m_pszMacroEndColumn,
    m_pszMacroEndColumnMode,
    m_pszMacroColumnNewLine,
    m_pszMacroColumnSpaceLine,
    m_pszMacroColumnShow,
    m_pszMacroColumnShowNoAdvance,
    m_pszMacroColumnShowNoAdvanceRight,
    m_pszMacroColumnBox,
    m_pszMacroDrawBox,
    m_pszMacroColumnIconComplete,
};

// Inner macro requirements per each macro
const unsigned int
    Printer::m_nMacroDependencies[Macro_Max] = {
    0,				// box - no dependencies
    (1 << Macro_Box),		// boxclip requires box
    (1 << Macro_Box),		// boxdraw requires box
    (1 << Macro_Box) + (1 << Macro_BoxClip),	// textbottom requires boxclip which requires box
    (1 << Macro_Box) + (1 << Macro_BoxClip),	// textcenter requires boxclip which requires box
    (1 << Macro_Box) + (1 << Macro_BoxClip),	// textleft requires boxclip which requires box
    (1 << Macro_Box) + (1 << Macro_BoxClip),	// textleftbottom requires boxclip which requires box
    (1 << Macro_Box) + (1 << Macro_BoxClip),	// textlefttop requires boxclip which requires box
    (1 << Macro_Box) + (1 << Macro_BoxClip),	// textright requires boxclip which requires box
    (1 << Macro_Box) + (1 << Macro_BoxClip),	// textrightbottom requires boxclip which requires box
    (1 << Macro_Box) + (1 << Macro_BoxClip),	// textrighttop requires boxclip which requires box
    (1 << Macro_Box) + (1 << Macro_BoxClip),	// texttop requires boxclip which requires box
    0,				// breakword - no dependencies
    (1 << Macro_BreakWord),	// breakstring dependencies
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_BoxDraw) +
	(1 << Macro_TextLeft)
	+ (1 << Macro_TextRight) + (1 << Macro_BreakWord) + (1 << Macro_BreakString),	// calendarbox requirements
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_TextCenter) +
	(1 << Macro_TextLeft)
	+ (1 << Macro_TextRight) + (1 << Macro_BreakWord) +
	(1 << Macro_BreakString)
	+ (1 << Macro_DrawBox) + (1 << Macro_StartColumn) +
	(1 << Macro_StartColumnMode)
	+ (1 << Macro_EndColumnMode) + (1 << Macro_EndColumnPage) +
	(1 << Macro_EndColumn)
	+ (1 << Macro_ColumnSpaceLine) + (1 << Macro_ColumnShow) +
	(1 << Macro_ColumnShowNoAdvance)
	+ (1 << Macro_ColumnBox) + (1 << Macro_ColumnNewLine) + (1 << Macro_PageTrailer),	// calendarend dependencies
    0,				// calendarstart dependencies
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_TextLeft) + (1 << Macro_TextRight),	// Page Trailer requirements
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_TextLeft) +
	(1 << Macro_DrawBox)
	+ (1 << Macro_StartColumn),	// startcolumnmode requires startcolumn
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_TextLeft) +
	(1 << Macro_TextRight)
	+ (1 << Macro_DrawBox),	// startcolumn dependencies
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_TextLeft) +
	(1 << Macro_TextRight)
	+ (1 << Macro_PageTrailer),	// endcolumnpage requires page trailer
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_TextLeft) +
	(1 << Macro_TextRight)
	+ (1 << Macro_DrawBox) + (1 << Macro_EndColumnPage) +
	(1 << Macro_StartColumn)
	+ (1 << Macro_PageTrailer),	// endcolumn dependencies
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_TextLeft) +
	(1 << Macro_TextRight)
	+ (1 << Macro_EndColumnPage) + (1 << Macro_PageTrailer) + (1 << Macro_DrawBox),	// endcolumnmode dependencies
    0,				// columnnewline - no dependencies
    (1 << Macro_ColumnNewLine),	// columnspaceline dependencies
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_TextLeft) +
	(1 << Macro_TextRight)
	+ (1 << Macro_BreakWord) + (1 << Macro_BreakString) +
	(1 << Macro_EndColumn)
	+ (1 << Macro_EndColumnPage) + (1 << Macro_StartColumn) +
	(1 << Macro_PageTrailer)
	+ (Macro_ColumnNewLine) + (1 << Macro_DrawBox),	// columnshow dependencies
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_TextLeft) +
	(1 << Macro_BreakWord)
	+ (1 << Macro_BreakString),	// columnshownoadvance dependencies
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_TextRight) +
	(1 << Macro_BreakWord)
	+ (1 << Macro_BreakString),	// columnshownoadvanceright dependencies
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_TextCenter) +
	(1 << Macro_TextLeft)
	+ (1 << Macro_TextRight) + (1 << Macro_EndColumnPage) +
	(1 << Macro_StartColumn)
	+ (1 << Macro_PageTrailer) + (1 << Macro_EndColumn) + (1 << Macro_DrawBox),	// columnbox dependencies
    (1 << Macro_Box) + (1 << Macro_BoxClip) + (1 << Macro_TextLeft),	// drawbox dependencies
    (1 << Macro_Box) + (1 << Macro_BoxDraw),	// columniconcomplete dependencies
};
#endif


//--------------------------------------------------------------//
// Default constructor.                                         //
//--------------------------------------------------------------//
Printer::Printer()
{
    m_bInDoc = false;
    m_bInPage = false;
    m_bInColumnMode = false;

#ifdef WIN32			// Windows printing
    m_hDC = NULL;
    m_hBoldFont = NULL;
    m_hFont = NULL;
    m_hNormalFont = NULL;
    m_nFontSize = 0;
    m_nPitchFamily = 0;
    m_pszFontFace = NULL;
#endif

#ifdef POSTSCRIPT

    char *pszMore = _("more...");
    char *pszPage = _("Page");

    // Translate postscript macros
    memcpy(m_ppszMacro, m_ppszMacro2, Macro_Max * sizeof(char *));

    // The Calendar Box macro requires translation
    m_pszMacroCalendarBox =
	new char[strlen(m_pszMacroCalendarBox2) + strlen(pszMore) + 1];
    sprintf(m_pszMacroCalendarBox, m_pszMacroCalendarBox2, pszMore);
    m_ppszMacro[Macro_CalendarBox] = m_pszMacroCalendarBox;

    // The Page Trailer macro requires translation
    m_pszMacroPageTrailer =
	new char[strlen(m_pszMacroPageTrailer2) + strlen(pszPage) + 1];
    sprintf(m_pszMacroPageTrailer, m_pszMacroPageTrailer2, pszPage);
    m_ppszMacro[Macro_PageTrailer] = m_pszMacroPageTrailer;

#endif

}


//--------------------------------------------------------------//
// Destructor, will close the print job normally if needed.     //
//--------------------------------------------------------------//
Printer::~Printer()
{
    // Close any active document
    if (m_bInDoc) {
	Close();
    }
#ifdef POSTSCRIPT
    // Remove any translated macros
    delete[]m_ppszMacro[Macro_CalendarBox];
    delete[]m_ppszMacro[Macro_PageTrailer];
#endif
}


//--------------------------------------------------------------//
// Draw a day's box for a monthly calendar.  The bOverflow      //
// argument is only really used for Windows printing.  It will  //
// be set to true if the events overflowed the area for the     //
// calendar box.  For postscript printing this will be set to   //
// false because the postscript macros will take care of this   //
// type of overflow.                                            //
//--------------------------------------------------------------//
bool
Printer::CalendarBox(time_t nDate,
		     int nDayOfMonth,
		     int nShade,
		     bool bCurrentDay,
		     int nX,
		     int nY,
		     int nWidth,
		     int nHeight,
		     vector < string > &vShortTime,
		     vector < string > &vTime,
		     vector < string > &vDescription, bool & bOverflow)
{
    bool bReturn = true;
    char szData[48];
    int i;
    int nMax;

#ifdef WIN32			// Windows printing

    COLORREF crOldBk;
    const char *pszMore = _("more...");
    HBRUSH hBrush =::CreateSolidBrush(RGB(nShade, nShade, nShade));
    HBRUSH hOriginalBrush;
    HFONT hFontLarge;
    HFONT hOriginalFont;
    HPEN hOriginalPen;
    HPEN hPen = (HPEN)::GetStockObject(BLACK_PEN);
    int nCurY;
    int nHeightUsed;
    int nSpaceWidth;
    SIZE size;
    RECT rect;

    // Reset the overflow flag
    bOverflow = false;

    // Continue if no errors
    if (hBrush != NULL && hPen != NULL) {
	// Set the background color
	crOldBk =::SetBkColor(m_hDC, RGB(nShade, nShade, nShade));

	// Draw the box for the date
	hOriginalBrush = (HBRUSH)::SelectObject(m_hDC, hBrush);
	hOriginalPen = (HPEN)::SelectObject(m_hDC, hPen);
	Win32ToPoints(rect, nX, nY + nHeight, nWidth, nHeight);
	::Rectangle(m_hDC, rect.left, rect.top, rect.right, rect.bottom);

	// Draw in the day of the month
	hFontLarge =::CreateFont(-(3 * m_nFontSize) / 2,
				 0,
				 0,
				 0,
				 (bCurrentDay ? FW_BOLD : FW_NORMAL),
				 FALSE,
				 FALSE,
				 FALSE,
				 ANSI_CHARSET,
				 OUT_TT_PRECIS,
				 CLIP_DEFAULT_PRECIS,
				 PROOF_QUALITY,
				 VARIABLE_PITCH | FF_ROMAN,
				 "Times New Roman");
	hOriginalFont = (HFONT)::SelectObject(m_hDC, hFontLarge);
	sprintf(szData, "%d", nDayOfMonth);
	nCurY = nY + nHeight;
	Win32DrawText(nX,
		      nCurY - (3 * m_nFontSize) / 2,
		      nWidth - m_nFontSize / 2,
		      (3 * m_nFontSize) / 2, szData, FL_ALIGN_RIGHT);
	nCurY -= (3 * m_nFontSize) / 2;

	// Get the size of a space (used to further reduce the width of a line)
	Win32MakeNormalFont();
	::SelectObject(m_hDC, m_hNormalFont);
	::GetTextExtentPoint32(m_hDC, " ", 1, &size);
	nSpaceWidth = size.cx;

	// Output each appointment in turn
	nMax = vShortTime.size();
	for (i = 0; i < nMax; ++i) {
	    // Output this appointment
	    Win32WrapTextWithHeader(nX + nSpaceWidth, nCurY - m_nFontSize, nWidth - 2 * nSpaceWidth, m_nFontSize, nY + m_nFontSize + 2,	// 2 point fudge factor
				    vShortTime[i],
				    vDescription[i], nHeightUsed);
	    if (nHeightUsed == 0) {
		// Out of space, don't do any more
		break;
	    }
	    nCurY -= nHeightUsed;
	}

	// Is there more ?
	if (i < nMax) {
	    ::SelectObject(m_hDC, m_hBoldFont);
	    ::GetTextExtentPoint32(m_hDC, pszMore, strlen(pszMore), &size);
	    size.cx += 4;	// 4 points larger
	    size.cy += 2;	// 2 points larger
	    Win32ToPoints(rect, nX + nWidth - size.cx, nY + size.cy, size.cx,
			  size.cy);
	    ::Rectangle(m_hDC, rect.left, rect.top, rect.right, rect.bottom);
	    Win32DrawText(rect.left,
			  rect.top,
			  rect.right - rect.left,
			  m_nFontSize, pszMore, FL_ALIGN_TOP);
	    bOverflow = true;
	}
	// Reset the device context
	::SelectObject(m_hDC, hOriginalBrush);
	::SelectObject(m_hDC, hOriginalFont);
	::SelectObject(m_hDC, hOriginalPen);

	// Clean up
	::DeleteObject(hBrush);
	::DeleteObject(hFontLarge);
	//::DeleteObject(hPen); // This was a stock object

	// Reset the background color
	::SetBkColor(m_hDC, crOldBk);
    } else {
	// Failed to create GDI objects
	bReturn = false;
    }

#endif


#ifdef POSTSCRIPT		// Linux/unix postscript printing

    string strPostscript;

    // Output the array of arrays for times and titles of appointments
    strPostscript = "[ (";
    sprintf(szData,
	    "%s %s",::GetDayOfWeek(nDate).c_str(),::FormatDate(nDate).
	    c_str());
    strPostscript += szData;
    strPostscript += ") ";
    for (i = 0; i < nMax; ++i) {
	strPostscript += "[ (";
	strPostscript += PostscriptString(vShortTime[i].c_str());
	strPostscript += ") (";
	strPostscript += PostscriptString(vDescription[i].c_str());
	strPostscript += ") (";
	strPostscript += PostscriptString(vTime[i].c_str());
	strPostscript += ") ] ";
    }
    strPostscript += ']';

    // Output the position and size
    sprintf(szData, " %d", nX);
    strPostscript += szData;
    sprintf(szData, " %d", nY);
    strPostscript += szData;
    sprintf(szData, " %d", nWidth);
    strPostscript += szData;
    sprintf(szData, " %d", nHeight);
    strPostscript += szData;

    // Output day-of-month, shading and current day flag
    sprintf(szData, " %d", nDayOfMonth);
    strPostscript += szData;
    sprintf(szData, " %d", nShade);
    strPostscript += szData;
    sprintf(szData, " %d", bCurrentDay ? 1 : 0);
    strPostscript += szData;
    strPostscript += " calendarbox";

    // Add to the postscript program
    m_listPostscript.push_back(strPostscript);

    // Mark this macro as needed
    m_nRequiredMacros |=
	(1 << Macro_CalendarBox) + m_nMacroDependencies[Macro_CalendarBox];

#endif

    return (bReturn);
}


//--------------------------------------------------------------//
// Finish overflow processing for a calendar.  The three vector //
// arguments are only really used for Windows printing, the     //
// postscript macros take care of this otherwise.               //
//--------------------------------------------------------------//
bool
Printer::CalendarEnd(const char *pszTitle,
		     int nShading,
		     vector < time_t > &vDate,
		     vector < vector < string > >&vvTime,
		     vector < vector < string > >&vvDescription)
{
    bool bReturn = false;
    char szData[48];
    const int nBorder = INCH / 2;	// 1/2 inch border

    if (m_bInDoc == true) {

#ifdef WIN32			// Windows printing

	bool bFirst;
	int nDay;
	const int nFontSize = 10;
	int nMax = vDate.size();
	time_t nTime;
	vector < string > *pvDescription;
	vector < string > *pvTime;
	vector < string >::iterator iterDescription;
	vector < string >::iterator iterTime;

	// Are there any overflow days
	if (nMax > 0) {
	    // Go to two column mode
	    Win32SetSerifFont(nFontSize);
	    Win32SetTwoColumnMode(425 * INCH / 100 - 2 * nBorder,
				  nBorder, pszTitle, NULL, nShading);

	    for (nDay = 0; nDay < nMax; ++nDay) {
		// Print each visible entry for this day
		nTime = vDate[nDay];

		// Print each row
		pvTime = &vvTime[nDay];
		pvDescription = &vvDescription[nDay];
		for (iterDescription = pvDescription->begin(), iterTime =
		     pvTime->begin(), bFirst = true;
		     iterTime != pvTime->end();
		     ++iterTime, ++iterDescription) {
		    // Has the date changed (it never will, but the logic works)
		    if (bFirst == true) {
			sprintf(szData,
				"%s %s",::GetDayOfWeek(nTime).
				c_str(),::FormatDate(nTime).c_str());

			// Yes, output a box for the date
			Win32ColumnSpaceLine();
			Win32ColumnSpaceLine();
			Win32ColumnSpaceLine();
			Win32ColumnBox(szData, 15, 5 * INCH / 16, 184);
			bFirst = false;
		    } else {
			Win32ColumnNewLine();
		    }

		    // Output the time
		    Win32SetBoldSerifFont(nFontSize);
		    Win32ColumnShowNoAdvance(iterTime->c_str(), 0);

		    // Output the description
		    Win32SetSerifFont(nFontSize);
		    Win32ColumnShow(iterDescription->c_str(), (3 * INCH) / 2,
				    0);
		}
	    }

	    // End the page
	    Win32EndTwoColumnMode();
	}
#endif


#ifdef POSTSCRIPT		// Linux/unix postscript printing

	string strData;

	// Output a call to the calendarend macro
	strData = '(';
	strData += PostscriptString(pszTitle);
	strData += ") () ";
	sprintf(szData, "%d", nShading);
	strData += szData;
	strData += ' ';
	sprintf(szData, "%d", 425 * INCH / 100 - 2 * nBorder);
	strData += szData;
	strData += ' ';
	sprintf(szData, "%d", nBorder);
	strData += szData;
	strData += ' ';
	strData += "calendarend";
	m_listPostscript.push_back(strData);

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_CalendarEnd) +
	    m_nMacroDependencies[Macro_CalendarEnd];

#endif

    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Start overflow processing for a calendar.                    //
//--------------------------------------------------------------//
bool
Printer::CalendarStart(vector < time_t > &vDate,
		       vector < vector < string > >&vvTime,
		       vector < vector < string > >&vvDescription)
{
    bool bReturn = true;

    // Clear all vectors
    vDate.clear();
    vvTime.clear();
    vvDescription.clear();

    // Start the actual processing
    if (m_bInDoc == true) {

#ifdef WIN32			// Windows printing

	// Nothing special

#endif


#ifdef POSTSCRIPT		// Linux/unix postscript printing

	// Output a call to the calendarstart macro
	m_listPostscript.push_back(string("calendarstart"));

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_CalendarStart) +
	    m_nMacroDependencies[Macro_CalendarStart];

#endif

    }

    return (bReturn);		// No error checking yet
}


//--------------------------------------------------------------//
// Close the current print job                                  //
//--------------------------------------------------------------//
bool
Printer::Close()
{
    bool bReturn = false;

    if (m_bInDoc == true) {
	// End the current page if needed
	if (m_bInPage == true) {
	    EndPage();
	}
#ifdef WIN32			// Windows printing

	int nResult;

	// Issue an EndPage
	nResult =::EndDoc(m_hDC);
	bReturn = (nResult != 0);

#endif


#ifdef POSTSCRIPT		// Linux/unix postscript printing

	int i;
	int nLength;
	list < string >::iterator iter;
	string strData;

	// Write common macros to the file
	m_filePrint << "/INCH 72 def" << endl;
	m_filePrint << "/pagenumber 0 def" << endl;
	m_filePrint << "/pagetrailerstring (" << m_strPageTrailer.c_str()
	    << ") def" << endl;
	m_filePrint << m_pszMacroSetupFont	// Always include this macro
	    << endl;

	// Write any required macros to the file
	for (i = 0; i < Macro_Max; ++i) {
	    if (((m_nRequiredMacros >> i) & 0x01) != 0) {
		m_filePrint << m_ppszMacro[i]
		    << endl;
	    }
	}

	// Now write the postscript program to the file
	nLength = 0;
	for (iter = m_listPostscript.begin();
	     iter != m_listPostscript.end(); ++iter) {
	    if (nLength + (*iter).length() > 105) {
		m_filePrint << endl;
		nLength = 0;
	    } else if (nLength != 0) {
		m_filePrint << ' ';
		++nLength;
	    }
	    m_filePrint << (*iter).c_str();
	    nLength += (*iter).length();
	    if ((*iter) == "showpage") {
		m_filePrint << endl;
		nLength = 0;
	    }
	}

	// Close the file
	m_filePrint << endl;
	m_filePrint.close();

	// Remove the postscript program from memory
	m_listPostscript.clear();

	// Set the return code
	bReturn = true;

#endif

    }
    // Reset that a document is not started
    m_bInDoc = false;

    return (bReturn);
}


//--------------------------------------------------------------//
// Draw text within a shaded box in a column.                   //
//--------------------------------------------------------------//
bool
Printer::ColumnBox(const char *pszText,
		   int nFontSize, int nBoxHeight, int nShading)
{
    bool bReturn;

    if (m_bInColumnMode == true) {

#ifdef WIN32			// Windows printing

	bReturn = Win32ColumnBox(pszText, nFontSize, nBoxHeight, nShading);


#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	char szData[16];
	string strText;

	strText = '(';
	strText += PostscriptString(pszText);
	strText += ") ";
	sprintf(szData, "%d", nFontSize);
	strText += szData;
	strText += ' ';
	sprintf(szData, "%d", nBoxHeight);
	strText += szData;
	strText += ' ';
	sprintf(szData, "%d", nShading);
	strText += szData;
	strText += " columnbox";
	m_listPostscript.push_back(strText);

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_ColumnBox) + m_nMacroDependencies[Macro_ColumnBox];

	// Set the return code
	bReturn = true;

#endif

    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Draw a checkbox within a column.                             //
//--------------------------------------------------------------//
bool
Printer::ColumnIconComplete(int nMargin, bool bComplete)
{
    bool bReturn;

    if (m_bInColumnMode == true) {

#ifdef WIN32			// Windows printing

	HPEN penBlack =::CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	HPEN penOld;
	POINT point;

	// Go to a new column if needed
	if (Win32TestColumnEnd()) {
	    Win32EndColumn();
	}
	// Draw the basic box
	penOld = (HPEN)::SelectObject(m_hDC, penBlack);
	::MoveToEx(m_hDC, m_nColX + nMargin, m_nColY, &point);
	::LineTo(m_hDC, m_nColX + nMargin + (3 * m_nFontSize) / 4, m_nColY);
	::LineTo(m_hDC,
		 m_nColX + nMargin + (3 * m_nFontSize) / 4,
		 m_nColY + (3 * m_nFontSize) / 4);
	::LineTo(m_hDC, m_nColX + nMargin, m_nColY + (3 * m_nFontSize) / 4);
	::LineTo(m_hDC, m_nColX + nMargin, m_nColY);

	// Now draw the checkmark if needed
	if (bComplete == true) {
	    ::MoveToEx(m_hDC,
		       m_nColX + nMargin + 1,
		       m_nColY + (3 * m_nFontSize) / 10, &point);
	    ::LineTo(m_hDC,
		     m_nColX + nMargin + (3 * m_nFontSize / 10), m_nColY + 1);
	    ::LineTo(m_hDC,
		     m_nColX + nMargin + (3 * m_nFontSize) / 4 - 1,
		     m_nColY + (3 * m_nFontSize) / 4);
	}
	// Reset the DC
	::SelectObject(m_hDC, penOld);
	::DeleteObject(penBlack);

	bReturn = true;


#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	char szData[16];
	string strText;

	sprintf(szData, "%d", nMargin);
	strText = szData;
	strText += ' ';
	strText += (bComplete == true ? '1' : '0');
	strText += ' ';
	strText += "columniconcomplete";
	m_listPostscript.push_back(strText);

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_ColumnIconComplete) +
	    m_nMacroDependencies[Macro_ColumnIconComplete];

	// Set the return code
	bReturn = true;

#endif

    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Advance by a line within a column.                           //
//--------------------------------------------------------------//
bool
Printer::ColumnNewLine()
{
    bool bReturn;

    if (m_bInColumnMode == true) {

#ifdef WIN32			// Windows printing

	bReturn = Win32ColumnNewLine();

#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	string strText;

	strText = "columnnewline";
	m_listPostscript.push_back(strText);

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_ColumnNewLine) +
	    m_nMacroDependencies[Macro_ColumnNewLine];

	// Set the return code
	bReturn = true;

#endif

    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Show text broken into lines within a column.                 //
//--------------------------------------------------------------//
bool
Printer::ColumnShow(const char *pszText, int nMargin, int nIndent)
{
    bool bReturn;

    // Only print if in column mode
    if (m_bInColumnMode == true) {

#ifdef WIN32			// Windows printing

	bReturn = Win32ColumnShow(pszText, nMargin, nIndent);

#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	char szData[16];
	int i;
	int nMax;
	string strText;
	vector < string > vLine;

	// Break the string into lines
	BreakLines(pszText, vLine);

	// Process each line
	nMax = vLine.size();
	for (i = 0; i < nMax; ++i) {

	    if (vLine[i].length() > 0) {
		strText = '(';
		strText += PostscriptString(vLine[i].c_str());
		strText += ") ";
		sprintf(szData, "%d", nMargin);
		strText += szData;
		strText += ' ';
		sprintf(szData, "%d", nIndent);
		strText += szData;
		strText += " columnshow";
	    } else {
		strText = "columnspaceline";

		// Mark this macro as needed
		m_nRequiredMacros |=
		    (1 << Macro_ColumnSpaceLine) +
		    m_nMacroDependencies[Macro_ColumnSpaceLine];
	    }
	    m_listPostscript.push_back(strText);

	}

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_ColumnShow) + m_nMacroDependencies[Macro_ColumnShow];

	// Set the return code
	bReturn = true;

#endif

    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Show a single line of text within a column with no line      //
// advance.                                                     //
//--------------------------------------------------------------//
bool
Printer::ColumnShowNoAdvance(const char *pszText, int nMargin)
{
    bool bReturn;

    if (m_bInColumnMode == true) {

#ifdef WIN32			// Windows printing

	bReturn = Win32ColumnShowNoAdvance(pszText, nMargin);

#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	char szData[16];
	string strText;

	strText = '(';
	strText += PostscriptString(pszText);
	strText += ") ";
	sprintf(szData, "%d", nMargin);
	strText += szData;
	strText += " columnshownoadvance";
	m_listPostscript.push_back(strText);

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_ColumnShowNoAdvance) +
	    m_nMacroDependencies[Macro_ColumnShowNoAdvance];

	// Set the return code
	bReturn = true;

#endif

    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Show a single line of text right justified within a column   //
// with no line advance.                                        //
//--------------------------------------------------------------//
bool
Printer::ColumnShowNoAdvanceRight(const char *pszText, int nMargin)
{
    bool bReturn;

    if (m_bInColumnMode == true) {

#ifdef WIN32			// Windows printing

	vector < string > vText;

	// Get the lines of text to be displayed
	Win32BreakString(pszText, m_nColWidth - nMargin, 0, vText);

	// Output only the first line
	if (vText.size() > 0) {
	    if (Win32TestColumnEnd()) {
		Win32EndColumn();
	    }
	    Win32DrawText(m_nColX + nMargin,
			  m_nColY,
			  m_nColWidth - nMargin,
			  m_nFontSize, vText[0].c_str(), FL_ALIGN_RIGHT);
	}

	bReturn = true;

#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	char szData[16];
	string strText;

	strText = '(';
	strText += PostscriptString(pszText);
	strText += ") ";
	sprintf(szData, "%d", nMargin);
	strText += szData;
	strText += " columnshownoadvanceright";
	m_listPostscript.push_back(strText);

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_ColumnShowNoAdvanceRight) +
	    m_nMacroDependencies[Macro_ColumnShowNoAdvanceRight];

	// Set the return code
	bReturn = true;

#endif

    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Advance by a blank line within a column.                     //
//--------------------------------------------------------------//
bool
Printer::ColumnSpaceLine()
{
    bool bReturn;

    if (m_bInColumnMode == true) {

#ifdef WIN32			// Windows printing

	bReturn = Win32ColumnSpaceLine();

#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	string strText;

	strText = "columnspaceline";
	m_listPostscript.push_back(strText);

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_ColumnSpaceLine) +
	    m_nMacroDependencies[Macro_ColumnSpaceLine];

	// Set the return code
	bReturn = true;

#endif

    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Draw a shaded box with left justified text.                  //
//                                                              //
// The origin for coordinates is at the lower left of the page. //
// All coordinates are in points (1/72 of an inch).             //
// The nX,nY point is the lower left corner of the clipping     //
// box.  The upper right will be at point nX+nWidth,nY+nHeight. //
//--------------------------------------------------------------//
bool
Printer::DrawBox(int nX,
		 int nY,
		 int nWidth, int nHeight, int nShading, const char *pszText)
{
    bool bReturn;

    if (m_bInPage == true) {

#ifdef WIN32			// Windows printing

	bReturn = Win32DrawBox(nX, nY, nWidth, nHeight, nShading, pszText);

#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	char szData[16];
	string strText;

	strText = '(';
	strText += PostscriptString(pszText);
	strText += ") ";
	sprintf(szData, "%d", nX);
	strText += szData;
	strText += ' ';
	sprintf(szData, "%d", nY);
	strText += szData;
	strText += ' ';
	sprintf(szData, "%d", nWidth);
	strText += szData;
	strText += ' ';
	sprintf(szData, "%d", nHeight);
	strText += szData;
	strText += ' ';
	sprintf(szData, "%d", nShading);
	strText += szData;
	strText += " drawbox";
	m_listPostscript.push_back(strText);

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_DrawBox) + m_nMacroDependencies[Macro_DrawBox];

	// Set the return code
	bReturn = true;

#endif

    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Draw text on the page.  The values FL_ALIGN_INSIDE,          //
// FL_ALIGN_CLIP and FL_ALIGN_WRAP will be ignored.             //
//                                                              //
// The origin for coordinates is at the lower left of the page. //
// All coordinates are in points (1/72 of an inch).             //
// The nX,nY point is the lower left corner of the clipping     //
// box.  The upper right will be at point nX+nWidth,nY+nHeight. //
//--------------------------------------------------------------//
bool
Printer::DrawText(int nX,
		  int nY,
		  int nWidth,
		  int nHeight, const char *pszText, Fl_Align nAlign)
{
    bool bReturn = false;

    if (m_bInPage == true) {
#ifdef WIN32			// Windows printing

	bReturn = Win32DrawText(nX, nY, nWidth, nHeight, pszText, nAlign);

#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	char szData[16];
	int nMacro;
	string strData;
	string strMacro;

	// Output Postscript to print this text
	if ((nAlign & FL_ALIGN_LEFT) != 0) {
	    if ((nAlign & FL_ALIGN_TOP) != 0) {
		nMacro = Macro_TextLeftTop;
		strMacro = "textlefttop";
	    } else if ((nAlign & FL_ALIGN_BOTTOM) != 0) {
		nMacro = Macro_TextLeftBottom;
		strMacro = "textleftbottom";
	    } else {
		nMacro = Macro_TextLeft;
		strMacro = "textleft";
	    }
	} else if ((nAlign & FL_ALIGN_RIGHT) != 0) {
	    if ((nAlign & FL_ALIGN_TOP) != 0) {
		nMacro = Macro_TextRightTop;
		strMacro = "textrighttop";
	    } else if ((nAlign & FL_ALIGN_BOTTOM) != 0) {
		nMacro = Macro_TextRightBottom;
		strMacro = "textrightbottom";
	    } else {
		nMacro = Macro_TextRight;
		strMacro = "textright";
	    }
	} else {
	    if ((nAlign & FL_ALIGN_TOP) != 0) {
		nMacro = Macro_TextTop;
		strMacro = "texttop";
	    } else if ((nAlign & FL_ALIGN_BOTTOM) != 0) {
		nMacro = Macro_TextBottom;
		strMacro = "textbottom";
	    } else {
		nMacro = Macro_TextCenter;
		strMacro = "textcenter";
	    }
	}

	// Output the postscript needed to draw this string
	strData = '(';
	strData += PostscriptString(pszText);
	strData += ") ";
	sprintf(szData, "%d", nX);
	strData += szData;
	strData += ' ';
	sprintf(szData, "%d", nY);
	strData += szData;
	strData += ' ';
	sprintf(szData, "%d", nWidth);
	strData += szData;
	strData += ' ';
	sprintf(szData, "%d", nHeight);
	strData += szData;
	strData += ' ';
	strData += strMacro;
	m_listPostscript.push_back(strData);

	// Mark this macro as needed
	m_nRequiredMacros |= (1 << nMacro) + m_nMacroDependencies[nMacro];

	// Set the return code
	bReturn = true;
#endif
    }

    return (bReturn);
}


//--------------------------------------------------------------//
// End the current page.                                        //
//--------------------------------------------------------------//
bool
Printer::EndPage()
{
    bool bReturn = false;

    if (m_bInPage == true) {

#ifdef WIN32			// Windows printing

	int nResult;

	// Issue an EndPage
	nResult =::EndPage(m_hDC);
	bReturn = (nResult != 0);

#endif


#ifdef POSTSCRIPT		// Linux/unix postscript printing

	// Add a "showpage" to the current postscript
	m_listPostscript.push_back(string("showpage"));
	bReturn = true;

#endif

    }
    // Reset that a page is not started
    m_bInPage = false;

    return (bReturn);
}


//--------------------------------------------------------------//
// End two column mode.                                         //
//--------------------------------------------------------------//
bool
Printer::EndTwoColumnMode()
{
    bool bReturn;

    if (m_bInColumnMode == true) {

#ifdef WIN32			// Windows printing

	bReturn = Win32EndTwoColumnMode();

#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	string strText;

	strText = "endcolumnmode";
	m_listPostscript.push_back(strText);

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_EndColumnMode) +
	    m_nMacroDependencies[Macro_EndColumnMode];

	// Set the return code
	bReturn = true;

#endif

    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Open the print file                                          //
//--------------------------------------------------------------//
bool
Printer::Open(const char *pszFileName)
{
    bool bReturn = false;
    time_t nNow = time(NULL);

    // Set here to allow testing both print methods
    m_nCopies = 1;

#ifdef WIN32			// Windows version of the routine

    DOCINFO DocInfo;
    PRINTDLG printdlg;

    // Get the printer settings
    memset(&printdlg, 0, sizeof(printdlg));
    printdlg.lStructSize = sizeof(printdlg);
    printdlg.Flags =
	PD_ALLPAGES | PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC;
    bReturn = (::PrintDlg(&printdlg) != 0);

    // Continue if no errors
    if (bReturn == true) {
	DEVMODE *pDevmode = *((DEVMODE **) (printdlg.hDevMode));

	// Get the number of copies requested
	m_nCopies = printdlg.nCopies;

	// Get the paper size in points
	m_nPaperHeight = int ((pDevmode->dmPaperLength * 72.0) / 254.0);
	m_nPaperWidth = int ((pDevmode->dmPaperWidth * 72.0) / 254.0);

	// Get a device context for the selected printer
	m_hDC = printdlg.hDC;

	// Issue a StartDoc escape
	memset(&DocInfo, 0, sizeof(DocInfo));
	DocInfo.cbSize = sizeof(DocInfo);
	DocInfo.lpszDocName = pszFileName;
	::StartDoc(m_hDC, &DocInfo);
    }
    // Reset the page number
    m_nPageNo = 0;

#endif

#ifdef POSTSCRIPT		// Postscript version of the routine

    string strFileName = pszFileName;

    // Open an output file
    strFileName += ".ps";
    m_filePrint.open(strFileName.c_str());

    if (m_filePrint.is_open()) {
	// Set the paper size in points
	m_nPaperHeight = 11 * 72;
	m_nPaperWidth = (85 * 72) / 10;

	// Write the first line
	m_filePrint << "%!PS" << endl;

	// Clear the saved program
	m_listPostscript.clear();

	// Set that no macros are required just yet
	m_nRequiredMacros = 0;

	// Set the return code
	bReturn = true;
    }
#endif

    // Make up a page footing string
    m_strPageTrailer =::FormatDate(nNow);
    m_strPageTrailer += ' ';
    m_strPageTrailer +=::FormatTime(nNow);
#ifdef POSTSCRIPT
    m_strPageTrailer = PostscriptString(m_strPageTrailer.c_str());
#endif

    // Set whether a document has been started or not
    m_bInDoc = bReturn;

    return (bReturn);
}


//--------------------------------------------------------------//
// Output a page trailer.                                       //
//--------------------------------------------------------------//
bool
Printer::PageTrailer(int nHMargin, int nVMargin)
{
    bool bReturn = false;
    int nFontSize = m_nFontSize;

    if (m_bInPage == true) {
	if (nFontSize != 8) {
	    SetSerifFont(8);
	}
#ifdef WIN32			// Windows printing

	bReturn = Win32PageTrailer(nHMargin, nVMargin);

#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	char szData[16];
	string strData;

	// Output the postscript needed to draw this string
	sprintf(szData, "%d", nHMargin);
	strData = szData;
	strData += ' ';
	sprintf(szData, "%d", nVMargin);
	strData += szData;
	strData += " pagetrailer";
	m_listPostscript.push_back(strData);

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_PageTrailer) +
	    m_nMacroDependencies[Macro_PageTrailer];

	// Set the return code
	bReturn = true;
#endif

	if (nFontSize != 8) {
	    SetSerifFont(nFontSize);
	}
    }

    return (bReturn);
}


#ifdef POSTSCRIPT		// Linux/unix Postscript printing properties
//--------------------------------------------------------------//
// Make a string safe for postscript.                           //
//--------------------------------------------------------------//
string
Printer::PostscriptString(const char *pszText)
{
    string strReturn;

    // Examine each character
    while (*pszText != '\0') {
	if (*pszText == '\b'
	    || *pszText == '\f'
	    || *pszText == '\n' || *pszText == '\r' || *pszText == '\t') {
	    // Ignore these characters rather than allow formatting changes
	    strReturn += ' ';
	} else if (*pszText == '\\' || *pszText == '(' || *pszText == ')') {
	    strReturn += '\\';
	    strReturn += *pszText;
	} else if (*pszText >= ' ' && *pszText <= 0x7e) {
	    strReturn += *pszText;
	} else {
	    // Illegal character, ignore it
	    strReturn += '.';
	}

	// Go to the next character
	++pszText;
    }

    // Trim leading spaces
    while (strReturn.length() > 0 && strReturn[0] == ' ') {
	strReturn = strReturn.substr(1, strReturn.length() - 1);
    }

    // Trim trailing spaces
    while (strReturn.length() > 0 && strReturn[strReturn.length() - 1] == ' ') {
	strReturn = strReturn.substr(0, strReturn.length() - 1);
    }

    return (strReturn);
}
#endif


//--------------------------------------------------------------//
// Reset the page number.                                       //
//--------------------------------------------------------------//
void
Printer::ResetPageNumber()
{
    // Continue if in a document
    if (m_bInDoc == true) {
#ifdef WIN32			// Windows printing

	// Reset the page number
	m_nPageNo = 0;

#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	m_listPostscript.push_back(string("/pagenumber 0 def"));

#endif

    }
}


//--------------------------------------------------------------//
// Set the font to be used for text.                            //
//--------------------------------------------------------------//
bool
Printer::SetFont(const char *pszFontName, int nFontSize)
{
    bool bReturn = false;
    static const char *pszFont[2] = {
	"Times-Roman",
	"Times-Bold",
    };
    int nFontIndex;

    // Get the index for this font name
    for (nFontIndex = 0; nFontIndex < 2; ++nFontIndex) {
	if (strcasecmp(pszFontName, pszFont[nFontIndex]) == 0) {
	    bReturn = true;
	    break;
	}
    }

    // Continue if the font was found
    if (bReturn == true) {
#ifdef WIN32			// Windows printing

	// These arrays must be in the same order as pszFont above
	static const char *pszFace[2] = {
	    "Times New Roman",
	    "Times New Roman",
	};
	static const int nWeight[2] = {
	    FW_NORMAL,
	    FW_BOLD,
	};
	static const int nPitchFamily[2] = {
	    FF_ROMAN | VARIABLE_PITCH,
	    FF_ROMAN | VARIABLE_PITCH,
	};

	// Unselect any older font
	if (m_hFont != NULL) {
	    Win32DeselectFont();
	}
	// Create a new font
	bReturn = Win32SetFont(pszFace[nFontIndex],
			       nWeight[nFontIndex],
			       nPitchFamily[nFontIndex], nFontSize);

#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	char szData[16];
	string strData;

	// Add a postscript sequence to set the "fontheight" macro
	sprintf(szData, "%d", nFontSize);
	strData = szData;
	strData += " /";
	strData += pszFontName;
	strData += " setupfont";
	m_listPostscript.push_back(strData);
	bReturn = true;

#endif

    }

    if (bReturn == true) {
	// Set the font size
	m_nFontSize = nFontSize;
    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Start two column mode.                                       //
//--------------------------------------------------------------//
bool
Printer::SetTwoColumnMode(int nLineLength,
			  int nStartX,
			  const char *pszTitle,
			  const char *pszRightTitle, int nHeaderShade)
{
    bool bReturn;

    if (m_bInPage == false) {

#ifdef WIN32			// Windows printing

	bReturn = Win32SetTwoColumnMode(nLineLength,
					nStartX,
					pszTitle,
					pszRightTitle, nHeaderShade);

#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing

	char szData[16];
	string strText;

	strText = '(';
	strText += PostscriptString(pszTitle);
	strText += ") (";
	if (pszRightTitle != NULL) {
	    strText += PostscriptString(pszRightTitle);
	}
	strText += ") ";
	sprintf(szData, "%d", nHeaderShade);
	strText += szData;
	strText += ' ';
	sprintf(szData, "%d", nLineLength);
	strText += szData;
	strText += ' ';
	sprintf(szData, "%d", nStartX);
	strText += szData;
	strText += " startcolumnmode";
	m_listPostscript.push_back(strText);

	// Mark this macro as needed
	m_nRequiredMacros |=
	    (1 << Macro_StartColumnMode) +
	    m_nMacroDependencies[Macro_StartColumnMode];

	// Set the return code
	bReturn = true;

#endif

    }

    return (bReturn);
}


//--------------------------------------------------------------//
// Start a new page.                                            //
//--------------------------------------------------------------//
bool
Printer::StartPage()
{
    bool bReturn;

    if (m_bInDoc == true) {
	if (m_bInPage == true) {
	    EndPage();
	}
#ifdef WIN32			// Windows printing
	bReturn = (::StartPage(m_hDC) != 0);

	// Get the printing offsets
	m_nOffsetX =
	    (72 *::GetDeviceCaps(m_hDC, PHYSICALOFFSETX)) /::
	    GetDeviceCaps(m_hDC, LOGPIXELSX);
	m_nOffsetY =
	    (72 *::GetDeviceCaps(m_hDC, PHYSICALOFFSETY)) /::
	    GetDeviceCaps(m_hDC, LOGPIXELSY);

	// Set the mapping mode to points (1/72 of an inch)
	::SetMapMode(m_hDC, MM_ANISOTROPIC);
	::SetWindowOrgEx(m_hDC, m_nOffsetX, m_nPaperHeight + m_nOffsetY - INCH / 2, NULL);	// Half inch offset from somewhere
	::SetWindowExtEx(m_hDC, 72, 72, NULL);
	::SetViewportOrgEx(m_hDC, 0, 0, NULL);
	::SetViewportExtEx(m_hDC,::GetDeviceCaps(m_hDC, LOGPIXELSX),
			   -::GetDeviceCaps(m_hDC, LOGPIXELSY), NULL);
#endif

#ifdef POSTSCRIPT		// Linux/unix postscript printing
	// Do nothing
	bReturn = true;
#endif

    } else {
	bReturn = false;
    }

    // Set whether a page has been started or not
    m_bInPage = bReturn;

    return (bReturn);
}


#ifdef WIN32
//--------------------------------------------------------------//
// Break a string into pieces that will fit within a given line //
// length.                                                      //
//--------------------------------------------------------------//
void
Printer::Win32BreakString(const char *pszText,
			  int nLineLength,
			  int nIndent, vector < string > &vText)
{
    bool bFirstTooLong = false;
    int nLastPos;
    int nPos;
    int nWidthLeft;
    SIZE size;
    string strLine;
    string strText;

    // Clear the returned vector of strings
    vText.clear();

    // Get the lines of non-bold text
    strText = pszText;
    while (isspace(strText[0])) {
	strText = strText.substr(1);
    }

    // Process a non-blank string
    if (strText.length() > 0) {
	// Is the text too long for the width
	nWidthLeft = nLineLength - nIndent;
	::GetTextExtentPoint32(m_hDC, strText.c_str(), strText.length(),
			       &size);
	if (size.cx > nWidthLeft) {
	    // Get each line of text
	    while (strText.length() > 0) {
		// Test if the remaining string will fit or not
		::GetTextExtentPoint32(m_hDC, strText.c_str(),
				       strText.length(), &size);
		if (size.cx >= nWidthLeft) {
		    // Test each space to see if it delimits a string too long for the width left
		    nPos = 0;
		    nLastPos = 0;
		    while (nPos < strText.length()) {
			if (isspace(strText[nPos])) {
			    strLine = strText.substr(0, nPos);
			    ::GetTextExtentPoint32(m_hDC, strLine.c_str(),
						   strLine.length(), &size);
			    if (nWidthLeft < size.cx) {
				// Too long for this area
				nPos = nLastPos;
				break;
			    } else {
				nLastPos = nPos;
			    }
			}
			// Go try the next character
			++nPos;
		    }

		    // Test if past the end of the string looking for a space
		    if (nPos >= strText.length()) {
			nPos = nLastPos;
		    }
		} else {
		    // The entire remaining string will fit
		    nPos = strText.length();
		}

		// Was the first word too long
		if (nPos == 0) {
		    // On the first line
		    if (vText.size() == 0) {
			bFirstTooLong = true;
			strLine = "";
		    } else {
			// Get characters up to the end of the width
			nPos = 0;
			do {
			    strLine = strText.substr(0, ++nPos);
			    ::GetTextExtentPoint32(m_hDC, strLine.c_str(),
						   strLine.length(), &size);
			} while (nWidthLeft > size.cx);

			// Reduce the size to be taken
			if (nPos > 1) {
			    --nPos;
			}
		    }

		    // Get this string
		    strLine = strText.substr(0, nPos);
		} else {
		    strLine = strText.substr(0, nPos);
		}
		vText.push_back(strLine);

		// Prepare for the next loop
		nWidthLeft = nLineLength;
		if (bFirstTooLong == false) {
		    if (nPos < strText.length()) {
			strText = strText.substr(nPos);
			while (isspace(strText[0])) {
			    strText = strText.substr(1);
			}
		    } else {
			strText = "";
		    }
		} else {
		    bFirstTooLong = false;
		}
	    }
	} else {
	    // Only one string needed
	    vText.push_back(strText);
	}
    } else {
	// Blank string
	vText.push_back(strText);
    }
}


//--------------------------------------------------------------//
// Draw text within a shaded box in a column.                   //
//--------------------------------------------------------------//
bool
Printer::Win32ColumnBox(const char *pszText,
			int nFontSize, int nBoxHeight, int nShading)
{
    COLORREF color = RGB(nShading, nShading, nShading);
    COLORREF oldcolor;
    HBRUSH brushGray =::CreateSolidBrush(color);
    HBRUSH brushOld;
    HPEN penGray =::CreatePen(PS_SOLID, 1, color);
    HPEN penOld;
    int nFontSize2;

    // Test if at the end of a column
    if (m_nColY - nBoxHeight <= (3 * nFontSize) / 2 + INCH / 2) {
	Win32EndColumn();
    }
    // Draw the rectangle
    brushOld = (HBRUSH)::SelectObject(m_hDC, brushGray);
    penOld = (HPEN)::SelectObject(m_hDC, penGray);
    ::Rectangle(m_hDC,
		m_nColX,
		m_nColY + nBoxHeight, m_nColX + m_nColWidth, m_nColY);
    ::SelectObject(m_hDC, brushOld);
    ::SelectObject(m_hDC, penOld);

    // Draw the text
    nFontSize2 = m_nFontSize;
    Win32SetBoldSerifFont(nFontSize);
    oldcolor =::SetBkColor(m_hDC, color);
    Win32DrawText(m_nColX,
		  m_nColY, m_nColWidth, nBoxHeight, pszText, FL_ALIGN_CENTER);
    ::SetBkColor(m_hDC, oldcolor);
    Win32SetSerifFont(nFontSize2);

    // Set the new vertical position
    m_nColY -= nBoxHeight + m_nFontSize / 2;

    return (true);		// No error checing yet
}


//--------------------------------------------------------------//
// Show text broken into lines within a column.                 //
//--------------------------------------------------------------//
bool
Printer::Win32ColumnShow(const char *pszText, int nMargin, int nIndent)
{
    int i;
    int nMax;
    vector < string > vLine;
    int nLine;
    vector < string > vText;

    // Break the string into lines
    BreakLines(pszText, vLine);

    // Process each line
    nMax = vLine.size();
    for (i = 0; i < nMax; ++i) {

	// Get the lines of text to be displayed
	Win32BreakString(vLine[i].c_str(), m_nColWidth - nMargin, nIndent,
			 vText);

	// Output each line
	for (nLine = 0; nLine < vText.size(); ++nLine) {
	    if (vText[nLine].length() > 0) {
		if (Win32TestColumnEnd()) {
		    Win32EndColumn();
		}
		Win32DrawText(m_nColX + nMargin + (nLine == 0 ? nIndent : 0),
			      m_nColY,
			      m_nColWidth - nMargin - (nLine ==
						       0 ? nIndent : 0),
			      m_nFontSize, vText[nLine].c_str(),
			      FL_ALIGN_LEFT);
		m_nColY -= GetLineSpacing();
	    } else {
		// Just space down a line
		Win32ColumnSpaceLine();
	    }
	}
    }
    return (true);		// No error checking yet
}


//--------------------------------------------------------------//
// Show a single line of text within a column with no line      //
// advance.                                                     //
//--------------------------------------------------------------//
bool
Printer::Win32ColumnShowNoAdvance(const char *pszText, int nMargin)
{
    vector < string > vText;

    // Get the lines of text to be displayed
    Win32BreakString(pszText, m_nColWidth - nMargin, 0, vText);

    // Output only the first line
    if (vText.size() > 0) {
	if (Win32TestColumnEnd()) {
	    Win32EndColumn();
	}
	Win32DrawText(m_nColX + nMargin,
		      m_nColY,
		      m_nColWidth - nMargin,
		      m_nFontSize, vText[0].c_str(), FL_ALIGN_LEFT);
    }
    return (true);		// No error checking yet
}


//--------------------------------------------------------------//
// Space down a blank line.                                     //
//--------------------------------------------------------------//
bool
Printer::Win32ColumnSpaceLine()
{
    // Test if at the end of a column
    if (Win32TestColumnEnd()) {
	// Just end the column
	Win32EndColumn();
    } else if (m_nColY != m_nColStartY) {
	// Move down if not at the top
	m_nColY -= GetLineSpacing();
    }
    return (true);		// No error checking yet
}


//--------------------------------------------------------------//
// Unselect any custom font.                                    //
//--------------------------------------------------------------//
void
Printer::Win32DeselectFont()
{
    // Deselect the current font
    if (m_hFont != NULL) {
	// Reset to the original font
	::SelectObject(m_hDC, m_hOldFont);

	// Reset the current font pointer
	m_hFont = NULL;
    }
    // Destroy the bold font
    if (m_hBoldFont != NULL) {
	// Destroy it
	::DeleteObject(m_hBoldFont);

	// Delete the custom font
	m_hBoldFont = NULL;
    }
    // Destroy the normal font
    if (m_hNormalFont != NULL) {
	// Destroy it
	::DeleteObject(m_hNormalFont);

	// Delete the custom font
	m_hNormalFont = NULL;
    }
}


//--------------------------------------------------------------//
// Draw a shaded box with text on the page.                     //
//                                                              //
// The nX,nY point is the lower left corner of the clipping     //
// box.  The upper right will be at point nX+nWidth,nY+nHeight. //
//                                                              //
// This method exists so that other methods can call its        //
// functionality without also producing Postscript code when    //
// both output methods are active for for testing purposes.     //
//--------------------------------------------------------------//
bool
Printer::Win32DrawBox(int nX,
		      int nY,
		      int nWidth,
		      int nHeight, int nShading, const char *pszText)
{
    COLORREF color = RGB(nShading, nShading, nShading);
    COLORREF oldcolor;
    HBRUSH brushGray =::CreateSolidBrush(color);
    HBRUSH brushOld;
    HPEN penGray =::CreatePen(PS_SOLID, 1, color);
    HPEN penOld;

    // Draw the rectangle
    brushOld = (HBRUSH)::SelectObject(m_hDC, brushGray);
    penOld = (HPEN)::SelectObject(m_hDC, penGray);
    ::Rectangle(m_hDC, nX, nY + nHeight, nX + nWidth, nY);
    ::SelectObject(m_hDC, brushOld);
    ::SelectObject(m_hDC, penOld);

    // Draw the text
    oldcolor =::SetBkColor(m_hDC, color);
    Win32DrawText(nX + m_nFontSize / 2,
		  nY, nWidth - m_nFontSize, nHeight, pszText, FL_ALIGN_LEFT);
    ::SetBkColor(m_hDC, oldcolor);

    return (true);		// No error checking yet
}


//--------------------------------------------------------------//
// Draw text on the page.  The values FL_ALIGN_INSIDE,          //
// FL_ALIGN_CLIP and FL_ALIGN_WRAP will be ignored.             //
//                                                              //
// The origin for coordinates is at the lower left of the page. //
// All coordinates are in points (1/72 of an inch).             //
// The nX,nY point is the lower left corner of the clipping     //
// box.  The upper right will be at point nX+nWidth,nY+nHeight. //
//                                                              //
// This method exists so that other methods can call its        //
// functionality without also producing Postscript code when    //
// both output methods are active for for testing purposes.     //
//--------------------------------------------------------------//
bool
Printer::Win32DrawText(int nX,
		       int nY,
		       int nWidth,
		       int nHeight, const char *pszText, Fl_Align nAlign)
{
    bool bReturn = false;

    if (m_bInPage == true) {
	int nResult;
	RECT rect;

	// Set the rectangle for the output (convert to Points)
	rect.top = nY + nHeight;
	rect.bottom = nY - m_nFontSize / 5;	// Extra space for windows fonts
	rect.left = nX;
	rect.right = nX + nWidth;

	// Output the text
	nResult =::DrawText(m_hDC,
			    Win32String(pszText).c_str(),
			    -1,
			    &rect,
			    ((nAlign & FL_ALIGN_TOP) !=
			     0 ? DT_TOP : ((nAlign & FL_ALIGN_BOTTOM) !=
					   0 ? DT_BOTTOM : DT_VCENTER))
			    | ((nAlign & FL_ALIGN_LEFT) !=
			       0 ? DT_LEFT : ((nAlign & FL_ALIGN_RIGHT) !=
					      0 ? DT_RIGHT : DT_CENTER))
			    | DT_NOPREFIX | DT_SINGLELINE);
	bReturn = (nResult != 0);
    }

    return (bReturn);
}


//--------------------------------------------------------------//
// End a column (WIN32 specific).                               //
//--------------------------------------------------------------//
bool
Printer::Win32EndColumn()
{
    if (m_nColumnNumber != 1) {
	Win32EndColumnPage();
	Win32StartPage();
    }
    Win32StartColumn();
    return (true);		// No error checking yet
}


//--------------------------------------------------------------//
// End a page in 2 column mode column (WIN32 specific).         //
//--------------------------------------------------------------//
bool
Printer::Win32EndColumnPage()
{
    POINT point;

    ::MoveToEx(m_hDC, 425 * INCH / 100,
	       INCH / 2 + (3 * PAGE_TRAILER_FONT_SIZE) / 2, &point);
    ::LineTo(m_hDC, 425 * INCH / 100,
	     95 * INCH / 10 + (3 * PAGE_TRAILER_FONT_SIZE) / 2);
    return (Win32EndPage());
}


//--------------------------------------------------------------//
// End a page (WIN32 specific).                                 //
//--------------------------------------------------------------//
bool
Printer::Win32EndPage()
{
    Win32PageTrailer(INCH / 2, INCH / 2);
    ::EndPage(m_hDC);
    m_bInPage = false;
    return (true);		// No error checking yet
}


//--------------------------------------------------------------//
// End two column mode.                                         //
//--------------------------------------------------------------//
bool
Printer::Win32EndTwoColumnMode()
{
    Win32EndColumnPage();
    m_bInColumnMode = false;
    return (true);		// No error checking yet
}


//--------------------------------------------------------------//
// Advance by a line within a column.                           //
//--------------------------------------------------------------//
bool
Printer::Win32ColumnNewLine()
{
    // Test if at the end of a column
    if (Win32TestColumnEnd()) {
	// Just end the column
	Win32EndColumn();
    } else {
	// Move down a line
	m_nColY -= GetLineSpacing();
    }
    return (true);		// No error checking yet
}


//--------------------------------------------------------------//
// Make a bold font of the requested weight.                    //
//--------------------------------------------------------------//
bool
Printer::Win32MakeBoldFont()
{
    if (m_hBoldFont == NULL) {
	m_hBoldFont = Win32MakeFont(FW_BOLD);
    }
    return (m_hBoldFont != NULL);
}


//--------------------------------------------------------------//
// Make a font of the requested weight.                         //
//--------------------------------------------------------------//
HFONT
Printer::Win32MakeFont(int nWeight)
{
    HFONT hFont;

    // Create a new font
    hFont =::CreateFont(-m_nFontSize,
			0,
			0,
			0,
			nWeight,
			FALSE,
			FALSE,
			FALSE,
			ANSI_CHARSET,
			OUT_TT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			PROOF_QUALITY, m_nPitchFamily, m_pszFontFace);
    return (hFont);
}


//--------------------------------------------------------------//
// Make a normal font of the requested weight.                  //
//--------------------------------------------------------------//
bool
Printer::Win32MakeNormalFont()
{
    if (m_hNormalFont == NULL) {
	m_hNormalFont = Win32MakeFont(FW_NORMAL);
    }
    return (m_hNormalFont != NULL);
}


//--------------------------------------------------------------//
// Output a page trailer (WIN32 specific).                      //
//--------------------------------------------------------------//
bool
Printer::Win32PageTrailer(int nHMargin, int nVMargin)
{
    char szData[32];
    HPEN hOriginalPen;
    HPEN hPen = (HPEN)::GetStockObject(BLACK_PEN);
    int nFontSize = m_nFontSize;
    int nFontWeight = m_nFontWeight;
    POINT point;

    // Draw the line above the trailer
    hOriginalPen = (HPEN)::SelectObject(m_hDC, hPen);
    ::MoveToEx(m_hDC, nHMargin, nVMargin + PAGE_TRAILER_FONT_SIZE, &point);
    ::LineTo(m_hDC, m_nPaperWidth - nHMargin,
	     nVMargin + PAGE_TRAILER_FONT_SIZE);
    ::SelectObject(m_hDC, hOriginalPen);

    // Output the text on the bottom left of the page
    Win32SetSerifFont(8);
    Win32DrawText(nHMargin,
		  nVMargin,
		  4 * INCH - nHMargin,
		  PAGE_TRAILER_FONT_SIZE,
		  m_strPageTrailer.c_str(), FL_ALIGN_TOP_LEFT);

    // Output the text on the bottom right of the page
    sprintf(szData, "%s %d", _("Page"), ++m_nPageNo);
    Win32DrawText(45 * INCH / 10,
		  nVMargin,
		  40 * INCH / 10 - nHMargin,
		  PAGE_TRAILER_FONT_SIZE, szData, FL_ALIGN_TOP_RIGHT);
    Win32SetFont("Times New Roman", nFontWeight, VARIABLE_PITCH | FF_ROMAN,
		 nFontSize);

    return (true);
}


//--------------------------------------------------------------//
// WIN32 specific code to set a serif font current.             //
//--------------------------------------------------------------//
bool
Printer::Win32SetBoldSerifFont(int nFontSize)
{
    return (Win32SetFont
	    ("Times New Roman", FW_BOLD, VARIABLE_PITCH | FF_ROMAN,
	     nFontSize));
}


//--------------------------------------------------------------//
// WIN32 specific code to set a font current.                   //
//--------------------------------------------------------------//
bool
Printer::Win32SetFont(const char *pszFace,
		      int nWeight, int nPitchFamily, int nFontSize)
{

    // Unselect any older font
    if (m_hFont != NULL) {
	Win32DeselectFont();
    }
    // Create a new font
    m_nFontSize = nFontSize;
    m_nPitchFamily = nPitchFamily;
    m_pszFontFace = pszFace;
    m_hFont = Win32MakeFont(nWeight);

    if (m_hFont != NULL) {
	// Select this font
	m_hOldFont = (HFONT)::SelectObject(m_hDC, m_hFont);
    }
    // Save the font handle in the correct place
    if (nWeight == FW_BOLD) {
	m_hBoldFont = m_hFont;
    } else {
	m_hNormalFont = m_hFont;
    }

    // Save the font weight
    m_nFontWeight = nWeight;

    return (m_hFont != NULL);
}


//--------------------------------------------------------------//
// WIN32 specific code to set a serif font current.             //
//--------------------------------------------------------------//
bool
Printer::Win32SetSerifFont(int nFontSize)
{
    return (Win32SetFont
	    ("Times New Roman", FW_NORMAL, FF_ROMAN | VARIABLE_PITCH,
	     nFontSize));
}


//--------------------------------------------------------------//
// Start two column mode.                                       //
//--------------------------------------------------------------//
bool
Printer::Win32SetTwoColumnMode(int nLineLength,
			       int nStartX,
			       const char *pszTitle,
			       const char *pszRightTitle, int nHeaderShade)
{
    m_nColWidth = nLineLength;
    m_nColStartX = nStartX;
    m_nColStartY = 95 * INCH / 10 - m_nFontSize;
    m_nColumnNumber = 0;
    m_bInColumnMode = true;
    m_strTitle = pszTitle;
    m_strRightTitle = (pszRightTitle != NULL ? pszRightTitle : "");
    m_nHeaderShade = nHeaderShade;
    Win32StartColumn();
    return (true);		// No error checking yet
}


//--------------------------------------------------------------//
// WIN32 specific code to start the next column.                //
//--------------------------------------------------------------//
void
Printer::Win32StartColumn()
{
    COLORREF crBkColor;
    int nFontSize;
    int nFontWeight;
    const int nHeaderFontSize = 20;
    const int nRightFontSize = 14;

    if (++m_nColumnNumber != 2) {
	m_nColumnNumber = 1;
    }
    m_nColX = m_nColStartX + ((m_nColumnNumber - 1) * 425 * INCH) / 100;
    m_nColY = m_nColStartY;
    if (m_nColumnNumber == 1) {
	StartPage();
	nFontSize = m_nFontSize;
	nFontWeight = m_nFontWeight;
	Win32SetBoldSerifFont(nHeaderFontSize);
	Win32DrawBox(m_nColStartX,
		     105 * INCH / 10 - (3 * nHeaderFontSize) / 2,
		     85 * INCH / 10 - 2 * m_nColStartX,
		     (3 * nHeaderFontSize) / 2,
		     m_nHeaderShade, m_strTitle.c_str());
	Win32SetBoldSerifFont(nRightFontSize);
	crBkColor =::SetBkColor(m_hDC,
				RGB(m_nHeaderShade, m_nHeaderShade,
				    m_nHeaderShade));
	Win32DrawText(m_nColStartX,
		      105 * INCH / 10 - (3 * nHeaderFontSize) / 2,
		      85 * INCH / 10 - 2 * m_nColStartX - m_nFontSize / 2,
		      (3 * nHeaderFontSize) / 2, m_strRightTitle.c_str(),
		      FL_ALIGN_RIGHT);
	::SetBkColor(m_hDC, crBkColor);
	Win32SetFont("Times New Roman", nFontWeight,
		     FF_ROMAN | VARIABLE_PITCH, nFontSize);
    }
}


//--------------------------------------------------------------//
// Remove special characters from a string for Windows.         //
//--------------------------------------------------------------//
string
Printer::Win32String(const char *pszText)
{
    string strReturn;

    // Examine each character
    while (*pszText != '\0') {
	if (*pszText >= ' ' && *pszText <= 0x7e) {
	    strReturn += *pszText;
	} else {
	    // Illegal character, ignore it
	    strReturn += ' ';
	}

	// Go to the next character
	++pszText;
    }

    // Trim leading spaces
    while (strReturn.length() > 0 && strReturn[0] == ' ') {
	strReturn = strReturn.substr(1, strReturn.length() - 1);
    }

    // Trim trailing spaces
    while (strReturn.length() > 0 && strReturn[strReturn.length() - 1] == ' ') {
	strReturn = strReturn.substr(0, strReturn.length() - 1);
    }

    return (strReturn);
}


//--------------------------------------------------------------//
// WIN32 specific code to convert a rectangle to the Points     //
// mapping mode in use by the printer DC.                       //
//--------------------------------------------------------------//
void
Printer::Win32ToPoints(RECT & rect, int nX, int nY, int nWidth, int nHeight)
{
    rect.top = nY - nHeight;
    rect.bottom = nY;
    rect.left = nX;
    rect.right = nX + nWidth;
}


//--------------------------------------------------------------//
// WIN32 specific code to output wrapped text with a bold       //
// header.  Separate from Postscript code so that this can be   //
// called without generating any postscript code when both      //
// types of output are turned on for testing purposes.          //
//--------------------------------------------------------------//
bool
Printer::Win32WrapTextWithHeader(int nX,
				 int nY,
				 int nWidth,
				 int nHeight,
				 int nLimit,
				 const string & strHeader,
				 const string & strText, int &nHeightUsed)
{
    HFONT hOriginalFont;
    int nHeaderHeight;
    int nHeaderWidth;
    int nLine;
    SIZE size;
    string strHeader2;
    vector < string > vLine;

    // Make up both bold and normal fonts
    Win32MakeBoldFont();
    Win32MakeNormalFont();

    // Break the text into lines based on the width of the area
    // Get the length of the bold text
    hOriginalFont = (HFONT)::SelectObject(m_hDC, m_hBoldFont);
    strHeader2 = strHeader + ' ';
    ::GetTextExtentPoint32(m_hDC, strHeader2.c_str(), strHeader2.length(),
			   &size);
    nHeaderWidth = size.cx;
    nHeaderHeight = size.cy;

    // Get the lines of non-bold text
    Win32BreakString(strText.c_str(), nWidth, nHeaderWidth, vLine);

    // Only output if there is room for all
    nHeightUsed = (vLine.size() > 0 ? vLine.size() : 1) * nHeaderHeight;
    if (nY - nLimit > nHeightUsed) {
	// Output the bold header (assume that no wrapping is possible)
	::SelectObject(m_hDC, m_hBoldFont);
	Win32DrawText(nX, nY, nWidth, nHeight, strHeader.c_str(),
		      FL_ALIGN_LEFT);

	// Output the first line of non-bold text
	if (vLine.size() > 0) {
	    ::SelectObject(m_hDC, m_hNormalFont);
	    Win32DrawText(nX + nHeaderWidth,
			  nY,
			  nWidth - nHeaderWidth,
			  nHeight, vLine[0].c_str(), FL_ALIGN_LEFT);

	    // Output remaining lines of text
	    for (nLine = 1; nLine < vLine.size(); ++nLine) {
		Win32DrawText(nX,
			      nY - nLine * nHeaderHeight,
			      nWidth,
			      nHeight, vLine[nLine].c_str(), FL_ALIGN_LEFT);
	    }
	}
    } else {
	nHeightUsed = 0;
    }

    // Reset the DC
    ::SelectObject(m_hDC, hOriginalFont);

    return (true);		// No error reporting yet...
}
#endif
