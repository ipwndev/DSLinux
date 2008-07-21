#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <lct/local.h>
#include <lct/utils.h>

/*
 * Skip spaces and read U+1234 or return -1 for error.
 * Return first non-read position in *p0 (unchanged on error).
 */ 
unicode sgetunicode(char **p0)
{
  char *p = *p0;

  while (*p == ' ' || *p == '\t')
    p++;
  if (*p != 'U' || p[1] != '+' || !isxdigit(p[2]) || !isxdigit(p[3]) ||
      !isxdigit(p[4]) || !isxdigit(p[5]) || isxdigit(p[6]))
    return (unicode)0xFFFF;
  *p0 = p+6;
  return strtol(p+2,0,16);
}


void unimapdesc_addpair(int fp, unicode un, struct unimapdesc *list, int *listsz)
{
  if (list->entry_ct == *listsz)
    {
      *listsz += 4096;
      list->entries = realloc((char *)list->entries, (*listsz) * sizeof(struct unipair*));
      if (!list->entries) 
	{
	  fprintf(stderr, _("loadunimap: out of memory\n"));
	  exit(1);
	}
    }
  list->entries[list->entry_ct].fontpos = fp;
  list->entries[list->entry_ct].unicode = un;
  list->entry_ct++;
}

/* to use after _addpair's, which overreserves memory */
void unimapdesc_adjust(struct unimapdesc* list)
{
  list->entries = realloc((char *)list->entries, (list->entry_ct) * sizeof(struct unipair*));
}
