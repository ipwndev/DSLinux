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



//
//  OKay, I know this is a  pretty wimpy string class, but I was in
//  a hurry, OK?  Nothing fancy, just basic "safe" strings.
//

#include <string.h>
#include <ctype.h>
#include <wstring.h>

RCLDLL char
    wString::dummy[2] =
    "";

RCLDLL wString & wString::Strip()
{
    char *
	temp;

    temp = buffer;
    while (isspace(*temp++));
    if (temp > buffer)
	strcpy(buffer, temp);
    temp = buffer + strlen(buffer);
    while (temp > buffer) {
	if (isspace(*(temp - 1)))
	    break;
	temp--;
    }
    *temp = '\0';
    return (*this);
}

RCLDLL void
wString::Initialize(const char *init, long size)
{
    dummy[0] = '\0';
    if (size < 1)
	if (init)
	    size = strlen(init);
	else
	    size = 1;
    BufferCapacity = 0;
    BufferLength = 0;
    buffer = (char *) calloc(1, size + 4);
    if (buffer) {
	BufferCapacity = size + 3;
	if (init)
	    strcpy(buffer, init);
	else
	    buffer[0] = '\0';
	BufferLength = strlen(buffer);
    }
}

RCLDLL
    wString::wString(const char *what, long where, long howmuch, bool stripit)
{
    buffer = (char *) calloc(1, howmuch + 4);
    if (buffer) {
	BufferCapacity = howmuch + 3;
	memcpy(buffer, &what[where], howmuch);
	buffer[howmuch] = '\0';
	BufferLength = strlen(buffer);
	if (stripit)
	    Strip();
	else {
	    *this += wString(howmuch, ' ');
	    this->ChopAt(howmuch);
	}
    } else
	BufferCapacity = 0;
}

RCLDLL wString::wString(char c, int num)
{
    Initialize("", num + 1);
    if (BufferCapacity > num) {
	memset(buffer, c, num);
	buffer[num + 1] = '\0';
    }
}

RCLDLL wString::~wString()
{
    if (buffer)
	free(buffer);
}

RCLDLL void
wString::Shrink()
{
    if (BufferCapacity > (BufferLength + 4)) {
	char *newbuf = (char *) calloc(1, BufferLength + 4);
	if (newbuf) {
	    if (buffer) {
		strcpy(newbuf, buffer);
		free(buffer);
	    }
	    BufferCapacity = BufferLength + 3;
	    buffer = newbuf;
	}
    }
}

RCLDLL void
wString::SSet(const char *val)
{
    long newsize;

    if (val) {
	newsize = strlen(val) + 4;
	if (BufferCapacity < newsize)
	    Grow(newsize - BufferCapacity + 4);
	if (buffer) {		// this will work whether or not the buffer got resized
	    // since if it did, the '\0' will get copied by the
	    // strncpy, and if it didn't, the next line will get it.
	    strncpy(buffer, val, BufferCapacity);
	    buffer[BufferCapacity] = '\0';
	    BufferLength = strlen(buffer);
	    Shrink();
	}
    } else {
	buffer[0] = '\0';
	BufferLength = 0;
	Shrink();
    }
}

RCLDLL wString &
wString::operator+=(const wString & str)
{
    long newsize = BufferLength + str.BufferLength + 4;
    if (newsize > BufferCapacity)
	Grow(newsize - BufferCapacity + 4);
    strcat(buffer, str.buffer);
    BufferLength = strlen(buffer);
    return (*this);
}

RCLDLL wString &
wString::operator+=(char c)
{
    if (BufferLength > BufferCapacity - 4)
	Grow();
    buffer[BufferLength++] = c;
    buffer[BufferLength] = '\0';
    return (*this);
}

RCLDLL wString
wString::Extract(long position, long length, bool cutit)
{
    long count;

    wString s("", length + 1);
    if (position < BufferLength) {
	if (s.buffer) {
	    for (count = 0; count < length; count++)
		s.buffer[count] = this->buffer[position + count];
	    s.buffer[count] = '\0';
	    s.BufferLength = length;
	    if (cutit)
		DeleteAt(position, length);
	}
    }
    return (s);
}

