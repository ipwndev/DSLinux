#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include <lct/utils.h>

#warning  RETURNs should free memory
#warning  should use paged table as in kernel

int sfm_fallback_read (FILE* f, unicode** *sfmf, unsigned *size)
{
#ifdef HAVE_GETLINE
  char* buf = NULL;
  ssize_t buf_size = 0;
  ssize_t buf_read;
#else
  char buf[1024];
#endif
  char* scanpos, *savedscpos;
  unsigned linecount, tokencount;
  unicode u;
  sigset_t sigset, old_sigset;

  /* check args */
  assert (f);
  assert (size);
  assert (sfmf);
  
  /* some validity check */
  if (0 == *size)
    {
      /* *sfmf should be NULL */
      if (*sfmf)
	{
	  errno = EINVAL;
	  return -1;
	}

      /* alloc initial space */
      *size = 512;
      if (NULL == (*sfmf = (unicode**) malloc ((*size) * sizeof(unicode*))))
	return -1;
    }

  /* block SIGCHLD */
  sigemptyset (&sigset);
  sigaddset (&sigset, SIGCHLD);
  sigprocmask (SIG_BLOCK, &sigset, &old_sigset);

  /* process the file */
  linecount = 0;
  while (!feof(f))
    {
      /* read a line */
#ifdef HAVE_GETLINE
      if ((buf_read = getline (&buf, &buf_size, f)) == -1)
	{
	  if (feof(f))				  /* should not modify errno */
	    break;
	  else
	    return -1;
	}
#else  /* HAVE_GETLINE */
      if (fgets (buf, sizeof (buf), f) == NULL)
	{
	  if (feof(f))				  /* should not modify errno */
	    break;
	  else
	    return -1;
	}
#endif /* HAVE_GETLINE */

      /* skip whitespace */
      scanpos = buf + strspn (buf, " \t\n\r");

      /* ignore empty lines */
      if (!scanpos[0])
	continue;
      
      /* ignore comments */
      if (scanpos[0] == '#')
	continue;
      
      /* alloc enought place so we don't need to check overflow */
      /* Note: a unicode *is* 6 chars long ("U+0034", never "U+34") */
      /* we add one more for terminator */
      savedscpos = scanpos;
      if (NULL == ((*sfmf)[linecount] = (unicode*) malloc ((1 + strlen(savedscpos)/6)
							   * sizeof(unicode))))
	return -1;
      
      /* process the line */
      tokencount = 0;
      while (0xFFFF != (u = sgetunicode (&scanpos)))
	{
	  (*sfmf)[linecount][tokencount] = u;
	  scanpos += strspn (scanpos, " \t\n\r");
	  tokencount++;
	}
      
      /* adjust memory for this line */
      if ((strlen(savedscpos)/6) > (tokencount))
	if (NULL == ((*sfmf)[linecount] = (unicode*) realloc ((*sfmf)[linecount],
							      tokencount * sizeof(unicode))))
	  return -1;

      /* add terminator */
      (*sfmf)[linecount][tokencount] = (unicode)0xFFFF;
      
      /* next line */
      linecount ++;
      
      /* if we reached the limit, double allocated size */
      if (linecount >= *size)
	{
	  *size <<= 1;
	  if (NULL == (*sfmf = (unicode**) realloc (*sfmf, (*size) * sizeof(unicode*))))
	    return -1;
	}
    }
  
  /* unblock SIGCHLD */
  sigprocmask (SIG_SETMASK, &old_sigset, NULL);

  /* adjust memory */
  *size = linecount;
  if (NULL == (*sfmf = (unicode**) realloc (*sfmf, (*size) * sizeof(unicode*))))
    return -1;

#ifdef HAVE_GETLINE
  /* cleanup glibc-allocated memory */
# warning sfm_fallback_read() does not free mem on "return -1"
  if (buf_size) free (buf);
#endif
  
  /* success */
  return 0;
}


#warning sfm_fallback_add() is awfully inefficient

/* builds unipairs from sfmf and ilist
 * and adds them to olist
 */
int sfm_fallback_add (unicode** sfmf, unsigned size,
		      struct unimapdesc* ilist,
		      struct unimapdesc* olist)
{
  unsigned count;
  struct unipair* ptr;
  unicode* arr;
  int listsz;

  assert (ilist);
  assert (olist);
  
  listsz = ilist->entry_ct;
  
  /* for each fallback entry */
  for (count = 0; count < size; sfmf++, count++)
    {
      /* for each possible fallback */
      for (arr = sfmf[0]; arr[0] != (unicode)0xFFFF; arr++)
	{
	  /* try to find it in `ilist' */
	  for (ptr = ilist->entries + ilist->entry_ct - 1; 
	       (ptr > ilist->entries) && (ptr->unicode != arr[0]);
	       ptr--);
	  
	  /* if found something */
	  if (ptr != ilist->entries)
	    {
	      /* if the main unicode was found, skip to next fallback entry */
	      if (ptr->unicode == sfmf[0][0])
		  break;
	  
	      /* else: a possible fallback was found, use it */
	      unimapdesc_addpair (ptr->fontpos, sfmf[0][0], olist, &listsz);
	      break;
	    }
	}
    }
  
  unimapdesc_adjust (olist);
  
  return 0;
}
