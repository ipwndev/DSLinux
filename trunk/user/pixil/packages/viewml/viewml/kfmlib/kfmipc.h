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
#ifndef ipc_h
#define ipc_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct stringList
{
    int elements;
    char **list;
};

struct intList
{
    int elements;
    int *list;
};

struct floatList
{
    int elements;
    float *list;
};

struct charList
{
    int elements;
    char *list;
};

#define boolList charList
#define write_bool write_char
#define read_bool read_char
#define free_bool free_char
#define free_boolList free_charList
#define len_bool len_char
#define len_boolList len_boolList

void write_int( int _fd, int _value );
void write_float( int _fd, float _value );
void write_char( int _fd, char _value );
void write_string( int _fd, const char* _value );
void write_intList( int _fd, intList* _list );
void write_floatList( int _fd, floatList* _list );
void write_charList( int _fd, charList* _list );
void write_stringList( int _fd, stringList* _list );
char* read_string( char *_data, int &_pos, int _len );
int read_int( char *_data, int &_pos, int _len );
char read_char( char *_data, int &_pos, int _len );
float read_float( char *_data, int &_pos, int _len );
void read_stringList( char *_data, int &_pos, int _len, stringList *_list );
void read_intList( char *_data, int &_pos, int _len, intList *_list );
void read_floatList( char *_data, int &_pos, int _len, floatList *_list );
void read_charList( char *_data, int &_pos, int _len, charList *_list );

#define free_int( x ); ;
#define free_char( x ); ;
#define free_float( x ); ;

void free_string( char *_str );
void free_stringList( stringList *_list );
void free_intList( intList *_list );
void free_floatList( floatList *_list );
void free_charList( charList *_list );

int len_int( int _value );
int len_float( float _value );
int len_char( char _value );
int len_string( const char *_str );
int len_stringList( stringList *_list );
int len_intList( intList *_list );
int len_floatList( floatList *_list );
int len_charList( charList *_list );

#endif
