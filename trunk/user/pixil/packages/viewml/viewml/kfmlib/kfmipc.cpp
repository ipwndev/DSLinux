/* This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "kfmipc.h"

void write_int( int _fd, int _value )
{
    char buffer[10];
    sprintf( buffer, "%i ", _value );
    write( _fd, (const char*)buffer, strlen(buffer) );
}

void write_float( int _fd, float _value )
{
    char buffer[10];
    sprintf( buffer, "%f ", _value );
    write( _fd, (const char*)buffer, strlen(buffer) );
}

void write_char( int _fd, char _value )
{
    write( _fd, (const char*)&_value, sizeof(char) );
}

void write_string( int _fd, const char* _value )
{
    if ( _value == 0L )
    {
	write_int( _fd, -1 );
	return;
    }
    
    int len = strlen( _value );
    write_int( _fd, len );
    write( _fd, (const char*)_value, strlen(_value) );
}

void write_intList( int _fd, intList* _list )
{
    int len = _list->elements * sizeof(int);
    write( _fd, (const char*)&(_list->elements), sizeof(int) );
    write( _fd, (const char*)(_list->list), len );
}

void write_floatList( int _fd, floatList* _list )
{
    int len = _list->elements * sizeof(float);
    write( _fd, (const char*)&(_list->elements), sizeof(int) );
    write( _fd, (const char*)(_list->list), len );
}

void write_charList( int _fd, charList* _list )
{
    int len = _list->elements * sizeof(char);
    write( _fd, (const char*)&(_list->elements), sizeof(int) );
    write( _fd, (const char*)(_list->list), len );
}

void write_stringList( int _fd, stringList* _list )
{
    write( _fd, (const char*)&(_list->elements), sizeof(int) );
    for( int i = 0; i < _list->elements; i++ )
    {
	write_string( _fd, _list->list[i] );
    }
}

char* read_string( char *_data, int &_pos, int _len )
{
    int tmp = read_int( _data, _pos, _len );
    if ( tmp == -1 )
	return 0L;
    
    char *str = (char*)malloc(tmp + 1);
    strncpy( str, _data + _pos, tmp);
    _pos += tmp;
    str[tmp] = 0;
    return str;
}

int read_int( char *_data, int &_pos, int _len )
{
    int i = _pos;
    while ( _data[ _pos ] != ' ' )
    {
	_pos++;
	if ( _pos == _len )
	    return 0;
    }
    _pos++;
    
    return atoi( _data + i );
}

char read_char( char *_data, int &_pos, int )
{
    char tmp = *((char*)_data + _pos);
    _pos += sizeof(char);
    return tmp;
}

float read_float( char *_data, int &_pos, int _len )
{
    int i = _pos;
    while ( _data[ _pos ] != ' ' )
    {
	_pos++;
	if ( _pos == _len )
	    return 0;
    }
    _pos++;
    
    return atof( _data + i );
}

void read_stringList( char *_data, int &_pos, int _len, stringList *_list )
{
    int tmp = *((int*)_data + _pos);
    _list->elements = tmp;
    _pos += sizeof(int);
    _list->list = (char**)malloc( tmp * sizeof(char*) );
    for( int i = 0; i < tmp; i++ )
    {
	_list->list[i] = read_string( _data, _pos, _len );
    }
}

void read_intList( char *_data, int &_pos, int _len, intList *_list )
{
    int tmp = *((int*)_data + _pos);
    _list->elements = tmp;
    _pos += sizeof(int);
    _list->list = (int*)malloc( tmp * sizeof(int) );
    for( int i = 0; i < tmp; i++ )
    {
	_list->list[i] = read_int( _data, _pos, _len );
    }
}

void read_floatList( char *_data, int &_pos, int _len, floatList *_list )
{
    int tmp = *((int*)_data + _pos);
    _list->elements = tmp;
    _pos += sizeof(int);
    _list->list = (float*)malloc( tmp * sizeof(float) );
    for( int i = 0; i < tmp; i++ )
    {
	_list->list[i] = read_float( _data, _pos, _len );
    }
}

void read_charList( char *_data, int &_pos, int _len, charList *_list )
{
    int tmp = *((int*)_data + _pos);
    _list->elements = tmp;
    _pos += sizeof(int);
    _list->list = (char*)malloc( tmp * sizeof(char) );
    for( int i = 0; i < tmp; i++ )
    {
	_list->list[i] = read_char( _data, _pos, _len );
    }
}

void free_string( char *_str )
{
    if ( _str != 0L )
	free( _str );
}

void free_stringList( stringList *_list )
{
    for( int i = 0; i < _list->elements; i++ )
    {
	free ( _list->list[i] );
    }
    free ( _list->list );
}

void free_intList( intList *_list )
{
    free ( _list->list );
}

void free_floatList( floatList *_list )
{
    free ( _list->list );
}

void free_charList( charList *_list )
{
    free ( _list->list );
}

int len_int( int _value )
{
    char buffer[ 20 ];
    sprintf( buffer, "%i", _value );
    return strlen(buffer) + 1;
}

int len_float( float _value )
{
    char buffer[ 20 ];
    sprintf( buffer, "%f", _value );
    return strlen(buffer) + 1;
}

int len_char( char )
{
    return 1;
}

int len_string( const char *_str )
{
    if ( _str == 0L )
	// The '-1' takes 2 characters + 1 space
	return 3;
    int len = strlen( _str );
    return len + len_int( len );
}

int len_stringList( stringList *_list )
{
    int len = sizeof(int);
    for( int i = 0; i < _list->elements; i++ )
    {
	len += len_string( _list->list[i] );
    }
    return len;
}

int len_intList( intList *_list )
{
    return ( sizeof(int) + _list->elements * sizeof(int) );
}

int len_floatList( floatList *_list )
{
    return ( sizeof(int) + _list->elements * sizeof(float) );
}

int len_charList( charList *_list )
{
    return ( sizeof(int) + _list->elements * sizeof(char) );
}
