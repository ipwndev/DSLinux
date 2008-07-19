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
 * See http://embedded.centurysoftware.com/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://embedded.centurysoftware.com/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


#ifndef		NXTMINPUT_INCLUDED
#define		NXTMINPUT_INCLUDED	1

/* System header files */
#include <time.h>


/* Local header files */
#include <FL/Fl_Widget.H>
#include <FL/Fl_Input.H>
#include <nxapp.h>


/* Typedef, macros, enum/struct/unions definitions */
#define			MAX_UNITS		4	// Maximum number of units (HH:MM:SS:[am/pm])

//const char                            *dayzn[2] = {{"AM"}, {"PM"}};                   // Array of either am or pm
const short max_val[3] = { 23, 59, 59 };	// Max values per unit (23 = hours, 59 = min/sec)


/* Forward declarations */
class NxTmUnit;

/* Class definitions */
class NxTmInput:public Fl_Widget
{
  private:
    char _tmstring[2 + 1 + 2 + 1 + 2 + 1 + 2 + 1];	// String for time
    int _nunits;		// Actual number in use
    void draw();		// Over-ridden draw function

  public:
      NxTmUnit * Units[MAX_UNITS];	// Maximum number of units

    char *GetTime(void);	// Returns the time in a char *
    void GetTime(struct tm *);	// Returns the time in a tm struct
    int GetUnits()
    {
	return (_nunits);
    }				// Returns the number of units
    void SetUnits(int num);	// Sets the _nunits value
    void SetTime(char *tm);	// Set the time from a ##:##:## string
    void SetTime(struct tm *);	// Set the time from a tm struct

    void hide();		// Hides everything
    void show();		// Shows all

    // Constructor
    NxTmInput(int x, int y, int w, int h, char *l = 0, int nfld = 4);	// Default constructor
};				// end of NxTmInput class

class NxTmUnit:public Fl_Input
{
  private:
    NxTmInput * parent;		// Parent of this unit

  public:
    void SetParent(NxTmInput * par)
    {
	parent = par;
    }				// Sets the parent
    int handle(int);		// Over-ridden handle() event

    NxTmUnit(int x, int y, int w, int h, const char *l =
	     0):Fl_Input(x, y, w, h, l)
    {
    };
};				// end of NxTmUnit class

#endif //      NX_TMINPUTINLUCDED
