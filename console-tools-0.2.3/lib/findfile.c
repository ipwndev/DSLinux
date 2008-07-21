#include <config.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sysexits.h>
#include <signal.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <lct/local.h>
#include <lct/utils.h>

typedef enum
{
  compressed_not,
  compressed_gzip,
  compressed_compress,
  compressed_bzip2,
  compressed_lzop,
  compressor_MAX
} compressed_type;

char* decompressor_names[compressor_MAX] =
{
  "",
  "gunzip",
  "uncompress",
  "bunzip2",
  "lzop"
};

char* decompressor_options[compressor_MAX] =
{
  NULL,
  NULL,
  NULL,
  NULL,
  "-d" /* lzop has no "standalone" decompressor */
};

static const char *compressor_suffixes[] = {
  "",
  ".gz",
  ".lzo",
  ".Z",
  ".bz2",
  0 };

static pid_t pid_feeder = 0;
static pid_t pid_decompressor = 0;
static pid_t pid_identifier = 0;
static struct sigaction old_sa;
static compressed_type is_compressed;		  /* current file's compression */


static void print_status (int status, char* name, char* parent)
{
  if (WIFEXITED(status))
    fprintf (stderr, _("%s: %s child exited anormally with code %d.\n"),
	     parent, name, WEXITSTATUS(status));
  else if (WIFSIGNALED(status))
    fprintf (stderr, _("%s: %s child was terminated by signal %d.\n"),
	     parent, name, WTERMSIG(status));
  else if (WIFSTOPPED(status))
    fprintf (stderr, _("%s: %s child was stopped by signal %d.\n"),
	     parent, name, WSTOPSIG(status));
}

static void chld_handler(int sig)
{
  int status;
  pid_t pid;
  char* name;
  
  /* FIXME: How to pass context pointer to here? */
  pid = wait(&status);
  
  if (pid == pid_feeder)
    {
      name = N_("Feeder");
      pid_feeder = 0;
    }
  else if (pid == pid_decompressor)
    {
      name = N_("Decompressor");
      pid_decompressor = 0;
    }
  else if (pid == pid_identifier)
    {
      name = N_("Identifier");
      pid_identifier = 0;
    }
  else
    {
      /* it's not one of our children */
      /* ...try to be nice to main program's handler - UNTESTED */
      if ((old_sa.sa_handler != SIG_IGN) && (old_sa.sa_handler != SIG_DFL))
	  old_sa.sa_handler (sig);
      return;
    }

  if (WIFEXITED(status) &&
      ((WEXITSTATUS(status) == 0) ||
       ((WEXITSTATUS(status) == 2) && (is_compressed == compressed_gzip))))
    {
      /* when no more of our children,
       * restore application's SIGCHLD handler 
       */
      if (! (pid_feeder || pid_decompressor || pid_identifier))
	{
	  sigaction (SIGCHLD, &old_sa, NULL);
	  memset (&old_sa, 0, sizeof(old_sa));
	}
    }
  else
    {
      print_status (status, _(name), _("chld_handler()"));
      exit (1);
    }

#if 0
  if (WIFEXITED(status))
      fprintf (stderr, _("%s child exited OK.\n"), _(name));
#endif
}


static int feed (FILE* inf, int outfd)
{
  char buf[512];		       /* FIXME: would it safe to make this larger ? */
  int count;
  
  do
    {
      if (-1 == (count = fread (buf, 1, sizeof(buf), inf)))
	  return -1;
      if ((count > 0) && (-1 == (count = write (outfd, buf, count))))
	  return -1;
    }
  while (count);
  
  return 0;
}


