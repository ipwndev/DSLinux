#include <stdio.h>
#include <errno.h>

/*
 * like fseek, but doesn't barf when trying SEEK_CUR on a pipe
 */
int xfseek (FILE *stream, long offset, int whence)
{
  int r;
  long pos;
  
  r = fseek(stream, offset, whence);
  /* if it is un-fseek-able */
  if ( (r == -1) && ((errno == ESPIPE) || (errno == EBADF)) )
    {
      switch (whence)
	{
	case SEEK_CUR:
	  /* if asked to go backward, refuse */
	  if (whence < 0)
	    {
	      errno = EINVAL;
	      return -1;
	    }

	  /* GO */
	  for (; offset > 0; offset--)
	    {
	      if (EOF == getc(stream))
		{
		  errno = EOF;	       /* should be something like EFSMALL (cf. EFBIG) */
		  return -1;
		}
	    }

	  errno = 0; 
	  return 0;
      
	case SEEK_SET:
	  pos = ftell (stream);
	  /* if asked to go backward, refuse */
	  if ((offset - pos) < 0)
	    {
	      errno = EINVAL;
	      return -1;
	    }
	  
	  /* GO */
	  return xfseek (stream, offset-pos, SEEK_CUR);
      
	case SEEK_END:
	  /* if asked anything else than goto EOF, fail */
	  if (offset != 0)
	    {
	      errno = EINVAL;
	      return -1;
	    }

	  /* GO */
	  while (!feof(stream))
	      fread (&pos, 1, 1, stream);

	  return 0;
	  
	default:
	  errno = EINVAL;
	  return -1;
	}
    }
  /* fseek worked [r=0] or can't do anything [keep errno] */
  else
      return r;

}
