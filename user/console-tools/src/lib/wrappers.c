#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lct/local.h>
#include <lct/utils.h>

/* from kbd 0.96 */
void* xmalloc(size_t n)
{
  void *p = malloc(n);
  if (p == NULL)
    {
      fprintf(stderr, _("Out of Memory\n"));
      exit(1);
    }
  return p;
}

char* xstrdup(char *p)
{
  char *q = strdup(p);
  if (q == NULL)
    {
      fprintf(stderr, _("Out of Memory?\n"));
      exit(1);
    }
  return q;
}