/* convenience macro - return FD */
static FILE* do_pipe(const char* filename, FILE* minus_meaning,
				const struct magicset* magics, int* magic_return)
{
  FILE *inf;
  int file[2];		/* pipe transmitting original file (to gzip or identifier) */
  int uncompressed[2];	/* pipe transmitting uncompressed file (raw or gunzipped) */
  int identified[2];	/* pipe transmitting uncompressed magically-identified file */
  int is_seekable;
  unsigned char zmagicbuf[9];	/* buffer used to get compressed magic number */
  unsigned i;
  FILE *in;
  struct magic* m;		       /* loop index */
  
  /* the input file */
  if (NULL == (inf = xfopen (filename, "r", minus_meaning)))
    return NULL;

  /* read for magic */
  if (fread (zmagicbuf, sizeof(zmagicbuf), 1, inf) != 1)
    {
      perror (_("fread magic")); /* Failed to read the magic ID at the start of the file */
      exit (1);
    }

  /* identify compressed format by magic */
  if ((zmagicbuf[0] == 037) && (zmagicbuf[1] == 0213))/* gzip */
    {
      is_compressed = compressed_gzip;
      is_seekable = 0;
    }
  else if ((zmagicbuf[0] == 037) && (zmagicbuf[1] == 0235))	/* compress */
    {
      is_compressed = compressed_compress;
      is_seekable = 0;
    }
  else if ((zmagicbuf[0] == 'B') && (zmagicbuf[1] == 'Z')
	   && (zmagicbuf[2] == 'h'))		  /* bzip2 */
    {
      is_compressed = compressed_bzip2;
      is_seekable = 0;
    }
  else if (!strncmp (zmagicbuf, "\211LZO\0\r\n\32\n", 9))
    {
      is_compressed = compressed_lzop;
      is_seekable = 0;
    }
  else
    {
      is_compressed = 0;
      
      is_seekable = (-1 != fseek (inf, 0, SEEK_SET));
      
      /* uncompressed seekable file and no need for
       * magic identification, just use raw file */
      if (is_seekable && !magic_return)
	  return inf;
    }

  /*
   * case when file is not seekable
   * ie, either compressed, or a "-" on command-line
   */
  if (!is_seekable)
    {
      /* open pipes */
      if (-1 == pipe (uncompressed))
	  return NULL;

      /*
       * case when file is compressed
       */
      if (is_compressed)
	{
	  if (-1 == pipe (file))
	      return NULL;
	  
	  /* fork a decompressor */
	  switch (pid_decompressor = fork())
	    {
	    case -1:
	      perror (_("decompressor fork"));
	      exit (1);
	    case 0:	/* gzip child */
	      /* unused FD's: */
	      if (-1 == close (file[1])) 
		  {
		    perror (_("decompressor: close file[1]"));
		    exit(1);
		  }
	      if (-1 == close (uncompressed[0])) 
		  {
		    perror (_("decompressor: close uncompressed[0]"));
		    exit(1);
		  }
	      
	      /* I/O pipes */
	      /*   "file" feeding stdin */
	      fclose (stdin);
	      if (-1 == dup2 (file[0], STDIN_FILENO)) 
		  {
		    perror (_("decompressor: dup2 file[0]"));
		    exit(1);
		  }
	      /*   "uncompressed" fed from stdout */
	      fclose (stdout);
	      if (-1 == dup2 (uncompressed[1], STDOUT_FILENO)) 
		  {
		    perror (_("decompressor: dup2 uncompressed[1]"));
		    exit(1);
		  }
	      
	      execlp (decompressor_names[is_compressed],
		      decompressor_names[is_compressed],
		      decompressor_options[is_compressed],
		      NULL);
	      if (errno==ENOENT)
		fprintf (stderr, _("Decompressor program not found: %s\n"),
			 decompressor_names[is_compressed]);
	      else
		perror (_("decompressor: execlp"));
	      
	      exit (1);
	      
	    default:	/* parent */
	      i = close (file[0]);
	      assert (-1 != i);
	      i = close (uncompressed[1]);
	      assert (-1 != i);
	    }
	}
      
      /* fork a feeder-process (for gzip/compress, or for reader directly) */
      switch (pid_feeder = fork())
	{
	case -1:
	  perror (_("feeder fork"));
	  exit (1);
	case 0:	/* feeder child: read from `inf' - write to `file' */
	  /* unused FD's: */
	  if (!is_compressed)
	      if (-1 == close (uncompressed[0])) 
		  {
		    perror (_("feeder: close uncompressed[0]"));
		    exit(1);
		  }
	  
	  /* feed zmagicbuf, whether magic or not */
	  if (write (is_compressed ? file[1] : uncompressed[1], zmagicbuf, sizeof(zmagicbuf))
	      != sizeof(zmagicbuf))
	    {
	      perror (_("feeder: write zmagicbuf"));
	      exit(1);
	    }
	  
	  if (feed (inf, is_compressed ? file[1] : uncompressed[1]))
	    {
	      perror (_("feeder: feed"));
	      exit(1);
	    }
	  
	  /* EOF */
	  exit (0);
	default:	/* parent */
	  i = close (is_compressed ? file[1] : uncompressed[1]);
	  assert (-1 != i);
	  i = fclose (inf);
	  assert (-1 != i);
	}
    } /* if unseekable */

  /*
   * test uncompressed file by magic if asked to
   */
  if (magic_return)
    {
      char* magicIDbuf;
      
       /* setup FD's */
      if (is_seekable)
	  in = inf;
      else 
	  in = fdopen (uncompressed[0], "r");
      
      /* try to read enough to identify the magic */
      magicIDbuf = (char*)malloc(magics->max_length_hint);
      if (fread (magicIDbuf, magics->max_length_hint, 1, in) != 1)
	{
#ifndef NDEBUG
	  perror (_("fread"));
#endif
	  errno = EINVAL;
	  return NULL;
	}

      /* check each magic until one matches */
      for (m = (struct magic*)magics->m; m->id != FF_END; m++)
	{
	  if (!m->magic)
	      *magic_return = m->id;
	  else
	    {
	      int ok = 1;
	      
	      for (i = 0; ok && i < m->length; i++)
		  ok = ok && ( ( magicIDbuf[i] & m->mask[i] ) == m->magic[i] );
	      
	      if (ok)
		{
		  *magic_return = m->id;
		  break; /* for magics */
		}
	    }
	}
      
      /* rewind if possible */
      if (is_seekable)
	  rewind (inf);
      else /* feed to user using a child process */
	{
	  if (-1 == pipe (identified))
	      return NULL;

	  switch (pid_identifier = fork())
	    {
	    case -1:
	      perror (_("identifier fork"));
	      exit (1);
	    case 0: /* identifier child */
	      if (-1 == close (identified[0])) 
		{
		  perror (_("identifier: close identified[0]"));
		  exit (1);
		}

	      /* feed magicIDbuf */
	      if (write (identified[1], magicIDbuf, magics->max_length_hint) != (int)magics->max_length_hint)
		{
		  perror (_("identifier: write magicIDbuf"));
		  exit (1);
		}
	      
	      if (feed (in, identified[1]))
		{
		  perror (_("identifier: feed"));
		  exit (1);
		}
	      
	      /* EOF */
	      exit (0);
	    default: /* parent */
	      if (is_seekable)
		{
		  i = close (uncompressed[0]);
		  assert (-1 != i);
		}
	      else
		{
		  i = fclose (inf);
		  assert (-1 != i);
		}
	      assert (-1 != i);
	      i = close (identified[1]);
	    }
	}
    }

  return is_seekable ? inf 
      : fdopen (magic_return ? identified[0] 
		: uncompressed[0], "r");
}


