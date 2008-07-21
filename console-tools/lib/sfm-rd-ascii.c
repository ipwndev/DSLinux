#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <linux/kd.h>

#include <lct/local.h>
#include <lct/utils.h>
#include <lct/font.h>

int sfm_read_ascii(FILE* mapf, struct unimapdesc *list, int fontlen)
{
  char buffer[65536];
  int i;
  int fp0, fp1, un0, un1;
  char *p, *p1;
  
  int listsz;
  
  list->entries = NULL;
  listsz = list->entry_ct = 0;
  
  while ( fgets(buffer, sizeof(buffer), mapf) != NULL ) 
    {
      if ( (p = strchr(buffer, '\n')) != NULL )
	  *p = '\0';
      else
	  fprintf(stderr, _("Warning: line too long in unicode map.\n"));
      
      p = buffer;

      /*
       * Syntax accepted:
       *	<fontpos>	<unicode> <unicode> ...
       *	<range>		idem
       *	<range>		<unicode range>
       *
       * where <range> ::= <fontpos>-<fontpos>
       * and <unicode> ::= U+<h><h><h><h>
       * and <h> ::= <hexadecimal digit>
       */

      while (*p == ' ' || *p == '\t')
	  p++;
      if (!*p || *p == '#')
	  continue;	/* skip comment or blank line */

      fp0 = strtol(p, &p1, 0);
      if (p1 == p) 
	{
	  fprintf(stderr, _("Bad input line: %s\n"), buffer);
	  errno = EBFONT;
	  return -1;
	}
      p = p1;
      
      while (*p == ' ' || *p == '\t')
	  p++;
      if (*p == '-') 
	{
	  p++;
	  fp1 = strtol(p, &p1, 0);
	  if (p1 == p) 
	    {
	      fprintf(stderr, _("Bad input line: %s\n"), buffer);
	      errno = EBFONT;
	      return -1;
	    }
	  p = p1;
	}
      else
	  fp1 = 0;

      if ( fp0 < 0 || fp0 >= fontlen ) 
	{
	  fprintf(stderr,
		  _("unicode map: glyph number (0x%x) larger than font length\n"), fp0);
	  errno = EBFONT;
	  return -1;
	  
	}
      if ( fp1 && (fp1 < fp0 || fp1 >= fontlen) ) 
	{
	  fprintf(stderr,
		  _("unicode map: bad end of range (0x%x)\n"), fp1);
	  errno = EBFONT;
	  return -1;
	}

      if (fp1) 
	{
	  /* we have a range; expect the word "idem" or a Unicode range of the
	   same length */
	  while (*p == ' ' || *p == '\t')
	      p++;
	  if (!strncmp(p, "idem", 4)) 
	    {
	      for (i=fp0; i<=fp1; i++)
		  unimapdesc_addpair(i, i, list, &listsz);
	      p += 4;
	    } 
	  else
	    {
	      un0 = sgetunicode(&p);
	      while (*p == ' ' || *p == '\t')
		  p++;
	      if (*p != '-') 
		{
		  fprintf(stderr,
			  _("unicode map: Corresponding to a range of font positions, "
			    "there should be a Unicode range\n"));
		  errno = EBFONT;
		  return -1;
		}
	      p++;
	      un1 = sgetunicode(&p);
	      if (un0 == 0xFFFF || un1 == 0xFFFF)
		{
		  fprintf(stderr,
			  _("unicode map: Bad Unicode range corresponding to "
			    "font position range 0x%x-0x%x\n"), fp0, fp1);
		  errno = EBFONT;
		  return -1;
		}
	      if (un1 - un0 != fp1 - fp0) 
		{
		  fprintf(stderr,
			  _("unicode map: Unicode range U+%x-U+%x not of the same length "
			    "as font position range 0x%x-0x%x\n"), un0, un1, fp0, fp1);
		  errno = EBFONT;
		  return -1;
		}
	      for(i=fp0; i<=fp1; i++)
		  unimapdesc_addpair(i, un0-fp0+i, list, &listsz);
	    }
	} 
      else
	{
	  /* no range; expect a list of unicode values
	   for a single font position */
	  
	  while ( (un0 = sgetunicode(&p)) != 0xFFFF )
	      unimapdesc_addpair(fp0, un0, list, &listsz);
	}
      while (*p == ' ' || *p == '\t')
	  p++;
      if (*p && *p != '#')
	  fprintf(stderr, _("unicode map: trailing junk (%s) ignored\n"), p);
    }

  unimapdesc_adjust (list);
  
  return 0;
}
