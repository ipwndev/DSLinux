#include <assert.h>
#include <string.h>
#include "qstring.h"
#include "qregexp.h"

int QString::find(const QRegExp & reg, int index) const 
{
  return reg.match(*this);
}

int QString::find(char c, int index=0, bool cs=true) const
{
  const char *cp = c_str();
  int len = strlen(cp);
  for(int i=index; i<len; i++)
  {
    if(cs)
    {
      if(cp[i] == c)
	return(i);
    }
    else
    {
      if(tolower(cp[i]) == tolower(c))
	return(i);
    }
  }
  return(-1);
}

int QString::find(const QString & str, int index=0, bool cs=true) const
{
  const char *c = c_str();
  int len = strlen(c);
  int slen = str.length();
  for(int i=index; i<len; i++)
  {
    if(cs)
    {
      if(strncmp(c+i,str,slen) == 0)
	return(i);
    }
    else
    {
      if(strncasecmp(c+i,str,slen) == 0)
	return(i);
    }
  }
  return(-1);
}

/*
**
** Finds the first occurrence of the string 'str', starting at position
** 'index' and searching backwards. If 'index' is negative, the search
** starts at the end.
**
** The search is case sensitive if 'cs' is TRUE, or case insensitive
** if 'cs' is FALSE.
**
** Returns the position of 'str', or -1 if 'str' could not be found.
**
*/
int QString::findRev(const QString & str, int index=-1, bool cs=true) const
{
  const char *ref = c_str();
  const char *sub = str.data();

  // this assertion fires if cs is FALSE. The code below doesn't
  // handle case insensitive searches (yet).
  assert(cs == TRUE);

  if(index == -1)
    index = strlen(ref) - 1;

  while(index >= 0)
  {
    if(strstr(ref + index,sub))
      break;
    --index;
  }

  //  printf("Searching for \"%s\" in \"%s\"\n",sub,ref);
  //  printf("Returning %d\n",index); getchar();

  return(index);
}

/*
**
** This is an overloaded member function, provided for convenience. It
** differs from the above function only in what argument(s) it accepts. 
**
*/
int QString::findRev(char c, int index=-1, bool cs=true) const
{
  const char *ref = c_str();

  // this assertion fires if cs is FALSE. The code below doesn't
  // handle case insensitive searches (yet).
  assert(cs == TRUE);

  if(index == -1)
    index = strlen(ref) - 1;

  while(index >= 0)
  {
    if(*(ref + index) == c)
      break;
    --index;
  }

  //  printf("Searching for '%c' in \"%s\"\n",c,ref);
  //  printf("Returning %d\n",index); getchar();

  return(index);
}




