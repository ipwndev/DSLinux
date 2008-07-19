#include "qregexp.h"
#include "regex.h"

QRegExp::QRegExp(const QString & reg, bool cs, bool wildcard)
{ 
  m_RegExp = reg; 
  m_bCaseSensitive = cs;
}


int QRegExp::match(const QString & str, int index, int * len, 
		   bool indexIsStart) const
{
  regex_t exp;
  int result;
  size_t nmatch;
  regmatch_t pmatch[2];

  nmatch = 2;
  
  result = regcomp(&exp,m_RegExp,REG_EXTENDED);

// CRH
  if(indexIsStart)
    result = regexec(&exp, (char *)str+index, nmatch,pmatch,0);
  else
    result = regexec(&exp, str, nmatch,pmatch,0);
//  result = regexec(&exp, str, nmatch,pmatch,0);

  if(len)
    *len = pmatch[0].rm_eo - pmatch[0].rm_so;
  regfree(&exp);

// CRH pretend to be KDE version of regexp
  if(result)
    return -1;
  else
    return pmatch[0].rm_so;
//  return result;
}