RCLDLL wString
wString::GetWordAt(long position) const
{
    char *temp;

    wString s("", this->BufferCapacity);
    if (s.buffer) {
	// rewind to start of current word
	while (!isspace(this->buffer[position - 1])
	       && !ispunct(this->buffer[position - 1]))
	    if (position)
		position--;
	    else
		break;
	// skip leading punctuation
	while (ispunct(this->buffer[position]))
	    if (position < BufferLength)
		position++;
	    else
		break;
	temp = s.buffer;
	while (this->buffer[position] && !(isspace(this->buffer[position]))) {
	    if (ispunct(this->buffer[position]))
		if (this->buffer[position] != '\'')
		    break;
	    *temp++ = this->buffer[position++];
	}
	*temp = '\0';
	s.BufferLength = strlen(s.buffer);
	s.Shrink();
    }
    return (s);
}

#include <stdio.h>
RCLDLL wString &
wString::InsertAt(long where, char character)
{
    if (AlmostFull()) {
	Grow();
    }

    if (BufferCapacity > BufferLength) {
	if (where <= (BufferLength + 1))	// insert allowed if before NULL terminator
	{
	    memmove(&buffer[where + 1], &buffer[where],
		    (BufferLength + 1) - where + 1);
	    buffer[where] = character;
	    BufferLength++;
	}
    }

    return (*this);
}

RCLDLL wString &
wString::InsertAt(long where, const char *insertme)
{
    long inslen = strlen(insertme);
    if ((BufferCapacity - BufferLength) < inslen)
	Grow(inslen);
    if (BufferCapacity >= (BufferLength + inslen))
	if (where <= (BufferLength + 1))	// insert allowed if before NULL terminator
	{
	    memmove(&buffer[where + inslen], &buffer[where],
		    BufferLength - where + 1);
	    memcpy(&buffer[where], insertme, inslen);
	    BufferLength += inslen;
	}
    return (*this);
}

RCLDLL wString &
wString::DeleteAt(long where, long howmany)
{
    if ((where >= 0) && (where < BufferLength)) {
	if (howmany < (BufferLength - where)) {
	    strcpy(&buffer[where], &buffer[where + howmany]);
	    BufferLength -= howmany;
	} else {
	    buffer[where] = '\0';
	    BufferLength = where;
	}
	Shrink();
    }
    return (*this);
}


RCLDLL wString &
wString::ChopAt(long limit, wString * remainder, bool special)
{
    if ((BufferLength == limit) && BufferLength && (limit >= 0) && special) {
	if (remainder)
	    remainder->SSet(" ");
	buffer[--limit] = '\0';
	BufferLength = limit;
	Shrink();
    } else if (BufferLength && (limit >= 0) && (limit < BufferLength)) {
	if (remainder)
	    remainder->SSet(&buffer[limit]);
	buffer[limit] = '\0';
	BufferLength = limit;
	Shrink();

    } else if (remainder)
	remainder->SSet("");
    return (*this);
}

RCLDLL wString &
wString::WrapAt(long limit, wString * remainder)
{
    if (remainder)
	remainder->SSet("");
    if (BufferLength && (limit >= 0) && (BufferLength == limit)) {
	if (isspace(buffer[limit - 1]))
	    ChopAt(limit, remainder, true);
    }
    if (BufferLength && (limit >= 0) && (limit < BufferLength)) {
	if (isspace(buffer[limit]) || (buffer[limit] == '\0'))
	    ChopAt(++limit, remainder);
	else {
	    bool have_space = false;
	    for (int idx = limit; idx >= 0; idx--) {
		if (isspace(buffer[idx])) {
		    have_space = true;
		}
	    }
	    if (have_space) {
		while (limit && !isspace(buffer[limit]))
		    limit--;
	    }
	    if (limit)
		ChopAt(++limit, remainder);	// skip past space, clip rest of line
	}
    }
    return (*this);
}

RCLDLL wString &
wString::Grow(long howmuch)
{
    char *newbuf = (char *) calloc(1, BufferCapacity + howmuch + 1);
    if (newbuf) {
	if (buffer) {
	    strcpy(newbuf, buffer);
	    free(buffer);
	} else
	    newbuf[0] = '\0';
	BufferCapacity += howmuch;
	buffer = newbuf;
    }
    return (*this);
}

RCLDLL wString
operator+(const char *str1, const wString & str2)
{
    wString s(str1);
    s += str2;
    return (s);
}

RCLDLL wString
operator+(const wString & str1, const char *str2)
{
    long len = strlen(str2);
    wString s(str1.Get(), str1.Length() + len + 4);
    strcat(s.buffer, str2);
    s.BufferLength += len;
    return (s);
}

RCLDLL void
Pack(wString & string)
{
    long count = 0;

    while (count < string.Length())
	if (ispunct(string[count]) || isspace(string[count]))
	    string.DeleteAt(count, 1);
	else
	    count++;
}
