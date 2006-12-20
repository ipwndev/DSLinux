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


#ifndef		NXCAL_INCLUDED
#define		NXCAL_INCLUDED	1

// System header files


// Local header files
#include <nxbutton.h>
#include "nxcal.h"
#include "sysconf.h"
#include <sysconfig.h>


// Typedefs, macros, enum/struct/union definitions
#define		UTILITY_PAR_NAME "screencal"
#define		UTILITY_NAME	"Screen Calibration"

typedef enum
{
    clNONINTER = 0,		// Non interactive mode
    clINTER			// Interactive mode
}
CalMode_t;

// Global variables


// Class definition
class NxCal
{
  public:
    // Constructor and destructor
    NxCal(int X, int Y, int W, int H, char *appname);	// Class constructor
     ~NxCal();			// Class destructor

    void ShowWindow(void);
    void HideWindow(void);

    bool InterMode(void)
    {
	return (_calmode == clNONINTER ? false : true);
    }
    char *GetTxtFile(void)
    {
	return (_txtfile);
    }
    int StartnxCal(void);

  private:
    CalMode_t _calmode;		// Calibration mode (either interactive/non)
    char *_nxcalpath,		// Path to the raw nx calibration binary
     *_txtfile;			// Textfile name to store data into
    int _winX,			// Windows TL X coordinate
      _winY;			// Windows TL Y coordinate

    // Fltk widgets/windows
    Fl_Group *_mainw;		// Windows
    NxButton *_proceed;		// Proceed button

    // Private functions    
    void GetAppData(void);	// Gets application data 
    void MakeWindow(int X, int Y, int W, int H);	// Creates the window/widgets

    // Private static widget callbacks
    static void cal_cb(Fl_Widget * w, void *d);	// Handles the calibration/exit functionality

};				// end of class NxBacklite


#endif //      NXCAL_INCLUDED
