/*
 * mapscrn.c - version 0.92
 */

#include <config.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/kd.h>

#include <lct/local.h>
#include <lct/console.h>

static long int ctoi(unsigned char *s, int *is_unicode);
int old_screen_map_read_ascii(FILE *fp, unsigned char buf[]);
int uni_screen_map_read_ascii(FILE *fp, unicode buf[], int* is_unicode);

int screen_map_load(int fd, FILE *fp) 
{
  struct stat stbuf;
  unicode wbuf[E_TABSZ];
  unsigned char buf[E_TABSZ];
  int parse_failed = 0;
  int is_unicode;

  if (fstat(fp->_fileno, &stbuf)) 
    perror(_("Cannot stat map file")), exit(1);

  /* first try a UTF screen-map: either ASCII (no restriction) or binary (regular file) */
  if (!(parse_failed = (-1 == uni_screen_map_read_ascii(fp,wbuf,&is_unicode))) ||
      (S_ISREG(stbuf.st_mode) && 
       (stbuf.st_size == (sizeof(unicode) * E_TABSZ))))  /* test for binary UTF map by size */
    {
      if (parse_failed)
	{
	  if (-1 == fseek (fp, 0, SEEK_SET))
	    {
	      if (errno == ESPIPE)
		fprintf (stderr, _("16bit screen-map MUST be a regular file.\n")), exit (1);
	      else
		perror (_("fseek failed reading binary 16bit screen-map")), exit (1);
	    }
	  
	  if (fread(wbuf, sizeof(unicode) * E_TABSZ, 1, fp) != 1) 
	    perror(_("Cannot read [new] map from file")), exit(1);
#if 0
	  else
	    fprintf(stderr, _("Input screen-map is binary.\n"));
#endif
	}

      /* if it was effectively a 16-bit ASCII, OK, else try to read as 8-bit map */
      /* same if it was binary, ie. if parse_failed */
      if (parse_failed || is_unicode)
	{
	  if (ioctl(fd,PIO_UNISCRNMAP,wbuf))
	    perror(_("PIO_UNISCRNMAP ioctl")), exit(1);
	  else
	    return 0;
	}
    }

  /* rewind... */
  if (-1 == fseek (fp, 0, SEEK_SET))
    {
      if (errno == ESPIPE)
	fprintf (stderr, _("Assuming 8bit screen-map - MUST be a regular file.\n")), exit (1);
      else
	perror (_("fseek failed assuming 8bit screen-map")), exit (1);
    }
  
  /* ... and try an old 8-bit screen-map */
  if (!(parse_failed = (-1 == old_screen_map_read_ascii(fp,buf))) ||
      (S_ISREG(stbuf.st_mode) && 
       (stbuf.st_size == E_TABSZ)))  /* test for binary old 8-bit map by size */
    {
      if (parse_failed)
	{
	  if (-1 == fseek (fp, 0, SEEK_SET))
	    {
	      if (errno == ESPIPE)
		/* should not - it succedeed above */
		fprintf (stderr, _("fseek() returned ESPIPE !\n")), exit (1);
	      else
		perror (_("fseek for binary 8bit screen-map")), exit (1);
	    }
	  
	  if (fread(buf,E_TABSZ,1,fp) != 1) 
	    perror(_("Cannot read [old] map from file")), exit(1);
#if 0
	  else
	    fprintf(stderr, "Input screen-map is binary.\n");
#endif
	}
      
      if (ioctl(fd,PIO_SCRNMAP,buf))
	perror(_("PIO_SCRNMAP ioctl")), exit(1);
      else
	return 0;
    }
  else
    {
      fprintf(stderr, _("Error parsing symbolic map\n"));
      exit(1);
    }
}


/*
 * - reads `fp' as a 16-bit ASCII SFM file.
 * - returns -1 on error.
 * - returns it in `unicode' in an E_TABSZ-elements array.
 * - sets `*is_unicode' flagiff there were any non-8-bit
 *   (ie. real 16-bit) mapping.
 *
 * FIXME: ignores everything after second word
 */