static void maybekillzombie (pid_t *pid, char* name)
{
  int status;
  
  /* if pid is 0, process is already dead */
  if (!*pid) return;

  /* return if no child to wait for */
  if (0 == waitpid (*pid, &status, WNOHANG)) return;
      
  if (WIFEXITED(status) &&
      ((WEXITSTATUS(status) == 0) ||
       ((WEXITSTATUS(status) == 2) && (is_compressed == compressed_gzip))))
    {
#ifndef NDEBUG
      fprintf (stderr, _("Zombie %d caught.\n"), *pid);
#endif
      *pid = 0;
    }
  else if (WIFEXITED(status) || WIFSIGNALED(status) || WIFSTOPPED(status))
    {
      print_status (status, name, "maybekillzombie()");
      exit (1);
    }
#ifndef NDEBUG
  else
    fprintf (stderr, _("One for nothing.\n"));
#endif
}


static int findfile_recursive(const char *fnam, const char *path, const char **suffixes,
			      char *fullname, size_t maxfullength,
			      int min_depth, int max_depth)
{
  const char **sp, **cs;
  char* target;					  /* what to search right now */
  char* first_slash;				  /* NULL means look for a file */
  char* newpath = 0;				  /* full path to target */
  char* ptr_to_target = 0;			  /* save allocated ptr */
  struct stat st;
  int found = 0;				  /* entry found */
  int retcode = 0;

  assert (fnam);
  assert (fnam[0]);
  assert ((!path) || path[0]);			  /* if not NULL, should not be "" */

  /* check for leading "/" and tune args */
  if ((path == NULL) && (fnam[0] == '/'))
    {
      fnam++;
      path = "";
    }


ffr_next_component:
  /* extract the dir entry to look for into `target' */
  ptr_to_target = target = xstrdup(fnam);
  first_slash = strchr(fnam, '/');
  if (first_slash)
    target[first_slash-fnam] = 0;

  /* readability macro */
#define WANT_A_DIR (first_slash)
  
  /* interpret a sequence of *'s to raise max_depth */
  if (target[0] == '*')
    {
      while (target[0] == '*')
	{
	  if (max_depth>=0) max_depth++;	  /* do not increment if -1 */
	  target++;
	}
      /* something else after *'s is an error */
      if (target[0])
	{
	  errno = EINVAL;
	  retcode = -1;
	  goto ffr_exit;
	}
      /* next path component */
      fnam = first_slash + 1;
      free (ptr_to_target);
      goto ffr_next_component;
    }
  

  /* maybe look for target in this dir */
  if (min_depth == 0)
    {
      /* set full path to target in newpath */
      if (path)
	{
	  newpath = xmalloc (strlen(path) + strlen (target) + 2);
	  sprintf (newpath, "%s/%s", path, target);
	}
      else
	{
	  newpath = xmalloc (strlen (target) + 1);
	  strcpy (newpath, target);
	}
    
    
      if (WANT_A_DIR)
	/* look for it */
	if (stat (newpath, &st) == -1)
	  {
	    if (first_slash)
	      {
		errno = ENOENT;
		retcode = -1;
		goto ffr_exit;
	      }
	  }
	else
	  /* if we look for a dir */
	  if (S_ISDIR(st.st_mode))
	    {
	      retcode = findfile_recursive(first_slash+1, newpath, suffixes,
					   fullname, maxfullength,
					   min_depth, max_depth);
	      found = 1;
	    }
	  else
	    {
	      /* not a dir */
	      errno = ENOTDIR;
	      retcode = -1;
	      goto ffr_exit;
	    }
      else
	/* we want a file */
	for (sp = suffixes; *sp && !found; sp++) 
	  for (cs = compressor_suffixes; *cs && !found; cs++) 
	    {
	      if (strlen(newpath) + strlen(*sp) + strlen(*cs) + 1 > maxfullength)
		{
		  fprintf (stderr, _("Warning: ignoring a possible path (too long)."));
		  continue;
		}
	      sprintf(fullname, "%s%s%s", newpath, *sp, *cs);

	      /* check for file */
	      if ((-1 != stat(fullname, &st)) && (!S_ISDIR(st.st_mode)))
		{
		  found = 1;
		  retcode = 0;
		}
	    }
    }

  
  /* maybe search in subdirs */
  if ((!found) && (max_depth > 0))
    {
      DIR* d;
      struct dirent *de;
      
      if (NULL == (d = opendir(path)))
	return -1;
      
      while ((!found) && (de = readdir(d)) != NULL)
	if (de->d_name[0] != '.')
	  {
	    free (newpath);
	    newpath = xmalloc (strlen(path) + strlen (de->d_name) + 2);
	    sprintf (newpath, "%s/%s", path, de->d_name);
	  
	    if ((-1 != stat (newpath, &st)) && S_ISDIR(st.st_mode))
	      {
		retcode = findfile_recursive (fnam, newpath, suffixes, fullname, maxfullength,
					      min_depth ? (min_depth - 1) : 0,
					      max_depth ? (max_depth - 1) : 0);
		if (retcode == 0)
		  found = 1;
		else if (errno != ENOENT)
		  {
		    closedir(d);		  /* should not change errno */
		    goto ffr_exit;
		  }
	      }
	  }
      closedir(d);
    }

  /* if nothing was found */
  if (!found)
    {
      errno = ENOENT;
      retcode = -1;
    }
  
ffr_exit:
  {
    int saved_errno = errno;
    if (newpath) free (newpath);
    if (ptr_to_target) free (ptr_to_target);
    errno = saved_errno;
  }
  return retcode;
}

