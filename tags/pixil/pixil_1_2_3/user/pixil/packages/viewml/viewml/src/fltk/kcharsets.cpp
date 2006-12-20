#include "kcharsets.h"

const KCharsetConversionResult & KCharsets::convertTag(const char * tag) 
{ 
  int l;
  return convertTag(tag,l);
}

typedef struct
{
  char *tag;
  char *conversion;
}TagEntry;

/* 
   Some of these tags were contributed by Johannes Thoma <joe@mond.at>
*/

static TagEntry tag_list[] =
{
  { "&amp","&"},
  { "&bull","*" },
  { "&copy","\xA9" },
  { "&emdash","-" },
  { "&emsp"," " },
  { "&endash","-" },
  { "&ensp"," " },
  { "&gt",">" },
  { "&hellip","..." },
  { "&lt","<" },
  { "&nbsp"," " },
  { "&quot","\"" },
  { "&reg","\xAE" },
  { "&trade","(tm)" },
  { "&AElig",	"\xc6" },	 /* capital AE diphthong (ligature) */
  { "&Aacute",	"\xc1" },	 /* capital A, acute accent */
  { "&Acirc",	"\xc2" },	 /* capital A, circumflex accent */
  { "&Agrave",	"\xc0" },	 /* capital A, grave accent */
  { "&Aring",	"\xc5" },	 /* capital A, ring */
  { "&Atilde",	"\xc3" },	 /* capital A, tilde */
  { "&Auml",	"\xc4" },	 /* capital A, dieresis or umlaut mark*/
  { "&Ccedil",	"\xc7" },	 /* capital C, cedilla */
  { "&ETH",	"\xd0" },	 /* capital Eth, Icelandic */
  { "&Eacute",	"\xc9" },	 /* capital E, acute accent */
  { "&Ecirc",	"\xca" },	 /* capital E, circumflex accent */
  { "&Egrave",	"\xc8" },	 /* capital E, grave accent */
  { "&Euml",	"\xcb" },	 /* capital E, dieresis or umlaut mark*/
  { "&Iacute",	"\xcd" },	 /* capital I, acute accent */
  { "&Icirc",	"\xce" },	 /* capital I, circumflex accent */
  { "&Igrave",	"\xcc" },	 /* capital I, grave accent */
  { "&Iuml",	"\xcf" },	 /* capital I, dieresis or umlaut mark*/
  { "&Ntilde",	"\xd1" },	 /* capital N, tilde */
  { "&Oacute",	"\xd3" },	 /* capital O, acute accent */
  { "&Ocirc",	"\xd4" },	 /* capital O, circumflex accent */
  { "&Ograve",	"\xd2" },	 /* capital O, grave accent */
  { "&Oslash",	"\xd8" },	 /* capital O, slash */
  { "&Otilde",	"\xd5" },	 /* capital O, tilde */
  { "&Ouml",	"\xd6" },	 /* capital O, dieresis or umlaut mark*/
  { "&THORN",	"\xdd" },	 /* capital THORN, Icelandic */
  { "&Uacute",	"\xda" },	 /* capital U, acute accent */
  { "&Ucirc",	"\xdb" },	 /* capital U, circumflex accent */
  { "&Ugrave",	"\xd9" },	 /* capital U, grave accent */
  { "&Uuml",	"\xdc" },	 /* capital U, dieresis or umlaut mark*/
  { "&Yacute",	"\xdd" },	 /* capital Y, acute accent */
  { "&aacute",	"\xe1" },	 /* small a, acute accent */
  { "&acirc",	"\xe2" },	 /* small a, circumflex accent */
  { "&aelig",	"\xe6" },	 /* small ae diphthong (ligature) */
  { "&agrave",	"\xe0" },	 /* small a, grave accent */
  { "&aring",	"\xe5" },	 /* small a, ring */
  { "&atilde",	"\xe3" },	 /* small a, tilde */
  { "&auml",	"\xe4" },	 /* small a, dieresis or umlaut mark */
  { "&ccedil",	"\xe7" },	 /* small c, cedilla */
  { "&eacute",	"\xe9" },	 /* small e, acute accent */
  { "&ecirc",	"\xea" },	 /* small e, circumflex accent */
  { "&egrave",	"\xe8" },	 /* small e, grave accent */
  { "&eth",	"\xf0" },	 /* small eth, Icelandic */
  { "&euml",	"\xeb" },	 /* small e, dieresis or umlaut mark */
  { "&iacute",	"\xed" },	 /* small i, acute accent */
  { "&icirc",	"\xee" },	 /* small i, circumflex accent */
  { "&igrave",	"\xec" },	 /* small i, grave accent */
  { "&iuml",	"\xef" },	 /* small i, dieresis or umlaut mark */
  { "&ntilde",	"\xf1" },	 /* small n, tilde */
  { "&oacute",	"\xf3" },	 /* small o, acute accent */
  { "&ocirc",	"\xf4" },	 /* small o, circumflex accent */
  { "&ograve",	"\xf2" },	 /* small o, grave accent */
  { "&oslash",	"\xf8" },	 /* small o, slash */
  { "&otilde",	"\xf5" },	 /* small o, tilde */
  { "&ouml",	"\xf6" },	 /* small o, dieresis or umlaut mark */
  { "&szlig",	"\xdf" },	 /* small sharp s, German (szligature)-> */
  { "&thorn",	"\xfe" },	 /* small thorn, Icelandic */
  { "&uacute",	"\xfa" },	 /* small u, acute accent */
  { "&ucirc",	"\xfb" },	 /* small u, circumflex accent */
  { "&ugrave",	"\xf9" },	 /* small u, grave accent */
  { "&uuml",	"\xfc" },	 /* small u, dieresis or umlaut mark */
  { "&yacute",	"\xfd" },  /* small y, acute accent */
  { "&yuml",	"\xff" },  /* small y, dieresis or umlaut mark */

  { "&","&" },   // must be last entry in table
    { NULL,NULL }
};

const KCharsetConversionResult & KCharsets::convertTag(const char * tag,
						       int & len) 
{
  char buf[8];
  int count,taglen;

  // check tag for number
  if(*(tag + 1) == '#')
  {
    buf[0] = atoi(tag + 2);
    buf[1] = 0x00;
    m_Result.setResult(buf);
    len = strlen(tag);
  }

  // scan list for tag
  else
  {
    count = 0;
    while(tag_list[count].tag != NULL)
    {
      taglen = strlen(tag_list[count].tag);
      if(strncmp(tag,tag_list[count].tag,taglen) == 0)
      {
	m_Result.setResult(tag_list[count].conversion);
	len = taglen;
	break;
      }
      ++count;
    }

    // no matching tag - substitute a space code
    if(tag_list[count].tag == NULL)
    {
      m_Result.setResult(" ");
      len = strlen(tag);
    }
  }

  //printf("KCharSet: \"%s\" = \"%s\"",tag,(char*)m_Result); getchar();

  // return result and exit
  return(m_Result);
}
