// $Id$

#ifndef _STREAMIO_HH_
#define _STREAMIO_HH_

// Miscellaneous functions for operating on IO streams in C++

#include <iostream>
#include <iomanip>

using namespace std;

inline void remove_white_space(istream& i) {
  char c;

  while ( i )
     {
       i.get(c);
       if ( c != ' ' && c != '\t' && c != '\n' )
          {
            i.putback(c);
            break;
          }
     }
}

inline void remove_spaces(istream& i)
{
  char c;

  while ( i )
     {
       i.get(c);
       if ( c != ' ' )
          {
            i.putback(c);
            break;
          }
     }
}

inline void remove_spaces_and_tabs(istream& i)
{
  char c;

  while ( i )
     {
       i.get(c);
       if ( c != ' ' && c != '\t' )
          {
            i.putback(c);
            break;
          }
     }
}

inline void read_till_eol(istream& i)
{
  char c;

  i.get(c);
  while ( i && c != '\n' )
     i.get(c);
}

#endif // #ifndef _STREAMIO_HH_

