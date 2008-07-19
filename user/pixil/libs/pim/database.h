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

#ifndef _DATABASE_H_
#define _DATABASE_H_

enum
{
    DB_TEXT = 0x001,
    DB_STRING = 0x002,
    DB_INTEGER = 0x004,
    DB_FLOAT = 0x008,
    DB_PLUSINTEGER = 0x010,
    DB_PRIMARY = 0x100
};

enum
{
    DB_OK = 0,
    DB_ERROR = -1,
    DB_TABLE_EXISTS = -2,
    DB_FILE_ERROR = -3,
    DB_NOT_OPENED = -4
};

class db_row
{
  private:
    char **fields_;
    char **data_;

    int cols_;

    int find_col(char *);

  public:
      db_row(void);
      db_row(char **, int, char **);
      db_row(db_row &);
     ~db_row(void);

    char *operator[] (int);
    char *operator[] (char *);

      db_row & operator=(const db_row &);
    int value(int col, void *dest, unsigned int size, int type);
    int value(char *field, void *dest, unsigned int size, int type);
};

class db_query
{
  private:
    db_row ** rows_;
    int count_;
  public:
      db_query(int, char **, int, char **);
     ~db_query();

    int rows(void)
    {
	return (count_);
    }
    db_row *row(int r)
    {
	if (r < count_)
	    return (rows_[r]);
	else
	    return (0);
    }
};


struct db_fields
{
    char field[20];
    int type;
};

struct db_table
{
    char name[20];
    int field_count;
    db_fields *fields;
};


class database
{

  private:
    struct sqlite *dbhandle;

  public:
      database(void)
    {
	dbhandle = 0;
    }
     ~database(void)
    {
	close();
    }

    int open(char *filename);
    void close(void);

    int add_table(db_table * table);

    db_query *get_names(char *);
    db_query *get_record(int);

    int exec(const char *str, ...);
    db_query *do_query(const char *str, ...);

    int insert_record(db_table *, char **, int n = -1);
    int update_record(db_table *, char **);
    void delete_record(int);
};

char *db_qexpand(char *str);

#endif
