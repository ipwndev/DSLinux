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
// A note saved from any of the four info types                 //
//--------------------------------------------------------------//
#ifndef NOTE_H_

#define NOTE_H_

#ifdef WIN32
#pragma warning(disable:4786)
#define inline __inline
#endif /*  */

#include <string>
using namespace std;
class Note
{
  public:Note(const char *pszFileName,
	 // Construct from a file
	 int nMaxLength = 0, const char *pszPrefix = NULL);
      Note(int nMaxLength = 0,	// Construct as empty
	   const char *pszPrefix = NULL);
     ~Note();			// Destructor
    void Delete();		// Delete this note - delete its file
    inline const string GetFileName() const	// Get the file name for this note
    {
	return (m_strFileName);
    }
    inline int GetMaxLength() const	// Get the maximum length for this note
    {
	return (m_nMaxLength);
    }
    inline const string GetText() const	// Get the text of the note
    {
	return (m_strText);
    }
    inline bool IsChanged() const	// Has this note been changed
    {
	return (m_bChanged);
    }
    void Save();		// Save this note to disk
    void SetText(const char *pszText);	// Change the text
  private:  bool m_bChanged;	// Has the text changed or not
    int m_nMaxLength;		// Maximum length for the text of this note
    string m_strFileName;	// The note file name
    string m_strPrefix;		// Prefix for note file names
    string m_strText;		// The text of the note
    void CreateFileName();	// Create a file name for this note
};


#endif /*  */