int uni_screen_map_read_ascii(FILE *fp, unicode buf[], int *is_unicode)
{
  char buffer[256];				  /* line buffer reading file */
  char *p, *q;					  /* 1st + 2nd words in line */
  int in, on;					  /* the same, as numbers */
  int tmp_is_unicode;				  /* tmp for is_unicode calculation */
  int i;					  /* loop index - result holder */
  int ret_code = 0;				  /* return code */
  sigset_t sigset, old_sigset;
  
  assert (is_unicode);
  
  *is_unicode = 0;
  
  /* first 128 codes defaults to ASCII */ 
  for (i=0; i<128; i++) buf[i] = i;
  /* remaining defaults to replacement char (usually E_TABSZ = 256) */
  for ( ; i<E_TABSZ; i++) buf[i] = 0xfffd;
  
  /* block SIGCHLD */
  sigemptyset (&sigset);
  sigaddset (&sigset, SIGCHLD);
  sigprocmask (SIG_BLOCK, &sigset, &old_sigset);

  do
    {
      if (NULL == fgets(buffer, sizeof(buffer),fp))
	{
	  if (feof (fp))
	    break;
	  else
	    {
	      perror (_("uni_screen_map_read_ascii() can't read line"));
	      exit (2);
	    }
	}
      
      /* get "charset-relative charcode", stripping leading spaces */
      p = strtok(buffer," \t\n");

      /* skip empty lines and comments */
      if (!p || *p == '#')
	  continue;

      /* get unicode mapping */
      q = strtok(NULL," \t\n");
      if (q) 
	{
	  in = ctoi(p, NULL);
	  if (in < 0 || in > 255)
	    {
	      ret_code = -1;
	      break;
	    }
	  
	  on = ctoi(q, &tmp_is_unicode);
	  if (in < 0 && on > 65535)
	    {
	      ret_code = -1;
	      break;
	    }
	  
	  *is_unicode |= tmp_is_unicode;
	  buf[in] = on;
	}
      else 
	{
	  ret_code = -1;
	  break;
	}
    }
  while (1); /* terminated by break on feof() */

  /* restore sig mask */
  sigprocmask (SIG_SETMASK, &old_sigset, NULL);

  return ret_code;
}


int old_screen_map_read_ascii(FILE *fp, unsigned char buf[])
{
  char buffer[256];
  int in, on;
  char *p, *q;
  
  for (in=0; in<256; in++) buf[in]=in;
  
  while (fgets(buffer,sizeof(buffer)-1,fp)) 
    {
      p = strtok(buffer," \t\n");
      
      if (!p || *p == '#')
	  continue;

      q = strtok(NULL," \t\n#");
      if (q) 
	{
	  in = ctoi(p, NULL);
	  if (in < 0 || in > 255) return -1;
	  
	  on = ctoi(q, NULL);
	  if (in < 0 && on > 255) return -1;
	  
	  buf[in] = on;
	}
      else return -1;
    }
  
  return(0);
}


/*
 * - converts a string into an int.
 * - supports dec and hex bytes, hex UCS2, single-quoted byte and UTF8 chars.
 * - returns the converted value
 * - if `is_unicode != NULL', use it to tell whether it was unicode
 *
 * CAVEAT: will report valid UTF mappings using only 1 byte as 8-bit ones.
 */
long int ctoi(unsigned char *s, int *is_unicode) 
{
  int i;
  size_t ls;
  
  ls = strlen(s);
  if (is_unicode) *is_unicode = 0;
  
  /* hex-specified UCS2 */
  if ((strncmp(s,"U+",2) == 0) &&
      (strspn(s+2,"0123456789abcdefABCDEF") == ls-2))
    {
      sscanf(s+2,"%x",&i);
      if (is_unicode) *is_unicode = 1;
    }

  /* hex-specified byte */
  else if ((ls <= 4) && (strncmp(s,"0x",2) == 0) &&
      (strspn(s+2,"0123456789abcdefABCDEF") == ls-2))
      sscanf(s+2,"%x",&i);

  /* oct-specified number (byte) */
  else if ((*s == '0') &&
	   (strspn(s,"01234567") == ls))
      sscanf(s,"%o",&i);
  
  /* dec-specified number (byte) */
  else if (strspn(s,"0123456789") == ls) 
      sscanf(s,"%d",&i);
  
  /* single-byte quoted char */
  else if ((strlen(s) == 3) && (s[0] == '\'') && (s[2] == '\''))
      i=s[1];
  
  /* multi-byte UTF8 quoted char */
  else if ((s[0] == '\'') && (s[ls-1] == '\''))
    {
      s[ls-1] = 0;	/* ensure we'll not "parse UTF too far" */
      i = utf8_to_ucs2(s+1);
      if (is_unicode) *is_unicode = 1;
    }
  else 
      return(-1);
  
  return(i);
}


#warning saveoldmap() should accept FILE* instead of char* as arg ?
void saveoldmap(int fd, char *omfil) 
{
  FILE *fp;
  char buf[E_TABSZ];
#ifdef GIO_UNISCRNMAP
  unicode xbuf[E_TABSZ];
  int is_old_map = 0;

  if (ioctl(fd,GIO_UNISCRNMAP,xbuf))
    {
      perror(_("GIO_UNISCRNMAP ioctl error"));
#endif
      if (ioctl(fd,GIO_SCRNMAP,buf))
	{
	  perror(_("GIO_SCRNMAP ioctl error"));
	  exit(1);
	}
      else 
	  is_old_map = 1;
#ifdef GIO_UNISCRNMAP
    }
#endif
  
  if ((fp = fopen(omfil, "w")) == NULL) 
    {
      perror(omfil);
      exit(1);
    }

#ifdef GIO_UNISCRNMAP
  if (is_old_map)
    {
#endif
      if (fwrite(buf,E_TABSZ,1,fp) != 1) 
        {
	  perror(_("Error writing map to file"));
	  exit(1);
	}
#ifdef GIO_UNISCRNMAP
    }
  else 
      if (fwrite(xbuf, sizeof(unicode) * E_TABSZ,1,fp) != 1) 
        {
	  perror(_("Error writing map to file"));
	  exit(1);
	}
#endif
  
  fclose(fp);
}
