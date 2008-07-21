#include <lct/unicode.h>

/*
 * Convert a UCS2 char into the equivalent UTF8 sequence, with
 * trailing zero.
 * Adapted from Linux kernel 2.0.30
 */
void ucs2_to_utf8(unicode c, char* utf)
{
  if (c < 0x80)
    {
      utf[0] = c;			/*  0*******  */
      utf[1] = 0;
    }
  else if (c < 0x800) 
    {
      utf[0] = 0xc0 | (c >> 6); 	/*  110***** 10******  */
      utf[1] = 0x80 | (c & 0x3f);
      utf[2] = 0;
    } 
  else
    {
      utf[0] = 0xe0 | (c >> 12); 	/*  1110**** 10****** 10******  */
      utf[1] = 0x80 | ((c >> 6) & 0x3f);
      utf[2] = 0x80 | (c & 0x3f);
      utf[3] = 0;
    }
  /* UTF-8 is defined for words of up to 31 bits,
     but we need only 16 bits here */
}


/* Combine UTF-8 into Unicode */
/* Incomplete characters silently ignored */
unicode utf8_to_ucs2 (char* buf)
{
  int utf_count = 0;
  long utf_char;
  unicode tc;
  unsigned char c;
  
  do
    {
      c = *buf;
      buf++;
      
      /* if byte should be part of multi-byte sequence */
      if(c & 0x80)
	{
	  /* if we have already started to parse a UTF8 sequence */
	  if (utf_count > 0 && (c & 0xc0) == 0x80)
	    {
	      utf_char = (utf_char << 6) | (c & 0x3f);
	      utf_count--;
	      if (utf_count == 0)
		  tc = utf_char;
	      else
		  continue;
	    } 
	  else	/* Possibly 1st char of a UTF8 sequence */
	    {
	      if ((c & 0xe0) == 0xc0) 
		{
		  utf_count = 1;
		  utf_char = (c & 0x1f);
		} 
	      else if ((c & 0xf0) == 0xe0) 
		{
		  utf_count = 2;
		  utf_char = (c & 0x0f);
		} 
	      else if ((c & 0xf8) == 0xf0) 
		{
		  utf_count = 3;
		  utf_char = (c & 0x07);
		} 
	      else if ((c & 0xfc) == 0xf8) 
		{
		  utf_count = 4;
		  utf_char = (c & 0x03);
		} 
	      else if ((c & 0xfe) == 0xfc) 
		{
		  utf_count = 5;
		  utf_char = (c & 0x01);
		} 
	      else
		  utf_count = 0;
	      continue;
	    }
	} 
      else /* not part of multi-byte sequence - treat as ASCII
	    * this makes incomplete sequences to be ignored
	    */
	{
	  tc = c;
	  utf_count = 0;
	}
    }
  while (utf_count);
  
  return tc;
}
