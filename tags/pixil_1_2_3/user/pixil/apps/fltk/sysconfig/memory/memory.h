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



#ifndef		NXMEMORY_INCLUDED
#define		NXMEMORY_INCLUDED	1

// System header files


// Local header files
#include <nxmenubutton.h>
#include <nxbox.h>
#include <nxoutput.h>
#include <nxslider.h>
#include "nxmem.h"
#include "sysconf.h"
#include <sysconfig.h>


// Typedefs, macros, enum/struct/union definitions

#define		UTILITY_NAME	"Memory Info"
#define		UTILITY_PAR_NAME "memory"

typedef struct
{
    NxBox *used,		// Output widget for used
     *free,			// Output widget for free
     *total;			// Output widget for total
    char *u_str,		// Ptr to the string for used
     *f_str,			// Ptr to the string for free
     *t_str;			// Ptr to the string for total
}
ramwid_t;

// Flag values for GetValues()
#define			MEM_GV_UPDMNT			0x0001	// Updates the proc list
#define			MEM_GV_UPDMEM			0x0002	// Updates the memory totals
#define			MEM_GV_UPDSTO			0x0004	// Updates the storage totals


// Global variables


// Class definition
class NxMemory
{
  public:
    // Constructor and destructor
    NxMemory(int X, int Y, int W, int H, char *appname);	// Class constructor
     ~NxMemory();		// Class destructor

    void ShowWindow(void);
    void HideWindow(void);

  private:
    // Private data
    int _winX,			// Windows TL X coordinate
      _winY;			// Windows TL Y coordinate

    // Fltk widgets/windows
    Fl_Group *_mainw;		// Windows
    ramwid_t _memory,		// Memory info
      _storage;			// Storage
    NxSlider *_memsl,		// Memory slider
     *_storsl;			// Storage slider
    NxMenuButton *_stormb;	// Storage menu button

    void GetMemory(void);	// Gets the memory values
    void GetStorage(void);
    void GetValues(int flag);	// Set the values for the widgets based on
    // the current state
    void MakeWindow(int, int, int, int);	// Creates the window/widgets
    // Private static widget callbacks
    static void stor_mb_cb(Fl_Widget * w, void *d);	// Call back for the Menu button widget

    // Private timer callbacks
    static void proc_tmr(void *d);

};				// end of class NxMemory


#endif //      NXMEMORY_INCLUDED
