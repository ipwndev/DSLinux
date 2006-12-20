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
// Category Editor List widget.                                 //
//--------------------------------------------------------------//
#ifndef CATEGORYEDITORLIST_H_

#define CATEGORYEDITORLIST_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <vector>
#include <Flek/Flv_List.H>
#include "CategoryDB.h"
using namespace std;
class CategoryEditorList:public Flv_List
{
  public:CategoryEditorList(int nX,
		       // Constructor
		       int nY, int nWidth, int nHeight, CategoryDB * pDB);
     ~CategoryEditorList();	// Destructor
    void Refresh();		// Refresh this list from the data base
  protected:void draw_row(int nOffset,
		  // Draw a row
		  int &nX, int &nY, int &nWidth, int &nHeight, int nRow);
    int handle(int nEvent);	// Handle events
  private:  CategoryDB * m_pDB;
    // Pointer to the database to be used for this dialog
    int m_nCurrentRow;		// Currently selected row - used in callback function
      vector < int >m_vnRow;	// Row numbers displayed in the list
    static void Callback(Fl_Widget * pWidget,	// FLTK callback function
			 void *pUserData);
};


#endif /*  */
