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

//  Definition of a string class (wstring)...

#ifndef STRINGCLASS_H

#define STRINGCLASS_H 1

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <FL/editvars.h>

#ifndef WIN32
#ifndef strlwr
inline void
strlwr(char *string)
{
    while (string && *string) {
	if (isupper(*string))
	    *string = tolower(*string);
	string++;
    }
}


#endif /*  */
#ifndef strupr
inline void
strupr(char *string)
{
    while (string && *string) {
	if (islower(*string))
	    *string = toupper(*string);
	string++;
    }
}


#endif /*  */
#endif /*  */
class CLASSEXPORT wString
{
  protected:static char dummy[2];
    // dummy string for if allocation fails...
    char *buffer;		// the buffer where the string is stored
    long BufferCapacity;	// how much it can hold without growing, one less than allocated byte count
    long BufferLength;		// length of the string currently in the buffer
    void Initialize(const char *init, long size);
  public:  wString()
    {
	Initialize("", 12);
    };
    wString(const wString & str)
    {
	Initialize((const char *) str.buffer, str.BufferCapacity);
    };
    wString(const char *init)
    {
	Initialize(init, (init) ? strlen(init) : 4);
    };
    wString(const char *init, long size)
    {
	Initialize(init, size);
    };
    wString(const char *init, long start, long length, bool stripit = TRUE);	// for cutting strings out of fields
    wString(char c, int nRepeat);
    ~wString();
    const char *Get() const
    {
	if (buffer)
	    return buffer;
	else
	    return dummy;
    };
    long Length() const
    {
	return BufferLength;
    };
    long Capacity() const
    {
	return BufferCapacity;
    };
    bool AlmostFull() const
    {
	return (BufferCapacity < (BufferLength + 4));
    };
    wString & Strip();
    wString & Grow(long amount = 10);
    wString Extract(long position, long length, bool cutit = FALSE);
    wString Cut(long position, long length)
    {
	return Extract(position, length, TRUE);
    };
    wString Copy(long position, long length)
    {
	return Extract(position, length, FALSE);
    };
    wString & Paste(long position, wString & str) {
	return InsertAt(position, str.buffer);
    };
    wString & Paste(long position, const char *str) {
	return InsertAt(position, str);
    };
    wString GetWordAt(long position) const;
    wString & InsertAt(long where, char character);
    wString & InsertAt(long where, const char *words);
    wString & DeleteAt(long where, long howmany = 1);
    wString & ChopAt(long limit, wString * remainder =
		     (wString *) NULL, bool special = false);
    wString & WrapAt(long limit, wString * remainder = (wString *) NULL);
    void RecalculateLength()
    {
	BufferLength = strlen(buffer);
    }
    void SSet(const char *value);
    void Shrink();		// trim unused allocation space
    char &LastCharacter() const
    {
	return buffer[BufferLength - 1];
    };
    friend wString operator+(const wString & str1, const wString & str2)
    {
	return str1 + str2.Get();
    };
    RCLDLL friend wString operator+(const char *str1, const wString & str2);
    RCLDLL friend wString operator+(const wString & str1, const char *str2);
    friend wString operator-(const wString & str1, const wString & str2)
    {
	return (str1 - str2.Get());
    };
    RCLDLL friend wString operator-(const wString & str1, const char *str2);
    wString & operator+=(const wString & str);	// concatenate
    wString & operator+=(char c);	// concatenate a character
    char &operator[] (long index)
    {
	return ((index < BufferLength) ? (buffer[index]) : buffer[0]);
    };				// array index operator
    long operator<(const char *str) const
    {
	return (strcmp(buffer, str) < 0);
    };
    long operator>(const char *str) const
    {
	return (strcmp(buffer, str) > 0);
    };
    long operator==(const char *str) const
    {
	return (strcmp(buffer, str) == 0);
    };
    wString & operator=(const char *str) {
	SSet(str);
	return (*this);
    };
    RCLDLL wString & wString::operator=(const wString & str)
    {
	SSet(str.buffer);
	return (*this);
    }
    wString & operator=(const unsigned char *str) {
	SSet((const char *) str);
	return *this;
    };
    operator    const char *() const
    {
	return Get();
    };
    wString & ToUpper() {
	strupr(buffer);
	return *this;
    };
    wString & ToLower() {
	strlwr(buffer);
	return *this;
    };
    long operator!=(const char *str) const
    {
	return !(*this == str);
    };
    long operator<=(const char *str) const
    {
	return !(*this > str);
    };
    long operator>=(const char *str) const
    {
	return !(*this < str);
    };
};


#endif /*  */
