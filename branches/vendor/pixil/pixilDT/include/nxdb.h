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

#ifndef NXDB_H
#define NXDB_H

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <stdio.h>
#include <map>
#include <string>

extern "C"
{
#include "file.h"
}

#ifdef WIN32
#ifdef _DEBUG
#define DEBUG
#endif
using namespace std;
#endif

typedef map < string, int, less < string > >TDatabase;
typedef
    TDatabase::value_type
    TValue;

#define INT_INDEX 7
#define CHAR_INDEX 5
#define TEMP_DB "/tmp/c0bxztemp"

class
    NxDb
{

  private:
    TDatabase
	db;
    int
	dbNum;
    int
	index[255];
    fildes *
	dbDesc[255];
    field *
	dbField[255];
    string
	path;

    // Register the Database
    void
    Register(string _dbName, fildes * _dbDesc, field * _dbField, int var);
  public:enum t_Flags
    {
	NONE =
	    0x00,
	DELETED =
	    0x01,
	NEW =
	    0x02,
	CHANGED =
	    0x04,
	ERASED =
	    0xF0,
    };
  public:NxDb(int argc, char *argv[]);
    void
    SetPath(const char *c)
    {
	path = c;
    }
    char *
    GetPath()
    {
	char *
	    tmp = (char *)
	    path.
	    c_str();
	return tmp;
    }

    // Basic Database Operations
    int
    Open(string _dbName, fildes * _dbDesc, field * _dbField, int var,
	 int path_flag = 0);
    int
    Close(string _dbName);

    // Creates a totally new Database (note: each instace can only have this one table)
    int
    Create(string _dbName, fildes * _dbDesc, field * _dbField, int var,
	   int path_flag = 0);

    // Basic Record Operations

    // Creates a list of  record numbers matching specific criteria
    // You then use Extract to get specific record numbers
    int
    Select(string _dbName, int *ret, int ret_size, bool bDeleteFlag =
	   false, int flags = -1);
    int
    Select(string _dbName, char *value, int fieldNo, int *ret, int ret_size,
	   bool bDeleteFlag = false);

    // Gets all fields or a specific field from a specific record
    int
    Extract(string _dbName, int recno, int fieldNo, char *ret_buf);
    int
    Extract(string _dbName, int recno, char *ret_buf);

    // Gets a specific field ONLY from a deleted record.
    int
    ExtractDeleted(string _dbName, int recno, int fieldNo, char *ret_buf);

    // Always inserts a new record by setting the NEW bit flag
    void
    Insert(string _dbName, char *record);
    void
    Insert(string _dbName, char *record, int &rec);

    // Makes changes to a record data
    void
    Edit(string _dbName, int recno, char *record);

    // Logically deletes a record
    void
    DeleteRec(string _dbName, int recno);

    // Logically deletes all records in database, if fieldNo == -1
    // OR logically deletes any records containing value in fieldNo.
    void
    Purge(string _dbName, int fieldNo = -1, char *value = 0);

    // Perm. deletes a record
    void
    EraseRec(string _dbName, int recno);

    // Sets Record metadata to flags
    int
    SetFlags(const string _dbName, const int &recno, const int &flags);
    int
    GetFlags(const string _dbName, const int &recno, int &flags);

    // Returns the number of total records in a database
    int
    NumRecs(string _dbName);
    fildes *
    GetFilDes(string _dbName);
    field *
    GetField(string _dbName);
};
void
fatal(int n);

#endif //NXDB_H
