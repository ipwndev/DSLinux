#include <stdio.h>

FILE *xfopen (const char *path, const char *mode, FILE* minus_meaning)
{
  if (minus_meaning && path && (path[0] == '-') && (path[1] == 0))
      return minus_meaning;
      
  return fopen(path, mode);
}
