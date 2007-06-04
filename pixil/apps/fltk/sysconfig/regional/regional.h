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


#ifndef		NXREGIONAL_INCLUDED
#define		NXREGIONAL_INCLUDED	1

// System header files


// Local header files
#include <nxbox.h>
#include <nxbutton.h>
#include <nxmenubutton.h>
#include "nxreg.h"


// Typedefs, macros, enum/struct/union definitions

#define		UTILITY_PAR_NAME "regional"
#define		UTILITY_NAME	"Regional Settings"
#define		REG_NUM_TMDT	3	// Number of regional time/date fields
#define		REG_TMDT_TIME	0
#define		REG_TMDT_SDT	1
#define		REG_TMDT_LDT	2
#define		REG_NMBRS_PN	0
#define		REG_NMBRS_NN	1
#define		REG_NMBRS_PC	2
#define		REG_NMBRS_NC	3
#define		REG_NUM_NMBRS	4	// Number of regional number values fields

typedef struct
{
    char *label;		// Widget label (static)
    NxBox *nb;			// Widget
}
RegData_t;

// Global variables


// Class definition
class NxRegional
{
  public:
    // Constructor and destructor
    NxRegional(int X, int Y, int W, int H, char *appname);	// Class constructor
     ~NxRegional();		// Class destructor

    void ShowWindow(void);
    void HideWindow(void);

  private:
    int _nregidx,		// New regional idx
      _oregidx,			// Old regional idx
      _winX,			// Windows TL X coordinate
      _winY;			// Windows TL Y coordinate
    RegData_t _regtmdt[REG_NUM_TMDT],	// Number of time/date samples
      _regnmbrs[REG_NUM_NMBRS];	// Number of number samples

    // Fltk widgets/windows
    Fl_Group *_mainw;		// Windows
    NxButton *_reset,		// Reset button
     *_save;			// Save button
    NxMenuButton *_mbreg;	// Mode selection menu button

    // Private functions    
    void clean_locale_dir(void);
    void ColosseumLogger(int level, char *str, ...);	// Colosseum Logger function
    int FindLocale(char *key);	// Finds the index of the given locale
    void GetAppPrefs(void);	// Get the application prefrences
//              int is_locale_dir(char *lcd);
    void MakeWindow(int, int, int, int);	// Creates the window/widgets
    void SetAppPrefs(void);	// Stores the application preferences
    void SetNmbrs(int idx, char *value);	// Sets the _regnmbrs member correctly
    void SetTmdt(int idx, char *value);	// Sets the _regtmdt member correctly
    void SetValues(void);	// Set the values for the widgets based on
    void set_locale_dir(void);
    // the current state
    // Private static widget callbacks
    static void mb_cb(Fl_Widget * w, void *d);	// Call back for menu button
    static void save_reset_cb(Fl_Widget * w, void *d);	// Callback for save/reset buttons

};				// end of class NxRegional


#endif //      NXREGIONAL_INCLUDED