FILE* findfile_simple(const char *fnam, const char **dirpath, const char **suffixes)
{
  return findfile (fnam, dirpath, suffixes,
		   NULL, 0, NULL, NULL, NULL);
}

/* find input file; leave name in fullname[] */
FILE* findfile(const char *fnam, const char **dirpath, const char **suffixes,
	       char *fullname, size_t maxfullength, FILE* minus_meaning,
	       const struct magicset* magics, int* magic_return)
{
  const char **dp;
  FILE *fp = NULL;
  struct sigaction sa;
  int found;
  sigset_t sigset, old_sigset;
#ifndef NDEBUG
  int retries = 300;
#else
  int retries = 30;
#endif

  if (maxfullength == 0)
    {
      maxfullength = 1024;
      fullname = (char*)malloc (maxfullength * (sizeof(char)));
      if (fullname == NULL) return NULL;
    }
  else
    assert (fullname);
  
  /* if a dead child of do_pipe was not caught in/after a
   * former call, fail
   */
  while (retries && (pid_feeder || pid_decompressor || pid_identifier))
    {
#ifndef NDEBUG
      fprintf (stderr, "f = %d, d = %d, i = %d\n", pid_feeder, pid_decompressor, pid_identifier);
#endif
      usleep (100);
#if 1
      maybekillzombie(&pid_feeder, _("feeder"));
      maybekillzombie(&pid_decompressor, _("decompressor"));
      maybekillzombie(&pid_identifier, _("identifier"));
#endif
      retries--;
    }
  if (!retries)
    {
      fprintf (stderr, _("findfile(): timeout waiting for undead child(ren) ?\n"));
      exit (EX_SOFTWARE);
    }

  /* install custom signal handler for dead children */
  /* only if our previous handler is no more here */
  sigaction (SIGCHLD, NULL, &sa);
  if (sa.sa_handler != chld_handler)
    {
      memset (&sa, 0, sizeof (sa));
      sa.sa_handler = chld_handler;
      sigaction (SIGCHLD, &sa, &old_sa);
    }

  /* if we are asked to test a magic, and we don't have a magic list... */
  if (magic_return && !magics)
    {
      /* ...then say we don't know */
      *magic_return = FF_UNKNOWN;
      /* and do as if we weren't asked to */
      magic_return = NULL;
    }
  
  /* try to find an existing file according to PATH dirpath, and to default suffixes */
  found = 0;
  for (dp = dirpath; *dp && !found; dp++) 
    {
      if (*fnam == '/' && **dp)	       /* if fnam is an absolute filename spec, */ 
	  continue;		       /* then skip until we have an empty entry in path */

      {
	char fname_buf[1024];

	if (strlen(*dp) + strlen(fnam) +1 > sizeof (fname_buf))
	  {
	    fprintf (stderr, _("Buffer overflow - aborting\n"));
	    exit (1);
	  }
	
	sprintf(fname_buf, "%s%s", *dp, fnam);

	if (-1 != findfile_recursive (fname_buf, NULL, suffixes, fullname, maxfullength, 0, 0))
	  {
	    /* block SIGCHLD */
	    sigemptyset (&sigset);
	    sigaddset (&sigset, SIGCHLD);
	    sigprocmask (SIG_BLOCK, &sigset, &old_sigset);

	    fp = do_pipe(fullname, minus_meaning, magics, magic_return);

	    /* restore sig mask */
	    sigprocmask (SIG_SETMASK, &old_sigset, NULL);

	    /* fp may be NULL if do_pipe() fails */
	    found = 1;
	  }
	else if (errno != ENOENT)
	  {
	    /* return, pass errno */
	    fp = NULL;
	    found = 1;
	  }
      }
    }

  return found ? fp : NULL;
}
